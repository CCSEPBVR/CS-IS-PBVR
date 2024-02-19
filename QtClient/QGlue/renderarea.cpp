#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "renderarea.h"

#include "Client/KeyFrameAnimation.h"
#include "Client/timer_simple.h"
#include "Client/v3defines.h"
#include "Panels/animationcontrols.h"
#include <QDebug>
#include <QKeyEvent>

// KVS2.7.0
// ADD BY)T.Osaki 2020.06.04
#include <kvs/Background>

#include "objnameutil.h"

#include "Client/LogManager.h"
#ifdef DISP_MODE_VR
#include "objectscale.h"
#endif // DISP_MODE_VR

#ifdef PBVR_DEBUG
int timestep = 0;
#endif

char RenderArea::shadinglevel[256] = "";
kvs::visclient::TimerEvent *RenderArea::g_timer_event = 0;

RenderArea::PointObjectProxy RenderArea::m_point_object;
RenderArea::RenderArea(QWidget *parent_surface) : m_obj_id_pair(-1, -1) {

  Q_UNUSED(parent_surface);
  //    pixelRatio=QPaintDevice::devicePixelRatioF();
  //    pixelRatio=2;
  pixelRatio = parent_surface->devicePixelRatio();

  this->m_scene->background()->setColor(kvs::RGBAColor(0, 0, 22, 1.0f));
  this->i_w = 620; // Qthis->width();
  this->i_h = 620; // Qthis->height();
  this->setSize(i_w, i_h);
  std::cout << "RenderArea::RenderArea" << std::endl;

  m_stepLabel = new QGlue::StepLabel(this, extCommand);
  m_stepLabel->setPosition(150 * pixelRatio, 50);
  m_labels.append(m_stepLabel);

  int size = 90 * pixelRatio;
  m_orientation_axis = new QGlue::OrientationAxis(this);
  m_orientation_axis->setPosition(ScreenBase::width() - (size + 10), 10);
  m_orientation_axis->setBoxType(QGlue::OrientationAxis::SolidBox);
  m_orientation_axis->setSize(size);
  m_orientation_axis->show();

  // Setup Legend bar
  g_legend = new QGlue::LegendBar(this, *extCommand);
  g_legend->setOrientation(QGlue::LegendBar::Horizontal);
  g_legend->setPosition(10, ScreenBase::height() - 10);
  g_legend->setWidth(50 * pixelRatio);
  g_legend->setHeight(180 * pixelRatio);

  g_legend->screenResizedAfterSelectTransferFunction(1);
  g_legend->show();

#ifdef USE_DUMMY_CAMERA_FOR_COMM
  m_dummy_camera = kvs::Camera();
#endif // USE_DUMMY_CAMERA_FOR_COMM
}

RenderArea::~RenderArea(void) { m_scene->removeObject(m_obj_id_pair.first, false, false); }

/**
 * @brief RenderArea::enableRendererShading, enable render shading
 */
void RenderArea::enableRendererShading() {
#ifdef ENABLE_FEATURE_DRAW_PARTICLE
  m_renderer.enableShading();
#endif // ENABLE_FEATURE_DRAW_PARTICLE
}
/**
 * @brief RenderArea::storeCurrentXForm stores the current ObjectManager XForm
 */
void RenderArea::storeCurrentXForm() { m_stored_xf = m_scene->objectManager()->xform(); }
/**
 * @brief RenderArea::restoreXForm restores the saved xform
 */
void RenderArea::restoreXForm() {
  // Restore Saved X form
  m_scene->objectManager()->translate(m_stored_xf.translation());
  m_scene->objectManager()->scale(m_stored_xf.scaling());
  m_scene->objectManager()->rotate(m_stored_xf.rotation());
}

/**
 * @brief RenderArea::setRenderRepetitionLevel, set render repetition level.
 * @param level
 */
void RenderArea::setRenderRepetitionLevel(int level) {
  level = level > 1 ? level : 1;
  //    if(level != m_renderer.getRepetitionLevel()){
  m_renderer.setRepetitionLevel(level);
#ifdef GPU_MODE
  m_compositor->setRepetitionLevel(level);
#endif
  // Replacement of renderer is required to get correct output with
  // kvs::glsl::ParticleBasedRenderer when changing repetition level after the
  // the renderers first render function call. (kvs 2.9.0)
  m_scene->replaceRenderer(m_obj_id_pair.second, m_renderer.pbr_pointer(), false);

  // The lines below are a work around for m_initial_modelview problem that occurs
  // when replacing the renderer while zoom is applied.
  // in kvs::glsl::ParticleBasedRenderer.  (kvs 2.9.0)
  makeCurrent();
  // Save current X form
  storeCurrentXForm();
  // Reset object Manager's XForm
  m_scene->objectManager()->resetXform();
  // Manually call  paint function, to set m_initial_xform in Renderer
#ifdef CPU_MODE
  m_scene->paintFunction();
#endif
#ifdef GPU_MODE
  m_compositor->update();
#endif
  // Restore xform to original scene xform
  restoreXForm();
  // Paint the frame again, to erase previous frame
#ifdef CPU_MODE
  m_scene->paintFunction();
#endif
#ifdef GPU_MODE
  m_compositor->update();
#endif
  doneCurrent();
  //    }
}

