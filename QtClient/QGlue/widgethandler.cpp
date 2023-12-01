#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "widgethandler.h"

#include <kvs/RGBAColor>

WidgetHandler::WidgetHandler() : m_width(512), m_height(512), background(0, 0, 0, 0) {}

WidgetHandler::WidgetHandler(int w, int h) : m_width(w), m_height(h), background(0, 0, 0, 0) {}

WidgetHandler::WidgetHandler(int w, int h, kvs::UInt8 r, kvs::UInt8 g, kvs::UInt8 b) : m_width(w), m_height(h), background(r, g, b, 0) {}

WidgetHandler::WidgetHandler(int w, int h, kvs::UInt8 r, kvs::UInt8 g, kvs::UInt8 b, kvs::UInt8 a)
    : m_width(w), m_height(h), background(r, g, b, a) {}

// void WidgetHandler::setLaserPoint(int x, int y)
//{
//     m_laserPoint_X = x;
//     m_laserPoint_Y = y;
// }
// int WidgetHandler::getLaserPointX()
//{
//     return m_laserPoint_X;
// }
// int WidgetHandler::getLaserPointY()
//{
//     return m_laserPoint_Y;
// }

void WidgetHandler::setBackgroundColor(kvs::RGBAColor c) { background = c; }
kvs::RGBAColor WidgetHandler::getBackgroundColor() { return background; }
int WidgetHandler::getTextureWidth() { return m_width; }
int WidgetHandler::getTextureHeight() { return m_height; }

bool WidgetHandler::initialize() { return true; }
bool WidgetHandler::destroy() { return true; }

void WidgetHandler::update() {}
