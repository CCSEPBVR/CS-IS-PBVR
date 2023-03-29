#ifndef KVS_OCULUS_JAEA_PBVR_PBVRWIDGETHANDLER_H
#define KVS_OCULUS_JAEA_PBVR_PBVRWIDGETHANDLER_H

#include "widgethandler.h"
#include "Panels/slidersynchronizer.h"

namespace {
}

namespace kvs {
namespace oculus {
namespace jaea {
class HeadMountedDisplay;
namespace pbvr {
typedef enum {
    MOUSE_STATUS_HOVERED,
    MOUSE_STATUS_PRESSED,
    MOUSE_STATUS_DRAGGED,
    MOUSE_STATUS_RELEASED,
} MouseButtonSatus;

class PBVRWidgetHandler : public kvs::oculus::jaea::WidgetHandler, public SliderSynchronizationClient
{
private:
    kvs::oculus::jaea::HeadMountedDisplay *m_hmd;

    // pseudo mouse cursor
    float m_cursorPositionX;
    float m_cursorPositionY;

    bool m_lefty_mode; // true=lefty mode(x=click, right stiock=move) / false=righty mode(a=click, left stick=move)

    // mouse button status
    bool m_mouse_button_pushed_curr;
    bool m_mouse_button_pushed_prev;
    MouseButtonSatus m_mouse_button_status;

    // bounding box on/off button
    bool m_showBoundingBoxFlag;

    // control sphere on/off button
    bool m_showControlSphereFlag;

    // time step sliders
    // time slider : slider_val[1] <= slider_val[0] <= slider_val[2]
    // min slider : slider_min <= slider_val[1] <= slider_val[2]
    // time slider : slider_val[1] <= slider_val[2] <= slider_max
    int slider_min;
    int slider_max;
    int slider_val[3];

    // singleton
    static PBVRWidgetHandler *s_instance;

private:
    void initializeImGui ();
    void initializeStatus ();

    void updateStatus ();

    void drawPseudoMouseCursor();
    void drawCheckBox (char *label, bool *enable);
    void drawSliders ();

public:
    PBVRWidgetHandler();
    PBVRWidgetHandler (int w, int h);

    void setHeadMountedDisplay (kvs::oculus::jaea::HeadMountedDisplay *hmd) {
        m_hmd = hmd;
    };

    void setLeftyMode (bool lefty_mode=true) {
        m_lefty_mode = lefty_mode;
    }

    bool leftyMode() { return m_lefty_mode; }

    bool initialize();
    bool destroy();
    void update();

    bool isEnabledBoundingBox () {return m_showBoundingBoxFlag;}
    bool isEnabledControlSphere () {return m_showControlSphereFlag;}

    void updateSliders(int min, int max, int curTime, int minTime, int maxTime);
};

} // namespace pbvr
} // namespace jaea
} // namespace oculus
} // namespace kvs


// singleton
kvs::oculus::jaea::pbvr::PBVRWidgetHandler *PBVRWidgetHandlerInstance();

#endif // KVS_OCULUS_JAEA_PBVR_PBVRWIDGETHANDLER_H
