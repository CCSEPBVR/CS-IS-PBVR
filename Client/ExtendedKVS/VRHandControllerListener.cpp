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
    auto* interactor = m_screen->openxrInteractor();
    if ( !interactor ) { return; }

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
            const bool overview_button = side == kvs::Side::Left && button == kvs::Controller::Button::Y;
            if ( !cs.is_active[side] || ( interactor->overviewSlideVisible() && !overview_button ) )
            {
                // Discard suppressed holds, including their eventual release/long press.
                m_down[side][button] = false;
                m_long_fired[side][button] = false;
                continue;
            }

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
                    const bool overview_was_visible = interactor->overviewSlideVisible();
                    handleShortRelease( button, cs, side );
                    if ( overview_was_visible != interactor->overviewSlideVisible() )
                    {
                        // Do not carry any pending button operation across open/close.
                        m_down = {};
                        m_long_fired = {};
                        return;
                    }
                }
                m_down[side][button] = false;
            }
        }
    }
}

void VRHandControllerListener::handleLongPress( kvs::UInt32 button, const kvs::Controller::ControllerStatus& cs, kvs::UInt32 side )
{
    Q_UNUSED( cs );
    // Y long press is disabled, and only left Y short press can close the slide.
    if ( button == kvs::Controller::Button::Y || m_screen->openxrInteractor()->overviewSlideVisible() ) { return; }
    if( button == kvs::Controller::Button::B )      emit toggleShowHideVRPlotOverLine();
    else if( button == kvs::Controller::Button::X && side == kvs::Side::Left )
    {
        m_screen->resetView();
    }
}

void VRHandControllerListener::handleShortRelease( kvs::UInt32 button, const kvs::Controller::ControllerStatus& cs, kvs::UInt32 side )
{
    Q_UNUSED( cs );
    if ( button == kvs::Controller::Button::Y )
    {
        if ( side == kvs::Side::Left ) { m_screen->openxrInteractor()->toggleOverviewSlide(); }
        return;
    }
    if ( m_screen->openxrInteractor()->overviewSlideVisible() ) { return; }
    // X short press no longer updates or shares a point.
    if( button == kvs::Controller::Button::X ) { return; }
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