/**
 * @brief RenderArea::setRenderSubPixelLevel, set render sub pixel level.
 *                    When this is called in GPU mode level will be squared before applied.
 *                    When this is called in GPU mode, renderer instance will be recreated and replaced in scene.
 * @param level
 */
void RenderArea::setRenderSubPixelLevel(int level) {
  level = level > 1 ? level : 1;
  setRenderRepetitionLevel(level * level);
}

/**
 * @brief RenderArea::getPointObjectXform, get point object transform from private active point object
 * @return
 */
kvs::Xform RenderArea::getPointObjectXform() {
#ifdef ENABLE_FEATURE_DRAW_PARTICLE
  return m_point_object->xform();
#else
  return kvs::Xform();
#endif // ENABLE_FEATURE_DRAW_PARTICLE
}
/**
 * @brief RenderArea::setPointObjectXform, set point object transfor of private active point object.
 * @param xf
 */
void RenderArea::setPointObjectXform(kvs::Xform xf) {
#ifdef ENABLE_FEATURE_DRAW_PARTICLE
  m_point_object->setXform(xf);
#endif // ENABLE_FEATURE_DRAW_PARTICLE
}

/**
 * @brief RenderArea::updateCommandInfo
 * @param extCommand
 */
void RenderArea::updateCommandInfo(ExtCommand *extCommand) {

#ifdef DISP_MODE_VR
#ifdef USE_DUMMY_CAMERA_FOR_COMM
  m_dummy_camera = *(this->scene()->camera());
  kvs::Vec3 pos(Screen::KVS_CAMERA_INITIAL_POSITION.x()/OBJECT_SCALING_FACTOR, Screen::KVS_CAMERA_INITIAL_POSITION.y()/OBJECT_SCALING_FACTOR, Screen::KVS_CAMERA_INITIAL_POSITION.z()/OBJECT_SCALING_FACTOR);
  m_dummy_camera.setPosition(pos);

  extCommand->m_parameter.m_camera = &m_dummy_camera;
#else // USE_DUMMY_CAMERA_FOR_COMM
  // KVS2.7.0
  // MOD BY)T.Osaki 2020.07.20
  extCommand->m_parameter.m_camera = this->m_scene->camera();
#endif // USE_DUMMY_CAMERA_FOR_COMM
#else // DISP_MODE_VR
  // KVS2.7.0
  // MOD BY)T.Osaki 2020.07.20
  extCommand->m_parameter.m_camera = this->m_scene->camera();
#endif // DISP_MODE_VR

  // Setup Controll Object
  kvs::PointObject *object1 = extCommand->m_detailed_particles[0];
#ifdef ENABLE_FEATURE_DRAW_PARTICLE
  const kvs::Vector3f &min = object1->minObjectCoord();
  const kvs::Vector3f &max = object1->maxObjectCoord();
  m_point_object->setMinMaxObjectCoords(min, max);
  m_point_object->setMinMaxExternalCoords(min, max);
#endif // ENABLE_FEATURE_DRAW_PARTICLE

  // Setup Extended Particle Volume Renderer
  int sp_level = extCommand->m_parameter.m_detailed_subpixel_level;
  setRenderSubPixelLevel(sp_level);

#ifdef DISP_MODE_2D
#ifdef ENABLE_FEATURE_DRAW_PARTICLE // Setup FPS Label
  m_fpsLabel = new QGlue::FPSLabel(this, *(m_renderer.pbr_pointer()));
  m_fpsLabel->setPosition(50 * pixelRatio, 50);
  m_labels.append(m_fpsLabel);
#endif // ENABLE_FEATURE_DRAW_PARTICLE
#endif // DISP_MODE_2D

  m_renderer.setShadingString(shadinglevel);
  Screen::update();
  this->setFocus();
}

/**
 * @brief RenderArea::onInitializeGL, overrides virtual onInitializeGL of PBVR Screen class.
 *                     PBVR Screen will call onInitializeGL when initializeGL is successful.
 */
void RenderArea::onInitializeGL(void) {
  qInfo("KVSRenderArea::initializeGL( void )");
#ifdef DISP_MODE_2D
  m_orientation_axis->initializeOpenGLFunctions();
#endif // DISP_MODE_2D
  g_legend->initializeOpenGLFunctions();
  this->setAutoFillBackground(false);

  m_scene->light()->on();
  m_scene->mouse()->attachCamera(m_scene->camera());

  m_point_object.swap();
  m_point_object->setName(OBJ_NAME_PARTICLE);
  m_point_object.swap();
  m_point_object->setName(OBJ_NAME_PARTICLE);
}
/**
 * @brief RenderArea::onResizeGL, GL Surface resized handler overrides PBVR Screen onResizeGL.
 *                               PBVR Screen will call onResizeGL at the end of Screen::resizeGL
 * @param w int width
 * @param h int height
 */
