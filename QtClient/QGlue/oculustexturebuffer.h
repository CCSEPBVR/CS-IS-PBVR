/* HUDTest02/OculusTetureBuffer.h */
#pragma once
#ifndef __OCULUS_TEXTURE_BUFFER_H__
#define __OCULUS_TEXTURE_BUFFER_H__

////#include <QtOpenGL>
//#include <GL/glew.h>
#include <kvs/OpenGL>
//#include <windows.h>
//#include <GL/glew.h>
//#include <GL/GL.h>
#include "OVR_CAPI_GL.h"

//#include <cassert>

namespace kvs {
namespace oculus {
namespace jaea {

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

    //std::vector<GLuint> colorTexId;
    //size_t nColorTex;
    //GLuint *colorTexId;

    OculusTextureBuffer(ovrSession session, /*Sizei size*/int width, int height,
            int sampleCount);
    ~OculusTextureBuffer();

    void Create();
    void BackgroundColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f);
    int getTexureWidth() const;
    int getTexureHeight() const;
    void SetAndClearRenderSurface();
    void UnsetRenderSurface();
    void Commit();


//#if 0
//    OculusTextureBuffer(ovrSession session, /*Sizei size*/int width, int height,
//            int sampleCount) :
//            Session(session), ColorTextureChain(nullptr), DepthTextureChain(
//                    nullptr), fboId(0),fboId_old(0), texWidth(0), texHeight(0) {
//        assert(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

//        texWidth = width;
//        texHeight = height;

//        // This texture isn't necessarily going to be a rendertarget, but it usually is.
//        assert(session); // No HMD? A little odd.

//        ovrTextureSwapChainDesc desc = { };
//        desc.Type = ovrTexture_2D;
//        desc.ArraySize = 1;
//        desc.Width = width;
//        desc.Height = height;
//        desc.MipLevels = 1;
//        desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
//        desc.SampleCount = sampleCount;
//        desc.StaticImage = ovrFalse;

//        {
//            ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc,
//                    &ColorTextureChain);

//            int length = 0;
//            ovr_GetTextureSwapChainLength(session, ColorTextureChain, &length);

//            if (OVR_SUCCESS(result)) {
//                // nColorTex = length;
//                // colorTexId = new GLuint[length];

//                for (int i = 0; i < length; ++i) {
//                    GLuint chainTexId;
//                    ovr_GetTextureSwapChainBufferGL(Session, ColorTextureChain,
//                            i, &chainTexId);
//                    glBindTexture(GL_TEXTURE_2D, chainTexId);

//                    // colorTexId[i] = chainTexId;

//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
//                    GL_LINEAR);
//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
//                    GL_LINEAR);
//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
//                            GL_CLAMP_TO_EDGE);
//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
//                            GL_CLAMP_TO_EDGE);
//                }
//            }
//        }

//        desc.Format = OVR_FORMAT_D32_FLOAT;

//        {
//            ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc,
//                    &DepthTextureChain);

//            int length = 0;
//            ovr_GetTextureSwapChainLength(session, DepthTextureChain, &length);

//            if (OVR_SUCCESS(result)) {
//                for (int i = 0; i < length; ++i) {
//                    GLuint chainTexId;
//                    ovr_GetTextureSwapChainBufferGL(Session, DepthTextureChain,
//                            i, &chainTexId);
//                    glBindTexture(GL_TEXTURE_2D, chainTexId);

//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
//                    GL_LINEAR);
//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
//                    GL_LINEAR);
//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
//                            GL_CLAMP_TO_EDGE);
//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
//                            GL_CLAMP_TO_EDGE);
//                }
//            }
//        }

//        glGenFramebuffers(1, &fboId);

//        // default background color = white
//        bgColor[0] = 1.0;
//        bgColor[1] = 1.0;
//        bgColor[2] = 1.0;
//        bgColor[3] = 1.0;
//    }

//    ~OculusTextureBuffer() {
//        if (ColorTextureChain) {
//            ovr_DestroyTextureSwapChain(Session, ColorTextureChain);
//            ColorTextureChain = nullptr;
//        }
//        if (DepthTextureChain) {
//            ovr_DestroyTextureSwapChain(Session, DepthTextureChain);
//            DepthTextureChain = nullptr;
//        }
//        if (fboId) {
//            glDeleteFramebuffers(1, &fboId);
//            fboId = 0;
//        }
//    }

//    void BackgroundColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f) {
//        this->bgColor[0] = r;
//        this->bgColor[1] = g;
//        this->bgColor[2] = b;
//        this->bgColor[3] = a;
//    }

//    int getTexureWidth() const {
//        return texWidth;
//    }
//    int getTexureHeight() const {
//        return texHeight;
//    }

//    void SetAndClearRenderSurface() {
//        GLuint curColorTexId;
//        GLuint curDepthTexId;
//        {
//            int curIndex;
//            ovr_GetTextureSwapChainCurrentIndex(Session, ColorTextureChain,
//                    &curIndex);
//            ovr_GetTextureSwapChainBufferGL(Session, ColorTextureChain,
//                    curIndex, &curColorTexId);
//        }
//        {
//            int curIndex;
//            ovr_GetTextureSwapChainCurrentIndex(Session, DepthTextureChain,
//                    &curIndex);
//            ovr_GetTextureSwapChainBufferGL(Session, DepthTextureChain,
//                    curIndex, &curDepthTexId);
//        }

//        fboId_old= kvs::OpenGL::Integer( GL_FRAMEBUFFER_BINDING );

//        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//        GL_TEXTURE_2D, curColorTexId, 0);
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
//        GL_TEXTURE_2D, curDepthTexId, 0);

//        glViewport(0, 0, texWidth, texHeight);
//        glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glEnable (GL_FRAMEBUFFER_SRGB);
//    }

//    void UnsetRenderSurface() {
//        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//        GL_TEXTURE_2D, 0, 0);
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
//        GL_TEXTURE_2D, 0, 0);

//        glBindFramebuffer(GL_FRAMEBUFFER, fboId_old);
//    }

//    void Commit() {
//        ovr_CommitTextureSwapChain(Session, ColorTextureChain);
//        ovr_CommitTextureSwapChain(Session, DepthTextureChain);
//    }
//#endif // 0
}; // struct OculusTextureBuffer
} // namespace jaea
} // namespace oculus
} // namespace kvs
#endif //ifndef __OCULUS_TEXTURE_BUFFER_H__
