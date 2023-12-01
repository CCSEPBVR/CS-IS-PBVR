#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX
#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "boundingbox.h"
#include "TimeMeasureUtility.h"
#include "objnameutil.h"
#include "stochastictexturedpolygonrenderer.h"
#include "TexturedPolygonObjectProxy.h"

#ifdef DISP_MODE_2D
#include "screen.h"
#endif
#ifdef DISP_MODE_VR
#include "objectscale.h"
#endif

LineObjectProxy BoundingBox::m_bounding_box;
int BoundingBox::m_bounding_box_id;

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
LineObjectProxy BoundingBox::m_bounding_box_submesh;
int BoundingBox::m_bounding_box_submesh_id;

TexturedPolygonObjectProxy BoundingBox::m_bounding_box_labels;
int BoundingBox::m_bounding_box_labels_id;
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE

BoundingBox::BoundingBox(kvs::jaea::pbvr::Scene *scene)
    : m_numberTexturesAreInitialized(false), m_bounding_box_scaling_ratio_min(1.0f), m_bounding_box_scaling_ratio_max(2.0f),
      m_bounding_box_scaling_ratio(1.0f), m_direction(kvs::Vec4(0.0f, 0.0f, -1.0f, 0.0f)), m_upvector(kvs::Vec4(0.0f, 1.0f, 0.0f, 0.0f)),
      m_isShown(INITIAL_STATE_BOUNDING_BOX), m_scene(nullptr) {
  m_scene = scene;
}

BoundingBox::~BoundingBox() {}

void BoundingBox::initializeNumberTextureMaps() {
  m_char_index_map['0'] = 0;
  m_char_index_map['1'] = 1;
  m_char_index_map['2'] = 2;
  m_char_index_map['3'] = 3;
  m_char_index_map['4'] = 4;
  m_char_index_map['5'] = 5;
  m_char_index_map['6'] = 6;
  m_char_index_map['7'] = 7;
  m_char_index_map['8'] = 8;
  m_char_index_map['9'] = 9;
  m_char_index_map['.'] = 10;
  m_char_index_map['+'] = 11;
  m_char_index_map['-'] = 12;
  m_char_index_map['e'] = 13;
  m_char_index_map['E'] = 13;

  // index filename_map
  m_index_filename_map[0] = ":/Resources/scale_images/num_0.bmp";
  m_index_filename_map[1] = ":/Resources/scale_images/num_1.bmp";
  m_index_filename_map[2] = ":/Resources/scale_images/num_2.bmp";
  m_index_filename_map[3] = ":/Resources/scale_images/num_3.bmp";
  m_index_filename_map[4] = ":/Resources/scale_images/num_4.bmp";
  m_index_filename_map[5] = ":/Resources/scale_images/num_5.bmp";
  m_index_filename_map[6] = ":/Resources/scale_images/num_6.bmp";
  m_index_filename_map[7] = ":/Resources/scale_images/num_7.bmp";
  m_index_filename_map[8] = ":/Resources/scale_images/num_8.bmp";
  m_index_filename_map[9] = ":/Resources/scale_images/num_9.bmp";
  m_index_filename_map[10] = ":/Resources/scale_images/char_dot.bmp";
  m_index_filename_map[11] = ":/Resources/scale_images/char_plus.bmp";
  m_index_filename_map[12] = ":/Resources/scale_images/char_minus.bmp";
  m_index_filename_map[13] = ":/Resources/scale_images/char_e.bmp";
}

