#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "TouchController.h"
#include "Scene.h"
#include "objnameutil.h"
#include "ovr.h"
#include "screen_vr.h"
#include "TimeMeasureUtility.h"

#include "ObjectManager.h"
#include <kvs/Bmp>
#include <kvs/IDManager>
#include <kvs/Matrix44>
#include <kvs/ObjectBase>
#include <kvs/Vector4>

#include <cmath>

extern kvs::Int64 frame_index;

#define VR_HANDS_RENDERER_REPETITION_LEVEL 1

#define SHOW_MANIPULATION_OBJECTS

// #define TRACE_COORDINATES
#ifdef TRACE_COORDINATES
#include <QDateTime>
#endif // def TRACE_COORDINATES

inline kvs::Mat4 ToMat4(const OVR::Matrix4f &m) {
  return kvs::Mat4(m.M[0][0], m.M[0][1], m.M[0][2], m.M[0][3], m.M[1][0], m.M[1][1], m.M[1][2], m.M[1][3], m.M[2][0], m.M[2][1], m.M[2][2], m.M[2][3],
                   m.M[3][0], m.M[3][1], m.M[3][2], m.M[3][3]);
}

inline kvs::Vec3 ToVec3(const OVR::Vector3f &v) { return kvs::Vec3(v.x, v.y, v.z); }

inline OVR::Vector3f ToVector3f(const kvs::Vec3 &v) { return OVR::Vector3f(v.x(), v.y(), v.z()); }

#ifdef ENABLE_FEATURE_DRAW_CONTROL_SPHERE
LineObjectProxy TouchController::m_control_sphere;
int TouchController::m_control_sphere_id;
#endif // ENABLE_FEATURE_DRAW_CONTROL_SPHERE

#ifdef ENABLE_FEATURE_DRAW_VR_HANDS
PolygonObjectProxy TouchController::m_vr_hands;
int TouchController::m_vr_hands_id;
#endif // ENABLE_FEATURE_DRAW_VR_HANDS

#ifdef TRACE_COORDINATES
std::ofstream trace_log;
long long int frame_count;
qint64 trace_msec;
#endif // TRACE_COORDINATES

TouchController::TouchController(HeadMountedDisplay &hmd, kvs::jaea::pbvr::Scene *scene)
    : ControllerBase(hmd, scene),
      // m_is_grabbed( false ),
      m_is_first_frame(true), m_both_is_grabbed(false), m_button_a_pressed(false), m_button_b_pressed(false), m_button_x_pressed(false),
    m_button_y_pressed(false),m_trigger_is_grabbed(false) , m_touch_distance(0.0f), m_rotation_factor(5000.0f), m_translation_factor(5000.0f), m_scaling_factor(30000.0f),
      m_rot_start_pos_right(0.0f, 0.0f, 0.0f), m_widget_handler(nullptr) {
  // initializeNumberTextureMaps();
#ifdef TRACE_COORDINATES
  trace_log.open("c:\\Users\\insight\\Documents\\kvs\\oculuspvbr_jaea\\trace_log");
  trace_log << "frame#,ihx,ihy,ihz,hx,hy,hz,lx,ly,lz,rx,ry,rz" << std::endl;
  frame_count = 0;
  trace_msec = QDateTime::currentMSecsSinceEpoch();
#endif // TRACE_COORDINATES
}

TouchController::~TouchController(){};

