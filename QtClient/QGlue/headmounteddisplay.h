#pragma once
#ifndef HEADMOUNTEDDISPLAY_H
#define HEADMOUNTEDDISPLAY_H

#include "oculus.h"
#include "oculustexturebuffer.h"
#include "version.h"
#include "widgethandler.h"

#include <iostream>
#include <kvs/FrameBufferObject>
#include <kvs/Indent>
#include <kvs/OpenGL>
#include <kvs/RenderBuffer>
#include <kvs/Texture2D>
#include <kvs/Type>
#include <kvs/Vector2>
#include <string>

#define INFO_OUTPUT_INDEX 10

class HeadMountedDisplay {
public:
#if KVS_OVR_VERSION_GREATER_OR_EQUAL(0, 8, 0)
  typedef ovrSession Handler;
#else
  typedef ovrHmd Handler;
#endif

#if KVS_OVR_VERSION_GREATER_OR_EQUAL(0, 7, 0)
  typedef ovrHmdDesc *Descriptor;
#else
  typedef ovrHmd Descriptor;
#endif

  // from OculusKVS (kvs::jaea::pbvr::Screen)
  enum MirrorBufferType { LeftEyeImage = 0, RightEyeImage, BothEyeImage, DistortedBothEyeImage, SizeOfEyeImage };

private:
  Handler m_handler;       ///< HMD device session handler
  Descriptor m_descriptor; ///< HMD device descriptor

  ovrEyeRenderDesc m_render_descs[2]; ///< rendering descriptors (rendering information of each eye)
  ovrPosef m_eye_poses[2];            ///< position and orientation of each eye
  ovrRecti m_viewports[2];            ///< viewport of each eye

  // HUD
  OculusTextureBuffer *hudTexture;
  WidgetHandler *m_widget_handler;

  kvs::RGBColor m_bgcolor;

  // // Rendering buffers.
  // ovrSizei m_buffer_size; ///< rendering buffer size
  // kvs::FrameBufferObject m_framebuffer; ///< frame buffer object
  // kvs::RenderBuffer m_depth_buffer; ///< depth buffer
  // #if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
  //     ovrLayerEyeFov m_layer_data; ///< frame layer data
  // #else
  //     kvs::Texture2D m_color_buffer; ///< color buffer
  //     ovrGLTexture m_color_textures[2]; ///< texture information of each eye
  // #endif

  OculusTextureBuffer *m_eyeRenderTexture[2] = {nullptr, nullptr};
  ovrTimewarpProjectionDesc m_posTimewarpProjectionDesc;
  double m_sensorSampleTime;

  // Mirror buffer.
  MirrorBufferType m_mirror_buffer_type; ///< mirror buffer type
  // GLuint m_mirror_fbo[4];
  GLuint m_mirror_fbo;
  GLuint m_mirror_fbo_old;
#if KVS_OVR_VERSION_GREATER_OR_EQUAL(1, 0, 0)
  // ovrMirrorTexture m_mirror_tex[SizeOfEyeImage];
  ovrMirrorTexture m_mirror_tex;
#else
  ovrGLTexture *m_mirror_tex;
#endif

public:
  HeadMountedDisplay();

  const Handler &handler() const { return m_handler; }
  const ovrEyeRenderDesc &renderDesc(const size_t eye_index) const { return m_render_descs[eye_index]; }
  const ovrPosef &eyePose(const size_t eye_index) const { return m_eye_poses[eye_index]; }
  const ovrRecti &viewport(const size_t eye_index) const { return m_viewports[eye_index]; }

  // Descriptor.
  std::string productName() const { return std::string(m_descriptor->ProductName); }
  std::string manufacturer() const { return std::string(m_descriptor->Manufacturer); }
  kvs::Int16 vendorId() const { return m_descriptor->VendorId; }
  kvs::Int16 productId() const { return m_descriptor->ProductId; }
  ovrFovPort defaultEyeFov(const size_t eye_index) const { return m_descriptor->DefaultEyeFov[eye_index]; }
  ovrFovPort maxEyeFov(const size_t eye_index) const { return m_descriptor->MaxEyeFov[eye_index]; }
  ovrSizei resolution() const { return m_descriptor->Resolution; }
  kvs::UInt32 availableHmdCaps() const;
  kvs::UInt32 availableTrackingCaps() const;
  kvs::UInt32 defaultHmdCaps() const;
  kvs::UInt32 defaultTrackingCaps() const;

  bool create(const int index = 0);
  void destroy();
  void print(std::ostream &os, const kvs::Indent &indent = kvs::Indent(0)) const;

  void setBackgroundColor(const kvs::RGBColor& col) { m_bgcolor = col; }

  // Rendering
  bool configureRendering();
  void beginFrame(const kvs::Int64 frame_index);
  void endFrame(const kvs::Int64 frame_index);
  double frameTiming(const kvs::Int64 frame_index) const;

  OculusTextureBuffer *eyeRenderTexture(int eye_index) const { return m_eyeRenderTexture[eye_index]; }

  // Rendering to mirror buffer
  //    void renderToMirrorLeftEyeImage();
  //    void renderToMirrorRightEyeImage();
  //    void renderToMirrorBothEyeImage();
  //    void renderToMirrorDistortedBothEyeImage();
  void renderToMirror();
  void setMirrorBufferType(const MirrorBufferType type) { m_mirror_buffer_type = type; }
  void setMirrorBufferTypeToLeftEyeImage() { this->setMirrorBufferType(LeftEyeImage); }
  void setMirrorBufferTypeToRightEyeImage() { this->setMirrorBufferType(RightEyeImage); }
  void setMirrorBufferTypeToBothEyeImage() { this->setMirrorBufferType(BothEyeImage); }
  void setMirrorBufferTypeToDistortedBothEyeImage() { this->setMirrorBufferType(DistortedBothEyeImage); }

  // Tracking
  bool configureTracking(const kvs::UInt32 supported_caps, const kvs::UInt32 required_caps);
  void resetTracking();
  ovrTrackingState trackingState(const double absolute_time = 0.0, const bool latency = true) const;
  ovrInputState inputState(const ovrControllerType type) const;

  // HUD
  void setWidgetHandler(WidgetHandler *wh);
  void updateHUD();
  bool initialize_hud_texture();

  // manipulator/laser
  const ovrPosef &Pose(const size_t eye_index) const { return m_eye_poses[eye_index]; }

private:
  bool initialize_render_texture();
  bool initialize_mirror_texture();
  void update_eye_poses(const kvs::Int64 frame_index);
  void update_viewport();
  // ovrSizei fov_texture_size( const ovrEyeType eye, const ovrFovPort fov, const float pixels_per_display_pixel );
  // void bind_mirror_buffer( const bool distorted = true );
  void bind_mirror_buffer();
  void unbind_mirror_buffer();
  void blit_mirror_buffer(const kvs::Vec4 mirror_viewport, const bool flip = false);
};

#endif // HEADMOUNTEDDISPLAY_H
