#include "VRHandControllerListener.h"

VRHandControllerListener::VRHandControllerListener( kvs::qt::jaea::Screen* screen )
    : m_screen( screen )
{
}

VRHandControllerListener::~VRHandControllerListener()
{
}

void VRHandControllerListener::onEvent( kvs::EventBase* event )
{
    auto* e = dynamic_cast<kvs::ControllerEvent*>( event );
    if( !e ) { return; }

    const int type = e->type();
    const auto& cs = e->controllerStatus();

    const kvs::UInt32 targets[] = {
        kvs::Controller::Button::B,
        kvs::Controller::Button::A, // NOTE:未使用
        kvs::Controller::Button::Y,
        kvs::Controller::Button::X
    };

    for( kvs::UInt32 side = 0; side < kvs::Side::Max; ++side )
    {
        for( kvs::UInt32 k = 0; k < 4; ++k )
        {
            const kvs::UInt32 button = targets[k];

            const bool pressed_edge  = cs.button_status[side][button].pressed;
            const bool released_edge = cs.button_status[side][button].released;

            if( type == kvs::EventBase::ControllerPressEvent && pressed_edge )
            {
                m_down[side][button] = true;
                m_long_fired[side][button] = false;
                m_down_at[side][button] = std::chrono::steady_clock::now();
            }

            if( m_down[side][button] && !m_long_fired[side][button] )
            {
                const auto now = std::chrono::steady_clock::now();
                const double sec = std::chrono::duration<double>( now - m_down_at[side][button] ).count();

                if( sec >= k_long_press_sec )
                {
                    m_long_fired[side][button] = true;
                    handleLongPress( button, cs, side );
                }
            }

            if( type == kvs::EventBase::ControllerReleaseEvent && released_edge && m_down[side][button] )
            {
                if( !m_long_fired[side][button] )
                {
                    handleShortRelease( button, cs, side );
                }
                m_down[side][button] = false;
            }
        }
    }
}

void VRHandControllerListener::handleLongPress( kvs::UInt32 button, const kvs::Controller::ControllerStatus& cs, kvs::UInt32 side )
{
    auto* scene = m_screen->scene();

    const kvs::Vec3 sT        = m_screen->openxrInteractor()->startInitialTranslation();
    const kvs::Vec3 eT        = m_screen->openxrInteractor()->endInitialTranslation();
    const kvs::ObjectBase* sP = m_screen->openxrInteractor()->startPoint();
    const kvs::ObjectBase* eP = m_screen->openxrInteractor()->endPoint();

    const kvs::Vec3 s = calculateCoord( scene, sT, sP );
    const kvs::Vec3 e = calculateCoord( scene, eT, eP );

    kvs::Real32 coordArray[6] = {
        kvs::Real32( s.x() ), kvs::Real32( s.y() ), kvs::Real32( s.z() ),
        kvs::Real32( e.x() ), kvs::Real32( e.y() ), kvs::Real32( e.z() )
    };

    if( button == kvs::Controller::Button::B )      emit toggleShowHideVRPlotOverLine();
    else if( button == kvs::Controller::Button::Y ) emit toggleShowHideVRPlotOverTime();
    else if( button == kvs::Controller::Button::X )
    {
        const kvs::Xform walkthrough = m_screen->openxrInteractor()->screen()->walkthrough();

        const kvs::Vec3 directionLeft = controllerForward( walkthrough, cs.xform[kvs::Side::Left] );
        const kvs::Vec3 directionRight = controllerForward( walkthrough, cs.xform[kvs::Side::Right] );

        kvs::Real32 directionArray[6] = {
            kvs::Real32( directionLeft.x()  ),
            kvs::Real32( directionLeft.y()  ),
            kvs::Real32( directionLeft.z()  ),

            kvs::Real32( directionRight.x() ),
            kvs::Real32( directionRight.y() ),
            kvs::Real32( directionRight.z() )
        };

        emit toggleShowHideSharePoint( coordArray, directionArray );
    }
}

