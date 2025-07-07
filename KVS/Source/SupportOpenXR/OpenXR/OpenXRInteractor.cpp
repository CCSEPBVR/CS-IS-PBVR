/*****************************************************************************/
/**
 *  @file   OpenXRInteractor.cpp
 *  @author Keisuke Tajiri
 */
/*****************************************************************************/
#include "OpenXRInteractor.h"
#include <kvs/Assert>
#include <kvs/ObjectManager>
#include <kvs/StochasticPolygonRenderer>
#include <kvs/StochasticLineRenderer>
#include <SupportQt/Event/EventTimer.h>

namespace kvs
{

namespace openxr
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new OpenXRInteractor class.
 */
/*===========================================================================*/
OpenXRInteractor::OpenXRInteractor( kvs::openxr::Screen* screen ) :
    m_openxr_screen( screen ),
    m_remote_desktop( new RemoteDesktop() )
{
    kvsMessageDebug( "OpenXRInteractor::OpenXRInteractor" );

    KVS_ASSERT( m_openxr_screen != nullptr );
    BaseClass::addEventType( kvs::EventBase::KeyPressEvent );
    BaseClass::addEventType( kvs::EventBase::InitializeEvent );
    BaseClass::addEventType( kvs::EventBase::PaintEvent );
    BaseClass::addEventType( kvs::EventBase::TimerEvent );
    BaseClass::addEventType( kvs::EventBase::ControllerPressEvent );
    BaseClass::addEventType( kvs::EventBase::ControllerReleaseEvent );
    BaseClass::addEventType( kvs::EventBase::ControllerMoveEvent );
    BaseClass::addEventType( kvs::EventBase::ControllerAxisEvent );
    BaseClass::setTimerInterval( 1 );
}

/*===========================================================================*/
/**
 *  @brief  Destructs a OpenXRInteractor class.
 */
 /*===========================================================================*/
OpenXRInteractor::~OpenXRInteractor()
{
    kvsMessageDebug( "OpenXRInteractor::~OpenXRInteractor" );

    if ( m_remote_desktop ) delete m_remote_desktop;
}

/*===========================================================================*/
/**
 *  @brief  Initialize event.
 */
 /*===========================================================================*/
void OpenXRInteractor::initializeEvent()
{
    KVS_ASSERT( m_openxr_screen != nullptr );
    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );

    m_remote_desktop->initialize();
    
#if 0
    {
        kvs::ValueArray<kvs::Real32> x_coords =
        {
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
        };
        m_axis_x = new kvs::LineObject();
        m_axis_x->setLineTypeToStrip();
        m_axis_x->setCoords(x_coords);
        m_axis_x->setColorTypeToLine();
        m_axis_x->setColor(kvs::RGBColor::Red());
        m_axis_x->setSize(30.0f);
        s->registerObject(m_axis_x, new kvs::StochasticLineRenderer());

        kvs::ValueArray<kvs::Real32> y_coords =
        {
            0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
        };
        m_axis_y = new kvs::LineObject();
        m_axis_y->setLineTypeToStrip();
        m_axis_y->setCoords(y_coords);
        m_axis_y->setColorTypeToLine();
        m_axis_y->setColor(kvs::RGBColor::Green());
        m_axis_y->setSize(30.0f);
        s->registerObject(m_axis_y, new kvs::StochasticLineRenderer());
    
        kvs::ValueArray<kvs::Real32> z_coords =
        {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
        };
        m_axis_z = new kvs::LineObject();
        m_axis_z->setLineTypeToStrip();
        m_axis_z->setCoords(z_coords);
        m_axis_z->setColorTypeToLine();
        m_axis_z->setColor(kvs::RGBColor::Blue());
        m_axis_z->setSize(30.0f);
        s->registerObject(m_axis_z, new kvs::StochasticLineRenderer());
    }
#endif

    for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
    {
        std::string path = i == kvs::Side::Left ?
            std::string( "Models/left_hand.ply" ) : std::string( "Models/right_hand.ply" );
        m_controller_model[i] = new kvs::PolygonImporter( path );
        kvs::StochasticPolygonRenderer* renderer = new kvs::StochasticPolygonRenderer();
        renderer->setShader( kvs::Shader::Phong() );
        s->registerObject( m_controller_model[i], renderer );
    }