void TouchController::initializeEvent() {
#ifdef DEBUG_TOUCH_CONTROLLER
  std::cout << "TouchController::initializeEvent()" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

  kvs::jaea::pbvr::Scene *my_scene = static_cast<kvs::jaea::pbvr::Scene *>(scene());

  if (!inputDevice().hasRight()) {
    std::cout << "Oculus Touch (R): Disable" << std::endl;
  }
  if (!inputDevice().hasLeft()) {
    std::cout << "Oculus Touch (L): Disable" << std::endl;
  }

  std::pair<int, int> id_pair;

#ifdef ENABLE_FEATURE_DRAW_VR_HANDS
#ifndef DISABLE_VR_HAND
  // VR hand
  // m_vr_hands = new kvs::PolygonObject();
  m_vr_hands.swap();
  m_vr_hands->setName(OBJ_NAME_VR_HANDS);
  m_vr_hands->setPolygonTypeToTriangle();
  m_vr_hands->setColorTypeToPolygon();
  m_vr_hands->setNormalTypeToPolygon();
  m_vr_hands.swap();
  m_vr_hands->setName(OBJ_NAME_VR_HANDS);
  m_vr_hands->setPolygonTypeToTriangle();
  m_vr_hands->setColorTypeToPolygon();
  m_vr_hands->setNormalTypeToPolygon();
  m_vr_hands.showAll();
  m_vr_hands_renderer = new kvs::jaea::pbvr::StochasticPolygonRenderer();
  // m_vr_hands_renderer->setRepetitionLevel(VR_HANDS_RENDERER_REPETITION_LEVEL);
#ifdef SHOW_MANIPULATION_OBJECTS
  id_pair = my_scene->registerObject(m_vr_hands, m_vr_hands_renderer);
  m_vr_hands_id = id_pair.first;
#endif // SHOW_MANIPULATION_OBJECTS
#endif // DISABLE_VR_HAND
#endif // ENABLE_FEATURE_DRAW_VR_HANDS

#ifdef ENABLE_FEATURE_DRAW_CONTROL_SPHERE
  // control sphere
  m_control_sphere.setName(OBJ_NAME_CONTROL_SPHERE);
  m_control_sphere.setLineType(kvs::LineObject::Segment);
  m_control_sphere.setColorType(kvs::LineObject::VertexColor);
  m_control_sphere.setColor(kvs::RGBColor::Blue());

  m_control_sphere.swap();
  m_control_sphere->setName(OBJ_NAME_CONTROL_SPHERE);
  m_control_sphere->setLineType(kvs::LineObject::Segment);
  m_control_sphere->setColorType(kvs::LineObject::VertexColor);
  m_control_sphere->setColor(kvs::RGBColor::Blue());
  m_control_sphere.swap();
  m_control_sphere->setName(OBJ_NAME_CONTROL_SPHERE);
  m_control_sphere->setLineType(kvs::LineObject::Segment);
  m_control_sphere->setColorType(kvs::LineObject::VertexColor);
  m_control_sphere->setColor(kvs::RGBColor::Green());
  m_control_sphere.hideAll();
  m_control_sphere_renderer = new kvs::jaea::pbvr::StochasticLineRenderer();
#ifdef SHOW_MANIPULATION_OBJECTS
#ifndef DISABLE_LINES
  id_pair = my_scene->registerObject(m_control_sphere, m_control_sphere_renderer);
  m_control_sphere_id = id_pair.first;
#endif // DISABLE_LINES
#endif // ENABLE_FEATURE_DRAW_CONTROL_SPHERE

#endif // SHOW_MANIPULATION_OBJECTS
}

