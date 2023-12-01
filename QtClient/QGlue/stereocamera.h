#ifndef STEREOCAMERA_H
#define STEREOCAMERA_H

#ifdef MR_MODE

#include "oculus.h"

#pragma warning(push)
#pragma warning(disable : 4251)
#pragma warning(disable : 4819)
#include <sl/Camera.hpp>
#pragma warning(pop)

#include <kvs/Timer>

#include <cublas_v2.h>

struct cudaGraphicsResource;

// Packed Zed data for threaded computation
struct ZEDThreadData {
  sl::Camera zed;
  sl::Mat zed_image[2];
  sl::Mat zed_depth[2];
  std::mutex mtx;
  bool run;
  bool new_frame;
  bool show_info;
};

class StereoCamera {
private:
  cudaGraphicsResource *m_cimg_l = nullptr;
  cudaGraphicsResource *m_cimg_r = nullptr;
  cudaGraphicsResource *m_cdep_l = nullptr;
  cudaGraphicsResource *m_cdep_r = nullptr;

  cublasHandle_t cb_handle;

  ZEDThreadData m_thread_data;
  std::thread *m_runner = nullptr;
  GLuint m_zedTextureID[2] = {0, 0};
  GLuint m_depthTextureID[2] = {0, 0};
  int m_zedWidth = -1;
  int m_zedHeight = -1;

  // these static variables require to define again in .cpp files.
  static sl::UNIT m_units; // For depth measure. default 0.2m - 15m
  static float m_min_depth;
  static float m_max_depth;

#ifdef ENABLE_TIME_MEASURE
  kvs::Timer m_timer_updateBuffer[5];
#endif // ENABLE_TIME_MEASURE

public:
  StereoCamera();
  ~StereoCamera(){};

  bool initialize();
  void finalize();
  bool newFrameExists() const;
  int width() const;
  int height() const;

  static sl::UNIT units() { return m_units; }
  static float maxDepth() { return m_max_depth; }
  static float minDepth() { return m_min_depth; }

  GLuint textureID(int eye) const;
  GLuint depthTextureID(int eye) const;

  void updateBuffer();
  void startCapturing();
  void stopCapturing();

  void mutex_lock() { m_thread_data.mtx.lock(); }
  void mutex_unlock() { m_thread_data.mtx.unlock(); }
};

#endif // MR_MODE

#endif // STEREOCAMERA_H
