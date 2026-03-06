#ifndef VRHANDCONTROLLERLISTENER_H
#define VRHANDCONTROLLERLISTENER_H
#include <array>
#include <chrono>
#include <algorithm>

#include <kvs/EventListener>
#include <kvs/ControllerEvent>
#include <kvs/Side>
#include <kvs/Scene>
#include <kvs/ObjectBase>
#include <kvs/Vector>
#include <kvs/Xform>
#include <kvs/Matrix33>

#include <QObject>

#include "Screen.h"

class VRHandControllerListener : public QObject, public kvs::EventListener
{
    Q_OBJECT

public:
    explicit VRHandControllerListener( kvs::qt::jaea::Screen* screen );
    ~VRHandControllerListener();

    void onEvent( kvs::EventBase* event ) override;

signals:
    void toggleShowHideVRPlotOverLine();
    void toggleShowHideVRPlotOverTime();
    void toggleShowHideSharePoint( kvs::Real32 coordArray[ 2 * 3 ], kvs::Real32 directionArray[ 2 * 3 ] );

    void drawVRPlotOverLine( kvs::Real32 coordArray[ 2 * 3 ] );
    void drawVRPlotOverTime( kvs::Real32 coordArray[ 2 * 3 ] );
    void drawVRSharePoint( kvs::Real32 coordArray[ 2 * 3 ], kvs::Real32 directionArray[ 2 * 3 ] );

private:
    kvs::qt::jaea::Screen* m_screen = nullptr;

    static constexpr double k_long_press_sec = 0.5;

    static constexpr size_t k_side_max   = kvs::Side::Max;
    static constexpr size_t k_button_max = kvs::Controller::Button::Max;

    std::array<std::array<bool, k_button_max>, k_side_max> m_down{};
    std::array<std::array<bool, k_button_max>, k_side_max> m_long_fired{};
    std::array<std::array<std::chrono::steady_clock::time_point, k_button_max>, k_side_max> m_down_at{};

private:
    void handleLongPress( kvs::UInt32 button, const kvs::Controller::ControllerStatus& cs, kvs::UInt32 side );
    void handleShortRelease( kvs::UInt32 button, const kvs::Controller::ControllerStatus& cs, kvs::UInt32 side );

    kvs::Vec3 calculateCoord( kvs::Scene* scene, const kvs::Vec3& initialT, const kvs::ObjectBase* p ) const;
    kvs::Vec3 controllerForward( const kvs::Xform& walkthrough, const kvs::Xform& controller_local ) const;
};

#endif // VRHANDCONTROLLERLISTENER_H