void BoundingBox::rebuildBoundingBox(bool force_rebuild_submesh) {
  MAKE_AND_START_TIMER(A_4_1_4_2);

  kvs::Vec4 direction_global = m_direction;
  kvs::Vec4 upvector_global = m_upvector;

  // direction & upvector in model coordinate system.
  kvs::Xform obj_xform = m_scene->objectManager()->xform();
  kvs::Mat4 obj_rotation = obj_xform.toMatrix();
  obj_rotation.invert();
  kvs::Vec3 direction_model = ((obj_rotation * direction_global).xyz()).normalized();
  kvs::Vec3 upvector_model = ((obj_rotation * upvector_global).xyz()).normalized();

  kvs::Vec3 right_model = (direction_model.cross(upvector_model)).normalized();
  bool bb_initialized = false;
  kvs::Vec3 min_coord(-3, -3, -3);
  kvs::Vec3 max_coord(3, 3, 3);

#if defined(BOUNDING_BOX_USE_OBJECT_COORD)
  // Object coord
  for (int iobj = 0; iobj < m_scene->objectManager()->numberOfObjects(); iobj++) {
    kvs::ObjectBase *obj = m_scene->objectManager()->object(iobj);
    if (obj != nullptr && !ObjectNameUtil::isToIgnoreBoundingBox(obj->name())) {
      if (bb_initialized) {
        kvs::Vec3 min_obj = obj->minObjectCoord();
        kvs::Vec3 max_obj = obj->maxObjectCoord();
        min_coord.x() = (min_obj.x() < min_coord.x()) ? min_obj.x() : min_coord.x();
        min_coord.y() = (min_obj.y() < min_coord.y()) ? min_obj.y() : min_coord.y();
        min_coord.z() = (min_obj.z() < min_coord.z()) ? min_obj.z() : min_coord.z();
        max_coord.x() = (max_obj.x() > max_coord.x()) ? max_obj.x() : max_coord.x();
        max_coord.y() = (max_obj.y() > max_coord.y()) ? max_obj.y() : max_coord.y();
        max_coord.z() = (max_obj.z() > max_coord.z()) ? max_obj.z() : max_coord.z();
      } else {
        min_coord = obj->minObjectCoord();
        max_coord = obj->maxObjectCoord();
        bb_initialized = true;
      }
    }
  }
  if (!bb_initialized) {
    min_coord = m_scene->objectManager()->minObjectCoord();
    max_coord = m_scene->objectManager()->maxObjectCoord();
  }
#ifdef DEBUG_BBOX
  std::cout << "##### rebuildBoundingBox by Object Coord" << std::endl;
  std::cout << "minX=" << min_coord.x() << ", minY=" << min_coord.y() << ", minZ=" << min_coord.z() << std::endl;
  std::cout << "maxX=" << max_coord.x() << ", maxY=" << max_coord.y() << ", maxZ=" << max_coord.z() << std::endl;
#endif // DEBUG_BBOX
#elif defined(BOUNDING_BOX_USE_EXTRENAL_COORD)
  // Ext coord
  for (int iobj = 0; iobj < m_scene->objectManager()->numberOfObjects(); iobj++) {
    kvs::ObjectBase *obj = m_scene->objectManager()->object(iobj);
    if (obj != nullptr && !ObjectNameUtil::isToIgnoreBoundingBox(obj->name())) {
      if (bb_initialized) {
        kvs::Vec3 min_obj = obj->minExternalCoord();
        kvs::Vec3 max_obj = obj->maxExternalCoord();
        min_coord.x() = (min_obj.x() < min_coord.x()) ? min_obj.x() : min_coord.x();
        min_coord.y() = (min_obj.y() < min_coord.y()) ? min_obj.y() : min_coord.y();
        min_coord.z() = (min_obj.z() < min_coord.z()) ? min_obj.z() : min_coord.z();
        max_coord.x() = (max_obj.x() > max_coord.x()) ? max_obj.x() : max_coord.x();
        max_coord.y() = (max_obj.y() > max_coord.y()) ? max_obj.y() : max_coord.y();
        max_coord.z() = (max_obj.z() > max_coord.z()) ? max_obj.z() : max_coord.z();
      } else {
        min_coord = obj->minExternalCoord();
        max_coord = obj->maxExternalCoord();
        bb_initialized = true;
      }
    }
  }
  if (!bb_initialized) {
    min_coord = m_scene->objectManager()->minObjectCoord();
    max_coord = m_scene->objectManager()->maxObjectCoord();
  }
#ifdef DEBUG_BBOX
  std::cout << "##### rebuildBoundingBox by External Coord" << std::endl;
  std::cout << "minX=" << min_coord.x() << ", minY=" << min_coord.y() << ", minZ=" << min_coord.z() << std::endl;
  std::cout << "maxX=" << max_coord.x() << ", maxY=" << max_coord.y() << ", maxZ=" << max_coord.z() << std::endl;
#endif // DEBUG_BBOX
#else
  // Point coord
  for (int iobj = 0; iobj < m_scene->objectManager()->numberOfObjects(); iobj++) {
    kvs::ObjectBase *obj = m_scene->objectManager()->object(iobj);
    if (obj != nullptr && !ObjectNameUtil::isToIgnoreBoundingBox(obj->name())) {
      kvs::PointObject *pointObj = dynamic_cast<kvs::PointObject *>(obj);
      const kvs::VolumeObjectBase::Coords &coords = pointObj->coords();
      const int nPoints = coords.size() / 3;

      for (int ipoint = 0; ipoint < nPoints; ipoint++) {
        kvs::Real32 x = coords[3 * ipoint];
        kvs::Real32 y = coords[3 * ipoint + 1];
        kvs::Real32 z = coords[3 * ipoint + 2];

        if (bb_initialized) {
          min_coord.x() = (x < min_coord.x()) ? x : min_coord.x();
          min_coord.y() = (y < min_coord.y()) ? y : min_coord.y();
          min_coord.z() = (z < min_coord.z()) ? z : min_coord.z();
          max_coord.x() = (x > max_coord.x()) ? x : max_coord.x();
          max_coord.y() = (y > max_coord.y()) ? y : max_coord.y();
          max_coord.z() = (z > max_coord.z()) ? z : max_coord.z();
        } else {
          min_coord.x() = x;
          min_coord.y() = y;
          min_coord.z() = z;
          max_coord.x() = x;
          max_coord.y() = y;
          max_coord.z() = z;
          bb_initialized = true;
        }
      }
    }
  }
  if (!bb_initialized) {
    min_coord = m_scene->objectManager()->minObjectCoord();
    max_coord = m_scene->objectManager()->maxObjectCoord();
  }
#ifdef DEBUG_BBOX
  std::cout << "##### rebuildBoundingBox by Particle Coord" << std::endl;
  std::cout << "minX=" << min_coord.x() << ", minY=" << min_coord.y() << ", minZ=" << min_coord.z() << std::endl;
  std::cout << "maxX=" << max_coord.x() << ", maxY=" << max_coord.y() << ", maxZ=" << max_coord.z() << std::endl;
#endif // DEBUG_BBOX
#endif

#ifdef DEBUG_BBOX
  std::cerr << "TouchController::rebuildBoundingBox(): bb_initialized=" << bb_initialized << "/ min=" << min_coord << ", max=" << max_coord
            << std::endl;
#endif // DEBUG_BBOX

  // #ifdef DEBUG_TOUCH_CONTROLLER
  //     std::cout << "bounding box : min=" << min_coord << " : max=" << max_coord << std::endl;
  // #endif // DEBUG_TOUCH_CONTROLLER

  // coordinates of nodes
  kvs::Real32 minx = min_coord.x();
  kvs::Real32 miny = min_coord.y();
  kvs::Real32 minz = min_coord.z();
  kvs::Real32 maxx = max_coord.x();
  kvs::Real32 maxy = max_coord.y();
  kvs::Real32 maxz = max_coord.z();
  kvs::Real32 rangex = maxx - minx;
  kvs::Real32 rangey = maxy - miny;
  kvs::Real32 rangez = maxz - minz;
  kvs::Real32 range = rangex;
  if (range < rangey)
    range = rangey;
  if (range < rangez)
    range = rangez;

  // #ifdef DEBUG_TOUCH_CONTROLLER
  //     std::cout << "bounding box : range=" << range << " : rangex=" << rangex << " : rangey=" << rangey << " : rangez=" << rangez << std::endl;
  // #endif // DEBUG_TOUCH_CONTROLLER

  const static int nDivBasic = DEFAULT_NUM_DIVISION_OF_BOUNDING_BOX;
  float standardLength = range / nDivBasic;
  if (m_bounding_box_scaling_ratio > 0) {
    standardLength = range / (m_bounding_box_scaling_ratio * nDivBasic);
#ifdef DISP_MODE_VR
    standardLength *= OBJECT_SCALING_FACTOR;
#endif
  }
  if (standardLength <= 0) {
    standardLength = 1.0f;
  }

  int nDivX = (int)(rangex / standardLength);
  int nDivY = (int)(rangey / standardLength);
  int nDivZ = (int)(rangez / standardLength);
  if (nDivX <= 0)
    nDivX = 1;
  if (nDivY <= 0)
    nDivY = 1;
  if (nDivZ <= 0)
    nDivZ = 1;
#ifdef DEBUG_TOUCH_CONTROLLER
  if (force_rebuild_submesh) {
    std::cout << "bounding box : standardLength=" << standardLength << std::endl;
    std::cout << "bounding box : min=" << minx << ", " << miny << ", " << minz << std::endl;
    std::cout << "bounding box : max=" << maxx << ", " << maxy << ", " << maxz << std::endl;
    std::cout << "bounding box : range=" << range << " : rangex=" << rangex << " : rangey=" << rangey << " : rangez=" << rangez << std::endl;
    std::cerr << "bounding box : nDivX=" << nDivX << " : nDivY=" << nDivY << " : nDivZ=" << nDivZ << std::endl;
  }
#endif // DEBUG_TOUCH_CONTROLLER

#ifdef DEBUG_BBOX
  std::cerr << "bounding box : nDivX=" << nDivX << " : nDivY=" << nDivY << " : nDivZ=" << nDivZ << std::endl;
#endif // DEBUG_BBOX

  // #ifdef DEBUG_TOUCH_CONTROLLER
  //     std::cout << "bounding box : nDivX=" << nDivX << " : nDivY=" << nDivY << " : nDivZ=" << nDivZ << std::endl;
  // #endif // DEBUG_TOUCH_CONTROLLER

  // make tiny gap
  minx -= range * 0.0001f;
  miny -= range * 0.0001f;
  minz -= range * 0.0001f;
  maxx += range * 0.0001f;
  maxy += range * 0.0001f;
  maxz += range * 0.0001f;

  // scale label size
  float characterWidth = range * 0.04f;
  float characterHeight = range * 0.05f;
  //    float scaleLabelNomal_x = -direction_model.x();
  //    float scaleLabelNomal_y = -direction_model.y();
  //    float scaleLabelNomal_z = -direction_model.z();
  float scaleLabelNomal_x = direction_model.x();
  float scaleLabelNomal_y = direction_model.y();
  float scaleLabelNomal_z = direction_model.z();

  // number of nodes
  const size_t nNodes = 8;
  const size_t nNodesOnAxis = (nDivX - 1) + (nDivY - 1) + (nDivZ - 1);
  const size_t nNodesSubmesh = nNodesOnAxis * 3;

  // number of edges
  const size_t nEdges = 12;
  const size_t nEdgesSubmesh = nNodesOnAxis * 2;

  const kvs::Real32 box_coords_array[nNodes * 3] = {
      //
      minx, miny, minz, // node 0
      maxx, miny, minz, // node 1
      maxx, maxy, minz, // node 2
      minx, maxy, minz, // node 3
      //
      minx, miny, maxz, // node 4
      maxx, miny, maxz, // node 5
      maxx, maxy, maxz, // node 6
      minx, maxy, maxz, // node 7
  };
  const kvs::UInt32 box_connections_array[nEdges * 2] = {
      0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7,
  };
  const static kvs::UInt8 colorEdgeR = 0;
  const static kvs::UInt8 colorEdgeG = 0;
  const static kvs::UInt8 colorEdgeB = 255;
  const static kvs::UInt8 colorEdgeSubmeshColorR = 50;
  const static kvs::UInt8 colorEdgeSubmeshColorG = 50;
  const static kvs::UInt8 colorEdgeSubmeshColorB = 50;
  const static kvs::UInt8 colorScaleLabelR = 0;
  const static kvs::UInt8 colorScaleLabelG = 0;
  const static kvs::UInt8 colorScaleLabelB = 255;
  kvs::Real32 lineThickness = 10.0f;
  kvs::Real32 lineThicknessSubmesh = 1.0f;

  kvs::ValueArray<kvs::Real32> box_coords(nNodes * 3);
  kvs::ValueArray<kvs::UInt32> box_connections(nEdges * 2);

  // change length of box_colors : 2021/06/17 nakamura
  // kvs::ValueArray<kvs::UInt8> box_colors(nEdges * 3);
  kvs::ValueArray<kvs::UInt8> box_colors(3);
  box_colors[0] = colorEdgeR;
  box_colors[1] = colorEdgeG;
  box_colors[2] = colorEdgeB;

  if (force_rebuild_submesh) {
    for (size_t i = 0; i < nNodes * 3; i++) {
      box_coords[i] = box_coords_array[i];
    }
    for (size_t i = 0; i < nEdges * 2; i++) {
      box_connections[i] = box_connections_array[i];
    }
    // disabled : 2021/06/17 nakamura
    // for(size_t i = 0; i < nEdges; i++){
    //    box_colors[i*3+0] = colorEdgeR;
    //    box_colors[i*3+1] = colorEdgeG;
    //    box_colors[i*3+2] = colorEdgeB;
    //}
  }

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
  // submesh
  kvs::ValueArray<kvs::Real32> submesh_coords(nNodesSubmesh * 3);
  kvs::ValueArray<kvs::UInt32> submesh_connections(nEdgesSubmesh * 2);

  // change length of submesh_colors : 2021/06/17 nakamura
  // kvs::ValueArray<kvs::UInt8> submesh_colors(nEdgesSubmesh * 3);
  kvs::ValueArray<kvs::UInt8> submesh_colors(3);
  submesh_colors[0] = colorEdgeSubmeshColorR;
  submesh_colors[1] = colorEdgeSubmeshColorG;
  submesh_colors[2] = colorEdgeSubmeshColorB;
  if (force_rebuild_submesh) {
    // disabled : 2021/06/17 nakamura
    // for(size_t i = 0; i < nEdgesSubmesh; i++){
    //    submesh_colors[i*3+0] = colorEdgeSubmeshColorR;
    //    submesh_colors[i*3+1] = colorEdgeSubmeshColorG;
    //    submesh_colors[i*3+2] = colorEdgeSubmeshColorB;
    //}
  }

  // value to display
  kvs::ValueArray<kvs::Real32> scale_values(nNodesOnAxis);

  // scale label
  //  TODO : replace to kvs::ValueArray<*> !!!
  std::vector<kvs::Real32> v_scale_label_coords;
  std::vector<kvs::UInt32> v_scale_label_connections;
  std::vector<kvs::Real32> v_scale_label_normals;
  std::vector<kvs::UInt8> v_scale_label_colors;
  std::vector<kvs::Real32> v_scale_label_texuture2DCoords;
  std::vector<kvs::UInt32> v_scale_label_textureIds;

  size_t offset_node = 0;
  size_t offset_edge = 0;
  size_t offset_scale_label_node = 0;

  for (int i = 0; i < (nDivX - 1) + (nDivY - 1) + (nDivZ - 1); i++) {
    float x0 = minx, y0 = miny, z0 = minz;
    float x1 = x0, y1 = y0, z1 = z0;
    float x2 = x0, y2 = y0, z2 = z0;
    bool x_flag = false;
    bool y_flag = false;
    bool z_flag = false;
    if (i < (nDivX - 1)) {
      int index = i + 1;
      // x
      x0 = minx + rangex * index / nDivX;
      y0 = miny;
      z0 = minz;
      x1 = x0;
      y1 = maxy;
      z1 = z0;
      x2 = x0;
      y2 = y0;
      z2 = maxz;
      x_flag = true;
    } else if (i < (nDivX - 1) + (nDivY - 1)) {
      int index = i - (nDivX - 1) + 1;
      // y
      x0 = minx;
      y0 = miny + rangey * index / nDivY;
      z0 = minz;
      x1 = maxx;
      y1 = y0;
      z1 = z0;
      x2 = x0;
      y2 = y0;
      z2 = maxz;
      y_flag = true;
    } else {
      int index = i - ((nDivX - 1) + (nDivY - 1)) + 1;
      // z
      x0 = minx;
      y0 = miny;
      z0 = minz + rangez * index / nDivZ;
      x1 = maxx;
      y1 = y0;
      z1 = z0;
      x2 = x0;
      y2 = maxy;
      z2 = z0;
      z_flag = true;
    }

    if (x_flag == true) {
      scale_values[i] = x0;
    } else if (y_flag == true) {
      scale_values[i] = y0;
    } else {
      scale_values[i] = z0;
    }

    kvs::Vec3 scale_base(x0, y0, z0);
    std::string strScaleLabel = std::to_string(scale_values[i]);
    int lengthScaleLabel = strScaleLabel.length();

    // #ifdef DEBUG_TOUCH_CONTROLLER
    //         std::cout << "### scale label : " << strScaleLabel << std::endl;
    // #endif // DEBUG_TOUCH_CONTROLLER

    if (force_rebuild_submesh) {
      // ##################################
      // submesh
      int node0 = offset_node + 0;
      int node1 = offset_node + 1;
      int node2 = offset_node + 2;
      offset_node += 3;

      submesh_coords[node0 * 3 + 0] = x0;
      submesh_coords[node0 * 3 + 1] = y0;
      submesh_coords[node0 * 3 + 2] = z0;
      submesh_coords[node1 * 3 + 0] = x1;
      submesh_coords[node1 * 3 + 1] = y1;
      submesh_coords[node1 * 3 + 2] = z1;
      submesh_coords[node2 * 3 + 0] = x2;
      submesh_coords[node2 * 3 + 1] = y2;
      submesh_coords[node2 * 3 + 2] = z2;

      submesh_connections[offset_edge * 2 + 0] = node0;
      submesh_connections[offset_edge * 2 + 1] = node1;
      offset_edge++;
      submesh_connections[offset_edge * 2 + 0] = node0;
      submesh_connections[offset_edge * 2 + 1] = node2;
      offset_edge++;
    }

#ifndef UPDATE_FRAME_LABELS_ALWAYS
    if (force_rebuild_submesh)
#endif
    {
      // ##################################
      // scale label
      for (int iChar = 0; iChar < lengthScaleLabel; iChar++) {
        char currentCharacter = strScaleLabel[iChar];
        int indexCharacter = m_char_index_map[currentCharacter];

        int scale_label_node0 = offset_scale_label_node + 0;
        int scale_label_node1 = offset_scale_label_node + 1;
        int scale_label_node2 = offset_scale_label_node + 2;
        int scale_label_node3 = offset_scale_label_node + 3;
        offset_scale_label_node += 4;

        kvs::Vec3 v0 = scale_base - characterHeight * upvector_model - (lengthScaleLabel - iChar) * right_model * characterWidth;
        kvs::Vec3 v1 = v0 + characterHeight * upvector_model;
        kvs::Vec3 v2 = v0 + characterWidth * right_model;
        kvs::Vec3 v3 = v1 + characterWidth * right_model;
        // kvs::Vec3 v3 = v2 + characterHeight * upvector_model;

        // set fixed point
        v_scale_label_coords.push_back(v0.x());
        v_scale_label_coords.push_back(v0.y());
        v_scale_label_coords.push_back(v0.z());

        v_scale_label_coords.push_back(v1.x());
        v_scale_label_coords.push_back(v1.y());
        v_scale_label_coords.push_back(v1.z());

        v_scale_label_coords.push_back(v2.x());
        v_scale_label_coords.push_back(v2.y());
        v_scale_label_coords.push_back(v2.z());

        v_scale_label_coords.push_back(v3.x());
        v_scale_label_coords.push_back(v3.y());
        v_scale_label_coords.push_back(v3.z());

        // set colors
        v_scale_label_colors.push_back(colorScaleLabelR);
        v_scale_label_colors.push_back(colorScaleLabelG);
        v_scale_label_colors.push_back(colorScaleLabelB);
        v_scale_label_colors.push_back(colorScaleLabelR);
        v_scale_label_colors.push_back(colorScaleLabelG);
        v_scale_label_colors.push_back(colorScaleLabelB);
        v_scale_label_colors.push_back(colorScaleLabelR);
        v_scale_label_colors.push_back(colorScaleLabelG);
        v_scale_label_colors.push_back(colorScaleLabelB);
        v_scale_label_colors.push_back(colorScaleLabelR);
        v_scale_label_colors.push_back(colorScaleLabelG);
        v_scale_label_colors.push_back(colorScaleLabelB);

        // set texture
        // set x, y
        // 1
        v_scale_label_texuture2DCoords.push_back(0.0);
        v_scale_label_texuture2DCoords.push_back(1.0);

        // 0
        v_scale_label_texuture2DCoords.push_back(0.0);
        v_scale_label_texuture2DCoords.push_back(0.0);

        // 3
        v_scale_label_texuture2DCoords.push_back(1.0);
        v_scale_label_texuture2DCoords.push_back(1.0);

        // 2
        v_scale_label_texuture2DCoords.push_back(1.0);
        v_scale_label_texuture2DCoords.push_back(0.0);

        // set texture ids
        v_scale_label_textureIds.push_back(indexCharacter);
        v_scale_label_textureIds.push_back(indexCharacter);
        v_scale_label_textureIds.push_back(indexCharacter);
        v_scale_label_textureIds.push_back(indexCharacter);

        // triangle 0
        v_scale_label_connections.push_back(scale_label_node0);
        v_scale_label_connections.push_back(scale_label_node2);
        v_scale_label_connections.push_back(scale_label_node1);
        v_scale_label_normals.push_back(scaleLabelNomal_x);
        v_scale_label_normals.push_back(scaleLabelNomal_y);
        v_scale_label_normals.push_back(scaleLabelNomal_z);

        // triangle 1
        v_scale_label_connections.push_back(scale_label_node2);
        v_scale_label_connections.push_back(scale_label_node3);
        v_scale_label_connections.push_back(scale_label_node1);
        v_scale_label_normals.push_back(scaleLabelNomal_x);
        v_scale_label_normals.push_back(scaleLabelNomal_y);
        v_scale_label_normals.push_back(scaleLabelNomal_z);
      }
    }
  }
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE

  if (force_rebuild_submesh) {
    // call my_scene->replaceObject() : 2021/06/17 nakamura
    m_bounding_box.swap();
    // m_bounding_box->clear();
    // m_bounding_box->setName(OBJ_NAME_BOUNDING_BOX);0
    m_bounding_box->setCoords(box_coords);
    m_bounding_box->setConnections(box_connections);
    m_bounding_box->setColors(box_colors);
    m_bounding_box->setSize(lineThickness);
    m_scene->replaceObject(m_bounding_box_id, m_bounding_box, false);
    //        my_scene->replaceObject(OBJ_NAME_BOUNDING_BOX, m_bounding_box, false);

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
    // call my_scene->replaceObject() : 2021/06/17 nakamura
    m_bounding_box_submesh.swap();
    // m_bounding_box_submesh->clear();
    // m_bounding_box_submesh->setName(OBJ_NAME_BOUNDING_BOX_SUBMESH);
    m_bounding_box_submesh->setCoords(submesh_coords);
    m_bounding_box_submesh->setConnections(submesh_connections);
    m_bounding_box_submesh->setColors(submesh_colors);
    m_bounding_box_submesh->setSize(lineThicknessSubmesh);
    m_scene->replaceObject(m_bounding_box_submesh_id, m_bounding_box_submesh, false);
//        my_scene->replaceObject(OBJ_NAME_BOUNDING_BOX_SUBMESH, m_bounding_box_submesh, false);
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
  }

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
#ifndef UPDATE_FRAME_LABELS_ALWAYS
  if (force_rebuild_submesh)
#endif
  {
    // vector to array
    //        kvs::ValueArray<kvs::Real32> scale_label_coords(v_scale_label_coords);
    //        kvs::ValueArray<kvs::UInt32> scale_label_connections(v_scale_label_connections);
    //        kvs::ValueArray<kvs::Real32> scale_label_normals(v_scale_label_normals);
    //        kvs::ValueArray<kvs::UInt8>  scale_label_colors(v_scale_label_colors);
    //        kvs::ValueArray<kvs::Real32> scale_label_texuture2DCoords(v_scale_label_texuture2DCoords);
    //        kvs::ValueArray<kvs::UInt32> scale_label_textureIds(v_scale_label_textureIds);
    kvs::ValueArray<kvs::Real32> scale_label_coords(v_scale_label_coords.size());
    kvs::ValueArray<kvs::UInt32> scale_label_connections(v_scale_label_connections.size());
    kvs::ValueArray<kvs::Real32> scale_label_normals(v_scale_label_normals.size());
    kvs::ValueArray<kvs::UInt8> scale_label_colors(v_scale_label_colors.size());
    kvs::ValueArray<kvs::Real32> scale_label_texuture2DCoords(v_scale_label_texuture2DCoords.size());
    kvs::ValueArray<kvs::UInt32> scale_label_textureIds(v_scale_label_textureIds.size());
    for (size_t i = 0; i < v_scale_label_coords.size(); i++) {
      scale_label_coords[i] = v_scale_label_coords[i];
    }
    for (size_t i = 0; i < v_scale_label_connections.size(); i++) {
      scale_label_connections[i] = v_scale_label_connections[i];
    }
    for (size_t i = 0; i < v_scale_label_normals.size(); i++) {
      scale_label_normals[i] = v_scale_label_normals[i];
    }
    for (size_t i = 0; i < v_scale_label_colors.size(); i++) {
      scale_label_colors[i] = v_scale_label_colors[i];
    }
    for (size_t i = 0; i < v_scale_label_texuture2DCoords.size(); i++) {
      scale_label_texuture2DCoords[i] = v_scale_label_texuture2DCoords[i];
    }
    for (size_t i = 0; i < v_scale_label_textureIds.size(); i++) {
      scale_label_textureIds[i] = v_scale_label_textureIds[i];
    }

    // m_bounding_box_labels->clear();
    m_bounding_box_labels.swap();
    m_bounding_box_labels->setCoords(scale_label_coords);
    m_bounding_box_labels->setConnections(scale_label_connections);
    m_bounding_box_labels->setNormals(scale_label_normals);
    m_bounding_box_labels->setColors(scale_label_colors);
    m_bounding_box_labels->setTexture2DCoords(scale_label_texuture2DCoords);
    m_bounding_box_labels->setTextureIds(scale_label_textureIds);
    // m_bounding_box_labels_renderer = new kvs::jaea::StochasticTexturedPolygonRenderer();
    //  to decrease noise
    m_bounding_box_labels_renderer->setAnisotropicFiltering();
    // my_scene->replaceRenderer( m_bounding_box_labels_renderer_id, m_bounding_box_labels_renderer);
    m_scene->replaceObject(m_bounding_box_labels_id, m_bounding_box_labels, false);
    m_scene->replaceObject(OBJ_NAME_BOUNDING_BOX_SCALE_LABELS, m_bounding_box_labels, false);
  }
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE

  STOP_AND_RECORD_TIMER(A_4_1_4_2);
}

