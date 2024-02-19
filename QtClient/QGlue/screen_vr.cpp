#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "screen_vr.h"

#include <QElapsedTimer>
#include <QMutex>
#include <QTimer>
#include <kvs/Camera>
#include <kvs/FrameBufferObject>
#include <kvs/ObjectBase>
#include <kvs/OpenGL>

#include <kvs/Background>
#include <kvs/FrameEvent>
#include <kvs/IDManager>
#include <kvs/ObjectManager>
#include <kvs/PaintEvent>
#include <kvs/RendererBase>
#include <kvs/RendererManager>
#include <kvs/StochasticRenderingCompositor>

#include "objnameutil.h"
#include "ovr.h"
#include "TimeMeasureUtility.h"
#include "touchcontroller.h"
#include "objectscale.h"


#ifdef DISP_MODE_VR
typedef unsigned char BYTE;
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void Screen::enableGrabFramebuffer(){
  m_is_enabled_grab_framebuffer = true;
}
QImage Screen::getGrabbedFramebuffer(){
  return m_grabbed_image;
}

QImage Screen::grabImage(int width, int height){
  int nChannel = 4;
  BYTE *pixels = (BYTE*)malloc(sizeof(BYTE) * width * height * nChannel);

  // Readback the contents from the Framebuffer
  GLenum mode= GL_COLOR_ATTACHMENT0;
  glReadBuffer(mode);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  QImage image = QImage(pixels,width, height,QImage::Format_RGBA8888);
  //flip upsidedown
  image.mirror(false, true);

  free(pixels);

  return image;
}
#endif

kvs::Int64 frame_index = 0;

namespace {
inline kvs::Vec3 ToVec3(const OVR::Vector3f &v) { return kvs::Vec3(v.x, v.y, v.z); }

inline OVR::Vector3f ToVec3(const kvs::Vec3 &v) { return OVR::Vector3f(v.x(), v.y(), v.z()); }

inline kvs::Mat4 ToMat4(const OVR::Matrix4f &m) {
  return kvs::Mat4(m.M[0][0], m.M[0][1], m.M[0][2], m.M[0][3], m.M[1][0], m.M[1][1], m.M[1][2], m.M[1][3], m.M[2][0], m.M[2][1], m.M[2][2], m.M[2][3],
                   m.M[3][0], m.M[3][1], m.M[3][2], m.M[3][3]);
}

inline OVR::Matrix4f ToMat4(const kvs::Mat3 &m) {
  return OVR::Matrix4f(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]);
}

inline OVR::Matrix4f ToMat4(const kvs::Mat4 &m) {
  return OVR::Matrix4f(m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3], m[2][0], m[2][1], m[2][2], m[2][3], m[3][0], m[3][1],
                       m[3][2], m[3][3]);
}
} // namespace

extern QMutex paint_mutex;

const kvs::Vec3 Screen::KVS_CAMERA_INITIAL_POSITION = kvs::Vec3(0.0f, 0.0f, 1.2f);
const kvs::Vec3 Screen::KVS_CAMERA_INITIAL_LOOK_AT = kvs::Vec3(0.0f, 0.0f, 0.0f);
const kvs::Vec3 Screen::KVS_CAMERA_INITIAL_UP = kvs::Vec3(0.0f, 1.0f, 0.0f);

/**
 * @brief Screen::Screen
 * @param parent_surface, parent Qt Widget
 */
