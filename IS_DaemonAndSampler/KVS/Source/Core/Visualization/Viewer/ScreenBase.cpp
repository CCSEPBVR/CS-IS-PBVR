/****************************************************************************/
/**
 *  @file ScreenBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ScreenBase.cpp 666 2011-02-02 11:40:58Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ScreenBase.h"
#include <kvs/DebugNew>
#include <kvs/PointRenderer>
#include <kvs/LineRenderer>
#include <kvs/PolygonRenderer>
#include <kvs/RayCastingRenderer>
#include <kvs/ImageRenderer>
#include <kvs/EventHandler>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ScreenBase class.
 */
/*===========================================================================*/
ScreenBase::ScreenBase( void ):
    m_title("<unknown>"),
    m_x( 0 ),
    m_y( 0 ),
    m_width( 512 ),
    m_height( 512 ),
    m_id( 0 ),
    m_is_fullscreen( false ),
    m_target( ScreenBase::TargetObject ),
    m_enable_move_all( true ),
    m_enable_collision_detection( false )
{
    this->create();
}

/*===========================================================================*/
/**
 *  @brief  Destructs the ScreenBase class.
 */
/*===========================================================================*/
ScreenBase::~ScreenBase( void )
{
    this->clear();
}

/*===========================================================================*/
/**
 *  @brief  Registers an object with a renderer.
 *  @param  object [in] pointer to the object
 *  @param  renderer [in] pointer to the renderer
 *  @return Pair of IDs (object ID and renderer ID)
 */
/*===========================================================================*/
const std::pair<int,int> ScreenBase::registerObject( kvs::ObjectBase* object, kvs::RendererBase* renderer )
{
    /* If the given pointer to the renderer is null, a renderer for the given
     * object is automatically created by using visualization pipeline class.
     */
    if ( !renderer )
    {
        kvs::VisualizationPipeline pipeline( object );
        if ( !pipeline.exec() )
        {
            kvsMessageError("Cannot create a renderer for the given object.");
            return( std::pair<int,int>( -1, -1 ) );
        }

        renderer = const_cast<kvs::RendererBase*>( pipeline.renderer() );
    }

    if ( !object->hasMinMaxObjectCoords() )
    {
        object->updateMinMaxCoords();
    }

    /* If the object has not been registered in the object managet,
     * the object is registered and then its ID is returned.
     */
    int object_id = m_object_manager->objectID( object );
    if ( object_id == -1 ) object_id = m_object_manager->insert( object );

    /* If the renderer has not been registered in the renderer managet,
     * the renderer is registered and then its ID is returned.
     */
//    int renderer_id = m_renderer_manager->rendererID( renderer );
//    if ( renderer_id == -1 ) renderer_id = m_renderer_manager->insert( renderer );
    int renderer_id = -1;
    for ( int i = 0; i < m_renderer_manager->nrenderers(); i++ )
    {
        if ( m_renderer_manager->renderer(i) == renderer ) renderer_id = i;
    }
    if ( renderer_id == -1 ) renderer_id = m_renderer_manager->insert( renderer );

    // Insert the IDs into the ID manager.
    m_id_manager->insert( object_id, renderer_id );

    return( std::pair<int,int>( object_id, renderer_id ) );
}

/*===========================================================================*/
/**
 *  @brief  Registers an object that is processed through the visualization pipeline.
 *  @param  pipeline [in] pointer to the visualization pipeline
 *  @return Pair of IDs (object ID and renderer ID)
 */
/*===========================================================================*/
const std::pair<int,int> ScreenBase::registerObject( kvs::VisualizationPipeline* pipeline )
{
    /* WARNING: It is necessary to increment the reference counter of the
     * pipeline.object() and the pipeline.renderer().
     */
    kvs::ObjectBase* object = const_cast<kvs::ObjectBase*>( pipeline->object() );
    kvs::RendererBase* renderer = const_cast<kvs::RendererBase*>( pipeline->renderer() );

    const int object_id = m_object_manager->insert( object );
    const int renderer_id = m_renderer_manager->insert( renderer );
    m_id_manager->insert( object_id, renderer_id );

    return( std::pair<int,int>( object_id, renderer_id ) );
}

