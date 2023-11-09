#ifndef KVSQS2creen_H
#define KVSQS2creen_H

#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif // KVS_ENABLE_GLEW

#include <QOpenGLWidget>
#include <QThread>
#include <qopenglfunctions.h>

#include <kvs/ScreenBase>
//#include <kvs/StochasticRenderingCompositor>

#include "Scene.h"
#include "StochasticRenderingCompositor.h"

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
#include "boundingbox.h"
#include <Panels/boundingboxsynchronizer.h>
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX

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
//class Screen : public kvs::ScreenBase, public QOpenGLWidget, public QOpenGLFunctions, public BoundingBoxSynchronizerClient {
class Screen : public kvs::ScreenBase, public QOpenGLWidget, public QOpenGLFunctions {

public:
  typedef kvs::ScreenBase BaseClass;
  typedef kvs::jaea::pbvr::Scene::ControlTarget ControlTarget;
  bool m_hold_paintGL = false;
  double m_fps = 0.0;

private:
  bool m_enable_default_paint_event;

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
    bool m_isEnabledBoundingBox;
//    BoundingBox *m_bounding_box;
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX

protected:
  kvs::jaea::pbvr::Scene *m_scene;
  bool m_gl_initialized;
#ifdef GPU_MODE
  kvs::jaea::pbvr::StochasticRenderingCompositor *m_compositor;
#endif

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

  void redraw() {
    this->update();
    // assert(false);
  }

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
  void updateBoundingBoxVisibility(bool isVisible);
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX

};

#endif // KVSQS2creen_H