void VRHandControllerListener::handleShortRelease( kvs::UInt32 button, const kvs::Controller::ControllerStatus& cs, kvs::UInt32 side )
{
    auto* scene = m_screen->scene();

    const kvs::Vec3 sT        = m_screen->openxrInteractor()->startInitialTranslation();
    const kvs::Vec3 eT        = m_screen->openxrInteractor()->endInitialTranslation();
    const kvs::ObjectBase* sP = m_screen->openxrInteractor()->startPoint();
    const kvs::ObjectBase* eP = m_screen->openxrInteractor()->endPoint();

    const kvs::Vec3 s = calculateCoord( scene, sT, sP );
    const kvs::Vec3 e = calculateCoord( scene, eT, eP );

    kvs::Real32 coordArray[6] = {
        kvs::Real32( s.x() ), kvs::Real32( s.y() ), kvs::Real32( s.z() ),
        kvs::Real32( e.x() ), kvs::Real32( e.y() ), kvs::Real32( e.z() )
    };

    if( button == kvs::Controller::Button::B )
    {
        emit drawVRPlotOverLine( coordArray );
    }
    else if( button == kvs::Controller::Button::Y )
    {
        emit drawVRPlotOverTime( coordArray );
    }
    else if( button == kvs::Controller::Button::X )
    {
        const kvs::Xform walkthrough = m_screen->openxrInteractor()->screen()->walkthrough();

        const kvs::Vec3 directionLeft = controllerForward( walkthrough, cs.xform[kvs::Side::Left] );
        const kvs::Vec3 directionRight = controllerForward( walkthrough, cs.xform[kvs::Side::Right] );

        kvs::Real32 directionArray[6] = {
            kvs::Real32( directionLeft.x()  ),
            kvs::Real32( directionLeft.y()  ),
            kvs::Real32( directionLeft.z()  ),

            kvs::Real32( directionRight.x() ),
            kvs::Real32( directionRight.y() ),
            kvs::Real32( directionRight.z() )
        };

        emit drawVRSharePoint( coordArray, directionArray );
    }
}

kvs::Vec3 VRHandControllerListener::calculateCoord( kvs::Scene* scene, const kvs::Vec3& initialT, const kvs::ObjectBase* p ) const
{
    kvs::Xform om = scene->objectManager()->xform();

    const auto* last_obj = scene->object( "Dummy" );
    const float scalingFactor = 1.0f / ( om.inverse() * last_obj->xform() ).scaling().x();

    const kvs::Vec3 pT = ( om.inverse() * p->xform() ).translation();

    const double tx = initialT.x() - pT.x() - om.translation().x() * om.inverse().scaling().x();
    const double ty = initialT.y() - pT.y() - om.translation().y() * om.inverse().scaling().y();
    const double tz = initialT.z() - pT.z() - om.translation().z() * om.inverse().scaling().z();

    double mx = ( tx * scalingFactor * -1.0 ) - ( om.translation().x() * scalingFactor * om.inverse().scaling().x() );
    double my = ( ty * scalingFactor * -1.0 ) - ( om.translation().y() * scalingFactor * om.inverse().scaling().y() );
    double mz = ( tz * scalingFactor * -1.0 ) - ( om.translation().z() * scalingFactor * om.inverse().scaling().z() );

    mx += p->externalCenter().x();
    my += p->externalCenter().y();
    mz += p->externalCenter().z();

    return kvs::Vec3( mx, my, mz );
}

kvs::Vec3 VRHandControllerListener::controllerForward( const kvs::Xform& walkthrough, const kvs::Xform& controller_local ) const
{
    const kvs::Xform controller = walkthrough * controller_local;
    const kvs::Mat3 R = controller.rotation();

    kvs::Vec3 forward( -R[0][2], -R[1][2], -R[2][2] );
    forward.normalize();
    return forward;
}
