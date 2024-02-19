#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "cameraimagerenderer.h"

#include <cassert>
#include <cmath>
#include <kvs/Camera>
#include <kvs/IgnoreUnusedVariable>
#include <kvs/Light>
#include <kvs/OpenGL>

#include "stereocamera.h"

const std::string GLSL_VERT_CODE =
#include "CameraImageShader.vert"
    ;
const std::string GLSL_FRAG_CODE =
#include "CameraImageShader.frag"
    ;

/*===========================================================================*/
/**
 *  @brief  Constructs a new CameraImageRenderer class.
 */
/*===========================================================================*/
CameraImageRenderer::CameraImageRenderer() : StochasticRendererBase(new Engine()) {}

/*===========================================================================*/
/**
 *  @brief  Constructs a new CameraImageRenderer class.
 *  @param  m [in] initial modelview matrix
 *  @param  p [in] initial projection matrix
 *  @param  v [in] initial viewport
 */
/*===========================================================================*/
CameraImageRenderer::CameraImageRenderer(const kvs::Mat4 &m, const kvs::Mat4 &p, const kvs::Vec4 &v) : StochasticRendererBase(new Engine(m, p, v)) {}

void CameraImageRenderer::initializeStereoCamera(float up, float right, float down, float left) {
  static_cast<Engine &>(engine()).initializeStereoCamera(up, right, down, left);
}

bool CameraImageRenderer::isStereoCameraInitialized() const { return static_cast<const Engine &>(engine()).isStereoCameraInitialized(); }

void CameraImageRenderer::setStereoCamera(StereoCamera *stereo_camera) { static_cast<Engine &>(engine()).setStereoCamera(stereo_camera); }

/*===========================================================================*/
/**
 *  @brief  Constructs a new Engine class.
 */
/*===========================================================================*/
CameraImageRenderer::Engine::Engine() {}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Engine class.
 *  @param  m [in] initial modelview matrix
 *  @param  p [in] initial projection matrix
 *  @param  v [in] initial viewport
 */
/*===========================================================================*/
CameraImageRenderer::Engine::Engine(const kvs::Mat4 &m, const kvs::Mat4 &p, const kvs::Vec4 &v) {}

/*===========================================================================*/
/**
 *  @brief  Destroys the Engine class.
 */
/*===========================================================================*/
CameraImageRenderer::Engine::~Engine() {
  glDeleteBuffers(3, m_rectVBO);
  assert(glGetError() == GL_NO_ERROR);
}

/*===========================================================================*/
/**
 *  @brief  Releases the GPU resources.
 */
/*===========================================================================*/
void CameraImageRenderer::Engine::release() {
  if (m_shader_program.isCreated()) {
    m_shader_program.release();
  }
}