/*==========================================================================*/
/**
 *  @brief  Disable move-all mode.
 */
/*==========================================================================*/
void ScreenBase::disableAllMove( void )
{
    m_enable_move_all = false;
}

/*==========================================================================*/
/**
 *  @brief  Enable move-all mode.
 */
/*==========================================================================*/
void ScreenBase::enableAllMove( void )
{
    m_enable_move_all = true;
}

/*===========================================================================*/
/**
 *  @brief  Disable collision detection mode.
 */
/*===========================================================================*/
void ScreenBase::disableCollisionDetection( void )
{
    m_enable_collision_detection = false;
}

/*===========================================================================*/
/**
 *  @brief  Enable collision detection mode.
 */
/*===========================================================================*/
void ScreenBase::enableCollisionDetection( void )
{
    m_enable_move_all = false;
    m_enable_collision_detection = true;
}

/*==========================================================================*/
/**
 *  @brief  Initalizes the screen.
 */
/*==========================================================================*/
void ScreenBase::initializeFunction( void )
{
    // Set the lighting parameters.
    m_light->on();

    // Attach the Camera to the Mouse
    m_mouse->attachCamera( m_camera );
}

/*==========================================================================*/
/**
 *  @brief  Core paint event function.
 */
/*==========================================================================*/
void ScreenBase::paintFunction( void )
{
    // Update the camera and light.
    m_camera->update();
    m_light->update( m_camera );

    // Set the background color or image.
    m_background->apply();

    // Rendering the resistered object by using the corresponding renderer.
    if ( m_object_manager->hasObject() )
    {
        const int size = m_id_manager->size();
        for ( int index = 0; index < size; index++ )
        {
            kvs::IDPair id_pair = (*m_id_manager)[index];
            kvs::ObjectBase* object = m_object_manager->object( id_pair.first );
            kvs::RendererBase* renderer = m_renderer_manager->renderer( id_pair.second );

            if ( object->isShown() )
            {
                glPushMatrix();
                object->transform( m_object_manager->objectCenter(), m_object_manager->normalize() );
                renderer->exec( object, m_camera, m_light );
                glPopMatrix();
            }
        }
    }
    else m_object_manager->applyXform();
}

/*==========================================================================*/
/**
 *  @brief  Core resize event function.
 *  @param  width [in] screen width
 *  @param  height [in] screen height
 */
/*==========================================================================*/
void ScreenBase::resizeFunction( int width, int height )
{
    // Update the viewport for OpenGL.
    glViewport( 0 , 0 , width , height );

    // Update the window size for camera and mouse.
    m_camera->setWindowSize( width, height );
    m_mouse->setWindowSize( width, height );
}

/*==========================================================================*/
/**
 *  @brief  Function which is called when the mouse button is released.
 *  @param  x [in] x coordinate value of the mouse cursor position
 *  @param  y [in] y coordinate value of the mouse cursor position
 */
/*==========================================================================*/
void ScreenBase::mouseReleaseFunction( int x, int y )
{
    m_enable_move_all = true;
    m_enable_collision_detection = false;
    m_mouse->release( x, y );

    if( !( m_mouse->isUseAuto() && m_mouse->isAuto() ) )
    {
        m_object_manager->releaseActiveObject();
    }
}

/*==========================================================================*/
/**
 *  @brief  Function which is called when the mouse button is released.
 *  @param  x [in] x coordinate value of the mouse cursor position
 *  @param  y [in] y coordinate value of the mouse cursor position
 *  @param  mode [in] mouse translation mode
 */