void RenderArea::onResizeGL(int w, int h) {
  //    MOD BY)T.Osaki 2020.04.28
  float scale = QPaintDevice::devicePixelRatioF();
  int h_scaled = h * scale;
  int w_scaled = w * scale;

#ifdef DISP_MODE_2D
  m_stepLabel->setPosition(150 * scale, 50);
  if (m_fpsLabel)
    m_fpsLabel->setPosition(50 * scale, 50);
  int size = 90 * scale;
  m_orientation_axis->setPosition(w_scaled - (size + 10), 10);
#endif // DISP_MODE_2D
  g_legend->setPosition(10, h_scaled - 10);
}
/**
 * @brief RenderArea::onPaintGL GL paint handler overrides PBVR Screen onPaintGL.
 *                               PBVR Screen will call onResizeGL at the end of Screen::paintGL
 */
void RenderArea::onPaintGL(void) {
#ifdef DISP_MODE_2D
  if (m_orientation_axis) {
    glPushMatrix();
    m_orientation_axis->paintEvent();
    glPopMatrix();
  }
  this->drawLabelList(m_labels, QColor(0, 0, 0, 255));

  if (g_legend) {
    glPushMatrix();
    g_legend->paintEvent();
    glPopMatrix();
  }
#endif // DISP_MODE_2D
}

/**
 * @brief RenderArea::setupEventHandlers,
 */
void RenderArea::setupEventHandlers() {

  std::string interval("TIMER_EVENT_INTERVAL");

  if (getenv(interval.c_str()) != NULL) {
    msec = std::atoi(getenv(interval.c_str()));
  } else {
    msec = DEFAULT_MSEC;
  }

  // Timer

  qt_timer = new QGlue::Timer(msec, extCommand);
  //    qt_timer->setRenderer(this->m_renderer);

  extCommand->m_glut_timer = qt_timer;
  //    qt_timer->setScreen(this);

  g_timer_event = new kvs::visclient::TimerEvent(extCommand, &extCommand->comthread);
  g_timer_event->setScreen(this);
  qt_timer->setEventListener(g_timer_event);
  qt_timer->start();
}
/**
 * @brief RenderArea::attachPointObject
 *        Attaches a new point to the scene, using the apropriate registerObject, or
 *        replaceObject, depending on wether a prevous object has been registered.
 *
 *        Ensures context current by explict makeCurrent() call.
 *
 *        Creates local copy of point object, to ensure life time.
 * @param point, the point to be attached
 */