Screen::Screen(QWidget *parent_surface) :
      m_hmd(), m_widget_handler(nullptr),
      m_compositor(nullptr),
      m_isEnabledBoundingBox(INITIAL_STATE_BOUNDING_BOX),
      m_is_enabled_grab_framebuffer(false)
    {
  Q_UNUSED(parent_surface);

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "Screen::Screen(): start" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  // m_scene = new kvs::Scene(this);
  m_scene = new kvs::jaea::pbvr::Scene(this);
  m_scene->camera()->setPosition(KVS_CAMERA_INITIAL_POSITION, KVS_CAMERA_INITIAL_LOOK_AT, KVS_CAMERA_INITIAL_UP);

  Screen::setFocusPolicy(Qt::StrongFocus);
  ScreenBase::create();
  m_scene->camera()->setWindowSize(340, 340);
  m_scene->mouse()->attachCamera(m_scene->camera());
  update();

  static bool flag = true;
  if (flag) {
    flag = false;
    if (!m_hmd.create()) {
      kvsMessageError("Failed HMD create");
    }

    // from OculusKVS (Screen)
    m_touch_controller = new TouchController(this->m_hmd, this->m_scene);
#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
    m_bounding_box = new BoundingBox(this->m_scene);
    BoundingBoxSynchronizer::instance().addClient(this);
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX

    // add widget handler
    this->m_widget_handler = PBVRWidgetHandlerInstance();

    m_hmd.setWidgetHandler(this->m_widget_handler);
    m_touch_controller->setWidgetHandler(this->m_widget_handler);

    const int desktop_width = 340;
    const int desktop_height = 340;
    const int hmd_width = m_hmd.resolution().w;
    const int hmd_height = m_hmd.resolution().h;
    const float ratio = static_cast<float>(desktop_width) / hmd_width;
    const int screen_width = (ratio >= 1.0f) ? hmd_width : desktop_width;
    const int screen_height = (ratio >= 1.0f) ? hmd_height : static_cast<int>(hmd_height * ratio);
    setSize(screen_width, screen_height);
  }

  m_screenCoord.up = -1.f;
  m_screenCoord.down = 1.f;
  m_screenCoord.right = 1.f;
  m_screenCoord.left = -1.f;

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "Screen::Screen(): finish" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

#ifdef ENABLE_TIME_MEASURE
  m_lengthTimerArray_paintGL = 100;
  // m_timerCount_paintGL = 0;
  m_array_time_paintGL = (double *)malloc(sizeof(double) * m_lengthTimerArray_paintGL);
#endif // ENABLE_TIME_MEASURE
}

/**
 * @brief Screen::resizeGL automatically called as part of Qt Event chain.
 *                         Do not call directly. Handles scene and  camera resizing based on devicePixelRatio.
 *
 *                         After resizing scene and camera, virtual onResizeGL is called to allow subclass to
 *                         take additional action if needed
 * @param width in pixels
 * @param height in pixels
 */
void Screen::resizeGL(int width, int height) {
  assert(glGetError() == GL_NO_ERROR);
  std::cout << "Screen::resizeGL" << std::endl;
  // This allows KVS to work with QOpenGLWidgets
  // Requires future release of KVS which includes commit
  // https://github.com/naohisas/KVS/commit/4e3a36473cc0b7cc1418e49b949e5f8d62d8c489
  kvs::FrameBufferObject::m_unbind_id = QOpenGLWidget::defaultFramebufferObject();

  const int dpr = QOpenGLWidget::devicePixelRatio();
  size_t dpr_width = static_cast<size_t>(width / dpr + 0.5);
  size_t dpr_height = static_cast<size_t>(height / dpr + 0.5);
  m_scene->resizeFunction(dpr_width, dpr_height);
  m_scene->camera()->setWindowSize(dpr_width, dpr_height);

  // HMD
  const int desktop_width = width;
  const int desktop_height = height;

  const int hmd_width = m_hmd.resolution().w;
  const int hmd_height = m_hmd.resolution().h;

  const float ratio = static_cast<float>(desktop_width) / hmd_width;
  const int screen_width = (ratio >= 1.0f) ? hmd_width : desktop_width;
  const int screen_height = (ratio >= 1.0f) ? hmd_height : static_cast<int>(hmd_height * ratio);
  setSize(screen_width, screen_height);

  onResizeGL(width, height);

  checkOpenGLError("Screen::resizeGL() : finish onResizeGL() :");

  assert(glGetError() == GL_NO_ERROR);
}

/**
 * @brief Screen::paintGL, paint GL method - automatically called as part of Qt Event chain.
 *                         Do not call directly.
 *
 *                         Attains paint_mutex lock, checks opengl status and paints the scene.
 *
 *                         After painting the scene, the virtual method onPaintGL is called to allow
 *                         any subclass to paint additional items on top of scene.
 *
 *                         Finally glGetError is called until any and all errors are cleared.
 */