void BoundingBox::checkScalingRatio(kvs::Real32 scaling_ratio, bool force_update) {
  if (!force_update && scaling_ratio >= m_bounding_box_scaling_ratio_min && scaling_ratio <= m_bounding_box_scaling_ratio_max) {
    rebuildBoundingBox(false);
    return;
  }

  MAKE_AND_START_TIMER(A_4_1_4_1);

  float ratio_log10 = log10(scaling_ratio);
  int ratio_log10_int = (int)ratio_log10;
  float threshold = pow(10, ratio_log10_int);
  int index = (int)(scaling_ratio / threshold);
  float threshold_min = threshold * index;
  float threshold_max = threshold * (index + 1);

#ifdef DEBUG_TOUCH_CONTROLLER
  if (std::isinf(ratio_log10)) {
    std::cout << "ERROR : TouchController::checkScalingRatio(): ratio_log10 is inf." << std::endl;
  }
  if (std::isnan(ratio_log10)) {
    std::cout << "ERROR : TouchController::checkScalingRatio(): ratio_log10 is NaN." << std::endl;
  }
  if (std::isinf(threshold)) {
    std::cout << "ERROR : TouchController::checkScalingRatio(): threshold is inf (ratio_log10_int=" << ratio_log10_int << ")." << std::endl;
  }
  if (std::isnan(threshold)) {
    std::cout << "ERROR : TouchController::checkScalingRatio(): threshold is NaN (ratio_log10_int=" << ratio_log10_int << ")." << std::endl;
  }
  if (-1e-100 < threshold && threshold < 1e-100) {
    std::cout << "ERROR : TouchController::checkScalingRatio(): index is nearly inf (threshold=" << threshold << ")." << std::endl;
  }
#endif // DEBUG_TOUCH_CONTROLLER

  m_bounding_box_scaling_ratio_min = threshold_min;
  m_bounding_box_scaling_ratio_max = threshold_max;
  m_bounding_box_scaling_ratio = scaling_ratio;

  STOP_AND_RECORD_TIMER(A_4_1_4_1);

  rebuildBoundingBox(true);
}