void RenderArea::attachPointObject(const kvs::PointObject *point, int sp_level) {
    bool single_data = false;
  if (QThread::currentThread() != this->thread()) {
    qWarning("RenderArea::attachPointObject was not called from main thread, ignoring point object");
    return;
  }
  if(extCommand->las_registerd == false)
  {
      if (point == NULL || point->coords().size() == 0) {
          qCritical("RenderArea::attachPointObject. !!!!!!! NULL OR ZERO OBJECT - INTERPRETED AS CLEAR !!!!!! t\n");
          m_scene->removeObject(m_obj_id_pair.first, false, false);
          m_obj_id_pair.first = -1;
          return;
      }
  }
  else
  {
      single_data = true;
  }
  if (point->coords().size() <= 3) {
    qCritical("RenderArea::attachPointObject.  PointObject with single point attached t\n");
  }
  m_point_object.swap();
  m_point_object->clear();
  m_point_object->add(*point);
  // Make sure context is current
  makeCurrent();
  // Update subpixel/renderrepetitoion level
  this->setRenderRepetitionLevel(sp_level * sp_level);

  if (m_obj_id_pair.first == -1 && m_obj_id_pair.second == -1) {

    // {
    //     float xform_prev[16];
    //     m_scene->objectManager()->xform().toArray(xform_prev);
    //     std::cerr << "RenderArea::attachPointObject(): register new object & renderer" << std::endl;
    //     std::cerr << "before apply:" << std::endl;
    //     std::cerr << " | " << xform_prev[0*4+0] << "  " << xform_prev[0*4+1] << "  " << xform_prev[0*4+2] << "  " << xform_prev[0*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[1*4+0] << "  " << xform_prev[1*4+1] << "  " << xform_prev[1*4+2] << "  " << xform_prev[1*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[2*4+0] << "  " << xform_prev[2*4+1] << "  " << xform_prev[2*4+2] << "  " << xform_prev[2*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[3*4+0] << "  " << xform_prev[3*4+1] << "  " << xform_prev[3*4+2] << "  " << xform_prev[3*4+3] << std::endl;
    // }

    m_obj_id_pair = this->m_scene->registerObject(m_point_object, m_renderer.pbr_pointer());
#ifdef DISP_MODE_VR
    m_scene->objectManager()->scaleObject(m_point_object, kvs::Vec3(OBJECT_SCALING_FACTOR, OBJECT_SCALING_FACTOR, OBJECT_SCALING_FACTOR));
#endif // DISP_MODE_VR

    // {
    //     float xform_prev[16];
    //     m_scene->objectManager()->xform().toArray(xform_prev);
    //     std::cerr << "RenderArea::attachPointObject(): register new object & renderer" << std::endl;
    //     std::cerr << "after apply:" << std::endl;
    //     std::cerr << " | " << xform_prev[0*4+0] << "  " << xform_prev[0*4+1] << "  " << xform_prev[0*4+2] << "  " << xform_prev[0*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[1*4+0] << "  " << xform_prev[1*4+1] << "  " << xform_prev[1*4+2] << "  " << xform_prev[1*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[2*4+0] << "  " << xform_prev[2*4+1] << "  " << xform_prev[2*4+2] << "  " << xform_prev[2*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[3*4+0] << "  " << xform_prev[3*4+1] << "  " << xform_prev[3*4+2] << "  " << xform_prev[3*4+3] << std::endl;
    // }

  } else if (m_obj_id_pair.first == -1) {

    // {
    //     float xform_prev[16];
    //     m_scene->objectManager()->xform().toArray(xform_prev);
    //     std::cerr << "RenderArea::attachPointObject(): register new object" << std::endl;
    //     std::cerr << "before apply:" << std::endl;
    //     std::cerr << " | " << xform_prev[0*4+0] << "  " << xform_prev[0*4+1] << "  " << xform_prev[0*4+2] << "  " << xform_prev[0*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[1*4+0] << "  " << xform_prev[1*4+1] << "  " << xform_prev[1*4+2] << "  " << xform_prev[1*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[2*4+0] << "  " << xform_prev[2*4+1] << "  " << xform_prev[2*4+2] << "  " << xform_prev[2*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[3*4+0] << "  " << xform_prev[3*4+1] << "  " << xform_prev[3*4+2] << "  " << xform_prev[3*4+3] << std::endl;
    // }

    m_obj_id_pair = m_scene->registerObject(m_point_object, m_renderer.pbr_pointer());
#ifdef DISP_MODE_VR
    m_scene->objectManager()->scaleObject(m_point_object, kvs::Vec3(OBJECT_SCALING_FACTOR, OBJECT_SCALING_FACTOR, OBJECT_SCALING_FACTOR));
#endif // DISP_MODE_VR

    // {
    //     float xform_prev[16];
    //     m_scene->objectManager()->xform().toArray(xform_prev);
    //     std::cerr << "RenderArea::attachPointObject(): register new object " << std::endl;
    //     std::cerr << "after apply:" << std::endl;
    //     std::cerr << " | " << xform_prev[0*4+0] << "  " << xform_prev[0*4+1] << "  " << xform_prev[0*4+2] << "  " << xform_prev[0*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[1*4+0] << "  " << xform_prev[1*4+1] << "  " << xform_prev[1*4+2] << "  " << xform_prev[1*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[2*4+0] << "  " << xform_prev[2*4+1] << "  " << xform_prev[2*4+2] << "  " << xform_prev[2*4+3] << std::endl;
    //     std::cerr << " | " << xform_prev[3*4+0] << "  " << xform_prev[3*4+1] << "  " << xform_prev[3*4+2] << "  " << xform_prev[3*4+3] << std::endl;
    // }

  } else {
#ifdef GPU_MODE
//        m_renderer.updateModelView();
#endif
    this->m_scene->replaceObject(m_obj_id_pair.first, m_point_object, false);
  }
#ifdef DISP_MODE_2D
  if(single_data)
  {
  }
  else
  {
      m_orientation_axis->setObject(m_point_object);
  }
#endif // DISP_MODE_2D
  if(single_data)
  {
  }
  else
  {
      m_scene->objectManager()->resetXform();
  }
  // Let Qt know we are done using the context.
  doneCurrent();
  // Trigger a paint event
  this->update();
}

/**
 * @brief RenderArea::setCoordinateBoundaries
 * @param crd
 */
void RenderArea::setCoordinateBoundaries(float crd[6]) {
#ifdef ENABLE_FEATURE_DRAW_PARTICLE
  kvs::Vector3f min_t(crd[0], crd[1], crd[2]);
  kvs::Vector3f max_t(crd[3], crd[4], crd[5]);
  m_point_object->setMinMaxObjectCoords(min_t, max_t);
  m_point_object->setMinMaxExternalCoords(min_t, max_t);
#endif // ENABLE_FEATURE_DRAW_PARTICLE


  std::cout << this->m_scene->objectManager()->minObjectCoord() << std::endl;
  std::cout << this->m_scene->objectManager()->maxObjectCoord() << std::endl;
  std::cout << this->m_scene->objectManager()->minExternalCoord() << std::endl;
  std::cout << this->m_scene->objectManager()->maxExternalCoord() << std::endl;
  std::cout << __LINE__ << std::endl;
  this->m_scene->objectManager()->updateExternalCoords();
  this->m_scene->objectManager()->setMinMaxObjectCoords( kvs::Vec3(-0.00951057,-0.01,-0.01),kvs::Vec3(0.00951057,0.01,0.01));
  std::cout << this->m_scene->objectManager()->minObjectCoord() << std::endl;
  std::cout << this->m_scene->objectManager()->maxObjectCoord() << std::endl;
  std::cout << this->m_scene->objectManager()->minExternalCoord() << std::endl;
  std::cout << this->m_scene->objectManager()->maxExternalCoord() << std::endl;
  //    if(m_reset_count == 0){
  //        m_reset_count++;
  //    }else{
  // #ifdef COMM_MODE_CS
  //        this->m_scene->reset();
  // #endif
  //    }
  std::cout << " !!!!!!!!!!!!!!!!!!! Reset Viewer Scale !!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
}

