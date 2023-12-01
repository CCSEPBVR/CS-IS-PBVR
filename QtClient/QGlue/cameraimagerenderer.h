#pragma once
#include <kvs/Matrix44>
#include <kvs/Module>
#include <kvs/ProgramObject>
#include <kvs/RendererBase>
#include <kvs/StochasticRendererBase>
#include <kvs/StochasticRenderingEngine>
#include <kvs/Vector4>

class StereoCamera;

class CameraImageRenderer : public kvs::StochasticRendererBase {
  kvsModule(CameraImageRenderer, Renderer);
  kvsModuleBaseClass(kvs::StochasticRendererBase);

public:
  class Engine;

public:
  CameraImageRenderer();
  CameraImageRenderer(const kvs::Mat4 &m, const kvs::Mat4 &p, const kvs::Vec4 &v);

  void enableShuffle();
  void enableZooming();
  void disableShuffle();
  void disableZooming();

  void initializeStereoCamera(float up, float right, float down, float left);
  bool isStereoCameraInitialized() const;
  void setStereoCamera(StereoCamera *stereo_camera);
};

/*===========================================================================*/
/**
 *  @brief  Engine class for particle-based renderer.
 */
/*===========================================================================*/
class CameraImageRenderer::Engine : public kvs::StochasticRenderingEngine {
private:
  kvs::ProgramObject m_shader_program;

  size_t ATTRIB_VERTICES_POS = 0;
  size_t ATTRIB_TEXTURE2D_POS = 0;
  bool m_stereo_camera_initialized = false;
  GLuint m_rectVBO[3] = {0, 0, 0};
  StereoCamera *m_stereo_camera = nullptr;

public:
  Engine();
  Engine(const kvs::Mat4 &m, const kvs::Mat4 &p, const kvs::Vec4 &v);
  virtual ~Engine();
  void release();
  void create(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light);
  void update(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light);
  void setup(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light);
  void draw(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light);

  void setStereoCamera(StereoCamera *stereo_camera) { m_stereo_camera = stereo_camera; };
  void initializeStereoCamera(float up, float right, float down, float left);
  bool isStereoCameraInitialized() const { return m_stereo_camera_initialized; }
  /**
   * @brief Rendereing stereo camera image
   *
   * @param eye: 0-left eye, other-right eye.
   */
  void drawStereoCameraImage(int eye);
};
