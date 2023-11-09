/*****************************************************************************/
/**
 *  @file   stochastictexturedpolygonrenderer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#ifndef KVS__STOCHASTIC_TEXTURED_POLYGON_RENDERER_H_INCLUDE
#define KVS__STOCHASTIC_TEXTURED_POLYGON_RENDERER_H_INCLUDE

#include <kvs/IndexBufferObject>
#include <kvs/Module>
#include <kvs/ProgramObject>
#include <kvs/VertexBufferObject>
// #include <kvs/StochasticRenderingEngine>
#include <kvs/StochasticRendererBase>

#include "TexturedPolygonObject.h"
#include "stochasticrenderingenginecustom.h"

/*===========================================================================*/
/**
 *  @brief  Stochastic textured polygon renderer class.
 */
/*===========================================================================*/
class StochasticTexturedPolygonRenderer : public kvs::StochasticRendererBase {
  kvsModule(StochasticTexturedPolygonRenderer, Renderer);
  kvsModuleBaseClass(kvs::StochasticRendererBase);

public:
  class Engine;

public:
  StochasticTexturedPolygonRenderer();
  void setPolygonOffset(const float polygon_offset);

  void setEnabledTextureLinear(const bool isEnabled);
  void setEnabledTextureMipmap(const bool isEnabled);
  void setEnabledTextureMipmapLinear(const bool isEnabled);
  void setEnabledTextureAnisotropy(const bool isEnabled);
  void setNearestInterpolation();
  void setBilinearInterpolation();
  void setTrilinearInterpolation();
  void setAnisotropicFiltering();
};

/*===========================================================================*/
/**
 *  @brief  Engine class for stochastic textured polygon renderer.
 */
/*===========================================================================*/
class StochasticTexturedPolygonRenderer::Engine : public StochasticRenderingEngineCustom {
private:
  bool m_has_normal;                   ///< check flag for the normal array
  bool m_has_connection;               ///< check flag for the connection array
  size_t m_random_index;               ///< index used for refering the random texture
  size_t m_texture_2d_coord_index;     ///< index used for refering the texture 2d coords
  float m_polygon_offset;              ///< polygon offset
  kvs::ProgramObject m_shader_program; ///< shader program
  kvs::VertexBufferObject m_vbo;       ///< vertex buffer object
  // std::map<kvs::UInt32, kvs::IndexBufferObject> m_map_ibo; ///< index buffer object
  std::map<kvs::UInt32, size_t> m_map_num_vertices; ///< number of vertices

  std::map<kvs::UInt32, kvs::Texture2D *> m_map_id_to_texture; ///< map texture id to Texture2D

  bool m_enabled_texture_linear;
  bool m_enabled_texture_mipmap;
  bool m_enabled_texture_mipmap_linear;
  bool m_enabled_texture_anisotropy;

public:
  Engine();
  ~Engine();
  void release();
  void create(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light);
  void update(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light);
  void setup(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light);
  void draw(kvs::ObjectBase *object, kvs::Camera *camera, kvs::Light *light);

  void setEnabledTextureLinear(const bool isEnabled) { m_enabled_texture_linear = isEnabled; }
  void setEnabledTextureMipmap(const bool isEnabled) { m_enabled_texture_mipmap = isEnabled; }
  void setEnabledTextureMipmapLinear(const bool isEnabled) { m_enabled_texture_mipmap_linear = isEnabled; }
  void setEnabledTextureAnisotropy(const bool isEnabled) { m_enabled_texture_anisotropy = isEnabled; }

  void setPolygonOffset(const float offset) { m_polygon_offset = offset; }

private:
  void create_shader_program();
  void create_buffer_object(const kvs::jaea::TexturedPolygonObject *texturedPolygon);
};

#endif // KVS__STOCHASTIC_TEXTURED_POLYGON_RENDERER_H_INCLUDE