    kvs::ValueArray<kvs::Real32> pointer_coords =
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f,
    };

    m_pointer = new kvs::LineObject();
    m_pointer->setLineTypeToStrip();
    m_pointer->setCoords( pointer_coords );
    m_pointer->setColorTypeToLine();
    m_pointer->setColor( kvs::RGBColor::Green() );
    m_pointer->setSize( 2.0f );
    s->registerObject( m_pointer, new kvs::StochasticLineRenderer() );

    BaseClass::setEventTimer( new kvs::qt::EventTimer( this ) );
    BaseClass::eventTimer()->start( BaseClass::timerInterval() );
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
 /*===========================================================================*/
void OpenXRInteractor::paintEvent()
{
    KVS_ASSERT( m_openxr_screen != nullptr );

    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );
    
    s->updateGLProjectionMatrix();
    s->updateGLViewingMatrix();
    
    if ( m_remote_desktop ) m_remote_desktop->render();
}

/*===========================================================================*/
/**
 *  @brief  Time event.
 *  @param  e [in] time event
 */
/*===========================================================================*/
void OpenXRInteractor::timerEvent( kvs::TimeEvent* e )
{
    BaseClass::screen()->redraw();
}

/*===========================================================================*/
/**
 *  @brief  controller press event.
 *  @param  e [in] controller event
 */
 /*===========================================================================*/
void OpenXRInteractor::controllerPressEvent( kvs::ControllerEvent* e )
{
    KVS_ASSERT( m_openxr_screen != nullptr );

    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );
    
    const kvs::Controller::ControllerStatus& cs = e->controllerStatus();
    if ( e->type() == kvs::EventBase::ControllerPressEvent )
    {
        for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
        {
            for( kvs::UInt32 j = 0; j < kvs::Controller::Button::Max; ++j )
            {
                if ( cs.button_status[i][j].pressed )
                {
                    if ( j == kvs::Controller::Button::Trigger )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed Trigger", i );
                    }
                    if ( j == kvs::Controller::Button::Menu )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed Menu", i );
                        // if ( m_remote_desktop )
                        // {
                        //     m_remote_desktop->setVisible( !m_remote_desktop->visible() );
                        //     if ( m_remote_desktop->visible() )
                        //     {
                        //         kvs::Xform head_xform = m_openxr_screen->walkthrough() * m_openxr_screen->headXform();
                        //         kvs::Xform rdp_offset_xform = kvs::Xform::Translation( kvs::Vec3( 0.0f, 0.2f, -1.0f ) );
                        //         kvs::Xform rdp_xform = head_xform * rdp_offset_xform;
                        //         m_remote_desktop->setRDPXform( rdp_xform );
                        //     }
                        // }
                    }
                    if ( j == kvs::Controller::Button::A )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed A", i );
                    }
                    if ( j == kvs::Controller::Button::B )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed B", i );
                        // if ( m_remote_desktop )
                        // {
                        //     m_remote_desktop->setVisible( !m_remote_desktop->visible() );
                        //     if ( m_remote_desktop->visible() )
                        //     {
                        //         kvs::Xform head_xform = m_openxr_screen->walkthrough() * m_openxr_screen->headXform();
                        //         kvs::Xform rdp_offset_xform = kvs::Xform::Translation( kvs::Vec3( 0.0f, 0.2f, -1.0f ) );
                        //         kvs::Xform rdp_xform = head_xform * rdp_offset_xform;
                        //         m_remote_desktop->setRDPXform( rdp_xform );
                        //     }
                        // }
                    }
                    if ( j == kvs::Controller::Button::X )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed X", i );
                    }
                    if ( j == kvs::Controller::Button::Y )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed Y", i );
                        // if ( m_remote_desktop )
                        // {
                        //     m_remote_desktop->setVisible( !m_remote_desktop->visible() );
                        //     if ( m_remote_desktop->visible() )
                        //     {
                        //         kvs::Xform head_xform = m_openxr_screen->walkthrough() * m_openxr_screen->headXform();
                        //         kvs::Xform rdp_offset_xform = kvs::Xform::Translation( kvs::Vec3( 0.0f, 0.2f, -1.0f ) );
                        //         kvs::Xform rdp_xform = head_xform * rdp_offset_xform;
                        //         m_remote_desktop->setRDPXform( rdp_xform );
                        //     }
                        // }
                    }
                }
            }
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Controller Release event.
 *  @param  e [in] controller event
 */
 /*===========================================================================*/