void Screen::paintGL() {
  // There is always an OpenGL error when paintGL() was invoked in The first time after app.exec().
  // We ignore the error for the time being, because we can't specify where the error occurred.
  checkOpenGLError ("Screen::paintGL() : start");

  QElapsedTimer timer;

  SET_INDEX_FOR_TIMER(frame_index);

  // from OculusKVS (Screen)
  // static kvs::Int64 frame_index = 0;
  static ovrVector3f initial_ep[2];
  static ovrVector3f initial_focus[2];

  if (QThread::currentThread() != this->thread()) {
    qWarning("Screen::paintGL was not called from main thread, skipping frame");
    return;
  }
  if (!Screen::m_gl_initialized || m_hold_paintGL) {
    qWarning("Screen either not initialized or on hold, skipping this frame");
    return;
  }
  int e;
  // Try to get paint_mutex lock.
  // This is probably not needed any more
  if (!paint_mutex.try_lock()) {
    qWarning("Screen::paintGL() : Screen could not get paint_mutex lock, skipping this frame");
    update(); // Schedule new update
    return;
  }

  // This allows KVS to work with QOpenGLWidgets
  // Requires future release of KVS which includes commit
  // https://github.com/naohisas/KVS/commit/4e3a36473cc0b7cc1418e49b949e5f8d62d8c489
  kvs::FrameBufferObject::m_unbind_id = QOpenGLWidget::defaultFramebufferObject();

  ovrSessionStatus sessionStatus;
  ovr_GetSessionStatus(m_hmd.handler(), &sessionStatus);

  if (frame_index % 100 == 0) {
    ovrErrorInfo error;
    ovr_GetLastErrorInfo(&error);
    if (!OVR_SUCCESS(error.Result)) {
      std::string message(error.ErrorString);
      std::cerr << "ERROR/Oculus : after ovr_GetSessionStatus() : " << message << std::endl;
    }
  }

  if (sessionStatus.ShouldQuit) {
    // Because the application is requested to quit, should not request retry
    // TODO : implement shutdown event.
  }
  if (sessionStatus.ShouldRecenter) {
    ovr_RecenterTrackingOrigin(m_hmd.handler());
  }
  if (sessionStatus.IsVisible) {
    MAKE_AND_START_TIMER(A);
    MAKE_AND_START_TIMER(A_1);

#ifdef ENABLE_MTX_LOCK
    m_zed->mutex_lock();
#endif

    const double frame_timing = m_hmd.frameTiming(frame_index);
    const ovrTrackingState &tracking_state = m_hmd.trackingState(frame_timing);
    // test1
    // m_hmd.beginFrame(frame_index);

    STOP_AND_RECORD_TIMER(A_1);

#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
    if (m_zed->newFrameExists()) {
      MAKE_AND_START_TIMER(A_2);
      m_zed->updateBuffer();
      STOP_AND_RECORD_TIMER(A_2);
    }
#endif // ENABLE_FEATURE_DRAW_CAMERA_IMAGE

    MAKE_AND_START_TIMER(A_3);
    {
      ovrErrorInfo info;
      ovr_GetLastErrorInfo(&info);
      assert(OVR_SUCCESS(info.Result));
    }

    const kvs::Vec4 current_viewport = kvs::OpenGL::Viewport();
    STOP_AND_RECORD_TIMER(A_3);
    MAKE_AND_START_TIMER(A_4);

    const ovrQuatf headOrientation = tracking_state.HeadPose.ThePose.Orientation;
    const OVR::Vector3f headPosition = tracking_state.HeadPose.ThePose.Position;

#if defined(ENABLE_FEATURE_DRAW_PARTICLE)
    // run touch-controller frameEvent only when particle exists
    const size_t size = m_scene->IDManager()->size();
    for (size_t i = 0; i < size; i++) {
      kvs::IDManager::IDPair id = m_scene->IDManager()->id(i);
      kvs::RendererBase *renderer = m_scene->rendererManager()->renderer(id.second);
      std::string renderer_classname = std::string(typeid(*renderer).name());
      if (renderer_classname == "class kvs::visclient::PBRProxy") {
        this->m_touch_controller->frameEvent();
#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX

        if (m_isEnabledBoundingBox != m_widget_handler->isEnabledBoundingBox()) {
            std::cerr << "Screen::paintGL() : m_isEnabledBoundingBox = " << m_isEnabledBoundingBox << " -> " << m_widget_handler->isEnabledBoundingBox() << std::endl;
        }

            m_isEnabledBoundingBox = m_widget_handler->isEnabledBoundingBox();
        if (this->m_isEnabledBoundingBox) {
          // direction & upvector in camera coordinate system.
          kvs::Vec4 initial_direction = kvs::Vec4(-(m_scene->camera()->position()), 0);
          kvs::Vec4 initial_upvector = kvs::Vec4(m_scene->camera()->upVector(), 0);

          // direction & upvector in Oculus coordinate system.
          const kvs::Mat4 headRotation = ToMat4(OVR::Matrix4f(headOrientation));
          kvs::Vec4 direction_global = headRotation * initial_direction;
          kvs::Vec4 upvector_global = headRotation * initial_upvector;

          this->m_bounding_box->setDirection(direction_global);
          this->m_bounding_box->setUpvector(upvector_global);
          this->m_bounding_box->show();
        } else {
          this->m_bounding_box->hide();
        }
        this->m_bounding_box->frameEvent();
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX
        break;
      }
    }
#else
    this->m_touch_controller->frameEvent();
#endif // defined(ENABLE_FEATURE_DRAW_PARTICLE)

    STOP_AND_RECORD_TIMER(A_4);
    MAKE_AND_START_TIMER(A_5);

    MAKE_AND_START_TIMER(A_5_1);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    STOP_AND_RECORD_TIMER(A_5_1);
    MAKE_AND_START_TIMER(A_5_2);
    kvs::OpenGL::WithPushedMatrix p(GL_MODELVIEW);
    p.loadIdentity();
    STOP_AND_RECORD_TIMER(A_5_2);

    MAKE_AND_START_TIMER(A_5_3);
    // original timing
    m_hmd.beginFrame(frame_index);
    STOP_AND_RECORD_TIMER_WITH_ID(A_5_3, "(A_5_3)  m_hmd.beginFrame()");
    MAKE_AND_START_TIMER(A_5_4);
    timer.start();
    STOP_AND_RECORD_TIMER(A_5_4);

    MAKE_AND_START_TIMER(A_5_5);
    scene()->updateGLLightParameters();
    STOP_AND_RECORD_TIMER(A_5_5);

    MAKE_AND_START_TIMER(A_5_6);
    // set background colors
    const kvs::RGBColor& bgcolor = kvs::RGBColor(0,0,0);
    this->m_compositor->setBackgroundColor(bgcolor);
    OculusTextureBuffer *renderTexture = m_hmd.eyeRenderTexture(0);
    renderTexture->BackgroundColor(bgcolor);

    // bind eye render target
    renderTexture->SetAndClearRenderSurface();
    KVS_GL_CALL(glEnable(GL_DEPTH_TEST));
    KVS_GL_CALL(glDepthFunc(GL_LESS));
    STOP_AND_RECORD_TIMER(A_5_6);

    MAKE_AND_START_TIMER(A_5_7);
    // 20210129 yodo append to force to locate KVS camera at fixed position.
    scene()->camera()->setPosition(KVS_CAMERA_INITIAL_POSITION);
    STOP_AND_RECORD_TIMER(A_5_7);

    MAKE_AND_START_TIMER(A_5_8);
    // preserve camera status
    const kvs::Vec3 camera_position = scene()->camera()->position();
    const kvs::Vec3 camera_lookat = scene()->camera()->lookAt();
    const kvs::Vec3 camera_upvector = scene()->camera()->upVector();
    const OVR::Vector3f position0 = ToVec3(camera_position);
    const OVR::Vector3f lookat0 = ToVec3(camera_lookat);
    const OVR::Vector3f upvector0 = ToVec3(camera_upvector);
    const OVR::Vector3f focus0 = ToVec3(camera_lookat - camera_position);

    STOP_AND_RECORD_TIMER(A_5_8);

    STOP_AND_RECORD_TIMER(A_5);
    MAKE_AND_START_TIMER(A_6);
    for (size_t eye = 0; eye < ovrEye_Count; eye++) {
      SET_EYE_FOR_TIMER(eye);
      MAKE_AND_START_TIMER(A_6_1or2);
      MAKE_AND_START_TIMER(A_6_1or2_1);

      const ovrPosef &eye_pose = m_hmd.eyePose(eye);
      const ovrRecti &viewport = m_hmd.viewport(eye);

      {
        ovrErrorInfo info;
        ovr_GetLastErrorInfo(&info);
        assert(OVR_SUCCESS(info.Result));
      }

      // 20210107 K.yodo append
      // same as Oculus SDK sample code
      scene()->camera()->setWindowSize(viewport.Size.w, viewport.Size.h);

      // Set projection matrix
      OVR::Matrix4f projectionMatrix = ovrMatrix4f_Projection(m_hmd.defaultEyeFov(eye), OCULUS_NEAR * OBJECT_SCALING_FACTOR, OCULUS_FAR * OBJECT_SCALING_FACTOR, ovrProjection_None);
      float m[16];
      kvs::Xform p(ToMat4(projectionMatrix));
      p.toArray(m);
      kvs::OpenGL::SetMatrixMode(GL_PROJECTION);
      kvs::OpenGL::LoadIdentity();
      kvs::OpenGL::MultMatrix(m);

      // Set viewing matrix.
      if (frame_index == 0) {
        initial_ep[eye] = eye_pose.Position;
        initial_focus[eye].x = focus0.x;
        initial_focus[eye].y = focus0.y;
        initial_focus[eye].z = focus0.z;
      } else if (frame_index > 0) {
        float transFactor = 1e0f;
        OVR::Vector3f position;
        position.x = eye_pose.Position.x - initial_ep[eye].x + position0.x;
        position.y = eye_pose.Position.y - initial_ep[eye].y + position0.y;
        position.z = eye_pose.Position.z - initial_ep[eye].z + position0.z;

        OVR::Matrix4f R = OVR::Matrix4f(eye_pose.Orientation);
        const OVR::Vector3f upvector = R.Transform(upvector0);
        const OVR::Vector3f forward = R.Transform(initial_focus[eye]);
        const OVR::Vector3f lookat = position + forward * transFactor;

        kvs::Vec3 position_v3 = ::ToVec3(position);
        kvs::Vec3 lookat_v3 = ::ToVec3(lookat);
        kvs::Vec3 upvector_v3 = ::ToVec3(upvector);
        scene()->camera()->setPosition(position_v3, lookat_v3, upvector_v3);
      }
      scene()->updateGLViewingMatrix();

      // Set viewport for each PBVRStochasticRendering
      int bufferWidth = m_hmd.eyeRenderTexture(0)->getTexureWidth();
      int bufferHeight = m_hmd.eyeRenderTexture(0)->getTexureHeight();
      glViewport(0, 0, bufferWidth / 2, bufferHeight);

      STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_1, std::string("(A_6_").append(std::to_string(eye + 1)).append("_1)  "));
      MAKE_AND_START_TIMER(A_6_1or2_2);

      // Setup the modeling matrix and render the objects.
      if (m_enable_default_paint_event) {
        this->defaultPaintEvent();
      } else {
        m_compositor->setRenderTarget(eye);
        m_compositor->update();
      }

      STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2, std::string("(A_6_").append(std::to_string(eye + 1)).append("_2)  "));
      STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2, std::string("(A_6_").append(std::to_string(eye + 1)).append(")  "));
      SET_EYE_FOR_TIMER(-1);
    } // for(eye)

    STOP_AND_RECORD_TIMER(A_6);
    MAKE_AND_START_TIMER(A_7);

    // recover camera position
    scene()->camera()->setPosition(camera_position, camera_lookat, camera_upvector);

    // unbind eye render target
    m_hmd.eyeRenderTexture(0)->UnsetRenderSurface();
    m_hmd.eyeRenderTexture(0)->Commit();

    if (m_widget_handler != nullptr) {
      m_hmd.updateHUD();
    }
    double elapsedTime = (double)timer.elapsed() / 1000.0;
    m_fps = 1.0 / (elapsedTime);

    // test2
    // m_hmd.beginFrame(frame_index);
    m_hmd.endFrame(frame_index);