/**
 * @brief RenderArea::drawLabelList, draws the label list
 * @param list
 * @param fontColor
 */
void RenderArea::drawLabelList(QList<QGlue::Label *> list, QColor fontColor) {
  const float front = 0.0;
  const float back = 1.0;
  const float left = 0.0f;
  const float bottom = 0.0f;
  const float right = 300.0f;
  const float top = 300.0f;

  glPushMatrix();
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glViewport(0, 0, 300, 300);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(left, right, bottom, top, front, back);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  for (int i = 0; i < list.size(); ++i) {
    QGlue::Label *item = list.at(i);
    item->contentUpdate();
    item->renderBitMap(item->x(), item->y(), 0.0);
  }
  glPopAttrib();
  glPopMatrix();
}

/**
 * @brief RenderArea::setLabelFont
 * @param f
 */
void RenderArea::setLabelFont(const QFont &f) {
#ifdef DISP_MODE_2D
  for (int i = 0; i < m_labels.size(); ++i) {
    QGlue::Label *item = m_labels.at(i);
    item->setFont(f);
  }
#endif // DISP_MODE_2D
  g_legend->setFont(f);
}

/**
 * @brief RenderArea::keyPressEvent
 * @param kbEvent
 */
void RenderArea::keyPressEvent(QKeyEvent *kbEvent) {

    // unicode() returns const QChar*. QChar::unicode returns ushort
//    ushort ucode=
    bool reset = false;
    if(kbEvent->key()==Qt::Key_Home)
    {
        reset = true;
    }
    unicodeCheck(kbEvent->text().unicode()->unicode(), reset);
//    qInfo()<<"@@@ KVSRenderArea::keyPressEvent :"<<ucode;
}

void RenderArea::unicodeCheck(ushort ucode, bool reset)
{
    LogManager lg;
    switch (ucode)
    {
    case kvs::Key::l:
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
        m_scene->controlTarget() = m_scene->TargetLight;
        if( m_isRec )
        {
            lg.Write("renderarea.cpp",__func__,"unicode",std::to_string(ucode));
        }
        break;
    case kvs::Key::c:
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
        m_scene->controlTarget() = m_scene->TargetCamera;
        if( m_isRec )
        {
            lg.Write("renderarea.cpp",__func__,"unicode",std::to_string(ucode));
        }
        break;
    case kvs::Key::o:
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
        m_scene->controlTarget() = m_scene->TargetObject;
        if( m_isRec )
        {
            lg.Write("renderarea.cpp",__func__,"unicode",std::to_string(ucode));
        }
        break;

    case kvs::Key::x:
        //2020,11,27 T.Osaki sceneが持っているObjectManagerのXfromを使用する。
        qInfo(" [debug] 'x' pressed. (add Xform)");
        //        KeyFrameAnimationAdd(this->getPointObjectXform());
        KeyFrameAnimationAdd(this->scene()->objectManager()->xform());
        if( m_isRec )
        {
            lg.Write("renderarea.cpp",__func__,"unicode",std::to_string(ucode) + "," + this->m_scene->objectManager()->xform().toMatrix().format(",",false));
        }
        break;
    case kvs::Key::d:
        qInfo(" [debug] 'd' pressed. (delete last Xform)");
        KeyFrameAnimationErase();
        if( m_isRec )
        {
            lg.Write("renderarea.cpp",__func__,"unicode",std::to_string(ucode));
        }
        break;
    case kvs::Key::D:
        qInfo(" [debug] 'D' pressed. (delete all Xform)");
        KeyFrameAnimationDelete();
        if( m_isRec )
        {
            lg.Write("renderarea.cpp",__func__,"unicode",std::to_string(ucode));
        }
        break;
    case kvs::Key::M:
        qInfo(" [debug] 'M' pressed. (toggle animation with Xform)");
        /* KeyFrameAnimationStart(); */
        KeyFrameAnimationToggle();
        if( m_isRec )
        {
            lg.Write("renderarea.cpp",__func__,"unicode",std::to_string(ucode));
        }
        break;
        /* case kvs::Key::m: */
        /*    printf(" [debug] 'm' pressed. (stop animation with Xform)"); */
        /*    KeyFrameAnimationStop(); */
        /*    break; */
    case kvs::Key::S:
        qInfo(" [debug] 'S' pressed. (write Xform to file)");
        KeyFrameAnimationWrite();
        if( m_isRec )
        {
            lg.Write("renderarea.cpp",__func__,"unicode",std::to_string(ucode));
        }
        break;
    case kvs::Key::L:
        qInfo(" [debug] 'F' pressed. (read Xform from file)");
        KeyFrameAnimationRead();
        if( m_isRec )
        {
            lg.Write("renderarea.cpp",__func__,"unicode",std::to_string(ucode));
        }
        break;
    case kvs::Key::G:
//        extCommand->toggleGlyph();
        std::cout << "-----------------------------------" << std::endl;
        std::cout << "[ObjectManager]" << std::endl;
        std::cout << *(this->scene()->objectManager()) << std::endl;
        std::cout << "[Translation]" << std::endl;
        std::cout << this->scene()->objectManager()->xform().translation() << std::endl;
        std::cout << "[Scaling]" << std::endl;
        std::cout << this->scene()->objectManager()->xform().scaling() << std::endl;
        std::cout << "[Rotation]" << std::endl;
        std::cout << this->scene()->objectManager()->xform().rotation() << std::endl;
        std::cout << "[min object coord]" << std::endl;
        std::cout << this->scene()->objectManager()->minObjectCoord() << std::endl;
        std::cout << "[max object coord]" << std::endl;
        std::cout << this->scene()->objectManager()->maxObjectCoord() << std::endl;
        std::cout << "[min object external coord]" << std::endl;
        std::cout << this->scene()->objectManager()->minExternalCoord() << std::endl;
        std::cout << "[max object external coord]" << std::endl;
        std::cout << this->scene()->objectManager()->maxExternalCoord() << std::endl;
        std::cout << "-----------------------------------" << std::endl;
        std::cout << "[Object]" << std::endl;
        std::cout << *(this->scene()->object()) << std::endl;
        std::cout << "[Translation]" << std::endl;
        std::cout << this->scene()->object()->xform().translation() << std::endl;
        std::cout << "[Scaling]" << std::endl;
        std::cout << this->scene()->object()->xform().scaling() << std::endl;
        std::cout << "[Rotation]" << std::endl;
        std::cout << this->scene()->object()->xform().rotation() << std::endl;
        std::cout << "[min object coord]" << std::endl;
        std::cout << this->scene()->object()->minObjectCoord() << std::endl;
        std::cout << "[max object coord]" << std::endl;
        std::cout << this->scene()->object()->maxObjectCoord() << std::endl;
        std::cout << "[min object external coord]" << std::endl;
        std::cout << this->scene()->object()->minExternalCoord() << std::endl;
        std::cout << "[max object external coord]" << std::endl;
        std::cout << this->scene()->object()->maxExternalCoord() << std::endl;
        std::cout << "-----------------------------------" << std::endl;
//        std::cout << "GLYPH" << std::endl;
//        std::cout << this->scene()->object("GLYPH")->minObjectCoord() << std::endl;
//        std::cout << this->scene()->object("GLYPH")->maxObjectCoord() << std::endl;
//        std::cout << this->scene()->object("GLYPH")->minExternalCoord() << std::endl;
//        std::cout << this->scene()->object("GLYPH")->maxExternalCoord() << std::endl;
        break;
    default:
        break;
    }
    if(reset){
        qInfo(" [debug] 'HOME' pressed. (Reset the viewer)");
        m_scene->reset();
        //MOD BY)T.Osaki 2020.06.29
        //kvs::ScreenBase::reset();
        this->update();
        if( m_isRec )
        {
        lg.Write("renderarea.cpp",__func__,"unicode","reset");
        }
    }
}

