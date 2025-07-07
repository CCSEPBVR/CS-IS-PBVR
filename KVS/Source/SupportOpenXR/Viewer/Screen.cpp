/*****************************************************************************/
/**
 *  @file   Screen.cpp
 *  @author Keisuke Tajiri
 */
 /*****************************************************************************/
#include "Screen.h"
#include <kvs/DebugNew>
#include <kvs/OpenGL>
#include <kvs/RGBColor>
#include <kvs/Camera>
#include <kvs/Light>
#include <kvs/Mouse>
#include <kvs/MouseButton>
#include <kvs/Background>
#include <kvs/InitializeEvent>
#include <kvs/PaintEvent>
#include <kvs/ResizeEvent>
#include <kvs/MouseEvent>
#include <kvs/KeyEvent>
#include <kvs/WheelEvent>
#include <kvs/ControllerEvent>
#include <kvs/ObjectManager>
#include <kvs/RendererManager>
#include <kvs/IDManager>
#include <kvs/ObjectBase>
#include <kvs/RendererBase>
#include <kvs/VisualizationPipeline>
#include <kvs/TrackballInteractor>
#include <kvs/qt/Qt>
#include <kvs/qt/Application>


namespace kvs
{

namespace openxr
{

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the qt screen downcasted from the screen base.
 *  @param  screen [in] the screen base.
 *  @return pointer to the openxr screen
 */
 /*===========================================================================*/
Screen* Screen::DownCast( kvs::ScreenBase* screen )
{
    return dynamic_cast<Screen*>( screen );
}

/*===========================================================================*/
/**
 *  @brief  Returns the const pointer to the qt screen downcasted from the screen base.
 *  @param  screen [in] the screen base.
 *  @return const pointer to the openxr screen
 */
 /*===========================================================================*/
const Screen* Screen::DownCast( const kvs::ScreenBase* screen )
{
    return dynamic_cast<Screen*>( const_cast<kvs::ScreenBase*>( screen ) );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Screen class.
 */
 /*===========================================================================*/
Screen::Screen( kvs::qt::Application* application, QWidget* parent ) :
    kvs::qt::ScreenBase( application, parent )
{
    kvsMessageDebug( "kvs::openxr::Screen::Screen()" );

    QWidget::setFocusPolicy( Qt::StrongFocus );
    
    m_scene = new kvs::Scene(this);

    m_interactor = new kvs::TrackballInteractor();
    BaseClass::setEvent( m_interactor, "DefaultInteractor" );

    m_openxr_interactor = new kvs::openxr::OpenXRInteractor( this );
    m_openxr_interactor->setScreen( this );
    m_openxr_interactor->setScene( m_scene );

    m_openxr_device = new kvs::openxr::OpenXRDevice();

    m_controller_event = new kvs::ControllerEvent();
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Screen class.
 */
 /*===========================================================================*/
Screen::~Screen()
{
    kvsMessageDebug( "kvs::openxr::Screen::~Screen()" );

    if ( m_scene ) { delete m_scene; }
    if ( m_interactor ) { delete m_interactor; }
    if ( m_openxr_device ) { delete m_openxr_device; }
    if ( m_openxr_interactor ) { delete m_openxr_interactor; }
    if ( m_controller_event ) { delete m_controller_event; }

    for ( auto it = m_color_to_depth_map.begin(); it != m_color_to_depth_map.end(); ++it )
    {
        if ( it->second != 0 )
        { 
            KVS_GL_CALL( glDeleteTexturesEXT( 1, &it->second ) );
        }
    }

    for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
    {
        if ( m_eye_fbo[i] != 0 )
        { 
            KVS_GL_CALL( glDeleteFramebuffersEXT( 1, &m_eye_fbo[i] ) );
            m_eye_fbo[i] = 0;
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Sets a window title.
 *  @param  title [in] title
 */
 /*===========================================================================*/
void Screen::setTitle( const std::string& title )
{
    BaseClass::setTitle( title );
#if ( KVS_QT_VERSION >= 4 )
    QWidget::setWindowTitle( QString( title.c_str() ) );
#else
    QWidget::setCaption( QString( title.c_str() ) );
#endif
}

/*===========================================================================*/
/**
 *  @brief  Sets a screen position.
 *  @param  x [in] x coordinate value of the screen position
 *  @param  y [in] y coordinate value of the screen position
 */
 /*===========================================================================*/
void Screen::setPosition( const int x, const int y )
{
    BaseClass::setPosition( x, y );
    QWidget::move( x, y );
}

/*===========================================================================*/
/**
 *  @brief  Sets a screen size.
 *  @param  width [in] screen width
 *  @param  height [in] screen height
 */
 /*===========================================================================*/
void Screen::setSize( const int width, const int height )
{
    QWidget::resize( width, height );
}

/*===========================================================================*/
/**
 *  @brief  Sets a screen geometry (position and size).
 *  @param  x [in] x coordinate value of the screen position
 *  @param  y [in] y coordinate value of the screen position
 *  @param  width [in] screen width
 *  @param  height [in] screen height
 */
 /*===========================================================================*/
void Screen::setGeometry( const int x, const int y, const int width, const int height )
{
    this->setPosition( x, y );
    this->setSize( width, height );
}

/*===========================================================================*/
/**
 *  @brief  Sets a background color.
 *  @param  color [in] RGB color
 */
 /*===========================================================================*/
void Screen::setBackgroundColor( const kvs::RGBColor& color )
{
    m_scene->background()->setColor( color );
}

void Screen::setBackgroundColor( const kvs::RGBColor& color1, const kvs::RGBColor& color2 )
{
    m_scene->background()->setColor( color1, color2 );
}

void Screen::setBackgroundImage( const kvs::ColorImage& image )
{
    m_scene->background()->setImage( image );
}

/*===========================================================================*/
/**
 *  @brief  Sets a control target.
 *  @param  target [in] control target
 */
 /*===========================================================================*/
void Screen::setControlTarget( const ControlTarget target )
{
    m_scene->controlTarget() = target;
}

void Screen::setControlTargetToObject()
{
    this->setControlTarget( kvs::Scene::TargetObject );
}

void Screen::setControlTargetToCamera()
{
    this->setControlTarget( kvs::Scene::TargetCamera );
}

void Screen::setControlTargetToLight()
{
    this->setControlTarget( kvs::Scene::TargetLight );
}

/*===========================================================================*/
/**
 *  @brief  Registers an object with a renderer.
 *  @param  object [in] pointer to the object
 *  @param  renderer [in] pointer to the renderer
 *  @return Pair of IDs (object ID and renderer ID)
 */
 /*===========================================================================*/
const std::pair<int, int> Screen::registerObject( kvs::ObjectBase* object, kvs::RendererBase* renderer )
{
    return m_scene->registerObject( object, renderer );
}

/*===========================================================================*/
/**
 *  @brief  Registers an object that is processed through the visualization pipeline.
 *  @param  pipeline [in] pointer to the visualization pipeline
 *  @return Pair of IDs (object ID and renderer ID)
 */
 /*===========================================================================*/
const std::pair<int, int> Screen::registerObject( kvs::VisualizationPipeline* pipeline )
{
    // WARNING: It is necessary to increment the reference counter of the
    // pipeline.object() and the pipeline.renderer().
    auto* object = const_cast<kvs::ObjectBase*>( pipeline->object() );
    auto* renderer = const_cast<kvs::RendererBase*>( pipeline->renderer() );

    const int object_id = m_scene->objectManager()->insert( object );
    const int renderer_id = m_scene->rendererManager()->insert( renderer );
    m_scene->IDManager()->insert( object_id, renderer_id );

    return std::pair<int, int>( object_id, renderer_id );
}

/*===========================================================================*/
/**
 *  @brief  Sets an event.
 *  @param  event [in] pointer to an event
 *  @param  name [in] event name
 */
 /*===========================================================================*/
void Screen::setEvent( kvs::EventListener* event, const std::string& name )
{
    event->setScene( m_scene );
    BaseClass::setEvent( event, name );
}

/*===========================================================================*/
/**
 *  @brief  Adds an event to the event handler.
 *  @param  event [in] pointer to an event
 *  @param  name [in] event name
 */
 /*===========================================================================*/
void Screen::addEvent( kvs::EventListener* event, const std::string& name )
{
    event->setScene( m_scene );
    BaseClass::addEvent( event, name );
}

/*===========================================================================*/
/**
 *  @brief  Enables mouse operation.
 */
 /*===========================================================================*/
void Screen::enable()
{
    m_scene->enableObjectOperation();
}

/*===========================================================================*/
/**
 *  @brief  Disables mouse operation.
 */
 /*===========================================================================*/
void Screen::disable()
{
    m_scene->disableObjectOperation();
}

/*===========================================================================*/
/**
 *  @brief  Resets the x-form matrix of the basic screen components.
 */
 /*===========================================================================*/
void Screen::reset()
{
    m_scene->reset();
    m_walkthrough_xform = kvs::Xform();
}

/*===========================================================================*/
/**
 *  @brief  Initialization event.
 */
 /*===========================================================================*/
void Screen::initializeEvent()
{
    m_openxr_device->setAppName( BaseClass::title().c_str() );
    if ( !m_openxr_device->initialized() )
    {
        if ( m_openxr_device->initialize() )
        {
            kvs::Vec2i resolution = m_openxr_device->resolution(0);
            m_width = resolution[0];
            m_height = resolution[1];
            QSize widget_size = QWidget::size();
            this->resizeEvent( widget_size.width(), widget_size.height() );

            KVS_GL_CALL( glGenFramebuffersEXT( 1, &m_eye_fbo[0] ) );
            KVS_GL_CALL( glGenFramebuffersEXT( 1, &m_eye_fbo[1] ) );
        }
        else
        {
            KVS_ASSERT( false );
        }
    }

    kvs::InitializeEvent event;
    BaseClass::eventHandler()->notify( &event );
    if ( m_openxr_interactor ) m_openxr_interactor->onEvent( &event );
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
 /*===========================================================================*/
void Screen::paintEvent()
{
    GLuint default_fbo = kvs::OpenGL::Integer( GL_FRAMEBUFFER_BINDING );
    if ( m_scene ) m_scene->background()->apply();
    KVS_GL_CALL( glBindFramebufferEXT( GL_FRAMEBUFFER, 0 ) );

    if ( m_openxr_device )
    {
        bool openxr_result = m_openxr_device->update();
        if ( openxr_result && m_openxr_device->beginFrame() )
        {
            m_head_xform = m_openxr_device->headXform();

            m_controller_event->setControllerStatus( m_openxr_device->controllerStatus() );
            m_controller_event->setAction( kvs::Controller::Action::Pressed );
            this->controllerPressEvent( m_controller_event );

            m_controller_event->setAction( kvs::Controller::Action::Released );
            this->controllerReleaseEvent( m_controller_event );

            m_controller_event->setAction( kvs::Controller::Action::AxisChanged );
            this->controllerAxisEvent( m_controller_event );

            m_controller_event->setAction( kvs::Controller::Action::Moved );
            this->controllerMoveEvent( m_controller_event );

            for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
            {
                GLuint color_image = m_openxr_device->requireImage( i );
                GLuint depth_image = getDepthTexture( color_image );
                KVS_GL_CALL( glBindFramebufferEXT( GL_FRAMEBUFFER, m_eye_fbo[i] ) );
                KVS_GL_CALL( glFramebufferTexture2DEXT( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_image, 0 ) );
                KVS_GL_CALL( glFramebufferTexture2DEXT( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_image, 0 ) );
                kvs::FrameBufferObject::SetUnbindID( m_eye_fbo[i] );
                
                m_scene->camera()->setProjectionMatrix( m_openxr_device->projectionMatrix(i) );
                m_scene->camera()->setXform( m_walkthrough_xform * m_openxr_device->cameraXform(i) );
                m_scene->camera()->update();
                kvs::OpenGL::SetViewport( 0, 0, m_width, m_height );
                m_scene->background()->apply();

                {
                    kvs::OpenGL::WithPushedMatrix p( GL_MODELVIEW );
                    p.loadIdentity();

                    kvs::PaintEvent event;
                    BaseClass::eventHandler()->notify( &event );

                    if ( m_openxr_interactor != nullptr )
                    {
                        m_openxr_interactor->onEvent( &event );
                    }
                }

                KVS_GL_CALL( glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER, default_fbo ) );
                KVS_GL_CALL( glBindFramebufferEXT( GL_READ_FRAMEBUFFER, m_eye_fbo[i] ) );

                kvs::Vec2i fbo_size = kvs::Vec2i( m_width, m_height );
                QSize widget_size = QWidget::size();
                kvs::Vec2i screen_size = kvs::Vec2i( widget_size.width() / 2, widget_size.height() );
                float scale = std::fmin( static_cast<float>( screen_size[0] ) / static_cast<float>( fbo_size[0] ),
                    static_cast<float>( screen_size[1] ) / static_cast<float>( fbo_size[1] ) );
                kvs::Vec2i scaled_size = kvs::Vec2i(fbo_size[0] * scale, fbo_size[1] * scale);
                int x0 = i == 0 ? ( screen_size[0] - scaled_size[0] ) / 2 : screen_size[0] + ( screen_size[0] - scaled_size[0] ) / 2;
                int y0 = ( screen_size[1] - scaled_size[1] ) / 2;
                int x1 = x0 + scaled_size[0];
                int y1 = y0 + scaled_size[1];

                KVS_GL_CALL( glBlitFramebufferEXT( 0, 0, fbo_size[0], fbo_size[1], x0, y0, x1, y1, GL_COLOR_BUFFER_BIT, GL_LINEAR ) );
                KVS_GL_CALL( glBindFramebufferEXT( GL_FRAMEBUFFER, 0 ) );
            }
            m_openxr_device->endFrame();
            kvs::OpenGL::Flush();
        }
    }

#if 0
    m_fps_timer.stop();
    double elapsed = m_fps_timer.msec();
    double fps = (elapsed == 0.0) ? 1000 : (1.0 / (elapsed / 1000.0));
    kvsMessage("fps = %lf", fps);
    m_fps_timer.start();
#endif

}

/*===========================================================================*/
/**
 *  @brief  Resize event.
 *  @param  width [in] resized screen width
 *  @param  height [in] resized screen height
 */
 /*===========================================================================*/
void Screen::resizeEvent( int width, int height )
{
    this->setSize( width, height );
    
    kvs::ResizeEvent event( m_width, m_height );
    BaseClass::eventHandler()->notify( &event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event.
 *  @param  event [in] pointer to the mouse event information
 */
 /*===========================================================================*/
void Screen::mousePressEvent( kvs::MouseEvent* event )
{
    BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse move event.
 *  @param  event [in] pointer to the mouse event information
 */
 /*===========================================================================*/
void Screen::mouseMoveEvent( kvs::MouseEvent* event )
{
    BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse release event.
 *  @param  event [in] pointer to the mouse event information
 */
 /*===========================================================================*/
void Screen::mouseReleaseEvent( kvs::MouseEvent* event )
{
    BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Mouse double-click event.
 *  @param  event [in] pointer to the mouse event information
 */
 /*===========================================================================*/
void Screen::mouseDoubleClickEvent( kvs::MouseEvent* event )
{
    BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Wheel event.
 *  @param  event [in] pointer to the wheel event information
 */
 /*===========================================================================*/
void Screen::wheelEvent( kvs::WheelEvent* event )
{
    BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Key press event.
 *  @param  event [in] pointer to the key event information
 */
 /*===========================================================================*/
void Screen::keyPressEvent( kvs::KeyEvent* event )
{
    BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Key repeat event.
 *  @param  event [in] pointer to the key event information
 */
 /*===========================================================================*/
void Screen::keyRepeatEvent( kvs::KeyEvent* event )
{
    BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Key release event.
 *  @param  event [in] pointer to the key event information
 */
 /*===========================================================================*/
void Screen::keyReleaseEvent( kvs::KeyEvent* event )
{
    BaseClass::eventHandler()->notify( event );
}

/*===========================================================================*/
/**
 *  @brief  Controller Press event.
 *  @param  event [in] pointer to the controller event information
 */
 /*===========================================================================*/
void Screen::controllerPressEvent( kvs::ControllerEvent* event )
{
    BaseClass::eventHandler()->notify( event );
    if (m_openxr_interactor)
    {
        m_openxr_interactor->onEvent(event);
    }
}

/*===========================================================================*/
/**
 *  @brief  Controller Release event.
 *  @param  event [in] pointer to the controller event information
 */
 /*===========================================================================*/
void Screen::controllerReleaseEvent( kvs::ControllerEvent* event )
{
    BaseClass::eventHandler()->notify( event );
    if ( m_openxr_interactor )
    {
        m_openxr_interactor->onEvent( event );
    }
}

/*===========================================================================*/
/**
 *  @brief  Controller Move event.
 *  @param  event [in] pointer to the controller event information
 */
 /*===========================================================================*/
void Screen::controllerMoveEvent( kvs::ControllerEvent* event )
{
    BaseClass::eventHandler()->notify( event );
    if ( m_openxr_interactor )
    {
        m_openxr_interactor->onEvent( event );
    }
}

/*===========================================================================*/
/**
 *  @brief  Controller Axis event.
 *  @param  event [in] pointer to the controller event information
 */
 /*===========================================================================*/
void Screen::controllerAxisEvent( kvs::ControllerEvent* event )
{
    BaseClass::eventHandler()->notify( event );
    if ( m_openxr_interactor )
    {
        m_openxr_interactor->onEvent( event );
    }
}

/*===========================================================================*/
/**
 *  @brief
 */
 /*===========================================================================*/
GLuint Screen::getDepthTexture( GLuint color_texture )
{
    auto it = m_color_to_depth_map.find( color_texture );
    if ( it != m_color_to_depth_map.end() ) { return it->second; }

    GLint width = 0;
    GLint height = 0;
    KVS_GL_CALL( glBindTexture( GL_TEXTURE_2D, color_texture ) );
    KVS_GL_CALL( glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width ) );
    KVS_GL_CALL( glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height ) );

    GLuint depth_texture = 0;
    KVS_GL_CALL( glGenTextures( 1, &depth_texture ) );
    KVS_GL_CALL( glBindTexture( GL_TEXTURE_2D, depth_texture) );
    KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
    KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
    KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
    KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) );
    KVS_GL_CALL( glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr ) );
    KVS_GL_CALL( glBindTexture( GL_TEXTURE_2D, 0 ) );
    m_color_to_depth_map.insert( std::make_pair( color_texture, depth_texture ) );

    return depth_texture;
}

} // end of namespace openxr

} // end of namespace kvs