#ifdef MR_MODE
#ifdef ENABLE_MTX_LOCK
    m_zed->mutex_unlock();
#endif
#endif // MR_MODE

    // Render to the frame buffer.
    kvs::OpenGL::SetViewport(current_viewport);
    m_hmd.setBackgroundColor(bgcolor);
    m_hmd.renderToMirror();
#ifdef DISP_MODE_VR
    if(m_is_enabled_grab_framebuffer){
      int width = current_viewport[2];
      int height = current_viewport[3];
      m_grabbed_image = grabImage(width, height);
      m_is_enabled_grab_framebuffer=false;
    }
#endif
    kvs::OpenGL::Flush();
    redraw();
    glPopAttrib();
    frame_index++;

    STOP_AND_RECORD_TIMER(A_7);
    STOP_AND_RECORD_TIMER(A);
    SHOW_TIMER_RESULT();
#ifdef ENABLE_TIME_MEASURE

    {
      m_array_time_paintGL[frame_index % m_lengthTimerArray_paintGL] = elapsedTime * 1000.0; // msec
      // m_timerCount_paintGL++;

      if (frame_index % m_lengthTimerArray_paintGL == 0) {
        std::cerr << "[TIME_MEASURE][" << (frame_index / m_lengthTimerArray_paintGL) << "] screen paintGL all ";
        for (int iTimer = 0; iTimer < m_lengthTimerArray_paintGL; iTimer++) {
          std::cerr << " " << m_array_time_paintGL[iTimer];
        }
        std::cerr << std::endl;
      }
      //            double elapsedTimeAverage = 0;
      //            if(m_timerCount_paintGL < m_lengthTimerArray_paintGL){
      //                for(int iTime=0;iTime<m_timerCount_paintGL;iTime++){
      //                    elapsedTimeAverage += m_array_time_paintGL[iTime];
      //                }
      //                elapsedTimeAverage /= m_timerCount_paintGL;
      //            } else {
      //                for(int iTime=0;iTime<m_lengthTimerArray_paintGL;iTime++){
      //                    elapsedTimeAverage += m_array_time_paintGL[iTime];
      //                }
      //                elapsedTimeAverage /= m_lengthTimerArray_paintGL;
      //            }

      //            std::cout << "ElapsedTimeAverage : Screen::paintGL() : " << elapsedTimeAverage << std::endl;
    }