/**
 * @brief RenderArea::ScreenShot
 * @param screen
 * @param tstep
 */
// MOD BY)T.Osaki 2020.06.29
// void    RenderArea::ScreenShot( kvs::ScreenBase* screen, const int tstep )
void RenderArea::ScreenShot(kvs::jaea::pbvr::Scene *scene, const int tstep) {

  std::stringstream step;
  AnimationControlParameter ac_params = AnimationControls::getParameters();
#ifndef _TIMER_
  if ((ac_params.m_capture == PBVR_ON) && (ac_params.m_mode == PBVR_IMAGE)) {
#endif
    PBVR_TIMER_STA(160);

    // KVS2.7.0
    // MOD BY)T.Osaki 2020.07.20
    scene->screen()->redraw();

    step << '.' << std::setw(5) << std::setfill('0') << tstep;

#ifdef PBVR_DEBUG
    std::string filename = AnimationControls::getImageFileName(step.str(), ".bmp");
#else
  std::string filename = ac_params.m_image_file + step.str() + ".bmp";
#endif

    //        kvs::ColorImage image( screen->camera()->snapshot() );
    //        image.write( filename.c_str() );
    QImage image = extCommand->m_screen->grabFramebuffer();
    image.save(QString(filename.c_str()));

#ifdef PBVR_DEBUG
    std::cout << "Snapshot >>" << filename << std::endl;
    timestep++;
    if (timestep > 10)
      timestep = 0;
#endif
    PBVR_TIMER_END(160);
#ifndef _TIMER_
  }
#endif
}

/**
 * @brief RenderArea::ScreenShotKeyFrame
 * @param screen
 * @param tstep
 */
