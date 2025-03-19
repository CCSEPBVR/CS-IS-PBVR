/****************************************************************************/
/**
 *  @file ScreenBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ScreenBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__SCREEN_BASE_H_INCLUDE
#define VIS_MODULE__SCREEN_BASE_H_INCLUDE

#include <string>
#include <map>
#include <vismodule/Camera>
#include <vismodule/Light>
#include <vismodule/Mouse>
#include <vismodule/Background>
#include <vismodule/ObjectManager>
#include <vismodule/RendererManager>
#include <vismodule/IDManager>
#include <vismodule/MouseEvent>
#include <vismodule/KeyEvent>
#include <vismodule/WheelEvent>
#include <vismodule/DisplayFormat>
#include <vismodule/VisualizationPipeline>
#include <vismodule/ClassName>
#include <vismodule/IgnoreUnusedVariable>


namespace vismodule
{

class EventHandler;

/*==========================================================================*/
/**
 *  Screen base class.
 */
/*==========================================================================*/
class ScreenBase
{
    visModuleClassName( vismodule::ScreenBase );

public:

    enum ControlTarget
    {
        TargetObject = 0, ///< controlling object
        TargetCamera,     ///< controlling camera
        TargetLight,      ///< controlling light
        NumberOfTargets   ///< number of control targets
    };

protected:

    // Screeen properties.
    std::string           m_title;            ///< window title
    int                   m_x;                ///< window position (y position)
    int                   m_y;                ///< window position (x position)
    int                   m_width;            ///< window size (width)
    int                   m_height;           ///< window size (height)
    int                   m_id;               ///< window ID
    bool                  m_is_fullscreen;    ///< check flag whether the window is fullscreen
    vismodule::DisplayFormat    m_display_format;   ///< display format

    // Screen events.
    vismodule::MouseEvent*      m_mouse_event;      ///< mouse event
    vismodule::KeyEvent*        m_key_event;        ///< key event
    vismodule::WheelEvent*      m_wheel_event;      ///< wheel event
    vismodule::Timer            m_elapse_time_counter; ///< elapse time counter for double click event

    // Basic components in the viewer.
    vismodule::Camera*          m_camera;           ///< camera
    vismodule::Light*           m_light;            ///< light
    vismodule::Mouse*           m_mouse;            ///< mouse
    vismodule::Background*      m_background;       ///< background

    // Parameters for controlling the viewer.
    ControlTarget         m_target;           ///< control target
    vismodule::ObjectManager*   m_object_manager;   ///< object manager
    vismodule::RendererManager* m_renderer_manager; ///< renderer manager
    vismodule::IDManager*       m_id_manager;       ///< ID manager ( object_id, renderer_id )

    bool m_enable_move_all;  ///< flag for object movement
    bool m_enable_collision_detection; ///< flag for collision detection

    vismodule::EventHandler* m_event_handler; ///< event handler

public:

    ScreenBase( void );

    virtual ~ScreenBase( void );

public:

    virtual int show( void ) = 0;

public:

    const std::pair<int,int> registerObject( vismodule::ObjectBase* object, vismodule::RendererBase* renderer = 0 );

    const std::pair<int,int> registerObject( vismodule::VisualizationPipeline* pipeline );

public:

    void initializeFunction( void );

    void paintFunction( void );

    void resizeFunction( int width, int height );

    void mouseReleaseFunction( int x, int y );

    void mousePressFunction( int x, int y, vismodule::Mouse::TransMode mode );

    void mouseMoveFunction( int x, int y );

    void wheelFunction( int value );

public:

    const int x( void ) const;

    const int y( void ) const;

    const int width( void ) const;

    const int height( void ) const;

    const int id( void ) const;

    const std::string& title( void ) const;

    const vismodule::DisplayFormat& displayFormat( void ) const;

    const bool isFullScreen( void ) const;

    vismodule::Camera* camera( void );

    vismodule::Light* light( void );

    vismodule::Mouse* mouse( void );

    vismodule::Background* background( void );

    vismodule::ScreenBase::ControlTarget& controlTarget( void );

    vismodule::ObjectManager* objectManager( void );

    vismodule::RendererManager* rendererManager( void );

    vismodule::IDManager* IDManager( void );

    vismodule::EventHandler* eventHandler( void );

public:

    void setDisplayFormat( const vismodule::DisplayFormat& display_format );

    void setPosition( const int x, const int y );

    void setSize( const int width, const int height );

    void setGeometry( const int x, const int y, const int width, const int height );

    void setTitle( const std::string& title );

public:

    void create( void );

    void clear( void );

    void reset( void );

public:

    void disableAllMove( void );

    void enableAllMove( void );

    void disableCollisionDetection( void );

    void enableCollisionDetection( void );

    bool isActiveMove( int x, int y );

    virtual void updateControllingObject( void );

    virtual void updateCenterOfRotation( void );

    virtual void updateXform( void );

    virtual void updateXform( vismodule::ObjectManager* manager );

    virtual void updateXform( vismodule::Camera* camera );

    virtual void updateXform( vismodule::Light* light );

public:

    virtual void createWindow( void );

    virtual void showFullScreen( void );

    virtual void showNormal( void );

    virtual void popUp( void );

    virtual void pushDown( void );

    virtual void hide( void );

    virtual void showWindow( void );

    virtual void redraw( void );

    virtual void resize( int width, int height );
};

} // end of namespace vismodule

#endif // VIS_MODULE__SCREEN_BASE_H_INCLUDE