// 2020118
void TouchController::rebuildControlSphere(bool force_rebuild) {
  MAKE_AND_START_TIMER(A_4_1_2_1);

  // static kvs::Vec3 initial_head_p;
  kvs::jaea::pbvr::Scene *my_scene = static_cast<kvs::jaea::pbvr::Scene *>(scene());

  // #ifdef DEBUG_TOUCH_CONTROLLER
  //     std::cout << "rebuild control sphere called" << std::endl;
  // #endif // DEBUG_TOUCH_CONTROLLER

  // get hands
  const ovrTrackingState ts = inputDevice().trackingState(0);
  ovrPosef hands[2] = {ts.HandPoses[ovrHand_Left].ThePose, ts.HandPoses[ovrHand_Right].ThePose};
  // #ifdef DEBUG_TOUCH_CONTROLLER
  //     ovrPosef head =  ts.HeadPose.ThePose;
  //     std::cout << "position : head :       (" << head.Position.x << ", " << head.Position.y << ", " << head.Position.z << ")" << std::endl;
  //     std::cout << "position : left hand :  (" << hands[ovrHand_Left].Position.x << ", " << hands[ovrHand_Left].Position.y << ", " <<
  //     hands[ovrHand_Left].Position.z << ")" << std::endl; std::cout << "position : right hand : (" << hands[ovrHand_Right].Position.x << ", " <<
  //     hands[ovrHand_Right].Position.y << ", " << hands[ovrHand_Right].Position.z << ")" << std::endl;
  // #endif // DEBUG_TOUCH_CONTROLLER

  const kvs::Vec3 left_p = ToVec3(hands[ovrHand_Left].Position);
  const kvs::Vec3 right_p = ToVec3(hands[ovrHand_Right].Position);
  const kvs::Mat4 left_rot = ToMat4(OVR::Matrix4f(hands[ovrHand_Left].Orientation));
  const kvs::Mat4 right_rot = ToMat4(OVR::Matrix4f(hands[ovrHand_Right].Orientation));

  // set camera info
  kvs::Vec3 camera_position = my_scene->camera()->position();

  // #ifdef DEBUG_TOUCH_CONTROLLER
  //     std::cout << "camera position is " << camera_position << std::endl;
  //     // show hands coordinates
  //     std::cout << "right_p " << right_p << std::endl;
  //     std::cout << "left_p  " << left_p << std::endl;
  // #endif // DEBUG_TOUCH_CONTROLLER

  const double touch_distance = (right_p - left_p).length();
  const double radius = touch_distance / 2;

  float base_position_x = camera_position.x() - m_initial_head_p.x();
  float base_position_y = camera_position.y() - m_initial_head_p.y();
  float base_position_z = camera_position.z() - m_initial_head_p.z();

  float mid_x = (right_p.x() + left_p.x()) / 2 + base_position_x;
  float mid_y = (right_p.y() + left_p.y()) / 2 + base_position_y;
  float mid_z = (right_p.z() + left_p.z()) / 2 + base_position_z;

  float line_thickness = 3.0;

  STOP_AND_RECORD_TIMER(A_4_1_2_1);
  if (force_rebuild || (m_both_is_grabbed && PBVRWidgetHandlerInstance()->isEnabledControlSphere())) {
    MAKE_AND_START_TIMER(A_4_1_2_2);
#ifdef ENABLE_FEATURE_DRAW_CONTROL_SPHERE
    const static size_t num_division = 20;
    const static float angle_increment = kvs::Math::pi * 2 / num_division;

    const static size_t num_nodes = num_division * 3;
    const static size_t num_lines = num_division * 3;

    kvs::ValueArray<kvs::Real32> control_sphere_coords_array(num_nodes * 3);
    kvs::ValueArray<kvs::UInt32> control_sphere_connections_array(num_lines * 2);
    kvs::ValueArray<kvs::UInt8> control_sphere_colors_array(num_nodes * 3);

    for (size_t iNode = 0; iNode < num_nodes; iNode++) {
      float x = 0, y = 0, z = 0;
      if (iNode < num_division) {
        float angle = iNode * angle_increment;
        // TODO : replace to reference to pre-calculated cos/sin tables.
        x = cos(angle);
        y = sin(angle);
        z = 0;
      } else if (iNode < num_division * 2) {
        float angle = (iNode - num_division) * angle_increment;
        y = cos(angle);
        z = sin(angle);
        x = 0;
      } else {
        float angle = (iNode - num_division * 2) * angle_increment;
        z = cos(angle);
        x = sin(angle);
        y = 0;
      }
      x = x * radius + mid_x;
      y = y * radius + mid_y;
      z = z * radius + mid_z;

      control_sphere_coords_array[iNode * 3 + 0] = x;
      control_sphere_coords_array[iNode * 3 + 1] = y;
      control_sphere_coords_array[iNode * 3 + 2] = z;

      if (iNode < num_division) {
        control_sphere_colors_array[iNode * 3 + 0] = 0;
        control_sphere_colors_array[iNode * 3 + 1] = 0;
        control_sphere_colors_array[iNode * 3 + 2] = 255;
      } else if (iNode < num_division * 2) {
        control_sphere_colors_array[iNode * 3 + 0] = 0;
        control_sphere_colors_array[iNode * 3 + 1] = 255;
        control_sphere_colors_array[iNode * 3 + 2] = 0;
      } else {
        control_sphere_colors_array[iNode * 3 + 0] = 255;
        control_sphere_colors_array[iNode * 3 + 1] = 0;
        control_sphere_colors_array[iNode * 3 + 2] = 0;
      }
    }

    for (size_t iLine = 0; iLine < num_lines; iLine++) {
      kvs::UInt32 node0 = iLine;
      kvs::UInt32 node1 = node0 + 1;
      if (iLine == num_division - 1) {
        node1 = 0;
      } else if (iLine == num_division * 2 - 1) {
        node1 = num_division;
      } else if (iLine == num_division * 3 - 1) {
        node1 = num_division * 2;
      }

      control_sphere_connections_array[iLine * 2 + 0] = node0;
      control_sphere_connections_array[iLine * 2 + 1] = node1;
    }

    // call my_scene->replaceObject() : 2021/06/17 nakamura
    // m_control_sphere = new kvs::LineObject();
    m_control_sphere.swap();
    m_control_sphere->clear();
    // m_control_sphere->setName(OBJ_NAME_CONTROL_SPHERE);
    m_control_sphere->setCoords(control_sphere_coords_array);
    m_control_sphere->setLineType(kvs::LineObject::Segment);
    m_control_sphere->setConnections(control_sphere_connections_array);
    m_control_sphere->setColorType(kvs::LineObject::VertexColor);
    m_control_sphere->setColors(control_sphere_colors_array);
    m_control_sphere->setSize(line_thickness);
    my_scene->replaceObject(m_control_sphere_id, m_control_sphere, false);
//        my_scene->replaceObject(OBJ_NAME_CONTROL_SPHERE, m_control_sphere, false);
#endif // ENABLE_FEATURE_DRAW_CONTROL_SPHERE

#ifdef ENABLE_FEATURE_DRAW_VR_HANDS
#ifndef DISABLE_VR_HAND
#ifndef USE_WIREFRAME_VR_HANDS
    m_vr_hands.hideAll();
#endif // USE_WIREFRAME_VR_HANDS
#endif // DISABLE_VR_HAND
#endif // ENABLE_FEATURE_DRAW_VR_HANDS

#ifdef ENABLE_FEATURE_DRAW_CONTROL_SPHERE
    m_control_sphere.showAll();
#endif // ENABLE_FEATURE_DRAW_CONTROL_SPHERE
    STOP_AND_RECORD_TIMER(A_4_1_2_2);
  } else {
    // VR hands
    MAKE_AND_START_TIMER(A_4_1_2_3);
    // #ifdef DEBUG_TOUCH_CONTROLLER
    //         std::cout << "hand position viewer called " << std::endl;
    // #endif // DEBUG_TOUCH_CONTROLLER

#ifdef ENABLE_FEATURE_DRAW_VR_HANDS
    const float hand_size = 0.05f;

    // position of hands
    float rx = right_p.x() + base_position_x;
    float ry = right_p.y() + base_position_y;
    float rz = right_p.z() + base_position_z;
    float lx = left_p.x() + base_position_x;
    float ly = left_p.y() + base_position_y;
    float lz = left_p.z() + base_position_z;

#ifdef ENABLE_FEATURE_DRAW_CONTROL_SPHERE
#ifdef USE_WIREFRAME_VR_HANDS
    const size_t num_nodes = 16;
    const size_t num_lines = 24;

    float rx_min = rx - hand_size / 2;
    float ry_min = ry - hand_size / 2;
    float rz_min = rz - hand_size / 2;
    float rx_max = rx + hand_size / 2;
    float ry_max = ry + hand_size / 2;
    float rz_max = rz + hand_size / 2;
    float lx_min = lx - hand_size / 2;
    float ly_min = ly - hand_size / 2;
    float lz_min = lz - hand_size / 2;
    float lx_max = lx + hand_size / 2;
    float ly_max = ly + hand_size / 2;
    float lz_max = lz + hand_size / 2;

    kvs::Real32 control_sphere_coords[num_nodes * 3]{
        rx_min, ry_min, rz_min, rx_max, ry_min, rz_min, rx_max, ry_max, rz_min, rx_min, ry_max, rz_min,
        rx_min, ry_min, rz_max, rx_max, ry_min, rz_max, rx_max, ry_max, rz_max, rx_min, ry_max, rz_max,

        lx_min, ly_min, lz_min, lx_max, ly_min, lz_min, lx_max, ly_max, lz_min, lx_min, ly_max, lz_min,
        lx_min, ly_min, lz_max, lx_max, ly_min, lz_max, lx_max, ly_max, lz_max, lx_min, ly_max, lz_max,
    };
    // clang-format off
    kvs::UInt32 control_sphere_connections[num_lines * 2]{
        //right
         0,  1,    1,  2,    2,  3,    3,  0,
         4,  5,    5,  6,    6,  7,    7,  4,
         0,  4,    1,  5,    2,  6,    3,  7,
        //left
         8,  9,    9, 10,   10, 11,   11,  8,
        12, 13,   13, 14,   14, 15,   15, 12,
         8, 12,    9, 13,   10, 14,   11, 15,
    };
    kvs::UInt8 control_sphere_colors[num_nodes * 3]{
        //right
        0, 0, 255,   0, 0, 255,   0, 0, 255,   0, 0, 255,
        0, 0, 255,   0, 0, 255,   0, 0, 255,   0, 0, 255,
        //left
        255, 0, 0,   255, 0, 0,   255, 0, 0,   255, 0, 0,
        255, 0, 0,   255, 0, 0,   255, 0, 0,   255, 0, 0,
    };
    // clang-format on

    // create coords connection colors
    kvs::ValueArray<kvs::Real32> control_sphere_coords_array(control_sphere_coords, num_nodes * 3);
    kvs::ValueArray<kvs::UInt32> control_sphere_connections_array(control_sphere_connections, num_lines * 2);
    kvs::ValueArray<kvs::UInt8> control_sphere_colors_array(control_sphere_colors, num_nodes * 3);

    m_control_sphere->clear();
    m_control_sphere->setLineType(kvs::LineObject::Segment);
    m_control_sphere->setColorType(kvs::LineObject::VertexColor);
    m_control_sphere->setCoords(control_sphere_coords_array);
    m_control_sphere->setConnections(control_sphere_connections_array);
    m_control_sphere->setColors(control_sphere_colors_array);
    m_control_sphere->setSize(line_thickness);
#else  // USE_WIREFRAME_VR_HANDS
#endif // ENABLE_FEATURE_DRAW_CONTROL_SPHERE

    const int ncoords = 8;
    const int ntriangles = 8;

    const kvs::Vec4 nvx(hand_size, 0, 0, 0);
    const kvs::Vec4 nvy(0, hand_size, 0, 0);
    const kvs::Vec4 nvz(0, 0, -hand_size * 2.0f, 0);

    kvs::Vec4 lvx = left_rot * nvx;
    kvs::Vec4 lvy = left_rot * nvy;
    kvs::Vec4 lvz = left_rot * nvz;

    kvs::Vec4 rvx = right_rot * nvx;
    kvs::Vec4 rvy = right_rot * nvy;
    kvs::Vec4 rvz = right_rot * nvz;

    // coords
    kvs::Real32 coords[ncoords * 3]{
        lx,           ly,           lz,           // 0 : left[0]
        lx + lvx.x(), ly + lvx.y(), lz + lvx.z(), // 1 : left[1]
        lx + lvy.x(), ly + lvy.y(), lz + lvy.z(), // 2 : left[2]
        lx + lvz.x(), ly + lvz.y(), lz + lvz.z(), // 3 : left[3]
        rx,           ry,           rz,           // 4 : right[0]
        rx + rvx.x(), ry + rvx.y(), rz + rvx.z(), // 5 : right[1]
        rx + rvy.x(), ry + rvy.y(), rz + rvy.z(), // 6 : right[2]
        rx + rvz.x(), ry + rvz.y(), rz + rvz.z(), // 7 : right[3]
    };

    // connections
    kvs::UInt32 connections[ntriangles * 3]{
        0, 1, 2, 0, 3, 1, 0, 2, 3, 1, 3, 2, 4, 5, 6, 4, 7, 5, 4, 6, 7, 5, 7, 6,
    };
    // polygon colors
    kvs::UInt8 colors[ntriangles * 3]{
        255, 0, 0,   // left[0]
        255, 0, 0,   // left[1]
        255, 0, 0,   // left[2]
        255, 0, 0,   // left[3]
        0,   0, 255, // right[0]
        0,   0, 255, // right[1]
        0,   0, 255, // right[2]
        0,   0, 255, // right[3]
    };
    // normals
    kvs::Real32 normals[ntriangles * 3];
    for (int itri = 0; itri < ntriangles; itri++) {
      kvs::UInt32 n[3]{connections[itri * 3 + 0], connections[itri * 3 + 1], connections[itri * 3 + 2]};
      kvs::Vector3f v0(coords[n[0] * 3 + 0], coords[n[0] * 3 + 1], coords[n[0] * 3 + 2]);
      kvs::Vector3f v1(coords[n[1] * 3 + 0], coords[n[1] * 3 + 1], coords[n[1] * 3 + 2]);
      kvs::Vector3f v2(coords[n[2] * 3 + 0], coords[n[2] * 3 + 1], coords[n[2] * 3 + 2]);
      kvs::Vector3f v01 = v1 - v0;
      kvs::Vector3f v02 = v2 - v0;
      kvs::Vector3f normal = v01.cross(v02).normalized();
      normals[itri * 3 + 0] = -normal.x();
      normals[itri * 3 + 1] = -normal.y();
      normals[itri * 3 + 2] = -normal.z();
    }
    kvs::ValueArray<kvs::Real32> vr_hands_coords(coords, ncoords * 3);
    kvs::ValueArray<kvs::UInt32> vr_hands_connections(connections, ntriangles * 3);
    kvs::ValueArray<kvs::UInt8> vr_hands_colors(colors, ntriangles * 3);
    kvs::ValueArray<kvs::Real32> vr_hands_normals(normals, ntriangles * 3);

#ifndef DISABLE_VR_HAND
    m_vr_hands.swap();
    m_vr_hands->clear();
    m_vr_hands->setCoords(vr_hands_coords);
    m_vr_hands->setConnections(vr_hands_connections);
    m_vr_hands->setColors(vr_hands_colors);
    m_vr_hands->setOpacity(100);
    m_vr_hands->setNormals(vr_hands_normals);
    my_scene->replaceObject(m_vr_hands_id, m_vr_hands, false);
    //        my_scene->replaceObject(OBJ_NAME_VR_HANDS, m_vr_hands, false);

    m_vr_hands.showAll();
#endif // DISABLE_VR_HAND
#endif // ENABLE_FEATURE_DRAW_VR_HANDS

#ifdef ENABLE_FEATURE_DRAW_CONTROL_SPHERE
    m_control_sphere.hideAll();
#endif // ENABLE_FEATURE_DRAW_CONTROL_SPHERE

#endif
    STOP_AND_RECORD_TIMER(A_4_1_2_3);
  }

  // #ifndef DISABLE_VR_HAND
  //     std::cerr << "TouchController::rebuildControlSphere(" << force_rebuild << ") : VRHand=" << (m_vr_hands->isShown() ? "show" : "hide") << "("
  //     << m_vr_hands.index() << "), CSphere=" << (m_control_sphere->isShown() ? "show" : "hide") << "(" << m_control_sphere.index() << ")" <<
  //     std::endl;
  // #else
  //     std::cerr << "TouchController::rebuildControlSphere(" << force_rebuild << ") : CSphere=" << (m_control_sphere->isShown() ? "show" : "hide")
  //     << "(" << m_control_sphere.index() << ")" << std::endl;
  // #endif // DISABLE_VR_HAND
}

