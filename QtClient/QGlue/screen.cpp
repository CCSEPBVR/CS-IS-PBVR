#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif
#include "screen.h"
#include <kvs/FrameBufferObject>
#include <kvs/OpenGL>
#include <kvs/Camera>
#include <QElapsedTimer>
#include <QMutex>

#include <kvs/ObjectBase>
#include <kvs/ObjectManager>
#include <kvs/IDManager>
#include <kvs/RendererManager>
#include <kvs/RendererBase>

extern QMutex paint_mutex;
/**
 * @brief Screen::Screen
 * @param parent_surface, parent Qt Widget
 */
Screen::Screen( QWidget* parent_surface)
{
    Q_UNUSED(parent_surface);
    m_scene = new kvs::Scene(this);
    Screen::setFocusPolicy(Qt::StrongFocus );
    ScreenBase::create();
    m_scene->camera()->setWindowSize(340,340);
    m_scene->mouse()->attachCamera( m_scene->camera());
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
void Screen::paintGL()
{
    if (QThread::currentThread() != this->thread()) {
        qWarning("Screen::paintGL was not called from main thread, skipping frame");
        return;
    }
    if (!Screen::m_gl_initialized || m_hold_paintGL){
        qWarning("Screen either not initialized or on hold, skipping this frame");
        return;
    }
    int e;
    // Try to get paint_mutex lock.
    // This is probably not needed any more
    if (!paint_mutex.try_lock()){
        qWarning("Screen could not get paint_mutex lock, skipping this frame");
        update(); // Schedule new update
        return;
    }
    // Check and clear GL errors
    while (e = glGetError()){
        qCritical("Screen::paintGL GL HAS ERROR BEFORE %d",e);
    }
    // This allows KVS to work with QOpenGLWidgets
    // Requires future release of KVS which includes commit
    // https://github.com/naohisas/KVS/commit/4e3a36473cc0b7cc1418e49b949e5f8d62d8c489
    kvs::FrameBufferObject::m_unbind_id=QOpenGLWidget::defaultFramebufferObject();
    QElapsedTimer timer;

    glPushAttrib( GL_ALL_ATTRIB_BITS );
    kvs::OpenGL::WithPushedMatrix p( GL_MODELVIEW );
    p.loadIdentity();
    {
        timer.start();
#ifdef CPU_MODE
        m_scene->paintFunction();
#endif
#ifdef GPU_MODE
        m_compositor->update();
#endif
        m_fps = 1.0 / ((double)timer.elapsed()/1000.0);
    }
    glPopAttrib();
    onPaintGL();

    // Check and clear GL errors
    while (e = glGetError()){
        qCritical("Screen::paintGL GL GOT GL ERROR %d",e);
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
void Screen::resizeGL(int width, int height)
{
    std::cout<< "Screen::resizeGL"<<std::endl;
    // This allows KVS to work with QOpenGLWidgets
    // Requires future release of KVS which includes commit
    // https://github.com/naohisas/KVS/commit/4e3a36473cc0b7cc1418e49b949e5f8d62d8c489
    kvs::FrameBufferObject::m_unbind_id=QOpenGLWidget::defaultFramebufferObject();
    const int dpr = QOpenGLWidget::devicePixelRatio();
    size_t dpr_width = static_cast<size_t>( width / dpr + 0.5 );
    size_t dpr_height = static_cast<size_t>( height / dpr + 0.5 );
    m_scene->resizeFunction( dpr_width, dpr_height );
    m_scene->camera()->setWindowSize(dpr_width,dpr_height);
    onResizeGL(width,height);
}

/**
 * @brief Screen::initializeGL automatically called as part of Qt Event chain.
 *                             Do not call directly. Handles initialization of OpenGL context (initializeOpenGLFunctions)
 *
 *                             After context has been initialized, virtual onInitializeGL is called to allow
 *                             a subclass to take additional actions
 */
void Screen::initializeGL()
{
    std::cout<< "Screen::initializeGL"<<std::endl;
    if (! isValid())
    {
        qWarning("Screen::initalizeGL while surface still not valid ");
        return;
    }
#ifdef KVS_ENABLE_GLEW
    // If KVS is compiled with glew support, it is important that we init glew before
    // we initalize OpenGLFunctions.
    GLenum result = glewInit();
    if ( result != GLEW_OK )
    {
        const GLubyte* message = glewGetErrorString( result );
        qFatal("GLEW initialization failed. ");
    }
#endif
    initializeOpenGLFunctions();
    onInitializeGL();
#ifdef GPU_MODE
    m_compositor = new kvs::StochasticRenderingCompositor( m_scene );
//    m_compositor->setRepetitionLevel( 16 );
    std::cout << __LINE__ <<  m_compositor->repetitionLevel() << std::endl;
    //m_compositor->enableLODControl();
    m_compositor->disableLODControl();
    BaseClass::eventHandler()->attach( m_compositor );
    m_compositor->initializeEvent();
    this->m_enable_default_paint_event = false;
#endif

    m_gl_initialized=true;
}

Screen::~Screen()
{
    delete (m_scene);
}