#endif // ENABLE_TIME_MEASURE

  } // if (sessionStatus.IsVisible)

  {
    ovrErrorInfo info;
    ovr_GetLastErrorInfo(&info);
    assert(OVR_SUCCESS(info.Result));
  }

  paint_mutex.unlock();
  assert(glGetError() == GL_NO_ERROR);
}

/**
 * @brief Screen::initializeGL automatically called as part of Qt Event chain.
 *                             Do not call directly. Handles initialization of OpenGL context (initializeOpenGLFunctions)
 *
 *                             After context has been initialized, virtual onInitializeGL is called to allow
 *                             a subclass to take additional actions
 */
void Screen::initializeGL() {
#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "Screen::initializeGL(): start" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  if (!isValid()) {
    qWarning("Screen::initalizeGL while surface still not valid ");
    return;
  }

#if defined(KVS_ENABLE_GLEW)
  GLenum result = glewInit();
  if (result != GLEW_OK) {
    const GLubyte *message = glewGetErrorString(result);
    qFatal("GLEW initialization failed. ");
  }
#endif // defined( KVS_ENABLE_GLEW )
  assert(glGetError() == GL_NO_ERROR);

  // from OculusKVS (Application)
  initializeOpenGLFunctions();
  onInitializeGL();

  checkOpenGLError("Screen::initializeGL() : finish onInitializeGL :");

  // Configure tracking.
  const kvs::UInt32 trac_caps = ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position;
  if (!m_hmd.configureTracking(trac_caps, 0)) {
    kvsMessageError("Failed tracking configuration");
  }

  checkOpenGLError("Screen::initializeGL() : finish HMD::configureTracking() :");

  // Configure rendering.
  if (!m_hmd.configureRendering()) {
    kvsMessageError("Failed rendering configuration.");
  }

  checkOpenGLError("Screen::initializeGL() : finish HMD::configureRendering() :");

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "Screen::initializeGL(): call StereoCamera::StereoCaemraEx()" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