/*===========================================================================*/
/**
 *  @brief  Create shaders.
 *  @param  point [in] pointer to the point object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void CameraImageRenderer::Engine::create(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light) {
  kvs::IgnoreUnusedVariable(object);
  kvs::IgnoreUnusedVariable(light);
  kvs::IgnoreUnusedVariable(camera);

  // Creates shader program.
  kvs::ShaderSource vert("PBR_zooming.vert");
  kvs::ShaderSource frag("PBR_zooming.frag");
  vert.clearCode();
  frag.clearCode();
  vert.setCode(GLSL_VERT_CODE);
  frag.setCode(GLSL_FRAG_CODE);
  m_shader_program.build(vert, frag);
}

/*===========================================================================*/
/**
 *  @brief  Update.
 *  @param  point [in] pointer to the point object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void CameraImageRenderer::Engine::update(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light) {
  kvs::IgnoreUnusedVariable(object);
  kvs::IgnoreUnusedVariable(light);
  kvs::IgnoreUnusedVariable(camera);
}

/*===========================================================================*/
/**
 *  @brief  Setup.
 *  @param  point [in] pointer to the point object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void CameraImageRenderer::Engine::setup(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light) {
  kvs::IgnoreUnusedVariable(object);
  kvs::IgnoreUnusedVariable(light);
  kvs::IgnoreUnusedVariable(camera);

  // The repetition counter must be reset here.
  resetRepetitions();

  kvs::OpenGL::Enable(GL_DEPTH_TEST);
  kvs::OpenGL::Enable(GL_VERTEX_PROGRAM_POINT_SIZE);
  ATTRIB_TEXTURE2D_POS = m_shader_program.attributeLocation("in_texCoord");
  ATTRIB_VERTICES_POS = m_shader_program.attributeLocation("in_vertex");

  const kvs::Mat4 P = kvs::OpenGL::ProjectionMatrix();
  m_shader_program.bind();
  m_shader_program.setUniform("ProjectionMatrix", P);
  m_shader_program.setUniform("maxZEDDepth", StereoCamera::maxDepth());
  m_shader_program.unbind();
}

/*===========================================================================*/
/**
 *  @brief  Draw.
 *  @param  point [in] pointer to the point object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void CameraImageRenderer::Engine::draw(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light) {
  kvs::IgnoreUnusedVariable(object);
  kvs::IgnoreUnusedVariable(light);
  kvs::IgnoreUnusedVariable(camera);
}

void CameraImageRenderer::Engine::initializeStereoCamera(float up, float right, float down, float left) {
  // Define polygon and texture-coordinate for ZED image
  float rectVertices[12] = {-left, -up, 0, right, -up, 0, right, down, 0, -left, down, 0};

  glGenBuffers(1, &m_rectVBO[0]);
  glBindBuffer(GL_ARRAY_BUFFER, m_rectVBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW);

  float rectTexCoord[8] = {0, 1, 1, 1, 1, 0, 0, 0};
  glGenBuffers(1, &m_rectVBO[1]);
  glBindBuffer(GL_ARRAY_BUFFER, m_rectVBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(rectTexCoord), rectTexCoord, GL_STATIC_DRAW);

  unsigned int rectIndices[6] = {0, 1, 2, 0, 2, 3};
  glGenBuffers(1, &m_rectVBO[2]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rectVBO[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectIndices), rectIndices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  m_stereo_camera_initialized = true;

  assert(glGetError() == GL_NO_ERROR);
}

void CameraImageRenderer::Engine::drawStereoCameraImage(int eye) {
  if (!m_stereo_camera_initialized) {
    // Stereo camera has not been initialized
    return;
  }
  if (m_stereo_camera == nullptr) {
    // Stereo camera has not been initialized
    return;
  }

  kvs::ProgramObject::Binder bind(m_shader_program);
  m_shader_program.setUniform("colorTexture", 1); // == GL_TEXTURE1
  m_shader_program.setUniform("depthTexture", 2); // == GL_TEXTURE2

  //    kvs::OpenGL::Enable( GL_DEPTH_TEST );

  // Bind the Vertex Buffer Objects of the rectangle that displays ZED images
  // vertices
  glEnableVertexAttribArray(ATTRIB_VERTICES_POS);
  glBindBuffer(GL_ARRAY_BUFFER, m_rectVBO[0]);
  glVertexAttribPointer(ATTRIB_VERTICES_POS, 3, GL_FLOAT, GL_FALSE, 0, 0);
  // indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rectVBO[2]);
  // texture coordinates
  glEnableVertexAttribArray(ATTRIB_TEXTURE2D_POS);
  glBindBuffer(GL_ARRAY_BUFFER, m_rectVBO[1]);
  glVertexAttribPointer(ATTRIB_TEXTURE2D_POS, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glActiveTexture(GL_TEXTURE1);
  if (eye == 0) {
    // left eye
    glBindTexture(GL_TEXTURE_2D, m_stereo_camera->textureID(eye));
    m_shader_program.setUniform("isLeft", 1);
  } else {
    // right eye
    glBindTexture(GL_TEXTURE_2D, m_stereo_camera->textureID(eye));
    m_shader_program.setUniform("isLeft", 0);
  }

  // Send depth map to shader
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, m_stereo_camera->depthTextureID(eye));

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glDisableVertexAttribArray(ATTRIB_TEXTURE2D_POS);
  glDisableVertexAttribArray(ATTRIB_VERTICES_POS);

  assert(glGetError() == GL_NO_ERROR);
}
