/****************************************************************************/
/**
 *  @file ScreenCore.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ScreenCore.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ScreenCore.h"
#include <kvs/DebugNew>
#include <kvs/GlobalCore>
#include <kvs/Camera>
#include <kvs/Background>
#include <kvs/Light>
#include <kvs/Mouse>
#include <kvs/ObjectManager>
#include <kvs/RendererManager>
#include <kvs/IDManager>


namespace kvs
{

// Instantiation of the additional functions.
InitializeFunc         ScreenCore::m_pfunc_add_initialize_func;
PaintEventFunc         ScreenCore::m_pfunc_add_paint_event;
ResizeEventFunc        ScreenCore::m_pfunc_add_resize_event;
MousePressEventFunc    ScreenCore::m_pfunc_add_mouse_press_event;
MouseMoveEventFunc     ScreenCore::m_pfunc_add_mouse_move_event;
MouseReleaseEventFunc  ScreenCore::m_pfunc_add_mouse_release_event;
KeyPressEventFunc      ScreenCore::m_pfunc_add_key_press_event;

// Instantiation of the set functions.
MousePressEventFunc    ScreenCore::m_pfunc_set_mouse_press_event;
MouseMoveEventFunc     ScreenCore::m_pfunc_set_mouse_move_event;
MouseReleaseEventFunc  ScreenCore::m_pfunc_set_mouse_release_event;
KeyPressEventFunc      ScreenCore::m_pfunc_set_key_press_event;

// Instantiation of the member parameters.
kvs::DisplayFormat ScreenCore::m_display_format;
std::string        ScreenCore::m_title;
int                ScreenCore::m_x;
int                ScreenCore::m_y;
int                ScreenCore::m_width;
int                ScreenCore::m_height;
int                ScreenCore::m_id;
bool               ScreenCore::m_is_fullscreen;
bool               ScreenCore::m_can_move_all;
kvs::MouseEvent*   ScreenCore::m_mouse_event;
kvs::KeyEvent*     ScreenCore::m_key_event;

/*==========================================================================*/
/**
 *  Consturctor.
 */
