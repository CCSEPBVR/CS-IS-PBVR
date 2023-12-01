#pragma once
#include "StochasticLineRenderer.h"
#include "StochasticPolygonRenderer.h"
#include "controllerbase.h"
#include "headmounteddisplay.h"
#include "lineobjectproxy.h"
#include "polygonobjectproxy.h"
#include "stochastictexturedpolygonrenderer.h"
#include "texturedpolygonobjectproxy.h"
#include "widgethandler.h"

#include "TexturedPolygonObject.h"

#include <kvs/LineObject>
#include <kvs/PointObject>
#include <kvs/PolygonObject>
#include <kvs/PolygonRenderer>
#include <kvs/Timer>

// #include <kvs/Scene>

// #include <kvs/VolumeObjectBase>
// #include <kvs/ArrowGlyph>
// #include <kvs/LineRenderer>

#include <map>
#include <set>

// #define DISABLE_VR_HAND

class TexturedPolygonObject;
class StochasticTexturedPolygonRenderer;

class TouchController : public ControllerBase {
private:
  // added 20210111
  bool m_is_first_frame;

  // bool m_is_grabbed;
  bool m_both_is_grabbed;

  bool m_button_a_pressed;
  bool m_button_b_pressed;
  bool m_button_x_pressed;
  bool m_button_y_pressed;

  float m_touch_distance;
  float m_rotation_factor;
  float m_translation_factor;
  float m_scaling_factor;
  kvs::Vec3 m_rot_start_pos_right;

  kvs::Vec3 m_prev_left_p;
  kvs::Vec3 m_prev_right_p;
  kvs::Vec3 m_start_left_p;
  kvs::Vec3 m_start_right_p;

  kvs::Vec3 m_initial_head_p;

  WidgetHandler *m_widget_handler;

#ifdef ENABLE_FEATURE_DRAW_VR_HANDS
#ifndef DISABLE_VR_HAND
  // vr hands
  static PolygonObjectProxy m_vr_hands;
  static int m_vr_hands_id;
  kvs::jaea::pbvr::StochasticPolygonRenderer *m_vr_hands_renderer;
#endif // DISABLE_VR_HAND
#endif // ENABLE_FEATURE_DRAW_VR_HANDS

#ifdef ENABLE_FEATURE_DRAW_CONTROL_SPHERE
  // control sphere
  static LineObjectProxy m_control_sphere;
  static int m_control_sphere_id;
  kvs::jaea::pbvr::StochasticLineRenderer *m_control_sphere_renderer;
#endif // ENABLE_FEATURE_DRAW_CONTROL_SPHERE

public:
  // default values for rescale submesh of bounding box
  constexpr static kvs::Real32 DEAFULT_SCALING_RATIO = 1.0f;
  constexpr static kvs::Real32 DEFAULT_THRESHOLD_MIN = 1.0f;
  constexpr static kvs::Real32 DEFAULT_THRESHOLD_MAX = 2.0;

private:
  MouseStatus getMouseStatus();

protected:
  virtual void rebuildControlSphere(bool force_rebuild);

public:
  TouchController(HeadMountedDisplay &hmd, kvs::jaea::pbvr::Scene *scene);
  ~TouchController();

  float rotationFactor() const { return m_rotation_factor; }
  float translationFactor() const { return m_translation_factor; }
  float scalingFactor() const { return m_scaling_factor; }
  void setRotationFactor(const float factor) { m_rotation_factor = factor; }
  void setTranslationFactor(const float factor) { m_translation_factor = factor; }
  void setScalingFactor(const float factor) { m_scaling_factor = factor; }

  void setWidgetHandler(WidgetHandler *handler) { m_widget_handler = handler; }

  virtual void initializeEvent();
  virtual void frameEvent();
};