// MOD BY)T.Osaki 2020.06.29
// void    RenderArea::ScreenShotKeyFrame( kvs::ScreenBase* screen, const int tstep )
void RenderArea::ScreenShotKeyFrame(kvs::jaea::pbvr::Scene *scene, const int tstep) {

  std::stringstream step;
  AnimationControlParameter ac_params = AnimationControls::getParameters();
#ifndef _TIMER_
  if ((ac_params.m_capture == PBVR_ON) && (ac_params.m_mode == PBVR_IMAGE)) {
#endif
    PBVR_TIMER_STA(160);

#ifdef DISP_MODE_VR
    extCommand->m_screen->enableGrabFramebuffer();
#endif

    // KVS2.7.0
    // MOD BY)T.Osaki 2020.07.20
    scene->screen()->redraw();

    step << '.' << std::setw(6) << std::setfill('0') << tstep;

#ifdef PBVR_DEBUG
    std::string filename = AnimationControls::getImageFileName("_k" + step.str(), ".bmp");
#else
  std::string filename = ac_params.m_image_file + "_k" + step.str() + ".bmp";
#endif

#ifdef DISP_MODE_2D
    QImage image = extCommand->m_screen->grabFramebuffer();
#endif
#ifdef DISP_MODE_VR
    QImage image = extCommand->m_screen->getGrabbedFramebuffer();
#endif
    image.save(QString(filename.c_str()));

    PBVR_TIMER_END(160);
#ifndef _TIMER_
  }
#endif
}

/**
 * @brief RenderArea::mousePressEvent
 * @param event
 */
void RenderArea::mousePressEvent(QMouseEvent *event) {

  //    ADD BY)T.Osaki 2020.03.03
  QWidget::setFocus();
  bool MOD_Shift = event->modifiers() & Qt::SHIFT;
  bool MOD_Ctrl = event->modifiers() & Qt::CTRL;

  bool BTN_LEFT = event->buttons() & Qt::LeftButton;
  bool BTN_RIGHT = event->buttons() & Qt::RightButton;
  bool BTN_MID = event->buttons() & Qt::MiddleButton;

  int mode = -1;

  if ((BTN_LEFT && MOD_Ctrl) || BTN_RIGHT) {
    mode = kvs::Mouse::Translation;
  } else if ((BTN_LEFT && MOD_Shift) || BTN_MID) {
    mode = kvs::Mouse::Scaling;
  } else if (BTN_LEFT) {
    mode = kvs::Mouse::Rotation;
    // KVS2.7.0
    // MOD BY)T.Osaki 2020.06.04
    makeCurrent();
    m_scene->updateCenterOfRotation();
    doneCurrent();
    // kvs::ScreenBase::updateCenterOfRotation();
  }
  const int x = event->x() * pixelRatio;
  const int y = event->y() * pixelRatio;
  if (mode >= 0) {

    // KVS2.7.0
    // MOD BY)T.Osaki 2020.06.04
    kvs::Mouse::OperationMode kvsMode = (kvs::Mouse::OperationMode)mode;
    m_scene->mousePressFunction(x, y, kvsMode);
    // kvs::Mouse::TransMode kvsMode=(kvs::Mouse::TransMode)mode;
    // kvs::ScreenBase::mousePressFunction(x,y,kvsMode );
  }
  std::cout << "mousePressEvent::" << mode << " " << x << "," << y << std::endl;
  update();
}

/**
 * @brief RenderArea::mouseMoveEvent
 * @param event
 */
void RenderArea::mouseMoveEvent(QMouseEvent *event) {
  //    std::cout<<"mouseMoveEvent"<<std::endl;
  //    BaseClass::eventHandler()->notify( event );
  // KVS2.7.0
  // MOD BY)T.Osaki 2020.06.04
  // if( kvs::ScreenBase::controlTarget() == kvs::ScreenBase::TargetObject )
  if (m_scene->controlTarget() == m_scene->TargetObject)
    ;
  {
    // if( !kvs::ScreenBase::objectManager()->isEnableAllMove() )
    if (!m_scene->isEnabledObjectOperation()) {
      // if( !kvs::ScreenBase::objectManager()->hasActiveObject() )
      if (!m_scene->objectManager()->hasActiveObject()) {
        return;
      }
    }
  }

#ifdef DISP_MODE_2D
  const int x = event->x() * pixelRatio;
  const int y = event->y() * pixelRatio;
#endif // DISP_MODE_2D
#ifdef DISP_MODE_VR
  int x = event->x() * pixelRatio;
  int y = event->y() * pixelRatio;
  if (m_scene->mouse()->operationMode() == kvs::Mouse::Scaling) {
    x *= 10;
    y *= 10;
  }
#endif // DISP_MODE_VR

  // KVS2.7.0
  // MOD BY)T.Osaki 2020.06.04
  m_scene->mouseMoveFunction(x, y);
  // kvs::ScreenBase::mouseMoveFunction(x,y );
  update();
}

/**
 * @brief RenderArea::mouseReleaseEvent
 * @param event
 */
void RenderArea::mouseReleaseEvent(QMouseEvent *event) {
  std::cout << "mouseReleaseEvent" << std::endl;
  //    BaseClass::eventHandler()->notify( event );
  // KVS2.7.0
  // MOD BY)T.Osaki 2020.06.04
  m_scene->mouseReleaseFunction(event->x(), event->y());
  if(m_scene->hasObjects())
  {
      if( m_isRec )
      {
          LogManager lg;
          lg.Write("renderarea.cpp",__func__,"xform",this->m_scene->object(4)->xform().toMatrix().format(",",false));
      }
  }
}

/**
 * @brief RenderArea::setSize
 * @param width
 * @param height
 */
