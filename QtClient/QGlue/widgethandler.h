/* Lib/WidgetHandler.h */
#pragma once
#ifndef WIDGETHANDLER_H
#define WIDGETHANDLER_H

#include <kvs/RGBAColor>

struct MouseStatus {
  bool updated;

  float cursor_dx;
  float cursor_dy;

  bool buttonPressed[3];

  MouseStatus() { clear(); }

  void clear() {
    updated = false;

    cursor_dx = 0.0f;
    cursor_dy = 0.0f;
    buttonPressed[0] = false;
    buttonPressed[1] = false;
    buttonPressed[2] = false;
  }
};

class WidgetHandler {
protected:
  int m_width;
  int m_height;

  MouseStatus m_mouse_status;

  kvs::RGBAColor background;

  //                int m_laserPoint_X;
  //                int m_laserPoint_Y;

public:
  WidgetHandler();
  WidgetHandler(int w, int h);
  WidgetHandler(int w, int h, kvs::UInt8 r, kvs::UInt8 g, kvs::UInt8 b);
  WidgetHandler(int w, int h, kvs::UInt8 r, kvs::UInt8 g, kvs::UInt8 b, kvs::UInt8 a);
  virtual bool initialize();
  virtual bool destroy();
  virtual void update();

  virtual void setMouseStatus(MouseStatus status) {
    m_mouse_status = status;
    updateStatus();
  }

  virtual void updateStatus() {}

  virtual void setBackgroundColor(kvs::RGBAColor c);
  virtual kvs::RGBAColor getBackgroundColor();
  virtual int getTextureWidth();
  virtual int getTextureHeight();

  //                virtual void setLaserPoint(int x, int y);
  //                virtual int getLaserPointX();
  //                virtual int getLaserPointY();

}; // WidgetHandler

#endif // WIDGETHANDLER_H
