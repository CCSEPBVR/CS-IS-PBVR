/*****************************************************************************/
/**
 *  @file   Screen.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#include "Screen.h"
#include <kvs/DebugNew>
#include <kvs/opencabin/OpenCABIN>
#include <kvs/opencabin/Camera>
#include <kvs/opencabin/Application>
#include <kvs/opencabin/Configuration>
#include <kvs/opencabin/MainLoop>
#include <kvs/TCPBarrier>
#if defined( KVS_SUPPORT_GLEW )
#include <kvs/glew/GLEW>
#endif


namespace { kvs::opencabin::Screen* context = 0; }

/*===========================================================================*/
/**
 *  @brief  Initialization function for renderer program (OpenCABIN predefined function).
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 *  @return pointer to the user defined data
 */
/*===========================================================================*/
void* rinit( int argc, char** argv )
{
    kvs::opencabin::Application::SetAsRenderer();

    static kvs::opencabin::MainLoop main_loop( argc, argv );
    if ( kvs::opencabin::Application::HasTrackpad() ) ::context->initializeTrackpad();
    for ( ; ; ) if ( kvs::opencabin::Application::IsDone() ) break;

    return( 0 );
}

/*===========================================================================*/
/**
 *  @brief  Initialization function for renderer program (OpenCABIN predefined function).
 *  @param  pdata [in] pointer to the user defined data returned from rinit
 */
/*===========================================================================*/
void ginit( void* pdata )
{
#if defined( KVS_SUPPORT_GLEW )
    if ( kvs::opencabin::Application::IsRenderer() )
    {
        if ( !kvs::glew::Initialize() )
        {
            kvsMessageError("GLEW initialization failed.");
        }
    }
#endif// KVS_SUPPORT_GLEW

    if ( ::context ) ::context->initializeEvent();
}

/*===========================================================================*/
/**
 *  @brief  Draw function for renderer program (OpenCABIN predefined function).
 *  @param  pdata [in] pointer to the user defined data returned from rinit
 */
/*===========================================================================*/
void gdraw( void* pdata )
{
    if ( ::context ) ::context->paintEvent();
}

/*===========================================================================*/
/**
 *  @brief  Idle function for renderer program (OpenCABIN predefined function).
 *  @param  pdata [in] pointer to the user defined data returned from rinit
 */
/*===========================================================================*/
void gidle( void* pdata )
{
    if ( kvs::opencabin::Application::HasTrackpad() ) ::context->updateTrackpad();
    if ( ::context ) ::context->idleEvent();

    oclPostRedisplay();
}

/*===========================================================================*/
/**
 *  @brief  Cleanup function for renderer program (OpenCABIN predefined function).
 *  @param  pdata [in] pointer to the user defined data returned from rinit
 */
/*===========================================================================*/
void rcleanup( void* pdata )
{
    // Do nothing.
}