void RenderArea::setSize(const int width, const int height) {
  int h_scaled = height * pixelRatio;
  int w_scaled = width * pixelRatio;

  kvs::ScreenBase::setSize(w_scaled, h_scaled);
  // KVS2.7.0
  // MOD BY)T.Osaki 2020.06.04
  if (m_scene->camera())
    m_scene->camera()->setWindowSize(w_scaled, h_scaled);
  if (m_scene->mouse())
    m_scene->camera()->setWindowSize(w_scaled, h_scaled);
  // if ( kvs::ScreenBase::camera() ) kvs::ScreenBase::camera()->setWindowSize( w_scaled, h_scaled);
  // if ( kvs::ScreenBase::mouse()  ) kvs::ScreenBase::mouse()->setWindowSize( w_scaled, h_scaled);
  QOpenGLWidget::resize(w_scaled, h_scaled);
  this->update();
}

/**
 * @brief RenderArea::setGeometry
 * @param geom
 */
void RenderArea::setGeometry(QRect geom) {
  QOpenGLWidget::setGeometry(geom);
  this->setMinimumSize(geom.width(), geom.height());
  this->setMaximumSize(geom.width(), geom.height());
  this->setSize(geom.width(), geom.height());
  this->update();
}

/**
 * @brief RenderArea::animation_add, helper for toolbar button
 */
void RenderArea::animation_add() { KeyFrameAnimationAdd(this->scene()->objectManager()->xform()); }
/**
 * @brief RenderArea::animation_del, helper for toolbar button
 */
void RenderArea::animation_del() { KeyFrameAnimationErase(); }
/**
 * @brief RenderArea::animation_play, helper for toolbar button
 */
void RenderArea::animation_play() { KeyFrameAnimationStart(); }

void RenderArea::logPlay(std::string funcName, std::string str[])
{
    if(funcName.compare("unicodeCheck") == 0)
    {
        if(str[0] == "reset")
        {
            unicodeCheck(0,true);
        }else if(str[0] == "120"){
            kvs::Mat4 matrix;
            matrix.set(std::stod(str[1]),std::stod(str[2]),std::stod(str[3]),std::stod(str[4]),
                       std::stod(str[5]),std::stod(str[6]),std::stod(str[7]),std::stod(str[8]),
                       std::stod(str[9]),std::stod(str[10]),std::stod(str[11]),std::stod(str[12]),
                       std::stod(str[13]),std::stod(str[14]),std::stod(str[15]),std::stod(str[16]));
            kvs::Xform xform(matrix);
            KeyFrameAnimationAdd(xform);

        }else{
            unicodeCheck(std::stoul(str[0]),false);
        }
    }
//        std::cout << "::" << m_scene->IDManager()->size() << std::endl;
    if(funcName.compare("mouseReleaseEvent") == 0)
    {
        kvs::Mat4 matrix;
        matrix.set(std::stod(str[0]),std::stod(str[1]),std::stod(str[2]),std::stod(str[3]),
                   std::stod(str[4]),std::stod(str[5]),std::stod(str[6]),std::stod(str[7]),
                   std::stod(str[8]),std::stod(str[9]),std::stod(str[10]),std::stod(str[11]),
                   std::stod(str[12]),std::stod(str[13]),std::stod(str[14]),std::stod(str[15]));
        kvs::Xform xform(matrix);
        if(m_scene->hasObjects())
        {
//            m_scene->object()->setXform(xform);
            this->scene()->object(4)->setXform(xform);
            this->update();
        }
    }
}
/**
 * @brief RenderArea::switch_gpu, helper for toolbar button
 */
// void RenderArea::switch_gpu(bool f)
//{
//     if (f){
//         m_renderer.use_gpu();
//     }
//     else{
//         m_renderer.use_cpu();
//     }
//     m_scene->replaceRenderer(m_obj_id_pair.second, m_renderer.pbr_pointer(),false);
//     // The lines below are a work around for m_initial_modelview problem
//     // in kvs::glsl::ParticleBasedRenderer.
//     makeCurrent();
//     // Save current X form
//     storeCurrentXForm();
//     // Reset object Manager's XForm
//     m_scene->objectManager()->resetXform();
//     // Manually call  paint function, to set m_initial_xform in Renderer
////    m_scene->paintFunction();
//    m_compositor->update();
//    // Restore xform to original scene xform
//    restoreXForm();
//    // Paint the frame again, to erase previous frame
////    m_scene->paintFunction();
//    m_compositor->update();
//    doneCurrent();
//    update();
//}

// void RenderArea::switch_gpu(bool f)
//{
//     if (f){
//         m_renderer.use_gpu();
//     }
//     else{
//         m_renderer.use_cpu();
//     }
//     m_scene->replaceRenderer(m_obj_id_pair.second, m_renderer.pbr_pointer(),false);
//     // The lines below are a work around for m_initial_modelview problem
//     // in kvs::glsl::ParticleBasedRenderer.
//     makeCurrent();
//     // Save current X form
//     storeCurrentXForm();
//     // Reset object Manager's XForm
//     m_scene->objectManager()->resetXform();
//     // Manually call  paint function, to set m_initial_xform in Renderer
//     m_scene->paintFunction();
//     // Restore xform to original scene xform
//     restoreXForm();
//     // Paint the frame again, to erase previous frame
//     m_scene->paintFunction();
//     doneCurrent();
//     update();
// }
