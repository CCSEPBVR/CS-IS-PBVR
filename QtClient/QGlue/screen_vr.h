#ifndef KVSQS2creen_H
#define KVSQS2creen_H

#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include <Panels/renderoptionpanel.h>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QThread>
#include <kvs/ScreenBase>
#include <kvs/Xform>

#include <OVR_CAPI.h>

#include "Scene.h"
#include "StochasticRenderingCompositor.h"
#include "headmounteddisplay.h"
#include "pbvrwidgethandler.h"
#include "stereocamera.h"
#include "touchcontroller.h"

#ifdef MR_MODE
#include "cameraimagerenderer.h"
#endif // MR_MODE

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
#include "boundingbox.h"
#include <Panels/boundingboxsynchronizer.h>
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX

class TouchController;
class BoundingBox;

namespace kvs {
namespace jaea {
namespace pbvr {
class Scene;
} // namespace pbvr
} // namespace jaea
} // namespace kvs

/**
 * @brief The Screen class, this class is designed as a combined and simplified version
 *        of the kvs::qt::ScreenBase, and   kvs::qt::Screen. This class uses Qt events directly
 *        and does not translate QtEvents to kvs::event class.
 *
 *        This class is desgined to work with  QOpenGLWidget as a baseclass, whereas
 *        in kvs 2.9.0, QOpenGLWidget is not supported.
 *
 *        It does however require a version of KVS which includes commit
 *        https://github.com/naohisas/KVS/commit/4e3a36473cc0b7cc1418e49b949e5f8d62d8c489
 */
class Screen : public kvs::ScreenBase, public QOpenGLWidget, public QOpenGLFunctions, public BoundingBoxSynchronizerClient {

public:
  static const kvs::Vec3 KVS_CAMERA_INITIAL_POSITION;
  static const kvs::Vec3 KVS_CAMERA_INITIAL_LOOK_AT;
  static const kvs::Vec3 KVS_CAMERA_INITIAL_UP;

  typedef kvs::ScreenBase BaseClass;
  //    typedef kvs::Scene::ControlTarget ControlTarget;
  bool m_hold_paintGL = false;
  double m_fps = 0.0;

  enum ManipulatorUseFlag {
    UseNoManipulator = 0,
    UseLeftManipulator,
    UseRightManipulator,
  };
  //shimomura 20240128
  float* m_Glyph_Coord; //3 componennt xyz
//  float* getGlyphXform(){return m_touch_controller-> getglyphxform(); }
  void setGlyphXform(const kvs::Xform glyphxform ){ m_GlyphXform = glyphxform;}
  kvs::Xform GlyphXform(){return m_GlyphXform;}
  void setObjectManagerXform(const kvs::Xform objectManagerxform ){ m_ObjectManagerXform = objectManagerxform;}
  kvs::Xform ObjectManagerXform(){return m_ObjectManagerXform;}

  void setGlyphCoord(float* Glyph_Coord){ m_Glyph_Coord = Glyph_Coord;}
  float* GlyphCoord() {return m_Glyph_Coord;}


private:
  bool m_enable_default_paint_event;

  // from OculusKVS (kvs::jaea::pbvr::Screen)
  HeadMountedDisplay m_hmd; ///< Oculus HMD
  TouchController *m_touch_controller;
  PBVRWidgetHandler *m_widget_handler;
  kvs::Xform m_GlyphXform;
  kvs::Xform m_ObjectManagerXform;

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
  bool m_isEnabledBoundingBox;
  BoundingBox *m_bounding_box;
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX

  // Rectangle size for stereo camera image
  struct GLScreenCoordinates {
    float left, up, right, down;
  } m_screenCoord;

#ifdef ENABLE_TIME_MEASURE
  int m_lengthTimerArray_paintGL;
  // int m_timerCount_paintGL;
  double *m_array_time_paintGL;
#endif // ENABLE_TIME_MEASURE

protected:
  kvs::jaea::pbvr::Scene *m_scene;
  bool m_gl_initialized;
  kvs::jaea::pbvr::StochasticRenderingCompositor *m_compositor;
#ifdef MR_MODE
  StereoCamera *m_zed;
  CameraImageRenderer m_cameraImageRenderer;
#endif // MR_MODE

#ifdef DISP_MODE_VR
  bool m_is_enabled_grab_framebuffer = false;
  QImage m_grabbed_image;
  QImage grabImage(int width, int height);
#endif

protected:
  virtual void defaultPaintEvent();

public:
  Screen(QWidget *parent_surface = NULL);
  ~Screen();
  kvs::jaea::pbvr::Scene *scene() { return m_scene; }

  void setPosition(const int x, const int y);
  void setGeometry(const int x, const int y, const int width, const int height);

  void paintGL();
  void resizeGL(int width, int height);
  void initializeGL();

  virtual void onPaintGL() {}
  virtual void onResizeGL(int width, int height) {}
  virtual void onInitializeGL() {}

  void update() {
    if (QThread::currentThread() != this->thread()) {
      // Called from different thread do nothing
      return;
    }
    QOpenGLWidget::update();
  }
  void redraw() { this->update(); }
  void setWidgetHandler(PBVRWidgetHandler *handler);

  void checkOpenGLError (const std::string& prefix);

#ifdef DISP_MODE_VR
  void enableGrabFramebuffer();
  QImage getGrabbedFramebuffer();
#endif

  // const HeadMountedDisplay& headMountedDisplay() const { return m_hmd; }
  // void setMirrorBufferTypeToLeftEyeImage() { m_hmd.setMirrorBufferTypeToLeftEyeImage(); }
  // void setMirrorBufferTypeToRightEyeImage() { m_hmd.setMirrorBufferTypeToRightEyeImage(); }
  // void setMirrorBufferTypeToBothEyeImage() { m_hmd.setMirrorBufferTypeToBothEyeImage(); }
  // void setMirrorBufferTypeToDistortedBothEyeImage() { m_hmd.setMirrorBufferTypeToDistortedBothEyeImage(); }

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
  void updateBoundingBoxVisibility(bool isVisible) { m_isEnabledBoundingBox = isVisible; }
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX
};

#endif // KVSQS2creen_H