void TouchController::frameEvent() {
  MAKE_AND_START_TIMER(A_4_1);
  MAKE_AND_START_TIMER(A_4_1_1);

  static const float GRAB_THRESHOLD = 0.4f;

  kvs::jaea::pbvr::Scene *my_scene = static_cast<kvs::jaea::pbvr::Scene *>(scene());

  // #ifdef DEBUG_TOUCH_CONTROLLER
  //     std::cout << "frameEvent() is being called" << std::endl;
  // #endif // DEBUG_TOUCH_CONTROLLER

  const ovrTrackingState ts = inputDevice().trackingState(0);
  ovrPosef head = ts.HeadPose.ThePose;
  ovrPosef hands[2] = {ts.HandPoses[ovrHand_Left].ThePose, ts.HandPoses[ovrHand_Right].ThePose};
  const kvs::Vec3 left_p = ToVec3(hands[ovrHand_Left].Position);
  const kvs::Vec3 right_p = ToVec3(hands[ovrHand_Right].Position);

  ovrInputState input_state = inputDevice().inputState(ovrControllerType_Touch);
#ifdef GRAB_USING_INDEX_TRIGGER
  const bool left_grabbed = input_state.IndexTrigger[ovrHand_Left] > GRAB_THRESHOLD;
  const bool right_grabbed = input_state.IndexTrigger[ovrHand_Right] > GRAB_THRESHOLD;
#else
  const bool left_grabbed = input_state.HandTrigger[ovrHand_Left] > GRAB_THRESHOLD;
  const bool right_grabbed = input_state.HandTrigger[ovrHand_Right] > GRAB_THRESHOLD;
  const bool right_index_trigger_grabbed = input_state.IndexTrigger[ovrHand_Right] > GRAB_THRESHOLD;
#endif

  if (m_is_first_frame) {
    m_initial_head_p = ToVec3(head.Position);
    m_is_first_frame = false;
  }
#ifdef TRACE_COORDINATES
  trace_log << frame_count << ", " << m_initial_head_p.x() << ", " << m_initial_head_p.y() << ", " << m_initial_head_p.z() << ", " << head.Position.x
            << ", " << head.Position.y << ", " << head.Position.z << ", " << left_p.x() << ", " << left_p.y() << ", " << left_p.z() << ", "
            << right_p.x() << ", " << right_p.y() << ", " << right_p.z() << std::endl;
  frame_count++;
#endif // TRACE_COORDINATES

  // #ifdef DEBUG_TOUCH_CONTROLLER
  //     // if ( !inputDevice().hasRight() ) { std::cout << "Oculus Touch (R): Disable" << std::endl; }
  //     // if ( inputDevice().hasRight() ) { std::cout << "Oculus Touch (R): Available" << std::endl; }
  //     // if ( !inputDevice().hasLeft() ) { std::cout << "Oculus Touch (L): Disable" << std::endl; }
  //     //std::cout << "  Position (R): " << kvs::jaea::pbvr::ToVec3( hands[ovrHand_Left].Position ) << std::endl;
  //     //std::cout << "  Position (L): " << kvs::jaea::pbvr::ToVec3( hands[ovrHand_Right].Position ) << std::endl;
  //     if ( input_state.Buttons & ovrButton_A ) { std::cout << "Pressed A" << std::endl; }
  //     if ( input_state.Buttons & ovrButton_B ) { std::cout << "Pressed B" << std::endl; }
  //     if ( input_state.Buttons & ovrButton_X ) { std::cout << "Pressed X" << std::endl; }
  //     if ( input_state.Buttons & ovrButton_Y ) { std::cout << "Pressed Y" << std::endl; }
  //     if ( input_state.HandTrigger[ovrHand_Left] > GRAB_THRESHOLD ) { std::cout << "Grip Left HandTrigger" << std::endl; }
  //     if ( input_state.HandTrigger[ovrHand_Right] > GRAB_THRESHOLD ) { std::cout << "Grip Right HandTrigger" << std::endl; }
  //     if ( input_state.IndexTrigger[ovrHand_Left] > GRAB_THRESHOLD ) { std::cout << "Grip Left IndexTrigger" << std::endl; }
  //     if ( input_state.IndexTrigger[ovrHand_Right] > GRAB_THRESHOLD ) { std::cout << "Grip Right IndexTrigger" << std::endl; }
  // #endif

  //            kvs::Vec3 eyePos = ::ToVec3(ts.HeadPose.ThePose.Position);
  //            OVR::Matrix4f R = OVR::Matrix4f(ts.HeadPose.ThePose.Orientation);
  //            OVR::Vector3f forward = R.Transform(OVR::Vector3f(0.0,0.0,-6.0));
  kvs::Vec3 hmdPos = ::ToVec3(head.Position);
  OVR::Matrix4f R = OVR::Matrix4f(head.Orientation);
  OVR::Vector3f forward = R.Transform(OVR::Vector3f(ToVector3f(Screen::KVS_CAMERA_INITIAL_POSITION)));
  kvs::Vec3 hmdDir = ::ToVec3(forward);

#ifdef CHECK_ZOOM_RATIO
  static float zoom_ratio_before = 1.0f;
#endif // CHECK_ZOOM_RATIO

  if (right_grabbed && left_grabbed) {
    if (m_both_is_grabbed) {
      my_scene->threedcontrollerPressFunction(hmdPos, m_initial_head_p, hmdDir, left_p, right_p, m_prev_left_p, m_prev_right_p);
      m_prev_right_p = right_p;
      m_prev_left_p = left_p;
    } else {
      // start grabbing
      m_both_is_grabbed = true;
      m_start_right_p = right_p;
      m_start_left_p = left_p;
      m_prev_right_p = right_p;
      m_prev_left_p = left_p;

#ifdef CHECK_ZOOM_RATIO
      zoom_ratio_before = this->scene()->objectManager()->xform().scaling()[0];
#endif // CHECK_ZOOM_RATIO

    } // if ( m_both_is_grabbed )
  }   // if (right_grabbed && left_grabbed)
  else {
    // Released.

#ifdef CHECK_ZOOM_RATIO
    if (m_both_is_grabbed) {
      float zoom_ratio_after = my_scene->objectManager()->xform().scaling()[0];
      std::cerr << "zoom ratio : " << zoom_ratio_before << " -> " << zoom_ratio_after << " (" << zoom_ratio_after / zoom_ratio_after * 100.0f << " %)"
                << std::endl;
    }
#endif // CHECK_ZOOM_RATIO

    m_both_is_grabbed = false;
    //        my_scene->threedcontrollerReleaseFunction(left_p, right_p, m_prev_left_p, m_prev_right_p, m_start_left_p, m_start_right_p);
    my_scene->threedcontrollerReleaseFunction(hmdPos, m_initial_head_p, hmdDir, left_p, right_p, m_prev_left_p, m_prev_right_p);
  } // if (right_grabbed && left_grabbed)

#ifdef DEBUG_SCENE
  bool prev_button_a_pressed = m_button_a_pressed;
  bool prev_button_b_pressed = m_button_b_pressed;
  bool prev_button_x_pressed = m_button_x_pressed;
  bool prev_button_y_pressed = m_button_y_pressed;
  m_button_a_pressed = (input_state.Buttons & ovrButton_A) != 0;
  m_button_b_pressed = (input_state.Buttons & ovrButton_B) != 0;
  m_button_x_pressed = (input_state.Buttons & ovrButton_X) != 0;
  m_button_y_pressed = (input_state.Buttons & ovrButton_Y) != 0;

  if ((!m_button_a_pressed && prev_button_a_pressed) || (!m_button_b_pressed && prev_button_b_pressed) ||
      (!m_button_x_pressed && prev_button_x_pressed) || (!m_button_y_pressed && prev_button_y_pressed)) {
    std::stringstream msg;
    msg << "Released Button :";
    if (!m_button_a_pressed && prev_button_a_pressed) {
      msg << " A";
    }
    if (!m_button_b_pressed && prev_button_b_pressed) {
      msg << " B";
    }
    if (!m_button_x_pressed && prev_button_x_pressed) {
      msg << " X";
    }
    if (!m_button_y_pressed && prev_button_y_pressed) {
      msg << " Y";
    }
    msg << " : " << prev_button_a_pressed << m_button_a_pressed << " " << prev_button_b_pressed << m_button_b_pressed << " " << prev_button_x_pressed
        << m_button_x_pressed << " " << prev_button_y_pressed << m_button_y_pressed;
    std::cout << msg.str() << std::endl;
  }
#endif // DEBUG_SCENE

  if ( right_index_trigger_grabbed ) {
    if (m_trigger_is_grabbed) {
      my_scene->glyphControllerPressFunction(hmdPos, m_initial_head_p, hmdDir, left_p, right_p, m_prev_left_p, m_prev_right_p);
      m_prev_right_p = right_p;
      m_prev_left_p = left_p;
    } else {
      // start grabbing
      m_trigger_is_grabbed = true;
      m_start_right_p = right_p;
      m_start_left_p = left_p;
      m_prev_right_p = right_p;
      m_prev_left_p = left_p;

#ifdef CHECK_ZOOM_RATIO
      zoom_ratio_before = this->scene()->objectManager()->xform().scaling()[0];
#endif // CHECK_ZOOM_RATIO

    } // if ( m_both_is_grabbed )
    }   // if (right_grabbed && left_grabbed)
  else {
    // Released.

#ifdef CHECK_ZOOM_RATIO
    if (m_trigger_is_grabbed) {
      float zoom_ratio_after = my_scene->objectManager()->xform().scaling()[0];
      std::cerr << "zoom ratio : " << zoom_ratio_before << " -> " << zoom_ratio_after << " (" << zoom_ratio_after / zoom_ratio_after * 100.0f << " %)"
                << std::endl;
    }
#endif // CHECK_ZOOM_RATIO

    m_trigger_is_grabbed = false;
    my_scene->glyphControllerReleaseFunction(hmdPos, m_initial_head_p, hmdDir, left_p, right_p, m_prev_left_p, m_prev_right_p);

  } // if (right_grabbed && left_grabbed)

  bool prev_button_b_pressed = m_button_b_pressed;
  m_button_b_pressed = (input_state.Buttons & ovrButton_B) != 0;
  if ( (!m_button_b_pressed && prev_button_b_pressed) )
  {
    if (!m_button_b_pressed && prev_button_b_pressed) {
      pressedB();
    }
  }

  bool prev_button_y_pressed = m_button_y_pressed;
  m_button_y_pressed = (input_state.Buttons & ovrButton_Y) != 0;
  if ( (!m_button_y_pressed && prev_button_y_pressed) )
  {
    if (!m_button_y_pressed && prev_button_y_pressed) {
      my_scene->setGuestGlyphPos();
    }
  }



  STOP_AND_RECORD_TIMER(A_4_1_1);
  MAKE_AND_START_TIMER(A_4_1_2);
  // update hands or control sphere.
  rebuildControlSphere(false);
  STOP_AND_RECORD_TIMER(A_4_1_2);

  // #ifdef DEBUG_TOUCH_CONTROLLER
  //     std::cout << "TouchController::frameEvent(): rebuildControlSphere() called." << std::endl;
  // #endif // DEBUG_TOUCH_CONTROLLER

  // update mouse status for 2D widgets.
  MAKE_AND_START_TIMER(A_4_1_7);
  PBVRWidgetHandlerInstance()->setMouseStatus(getMouseStatus());
  STOP_AND_RECORD_TIMER(A_4_1_7);

  // #ifdef DEBUG_TOUCH_CONTROLLER
  //     std::cout << "end of frame event" << std::endl;
  // #endif // DEBUG_TOUCH_CONTROLLER

  STOP_AND_RECORD_TIMER(A_4_1);
}

