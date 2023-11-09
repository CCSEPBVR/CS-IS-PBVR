#pragma once
#ifndef __OCULUS_TEXTURE_BUFFER_H__
#define __OCULUS_TEXTURE_BUFFER_H__

#include <kvs/OpenGL>
#include "OVR_CAPI_GL.h"

struct OculusTextureBuffer {
  ovrSession Session;
  ovrTextureSwapChain ColorTextureChain;
  ovrTextureSwapChain DepthTextureChain;
  GLuint fboId;
  GLuint fboId_old;
  int texHeight;
  int texWidth;
  GLfloat bgColor[4];
  int m_sampleCount;

#ifdef ENABLE_SNAPSHOT
  GLuint colorBufferId;
  GLuint depthBufferId;
#endif // ENABLE_SNAPSHOT

  // std::vector<GLuint> colorTexId;
  // size_t nColorTex;
  // GLuint *colorTexId;

  OculusTextureBuffer(ovrSession session, /*Sizei size*/ int width, int height, int sampleCount);
  ~OculusTextureBuffer();

  void Create();
  void BackgroundColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f);
  void BackgroundColor(const kvs::RGBColor& col);
  int getTexureWidth() const;
  int getTexureHeight() const;
  void SetAndClearRenderSurface();
  void UnsetRenderSurface();
  void Commit();

}; // struct OculusTextureBuffer

#endif // ifndef __OCULUS_TEXTURE_BUFFER_H__