/*==========================================================================*/
void ScreenBase::mousePressFunction( int x, int y, kvs::Mouse::TransMode mode )
{
    if ( m_enable_move_all || m_object_manager->hasActiveObject() )
    {
        this->updateControllingObject();
        m_mouse->setMode( mode );
        m_mouse->press( x, y );
    }
}

/*==========================================================================*/
/**
 *  @brief  Function which is called when the mouse cursor is moved.
 *  @param  x [in] x coordinate value of the mouse cursor position
 *  @param  y [in] y coordinate value of the mouse cursor position
 */
/*==========================================================================*/
void ScreenBase::mouseMoveFunction( int x, int y )
{
    if ( m_enable_move_all || m_object_manager->hasActiveObject() )
    {
        m_mouse->move( x, y );
        this->updateXform();
    }
}

/*===========================================================================*/
/**
 *  @brief  Function which is called when the mouse wheel is scrolled.
 *  @param  value [in] incremental value
 */
/*===========================================================================*/
void ScreenBase::wheelFunction( int value )
{
    if ( m_enable_move_all || m_object_manager->hasActiveObject() )
    {
        this->updateControllingObject();
        m_mouse->setMode( kvs::Mouse::Scaling );
        m_mouse->press( 0, 0 );
        m_mouse->move( 0, value );
    }
}

/*===========================================================================*/
/**
 *  @brief  Returns the x coordinate value of the window position.
 *  @return x coordinate value of the window position
 */
/*===========================================================================*/
const int ScreenBase::x( void ) const
{
    return( m_x );
}

/*===========================================================================*/
/**
 *  @brief  Returns the y coordinate value of the window position.
 *  @return y coordinate value of the window position
 */
/*===========================================================================*/
const int ScreenBase::y( void ) const
{
    return( m_y );
}

/*===========================================================================*/
/**
 *  @brief  Returns the window width.
 *  @return window width
 */
/*===========================================================================*/
const int ScreenBase::width( void ) const
{
    return( m_width );
}

/*===========================================================================*/
/**
 *  @brief  Returns the window height.
 *  @return window height
 */
/*===========================================================================*/
const int ScreenBase::height( void ) const
{
    return( m_height );
}

/*===========================================================================*/
/**
 *  @brief  Returns the window ID.
 *  @return window ID
 */
/*===========================================================================*/
const int ScreenBase::id( void ) const
{
    return( m_id );
}

/*===========================================================================*/
/**
 *  @brief  Returns the window title.
 *  @return window title
 */
/*===========================================================================*/
const std::string& ScreenBase::title( void ) const
{
    return( m_title );
}

/*===========================================================================*/
/**
 *  @brief  Returns the display format.
 */
/*===========================================================================*/
const kvs::DisplayFormat& ScreenBase::displayFormat( void ) const
{
    return( m_display_format );
}

/*===========================================================================*/
/**
 *  @brief  Checks whether the window is full-screen or not.
 *  @return true, if the window is full-screen
 */
