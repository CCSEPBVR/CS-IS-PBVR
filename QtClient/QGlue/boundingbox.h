#pragma once
#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
#include "Scene.h"
#include "lineobjectproxy.h"
#include "stochastictexturedpolygonrenderer.h"
#include "TexturedPolygonObjectProxy.h"

#include <kvs/Bmp>
#include <kvs/IDManager>
#include <kvs/LineObject>
#include <kvs/Matrix44>
#include <kvs/ObjectBase>
#include <kvs/PointObject>
#include <kvs/PolygonObject>
#include <kvs/PolygonRenderer>
#include <kvs/StochasticLineRenderer>
#include <kvs/Vector4>
#include <kvs/VolumeObjectBase>

#include <QDirIterator>
#include <QImage>
#include <cmath>
#include <map>
#include <set>

#define INITIAL_STATE_BOUNDING_BOX false

#define DEFAULT_NUM_DIVISION_OF_BOUNDING_BOX 5

class BoundingBox {
private:
  bool m_isShown;
  bool m_numberTexturesAreInitialized;
  std::map<int, std::string> m_index_filename_map;
  std::map<char, int> m_char_index_map;

  kvs::jaea::pbvr::Scene *m_scene;

  kvs::Vec4 m_direction;
  kvs::Vec4 m_upvector;

  // bounding box
  static LineObjectProxy m_bounding_box;
  static int m_bounding_box_id;
  kvs::StochasticLineRenderer *m_bounding_box_renderer;

  // bounding box submesh parameters.
  kvs::Real32 m_bounding_box_scaling_ratio_min;
  kvs::Real32 m_bounding_box_scaling_ratio_max;
  kvs::Real32 m_bounding_box_scaling_ratio;

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
  // submesh of bounding box
  static LineObjectProxy m_bounding_box_submesh;
  static int m_bounding_box_submesh_id;
  kvs::StochasticLineRenderer *m_bounding_box_submesh_renderer;

  // scale labels for submesh
  static TexturedPolygonObjectProxy m_bounding_box_labels;
  static int m_bounding_box_labels_id;
  StochasticTexturedPolygonRenderer *m_bounding_box_labels_renderer;
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE

private:
  virtual void initializeNumberTextureMaps();

protected:
  virtual void rebuildBoundingBox(bool force_rebuild_submesh);
  virtual void checkScalingRatio(kvs::Real32 scaling_ratio, bool force_update = false);

public:
  BoundingBox(kvs::jaea::pbvr::Scene *scene);
  ~BoundingBox();

  virtual void initializeEvent();
  virtual void initializeScaleLabel();
  virtual void frameEvent();
  virtual void show() { m_isShown = true; }
  virtual void hide() { m_isShown = false; }
  virtual bool isShown() { return m_isShown; }
  virtual void setDirection(kvs::Vec4 direction) { m_direction = direction; }
  virtual void setUpvector(kvs::Vec4 upvector) { m_upvector = upvector; }
};

#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX
