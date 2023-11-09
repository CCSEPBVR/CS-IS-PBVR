#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

// #include <QOpenGLFunctions>
#include <QtGlobal>

#include <kvs/OpenGL>
// #include <QtOpenGL>
#include <OVR_CAPI_GL.h>

#include "call.h"
#include "oculus.h"
#include "oculustexturebuffer.h"

#include <cassert>

static long long int frame_index = 0L;

OculusTextureBuffer::OculusTextureBuffer(ovrSession session, /*Sizei size*/ int width, int height, int sampleCount)
    : Session(session), ColorTextureChain(nullptr), DepthTextureChain(nullptr), fboId(0), fboId_old(0) /*, texWidth(0), texHeight(0)*/ {

#ifdef DEBUG_SCREEN
  std::cout << "MSG OculusTextureBuffer::OculusTExtureBuffer() called." << std::endl;
#endif // DEBUG_SCREEN

  assert(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

  texWidth = width;
  texHeight = height;
  m_sampleCount = sampleCount;

  // This texture isn't necessarily going to be a rendertarget, but it usually is.
  assert(session); // No HMD? A little odd.
  Create();
}

void OculusTextureBuffer::Create() {

#ifdef DEBUG_SCREEN
  std::cout << "MSG OculusTextureBuffer::Create() called." << std::endl;
#endif // DEBUG_SCREEN

  ovrTextureSwapChainDesc desc = {};
  desc.Type = ovrTexture_2D;
  desc.ArraySize = 1;
  desc.Width = texWidth;
  desc.Height = texHeight;
  desc.MipLevels = 1;
  desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
  desc.SampleCount = m_sampleCount;
  desc.StaticImage = ovrFalse;

  ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc, &ColorTextureChain);

  int length = 0;
  ovr_GetTextureSwapChainLength(Session, ColorTextureChain, &length);

  if (OVR_SUCCESS(result)) {
    // nColorTex = length;
    // colorTexId = new GLuint[length];

    for (int i = 0; i < length; ++i) {
      GLuint chainTexId;
      ovr_GetTextureSwapChainBufferGL(Session, ColorTextureChain, i, &chainTexId);
      glBindTexture(GL_TEXTURE_2D, chainTexId);

#ifdef DEBUG_SCREEN
      std::cout << "MSG OculusTextureBuffer::Create() i=" << i << " : bind color texture " << chainTexId << std::endl;
#endif // DEBUG_SCREEN

      // colorTexId[i] = chainTexId;

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
  }

  desc.Format = OVR_FORMAT_D32_FLOAT;

  result = ovr_CreateTextureSwapChainGL(Session, &desc, &DepthTextureChain);

  length = 0;
  ovr_GetTextureSwapChainLength(Session, DepthTextureChain, &length);

  if (OVR_SUCCESS(result)) {
    for (int i = 0; i < length; ++i) {
      GLuint chainTexId;
      ovr_GetTextureSwapChainBufferGL(Session, DepthTextureChain, i, &chainTexId);
      glBindTexture(GL_TEXTURE_2D, chainTexId);

#ifdef DEBUG_SCREEN
      std::cout << "MSG OculusTextureBuffer::Create() i=" << i << " : bind depth texture " << chainTexId << std::endl;
#endif // DEBUG_SCREEN

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
  }
  glGenFramebuffers(1, &fboId);

#ifdef DEBUG_SCREEN
  std::cout << "MSG OculusTextureBuffer::Create() new FBO ID=" << fboId << std::endl;
#endif // DEBUG_SCREEN

  {
    int e;
    while ((e = glGetError()) != 0) {
      qCritical("OculusTextureBuffer::Create GL HAS ERROR : %d", e);
    }
  }

  {
    ovrErrorInfo errInfo;
    for (int ierr = 0;; ierr++) {
      ovr_GetLastErrorInfo(&errInfo);
      if (OVR_SUCCESS(errInfo.Result)) {
        break;
      }
      std::fprintf(stdout, "OculusTextureBuffer::OculusTextureBuffer() has OVR error [%d] %s\n", ierr, errInfo.ErrorString);
    }
  }

  // default background color = white
  bgColor[0] = 1.0f;
  bgColor[1] = 1.0f;
  bgColor[2] = 1.0f;
  bgColor[3] = 1.0f;
}

OculusTextureBuffer::~OculusTextureBuffer() {

#ifdef DEBUG_SCREEN
  std::cout << "MSG OculusTextureBuffer::~OculusTExtureBuffer() called." << std::endl;
#endif // DEBUG_SCREEN

  if (ColorTextureChain) {
    ovr_DestroyTextureSwapChain(Session, ColorTextureChain);
    ColorTextureChain = nullptr;
  }
  if (DepthTextureChain) {
    ovr_DestroyTextureSwapChain(Session, DepthTextureChain);
    DepthTextureChain = nullptr;
  }
  if (fboId) {
    glDeleteFramebuffers(1, &fboId);
    fboId = 0;
  }
}

void OculusTextureBuffer::BackgroundColor(const kvs::RGBColor& col) {
  kvs::Vec3 v = col.toVec3();
  BackgroundColor(v.x(), v.y(), v.z());
}

void OculusTextureBuffer::BackgroundColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {

#ifdef DEBUG_SCREEN
  std::cout << "MSG OculusTextureBuffer::BackgroundColor() called." << std::endl;
#endif // DEBUG_SCREEN

  this->bgColor[0] = r;
  this->bgColor[1] = g;
  this->bgColor[2] = b;
  this->bgColor[3] = a;
}

int OculusTextureBuffer::getTexureWidth() const { return texWidth; }
int OculusTextureBuffer::getTexureHeight() const { return texHeight; }

void OculusTextureBuffer::SetAndClearRenderSurface() {
  frame_index++;

  GLuint curColorTexId;
  GLuint curDepthTexId;
  int curColorIndex;
  int curDepthIndex;
  {
    // int curIndex;
    KVS_OVR_CALL(ovr_GetTextureSwapChainCurrentIndex(Session, ColorTextureChain, &curColorIndex));
    KVS_OVR_CALL(ovr_GetTextureSwapChainBufferGL(Session, ColorTextureChain, curColorIndex, &curColorTexId));
  }
  {
    // int curIndex;
    KVS_OVR_CALL(ovr_GetTextureSwapChainCurrentIndex(Session, DepthTextureChain, &curDepthIndex));
    KVS_OVR_CALL(ovr_GetTextureSwapChainBufferGL(Session, DepthTextureChain, curDepthIndex, &curDepthTexId));
  }

  // KVS_GL_CALL(fboId_old = kvs::OpenGL::Integer(GL_FRAMEBUFFER_BINDING));
  fboId_old = kvs::OpenGL::Integer(GL_FRAMEBUFFER_BINDING);

#ifdef ENABLE_SNAPSHOT
  colorBufferId = curColorTexId;
  depthBufferId = curDepthTexId;
#endif // ENABLE_SNAPSHOT

#ifdef DEBUG_SCREEN
  std::cout << "MSG OculusTextureBuffer::SetAndClearRenderSurface() switch fbo : " << fboId_old << " -> " << fboId << "(color=" << curColorTexId
            << "[" << curColorIndex << "], depth=" << curDepthTexId << "[" << curDepthIndex << "])" << std::endl;
#endif // DEBUG_SCREEN

  KVS_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fboId));
  KVS_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curColorTexId, 0));
  KVS_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, curDepthTexId, 0));

  KVS_GL_CALL(glViewport(0, 0, texWidth, texHeight));
  KVS_GL_CALL(glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]));
  KVS_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  // KVS_GL_CALL( glEnable (GL_FRAMEBUFFER_SRGB));
  KVS_GL_CALL(glDisable(GL_FRAMEBUFFER_SRGB));

  // Check and clear GL errors
  if (frame_index % 100 == 0) {
    int e;
    while ((e = glGetError()) != 0) {
      qCritical("OculusTextureBuffer::SetAndClearRenderSurface GL HAS ERROR : %d", e);
    }
  }

  {
    ovrErrorInfo errInfo;
    for (int ierr = 0;; ierr++) {
      ovr_GetLastErrorInfo(&errInfo);
      if (OVR_SUCCESS(errInfo.Result)) {
        break;
      }
      std::fprintf(stdout, "OculusTextureBuffer::SetAndClearRenderSurface() has OVR error [%d] %s\n", ierr, errInfo.ErrorString);
    }
  }
}

