#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "screen.h"
#include <QElapsedTimer>
#include <QMutex>
#include <kvs/Camera>
#include <kvs/FrameBufferObject>
#include <kvs/OpenGL>

#include <kvs/IDManager>
#include <kvs/ObjectBase>
#include <kvs/ObjectManager>
#include <kvs/RendererBase>
#include <kvs/RendererManager>
#include <kvs/Background>

#include "Client/PBRProxy.h"

extern QMutex paint_mutex;

/**
 * @brief Screen::Screen
 * @param parent_surface, parent Qt Widget
 */
Screen::Screen(QWidget *parent_surface) {
  Q_UNUSED(parent_surface);
  m_scene = new kvs::jaea::pbvr::Scene(this);
  Screen::setFocusPolicy(Qt::StrongFocus);
  ScreenBase::create();
  m_scene->camera()->setWindowSize(340, 340);
  m_scene->mouse()->attachCamera(m_scene->camera());

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
#ifdef VR_MODE
  m_bounding_box = new BoundingBox(this->m_scene);
  BoundingBoxSynchronizer::instance().addClient(this);
  m_isEnabledBoundingBox = INITIAL_STATE_BOUNDING_BOX;
#endif
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX

  update();
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
    qWarning("Screen could not get paint_mutex lock, skipping this frame");
    update(); // Schedule new update
    return;
  }
  // Check and clear GL errors
  while (e = glGetError()) {
    qCritical("Screen::paintGL GL HAS ERROR BEFORE %d", e);
  }
  // This allows KVS to work with QOpenGLWidgets
  // Requires future release of KVS which includes commit
  // https://github.com/naohisas/KVS/commit/4e3a36473cc0b7cc1418e49b949e5f8d62d8c489
  kvs::FrameBufferObject::m_unbind_id = QOpenGLWidget::defaultFramebufferObject();
  QElapsedTimer timer;

#if defined(ENABLE_FEATURE_DRAW_PARTICLE) && defined(ENABLE_FEATURE_DRAW_BOUNDING_BOX)
  // update bounding box
  const size_t size = m_scene->IDManager()->size();
  for (size_t i = 0; i < size; i++) {
    kvs::IDManager::IDPair id = m_scene->IDManager()->id(i);
    kvs::RendererBase *renderer = m_scene->rendererManager()->renderer(id.second);
    if (typeid(*renderer) == typeid(kvs::visclient::PBRProxy)) {
        if (this->m_isEnabledBoundingBox) {
            // direction & upvector in camera coordinate system.
            kvs::Vec4 initial_direction = kvs::Vec4(-(m_scene->camera()->position()), 0);
            kvs::Vec4 initial_upvector = kvs::Vec4(m_scene->camera()->upVector(), 0);
#ifdef VR_MODE
            this->m_bounding_box->setDirection(initial_direction);
            this->m_bounding_box->setUpvector(initial_upvector);
            this->m_bounding_box->show();
#endif
        } else {
#ifdef VR_MODE
            this->m_bounding_box->hide();
#endif
        }
#ifdef VR_MODE
        this->m_bounding_box->frameEvent();
#endif
        break;
    }
  }
#endif

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  kvs::OpenGL::WithPushedMatrix p(GL_MODELVIEW);
  p.loadIdentity();
  {
    timer.start();
#ifdef CPU_MODE
    m_scene->paintFunction();
#endif
#ifdef GPU_MODE
    m_compositor->setBackgroundColor(scene()->background()->color());
    m_compositor->update();
#endif
    qint64 elapsed = timer.elapsed();
    double fps = (elapsed == 0) ? 1000 : (1.0 / (((double)elapsed) / 1000.0));
    m_fps = fps;

    //m_fps = 1.0 / ((double)timer.elapsed() / 1000.0);
  }
  glPopAttrib();
  onPaintGL();

  // Check and clear GL errors
  while (e = glGetError()) {
    qCritical("Screen::paintGL GL GOT GL ERROR %d", e);
  }
  paint_mutex.unlock();
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
  onResizeGL(width, height);
}

/**
 * @brief Screen::initializeGL automatically called as part of Qt Event chain.
 *                             Do not call directly. Handles initialization of OpenGL context (initializeOpenGLFunctions)
 *
 *                             After context has been initialized, virtual onInitializeGL is called to allow
 *                             a subclass to take additional actions
 */
void Screen::initializeGL() {
  std::cout << "Screen::initializeGL" << std::endl;
  if (!isValid()) {
    qWarning("Screen::initalizeGL while surface still not valid ");
    return;
  }
#ifdef KVS_ENABLE_GLEW
  // If KVS is compiled with glew support, it is important that we init glew before
  // we initalize OpenGLFunctions.
  GLenum result = glewInit();
  if (result != GLEW_OK) {
    const GLubyte *message = glewGetErrorString(result);
    qFatal("GLEW initialization failed. ");
  }
#endif
  initializeOpenGLFunctions();
  onInitializeGL();
#ifdef GPU_MODE
  m_compositor = new kvs::jaea::pbvr::StochasticRenderingCompositor(m_scene);
  //    m_compositor->setRepetitionLevel( 16 );
  std::cout << __LINE__ << m_compositor->repetitionLevel() << std::endl;
  // m_compositor->enableLODControl();
  m_compositor->disableLODControl();
  BaseClass::eventHandler()->attach(m_compositor);
  m_compositor->initializeEvent();
  this->m_enable_default_paint_event = false;

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
#ifdef VR_MODE
  m_bounding_box->initializeEvent();
#endif
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX

#endif

  m_gl_initialized = true;
}

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
void Screen::updateBoundingBoxVisibility(bool isVisible) {
  m_isEnabledBoundingBox = isVisible;
  update();
}
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX

Screen::~Screen() { delete (m_scene); }
