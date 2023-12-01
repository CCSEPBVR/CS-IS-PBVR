#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "stereocamera.h"

#include "TimeMeasureUtility.h"

#include <cassert>
#include <cuda.h>
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>

#include <cublas_v2.h>

#include <crt\host_defines.h>

extern kvs::Int64 frame_index;

#ifdef ENABLE_CUDA_DYNAMIC_PARALLELISM
__global__ void child(ZEDThreadDataEx &thread_data) {
  thread_data.zed.retrieveImage(thread_data.zed_image[0], sl::VIEW::LEFT, sl::MEM::GPU);
  thread_data.zed.retrieveImage(thread_data.zed_image[1], sl::VIEW::RIGHT, sl::MEM::GPU);

  sl::ERROR_CODE leftStatus = thread_data.zed.retrieveMeasure(thread_data.zed_depth[0], sl::MEASURE::DEPTH, sl::MEM::GPU);
  sl::ERROR_CODE rightStatus = thread_data.zed.retrieveMeasure(thread_data.zed_depth[1], sl::MEASURE::DEPTH_RIGHT, sl::MEM::GPU);

  thread_data.new_frame = true;
}
#endif

// ZED image grab thread
void __zed_runner__(ZEDThreadData &thread_data) {
  sl::RuntimeParameters runtime_parameters;
  runtime_parameters.enable_depth = true;
  runtime_parameters.sensing_mode = sl::SENSING_MODE::FILL;
  // runtime_parameters.sensing_mode = sl::SENSING_MODE::STANDARD;

  // Set the ZED's CUDA context to this separate CPU thread
  cuCtxSetCurrent(thread_data.zed.getCUDAContext());
  // Loop while the main loop is not over
  while (thread_data.run) {
    // try to grab a new image
    if (thread_data.zed.grab(runtime_parameters) == sl::ERROR_CODE::SUCCESS) {
#ifdef ENABLE_CUDA_DYNAMIC_PARALLELISM
      // child<<<1,1>>>(thread_data);
#else // ENABLE_CUDA_DYNAMIC_PARALLELISM
      //  copy both left and right images
      MAKE_AND_START_TIMER(B_1);
      MAKE_AND_START_TIMER(B_1_1);
#ifdef ENABLE_MTX_LOCK
      thread_data.mtx.lock();
#endif
      STOP_AND_RECORD_TIMER(B_1_1);

      MAKE_AND_START_TIMER(B_1_2);
      thread_data.zed.retrieveImage(thread_data.zed_image[0], sl::VIEW::LEFT, sl::MEM::GPU);
      thread_data.zed.retrieveImage(thread_data.zed_image[1], sl::VIEW::RIGHT, sl::MEM::GPU);
      STOP_AND_RECORD_TIMER(B_1_2);

      MAKE_AND_START_TIMER(B_1_3);

#ifdef DEBUG_DEPTH_MAP_ON_GPU
      std::cerr << "__zed_runner__(): call retrieveMeasure(GPU)" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

      sl::ERROR_CODE leftStatus = thread_data.zed.retrieveMeasure(thread_data.zed_depth[0], sl::MEASURE::DEPTH, sl::MEM::GPU);
      sl::ERROR_CODE rightStatus = thread_data.zed.retrieveMeasure(thread_data.zed_depth[1], sl::MEASURE::DEPTH_RIGHT, sl::MEM::GPU);

#ifdef DEBUG_DEPTH_MAP_ON_GPU
      std::cerr << "__zed_runner__(): finish retrieveMeasure(GPU)" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

      STOP_AND_RECORD_TIMER(B_1_3);

      if (thread_data.show_info) {
        std::cout << "retrieveMeasure:" << sl::toString(leftStatus) << ", " << sl::toString(rightStatus) << std::endl;
      }

      MAKE_AND_START_TIMER(B_1_4);
#ifdef ENABLE_MTX_LOCK
      thread_data.mtx.unlock();
#endif
      STOP_AND_RECORD_TIMER(B_1_4);

      thread_data.new_frame = true;
      STOP_AND_RECORD_TIMER(B_1);

      // TODO
      sl::sleep_ms(75);
      // std::this_thread::sleep_for(std::chrono::milliseconds(50));
#endif // ENABLE_CUDA_DYNAMIC_PARALLELISM
    } else {
      MAKE_AND_START_TIMER(B_2);
      sl::sleep_ms(2);
      STOP_AND_RECORD_TIMER(B_2);
    }
  }

  // Release the memory
  for (int eye = 0; eye < 2; eye++) {
    thread_data.zed_image[eye].free();
    thread_data.zed_depth[eye].free();
  }
  // Close the zed Camera
  thread_data.zed.close();
}

