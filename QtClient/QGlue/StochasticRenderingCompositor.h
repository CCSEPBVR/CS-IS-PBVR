/*****************************************************************************/
/**
 *  @file   StochasticRenderingCompositor.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#pragma once
#include <kvs/Deprecated>
#include <kvs/EnsembleAverageBuffer>
#include <kvs/Matrix44>
#include <kvs/PaintEventListener>
#include <kvs/Timer>
#include <kvs/Vector3>

#include "Scene.h"

#ifdef MR_MODE
#include "cameraimagerenderer.h"
#include "stereocamera.h"
#endif // MR_MODE

namespace kvs {
namespace jaea {
namespace pbvr {

/*===========================================================================*/
/**
 *  @brief  Stochastic rendering compositor class.
 */
/*===========================================================================*/
class StochasticRenderingCompositor : public kvs::PaintEventListener {
private:
  kvs::Timer m_timer;
  kvs::jaea::pbvr::Scene *m_scene;              ///< pointer to the scene
  size_t m_window_width;                        ///< window width
  size_t m_window_height;                       ///< window height
  size_t m_repetition_level;                    ///< repetition level
  size_t m_coarse_level;                        ///< repetition level for the coarse rendering (LOD)
  bool m_enable_lod;                            ///< flag for LOD rendering
  bool m_enable_refinement;                     ///< flag for progressive refinement rendering
  kvs::Mat4 m_object_xform;                     ///< object xform matrix used for LOD control
  kvs::Vec3 m_light_position;                   ///< light position used for LOD control
  kvs::Vec3 m_camera_position;                  ///< camera position used for LOD control
  kvs::EnsembleAverageBuffer m_ensemble_buffer; ///< ensemble averaging buffer

#ifdef MR_MODE
  bool m_enable_camera_image; // flag to draw camera image as background
  StereoCamera *m_stereo_camera;
#endif                           // MR_MODE
  int m_eye = 0;                 ///< rendering target eye. 0:left, other:right.
  int m_buffer_size[2] = {0, 0}; ///< rendering buffer size. size[2] = {width, heiht}.
#ifdef MR_MODE
#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
  CameraImageRenderer *m_camera_image_renderer = nullptr;
#endif // ENABLE_FEATURE_DRAW_CAMERA_IMAGE
#endif // MR_MODE

#ifdef ENABLE_TIME_MEASURE
  //    kvs::Timer m_timer_camera;
  //    int m_lengthTimerArray_camera;
  //    //int m_timerCount_camera;
  //    double *m_array_time_camera;

  int m_max_num_timers;
  int m_max_repetition_level;
  kvs::Timer *m_temp_timer;  // only use in engines_draw()
  double *m_temp_array_time; // only use in engines_draw()
  int m_lengthTimerArray;
  // int m_timerCount_renderer;
  double ****m_array_time;
#endif // ENABLE_TIME_MEASURE

  GLfloat m_background_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  GLfloat m_clear_depth;

public:
  StochasticRenderingCompositor(kvs::jaea::pbvr::Scene *scene);
  ~StochasticRenderingCompositor();
  const kvs::Timer &timer() const { return m_timer; }
  size_t repetitionLevel() const { return m_repetition_level; }
  bool isEnabledLODControl() const { return m_enable_lod; }
  bool isEnabledRefinement() const { return m_enable_refinement; }
  void setRepetitionLevel(const size_t repetition_level) { m_repetition_level = repetition_level; }
  void setEnabledLODControl(const bool enable) { m_enable_lod = enable; }
  void setEnabledRefinement(const bool enable) { m_enable_refinement = enable; }
  void enableLODControl() { this->setEnabledLODControl(true); }
  void enableRefinement() { this->setEnabledRefinement(true); }
  void disableLODControl() { this->setEnabledLODControl(false); }
  void disableRefinement() { this->setEnabledRefinement(false); }
  void update();

  void setRenderTarget(int eye) { m_eye = eye; }
  void setRenderBufferSize(int width, int height) {
    m_buffer_size[0] = width;
    m_buffer_size[1] = height;
  }

  void setBackgroundColor(const kvs::RGBColor& c);
  void setBackgroundColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
  void setClearDepth(GLfloat depth);

#ifdef MR_MODE
  void setEnabledCameraImage(bool enable_camera_image) { m_enable_camera_image = enable_camera_image; }
  void setStereoCamera(StereoCamera *camera) { m_stereo_camera = camera; }
  StereoCamera *stereoCamera() { return m_stereo_camera; }
  void setCameraImageRenderer(CameraImageRenderer *renderer) {
#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
    m_camera_image_renderer = renderer;
#ifdef DEBUG_TOUCH_CONTROLLER
    std::cerr << "setCameraImageRenderer() : renderer=" << renderer << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER
#endif // ENABLE_FEATURE_DRAW_CAMERA_IMAGE
  }
#endif // MR_MODE

private:
  StochasticRenderingCompositor();
  void draw();
  void check_window_created();
  void check_window_resized();
  void check_object_changed();
  kvs::Mat4 object_xform();
  void engines_create();
  void engines_update();
  void engines_setup();
  void engines_draw();

public:
  KVS_DEPRECATED(bool isEnabledShading() const) { return false; /* do not use */ }
  KVS_DEPRECATED(void setEnabledShading(const bool /* enable */)) { /* not do anything */
  }
  KVS_DEPRECATED(void enableShading()) { /* not do anything */
  }
  KVS_DEPRECATED(void disableShading()) { /* not do anything */
  }
};

} // end of namespace pbvr
} // end of namespace jaea
} // end of namespace kvs
