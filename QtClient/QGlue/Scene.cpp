/*****************************************************************************/
/**
 *  @file   Scene.cpp
 *  @author Insight, Inc.
 */
/*****************************************************************************/
#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "Scene.h"
#include "SphereGlyph.h"
#include <cmath>
#include <iostream>
#include <kvs/Background>
#include <kvs/CubicImage>
#include <kvs/IDManager>
#include <kvs/ObjectBase>
#include <kvs/OpenGL>
#include <kvs/RendererManager>
#include <kvs/ScreenBase>
#include <kvs/VisualizationPipeline>
#include <kvs/Xform>
#include <vector>

#include "objnameutil.h"

#ifdef DISP_MODE_2D
//#include "screen.h"
#else
#include "objectscale.h"
#include "screen_vr.h"
#endif

namespace {

inline kvs::Vec3 World2Camera(const kvs::Vec3 &p, const kvs::Camera *camera) {
  return kvs::jaea::pbvr::WorldCoordinate(p).toCameraCoordinate(camera).position();
}

inline kvs::Vec2 Object2Window(const kvs::Vec3 &p, const kvs::Camera *camera) {
  const float w = camera->windowWidth();
  const float h = camera->windowHeight();
  const kvs::Xform pvm(camera->projectionMatrix() * kvs::OpenGL::ModelViewMatrix());
  const kvs::Vec3 p_cam = pvm.project(p);
  return (kvs::Vec2(1, 1) + p_cam.xy()) * kvs::Vec2(w, h) * 0.5f;
}

} // namespace