void BoundingBox::initializeEvent() {
  std::pair<int, int> id_pair;

  // bounding box
  m_bounding_box.swap();
  m_bounding_box->setName(OBJ_NAME_BOUNDING_BOX);
  m_bounding_box->setLineType(kvs::LineObject::Segment);
  m_bounding_box->setColorType(kvs::LineObject::LineColor);
  m_bounding_box->setColor(kvs::RGBColor::Blue());
  m_bounding_box.swap();
  m_bounding_box->setName(OBJ_NAME_BOUNDING_BOX);
  m_bounding_box->setLineType(kvs::LineObject::Segment);
  m_bounding_box->setColorType(kvs::LineObject::LineColor);
  m_bounding_box->setColor(kvs::RGBColor::Blue());
  m_bounding_box.hideAll();
  m_bounding_box_renderer = new kvs::StochasticLineRenderer();
#ifndef DISABLE_LINES
  id_pair = m_scene->registerObject(m_bounding_box, m_bounding_box_renderer);
  m_bounding_box_id = id_pair.first;
#endif // DISABLE_LINES

#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
  // submesh
  m_bounding_box_submesh.swap();
  m_bounding_box_submesh->setName(OBJ_NAME_BOUNDING_BOX_SUBMESH);
  m_bounding_box_submesh->setLineType(kvs::LineObject::Segment);
  m_bounding_box_submesh->setColorType(kvs::LineObject::LineColor);
  m_bounding_box_submesh->setColor(kvs::RGBColor::Cyan());
  m_bounding_box_submesh.swap();
  m_bounding_box_submesh->setName(OBJ_NAME_BOUNDING_BOX_SUBMESH);
  m_bounding_box_submesh->setLineType(kvs::LineObject::Segment);
  m_bounding_box_submesh->setColorType(kvs::LineObject::LineColor);
  m_bounding_box_submesh->setColor(kvs::RGBColor::Cyan());
  m_bounding_box_submesh.hideAll();
  m_bounding_box_submesh_renderer = new kvs::StochasticLineRenderer();
#ifndef DISABLE_LINES
  id_pair = m_scene->registerObject(m_bounding_box_submesh, m_bounding_box_submesh_renderer);
  m_bounding_box_submesh_id = id_pair.first;
#endif // DISABLE_LINES

  // set scale label
  m_bounding_box_labels.swap();
  m_bounding_box_labels->setName(OBJ_NAME_BOUNDING_BOX_SCALE_LABELS);
  m_bounding_box_labels->setPolygonType(kvs::jaea::TexturedPolygonObject::Triangle);
  m_bounding_box_labels->setColorType(kvs::jaea::TexturedPolygonObject::VertexColor);
  m_bounding_box_labels->setNormalType(kvs::jaea::TexturedPolygonObject::VertexNormal);
  m_bounding_box_labels.setColor(kvs::RGBColor::Blue());
  m_bounding_box_labels.swap();
  m_bounding_box_labels->setName(OBJ_NAME_BOUNDING_BOX_SCALE_LABELS);
  m_bounding_box_labels->setPolygonType(kvs::jaea::TexturedPolygonObject::Triangle);
  m_bounding_box_labels->setColorType(kvs::jaea::TexturedPolygonObject::VertexColor);
  m_bounding_box_labels->setNormalType(kvs::jaea::TexturedPolygonObject::VertexNormal);
  m_bounding_box_labels.setColor(kvs::RGBColor::Blue());
  m_bounding_box_labels.hideAll();
  m_bounding_box_labels_renderer = new StochasticTexturedPolygonRenderer();
  id_pair = m_scene->registerObject(m_bounding_box_labels, m_bounding_box_labels_renderer);
  m_bounding_box_labels_id = id_pair.first;
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE

  initializeScaleLabel();
  rebuildBoundingBox(true);
}