#ifdef MR_MODE
  // ZED mini initiaization : Eye render buffers are initialized.
  m_zed = new StereoCamera();

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "Screen::initializeGL(): finish StereoCamera::StereoCaemraEx()" << std::endl;
  std::cerr << "Screen::initializeGL(): call StereoCamera::initialize()" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

  m_zed->initialize();

  checkOpenGLError("Screen::initializeGL() : finish StreoCamera::initialize() :");

#ifdef DEBUG_DEPTH_MAP_ON_GPU
  std::cerr << "Screen::initializeGL(): finish StereoCamera::initialize()" << std::endl;
#endif // DEBUG_DEPTH_MAP_ON_GPU

#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
  m_zed->startCapturing();

  checkOpenGLError("Screen::initializeGL() : finish StereoCamera::startCapture() :");

#endif // ENABLE_FEATURE_DRAW_CAMERA_IMAGE
#endif // MR_MODE

  int bufferWidth = m_hmd.eyeRenderTexture(0)->getTexureWidth();
  int bufferHeight = m_hmd.eyeRenderTexture(0)->getTexureHeight();
#ifdef MR_MODE
  m_scene->camera()->setWindowSize(bufferWidth / 2, bufferHeight);

  checkOpenGLError("Screen::initializeGL() : finish Camera::setWindwSize() :");