sl::UNIT StereoCamera::m_units; // For depth measure. default 0.2m - 15m
float StereoCamera::m_min_depth;
float StereoCamera::m_max_depth;

StereoCamera::StereoCamera() {
  m_units = sl::UNIT::METER;
  m_min_depth = 0.2f;
  m_max_depth = 15.0f;
}

bool StereoCamera::initialize() {
#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "StereoCamera::initialize(): start" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  // Initialize the ZED Camera
  sl::InitParameters init_parameters;
  init_parameters.depth_mode = sl::DEPTH_MODE::QUALITY; // NONE, PERFORMANCE, QUALITY or ULTRA
  init_parameters.camera_resolution = sl::RESOLUTION::HD720;
  //	init_parameters.coordinate_units = sl::UNIT::METER; // For depth measure. default 0.2m - 15m
  //	init_parameters.depth_minimum_distance = 0.2f;
  //	init_parameters.depth_maximum_distance = 15.0f;
  init_parameters.coordinate_units = units(); // For depth measure. default 0.2m - 15m
  init_parameters.depth_minimum_distance = minDepth();
  init_parameters.depth_maximum_distance = maxDepth();
  init_parameters.enable_right_side_measure = true;

  sl::ERROR_CODE err_ = m_thread_data.zed.open(init_parameters);
  if (err_ != sl::ERROR_CODE::SUCCESS) {
    std::cerr << "ERROR: " << sl::toString(err_) << std::endl;
    m_thread_data.zed.close();
    assert(false);
    return false;
  }

  const auto &resolution = m_thread_data.zed.getCameraInformation().camera_configuration.resolution;
  m_zedWidth = static_cast<int>(resolution.width);
  m_zedHeight = static_cast<int>(resolution.height);

  // Generate image texture for left/right eye
  sl::uchar4 dark_bckgrd(44, 44, 44, 255);
  glGenTextures(2, m_zedTextureID);
  for (int eye = 0; eye < 2; eye++) {
    // Generate OpenGL texture
    glBindTexture(GL_TEXTURE_2D, m_zedTextureID[eye]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_zedWidth, m_zedHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set size and default value to texture
    m_thread_data.zed_image[eye].alloc(m_zedWidth, m_zedHeight, sl::MAT_TYPE::U8_C4, sl::MEM::GPU);
    m_thread_data.zed_image[eye].setTo(dark_bckgrd, sl::MEM::GPU);
  }

  // Generate depth texture for left/right eye
  glGenTextures(2, m_depthTextureID);
  for (int eye = 0; eye < 2; eye++) {
    glBindTexture(GL_TEXTURE_2D, m_depthTextureID[eye]);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_zedWidth, m_zedHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_zedWidth, m_zedHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

#ifdef DEBUG_DEPTH_MAP_ON_GPU
    std::cerr << "StereoCamera::initialize(): call alloc(" << eye << ", GPU)" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

    m_thread_data.zed_depth[eye].alloc(m_zedWidth, m_zedHeight, sl::MAT_TYPE::F32_C1, sl::MEM::GPU);
    m_thread_data.zed_depth[eye].setTo(1.f, sl::MEM::GPU);

#ifdef DEBUG_DEPTH_MAP_ON_GPU
    std::cerr << "StereoCamera::initialize(): finish alloc(" << eye << ", GPU)" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  }

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "StereoCamera::initialize(): call cublasCreate()" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  cublasCreate(&cb_handle);

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "StereoCamera::initialize(): finish cublasCreate()" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  // Register texture
  cudaError_t err = cudaGraphicsGLRegisterImage(&m_cimg_l, m_zedTextureID[ovrEye_Left], GL_TEXTURE_2D, cudaGraphicsRegisterFlagsWriteDiscard);
  cudaError_t err2 = cudaGraphicsGLRegisterImage(&m_cimg_r, m_zedTextureID[ovrEye_Right], GL_TEXTURE_2D, cudaGraphicsRegisterFlagsWriteDiscard);
  if (err != cudaSuccess) {
    std::cout << "ERROR: cannot create CUDA texture : left  : error_code=" << err << " : " << cudaGetErrorString(err) << std::endl;
  }
  if (err2 != cudaSuccess) {
    std::cout << "ERROR: cannot create CUDA texture : right : error_code=" << err2 << " : " << cudaGetErrorString(err2) << std::endl;
  }

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "StereoCamera::initialize(): call cudaGraphicsGLRegisterImage()" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cout << "##### 3 b : " << m_cdep_l << std::endl;
  std::cout << "##### 3 b : " << m_depthTextureID[ovrEye_Left] << std::endl;
#endif
  cudaError_t err3 = cudaGraphicsGLRegisterImage(&m_cdep_l, m_depthTextureID[ovrEye_Left], GL_TEXTURE_2D, cudaGraphicsRegisterFlagsWriteDiscard);
#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cout << "##### 3 a : " << m_cdep_l << std::endl;
  std::cout << "##### 3 a : " << cudaGetErrorString(err3) << std::endl;
  std::cout << "##### 3 a : " << err3 << std::endl;
#endif
  cudaError_t err4 = cudaGraphicsGLRegisterImage(&m_cdep_r, m_depthTextureID[ovrEye_Right], GL_TEXTURE_2D, cudaGraphicsRegisterFlagsWriteDiscard);
  if (err3 != cudaSuccess) {
    std::cout << "ERROR: cannot create CUDA texture for depth : left  : error_code=" << err3 << " : " << cudaGetErrorString(err3) << std::endl;
  }
  if (err4 != cudaSuccess) {
    std::cout << "ERROR: cannot create CUDA texture for depth : right : error_code=" << err4 << " : " << cudaGetErrorString(err4) << std::endl;
  }

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "StereoCamera::initialize(): finish cudaGraphicsGLRegisterImage()" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  cuCtxSetCurrent(m_thread_data.zed.getCUDAContext());

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "StereoCamera::initialize(): finished" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  return true;
}

void StereoCamera::finalize() {
  cublasDestroy(cb_handle);
}

bool StereoCamera::newFrameExists() const { return m_thread_data.new_frame; }

int StereoCamera::width() const { return m_zedWidth; }

int StereoCamera::height() const { return m_zedHeight; }

GLuint StereoCamera::textureID(int eye) const { return m_zedTextureID[eye]; }

GLuint StereoCamera::depthTextureID(int eye) const { return m_depthTextureID[eye]; }

void StereoCamera::startCapturing() {
  m_thread_data.run = true;
  m_thread_data.new_frame = true;
  m_thread_data.show_info = false;

  // Launch ZED grab thread
  m_runner = new std::thread(__zed_runner__, std::ref(m_thread_data));

  cudaGraphicsMapResources(1, &m_cimg_l, 0);
  cudaGraphicsMapResources(1, &m_cimg_r, 0);

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "StereoCamera::startCapturing(): call cudaGraphicsMapResources() for depth." << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  cudaGraphicsMapResources(1, &m_cdep_l, 0);
  cudaGraphicsMapResources(1, &m_cdep_r, 0);

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "StereoCamera::startCapturing(): finish cudaGraphicsMapResources() for depth." << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU
}

void StereoCamera::stopCapturing() {
#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "StereoCamera::stopCapturing(): call cudaGraphicsUnmapResources() for depth." << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  cudaGraphicsUnmapResources(1, &m_cdep_l);
  cudaGraphicsUnmapResources(1, &m_cdep_r);

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "StereoCamera::stopCapturing(): finish cudaGraphicsUnmapResources() for depth." << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  cudaGraphicsUnmapResources(1, &m_cimg_l);
  cudaGraphicsUnmapResources(1, &m_cimg_r);
  m_thread_data.run = false;

  if (m_runner != nullptr) {
    m_runner->join();
    delete m_runner;
    m_runner = nullptr;
  }
}

void StereoCamera::updateBuffer() {
  MAKE_AND_START_TIMER(A_2_1);
  bool showInfo = false;

  // Update the ZED frame counter
  MAKE_AND_START_TIMER(A_2_1_1);
#ifdef ENABLE_MTX_LOCK
  // m_thread_data.mtx.lock();
#endif
  // STOP_AND_RECORD_TIMER(A_2_1_1);
  STOP_AND_RECORD_TIMER_WITH_ID(A_2_1_1, "(A_2_1_1)  mtx.lock()");

  MAKE_AND_START_TIMER(A_2_1_2);
  m_thread_data.show_info = showInfo;
  cudaArray_t arrIm;
  cudaGraphicsSubResourceGetMappedArray(&arrIm, m_cimg_l, 0, 0);
  cudaMemcpy2DToArray(arrIm, 0, 0, m_thread_data.zed_image[ovrEye_Left].getPtr<sl::uchar1>(sl::MEM::GPU),
                      m_thread_data.zed_image[ovrEye_Left].getStepBytes(sl::MEM::GPU), m_thread_data.zed_image[ovrEye_Left].getWidth() * 4,
                      m_thread_data.zed_image[ovrEye_Left].getHeight(), cudaMemcpyDeviceToDevice);

  cudaGraphicsSubResourceGetMappedArray(&arrIm, m_cimg_r, 0, 0);
  cudaMemcpy2DToArray(arrIm, 0, 0, m_thread_data.zed_image[ovrEye_Right].getPtr<sl::uchar1>(sl::MEM::GPU),
                      m_thread_data.zed_image[ovrEye_Right].getStepBytes(sl::MEM::GPU), m_thread_data.zed_image[ovrEye_Left].getWidth() * 4,
                      m_thread_data.zed_image[ovrEye_Left].getHeight(), cudaMemcpyDeviceToDevice);
  // STOP_AND_RECORD_TIMER(A_2_1_2);
  STOP_AND_RECORD_TIMER_WITH_ID(A_2_1_2, "(A_2_1_2)  cuda");

  cudaError_t errCode;
  cublasStatus_t cublasErrCode;

  // normalization factor
  float depthAlpha = (StereoCamera::maxDepth() <= 1.0f) ? -1.0f : 1.0f / StereoCamera::maxDepth();

  // left depth
  size_t depthWidth = m_thread_data.zed_depth[ovrEye_Left].getWidth();
  size_t depthHeight = m_thread_data.zed_depth[ovrEye_Left].getHeight();

#ifdef CHECK_DEPTH_MAP_ON_GPU
  // i : 0:center / 1:lefttop / 2:neighbor of lefttop / 3:rightbottom
  // j : 0:CUDA / 1:ZED
  // k : 0:before / 1:after
  static bool matAlloced = false;
  static sl::Mat lMat;
  static sl::Mat rMat;
  float ldepth[4][2][2];
  float rdepth[4][2][2];
  size_t offset[4][2];
  offset[0][0] = depthWidth / 2;
  offset[0][1] = depthHeight / 2;
  offset[1][0] = 0;
  offset[1][1] = 0;
  offset[2][0] = 1;
  offset[2][1] = 0;
  offset[3][0] = depthWidth - 1;
  offset[3][1] = depthHeight - 1;
  if (!matAlloced) {
    lMat.alloc(depthWidth, depthHeight, sl::MAT_TYPE::F32_C1, sl::MEM::CPU);
  }
  lMat.setFrom(m_thread_data.zed_depth[ovrEye_Left], sl::COPY_TYPE::GPU_CPU);
  for (size_t ipoint = 0; ipoint < 4; ipoint++) {
    cudaMemcpy(&(ldepth[ipoint][0][0]),
               m_thread_data.zed_depth[ovrEye_Left].getPtr<sl::float1>(sl::MEM::GPU) + (offset[ipoint][1] * depthWidth + offset[ipoint][0]),
               sizeof(float), cudaMemcpyDeviceToHost);
    lMat.getValue(offset[ipoint][0], offset[ipoint][1], &(ldepth[ipoint][1][0]), sl::MEM::CPU);
  }
#endif // CHECK_DEPTH_MAP_ON_GPU

  if (depthAlpha >= 0.0f) {
    // normalize
    cublasErrCode =
        cublasSscal(cb_handle, depthWidth * depthHeight, &depthAlpha, m_thread_data.zed_depth[ovrEye_Left].getPtr<sl::float1>(sl::MEM::GPU), 1);
    if (cublasErrCode != CUBLAS_STATUS_SUCCESS) {
      std::cerr << "ERROR : cublasSscal(left) : errCode=" << cublasErrCode << std::endl;
    }
  }

#ifdef CHECK_DEPTH_MAP_ON_GPU
  lMat.setFrom(m_thread_data.zed_depth[ovrEye_Left], sl::COPY_TYPE::GPU_CPU);
  for (size_t ipoint = 0; ipoint < 4; ipoint++) {
    cudaMemcpy(&(ldepth[ipoint][0][1]),
               m_thread_data.zed_depth[ovrEye_Left].getPtr<sl::float1>(sl::MEM::GPU) + (offset[ipoint][1] * depthWidth + offset[ipoint][0]),
               sizeof(float), cudaMemcpyDeviceToHost);
    lMat.getValue(offset[ipoint][0], offset[ipoint][1], &(ldepth[ipoint][1][1]), sl::MEM::CPU);
  }

//    cudaMemcpy(&(ldepth[0][1]), m_thread_data.zed_depth[ovrEye_Left].getPtr<sl::float1>(sl::MEM::GPU) + depthWidth*depthHeight*4/2, sizeof(float),
//    cudaMemcpyDeviceToHost); m_thread_data.zed_depth[ovrEye_Left].getValue(depthWidth/2, depthHeight/2, &(ldepth[1][1]), sl::MEM::GPU);
#endif // CHECK_DEPTH_MAP_ON_GPU

  cudaArray_t arrDe;

  errCode = cudaGraphicsSubResourceGetMappedArray(&arrDe, m_cdep_l, 0, 0);
  if (errCode != cudaSuccess) {
    std::cerr << "ERROR : cudaGraphicsSubResourceGetMappedArray(left) : errCode=" << errCode << std::endl;
  }
#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cout << "##### $$$ a1 : " << arrDe << std::endl;
#endif

  errCode = cudaMemcpy2DToArray(arrDe, 0, 0, m_thread_data.zed_depth[ovrEye_Left].getPtr<sl::float1>(sl::MEM::GPU),
                                m_thread_data.zed_depth[ovrEye_Left].getStepBytes(sl::MEM::GPU), depthWidth * sizeof(float),
                                // depthWidth,
                                depthHeight, cudaMemcpyDeviceToDevice);
#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cout << "##### $$$ a2 "
            << ": " << m_thread_data.zed_depth[ovrEye_Left].getStepBytes(sl::MEM::GPU) << ": " << depthWidth << ": " << sizeof(float) << ": "
            << depthHeight << ": " << cudaMemcpyDeviceToDevice << ": " << m_zedWidth << ": " << m_zedHeight << std::endl;
  std::cout << "##### $$$ a3 : " << errCode << std::endl;
  std::cout << "##### $$$ a4 : " << cudaGetErrorString(errCode) << std::endl;
#endif
  if (errCode != cudaSuccess) {
    std::cerr << "ERROR : cudaMemcpy2DToArray(left) : errCode=" << errCode << std::endl;
  }

  // right depth
  depthWidth = m_thread_data.zed_depth[ovrEye_Right].getWidth();
  depthHeight = m_thread_data.zed_depth[ovrEye_Right].getHeight();

#ifdef CHECK_DEPTH_MAP_ON_GPU
  offset[0][0] = depthWidth / 2;
  offset[0][1] = depthHeight / 2;
  offset[1][0] = 0;
  offset[1][1] = 0;
  offset[2][0] = 1;
  offset[2][1] = 0;
  offset[3][0] = depthWidth - 1;
  offset[3][1] = depthHeight - 1;

  if (!matAlloced) {
    rMat.alloc(depthWidth, depthHeight, sl::MAT_TYPE::F32_C1, sl::MEM::CPU);
  }
  //    cudaMemcpy(&(rdepth[0][0]), m_thread_data.zed_depth[ovrEye_Right].getPtr<sl::float1>(sl::MEM::GPU) + depthWidth*depthHeight*4/2,
  //    sizeof(float), cudaMemcpyDeviceToHost); m_thread_data.zed_depth[ovrEye_Right].getValue(depthWidth/2, depthHeight/2, &(ldepth[1][0]),
  //    sl::MEM::GPU);
  rMat.setFrom(m_thread_data.zed_depth[ovrEye_Right], sl::COPY_TYPE::GPU_CPU);
  for (size_t ipoint = 0; ipoint < 4; ipoint++) {
    cudaMemcpy(&(rdepth[ipoint][0][0]),
               m_thread_data.zed_depth[ovrEye_Right].getPtr<sl::float1>(sl::MEM::GPU) + (offset[ipoint][1] * depthWidth + offset[ipoint][0]),
               sizeof(float), cudaMemcpyDeviceToHost);
    rMat.getValue(offset[ipoint][0], offset[ipoint][1], &(rdepth[ipoint][1][0]), sl::MEM::CPU);
  }
#endif // CHECK_DEPTH_MAP_ON_GPU

  if (depthAlpha >= 0.0f) {
    // normalize
    cublasErrCode =
        cublasSscal(cb_handle, depthWidth * depthHeight, &depthAlpha, m_thread_data.zed_depth[ovrEye_Right].getPtr<sl::float1>(sl::MEM::GPU), 1);
    if (cublasErrCode != CUBLAS_STATUS_SUCCESS) {
      std::cerr << "ERROR : cublasSscal(right) : errCode=" << cublasErrCode << std::endl;
    }
  }

#ifdef CHECK_DEPTH_MAP_ON_GPU
  rMat.setFrom(m_thread_data.zed_depth[ovrEye_Right], sl::COPY_TYPE::GPU_CPU);
  for (size_t ipoint = 0; ipoint < 4; ipoint++) {
    cudaMemcpy(&(rdepth[ipoint][0][1]),
               m_thread_data.zed_depth[ovrEye_Right].getPtr<sl::float1>(sl::MEM::GPU) + (offset[ipoint][1] * depthWidth + offset[ipoint][0]),
               sizeof(float), cudaMemcpyDeviceToHost);
    rMat.getValue(offset[ipoint][0], offset[ipoint][1], &(rdepth[ipoint][1][1]), sl::MEM::CPU);
  }
  matAlloced = true;

//    cudaMemcpy(&(rdepth[0][1]), m_thread_data.zed_depth[ovrEye_Right].getPtr<sl::float1>(sl::MEM::GPU) + depthWidth*depthHeight*4/2, sizeof(float),
//    cudaMemcpyDeviceToHost); m_thread_data.zed_depth[ovrEye_Right].getValue(depthWidth/2, depthHeight/2, &(ldepth[1][1]), sl::MEM::GPU);
#endif // CHECK_DEPTH_MAP_ON_GPU

  errCode = cudaGraphicsSubResourceGetMappedArray(&arrDe, m_cdep_r, 0, 0);
  if (errCode != cudaSuccess) {
    std::cerr << "ERROR : cudaGraphicsSubResourceGetMappedArray(right) : errCode=" << errCode << std::endl;
  }

  errCode = cudaMemcpy2DToArray(arrDe, 0, 0, m_thread_data.zed_depth[ovrEye_Right].getPtr<sl::float1>(sl::MEM::GPU),
                                m_thread_data.zed_depth[ovrEye_Right].getStepBytes(sl::MEM::GPU), depthWidth * sizeof(float),
                                // depthWidth,
                                depthHeight, cudaMemcpyDeviceToDevice);

  if (errCode != cudaSuccess) {
    std::cerr << "ERROR : cudaMemcpy2DToArray(right) : errCode=" << errCode << std::endl;
  }

#ifdef CHECK_DEPTH_MAP_ON_GPU
  std::cerr << "retrieveDepth on GPU : depthAlpha=" << depthAlpha << std::endl;
  std::cerr << "  center ---------------------------------" << std::endl;
  std::cerr << "    cuda : " << ldepth[0][0][0] << " -> " << ldepth[0][0][1] << " / " << rdepth[0][0][0] << " -> " << rdepth[0][0][1] << " / "
            << std::endl;
  std::cerr << "    zed  : " << ldepth[0][1][0] << " -> " << ldepth[0][1][1] << " / " << rdepth[0][1][0] << " -> " << rdepth[0][1][1] << " / "
            << std::endl;
  std::cerr << "  left-top ---------------------------------" << std::endl;
  std::cerr << "    cuda : " << ldepth[1][0][0] << " -> " << ldepth[1][0][1] << " / " << rdepth[1][0][0] << " -> " << rdepth[1][0][1] << " / "
            << std::endl;
  std::cerr << "    zed  : " << ldepth[1][1][0] << " -> " << ldepth[1][1][1] << " / " << rdepth[1][1][0] << " -> " << rdepth[1][1][1] << " / "
            << std::endl;
  std::cerr << "  neighbor ---------------------------------" << std::endl;
  std::cerr << "    cuda : " << ldepth[2][0][0] << " -> " << ldepth[2][0][1] << " / " << rdepth[2][0][0] << " -> " << rdepth[2][0][1] << " / "
            << std::endl;
  std::cerr << "    zed  : " << ldepth[2][1][0] << " -> " << ldepth[2][1][1] << " / " << rdepth[2][1][0] << " -> " << rdepth[2][1][1] << " / "
            << std::endl;
  std::cerr << "  right-bottom ---------------------------------" << std::endl;
  std::cerr << "    cuda : " << ldepth[3][0][0] << " -> " << ldepth[3][0][1] << " / " << rdepth[3][0][0] << " -> " << rdepth[3][0][1] << " / "
            << std::endl;
  std::cerr << "    zed  : " << ldepth[3][1][0] << " -> " << ldepth[3][1][1] << " / " << rdepth[3][1][0] << " -> " << rdepth[3][1][1] << " / "
            << std::endl;
#endif // CHECK_DEPTH_MAP_ON_GPU


  MAKE_AND_START_TIMER(A_2_1_5);
  // m_thread_data.mtx.unlock();
  // STOP_AND_RECORD_TIMER(A_2_1_5);
  STOP_AND_RECORD_TIMER_WITH_ID(A_2_1_5, "(A_2_1_5)  mtx.unlock()");

  m_thread_data.new_frame = false;
  STOP_AND_RECORD_TIMER(A_2_1);
}