void BoundingBox::initializeScaleLabel() {
#ifdef DEBUG_TOUCH_CONTROLLER
  std::cout << "initializeScaleLabel called" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

  if (!m_numberTexturesAreInitialized) {
    initializeNumberTextureMaps();
    m_numberTexturesAreInitialized = true;

    // load bmp
    for (auto index_filename_pair : m_index_filename_map) {
#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
      int texture_id = index_filename_pair.first;
      std::string resource_name = index_filename_pair.second;

      QImage *image_file = new QImage(resource_name.c_str());

      size_t image_width = image_file->width();
      size_t image_height = image_file->height();

#ifdef DEBUG_TOUCH_CONTROLLER
      std::cout << "image width" << image_width << std::endl;
      std::cout << "image height" << image_height << std::endl;
      std::cout << "image width, height obtained" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

      kvs::ValueArray<kvs::UInt8> image_pixel_with_alpha(image_width * image_height * 4);
      // rgb -> rgba

      size_t index = 0;
      for (size_t iY = 0; iY < image_height; iY++) {
        for (size_t iX = 0; iX < image_width; iX++) {
          const QRgb rgb = image_file->pixel(iX, iY);
          kvs::UInt8 r = qRed(rgb);
          kvs::UInt8 g = qGreen(rgb);
          kvs::UInt8 b = qBlue(rgb);
          kvs::UInt8 a = (r == 255 && g == 255 && b == 255) ? 0 : 255;

          image_pixel_with_alpha[index * 4 + 0] = r;
          image_pixel_with_alpha[index * 4 + 1] = g;
          image_pixel_with_alpha[index * 4 + 2] = b;
          image_pixel_with_alpha[index * 4 + 3] = a;

          index++;
        }
      }
      m_bounding_box_labels.swap();
      m_bounding_box_labels->addColorArray(texture_id, image_pixel_with_alpha, image_width, image_height);
      m_bounding_box_labels.swap();
      m_bounding_box_labels->addColorArray(texture_id, image_pixel_with_alpha, image_width, image_height);
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
    }
  }
}