#endif // MR_MODE

  // 20210111 added by K.Yodo.
  // BaseClass::eventHandler()->attach( m_touch_controller );
  m_touch_controller->initializeEvent();

  checkOpenGLError("Screen::initializeGL() : finish TouchController::initializeEvent() :");

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
  m_bounding_box->initializeEvent();

  checkOpenGLError("Screen::initializeGL() : finish BoundingBox::initializeEvent() :");

#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX

  std::cerr << "Screen::initializeEvent() : TouchController::initializeEvent() finished." << std::endl;

  // set compositor
  if (m_compositor == nullptr) {
    m_compositor = new kvs::jaea::pbvr::StochasticRenderingCompositor(m_scene);
    m_compositor->setBackgroundColor(0.0f, 0.0f, 0.0f, 1.0f);
#ifdef MR_MODE
    // set default depth to render particles only in range of camera image
    m_compositor->setClearDepth(0.0f);
#endif // MR_MODE
  }
  m_compositor->setRenderBufferSize(bufferWidth, bufferHeight);
  m_compositor->setRepetitionLevel(5);
  m_compositor->disableLODControl();
  BaseClass::eventHandler()->attach(m_compositor);
  m_compositor->initializeEvent();
  this->m_enable_default_paint_event = false;

  checkOpenGLError("Screen::initializeGL() : finish Compositor initialize :");

  if (m_widget_handler != nullptr) {
    // ImGui initialize functions must be called after initialize glut.
    m_hmd.setWidgetHandler(m_widget_handler);
  }

#ifdef MR_MODE
  ovrFovPort defaultEyeFov = m_hmd.defaultEyeFov(0);
  // Compute the useful part of the ZED image
  int zedWidth = m_zed->width();
  int zedHeight = m_zed->height();

  float heightGL = 1.f;
  float widthGL = 1.f;

  unsigned int widthFinal = bufferWidth / 2;
  if (zedWidth > 0.f) {
    unsigned int heightFinal = (unsigned int)(zedHeight * widthFinal / (float)zedWidth);
    // Convert this size to OpenGL viewport's frame's coordinates
    heightGL = (heightFinal) / (float)(bufferHeight);
    widthGL = 1.f; // = ((zedWidth * (heightFinal / (float) zedHeight)) / (float) widthFinal);
  } else {
    std::cout << "WARNING: ZED parameters got wrong values."
                 "Default vertical and horizontal FOV are used.\n"
                 "Check your calibration file or check if your ZED is not too close to a surface or an object."
              << std::endl;
  }

  // Compute the Horizontal Oculus' field of view with its parameters
  float ovrFovH = (atanf(defaultEyeFov.LeftTan) + atanf(defaultEyeFov.RightTan));
  // Compute the Vertical Oculus' field of view with its parameters
  float ovrFovV = (atanf(defaultEyeFov.UpTan) + atanf(defaultEyeFov.DownTan));

  // Compute the center of the optical lenses of the headset
  float offsetLensCenterX = ((atanf(defaultEyeFov.LeftTan)) / ovrFovH) * 2.f - 1.f;
  float offsetLensCenterY = ((atanf(defaultEyeFov.UpTan)) / ovrFovV) * 2.f - 1.f;

  // Set a rectangle with the computed coordinates
  m_screenCoord.up = heightGL + offsetLensCenterY;
  m_screenCoord.down = heightGL - offsetLensCenterY;
  m_screenCoord.right = widthGL + offsetLensCenterX;
  m_screenCoord.left = widthGL - offsetLensCenterX;