float calcCursorMove(float x) {
  const static float MIN_STICK = 0.3f;
  const static float MAX_STICK = 1.0f;
  const static float D = MAX_STICK - MIN_STICK;
  const static float MAX_MOVE = 5.0f;

  if (x <= -MIN_STICK) {
    return -MAX_MOVE / D / D * (-x - MIN_STICK) * (-x - MIN_STICK);
  } else if (x >= MIN_STICK) {
    return MAX_MOVE / D / D * (x - MIN_STICK) * (x - MIN_STICK);
  }
  return 0.0f;
}

MouseStatus TouchController::getMouseStatus() {
  static int prev_buttons = 0;

  MouseStatus status = MouseStatus();
  status.updated = false;

  ovrInputState input_state = inputDevice().inputState(ovrControllerType_Touch);
  bool lefty_mode = PBVRWidgetHandlerInstance()->leftyMode();
  int cursor_hand = lefty_mode ? ovrHand_Right : ovrHand_Left;
  int click_button = lefty_mode ? ovrButton_X : ovrButton_A;

  // #ifdef DEBUG_TOUCH_CONTROLLER
  //     std::cout << "TouchController::getMouseStatus() : lefty_mode = " << lefty_mode << std::endl;
  // #endif // DEBUG_TOUCH_CONTROLLER

  // this class handles left thumb stick as d-pad
  ovrVector2f dpad = input_state.Thumbstick[cursor_hand];
  status.cursor_dx = calcCursorMove(dpad.x);
  status.cursor_dy = calcCursorMove(dpad.y);
  if (status.cursor_dx != 0.0f || status.cursor_dy != 0.0f) {
    status.updated = true;
  }

  int buttons = input_state.Buttons;
  status.buttonPressed[0] = ((buttons & click_button) != 0);
  status.buttonPressed[1] = false;
  status.buttonPressed[2] = false;
  if (prev_buttons != buttons) {
    status.updated = true;
  }
  prev_buttons = buttons;

  return status;
}

void TouchController::pressedB()
{
  kvs::jaea::pbvr::Scene *my_scene = static_cast<kvs::jaea::pbvr::Scene *>(scene());
  my_scene->m_current_glyph_pos = my_scene->m_before_right * 100;
  std::cout << my_scene->m_current_glyph_pos.x() << "," << my_scene->m_current_glyph_pos.y() << "," << my_scene->m_current_glyph_pos.z() << std::endl;
}