namespace kvs
{

namespace opencabin
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new screen class.
 *  @param  application [in] pointer to the application class
 */
/*===========================================================================*/
Screen::Screen( kvs::opencabin::Application* application ):
    kvs::ScreenBase()
{
    if ( application ) application->attach( this );

    // Create a camera for OpenCABIN instead of the default camera.
    if ( BaseClass::m_camera ) delete BaseClass::m_camera;
    BaseClass::m_camera = new kvs::opencabin::Camera();

    // Set a camera position.
    const int sensor_index = 0;
    const kvs::Vector3f sensor_position( 0.0f, 0.0f, 0.0f );
    const kvs::Vector3f sensor_offset( kvs::opencabin::Configuration::Sensor::Offset( sensor_index ) );
    BaseClass::camera()->setPosition( sensor_position - sensor_offset );

    // Set a window size.
    const kvs::Vector2i size = kvs::opencabin::Configuration::KVSApplication::DisplayResolution();
    BaseClass::camera()->setWindowSize( size.x(), size.y() );

    // Set a light position.
    /* NOTICE: Although the default light position in KVS is (0, 0, 12) in
     * the world coordinate system, the same scene cannot be displayed by
     * using the same light settings. Therefore, the default light position
     * in the OpenCABIN space is appropriately set as (0, 0, 20).
     * Generally, the light position is transformed by the modelview matrix
     * when glLight is called and it's stored in the camera coordinate
     * system. However, in OpenCABIN, only the model matrix is stored in
     * the modelview matrix stack in the OpenGL state machine. Therefore,
     * it is difficult to specify the light position in the OpenCABIN space
     * by using kvs::Light::setPosition.
     */
    const kvs::Vector3f position( 0.0f, 0.0f, 20.0f );
    BaseClass::light()->setPosition( position );

    // Set window position and size for each renderer.
    if ( kvs::opencabin::Application::IsRenderer() )
    {
        const std::string name = this->name();
        const kvs::Vector2i position = kvs::opencabin::Configuration::Renderer::WindowPosition( name );
        const kvs::Vector2i size = kvs::opencabin::Configuration::Renderer::WindowSize( name );
        this->setPosition( position.x(), position.y() );
        this->setSize( size.x(), size.y() );
    }

    m_paint_event_func = &Screen::default_paint_event;
    m_initialize_event_handler = new kvs::EventHandler();
    m_idle_event_handler = new kvs::EventHandler();

    ::context = this;
}

/*===========================================================================*/
/**
 *  @brief  Destructs the screen class.
 */
/*===========================================================================*/
Screen::~Screen( void )
{
    if ( m_initialize_event_handler ) { delete m_initialize_event_handler; }
    if ( m_idle_event_handler ) { delete m_idle_event_handler; }
}

/*===========================================================================*/
/**
 *  @brief  Returns screen name (renderer name).
 *  @return screen name
 */
/*===========================================================================*/
std::string Screen::name( void ) const
{
    return( kvs::opencabin::Configuration::Renderer::Name() );
}

/*===========================================================================*/
/**
 *  @brief  Shows the screen.
 *  @return window ID
 */
/*===========================================================================*/
int Screen::show( void )
{
    return( 0 );
}

/*===========================================================================*/
/**
 *  @brief  Redraws the window.
 */
/*===========================================================================*/
void Screen::redraw( void )
{
    oclPostRedisplay();
}

/*===========================================================================*/
/**
 *  @brief  Initialization event.
 */
/*===========================================================================*/
void Screen::initializeEvent( void )
{
    if ( kvs::opencabin::Application::IsRenderer() )
    {
        BaseClass::initializeFunction();
        m_initialize_event_handler->notify();
    }
}

/*===========================================================================*/
/**
 *  @brief  Idle event.
 */
/*===========================================================================*/
void Screen::idleEvent( void )
{
    if ( kvs::opencabin::Application::IsRenderer() )
    {
        m_idle_event_handler->notify();
    }
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
void Screen::paintEvent( void )
{
    if ( kvs::opencabin::Application::IsRenderer() )
    {
        if ( m_paint_event_func ) (this->*m_paint_event_func)();
        else BaseClass::eventHandler()->notify();

        if ( kvs::opencabin::Application::IsEnabledBarrier() )
        {
            static kvs::SocketAddress master = Configuration::KVSApplication::MasterAddress();
            kvs::TCPBarrier barrier( master.ip(), master.port() );
            barrier.wait();
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Set paint event.
 *  @param  event [in] pointer to the paint event
 */
/*===========================================================================*/
void Screen::setPaintEvent( kvs::PaintEventListener* event )
{
    m_paint_event_func = NULL;
    this->addPaintEvent( event );
}

/*===========================================================================*/
/**
 *  @brief  Add initialization event.
 *  @param  event [in] pointer to the initialization event
 */
/*===========================================================================*/
void Screen::addInitializeEvent( kvs::InitializeEventListener* event )
{
    event->setScreen( this );
    m_initialize_event_handler->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Add idle event.
 *  @param  event [in] pointer to the idle event
 */
/*===========================================================================*/
void Screen::addIdleEvent( kvs::IdleEventListener* event )
{
    event->setScreen( this );
    m_idle_event_handler->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Add paint event.
 *  @param  event [in] pointer to the paint event
 */
/*===========================================================================*/
void Screen::addPaintEvent( kvs::PaintEventListener* event )
{
    event->setScreen( this );
    BaseClass::eventHandler()->attach( event );
}

/*===========================================================================*/
/**
 *  @brief  Default paint event.
 */
/*===========================================================================*/
void Screen::default_paint_event( void )
{
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glPushMatrix();

    BaseClass::paintFunction();
    BaseClass::eventHandler()->notify();

    glPopMatrix();

    glFlush();
    //glutSwapBuffers();
}

/*===========================================================================*/
/**
 *  @brief  Set screen position.
 *  @param  x [in] x coordinate
 *  @param  y [in] y coordinate
 */
/*===========================================================================*/
void Screen::setPosition( const int x, const int y )
{
    BaseClass::setPosition( x, y );
}

/*===========================================================================*/
/**
 *  @brief  Set screen size.
 *  @param  width [in] screen width
 *  @param  height [in] screen height
 */
/*===========================================================================*/
void Screen::setSize( const int width, const int height )
{
    BaseClass::setSize( width, height );
}

/*===========================================================================*/
/**
 *  @brief  Set screen geometry.
 *  @param  x [in] x coordinate
 *  @param  y [in] y coordinate
 *  @param  width [in] screen width
 *  @param  height [in] screen height
 */
/*===========================================================================*/
void Screen::setGeometry( const int x, const int y, const int width, const int height )
{
    BaseClass::setGeometry( x, y, width, height );
}

void Screen::initializeTrackpad( void )
{
    if ( kvs::opencabin::Application::HasTrackpad() )
    {
        kvs::opencabin::Application::InitializeTrackpadForRenderer();
    }
}

void Screen::updateTrackpad( void )
{
    if ( kvs::opencabin::Application::HasTrackpad() )
    {
        kvs::opencabin::Application::ReferenceTrackpad();

        float* scaling = kvs::opencabin::Application::GetTrackpadScaling();
        float* translation = kvs::opencabin::Application::GetTrackpadTranslation();
        float* rotation = kvs::opencabin::Application::GetTrackpadRotation();
        m_mouse->setScaling( kvs::Vector3f( scaling ) );
        m_mouse->setTranslation( kvs::Vector3f( translation ) );
        m_mouse->setRotation( kvs::Quaternion<float>( rotation ) );

        BaseClass::updateXform();
    }
}

} // end of namespace opencabin

} // end of namespace kvs