#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
  // Set up CameraImageRenderer
  if (!(m_cameraImageRenderer.isStereoCameraInitialized())) {
    m_cameraImageRenderer.setStereoCamera(m_zed);
    m_cameraImageRenderer.initializeStereoCamera(m_screenCoord.up, m_screenCoord.right, m_screenCoord.down, m_screenCoord.left);
  }
  m_compositor->setCameraImageRenderer(&m_cameraImageRenderer);

  checkOpenGLError("Screen::initializeGL() : finish CameraImageRenderer initialize :");

#endif // ENABLE_FEATURE_DRAW_CAMERA_IMAGE
#endif // MR_MODE

  m_gl_initialized = true;
  assert(glGetError() == GL_NO_ERROR);
}

Screen::~Screen() {
  // There is always an OpenGL error.
  // We ignore the error for now, because we can't find where the error occurred.
  GLenum err;
  while ((err = glGetError()) != 0) {
    std::cerr << "Detect OpenGL error in Screen::~Screen() and ignore it." << std::endl;
    std::cerr << "    OpenGL error:" << gluErrorString(err) << std::endl;
  }
  assert(glGetError() == GL_NO_ERROR);

#ifdef MR_MODE
#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
  m_zed->stopCapturing();
#endif // ENABLE_FEATURE_DRAW_CAMERA_IMAGE

  m_zed->finalize();
  delete m_zed;
#endif // MR_MODE

  // Detach object and render from scene so that the scene's destructor does not cause a memory leak.
  const size_t size = m_scene->IDManager()->size();
  for (size_t i = 0; i < size; i++) {
    kvs::IDManager::IDPair id = m_scene->IDManager()->id(i);
    m_scene->removeObject(id.first, false, false);
  }

  delete (m_scene);
  // from OculusKVS (Screen)
  m_hmd.destroy();

#ifdef ENAVLE_TIME_MEASURE
  free(m_array_time_paintGL);
#endif

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
  BoundingBoxSynchronizer::instance().removeClient(this);
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX
}

// ------------------------------------------------------------
// from OculusKVS (Screen)

void Screen::setWidgetHandler(PBVRWidgetHandler *handler) { m_widget_handler = handler; }

void Screen::defaultPaintEvent() {
  if (scene()->objectManager()->hasObject()) {
    const int size = scene()->IDManager()->size();
    for (int index = 0; index < size; index++) {
      kvs::IDManager::IDPair id = scene()->IDManager()->id(index);
      kvs::ObjectBase *object = scene()->objectManager()->object(id.first);
      kvs::RendererBase *renderer = scene()->rendererManager()->renderer(id.second);
      if (object->isShown()) {
        kvs::OpenGL::PushMatrix();
        if (!ObjectNameUtil::isToIgnoreModelingMatrix(object->name())) {
          scene()->updateGLModelingMatrix(object);
        }
        renderer->exec(object, scene()->camera(), scene()->light());
        kvs::OpenGL::PopMatrix();
      }
    }
  } else {
    scene()->updateGLModelingMatrix();
  }
}

void Screen::checkOpenGLError (const std::string& prefix) {
  GLenum err;
  while ((err = glGetError()) != 0) {
    std::cerr << "Detect OpenGL error in " << prefix << " and ignore it." << std::endl;
    std::cerr << "    OpenGL error: crd=" << err << std::endl;
    std::cerr << "    OpenGL error: msg=" << gluErrorString(err) << std::endl;
  }
  assert(glGetError() == GL_NO_ERROR);

}