void OpenXRInteractor::controllerReleaseEvent(kvs::ControllerEvent* e)
{
    KVS_ASSERT( m_openxr_screen != nullptr );

    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );

    const kvs::Controller::ControllerStatus& cs = e->controllerStatus();

    if ( e->type() == kvs::EventBase::ControllerReleaseEvent )
    {
        for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
        {
            for ( kvs::UInt32 j = 0; j < kvs::Controller::Button::Max; ++j )
            {
                bool released = cs.button_status[i][j].released;
                if ( released )
                {
                    if (j == kvs::Controller::Button::Trigger)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released Trigger", i );
                    }
                    if (j == kvs::Controller::Button::Menu)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released Menu", i );
                    }
                    if (j == kvs::Controller::Button::A)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released A", i );
                    }
                    if (j == kvs::Controller::Button::B)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released B", i );
                    }
                    if (j == kvs::Controller::Button::X)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released X", i );
                    }
                    if (j == kvs::Controller::Button::Y)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released Y", i );
                    }
                }
            }
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Controller Release event.
 *  @param  e [in] controller event
 */
 /*===========================================================================*/
void OpenXRInteractor::controllerMoveEvent( kvs::ControllerEvent* e )
{
    KVS_ASSERT( m_openxr_screen != nullptr);

    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );

    if ( m_axis_x ) { m_axis_x->resetXform(); }
    if ( m_axis_y ) { m_axis_y->resetXform(); }
    if ( m_axis_z ) { m_axis_z->resetXform(); }

    const kvs::Controller::ControllerStatus& cs = e->controllerStatus();
    if ( e->type() == kvs::EventBase::ControllerMoveEvent )
    {
        kvs::Xform walkthrough_xform = m_openxr_screen->walkthrough();
        kvs::Xform head_xform = walkthrough_xform * m_openxr_screen->headXform();
        kvs::UInt32 ops_controller_index = m_openxr_screen->opsHand();
        kvs::Xform ops_cotroller_xform = walkthrough_xform * cs.xform[ops_controller_index];
        bool pressed = cs.button_status[ops_controller_index][kvs::Controller::Button::Trigger].pressed;
        bool pressing = cs.button_status[ops_controller_index][kvs::Controller::Button::Trigger].pressing;
        bool released = cs.button_status[ops_controller_index][kvs::Controller::Button::Trigger].released;
        if ( m_remote_desktop->visible() )
        {
            float distance = 0.0f;
            m_remote_desktop->update( ops_cotroller_xform, pressed, pressing, released, distance );
            m_pointer_scale = kvs::Vec3(1.0f, 1.0f, distance);
        }
        else
        {
            if ( cs.button_status[kvs::Side::Left][kvs::Controller::Button::Trigger].pressing &&
                 cs.button_status[kvs::Side::Right][kvs::Controller::Button::Trigger].pressing )
            {
                kvs::Xform left_c = walkthrough_xform * cs.xform[kvs::Side::Left];
                kvs::Xform left_p = walkthrough_xform * cs.last_xform[kvs::Side::Left];
                kvs::Xform right_c = walkthrough_xform * cs.xform[kvs::Side::Right];
                kvs::Xform right_p = walkthrough_xform * cs.last_xform[kvs::Side::Right];

                // scaling
                float dist_c = ( left_c.translation() - right_c.translation() ).length();
                float dist_p = ( left_p.translation() - right_p.translation() ).length();
                float scale = dist_c / dist_p;

                // translation
                kvs::Vec3 head_pos = head_xform.translation();
                kvs::Mat4 m = head_xform.toMatrix();
                kvs::Vec3 head_dir = -kvs::Vec3( m[0][2], m[1][2], m[2][2] );
                head_dir.normalize();
                
                kvs::Vec3 center_c = (left_c.translation() + right_c.translation()) / 2.0f;
                kvs::Vec3 center_p = (left_p.translation() + right_p.translation()) / 2.0f;
                kvs::Vec3 center_diff = center_c - center_p;
                kvs::Vec3 center_diff_xy = kvs::Vec3( center_diff.x(), center_diff.y(), 0.0f );
                kvs::Vec3 cont_to_head = center_p - head_pos;
                float cont_distance = kvs::Math::Abs( cont_to_head.dot( head_dir ) );
                kvs::Vec3 object_pos = s->objectManager()->xform().translation();
                kvs::Vec3 object_to_head = object_pos - head_pos;
                float obj_distance = kvs::Math::Abs( object_to_head.dot( head_dir ) );
                float factor = cont_distance < 1e-100 ? 1.0f : obj_distance / cont_distance;
                kvs::Vec3 translation = center_diff * factor;

                // rotation
                kvs::Vec3 rot_c = left_c.translation() - right_c.translation();
                kvs::Vec3 rot_p = left_p.translation() - right_p.translation();
                kvs::Vec3 rot_axis = ( rot_c.cross( rot_p ).normalized() ) * -1.0f;
                float cos = ( rot_c.dot( rot_p ) / ( rot_c.length() * rot_p.length() ) );
                cos = kvs::Math::Clamp( cos, -1.0f, 1.0f );
                float deg = kvs::Math::Rad2Deg( std::acos( cos ) );
                kvs::Mat3 rot_mat = kvs::Mat3::Rotation( rot_axis, deg );

                s->objectManager()->scale( kvs::Vec3( scale, scale, scale ) );
                s->objectManager()->translate( translation );
                s->objectManager()->rotate( rot_mat );
            }
            m_pointer_scale = kvs::Vec3::Ones();
        }

        for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
        {
            kvs::Xform xform = walkthrough_xform * cs.xform[i];
            if ( m_controller_model[i] )
            {
                m_controller_model[i]->setVisible(cs.is_active[i]);
                m_controller_model[i]->setXform(xform);
            }

            if ( i == ops_controller_index && m_pointer )
            {
                m_pointer->setVisible( cs.is_active[i] );
                kvs::Xform pointer_xform = kvs::Xform( xform.translation(), m_pointer_scale, xform.rotation() );
                m_pointer->setXform( pointer_xform );
            }
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Controller Axis event.
 *  @param  e [in] controller event
 */
 /*===========================================================================*/
void OpenXRInteractor::controllerAxisEvent( kvs::ControllerEvent* e )
{
    KVS_ASSERT( m_openxr_screen != nullptr);

    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );

    m_elapsed_timer.stop();
    float elapsedTime = static_cast<float>( m_elapsed_timer.sec() );
    m_elapsed_timer.start();

    const kvs::Controller::ControllerStatus& cs = e->controllerStatus();
    if ( e->type() == kvs::EventBase::ControllerAxisEvent )
    {
        for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
        {
            kvs::Vec2 axis = cs.axis_value[i];
            if ( ( kvs::Math::Abs( axis.x() ) > 0 || kvs::Math::Abs( axis.y() ) > 0 ) && elapsedTime > 0.0f )
            {
                kvs::Xform walkthrough_xform = m_openxr_screen->walkthrough();
                kvs::Mat4 m = cs.xform[i].toMatrix();
                kvs::Vec3 dir = -kvs::Vec3( m[0][2], m[1][2], m[2][2] );
                dir.normalize();

                kvs::Vec3 translation = dir * axis[1] * 3.0f * elapsedTime;
                float heading = -axis[0] * 3.0f * elapsedTime;
                kvs::Mat3 rotation;
                rotation[0][0] = std::cosf( heading );
                rotation[0][1] = 0.0f;
                rotation[0][2] = std::sinf( heading );
                rotation[1][0] = 0.0f;
                rotation[1][1] = 1.0f;
                rotation[1][2] = 0.0f;
                rotation[2][0] = -std::sinf( heading );
                rotation[2][1] = 0.0f;
                rotation[2][2] = std::cosf( heading );

                kvs::Xform move_xform = kvs::Xform( translation, kvs::Vec3::Ones(), rotation );
                m_openxr_screen->setWalkthrough( walkthrough_xform * move_xform );
                break;
            }
        }
    }
}

} // end of namespace openxr

} // end of namespace kvs