void OculusTextureBuffer::UnsetRenderSurface() {

#ifdef DEBUG_SCREEN
  std::cout << "MSG OculusTextureBuffer::UnsetRenderSurface() switch fbo : " << fboId << " -> " << fboId_old << std::endl;
#endif // DEBUG_SCREEN

  KVS_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fboId));
  KVS_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0));
  KVS_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0));

  KVS_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fboId_old));

#ifdef ENABLE_SNAPSHOT
  colorBufferId = 0;
  depthBufferId = 0;
#endif // ENABLE_SNAPSHOT

  if (frame_index % 100 == 0) {
    int e;
    while ((e = glGetError()) != 0) {
      qCritical("OculusTextureBuffer::UnsetRenderSurface GL HAS ERROR : %d", e);
    }
  }

  if (frame_index % 100 == 0) {
    ovrErrorInfo errInfo;
    for (int ierr = 0;; ierr++) {
      ovr_GetLastErrorInfo(&errInfo);
      if (OVR_SUCCESS(errInfo.Result)) {
        break;
      }
      std::fprintf(stdout, "OculusTextureBuffer::UnsetRenderSurface() has OVR error [%d] %s\n", ierr, errInfo.ErrorString);
    }
  }
}

void OculusTextureBuffer::Commit() {

#ifdef DEBUG_SCREEN
  std::cout << "MSG OculusTextureBuffer::Commit() called." << std::endl;
#endif // DEBUG_SCREEN

  KVS_OVR_CALL(ovr_CommitTextureSwapChain(Session, ColorTextureChain));
  KVS_OVR_CALL(ovr_CommitTextureSwapChain(Session, DepthTextureChain));

  if (frame_index % 100 == 0) {
    int e;
    while ((e = glGetError()) != 0) {
      qCritical("OculusTextureBuffer::Commit GL HAS ERROR : %d", e);
    }
  }

  if (frame_index % 100 == 0) {
    ovrErrorInfo errInfo;
    for (int ierr = 0;; ierr++) {
      ovr_GetLastErrorInfo(&errInfo);
      if (OVR_SUCCESS(errInfo.Result)) {
        break;
      }
      std::fprintf(stdout, "OculusTextureBuffer::Commit() has OVR error [%d] %s\n", ierr, errInfo.ErrorString);
    }
  }
}