/*==========================================================================*/
ScreenCore::ScreenCore( void )
{
    m_title         = "<unknown>";
    m_x             = 0;
    m_y             = 0;
    m_width         = 512;
    m_height        = 512;
    m_id            = 0;
    m_is_fullscreen = false;
    m_can_move_all  = true;
    m_mouse_event   = new kvs::MouseEvent();
    m_key_event     = new kvs::KeyEvent();

    /* Link the pre-defined additional event functions to the event
     * functions in this class.
     */
    m_pfunc_add_initialize_func     = add_initialize_func;
    m_pfunc_add_paint_event         = add_paint_event;
    m_pfunc_add_resize_event        = add_resize_event;
    m_pfunc_add_mouse_press_event   = add_mouse_press_event;
    m_pfunc_add_mouse_move_event    = add_mouse_move_event;
    m_pfunc_add_mouse_release_event = add_mouse_release_event;
    m_pfunc_add_key_press_event     = add_key_press_event;

    /* Link the pre-defined set event functions to the event functions
     * in this class.
     */
    m_pfunc_set_mouse_press_event   = set_mouse_press_event;
    m_pfunc_set_mouse_move_event    = set_mouse_move_event;
    m_pfunc_set_mouse_release_event = set_mouse_release_event;
    m_pfunc_set_key_press_event     = set_key_press_event;
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
ScreenCore::~ScreenCore( void )
{
    if( m_mouse_event ) delete m_mouse_event;
    if( m_key_event )   delete m_key_event;
}

/*==========================================================================*/
/**
 *  Registrate a additional initialize function.
 *  @param pfunc [in] pointer to the given function
 */
/*==========================================================================*/
void ScreenCore::addInitializeFunc( InitializeFunc pfunc )
{
    m_pfunc_add_initialize_func = pfunc;
}

/*==========================================================================*/
/**
 *  Registrate a additional paint event function.
 *  @param event [in] pointer to the event function
 */
/*==========================================================================*/
void ScreenCore::addPaintEvent( PaintEventFunc event )
{
    m_pfunc_add_paint_event = event;
}

/*==========================================================================*/
/**
 *  Registrate a additional resize event function.
 *  @param event [in] pointer to the event function.
 */
/*==========================================================================*/
void ScreenCore::addResizeEvent( ResizeEventFunc event )
{
    m_pfunc_add_resize_event = event;
}

/*==========================================================================*/
/**
 *  Registrate a additional mouse press event function.
 *  @param event [in] pointer to the event function.
 */
/*==========================================================================*/
void ScreenCore::addMousePressEvent( MousePressEventFunc event )
{
    m_pfunc_add_mouse_press_event = event;
}

/*==========================================================================*/
/**
 *  Registrate a additional mouse move event function.
 *  @param event [in] pointer to the event function.
 */
/*==========================================================================*/
void ScreenCore::addMouseMoveEvent( MouseMoveEventFunc event )
{
    m_pfunc_add_mouse_move_event = event;
}

/*==========================================================================*/
/**
 *  Registrate a additional mouse release event function.
 *  @param event [in] pointer to the event function.
 */
/*==========================================================================*/
void ScreenCore::addMouseReleaseEvent( MouseReleaseEventFunc event )
{
    m_pfunc_add_mouse_release_event = event;
}

/*==========================================================================*/
/**
 *  Registrate a additional key press event function.
 *  @param event [in] pointer to the event function
 */
/*==========================================================================*/
void ScreenCore::addKeyPressEvent( KeyPressEventFunc event )
{
    m_pfunc_add_key_press_event = event;
}

/*==========================================================================*/
/**
 *  Set the display format.
 *  @param display_format [in] display format
 */
/*==========================================================================*/
void ScreenCore::setDisplayFormat( const kvs::DisplayFormat& display_format )
{
    m_display_format = display_format;
}

/*==========================================================================*/
/**
 *  Set screen position.
 *  @param x [in] x coordinate on the desktop
 *  @param y [in] y coordinate on the desktop
 */
/*==========================================================================*/
void ScreenCore::setPosition( int x, int y )
{
    m_x = x;
    m_y = y;
}

/*==========================================================================*/
/**
 *  Set screen size.
 *  @param width [in] width
 *  @param height [in] height
 */
/*==========================================================================*/
void ScreenCore::setSize( int width, int height )
{
    m_width  = width;
    m_height = height;

    if( kvs::GlobalCore::camera ) kvs::GlobalCore::camera->setWindowSize( width, height );
    if( kvs::GlobalCore::mouse  ) kvs::GlobalCore::mouse->setWindowSize( width, height );
}

/*==========================================================================*/
/**
 *  Set screen geometry.
 *  @param x [in] x coordinate on the desktop
 *  @param y [in] y coordinate on the desktop
 *  @param width [in] width
 *  @param height [in] height
 */
/*==========================================================================*/
void ScreenCore::setGeometry( int x, int y, int width, int height )
{
    setPosition( x, y );
    setSize( width, height );
}

/*==========================================================================*/
/**
 *  Set screen title.
 *  @param title [in] title
 */
/*==========================================================================*/
void ScreenCore::setTitle( const std::string& title )
{
    m_title = title;
}

/*==========================================================================*/
/**
 *  Returns the display format.
 *  @return display format
 */
/*==========================================================================*/
const kvs::DisplayFormat& ScreenCore::displayFormat( void ) const
{
    return( m_display_format );
}

/*==========================================================================*/
/**
 *  Get x coordinate value of screen position.
 *  @return x coordinate value
 */
/*==========================================================================*/
const int ScreenCore::x( void ) const
{
    return( m_x );
}

/*==========================================================================*/
/**
 *  Get y coordinate value of screen position.
 *  @return y coordinate value
 */
/*==========================================================================*/
const int ScreenCore::y( void ) const
{
    return( m_y );
}

/*==========================================================================*/
/**
 *  Get screen width.
 *  @return width
 */
/*==========================================================================*/
const int ScreenCore::width( void ) const
{
    return( m_width );
}

/*==========================================================================*/
/**
 *  Get screen height
 *  @return height
 */
/*==========================================================================*/
const int ScreenCore::height( void ) const
{
    return( m_height );
}

/*==========================================================================*/
/**
 *  Get screen ID.
 *  @return ID
 */
/*==========================================================================*/
const int ScreenCore::id( void ) const
{
    return( m_id );
}

/*==========================================================================*/
/**
 *  Check for fullscreen mode.
 *  @return true, current screen is fullscreen
 */
/*==========================================================================*/
bool ScreenCore::isFullScreen( void )
{
    return( m_is_fullscreen );
}

/*==========================================================================*/
/**
 *  Disable all-move mode.
 */
/*==========================================================================*/
void ScreenCore::disableAllMove( void )
{
    m_can_move_all = false;
}

/*==========================================================================*/
/**
 *  Initalize the screen.
 */
/*==========================================================================*/
void ScreenCore::initialize( void )
{
    // Set the lighting parameters.
    kvs::GlobalCore::light->setID( GL_LIGHT0 );
    kvs::GlobalCore::light->setPosition( 0.0, 0.0, 12.0 );
    kvs::GlobalCore::light->setColor( 1.0, 1.0, 1.0 );
    kvs::GlobalCore::light->on();

    // Attach the Camera to the Mouse
    kvs::GlobalCore::mouse->attachCamera( kvs::GlobalCore::camera );

    // Call the additional initializing function.
    m_pfunc_add_initialize_func();
}

/*==========================================================================*/
/**
 *  Core paint event function.
 */
/*==========================================================================*/
void ScreenCore::paint_event_core( void )
{
     glMatrixMode( GL_MODELVIEW );
     {
         glLoadIdentity();

         // Update the camera and light.
         kvs::GlobalCore::camera->update();
         kvs::GlobalCore::light->update( kvs::GlobalCore::camera );

         // Set the background color or image.
         kvs::GlobalCore::background->apply();

         // Aliases for the object manager and the renderer manager.
         kvs::ObjectManager*   om = kvs::GlobalCore::object_manager;
         kvs::RendererManager* rm = kvs::GlobalCore::renderer_manager;
         kvs::IDManager*       id = kvs::GlobalCore::id_manager;
         kvs::Camera*          c  = kvs::GlobalCore::camera;
         kvs::Light*           l  = kvs::GlobalCore::light;

         // Rendering the resistered object by using the corresponding renderer.
         glPushMatrix();
         if( om->hasObject() )
         {
             const int size = id->size();
             for( int index = 0; index < size; index++ )
             {
                 kvs::IDPair id_pair = (*id)[index];

                 kvs::ObjectBase*   o = om->object( id_pair.first );
                 kvs::RendererBase* r = rm->renderer( id_pair.second );

                 if( o->isShown() )
                 {
                     glPushMatrix();
                     o->transform( om->objectCenter(), om->normalize() );
                     r->exec( o, c, l );
                     glPopMatrix();
                 }
             }
         }
         else om->applyXform();

         // Call the additional paint function here.
         m_pfunc_add_paint_event();
         glPopMatrix();
     }
}

/*==========================================================================*/
/**
 *  Core resize event function.
 *  @param width [in] screen width
 *  @param height [in] screen height
 */
/*==========================================================================*/
void ScreenCore::resize_event_core( int width, int height )
{
    if( !m_is_fullscreen )
    {
        m_width  = width;
        m_height = height;
    }

    // Update the viewport for OpenGL.
    glViewport( 0 , 0 , width , height );

    // Update the window size for camera and mouse.
    kvs::GlobalCore::camera->setWindowSize( width, height );
    kvs::GlobalCore::mouse->setWindowSize( width, height );

    // Call the additional resize function.
    m_pfunc_add_resize_event( width, height );
}

/*==========================================================================*/
/**
 *  Function which is called when the mouse button is released.
 *  @param x [in] mouse press position
 *  @param y [in] mouse press position
 */
/*==========================================================================*/
void ScreenCore::mouse_button_release( int x, int y )
{
    m_can_move_all = true;
    kvs::GlobalCore::mouse->release( x, y );

    if( !( kvs::GlobalCore::mouse->isUseAuto() &&
           kvs::GlobalCore::mouse->isAuto() ) )
    {
        kvs::GlobalCore::object_manager->releaseActiveObject();
    }
}

/*==========================================================================*/
/**
 *  Function which is called when the mouse button is released.
 *  @param x [in] mouse press position
 *  @param y [in] mouse press position
 *  @param mode [in] mouse translation mode
 */
/*==========================================================================*/
void ScreenCore::mouse_button_press( int x, int y, kvs::Mouse::TransMode mode )
{
    set_object_manager_params();

    kvs::GlobalCore::mouse->setMode( mode );
    kvs::GlobalCore::mouse->press( x, y );
}

/*==========================================================================*/
/**
 *  Test whether the screen is the active move mode.
 *  @param x [in] mouse press position
 *  @param y [in] mouse press position
 *  @return true, if the screen is the active move mode
 */
/*==========================================================================*/
bool ScreenCore::is_active_move( int x, int y )
{
    if( !kvs::GlobalCore::object_manager->hasObject() ) return( true );

    if( kvs::GlobalCore::target == kvs::GlobalCore::TargetObject )
    {
        if( !m_can_move_all )
        {
            // Collision detection.
            kvs::ObjectManager* om = kvs::GlobalCore::object_manager;
            kvs::Camera*        c  = kvs::GlobalCore::camera;
            const float px = static_cast<float>(x);
            const float py = static_cast<float>(y);
            const kvs::Vector2f p  = kvs::Vector2f( px, py );
            return( om->detectCollision( p, c ) );
        }
    }

    return( true );
}

/*==========================================================================*/
/**
 *  Set the object manager parameters.
 */
/*==========================================================================*/
void ScreenCore::set_object_manager_params( void )
{
    if( kvs::GlobalCore::target == kvs::GlobalCore::TargetObject )
    {
        if( m_can_move_all )
        {
            kvs::GlobalCore::object_manager->enableAllMove();
            kvs::GlobalCore::object_manager->releaseActiveObject();
        }
        else
        {
            kvs::GlobalCore::object_manager->disableAllMove();
        }
    }
}

/*==========================================================================*/
/**
 *  Set the center of rotation.
 */
/*==========================================================================*/
void ScreenCore::set_center_of_rotation( void )
{
    // Center of rotation in the device coordinate system.
    kvs::Vector2f rot_center( 0.0, 0.0 );

    switch( kvs::GlobalCore::target )
    {
    case kvs::GlobalCore::TargetCamera:
    case kvs::GlobalCore::TargetLight:
        /* Get an at-point of the camera, which is the center of rotation,
         * in the device coord.
         */
        rot_center = kvs::GlobalCore::camera->lookAtInDevice();
        break;
    case kvs::GlobalCore::TargetObject:
        if( m_can_move_all || !kvs::GlobalCore::object_manager->hasObject() )
        {
            kvs::ObjectManager* om = kvs::GlobalCore::object_manager;
            rot_center = om->positionInDevice( kvs::GlobalCore::camera );
        }
        else
        {
            kvs::ObjectManager*  om = kvs::GlobalCore::object_manager;
            kvs::ObjectBase*     o  = om->activeObject();
            kvs::Camera*         c  = kvs::GlobalCore::camera;
            const kvs::Vector3f& t  = om->objectCenter();
            const kvs::Vector3f& s  = om->normalize();
            rot_center = o->positionInDevice( c, t, s );
        }
        break;
    default:
        break;
    }

    kvs::GlobalCore::mouse->setRotationCenter( rot_center );
}

/*==========================================================================*/
/**
 *  Update the xform of control target.
 */
/*==========================================================================*/
void ScreenCore::update_xform( void )
{
    switch( kvs::GlobalCore::target )
    {
    case kvs::GlobalCore::TargetCamera:
        update_camera_xform( kvs::GlobalCore::camera );
        break;
    case kvs::GlobalCore::TargetLight:
        update_light_xform( kvs::GlobalCore::light );
        break;
    case kvs::GlobalCore::TargetObject:
        update_object_manager_xform( kvs::GlobalCore::object_manager );
        break;
    default:
        break;
    }
}

/*==========================================================================*/
/**
 *  Update the xform of the object manager.
 *  @param manager [in] pointer to the object manager
 */
/*==========================================================================*/
void ScreenCore::update_object_manager_xform( kvs::ObjectManager* manager )
{
    switch( kvs::GlobalCore::mouse->mode() )
    {
    case kvs::Mouse::Rotation:
        manager->rotate( kvs::GlobalCore::mouse->rotation().toMatrix() );
        break;
    case kvs::Mouse::Translation:
        manager->translate( kvs::GlobalCore::mouse->translation() );
        break;
    case kvs::Mouse::Scaling:
        manager->scale( kvs::GlobalCore::mouse->scaling() );
        break;
    default:
        break;
    }
}

/*==========================================================================*/
/**
 *  Update the xform of the camera.
 *  @param camera [in] pointer to the camera
 */
/*==========================================================================*/
void ScreenCore::update_camera_xform( kvs::Camera* camera )
{
    switch( kvs::GlobalCore::mouse->mode() )
    {
    case kvs::Mouse::Rotation:
        camera->rotate( kvs::GlobalCore::mouse->rotation().toMatrix() );
        break;
    case kvs::Mouse::Translation:
        camera->translate( kvs::GlobalCore::mouse->translation() );
        break;
    case kvs::Mouse::Scaling:
        camera->scale( kvs::GlobalCore::mouse->scaling() );
        break;
    default:
        break;
    }
}

/*==========================================================================*/
/**
 *  Update the xform of the light.
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
void ScreenCore::update_light_xform( kvs::Light* light )
{
    switch( kvs::GlobalCore::mouse->mode() )
    {
    case kvs::Mouse::Rotation:
        light->rotate( kvs::GlobalCore::mouse->rotation().toMatrix() );
        break;
    case kvs::Mouse::Translation:
        light->translate( kvs::GlobalCore::mouse->translation() );
        break;
    case kvs::Mouse::Scaling:
        light->scale( kvs::GlobalCore::mouse->scaling() );
        break;
    default:
        break;
    }
}

} // end of namespace kvs
