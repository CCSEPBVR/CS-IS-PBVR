#ifndef VRHANDCONTROLLERLISTENER_H
#define VRHANDCONTROLLERLISTENER_H

#include <kvs/EventListener>
#include <kvs/ControllerEvent>
#include <kvs/Side>

#include <QObject>
#include <array>
#include <chrono>

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
    void drawVRPlotOverLine( kvs::Real32 coordArray[ 2 * 3 ] );

    void vrSharePoint( kvs::Real32 coordArray[ 2 * 3 ], kvs::Real32 directionArray[ 3 ] );

private:
    kvs::qt::jaea::Screen* m_screen = nullptr;

    static constexpr double k_long_press_sec = 0.5;

    static constexpr size_t k_side_max = kvs::Side::Max;
    static constexpr size_t k_button_max  = kvs::Controller::Button::Max;

    std::array<std::array<bool, k_button_max>, k_side_max> m_down{};
    std::array<std::array<bool, k_button_max>, k_side_max> m_longFired{};
    std::array<std::array<std::chrono::steady_clock::time_point, k_button_max>, k_side_max> m_downAt{};

private:
    std::array<kvs::Real32, 6> makeCoordArrayForLine(
        kvs::Scene* scene,
        const kvs::Vec3& startInitialTranslation,
        const kvs::Vec3& endInitialTranslation,
        const kvs::ObjectBase* startPoint,
        const kvs::ObjectBase* endPoint ) const;
};

#endif // VRHANDCONTROLLERLISTENER_H