/*===========================================================================*/
const bool ScreenBase::isFullScreen( void ) const
{
    return( m_is_fullscreen );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the camera.
 *  @return pointer to the camera
 */
/*===========================================================================*/
kvs::Camera* ScreenBase::camera( void )
{
    return( m_camera );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the light.
 *  @return pointer to the light
 */
/*===========================================================================*/
kvs::Light* ScreenBase::light( void )
{
    return( m_light );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the mouse.
 *  @return pointer to the mouse
 */
/*===========================================================================*/
kvs::Mouse* ScreenBase::mouse( void )
{
    return( m_mouse );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the background class.
 *  @return pointer to the background class
 */
/*===========================================================================*/
kvs::Background* ScreenBase::background( void )
{
    return( m_background );
}

/*===========================================================================*/
/**
 *  @brief  Returns the control target.
 *  @return control target
 */
/*===========================================================================*/
kvs::ScreenBase::ControlTarget& ScreenBase::controlTarget( void )
{
    return( m_target );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the object manager.
 *  @return pointer to the object manager
 */
/*===========================================================================*/
kvs::ObjectManager* ScreenBase::objectManager( void )
{
    return( m_object_manager );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the renderer manager.
 *  @return pointer to the renderer manager
 */
/*===========================================================================*/
kvs::RendererManager* ScreenBase::rendererManager( void )
{
    return( m_renderer_manager );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the ID manager.
 *  @return pointer to the ID manager
 */
/*===========================================================================*/
kvs::IDManager* ScreenBase::IDManager( void )
{
    return( m_id_manager );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the event handler.
 *  @return pointer to the event handler
 */
/*===========================================================================*/
kvs::EventHandler* ScreenBase::eventHandler( void )
{
    return( m_event_handler );
}

/*===========================================================================*/
/**
 *  @brief  Sets a display format.
 *  @param  display_format [in] display format
 */
/*===========================================================================*/
void ScreenBase::setDisplayFormat( const kvs::DisplayFormat& display_format )
{
    m_display_format = display_format;
}

/*===========================================================================*/
/**
 *  @brief  Sets a window position.
 *  @param  x [in] x coordinate value of the window
 *  @param  y [in] y coordinate value of the window
 */
/*===========================================================================*/
void ScreenBase::setPosition( const int x, const int y )
{
    m_x = x;
    m_y = y;
}

/*===========================================================================*/
/**
 *  @brief  Sets a window size.
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
void ScreenBase::setSize( const int width, const int height )
{
    m_width  = width;
    m_height = height;
}

/*===========================================================================*/
/**
 *  @brief  Sets a window geometry (position and size).
 *  @param  x [in] x coordinate value of the window
 *  @param  y [in] y coordinate value of the window
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
void ScreenBase::setGeometry( const int x, const int y, const int width, const int height )
{
    this->setPosition( x, y );
    this->setSize( width, height );
}

/*===========================================================================*/
/**
 *  @brief  Sets a window title.
 *  @param  title [in] window title
 */
/*===========================================================================*/
void ScreenBase::setTitle( const std::string& title )
{
    m_title = title;
}

/*===========================================================================*/
/**
 *  @brief  Creates basic screen components.
 */
/*===========================================================================*/
void ScreenBase::create( void )
{
    m_mouse_event = new kvs::MouseEvent();
    if ( !m_mouse_event )
    {
        kvsMessageError("Cannot allocate memory for the mouse event.");
        return;
    }

    m_key_event = new kvs::KeyEvent();
    if ( !m_key_event )
    {
        kvsMessageError("Cannot allocate memory for the key event.");
        return;
    }

    m_wheel_event = new kvs::WheelEvent();
    if ( !m_wheel_event )
    {
        kvsMessageError("Cannot allocate memory for the wheel event.");
        return;
    }

    m_camera = new kvs::Camera();
    if ( !m_camera )
    {
        kvsMessageError("Cannot allocate memory for the camera.");
        return;
    }

    m_light = new kvs::Light();
    if ( !m_light )
    {
        kvsMessageError("Cannot allocate memory for the light.");
        return;
    }

    m_mouse = new kvs::Mouse();
    if ( !m_mouse )
    {
        kvsMessageError("Cannot allocate memory for the mouse.");
        return;
    }

    m_background = new kvs::Background( kvs::RGBColor( 212, 221, 229 ) );
    if ( !m_background )
    {
        kvsMessageError("Cannot allocate memory for the background.");
        return;
    }

    m_object_manager = new kvs::ObjectManager();
    if ( !m_object_manager )
    {
        kvsMessageError("Cannot allocate memory for the object manager.");
        return;
    }

    m_renderer_manager = new kvs::RendererManager();
    if( !m_renderer_manager )
    {
        kvsMessageError("Cannot allocate memory for the renderer manager.");
        return;
    }

    m_id_manager = new kvs::IDManager();
    if( !m_id_manager )
    {
        kvsMessageError("Cannot allocate memory for the ID manager.");
        return;
    }

    m_event_handler = new kvs::EventHandler();
    if( !m_event_handler )
    {
        kvsMessageError("Cannot allocate memory for the event handler.");
        return;
    }
}

/*===========================================================================*/
/**
 *  @brief  Clears the basic screen components.
 */
/*===========================================================================*/
void ScreenBase::clear( void )
{
    if ( m_mouse_event      ) { delete m_mouse_event;      m_mouse_event      = NULL; }
    if ( m_key_event        ) { delete m_key_event;        m_key_event        = NULL; }
    if ( m_wheel_event      ) { delete m_wheel_event;      m_wheel_event      = NULL; }
    if ( m_camera           ) { delete m_camera;           m_camera           = NULL; }
    if ( m_light            ) { delete m_light;            m_light            = NULL; }
    if ( m_mouse            ) { delete m_mouse;            m_mouse            = NULL; }
    if ( m_background       ) { delete m_background;       m_background       = NULL; }
    if ( m_object_manager   ) { delete m_object_manager;   m_object_manager   = NULL; }
    if ( m_renderer_manager ) { delete m_renderer_manager; m_renderer_manager = NULL; }
    if ( m_id_manager       ) { delete m_id_manager;       m_id_manager       = NULL; }
    if ( m_event_handler    ) { delete m_event_handler;    m_event_handler    = NULL; }
}

/*===========================================================================*/
/**
 *  @brief  Resets the x-form matrix of the basic screen components.
 */
/*===========================================================================*/
void ScreenBase::reset( void )
{
    // Reset viewpoint to the initial position.
    m_mouse->reset();

    // Reset the xform of the object.
    if( m_object_manager->hasActiveObject() )
    {
        m_object_manager->resetActiveObjectXform();
    }
    else
    {
        m_object_manager->resetXform();
    }

    // Reset the xform of the camera.
    m_camera->resetXform();

    // Reset the xform of the light.
    m_light->resetXform();
}

/*==========================================================================*/
/**
 *  @brief  Test whether the screen is the active move mode.
 *  @param  x [in] x coordinate value of the mouse cursor position
 *  @param  y [in] y coordinate value of the mouse cursor position
 *  @return true, if the screen is the active move mode
 */
/*==========================================================================*/
bool ScreenBase::isActiveMove( int x, int y )
{
    if( !m_object_manager->hasObject() ) return( true );

    if( m_target == TargetObject )
    {
        if( !m_enable_move_all && m_enable_collision_detection )
        {
            const float px = static_cast<float>(x);
            const float py = static_cast<float>(y);
            const kvs::Vector2f p = kvs::Vector2f( px, py );
            return( m_object_manager->detectCollision( p, m_camera ) );
        }
    }

    return( true );
}

/*==========================================================================*/
/**
 *  @brief  Updates the controlling object.
 */
/*==========================================================================*/
void ScreenBase::updateControllingObject( void )
{
    if( m_target == TargetObject )
    {
        if( m_enable_move_all )
        {
            m_object_manager->enableAllMove();
            m_object_manager->releaseActiveObject();
        }
        else
        {
            m_object_manager->disableAllMove();
        }
    }
}

/*==========================================================================*/
/**
 *  @brief  Updates the center of rotation.
 */
/*==========================================================================*/
void ScreenBase::updateCenterOfRotation( void )
{
    // Center of rotation in the device coordinate system.
    kvs::Vector2f center( 0.0, 0.0 );

    switch( m_target )
    {
    case TargetCamera:
    case TargetLight:
        /* Get an at-point of the camera, which is the center of rotation,
         * in the device coord.
         */
        center = m_camera->lookAtInDevice();
        break;
    case TargetObject:
        if( m_enable_move_all || !m_object_manager->hasObject() )
        {
            center = m_object_manager->positionInDevice( m_camera );
        }
        else
        {
            /* If the object manager has no active object, the center
             * of rotation is not updated.
             */
            if ( !m_object_manager->hasActiveObject() ) return;

            kvs::ObjectBase* object = m_object_manager->activeObject();
            const kvs::Vector3f& t = m_object_manager->objectCenter();
            const kvs::Vector3f& s = m_object_manager->normalize();
            center = object->positionInDevice( m_camera, t, s );
        }
        break;
    default:
        break;
    }

    m_mouse->setRotationCenter( center );
}

/*==========================================================================*/
/**
 *  @brief  Updates the xform of the control target.
 */
/*==========================================================================*/
void ScreenBase::updateXform( void )
{
    switch( m_target )
    {
    case TargetCamera:
        this->updateXform( m_camera );
        break;
    case TargetLight:
        this->updateXform( m_light );
        break;
    case TargetObject:
        this->updateXform( m_object_manager );
        break;
    default:
        break;
    }
}

/*==========================================================================*/
/**
 *  @brief  Updates the xform of the object manager.
 *  @param  manager [in] pointer to the object manager
 */
/*==========================================================================*/
void ScreenBase::updateXform( kvs::ObjectManager* manager )
{
    switch( m_mouse->mode() )
    {
    case kvs::Mouse::Rotation:
        manager->rotate( m_mouse->rotation().toMatrix() );
        break;
    case kvs::Mouse::Translation:
        manager->translate( m_mouse->translation() );
        break;
    case kvs::Mouse::Scaling:
        manager->scale( m_mouse->scaling() );
        break;
    default:
        break;
    }
}

/*==========================================================================*/
/**
 *  @brief  Updates the xform of the camera.
 *  @param  camera [in] pointer to the camera
 */
/*==========================================================================*/
void ScreenBase::updateXform( kvs::Camera* camera )
{
    switch( m_mouse->mode() )
    {
    case kvs::Mouse::Rotation:
        camera->rotate( m_mouse->rotation().toMatrix() );
        break;
    case kvs::Mouse::Translation:
        camera->translate( m_mouse->translation() );
        break;
    case kvs::Mouse::Scaling:
        camera->scale( m_mouse->scaling() );
        break;
    default:
        break;
    }
}

/*==========================================================================*/
/**
 *  @brief  Updates the xform of the light.
 *  @param  light [in] pointer to the light
 */
/*==========================================================================*/
void ScreenBase::updateXform( kvs::Light* light )
{
    switch( m_mouse->mode() )
    {
    case kvs::Mouse::Rotation:
        light->rotate( m_mouse->rotation().toMatrix() );
        break;
    case kvs::Mouse::Translation:
        light->translate( m_mouse->translation() );
        break;
    case kvs::Mouse::Scaling:
        light->scale( m_mouse->scaling() );
        break;
    default:
        break;
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates the window.
 */
/*===========================================================================*/
void ScreenBase::createWindow( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Shows the window as full-screen.
 */
/*===========================================================================*/
void ScreenBase::showFullScreen( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Shows the window as normal screen.
 */
/*===========================================================================*/
void ScreenBase::showNormal( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Pops up the window.
 */
/*===========================================================================*/
void ScreenBase::popUp( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Pushes down the window.
 */
/*===========================================================================*/
void ScreenBase::pushDown( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Hides the window.
 */
/*===========================================================================*/
void ScreenBase::hide( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Show the hiding window.
 */
/*===========================================================================*/
void ScreenBase::showWindow( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Redraws the window.
 */
/*===========================================================================*/
void ScreenBase::redraw( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Resizes the window.
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
void ScreenBase::resize( int width, int height )
{
    kvs::IgnoreUnusedVariable( width );
    kvs::IgnoreUnusedVariable( height );
}

} // end of namespace kvs