void BoundingBox::frameEvent() {
  // update bounding box

  if (m_isShown) {
    MAKE_AND_START_TIMER(A_4_1_3);
    kvs::Real32 currentScalingRatio = m_scene->getScalingRatioAccum();
    bool force_update = !m_bounding_box->isShown(); // force update == check box is checked but bounding box is hidden.

    STOP_AND_RECORD_TIMER(A_4_1_3);

    MAKE_AND_START_TIMER(A_4_1_4);
    checkScalingRatio(currentScalingRatio, force_update);
    STOP_AND_RECORD_TIMER(A_4_1_4);

    MAKE_AND_START_TIMER(A_4_1_5);
    //        std::cout << "m_bounding_box.numberOfSizes()=" << m_bounding_box.numberOfSizes() << std::endl;
    m_bounding_box.showAll();
#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
    m_bounding_box_submesh.showAll();
    m_bounding_box_labels.showAll();
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
    STOP_AND_RECORD_TIMER(A_4_1_5);
  } else {
    MAKE_AND_START_TIMER(A_4_1_6);
    m_bounding_box.hideAll();
#ifdef ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
    m_bounding_box_submesh.hideAll();
    m_bounding_box_labels.hideAll();
#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX_SCALE
    STOP_AND_RECORD_TIMER(A_4_1_6);
  }
}

#endif // ENABLE_FEATURE_DRAW_BOUNDING_BOX