namespace kvs {
namespace jaea {
namespace pbvr {
/*===========================================================================*/
/**
 *  @brief  Constructs a new Scene class.
 */
/*===========================================================================*/
Scene::Scene(kvs::ScreenBase *screen)
    : //    kvs::Scene( screen ),
      m_screen(screen), m_target(Scene::TargetObject), m_enable_object_operation(true), m_enable_collision_detection(false),
    m_scaling_ratio_accum(1.0), m_before_right(kvs::Vec3(0,0,0)) {
#ifdef DEBUG_SCENE
  std::cout << "kvs::jaea::pbvr::Scene constructed." << std::endl;
#endif // DEBUG_SCENE

  m_camera = new kvs::Camera();
  m_light = new kvs::Light();
  m_mouse = new kvs::Mouse();
  m_background = new kvs::Background(kvs::RGBColor(212, 221, 229));
  m_object_manager = new kvs::jaea::pbvr::ObjectManager();
  m_renderer_manager = new kvs::RendererManager();
  m_id_manager = new kvs::IDManager();
}

// copy from kvs::Scene

/*===========================================================================*/
/**
 *  @brief  Destroys the Scene class.
 */
/*===========================================================================*/
Scene::~Scene() {
  delete m_camera;
  delete m_light;
  delete m_mouse;
  delete m_background;
  delete m_object_manager;
  delete m_renderer_manager;
  delete m_id_manager;
}

/*===========================================================================*/
/**
 *  @brief  Registers an object with a renderer.
 *  @param  object [in] pointer to the object
 *  @param  renderer [in] pointer to the renderer
 *  @return Pair of IDs (object ID and renderer ID)
 */
/*===========================================================================*/
const std::pair<int, int> Scene::registerObject(kvs::ObjectBase *object, kvs::RendererBase *renderer) {
  // If the given pointer to the renderer is null, a renderer for the given
  // object is automatically created by using visualization pipeline class.
  if (!renderer) {
    kvs::VisualizationPipeline pipeline(object);
    if (!pipeline.exec()) {
      kvsMessageError("Cannot create a renderer for the given object.");
      return std::pair<int, int>(-1, -1);
    }

    renderer = const_cast<kvs::RendererBase *>(pipeline.renderer());
  }

  if (!object->hasMinMaxObjectCoords()) {
    object->updateMinMaxCoords();
  }

  // If the object has not been registered in the object managet,
  // the object is registered and then its ID is returned.
  int object_id = m_object_manager->objectID(object);
  if (object_id == -1)
    object_id = m_object_manager->insert(object);

  // If the renderer has not been registered in the renderer managet,
  // the renderer is registered and then its ID is returned.
  int renderer_id = m_renderer_manager->rendererID(renderer);
  if (renderer_id == -1)
    renderer_id = m_renderer_manager->insert(renderer);

  // Insert the IDs into the ID manager.
  m_id_manager->insert(object_id, renderer_id);
  renderer->setScreen(this->screen());

  return std::pair<int, int>(object_id, renderer_id);
}

/*===========================================================================*/
/**
 *  @brief  Removes the object specified by the given object ID.
 *  @param  object_id [in] object ID
 *  @param  delete_object [in] if true, the object will be deleted
 *  @param  delete_renderer [in] if true, the renderers for the object will be deleted
 */
/*===========================================================================*/
void Scene::removeObject(int object_id, bool delete_object, bool delete_renderer) {
  // Remove the object specified by the given object ID for the object manager.
  m_object_manager->erase(object_id, delete_object);

  // Remove the all of renderers assigned for the specified object from
  // the renderer manager.
  const std::vector<int> renderer_ids = m_id_manager->rendererID(object_id);
  for (size_t i = 0; i < renderer_ids.size(); i++) {
    m_renderer_manager->erase(renderer_ids[i], delete_renderer);
  }

  // Remove IDs specified by the given object ID from the ID manager.
  m_id_manager->eraseByObjectID(object_id);
}

/*===========================================================================*/
/**
 *  @brief  Removes the object specified by the given object name.
 *  @param  object_name [in] object name
 *  @param  delete_object [in] if true, the registered object will be deleted
 *  @param  delete_renderer [in] if true, the registered renderers for the object will be deleted
 */
/*===========================================================================*/
void Scene::removeObject(std::string object_name, bool delete_object, bool delete_renderer) {
  const kvs::ObjectBase *object = m_object_manager->object(object_name);
  const int object_id = m_object_manager->objectID(object);
  this->removeObject(object_id, delete_object, delete_renderer);
}

/*===========================================================================*/
/**
 *  @brief  Replaces the object specified by the given object ID with the input object.
 *  @param  object_id [in] object ID
 *  @param  object [in] pointer to the object will be registered
 *  @param  delete_object [in] if true, the registered object will be deleted
 */
/*===========================================================================*/
void Scene::replaceObject(int object_id, kvs::ObjectBase *object, bool delete_object) { m_object_manager->change(object_id, object, delete_object); }

/*===========================================================================*/
/**
 *  @brief  Replaces the object specified by the given object name with the input object.
 *  @param  object_id [in] object name
 *  @param  object [in] pointer to the object will be registered
 *  @param  delete_object [in] if true, the registered object will be deleted
 */
/*===========================================================================*/
void Scene::replaceObject(std::string object_name, kvs::ObjectBase *object, bool delete_object) {
  m_object_manager->change(object_name, object, delete_object);
}

/*===========================================================================*/
/**
 *  @brief  Replaces the renderer specified by the given renderer ID with the input renderer.
 *  @param  renderer_id [in] renderer ID
 *  @param  renderer [in] pointer to the renderer will be registered
 *  @param  delete_renderer [in] if true, the registered renderer will be deleted
 */
/*===========================================================================*/
void Scene::replaceRenderer(int renderer_id, kvs::RendererBase *renderer, bool delete_renderer) {
  m_renderer_manager->change(renderer_id, renderer, delete_renderer);
}

/*===========================================================================*/
/**
 *  @brief  Replaces the renderer specified by the given renderer name with the input renderer.
 *  @param  renderer_name [in] renderer name
 *  @param  renderer [in] pointer to the renderer will be registered
 *  @param  delete_renderer [in] if true, the registered renderer will be deleted
 */
/*===========================================================================*/
void Scene::replaceRenderer(std::string renderer_name, kvs::RendererBase *renderer, bool delete_renderer) {
  m_renderer_manager->change(renderer_name, renderer, delete_renderer);
}

/*===========================================================================*/
/**
 *  @brief  Returns number of objects contained in the scene.
 *  @return number of objects
 */
/*===========================================================================*/
int Scene::numberOfObjects() const { return m_object_manager->numberOfObjects(); }

/*===========================================================================*/
/**
 *  @brief  Return true if the scene has one or more objects.
 *  @return true if the scene has one or more objects
 */
/*===========================================================================*/
bool Scene::hasObjects() const { return m_object_manager->hasObject(); }

/*===========================================================================*/
/**
 *  @brief  Return true if the scene has the object specified by given ID.
 *  @param  id [in] object ID
 *  @return true if the scene has the object
 */
/*===========================================================================*/
bool Scene::hasObject(int id) const { return m_object_manager->object(id) != NULL; }

/*===========================================================================*/
/**
 *  @brief  Return true if the scene has the object specified by given name.
 *  @param  name [in] object name
 *  @return true if the scene has the object
 */
/*===========================================================================*/
bool Scene::hasObject(std::string name) const { return m_object_manager->object(name) != NULL; }

/*===========================================================================*/
/**
 *  @brief  Return pointer to the object specified by given ID.
 *  @param  id [in] object ID
 *  @return pointer to the object
 */
/*===========================================================================*/
kvs::ObjectBase *Scene::object(int id) {
  if (id < 0) {
    return m_object_manager->object();
  }
  return m_object_manager->object(id);
}

/*===========================================================================*/
/**
 *  @brief  Return pointer to the object specified by given name.
 *  @param  name [in] object name
 *  @return pointer to the object
 */
/*===========================================================================*/
kvs::ObjectBase *Scene::object(std::string name) { return m_object_manager->object(name); }

/*===========================================================================*/
/**
 *  @brief  Return pointer to the renderer specified by given ID.
 *  @param  id [in] renderer ID
 *  @return pointer to the renderer
 */
/*===========================================================================*/
kvs::RendererBase *Scene::renderer(int id) {
  if (id < 0) {
    return m_renderer_manager->renderer();
  }
  return m_renderer_manager->renderer(id);
}

/*===========================================================================*/
/**
 *  @brief  Return pointer to the renderer specified by given name.
 *  @param  name [in] renderer name
 *  @return pointer to the renderer
 */
/*===========================================================================*/
kvs::RendererBase *Scene::renderer(std::string name) { return m_renderer_manager->renderer(name); }

/*===========================================================================*/
/**
 *  @brief  Resets the x-form matrix of the basic screen components.
 */
/*===========================================================================*/
void Scene::reset() {
  // Reset viewpoint to the initial position.
  m_mouse->reset();

  // Reset the xform of the object.
  if (m_object_manager->hasActiveObject()) {
    m_object_manager->resetActiveObjectXform();
  } else {
    m_object_manager->resetXform();
  }

  // Reset the xform of the camera.
  m_camera->resetXform();

  // Reset the xform of the light.
  m_light->resetXform();
}

/*==========================================================================*/
/**
 *  @brief  Test whether the screen is the active move mode.
 *  @param  x [in] x coordinate value of the mouse cursor position
 *  @param  y [in] y coordinate value of the mouse cursor position
 *  @return true, if the screen is the active move mode
 */
/*==========================================================================*/
bool Scene::isActiveMove(int x, int y) {
  if (!m_object_manager->hasObject())
    return true;

  if (m_target == TargetObject) {
    if (!m_enable_object_operation && m_enable_collision_detection) {
      const float px = static_cast<float>(x);
      const float py = static_cast<float>(y);
      return this->detect_collision(kvs::Vec2(px, py));
    }
  }

  return true;
}

/*==========================================================================*/
/**
 *  @brief  Updates the controlling object.
 */
/*==========================================================================*/
void Scene::updateControllingObject() {
  if (m_target == TargetObject) {
    if (m_enable_object_operation) {
      m_object_manager->releaseActiveObject();
    }
  }
}

/*==========================================================================*/
/**
 *  @brief  Updates the center of rotation.
 */
/*==========================================================================*/
void Scene::updateCenterOfRotation() {
  // Center of rotation in the device coordinate system.
  kvs::Vec2 center(0.0, 0.0);

  switch (m_target) {
  case TargetCamera:
  case TargetLight:
    // Get an at-point of the camera, which is the center of rotation,
    // in the device coord.
    center = m_camera->lookAtInDevice();
    break;
  case TargetObject:
    if (m_enable_object_operation || !m_object_manager->hasObject()) {
      // The center of rotation will be set to the gravity of the object
      // manager in device coordinates.
      center = this->position_in_device();
    } else {
      // If the object manager has no active object, the center
      // of rotation is not updated.
      if (!m_object_manager->hasActiveObject())
        return;

      // The center of rotation will be set to the gravity of the object
      // in device coordinates.
      kvs::ObjectBase *object = m_object_manager->activeObject();
      center = this->position_in_device(object);
    }
    break;
  default:
    break;
  }

  m_mouse->trackball().setRotationCenter(center);
}

/*==========================================================================*/
/**
 *  @brief  Updates the xform of the control target.
 */
/*==========================================================================*/
void Scene::updateXform() {
  switch (m_target) {
  case TargetCamera:
    this->updateXform(m_camera);
    break;
  case TargetLight:
    this->updateXform(m_light);
    break;
  case TargetObject:
    this->updateXform(m_object_manager);
    break;
  default:
    break;
  }
}

/*==========================================================================*/
/**
 *  @brief  Updates the xform of the object manager.
 *  @param  manager [in] pointer to the object manager
 */
/*==========================================================================*/
void Scene::updateXform(kvs::jaea::pbvr::ObjectManager *manager) {
  switch (m_mouse->operationMode()) {
  case kvs::Mouse::Rotation:
    manager->rotate(m_mouse->trackball().rotation().toMatrix());
    break;
  case kvs::Mouse::Translation:
    manager->translate(m_mouse->trackball().translation());
    break;
  case kvs::Mouse::Scaling:
    manager->scale(m_mouse->trackball().scaling());
    break;
  default:
    break;
  }
}

/*==========================================================================*/
/**
 *  @brief  Updates the xform of the camera.
 *  @param  camera [in] pointer to the camera
 */
/*==========================================================================*/
void Scene::updateXform(kvs::Camera *camera) {
  switch (m_mouse->operationMode()) {
  case kvs::Mouse::Rotation:
    camera->rotate(m_mouse->trackball().rotation().toMatrix().transposed());
    break;
  case kvs::Mouse::Translation:
    camera->translate(-m_mouse->trackball().translation());
    break;
  case kvs::Mouse::Scaling: {
    const kvs::Vec3 s = m_mouse->trackball().scaling();
    camera->scale(kvs::Vec3(1 / s.x(), 1 / s.y(), 1 / s.z()));
    break;
  }
  default:
    break;
  }
}

/*==========================================================================*/
/**
 *  @brief  Updates the xform of the light.
 *  @param  light [in] pointer to the light
 */
/*==========================================================================*/
void Scene::updateXform(kvs::Light *light) {
  switch (m_mouse->operationMode()) {
  case kvs::Mouse::Rotation:
    light->rotate(m_mouse->trackball().rotation().toMatrix());
    break;
  case kvs::Mouse::Translation:
    light->translate(m_mouse->trackball().translation());
    break;
  case kvs::Mouse::Scaling:
    light->scale(m_mouse->trackball().scaling());
    break;
  default:
    break;
  }
}

/*===========================================================================*/
/**
 *  @brief  Updates the modeling matrinx for the object.
 *  @param  object [in] pointer to the object
 */
/*===========================================================================*/
void Scene::updateGLModelingMatrix(const kvs::ObjectBase *object) const {
  float X[16];
  object->xform().toArray(X);
  kvs::OpenGL::SetMatrixMode(GL_MODELVIEW);
  kvs::OpenGL::MultMatrix(X);
}

/*===========================================================================*/
/**
 *  @brief  Updates the modeling matrinx for the object manager.
 *  @param  object [in] pointer to the object
 */
/*===========================================================================*/
void Scene::updateGLModelingMatrix() const {
  float X[16];
  m_object_manager->xform().toArray(X);
  kvs::OpenGL::SetMatrixMode(GL_MODELVIEW);
  kvs::OpenGL::MultMatrix(X);
}

/*===========================================================================*/
/**
 *  @brief  Updates the viewing matrinx.
 */
/*===========================================================================*/
void Scene::updateGLViewingMatrix() const {
  float m[16];
  kvs::OpenGL::SetMatrixMode(GL_MODELVIEW);
  kvs::OpenGL::LoadIdentity();
  kvs::Xform v(m_camera->viewingMatrix());
  v.toArray(m);
  kvs::OpenGL::MultMatrix(m);
}

/*===========================================================================*/
/**
 *  @brief  Updates the projection matrinx.
 */
/*===========================================================================*/
void Scene::updateGLProjectionMatrix() const {
  float m[16];
  kvs::OpenGL::SetMatrixMode(GL_PROJECTION);
  kvs::OpenGL::LoadIdentity();
  kvs::Xform p(m_camera->projectionMatrix());
  p.toArray(m);
  kvs::OpenGL::MultMatrix(m);
}

/*===========================================================================*/
/**
 *  @brief  Updates the light source parameters.
 */
/*===========================================================================*/
void Scene::updateGLLightParameters() const {
  const kvs::Vec3 p = ::World2Camera(m_light->position(), m_camera);
  const kvs::Vec4 position(p, 1.0f);
  const kvs::Vec4 diffuse(m_light->diffuse(), 1.0f);
  const kvs::Vec4 ambient(m_light->ambient(), 1.0f);
  const kvs::Vec4 specular(m_light->specular(), 1.0f);

  kvs::OpenGL::PushMatrix();
  kvs::OpenGL::LoadIdentity();
  kvs::OpenGL::SetLight(m_light->id(), GL_POSITION, (GLfloat *)&(position[0]));
  kvs::OpenGL::SetLight(m_light->id(), GL_DIFFUSE, (GLfloat *)&(diffuse[0]));
  kvs::OpenGL::SetLight(m_light->id(), GL_AMBIENT, (GLfloat *)&(ambient[0]));
  kvs::OpenGL::SetLight(m_light->id(), GL_SPECULAR, (GLfloat *)&(specular[0]));
  kvs::OpenGL::PopMatrix();
}

/*===========================================================================*/
/**
 *  @brief  Returns cube map image generated from 6 direction images.
 *  @return cube map image
 */
/*===========================================================================*/
kvs::CubicImage Scene::cubemap() {
  kvs::CubicImage cube_map;

  const float c_fov = this->camera()->fieldOfView();
  const float c_front = this->camera()->front();
  const kvs::Vec3 c_at = this->camera()->lookAt();
  const kvs::Vec3 c_up = this->camera()->upVector();
  const kvs::Vec3 c_p = this->camera()->position();
  const kvs::Vec3 l_p = this->light()->position();
  {
    this->camera()->setFieldOfView(90.0);
    this->camera()->setFront(0.1);
    for (size_t i = 0; i < kvs::CubicImage::NumberOfDirections; i++) {
      const kvs::CubicImage::Direction dir = kvs::CubicImage::Direction(i);
      const kvs::Vec3 dir_vec = kvs::CubicImage::DirectionVector(dir);
      const kvs::Vec3 up_vec = kvs::CubicImage::UpVector(dir);
      this->light()->setPosition(c_p);
      this->camera()->setPosition(c_p, c_p + dir_vec, up_vec);
      this->screen()->draw();
      cube_map.setImage(dir, this->screen()->capture());
    }
  }
  this->camera()->setFieldOfView(c_fov);
  this->camera()->setFront(c_front);
  this->camera()->setPosition(c_p, c_at, c_up);
  this->light()->setPosition(l_p);

  return cube_map;
}

/*===========================================================================*/
/**
 *  @brief  Enables or disables the collision detection.
 *  @param  enable [in] flag for the collision detection
 */
/*===========================================================================*/
void Scene::setEnabledCollisionDetection(bool enable) {
  m_enable_collision_detection = enable;
  if (enable)
    m_enable_object_operation = false;
}

/*==========================================================================*/
/**
 *  @brief  Initalizes the screen.
 */
/*==========================================================================*/
void Scene::initializeFunction() {
  // Set the lighting parameters.
  m_light->on();

  // Attach the Camera to the Mouse
  m_mouse->trackball().attachCamera(m_camera);
}

/*==========================================================================*/
/**
 *  @brief  Core paint event function.
 */
/*==========================================================================*/
void Scene::paintFunction() {
  this->updateGLProjectionMatrix();
  this->updateGLViewingMatrix();
  this->updateGLLightParameters();

  // Set the background color or image.
  m_background->apply();

  // Rendering the resistered object by using the corresponding renderer.
  if (m_object_manager->hasObject()) {
    const int size = m_id_manager->size();
    for (int index = 0; index < size; index++) {
      kvs::IDManager::IDPair id = m_id_manager->id(index);
      kvs::ObjectBase *object = m_object_manager->object(id.first);
      kvs::RendererBase *renderer = m_renderer_manager->renderer(id.second);
      if (object->isShown()) {
        kvs::OpenGL::PushMatrix();
        this->updateGLModelingMatrix(object);
        renderer->exec(object, m_camera, m_light);
        kvs::OpenGL::PopMatrix();
      }
    }
  } else {
    this->updateGLModelingMatrix();
  }
}

/*==========================================================================*/
/**
 *  @brief  Core resize event function.
 *  @param  width [in] screen width
 *  @param  height [in] screen height
 */
/*==========================================================================*/
void Scene::resizeFunction(int width, int height) {
  // Update the window size for camera.
  m_camera->setWindowSize(width, height);

  // Update the viewport for OpenGL.
  const float dpr = m_camera->devicePixelRatio();
  const int framebuffer_width = width * dpr;
  const int framebuffer_height = height * dpr;
  kvs::OpenGL::SetViewport(0, 0, framebuffer_width, framebuffer_height);
}

/*==========================================================================*/
/**
 *  @brief  Function which is called when the mouse button is released.
 *  @param  x [in] x coordinate value of the mouse cursor position
 *  @param  y [in] y coordinate value of the mouse cursor position
 */
/*==========================================================================*/
void Scene::mouseReleaseFunction(int x, int y) {
  m_enable_object_operation = true;
  m_enable_collision_detection = false;
  m_mouse->release(x, y);

  if (!(m_mouse->isEnabledAutoUpdating() && m_mouse->isUpdating())) {
    m_object_manager->releaseActiveObject();
  }
}

/*==========================================================================*/
/**
 *  @brief  Function which is called when the mouse button is released.
 *  @param  x [in] x coordinate value of the mouse cursor position
 *  @param  y [in] y coordinate value of the mouse cursor position
 *  @param  mode [in] mouse operation mode
 */
/*==========================================================================*/
void Scene::mousePressFunction(int x, int y, kvs::Mouse::OperationMode mode) {
  if (m_enable_object_operation || m_object_manager->hasActiveObject()) {
    this->updateControllingObject();
    m_mouse->setOperationMode(mode);
    m_mouse->press(x, y);
  }
}

/*==========================================================================*/
/**
 *  @brief  Function which is called when the mouse cursor is moved.
 *  @param  x [in] x coordinate value of the mouse cursor position
 *  @param  y [in] y coordinate value of the mouse cursor position
 */
/*==========================================================================*/
void Scene::mouseMoveFunction(int x, int y) {
  if (m_enable_object_operation || m_object_manager->hasActiveObject()) {
    m_mouse->move(x, y);
    this->updateXform();
  }
}

/*===========================================================================*/
/**
 *  @brief  Function which is called when the mouse wheel is scrolled.
 *  @param  value [in] incremental value
 */
/*===========================================================================*/
void Scene::wheelFunction(int value) {
  if (m_enable_object_operation || m_object_manager->hasActiveObject()) {
    this->updateControllingObject();
    m_mouse->setOperationMode(kvs::Mouse::Scaling);
    m_mouse->press(0, 0);
    m_mouse->move(0, value);
  }
}

/*===========================================================================*/
/**
 *  @brief  Returns the gravity of the center of the object in device coordinates.
 *  @param  object [in] pointer to the object
 *  @return gravity of the center of the object in device coordinates
 */
/*===========================================================================*/
kvs::Vec2 Scene::position_in_device(const kvs::ObjectBase *object) const {
  kvs::Vec2 p_dev;
  kvs::Vec3 p_obj = object->objectCenter();
  kvs::OpenGL::PushMatrix();
  {
    this->updateGLProjectionMatrix();
    this->updateGLViewingMatrix();
    this->updateGLModelingMatrix(object);

    const kvs::Vec2 p_win = ::Object2Window(p_obj, m_camera);
    p_dev.x() = p_win.x();
    p_dev.y() = m_camera->windowHeight() - p_win.y();
  }
  kvs::OpenGL::PopMatrix();

  return p_dev;
}

/*===========================================================================*/
/**
 *  @brief  Returns the gravity of the center of the object manager in device coordinates.
 *  @return gravity of the center of the object manager in device coordinates
 */
/*===========================================================================*/
kvs::Vec2 Scene::position_in_device() const {
  kvs::Vec2 p_dev;
  kvs::Vec3 p_obj = m_object_manager->xform().translation();
  kvs::OpenGL::PushMatrix();
  {
    this->updateGLProjectionMatrix();
    this->updateGLViewingMatrix();

    const kvs::Vec2 p_win = ::Object2Window(p_obj, m_camera);
    p_dev.x() = p_win.x();
    p_dev.y() = m_camera->windowHeight() - p_win.y();
  }
  kvs::OpenGL::PopMatrix();

  return p_dev;
}

/*===========================================================================*/
/**
 *  @brief  Returns true if the object collision is detected by clicking the mouse.
 *  @param  p_win [in] clicking position in the window coordinates
 *  @return true if the collision is detected
 */
/*===========================================================================*/
bool Scene::detect_collision(const kvs::Vec2 &p_win) {
  double min_distance = 100000.0;

  int active_object_id = -1;
  kvs::ObjectBase *active_object = 0;
  const size_t size = m_id_manager->size();
  for (size_t i = 0; i < size; i++) {
    kvs::IDManager::IDPair id = m_id_manager->id(i);
    kvs::ObjectBase *object = m_object_manager->object(id.first);
    const kvs::Vec2 p = this->position_in_device(object);
    const kvs::Vec2 diff = p - p_win;
    const double distance = diff.length();
    if (distance < min_distance) {
      min_distance = distance;
      active_object_id = id.first;
      active_object = object;
    }
  }

  bool has_active_object = false;
  if (this->detect_collision(active_object, p_win)) {
    m_object_manager->setActiveObject(active_object_id);
    has_active_object = true;
  }

  return has_active_object;
}

/*===========================================================================*/
/**
 *  @brief  Returns true if the object is selected by clicking the mouse.
 *  @param  object [in] pointer to the object
 *  @param  p_win [in] clicking position in the window coordinates
 *  @return true if the object is selected
 */
/*===========================================================================*/
bool Scene::detect_collision(const kvs::ObjectBase *object, const kvs::Vec2 &p_win) {
  float max_distance = -1.0f;

  // Center of this object in the window coordinate system.
  kvs::Vec2 center;

  kvs::OpenGL::PushMatrix();
  {
    this->updateGLProjectionMatrix();
    this->updateGLViewingMatrix();
    this->updateGLModelingMatrix(object);

    center = ::Object2Window(object->objectCenter(), m_camera);

    // Object's corner points in the object coordinate system.
    const kvs::Vec3 min_object_coord = object->minObjectCoord();
    const kvs::Vec3 max_object_coord = object->maxObjectCoord();
    const kvs::Vec3 corners[8] = {kvs::Vec3(min_object_coord.x(), min_object_coord.y(), min_object_coord.z()),
                                  kvs::Vec3(max_object_coord.x(), min_object_coord.y(), min_object_coord.z()),
                                  kvs::Vec3(min_object_coord.x(), min_object_coord.y(), max_object_coord.z()),
                                  kvs::Vec3(max_object_coord.x(), min_object_coord.y(), max_object_coord.z()),
                                  kvs::Vec3(min_object_coord.x(), max_object_coord.y(), min_object_coord.z()),
                                  kvs::Vec3(max_object_coord.x(), max_object_coord.y(), min_object_coord.z()),
                                  kvs::Vec3(min_object_coord.x(), max_object_coord.y(), max_object_coord.z()),
                                  kvs::Vec3(max_object_coord.x(), max_object_coord.y(), max_object_coord.z())};

    // Calculate max distance between the center and the corner in
    // the window coordinate system.
    for (int i = 0; i < 8; i++) {
      const kvs::Vec2 corner = ::Object2Window(corners[i], m_camera);
      const float distance = static_cast<float>((corner - center).length());
      max_distance = kvs::Math::Max(max_distance, distance);
    }
  }
  kvs::OpenGL::PopMatrix();

  kvs::Vec2 pos_window(p_win.x(), m_camera->windowHeight() - p_win.y());

  return (pos_window - center).length() < max_distance;
}

// kvs::jaea::pbvr::Scene original
float Scene::calcTranslationFactor(kvs::Vec3 hmdPos, kvs::Vec3 hmdDir, kvs::Vec3 hand_prev, kvs::Vec3 cameraPos) {
  int counter = 0;
  kvs::Vec3 objCenter = kvs::Vec3::Zero();

  for (int iobj = 0; iobj < objectManager()->numberOfObjects(); iobj++) {
    kvs::ObjectBase *obj = objectManager()->object(iobj);
    if (obj != nullptr && obj->isShown() && !ObjectNameUtil::isToIgnoreTranslationFactor(obj->name())) {
      counter++;

      kvs::Vec3 extCenter3 = obj->externalCenter();
      kvs::Vec4 extCenter4 = kvs::Vec4(extCenter3, 1);
      kvs::Mat4 xform = obj->xform().toMatrix();
      kvs::Vec4 objCenter4 = xform * extCenter4;
      kvs::Vec3 objCenter3 = objCenter4.xyz();
      objCenter += objCenter3;

#ifdef DEBUG_SCENE
      std::fprintf(stderr, "obj[%d](%s) : extCenter(% e % e % e) -> (% e % e % e)\n", iobj, obj->name().c_str(), extCenter3.x(), extCenter3.y(),
                   extCenter3.z(), objCenter3.x(), objCenter3.y(), objCenter3.z());
      std::fprintf(stderr, "obj[%d].xform0 | % e % e % e % e |\n", iobj, xform[0][0], xform[0][1], xform[0][2], xform[0][3]);
      std::fprintf(stderr, "obj[%d].xform1 | % e % e % e % e |\n", iobj, xform[1][0], xform[1][1], xform[1][2], xform[1][3]);
      std::fprintf(stderr, "obj[%d].xform2 | % e % e % e % e |\n", iobj, xform[2][0], xform[2][1], xform[2][2], xform[2][3]);
      std::fprintf(stderr, "obj[%d].xform3 | % e % e % e % e |\n", iobj, xform[3][0], xform[3][1], xform[3][2], xform[3][3]);
#endif // DEBUG_SCENE
    }
  }
  if (counter > 0) {
    objCenter /= ((float)counter);
  }

  // eye direction
  float len = hmdDir.length();
  kvs::Vec3 dir = (len < 1e-20) ? hmdDir : hmdDir / len;

  // center of object
  kvs::Vec3 o = objCenter - cameraPos;
  float objDistance = ::fabs(dir.x() * o.x() + dir.y() * o.y() + dir.z() * o.z());

  // mid-point of hands
  kvs::Vec3 h = hand_prev - hmdPos;
  float handDistance = ::fabs(dir.x() * h.x() + dir.y() * h.y() + dir.z() * h.z());

#ifdef DEBUG_SCENE
  std::cerr << "camera=h(" << hmdPos.x() << ", " << hmdPos.y() << ", " << hmdPos.z() << "), o(" << cameraPos.x() << ", " << cameraPos.y() << ", "
            << cameraPos.z() << ")" << std::endl;
  std::cerr << "ve=g(" << hmdDir.x() << ", " << hmdDir.y() << ", " << hmdDir.z() << "), c(" << dir.x() << ", " << dir.y() << ", " << dir.z() << ")"
            << std::endl;
  std::cerr << "vo=g(" << objCenter.x() << ", " << objCenter.y() << ", " << objCenter.z() << "), c(" << o.x() << ", " << o.y() << ", " << o.z() << ")"
            << std::endl;
  std::cerr << "vh=g(" << hand_prev.x() << ", " << hand_prev.y() << ", " << hand_prev.z() << "), c(" << h.x() << ", " << h.y() << ", " << h.z() << ")"
            << std::endl;
  std::cerr << "diatance : obj=" << objDistance << ", hand=" << handDistance << std::endl;
#endif

  if (handDistance < 1e-100) {
    return 1.0;
  }
  return objDistance / handDistance;
}

#ifdef DISP_MODE_VR
void Scene::threedcontrollerPressFunction(kvs::Vec3 hmdPos, kvs::Vec3 initialHmdPos, kvs::Vec3 hmdDir, kvs::Vec3 left_p, kvs::Vec3 right_p,
                                          kvs::Vec3 prev_left_p, kvs::Vec3 prev_right_p) {
  // scaling
  float current_distance = (left_p - right_p).length();
  float prev_distance = (prev_left_p - prev_right_p).length();
  float scaling_ratio = current_distance / prev_distance;
#ifdef DEBUG_SCENE
   float scaling_ratio_accum = this->getScalingRatioAccum();
#endif // DEBUG_SCENE

  // translation
  kvs::Vec3 center_current = (left_p + right_p) / 2.0f;
  kvs::Vec3 center_prev = (prev_left_p + prev_right_p) / 2.0f;
  kvs::Vec3 center_diff = center_current - center_prev;
  kvs::Vec3 center_diff_xy = kvs::Vec3(center_diff.x(), center_diff.y(), 0.0);
  kvs::Vec3 cameraPos = Screen::KVS_CAMERA_INITIAL_POSITION + hmdPos - initialHmdPos;
  float translation_factor = calcTranslationFactor(hmdPos, hmdDir, center_prev, cameraPos);
  // kvs::Vec3 diff_translate = center_diff * translation_factor / scaling_ratio_accum;
  // kvs::Vec3 diff_translate = center_diff * translation_factor;
  kvs::Vec3 diff_translate = center_diff_xy * translation_factor;

#ifdef DEBUG_SCENE
  std::cerr << "3DController : trans=(" << diff_translate.x() << ", " << diff_translate.y() << ", " << diff_translate.z()
            << "), factor=" << translation_factor << ", scaling=" << scaling_ratio_accum << std::endl;
#endif // DEBUG_SCENE

  // rotation
  kvs::Vec3 current_rotation = left_p - right_p;
  kvs::Vec3 prev_rotation = prev_left_p - prev_right_p;
  kvs::Vec3 rotation_axis = ((current_rotation.cross(prev_rotation)).normalized()) * (-1.0f);

  float cos = (current_rotation.dot(prev_rotation)) / (current_rotation.length() * prev_rotation.length());
  cos = kvs::Math::Clamp(cos, -1.0f, 1.0f);
  float rotation_degree = kvs::Math::Rad2Deg(acos(cos));
  kvs::Mat3 rotation_mat = kvs::Mat3::Rotation(rotation_axis, rotation_degree);

  kvs::Vec3 param_scale(scaling_ratio, scaling_ratio, scaling_ratio);
  kvs::Vec3 param_trans = diff_translate;
  kvs::Mat3 param_rot = rotation_mat;

#ifdef DEBUG_SCENE
  auto old_obj_xform = objectManager()->xform().toMatrix();
  auto old_obj_pos = (old_obj_xform * kvs::Vec4(objectManager()->externalCenter(), 1)).xyz();
#endif // DEBUG_SCENE

  objectManager()->scale(param_scale);
  objectManager()->translate(param_trans);
  objectManager()->rotate(param_rot);

#ifdef DEBUG_SCENE
  auto new_obj_xform = objectManager()->xform().toMatrix();
  auto new_obj_pos = (new_obj_xform * kvs::Vec4(objectManager()->externalCenter(), 1)).xyz();

  std::fprintf(stderr,
               "- PressFunction -------------------\n"
               "param : scale=% e, trans=(% e % e % e)\n"
               "prev | % e % e % e % e |  % e  ::  % e\n"
               "     | % e % e % e % e |  % e  ::  % e\n"
               "     | % e % e % e % e |  % e  ::  % e\n"
               "     | % e % e % e % e |\n"
               "new  | % e % e % e % e |  % e  ::  % e\n"
               "     | % e % e % e % e |  % e  ::  % e\n"
               "     | % e % e % e % e |  % e  ::  % e\n"
               "     | % e % e % e % e |\n",
               param_scale[0], param_trans[0], param_trans[1], param_trans[2], old_obj_xform[0][0], old_obj_xform[0][1], old_obj_xform[0][2],
               old_obj_xform[0][3], old_obj_pos[0], center_prev[0], old_obj_xform[1][0], old_obj_xform[1][1], old_obj_xform[1][2],
               old_obj_xform[1][3], old_obj_pos[1], center_prev[1], old_obj_xform[2][0], old_obj_xform[2][1], old_obj_xform[2][2],
               old_obj_xform[2][3], old_obj_pos[2], center_prev[2], old_obj_xform[3][0], old_obj_xform[3][1], old_obj_xform[3][2],
               old_obj_xform[3][3], new_obj_xform[0][0], new_obj_xform[0][1], new_obj_xform[0][2], new_obj_xform[0][3], new_obj_pos[0],
               center_current[0], new_obj_xform[1][0], new_obj_xform[1][1], new_obj_xform[1][2], new_obj_xform[1][3], new_obj_pos[1],
               center_current[1], new_obj_xform[2][0], new_obj_xform[2][1], new_obj_xform[2][2], new_obj_xform[2][3], new_obj_pos[2],
               center_current[2], new_obj_xform[3][0], new_obj_xform[3][1], new_obj_xform[3][2], new_obj_xform[3][3]);
#endif // DEBUG_SCENE
}

void Scene::threedcontrollerReleaseFunction(kvs::Vec3 hmdPos, kvs::Vec3 initialHmdPos, kvs::Vec3 hmdDir, kvs::Vec3 left_p, kvs::Vec3 right_p,
                                            kvs::Vec3 prev_left_p, kvs::Vec3 prev_right_p) {
  objectManager()->releaseActiveObject();
}

void Scene::glyphControllerPressFunction(kvs::Vec3 hmdPos, kvs::Vec3 initialHmdPos, kvs::Vec3 hmdDir, kvs::Vec3 left_p, kvs::Vec3 right_p,
                                  kvs::Vec3 prev_left_p, kvs::Vec3 prev_right_p)
{
  // scaling
//  float current_distance = (left_p - right_p).length();
//  float prev_distance = (prev_left_p - prev_right_p).length();
//  float scaling_ratio = current_distance / prev_distance;
#ifdef DEBUG_SCENE
  float scaling_ratio_accum = this->getScalingRatioAccum();
#endif // DEBUG_SCENE

  // translation
//  kvs::Vec3 center_current = (left_p + right_p) / 2.0f;
//  kvs::Vec3 center_prev = (prev_left_p + prev_right_p) / 2.0f;
  kvs::Vec3 center_current = right_p;
  kvs::Vec3 center_prev = prev_right_p;
  kvs::Vec3 center_diff = center_current - center_prev;
  kvs::Vec3 center_diff_xy = kvs::Vec3(center_diff.x(), center_diff.y(), center_diff.z());
  kvs::Vec3 cameraPos = Screen::KVS_CAMERA_INITIAL_POSITION + hmdPos - initialHmdPos;
  float translation_factor = calcTranslationFactor(hmdPos, hmdDir, center_prev, cameraPos);
  // kvs::Vec3 diff_translate = center_diff * translation_factor / scaling_ratio_accum;
  // kvs::Vec3 diff_translate = center_diff * translation_factor;
  kvs::Vec3 diff_translate = center_diff_xy * translation_factor;

//  // rotation
//  kvs::Vec3 current_rotation = left_p - right_p;
//  kvs::Vec3 prev_rotation = prev_left_p - prev_right_p;
//  kvs::Vec3 rotation_axis = ((current_rotation.cross(prev_rotation)).normalized()) * (-1.0f);

//  float cos = (current_rotation.dot(prev_rotation)) / (current_rotation.length() * prev_rotation.length());
//  cos = kvs::Math::Clamp(cos, -1.0f, 1.0f);
//  float rotation_degree = kvs::Math::Rad2Deg(acos(cos));
//  kvs::Mat3 rotation_mat = kvs::Mat3::Rotation(rotation_axis, rotation_degree);

//  kvs::Vec3 param_scale(scaling_ratio, scaling_ratio, scaling_ratio);
  //kvs::Vec3 param_trans = center_current;
  kvs::Vec3 param_trans = (right_p - prev_right_p) + m_before_right;

//  kvs::Mat3 param_rot = rotation_mat;

//  objectManager()->scale(param_scale);
//  objectManager()->translate(param_trans);
//  std::cout << __LINE__ << std::endl;
//  std::cout << param_trans << std::endl;
//  std::cout << objectManager()->object("GLYPH")->xform().toMatrix() << std::endl;
//  objectManager()->object("GLYPH")->translate( param_trans );

  //---
//  kvs::Xform xform(kvs::Mat4(0.00804317,-0.000290376,2.63251e-05,-0.12550,
//             0.000290588, 0.00804293, -6.76433e-05, -0.262974,
//             -2.38665e-05, 6.85494e-05, 0.00804813, 0.296044,
//                             0, 0, 0, 1));
//  objectManager()->object("GLYPH")->setXform(xform);
  //---

//  objectManager()->rotate(param_rot);
//  kvs::Vec3 posV = param_trans;
  float posX = param_trans.x() * 1;
  float posY = param_trans.y() * 1;
  float posZ = param_trans.z() * 1;
//  posX = ;
//  posY = 0;
//  posZ = 0;
//  std::cout << posX << "," << posY << "," << posZ << std::endl;
  if( this->hasObject( "GLYPH" ) == true )
  {
    kvs::ValueArray<kvs::Real32> pos;
    pos.allocate(3*1);
    pos.at(0) = posX;
    pos.at(1) = posY;
    pos.at(2) = posZ;

    kvs::PolygonObject* glyph_polygon = new kvs::SphereGlyph( 1, pos );
    pos.release();
    glyph_polygon->setName( "GLYPH" );
    glyph_polygon->setColor( kvs::RGBColor( 0, 255, 0 ) );
    this->replaceObject( "GLYPH", glyph_polygon );
//    delete glyph_polygon;
  }
  //m_before_right = right_p;
    m_before_right = param_trans;
}

void Scene::glyphControllerReleaseFunction(kvs::Vec3 hmdPos, kvs::Vec3 initialHmdPos, kvs::Vec3 hmdDir, kvs::Vec3 left_p, kvs::Vec3 right_p,
                                  kvs::Vec3 prev_left_p, kvs::Vec3 prev_right_p)
{
//  std::cout << __LINE__ << std::endl;
//  std::cout << objectManager()->object("GLYPH")->xform().toMatrix() << std::endl;
  objectManager()->releaseActiveObject();

//  std::cout << __LINE__ << std::endl;
//  std::cout << objectManager()->object("GLYPH")->xform().toMatrix() << std::endl;
//  std::cout << __LINE__ << std::endl;
//  std::cout << objectManager()->xform().toMatrix() << std::endl;



//  float* test;
//  objectManager()->xform().toArray(test);
//  m_touch_controller();

}

void Scene::setGuestGlyphPos()
{
  std::cout << __LINE__ << std::endl;
  if( this->hasObject( "GLYPH" ) == true )
  {
    kvs::ValueArray<kvs::Real32> pos;
    pos.allocate(3*1);
    pos.at(0) = m_current_glyph_pos.x() / 100;
    pos.at(1) = m_current_glyph_pos.y() / 100;
    pos.at(2) = m_current_glyph_pos.z() / 100;

//    pos.at(0) = 24.9545 / 100;
//    pos.at(1) = -3.00874 / 100;
//    pos.at(2) = 0.159492 / 100;

    kvs::PolygonObject* glyph_polygon = new kvs::SphereGlyph( 1, pos );
    pos.release();
    glyph_polygon->setName( "GLYPH" );
    glyph_polygon->setColor( kvs::RGBColor( 0, 255, 0 ) );
    this->replaceObject( "GLYPH", glyph_polygon );
    //    delete glyph_polygon;
  }
}
#endif // DISP_MODE_VR

kvs::Real32 Scene::getScalingRatioAccum() {
  kvs::Xform x = objectManager()->xform();
  kvs::Vec3 scale = x.scaling();
  return (scale.x() + scale.y() + scale.z()) / 3.0f;
}

void Scene::resetObjects() {
  // Reset the xform of the object.
  if (this->objectManager()->hasActiveObject()) {
    this->objectManager()->resetActiveObjectXform();
  } else {
    this->objectManager()->resetXform();
    this->setScaleToObjectManager();
  }
}

void Scene::setScaleToObjectManager() {
#ifdef DISP_MODE_VR
  this->objectManager()->scale(kvs::Vec3(OBJECT_SCALING_FACTOR, OBJECT_SCALING_FACTOR, OBJECT_SCALING_FACTOR));
#endif
}

// copy from kvs::CameraCoordinate
/*===========================================================================*/
/**
 *  @brief  Constructs a new CameraCoordinate class.
 *  @param  position [in] position in camera coordinates
 *  @param  camera [in] pointer to a camera defines camera coordinates
 */
/*===========================================================================*/
CameraCoordinate::CameraCoordinate(const kvs::Vec3 &position, const kvs::Camera *camera) : m_position(position), m_camera(camera) {}

/*===========================================================================*/
/**
 *  @brief  Transforms camera coordinates to normalized device coordinates.
 *  @return normalized device coordinates
 */
/*===========================================================================*/
const NormalizedDeviceCoordinate CameraCoordinate::toNormalizedDeviceCoordinate() const {
  const kvs::Vec3 position = kvs::Xform(m_camera->projectionMatrix()).project(m_position);
  return NormalizedDeviceCoordinate(position);
}

/*===========================================================================*/
/**
 *  @brief  Transforms camera coordinate to world coordinates.
 *  @return world coordinates
 */
/*===========================================================================*/
const WorldCoordinate CameraCoordinate::toWorldCoordinate() const { return WorldCoordinate(m_camera->xform().transform(m_position)); }

// copy ftom kvs::NormalizedDeviceCoordinate
/*===========================================================================*/
/**
 *  @brief  Constructs a new NormalizedDeviceCoordinate class.
 *  @param  position [in] position in normalized device coordinates
 */
/*===========================================================================*/
NormalizedDeviceCoordinate::NormalizedDeviceCoordinate(const kvs::Vec3 &position) : m_position(position) {}

/*===========================================================================*/
/**
 *  @brief  Transforms normalized device coordinates to world coordinates.
 *  @param  x [in] x coordinate value of left corner of the viewport
 *  @param  y [in] y coordinate value of left corner of the viewport
 *  @param  width [in] width of the viewport
 *  @param  height [in] height of the viewport
 *  @return world coordinates
 */
/*===========================================================================*/
const WindowCoordinate NormalizedDeviceCoordinate::toWindowCoordinate(const int x, const int y, const size_t width, const size_t height) const {
  const float px = (m_position[0] + 1.0f) * width * 0.5f + x;
  const float py = (m_position[1] + 1.0f) * height * 0.5f + y;
  const float pz = WindowCoordinate::CalculateDepth((m_position[2] + 1.0f) / 2.0f);
  const kvs::Vec3 position(px, py, pz);
  return WindowCoordinate(position, x, y, width, height);
}

/*===========================================================================*/
/**
 *  @brief  Transforms normalized device coordinates to world coordinates.
 *  @param  viewport [in] viewport vector
 *  @return world coordinates
 */
/*===========================================================================*/
const WindowCoordinate NormalizedDeviceCoordinate::toWindowCoordinate(const kvs::Vec4i &viewport) const {
  return this->toWindowCoordinate(viewport[0], viewport[1], static_cast<size_t>(viewport[2]), static_cast<size_t>(viewport[3]));
}

/*===========================================================================*/
/**
 *  @brief  Transforms normalized device coordinates to camera coordinates.
 *  @param  camera [in] pointer to a camera defines camera coordinates
 *  @return camera coordinates
 */
/*===========================================================================*/
const CameraCoordinate NormalizedDeviceCoordinate::toCameraCoordinate(const kvs::Camera *camera) const {
  const kvs::Xform xform(camera->projectionMatrix());
  const kvs::Vec3 position = xform.inverse().project(m_position);
  return CameraCoordinate(position, camera);
}

// copy fomr kvs::ObjectCoordinate
/*===========================================================================*/
/**
 *  @brief  Constructs a new ObjectCoordinate class.
 *  @param  position [in] position in object coordinates
 *  @param  object [in] pointer to an object defines object coordinates
 */
/*===========================================================================*/
ObjectCoordinate::ObjectCoordinate(const kvs::Vec3 &position, const kvs::ObjectBase *object) : m_position(position), m_object(object) {}

/*===========================================================================*/
/**
 *  @brief  Transforms object coordinates to camera coordinates.
 *  @return world coordinates
 */
/*===========================================================================*/
const WorldCoordinate ObjectCoordinate::toWorldCoordinate() const {
  KVS_ASSERT(m_object != NULL);

  const kvs::Vec3 position = m_object->xform().transform(m_position);
  return WorldCoordinate(position);
}

// copy from kvs::WindowCoordinate

float WindowCoordinate::m_front = 0.0f;
float WindowCoordinate::m_back = 1.0f;

/*===========================================================================*/
/**
 *  @brief  Sets depth range.
 *  @param  front [in] depth value of the front clipping plane
 *  @param  back [in] depth value of the back clipping plane
 */
/*===========================================================================*/
void WindowCoordinate::SetDepthRange(const float front, const float back) {
  m_front = front;
  m_back = back;
}

/*===========================================================================*/
/**
 *  @brief  Calculates the depth value.
 *  @param  nd_depth [in] depth value
 *  @return depth value
 */
/*===========================================================================*/
float WindowCoordinate::CalculateDepth(const float nd_depth) { return m_front + (m_back - m_front) * nd_depth; }

/*===========================================================================*/
/**
 *  @brief  Inverts the depth value.
 *  @param  depth [in] depth value
 *  @return depth value
 */
/*===========================================================================*/
float WindowCoordinate::InvertDepth(const float depth) { return (depth - m_front) / (m_back - m_front); }

/*===========================================================================*/
/**
 *  @brief  Constructs a new WindowCoordinate class.
 *  @param  position [in] position in the window coordinates
 *  @param  x [in] x coordinate value of left corner of the viewport
 *  @param  y [in] y coordinate value of left corner of the viewport
 *  @param  width [in] width of the viewport
 *  @param  height [in] height of the viewport
 */
/*===========================================================================*/
WindowCoordinate::WindowCoordinate(const kvs::Vec3 &position, const int x, const int y, const size_t width, const size_t height)
    : m_position(position), m_x(x), m_y(y), m_width(width), m_height(height) {}

/*===========================================================================*/
/**
 *  @brief  Constructs a new WindowCoordinate class.
 *  @param  position [in] position in the window coordinates
 *  @param  viewport [in] viewport vector
 */
/*===========================================================================*/
WindowCoordinate::WindowCoordinate(const kvs::Vec3 &position, const kvs::Vec4i &viewport)
    : m_position(position), m_x(viewport[0]), m_y(viewport[1]), m_width(static_cast<size_t>(viewport[2])),
      m_height(static_cast<size_t>(viewport[3])) {}

/*===========================================================================*/
/**
 *  @brief  Transforms the widnow coordinates to the normalized device coordinates.
 *  @return transformed position in the normalized device coordinates
 */
/*===========================================================================*/
const NormalizedDeviceCoordinate WindowCoordinate::toNormalizedDeviceCoordinate() const {
  const float x = 2.0f * (m_position[0] - m_x) / m_width - 1.0f;
  const float y = 2.0f * (m_position[1] - m_y) / m_height - 1.0f;
  const float z = WindowCoordinate::InvertDepth(m_position[2]) * 2.0f - 1.0f;
  const kvs::Vec3 position(x, y, z);
  return NormalizedDeviceCoordinate(position);
}

// copy from kvs::WorldCoordinate
/*===========================================================================*/
/**
 *  @brief  Constructs a new WorldCoordinate class.
 *  @param  position [in] position in world coordinates
 */
/*===========================================================================*/
WorldCoordinate::WorldCoordinate(const kvs::Vec3 &position) : m_position(position) {}

/*===========================================================================*/
/**
 *  @brief  Transforms world coordinates to camera coordinates.
 *  @param  camera [in] pointer to a camera defines camera coordiantes
 *  @return transformed position in camera coordinates
 */
/*===========================================================================*/
const CameraCoordinate WorldCoordinate::toCameraCoordinate(const kvs::Camera *camera) const {
  const kvs::Vec3 position = kvs::Xform(camera->viewingMatrix()).transform(m_position);
  return CameraCoordinate(position, camera);
}

/*===========================================================================*/
/**
 *  @brief  Transforms the world coordinates to the object coordinates.
 *  @param  object [in] pointer to an object defines object coordiantes
 *  @return transformed position in object coordinates
 */
/*===========================================================================*/
const ObjectCoordinate WorldCoordinate::toObjectCoordinate(const kvs::ObjectBase *object) const {
  const kvs::Vec3 position = object->xform().inverse().transform(m_position);
  return ObjectCoordinate(position, object);
}

} // end of namespace pbvr

} // end of namespace jaea

} // end of namespace kvs
