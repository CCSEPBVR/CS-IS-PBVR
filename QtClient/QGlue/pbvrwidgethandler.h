#ifndef KVS_JAEA_PBVR_PBVRWIDGETHANDLER_H
#define KVS_JAEA_PBVR_PBVRWIDGETHANDLER_H

#include "Panels/slidersynchronizer.h"
#include "Panels/boundingboxsynchronizer.h"
#include "widgethandler.h"

#ifdef DISP_MODE_VR
#include "headmounteddisplay.h"
#endif

namespace {}

class HeadMountedDisplay;

typedef enum {
  MOUSE_STATUS_HOVERED,
  MOUSE_STATUS_PRESSED,
  MOUSE_STATUS_DRAGGED,
  MOUSE_STATUS_RELEASED,
} MouseButtonSatus;

class PBVRWidgetHandler : public WidgetHandler, public SliderSynchronizationClient, public BoundingBoxSynchronizerClient {
private:
#ifdef DISP_MODE_VR
  HeadMountedDisplay *m_hmd;
#endif

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

#ifdef MR_MODE
  // camera image rendering on/off
  bool m_showCameraImageFlag;
#endif // MR_MODE

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
  void initializeImGui();
  void initializeStatus();

  void updateStatus();

  void drawPseudoMouseCursor();
  void drawCheckBox(char *label, bool *enable);
  void drawSliders();

public:
  PBVRWidgetHandler();
  PBVRWidgetHandler(int w, int h);

#ifdef DISP_MODE_VR
  void setHeadMountedDisplay(HeadMountedDisplay *hmd) { m_hmd = hmd; };
#endif

  void setLeftyMode(bool lefty_mode = true) { m_lefty_mode = lefty_mode; }

  bool leftyMode() { return m_lefty_mode; }

  bool initialize();
  bool destroy();
  void update();

  bool isEnabledBoundingBox() { return m_showBoundingBoxFlag; }
  bool isEnabledControlSphere() { return m_showControlSphereFlag; }
#ifdef MR_MODE
  bool isEnabledCameraImage() { return m_showCameraImageFlag; }
#endif // MR_MODE

  void updateSliders(int min, int max, int curTime, int minTime, int maxTime);
  void updateBoundingBoxVisibility(bool isVisible);
};

// singleton
PBVRWidgetHandler *PBVRWidgetHandlerInstance();

#endif // KVS_JAEA_PBVR_PBVRWIDGETHANDLER_H
