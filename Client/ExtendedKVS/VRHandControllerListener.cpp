#include "VRHandControllerListener.h"

#include <iostream>

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
    auto* scene = m_screen->scene();
    const int type = e->type();
    const auto& cs = e->controllerStatus();

    const kvs::UInt32 targets[] = {
        kvs::Controller::Button::B,
        kvs::Controller::Button::A,
        kvs::Controller::Button::Y,
        kvs::Controller::Button::X
    };

    for( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
    {
        for( kvs::UInt32 k = 0; k < 4; ++k )
        {
            const kvs::UInt32 j = targets[k];

            const bool pressed_edge  = cs.button_status[i][j].pressed;
            const bool released_edge = cs.button_status[i][j].released;

            if( type == kvs::EventBase::ControllerPressEvent && pressed_edge )
            {
                m_down[i][j] = true;
                m_longFired[i][j] = false;
                m_downAt[i][j] = std::chrono::steady_clock::now();
            }

            if( m_down[i][j] && !m_longFired[i][j] )
            {
                const auto now = std::chrono::steady_clock::now();
                const double sec = std::chrono::duration<double>( now - m_downAt[i][j] ).count();

                if( sec >= k_long_press_sec )
                {
                    m_longFired[i][j] = true;

                    if     ( j == kvs::Controller::Button::B )
                    {
                        emit toggleShowHideVRPlotOverLine();
                    }
                    else if( j == kvs::Controller::Button::A )
                    {
                    }
                    else if( j == kvs::Controller::Button::Y )
                    {
                    }
                    else if( j == kvs::Controller::Button::X )
                    {
                        emit toggleShowHideSharePoint();
                    }
                }
            }

            if( type == kvs::EventBase::ControllerReleaseEvent && released_edge && m_down[i][j] )
            {
                if( !m_longFired[i][j] )
                {
                    if     ( j == kvs::Controller::Button::B )
                    {
                        auto coord = makeCoordArrayForLine(
                            scene,
                            m_screen->openxrInteractor()->startInitialTranslation(),
                            m_screen->openxrInteractor()->endInitialTranslation(),
                            m_screen->openxrInteractor()->startPoint(),
                            m_screen->openxrInteractor()->endPoint()
                            );

                        kvs::Real32 coordArray[6];
                        std::copy( coord.begin(), coord.end(), coordArray );

                        emit drawVRPlotOverLine( coordArray );
                    }
                    else if( j == kvs::Controller::Button::A )
                    {
                    }
                    else if( j == kvs::Controller::Button::Y )
                    {
                    }
                    else if( j == kvs::Controller::Button::X )
                    {
                        auto coord = makeCoordArrayForLine(
                            scene,
                            m_screen->openxrInteractor()->startInitialTranslation(),
                            m_screen->openxrInteractor()->endInitialTranslation(),
                            m_screen->openxrInteractor()->startPoint(),
                            m_screen->openxrInteractor()->endPoint()
                            );

                        kvs::Real32 coordArray[6];
                        std::copy(coord.begin(), coord.end(), coordArray);

                        // NOTE:ture 左手 ,false 右手
                        kvs::UInt32 index = true
                                                ? kvs::UInt32( kvs::Side::Left )
                                                : kvs::UInt32( kvs::Side::Right );

                        kvs::Xform walkthroughXform = m_screen->openxrInteractor()->screen()->walkthrough();
                        kvs::Xform controllerXform = walkthroughXform * cs.xform[index];

                        kvs::Mat3 R = controllerXform.rotation();

                        kvs::Vec3 forward( -R[0][2], -R[1][2], -R[2][2] );
                        forward.normalize();

                        kvs::Real32 directionArray[3] = { forward.x(), forward.y(), forward.z() };

                        emit vrSharePoint( coordArray, directionArray );
                    }
                }
                m_down[i][j] = false;
            }
        }
    }
}

std::array<kvs::Real32, 6> VRHandControllerListener::makeCoordArrayForLine(
    kvs::Scene* scene,
    const kvs::Vec3& startInitialTranslation,
    const kvs::Vec3& endInitialTranslation,
    const kvs::ObjectBase* startPoint,
    const kvs::ObjectBase* endPoint ) const
{
    kvs::Xform om = scene->objectManager()->xform();

    const auto* last_obj = scene->object( scene->numberOfObjects() - 1 );
    float scalingFactor = 1.0f / ( om.inverse() * last_obj->xform() ).scaling().x();

    auto calcOne = [&]( const kvs::Vec3& initialT, const kvs::ObjectBase* p ) -> kvs::Vec3
    {
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
    };

    const kvs::Vec3 start = calcOne( startInitialTranslation, startPoint );
    const kvs::Vec3 end   = calcOne( endInitialTranslation, endPoint );

    return {
        kvs::Real32( start.x() ), kvs::Real32( start.y() ), kvs::Real32( start.z() ),
        kvs::Real32( end.x() )  , kvs::Real32( end.y() )  , kvs::Real32( end.z() )
    };
}
