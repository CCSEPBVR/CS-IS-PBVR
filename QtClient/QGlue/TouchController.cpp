#include "TouchController.h"
#include "ovr.h"
#include "Scene.h"
#include "objnameutil.h"

#include <kvs/Bmp>
//#include <kvs/ObjectManager>
#include "PBVRObjectManager.h"
#include <kvs/ObjectBase>
#include <kvs/IDManager>
#include <kvs/Matrix44>
#include <kvs/Vector4>

#include <cmath>

#define VR_HANDS_RENDERER_REPETITION_LEVEL 1

#define SHOW_MANIPULATION_OBJECTS

//#define TRACE_COORDINATES
#ifdef TRACE_COORDINATES
#include <QDateTime>
#endif //def TRACE_COORDINATES

inline kvs::Mat4 ToMat4(const OVR::Matrix4f& m) {
    return kvs::Mat4(
            m.M[0][0], m.M[0][1], m.M[0][2], m.M[0][3],
            m.M[1][0], m.M[1][1], m.M[1][2], m.M[1][3],
            m.M[2][0], m.M[2][1], m.M[2][2], m.M[2][3],
            m.M[3][0], m.M[3][1], m.M[3][2], m.M[3][3]
            );
}

inline kvs::Vec3 ToVec3(const OVR::Vector3f& v) {
    return kvs::Vec3(v.x, v.y, v.z);
}

inline OVR::Vector3f ToVector3f(const kvs::Vec3& v) {
    return OVR::Vector3f(v.x(), v.y(), v.z());
}

namespace kvs
{
namespace oculus
{
namespace jaea
{

LineObjectProxy TouchController::m_control_sphere;
int TouchController::m_control_sphere_id;
LineObjectProxy TouchController::m_bounding_box;
int TouchController::m_bounding_box_id;
LineObjectProxy TouchController::m_bounding_box_submesh;
int TouchController::m_bounding_box_submesh_id;
TexturedPolygonObjectProxy TouchController::m_bounding_box_labels;
int TouchController::m_bounding_box_labels_id;
PolygonObjectProxy TouchController::m_vr_hands;
int TouchController::m_vr_hands_id;

#ifdef TRACE_COORDINATES
std::ofstream trace_log;
long long int frame_count;
qint64 trace_msec;
#endif // TRACE_COORDINATES

TouchController::TouchController(kvs::oculus::jaea::HeadMountedDisplay& hmd, kvs::oculus::jaea::Scene *scene ):
    kvs::oculus::jaea::ControllerBase(hmd, scene),
    //m_is_grabbed( false ),
    m_is_first_frame(true),
    m_both_is_grabbed( false ),
    m_button_a_pressed(false),
    m_button_b_pressed(false),
    m_button_x_pressed(false),
    m_button_y_pressed(false),
    m_touch_distance( 0.0f ),
    m_rotation_factor( 5000.0f ),
    m_translation_factor( 5000.0f ),
    m_scaling_factor( 30000.0f ),
    m_rot_start_pos_right(0.0f, 0.0f, 0.0f),
    m_widget_handler(nullptr),
    m_numberTexturesAreInitialized(false),
    m_bouniding_box_scaling_ratio_min( 1.0f ),
    m_bouniding_box_scaling_ratio_max( 2.0f ),
    m_bounding_box_scaling_ratio( 1.0f )
{
    //initializeNumberTextureMaps();
#ifdef TRACE_COORDINATES
    trace_log.open("c:\\Users\\insight\\Documents\\kvs\\oculuspvbr_jaea\\trace_log");
    trace_log << "frame#,ihx,ihy,ihz,hx,hy,hz,lx,ly,lz,rx,ry,rz" << std::endl;
    frame_count = 0;
    trace_msec = QDateTime::currentMSecsSinceEpoch();
#endif // TRACE_COORDINATES
}

void TouchController::initializeEvent()
{
#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "TouchController::initializeEvent()" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    kvs::oculus::jaea::Scene* my_scene = static_cast<kvs::oculus::jaea::Scene*>(scene());

    if ( !inputDevice().hasRight() ) { std::cout << "Oculus Touch (R): Disable" << std::endl; }
    if ( !inputDevice().hasLeft() ) { std::cout << "Oculus Touch (L): Disable" << std::endl; }

    std::pair<int, int> id_pair;

    // VR hand
    //m_vr_hands = new kvs::PolygonObject();
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
    m_vr_hands_renderer = new kvs::StochasticPolygonRenderer();
//    m_vr_hands_renderer->setRepetitionLevel(VR_HANDS_RENDERER_REPETITION_LEVEL);
#ifdef SHOW_MANIPULATION_OBJECTS
    id_pair = my_scene->registerObject( m_vr_hands, m_vr_hands_renderer );
    m_vr_hands_id = id_pair.first;
#endif // SHOW_MANIPULATION_OBJECTS

    // control sphere
    m_control_sphere.setName(OBJ_NAME_CONTROL_SPHERE);
    m_control_sphere.setLineType(kvs::LineObject::Segment);
    m_control_sphere.setColorType(kvs::LineObject::VertexColor);
    m_control_sphere.setColor(RGBColor::Blue());

    m_control_sphere.swap();
    m_control_sphere->setName(OBJ_NAME_CONTROL_SPHERE);
    m_control_sphere->setLineType(kvs::LineObject::Segment);
    m_control_sphere->setColorType(kvs::LineObject::VertexColor);
    m_control_sphere->setColor(RGBColor::Blue());
    m_control_sphere.swap();
    m_control_sphere->setName(OBJ_NAME_CONTROL_SPHERE);
    m_control_sphere->setLineType(kvs::LineObject::Segment);
    m_control_sphere->setColorType(kvs::LineObject::VertexColor);
    m_control_sphere->setColor(RGBColor::Green());
    m_control_sphere.hideAll();
    m_control_sphere_renderer = new kvs::StochasticLineRenderer();
#ifdef SHOW_MANIPULATION_OBJECTS
#ifndef DISABLE_LINES
    id_pair = my_scene->registerObject( m_control_sphere, m_control_sphere_renderer );
    m_control_sphere_id = id_pair.first;
#endif // DISABLE_LINES
#endif // SHOW_MANIPULATION_OBJECTS

    // bounding box
    m_bounding_box.setName(OBJ_NAME_BOUNDING_BOX);
    m_bounding_box.setLineType(kvs::LineObject::Segment);
    m_bounding_box.setColorType(kvs::LineObject::LineColor);
    m_bounding_box.setColor(RGBColor::Blue());

    m_bounding_box.swap();
    m_bounding_box->setName(OBJ_NAME_BOUNDING_BOX);
    m_bounding_box->setLineType(kvs::LineObject::Segment);
    m_bounding_box->setColorType(kvs::LineObject::LineColor);
    m_bounding_box->setColor(RGBColor::Blue());
    m_bounding_box.swap();
    m_bounding_box->setName(OBJ_NAME_BOUNDING_BOX);
    m_bounding_box->setLineType(kvs::LineObject::Segment);
    m_bounding_box->setColorType(kvs::LineObject::LineColor);
    m_bounding_box->setColor(RGBColor::Blue());
    m_bounding_box.hideAll();
    m_bounding_box_renderer = new kvs::StochasticLineRenderer();
#ifdef SHOW_MANIPULATION_OBJECTS
#ifndef DISABLE_LINES
    id_pair = my_scene->registerObject( m_bounding_box, m_bounding_box_renderer );
    m_bounding_box_id = id_pair.first;
#endif // DISABLE_LINES
#endif // SHOW_MANIPULATION_OBJECTS

    // submesh
    m_bounding_box_submesh.setName(OBJ_NAME_BOUNDING_BOX_SUBMESH);
    m_bounding_box_submesh.setLineType(kvs::LineObject::Segment);
    m_bounding_box_submesh.setColorType(kvs::LineObject::LineColor);
    m_bounding_box_submesh.setColor(RGBColor::Cyan());

    m_bounding_box_submesh.swap();
    m_bounding_box_submesh->setName(OBJ_NAME_BOUNDING_BOX_SUBMESH);
    m_bounding_box_submesh->setLineType(kvs::LineObject::Segment);
    m_bounding_box_submesh->setColorType(kvs::LineObject::LineColor);
    m_bounding_box_submesh->setColor(RGBColor::Cyan());
    m_bounding_box_submesh.swap();
    m_bounding_box_submesh->setName(OBJ_NAME_BOUNDING_BOX_SUBMESH);
    m_bounding_box_submesh->setLineType(kvs::LineObject::Segment);
    m_bounding_box_submesh->setColorType(kvs::LineObject::LineColor);
    m_bounding_box_submesh->setColor(RGBColor::Cyan());
    m_bounding_box_submesh.hideAll();
    m_bounding_box_submesh_renderer = new kvs::StochasticLineRenderer();
#ifdef SHOW_MANIPULATION_OBJECTS
#ifndef DISABLE_LINES
#ifdef ENABLE_BOUNDINGBOX_SUBMESH
    id_pair = my_scene->registerObject( m_bounding_box_submesh, m_bounding_box_submesh_renderer );
    m_bounding_box_submesh_id = id_pair.first;
#endif // ENABLE_BOUNDINGBOX_SUBMESH
#endif // DISABLE_LINES
#endif // SHOW_MANIPULATION_OBJECTS

    // set scale label
    m_bounding_box_labels.swap();
    m_bounding_box_labels->setName(OBJ_NAME_BOUNDING_BOX_SCALE_LABELS);
    m_bounding_box_labels->setPolygonType(kvs::jaea::TexturedPolygonObject::Triangle);
    m_bounding_box_labels->setColorType(kvs::jaea::TexturedPolygonObject::VertexColor);
    m_bounding_box_labels->setNormalType(kvs::jaea::TexturedPolygonObject::VertexNormal);
    m_bounding_box_labels.setColor(RGBColor::Blue());
    m_bounding_box_labels.swap();
    m_bounding_box_labels->setName(OBJ_NAME_BOUNDING_BOX_SCALE_LABELS);
    m_bounding_box_labels->setPolygonType(kvs::jaea::TexturedPolygonObject::Triangle);
    m_bounding_box_labels->setColorType(kvs::jaea::TexturedPolygonObject::VertexColor);
    m_bounding_box_labels->setNormalType(kvs::jaea::TexturedPolygonObject::VertexNormal);
    m_bounding_box_labels.setColor(RGBColor::Blue());
    m_bounding_box_labels.hideAll();
    m_bounding_box_labels_renderer = new kvs::jaea::StochasticTexturedPolygonRenderer();
#ifdef SHOW_MANIPULATION_OBJECTS
#ifdef ENABLE_BOUNDINGBOX_LABEL
    id_pair = my_scene->registerObject( m_bounding_box_labels, m_bounding_box_labels_renderer);
    m_bounding_box_labels_id = id_pair.first;
#endif // ENABLE_BOUNDINGBOX_LABEL
#endif // SHOW_MANIPULATION_OBJECTS

//    // VR hand
//    m_vr_hands = new kvs::PolygonObject();
//    m_vr_hands->setName(OBJ_NAME_VR_HANDS);
//    m_vr_hands->show();
////    m_vr_hands_renderer = new kvs::PolygonRenderer();
//    m_vr_hands_renderer = new kvs::StochasticPolygonRenderer();
////    m_vr_hands_renderer->setRepetitionLevel(VR_HANDS_RENDERER_REPETITION_LEVEL);
//#ifdef SHOW_MANIPULATION_OBJECTS
//    std::pair<int, int> ids_vr_hands = my_scene->registerObject( m_vr_hands, m_vr_hands_renderer );
//    m_vr_hands_renderer_id = ids_vr_hands.second;
//#endif // SHOW_MANIPULATION_OBJECTS

//    // control sphere
//    m_control_sphere = new kvs::LineObject();
//    m_control_sphere->setName(OBJ_NAME_CONTROL_SPHERE);
//    //m_control_sphere->show();
//    m_control_sphere->hide();
//    //m_control_sphere_renderer = new kvs::LineRenderer();
//    m_control_sphere_renderer = new kvs::StochasticLineRenderer();
//#ifdef SHOW_MANIPULATION_OBJECTS
//#ifndef DISABLE_LINES
//    my_scene->registerObject( m_control_sphere, m_control_sphere_renderer );
//#endif // DISABLE_LINES
//#endif // SHOW_MANIPULATION_OBJECTS

//    //rebuildControlSphere();

//    // bounding box
//    m_bounding_box = new kvs::LineObject();
//    m_bounding_box->setName(OBJ_NAME_BOUNDING_BOX);
//    m_bounding_box->hide();
//    //m_bounding_box_renderer = new kvs::LineRenderer();
//    m_bounding_box_renderer = new kvs::StochasticLineRenderer();
//#ifdef SHOW_MANIPULATION_OBJECTS
//#ifndef DISABLE_LINES
//    my_scene->registerObject( m_bounding_box, m_bounding_box_renderer );
//#endif // DISABLE_LINES
//#endif // SHOW_MANIPULATION_OBJECTS

//    // submesh
//    m_bounding_box_submesh = new kvs::LineObject();
//    m_bounding_box_submesh->setName(OBJ_NAME_BOUNDING_BOX_SUBMESH);
//    m_bounding_box_submesh->hide();
//    m_bounding_box_submesh_renderer = new kvs::StochasticLineRenderer();
//#ifdef SHOW_MANIPULATION_OBJECTS
//#ifndef DISABLE_LINES
//    my_scene->registerObject( m_bounding_box_submesh, m_bounding_box_submesh_renderer );
//#endif // DISABLE_LINES
//#endif // SHOW_MANIPULATION_OBJECTS

//    // set scale label
//    m_bounding_box_labels = new kvs::jaea::TexturedPolygonObject();
//    m_bounding_box_labels->setName(OBJ_NAME_BOUNDING_BOX_SCALE_LABELS);
//    m_bounding_box_labels->hide();
//    m_bounding_box_labels_renderer = new kvs::jaea::StochasticTexturedPolygonRenderer();
//#ifdef SHOW_MANIPULATION_OBJECTS
//    std::pair<int, int> ids_scale_labels = my_scene->registerObject( m_bounding_box_labels, m_bounding_box_labels_renderer);
//    m_bounding_box_labels_renderer_id = ids_scale_labels.second;
//#endif // SHOW_MANIPULATION_OBJECTS

    const ovrTrackingState ts = inputDevice().trackingState( 0 );
    ovrPosef head =  ts.HeadPose.ThePose;
    m_initial_head_p = kvs::oculus::jaea::ToVec3( head.Position );

    initializeScaleLabel();

    rebuildBoundingBox(true);
    rebuildControlSphere(true);
    rebuildControlSphere(true);
    rebuildControlSphere(false);
    rebuildControlSphere(false);

}

void TouchController::initializeNumberTextureMaps () {
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
    m_index_filename_map[0] = "num_0.bmp";
    m_index_filename_map[1] = "num_1.bmp";
    m_index_filename_map[2] = "num_2.bmp";
    m_index_filename_map[3] = "num_3.bmp";
    m_index_filename_map[4] = "num_4.bmp";
    m_index_filename_map[5] = "num_5.bmp";
    m_index_filename_map[6] = "num_6.bmp";
    m_index_filename_map[7] = "num_7.bmp";
    m_index_filename_map[8] = "num_8.bmp";
    m_index_filename_map[9] = "num_9.bmp";
    m_index_filename_map[10] = "char_dot.bmp";
    m_index_filename_map[11] = "char_plus.bmp";
    m_index_filename_map[12] = "char_minus.bmp";
    m_index_filename_map[13] = "char_e.bmp";
}


void TouchController::initializeScaleLabel(){
#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "initializeScaleLabel called"<< std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    if (!m_numberTexturesAreInitialized) {
        initializeNumberTextureMaps();
        m_numberTexturesAreInitialized = true;

        //load bmp
        // set full path of filename
        std::string basepath = std::string(std::getenv("CGFORMAT_EXT4KVS_SHADER_DIR"))+"/res/";
        for (auto index_filename_pair : m_index_filename_map){
           int texture_id = index_filename_pair.first;
            std::string basename = index_filename_pair.second;

            std::string file_name = basepath + basename;
            kvs::Bmp image_file(file_name);

            size_t image_width = image_file.width();
            size_t image_height = image_file.height();

#ifdef DEBUG_TOUCH_CONTROLLER
            std::cout << "image width" << image_width << std::endl;
            std::cout << "image height" << image_height << std::endl;
            std::cout << "image width, height obtained" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

            const kvs::ValueArray<kvs::UInt8> image_pixel = image_file.pixels();
            kvs::ValueArray<kvs::UInt8> image_pixel_with_alpha(image_width * image_height * 4);
            // rgb -> rgba

#ifdef DEBUG_TOUCH_CONTROLLER
            std::cout << "loop start" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

            size_t index = 0;
            for (size_t iY=0; iY<image_height; iY++){
                for (size_t iX=0; iX<image_width; iX++){
                    kvs::UInt8 r = image_pixel[index * 3 + 0];
                    kvs::UInt8 g = image_pixel[index * 3 + 1];
                    kvs::UInt8 b = image_pixel[index * 3 + 2];
                    kvs::UInt8 a = (r == 255 && g == 255 && b == 255) ? 0 : 255;

                    image_pixel_with_alpha[index * 4 + 0] = r;
                    image_pixel_with_alpha[index * 4 + 1] = g;
                    image_pixel_with_alpha[index * 4 + 2] = b;
                    image_pixel_with_alpha[index * 4 + 3] = a;

                    index++;
                }
            }
//          m_bounding_box_labels->addColorArray(texture_id, image_pixel_with_alpha, image_width, image_height);
            m_bounding_box_labels.swap();
            m_bounding_box_labels->addColorArray(texture_id, image_pixel_with_alpha, image_width, image_height);
            m_bounding_box_labels.swap();
            m_bounding_box_labels->addColorArray(texture_id, image_pixel_with_alpha, image_width, image_height);

#ifdef DEBUG_TOUCH_CONTROLLER
            std::cout << "set color array" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER
        }

//      m_bounding_box_labels->setPolygonType(kvs::jaea::TexturedPolygonObject::Triangle);
//      m_bounding_box_labels->setColorType(kvs::jaea::TexturedPolygonObject::VertexColor);
//      m_bounding_box_labels->setNormalType(kvs::jaea::TexturedPolygonObject::VertexNormal);
    }
}

// 20210118
void TouchController::rebuildBoundingBox(bool force_rebuild_submesh){
    //std::cout << "rebuildBoundingBox() : force_rebuild_submesh=" << force_rebuild_submesh << std::endl;

    kvs::oculus::jaea::Scene* my_scene = static_cast<kvs::oculus::jaea::Scene*>(scene());

    // get head orientation
    const ovrTrackingState ts = inputDevice().trackingState( 0 );
    const ovrQuatf headOrientation = ts.HeadPose.ThePose.Orientation;

    // direction & upvector in camera coordinate system.
    kvs::Vec4 initial_direction = kvs::Vec4(-(my_scene->camera()->position()), 0);
    kvs::Vec4 initial_upvector = kvs::Vec4(my_scene->camera()->upVector(), 0);

    // direction & upvector in Oculus coordinate system.
    const kvs::Mat4 headRotation = ::ToMat4(OVR::Matrix4f( headOrientation ));
    kvs::Vec4 direction_global = headRotation * initial_direction;
    kvs::Vec4 upvector_global = headRotation * initial_upvector;

    // direction & upvector in model coordinate system.
    kvs::Xform obj_xform = my_scene->objectManager()->xform();
    kvs::Mat4 obj_rotation = obj_xform.toMatrix();
    obj_rotation.invert();
    kvs::Vec3 direction_model = ((obj_rotation * direction_global).xyz()).normalized();
    kvs::Vec3 upvector_model = ((obj_rotation * upvector_global).xyz()).normalized();

    // right-direction vector
    kvs::Vec3 right_model = (direction_model.cross(upvector_model)).normalized();

    bool bb_initialized = false;
    kvs::Vec3 min_coord(-3,-3,-3);
    kvs::Vec3 max_coord(3,3,3);
    for (int iobj = 0; iobj < my_scene->objectManager()->numberOfObjects(); iobj++) {
        kvs::ObjectBase *obj = my_scene->objectManager()->object(iobj);
        if (obj != nullptr && !kvs::oculus::jaea::pbvr::ObjectNameUtil::isToIgnoreBoundingBox(obj->name())) {
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
        min_coord = my_scene->objectManager()->minObjectCoord();
        max_coord = my_scene->objectManager()->maxObjectCoord();
    }

#ifdef DEBUG_BBOX
    std::cerr << "TouchController::rebuildBoundingBox(): bb_initialized=" << bb_initialized << "/ min=" << min_coord << ", max=" << max_coord << std::endl;
#endif // DEBUG_BBOX

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "bounding box : min=" << min_coord << " : max=" << max_coord << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

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
    if(range < rangey) range = rangey;
    if(range < rangez) range = rangez;

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "bounding box : range=" << range << " : rangex=" << rangex << " : rangey=" << rangey << " : rangez=" << rangez << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    const static int nDivBasic = 5;
    float standardLength = range / nDivBasic;
    if(m_bounding_box_scaling_ratio > 0){
        standardLength = range / (m_bounding_box_scaling_ratio * nDivBasic);
    }
    if(standardLength <= 0){
        standardLength = 1.0f;
    }

    int nDivX = (int)(rangex / standardLength);
    int nDivY = (int)(rangey / standardLength);
    int nDivZ = (int)(rangez / standardLength);
    if(nDivX <= 0) nDivX = 1;
    if(nDivY <= 0) nDivY = 1;
    if(nDivZ <= 0) nDivZ = 1;

#ifdef DEBUG_BBOX
    std::cerr << "bounding box : nDivX=" << nDivX << " : nDivY=" << nDivY << " : nDivZ=" << nDivZ << std::endl;
#endif // DEBUG_BBOX

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "bounding box : nDivX=" << nDivX << " : nDivY=" << nDivY << " : nDivZ=" << nDivZ << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    //make tiny gap
    minx -= range * 0.0001f;
    miny -= range * 0.0001f;
    minz -= range * 0.0001f;
    maxx += range * 0.0001f;
    maxy += range * 0.0001f;
    maxz += range * 0.0001f;

    //scale label size
    float characterWidth = range * 0.04f;
    float characterHeight = range * 0.05f;
    float scaleLabelNomal_x = -direction_model.x();
    float scaleLabelNomal_y = -direction_model.y();
    float scaleLabelNomal_z = -direction_model.z();

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
        0, 1,   1, 2,   2, 3,   3, 0,
        4, 5,   5, 6,   6, 7,   7, 4,
        0, 4,   1, 5,   2, 6,   3, 7,
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
    //kvs::ValueArray<kvs::UInt8> box_colors(nEdges * 3);
    kvs::ValueArray<kvs::UInt8> box_colors(3);
    box_colors[0] = colorEdgeR;
    box_colors[1] = colorEdgeG;
    box_colors[2] = colorEdgeB;

    if (force_rebuild_submesh){
        for(size_t i = 0; i < nNodes * 3; i++){
            box_coords[i] = box_coords_array[i];
        }
        for(size_t i = 0; i < nEdges * 2; i++){
            box_connections[i] = box_connections_array[i];
        }
        // disabled : 2021/06/17 nakamura
        //for(size_t i = 0; i < nEdges; i++){
        //    box_colors[i*3+0] = colorEdgeR;
        //    box_colors[i*3+1] = colorEdgeG;
        //    box_colors[i*3+2] = colorEdgeB;
        //}
    }

    //submesh
    kvs::ValueArray<kvs::Real32> submesh_coords(nNodesSubmesh * 3);
    kvs::ValueArray<kvs::UInt32> submesh_connections(nEdgesSubmesh * 2);

    // change length of submesh_colors : 2021/06/17 nakamura
    //kvs::ValueArray<kvs::UInt8> submesh_colors(nEdgesSubmesh * 3);
    kvs::ValueArray<kvs::UInt8> submesh_colors(3);
    submesh_colors[0] = colorEdgeSubmeshColorR;
    submesh_colors[1] = colorEdgeSubmeshColorG;
    submesh_colors[2] = colorEdgeSubmeshColorB;
    if (force_rebuild_submesh){
        // disabled : 2021/06/17 nakamura
        //for(size_t i = 0; i < nEdgesSubmesh; i++){
        //    submesh_colors[i*3+0] = colorEdgeSubmeshColorR;
        //    submesh_colors[i*3+1] = colorEdgeSubmeshColorG;
        //    submesh_colors[i*3+2] = colorEdgeSubmeshColorB;
        //}
    }

    // value to display
    kvs::ValueArray<kvs::Real32> scale_values(nNodesOnAxis);

    //scale label
    // TODO : replace to kvs::ValueArray<*> !!!
    std::vector<kvs::Real32> v_scale_label_coords;
    std::vector<kvs::UInt32> v_scale_label_connections;
    std::vector<kvs::Real32> v_scale_label_normals;
    std::vector<kvs::UInt8>  v_scale_label_colors;
    std::vector<kvs::Real32> v_scale_label_texuture2DCoords;
    std::vector<kvs::UInt32> v_scale_label_textureIds;

    size_t offset_node = 0;
    size_t offset_edge = 0;
    size_t offset_scale_label_node = 0;

    for(int i = 0; i < (nDivX - 1) + (nDivY - 1) + (nDivZ - 1); i++){
        float x0 = minx, y0 = miny, z0 = minz;
        float x1 = x0,   y1 = y0,   z1 = z0;
        float x2 = x0,   y2 = y0,   z2 = z0;
        bool x_flag = false;
        bool y_flag = false;
        bool z_flag = false;
        if(i < (nDivX - 1)){
            int index = i + 1;
            // x
            x0 = minx + rangex * index / nDivX; y0 = miny; z0 = minz;
            x1 = x0;                            y1 = maxy; z1 = z0;
            x2 = x0;                            y2 = y0;   z2 = maxz;
            x_flag = true;
        } else if(i < (nDivX - 1) + (nDivY - 1)){
            int index = i - (nDivX - 1) + 1;
            // y
            x0 = minx; y0 = miny + rangey * index / nDivY; z0 = minz;
            x1 = maxx; y1 = y0;                            z1 = z0;
            x2 = x0;   y2 = y0;                            z2 = maxz;
            y_flag = true;
        } else {
            int index = i - ((nDivX - 1) + (nDivY - 1)) + 1;
            // z
            x0 = minx; y0 = miny; z0 = minz + rangez * index / nDivZ;
            x1 = maxx; y1 = y0;   z1 = z0;
            x2 = x0;   y2 = maxy; z2 = z0;
            z_flag = true;
        }

        if (x_flag==true){
            scale_values[i] = x0;
        } else if (y_flag==true) {
            scale_values[i] = y0;
        } else {
            scale_values[i] = z0;
        }

        kvs::Vec3 scale_base(x0, y0, z0);
        std::string strScaleLabel = std::to_string(scale_values[i]);
        int lengthScaleLabel = strScaleLabel.length();

#ifdef DEBUG_TOUCH_CONTROLLER
        std::cout << "### scale label : " << strScaleLabel << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

        if (force_rebuild_submesh) {
            //##################################
            //submesh
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

        // scale labels are always updated.
        {
            //##################################
            //scale label
            for(int iChar=0;iChar<lengthScaleLabel;iChar++){
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
                //kvs::Vec3 v3 = v2 + characterHeight * upvector_model;


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

                //triangle 0
                v_scale_label_connections.push_back(scale_label_node0);
                v_scale_label_connections.push_back(scale_label_node2);
                v_scale_label_connections.push_back(scale_label_node1);
                v_scale_label_normals.push_back(scaleLabelNomal_x);
                v_scale_label_normals.push_back(scaleLabelNomal_y);
                v_scale_label_normals.push_back(scaleLabelNomal_z);

                //triangle 1
                v_scale_label_connections.push_back(scale_label_node2);
                v_scale_label_connections.push_back(scale_label_node3);
                v_scale_label_connections.push_back(scale_label_node1);
                v_scale_label_normals.push_back(scaleLabelNomal_x);
                v_scale_label_normals.push_back(scaleLabelNomal_y);
                v_scale_label_normals.push_back(scaleLabelNomal_z);
            }
        }
    }

    if(force_rebuild_submesh){
        // call my_scene->replaceObject() : 2021/06/17 nakamura
        m_bounding_box.swap();
        m_bounding_box->clear();
        //m_bounding_box->setName(OBJ_NAME_BOUNDING_BOX);0
        m_bounding_box->setCoords(box_coords);
        m_bounding_box->setConnections(box_connections);
        m_bounding_box->setColors(box_colors);
        m_bounding_box->setSize(lineThickness);
        my_scene->replaceObject(m_bounding_box_id, m_bounding_box, false);
//        my_scene->replaceObject(OBJ_NAME_BOUNDING_BOX, m_bounding_box, false);

        // call my_scene->replaceObject() : 2021/06/17 nakamura
        m_bounding_box_submesh.swap();
        m_bounding_box_submesh->clear();
        //m_bounding_box_submesh->setName(OBJ_NAME_BOUNDING_BOX_SUBMESH);
        m_bounding_box_submesh->setCoords(submesh_coords);
        m_bounding_box_submesh->setConnections(submesh_connections);
        m_bounding_box_submesh->setColors(submesh_colors);
        m_bounding_box_submesh->setSize(lineThicknessSubmesh);
#ifdef ENABLE_BOUNDINGBOX_SUBMESH
        my_scene->replaceObject(m_bounding_box_submesh_id, m_bounding_box_submesh, false);
#endif

//        // call my_scene->replaceObject() : 2021/06/17 nakamura
//        m_bounding_box = new kvs::LineObject();
//        //m_bounding_box->clear();
//        m_bounding_box->setName(OBJ_NAME_BOUNDING_BOX);
//        m_bounding_box->setLineType(kvs::LineObject::Segment);
//        m_bounding_box->setColorType(kvs::LineObject::LineColor);
//        m_bounding_box->setCoords(box_coords);
//        m_bounding_box->setConnections(box_connections);
//        m_bounding_box->setColors(box_colors);
//        m_bounding_box->setSize(lineThickness);
//        my_scene->replaceObject(OBJ_NAME_BOUNDING_BOX, m_bounding_box);

//        // call my_scene->replaceObject() : 2021/06/17 nakamura
//        m_bounding_box_submesh = new kvs::LineObject();
//        //m_bounding_box_submesh->clear();
//        m_bounding_box_submesh->setName(OBJ_NAME_BOUNDING_BOX_SUBMESH);
//        m_bounding_box_submesh->setLineType(kvs::LineObject::Segment);
//        m_bounding_box_submesh->setColorType(kvs::LineObject::LineColor);
//        m_bounding_box_submesh->setCoords(submesh_coords);
//        m_bounding_box_submesh->setConnections(submesh_connections);
//        m_bounding_box_submesh->setColors(submesh_colors);
//        m_bounding_box_submesh->setSize(lineThicknessSubmesh);
//        my_scene->replaceObject(OBJ_NAME_BOUNDING_BOX_SUBMESH, m_bounding_box_submesh);

    }

    // scale labels are always updated.
    {
        //vector to array
        kvs::ValueArray<kvs::Real32> scale_label_coords(v_scale_label_coords.size());
        kvs::ValueArray<kvs::UInt32> scale_label_connections(v_scale_label_connections.size());
        kvs::ValueArray<kvs::Real32> scale_label_normals(v_scale_label_normals.size());
        kvs::ValueArray<kvs::UInt8>  scale_label_colors(v_scale_label_colors.size());
        kvs::ValueArray<kvs::Real32> scale_label_texuture2DCoords(v_scale_label_texuture2DCoords.size());
        kvs::ValueArray<kvs::UInt32> scale_label_textureIds(v_scale_label_textureIds.size());
        for(size_t i=0;i<v_scale_label_coords.size();i++){
            scale_label_coords[i] = v_scale_label_coords[i];
        }
        for(size_t i=0;i<v_scale_label_connections.size();i++){
            scale_label_connections[i] = v_scale_label_connections[i];
        }
        for(size_t i=0;i<v_scale_label_normals.size();i++){
            scale_label_normals[i] = v_scale_label_normals[i];
        }
        for(size_t i=0;i<v_scale_label_colors.size();i++){
            scale_label_colors[i] = v_scale_label_colors[i];
        }
        for(size_t i=0;i<v_scale_label_texuture2DCoords.size();i++){
            scale_label_texuture2DCoords[i] = v_scale_label_texuture2DCoords[i];
        }
        for(size_t i=0;i<v_scale_label_textureIds.size();i++){
            scale_label_textureIds[i] = v_scale_label_textureIds[i];
        }

        m_bounding_box_labels.swap();
        m_bounding_box_labels->setCoords(scale_label_coords);
        m_bounding_box_labels->setConnections(scale_label_connections);
        m_bounding_box_labels->setNormals(scale_label_normals);
        m_bounding_box_labels->setColors(scale_label_colors);
        m_bounding_box_labels->setTexture2DCoords(scale_label_texuture2DCoords);
        m_bounding_box_labels->setTextureIds(scale_label_textureIds);
        // to decrease noise
        m_bounding_box_labels_renderer->setAnisotropicFiltering();
#ifdef ENABLE_BOUNDINGBOX_LABEL
        my_scene->replaceObject(m_bounding_box_labels_id, m_bounding_box_labels, false);
        //my_scene->replaceObject(OBJ_NAME_BOUNDING_BOX_SCALE_LABELS, m_bounding_box_labels, false);
#endif

//        m_bounding_box_labels->clear();
//        m_bounding_box_labels->setPolygonType(kvs::jaea::TexturedPolygonObject::Triangle);
//        m_bounding_box_labels->setColorType(kvs::jaea::TexturedPolygonObject::VertexColor);
//        m_bounding_box_labels->setNormalType(kvs::jaea::TexturedPolygonObject::VertexNormal);
//        m_bounding_box_labels->setCoords(scale_label_coords);
//        m_bounding_box_labels->setConnections(scale_label_connections);
//        m_bounding_box_labels->setNormals(scale_label_normals);
//        m_bounding_box_labels->setColors(scale_label_colors);
//        m_bounding_box_labels->setTexture2DCoords(scale_label_texuture2DCoords);
//        m_bounding_box_labels->setTextureIds(scale_label_textureIds);
//        m_bounding_box_labels_renderer = new kvs::jaea::StochasticTexturedPolygonRenderer();
//        // to decrease noise
//        m_bounding_box_labels_renderer->setAnisotropicFiltering();
//        my_scene->replaceRenderer( m_bounding_box_labels_renderer_id, m_bounding_box_labels_renderer);
    }
}

void TouchController::checkScalingRatio(kvs::Real32 scaling_ratio, bool force_update){

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "TouchController::checkScalingRatio(): scaling_ratio=" << scaling_ratio << ", force_update=" << force_update << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    if(!force_update &&
       scaling_ratio >= m_bouniding_box_scaling_ratio_min &&
       scaling_ratio <= m_bouniding_box_scaling_ratio_max){
        rebuildBoundingBox(false);
        return;
    }

    float ratio_log10 = log10(scaling_ratio);
    int ratio_log10_int = (int)ratio_log10;
    float threshold = pow(10,ratio_log10_int);
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

    m_bouniding_box_scaling_ratio_min = threshold_min;
    m_bouniding_box_scaling_ratio_max = threshold_max;
    m_bounding_box_scaling_ratio = scaling_ratio;

    rebuildBoundingBox(true);
}

// 2020118
void TouchController::rebuildControlSphere(bool force_rebuild){
    //static kvs::Vec3 initial_head_p;
    kvs::oculus::jaea::Scene* my_scene = static_cast<kvs::oculus::jaea::Scene*>(scene());

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "rebuild control sphere called" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    // get hands
    const ovrTrackingState ts = inputDevice().trackingState( 0 );
    ovrPosef hands[2] = { ts.HandPoses[ovrHand_Left].ThePose, ts.HandPoses[ovrHand_Right].ThePose };
#ifdef DEBUG_TOUCH_CONTROLLER
    ovrPosef head =  ts.HeadPose.ThePose;
    std::cout << "position : head :       (" << head.Position.x << ", " << head.Position.y << ", " << head.Position.z << ")" << std::endl;
    std::cout << "position : left hand :  (" << hands[ovrHand_Left].Position.x << ", " << hands[ovrHand_Left].Position.y << ", " << hands[ovrHand_Left].Position.z << ")" << std::endl;
    std::cout << "position : right hand : (" << hands[ovrHand_Right].Position.x << ", " << hands[ovrHand_Right].Position.y << ", " << hands[ovrHand_Right].Position.z << ")" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    const kvs::Vec3 left_p = kvs::oculus::jaea::ToVec3( hands[ovrHand_Left].Position );
    const kvs::Vec3 right_p = kvs::oculus::jaea::ToVec3( hands[ovrHand_Right].Position );
    const kvs::Mat4 left_rot = ToMat4(OVR::Matrix4f(hands[ovrHand_Left].Orientation));
    const kvs::Mat4 right_rot = ToMat4(OVR::Matrix4f(hands[ovrHand_Right].Orientation));

    // set camera info
    kvs::Vec3 camera_position = my_scene->camera()->position();

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "camera position is " << camera_position << std::endl;
    // show hands coordinates
    std::cout << "right_p " << right_p << std::endl;
    std::cout << "left_p  " << left_p << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    const double touch_distance = ( right_p - left_p ).length();
    const double radius = touch_distance / 2;

    float base_position_x = camera_position.x() - m_initial_head_p.x();
    float base_position_y = camera_position.y() - m_initial_head_p.y();
    float base_position_z = camera_position.z() - m_initial_head_p.z();

    float mid_x = (right_p.x() + left_p.x()) / 2 + base_position_x;
    float mid_y = (right_p.y() + left_p.y()) / 2 + base_position_y;
    float mid_z = (right_p.z() + left_p.z()) / 2 + base_position_z;

    float line_thickness = 3.0;

    if ( force_rebuild || (m_both_is_grabbed && PBVRWidgetHandlerInstance()->isEnabledControlSphere())) {
#ifdef DEBUG_TOUCH_CONTROLLER
        std::cout << "control sphere called " << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

#ifndef USE_WIREFRAME_VR_HANDS
        m_vr_hands->hide();
        m_control_sphere->show();
#endif // USE_WIREFRAME_VR_HANDS

        const static size_t num_division = 20;
        const static float angle_increment = kvs::Math::pi * 2 / num_division;

        const static size_t num_nodes = num_division * 3;
        const static size_t num_lines = num_division * 3;

        kvs::ValueArray<kvs::Real32> control_sphere_coords_array(num_nodes * 3);
        kvs::ValueArray<kvs::UInt32> control_sphere_connections_array(num_lines * 2);
        kvs::ValueArray<kvs::UInt8> control_sphere_colors_array(num_nodes * 3);

        for(size_t iNode=0;iNode<num_nodes;iNode++){
            float x=0, y=0, z=0;
            if(iNode < num_division){
                float angle = iNode * angle_increment;
                // TODO : replace to reference to pre-calculated cos/sin tables.
                x = cos(angle);
                y = sin(angle);
                z = 0;
            } else if(iNode < num_division * 2){
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

            if(iNode < num_division){
                control_sphere_colors_array[iNode * 3 + 0] = 0;
                control_sphere_colors_array[iNode * 3 + 1] = 0;
                control_sphere_colors_array[iNode * 3 + 2] = 255;
            } else if(iNode < num_division * 2){
                control_sphere_colors_array[iNode * 3 + 0] = 0;
                control_sphere_colors_array[iNode * 3 + 1] = 255;
                control_sphere_colors_array[iNode * 3 + 2] = 0;
            } else {
                control_sphere_colors_array[iNode * 3 + 0] = 255;
                control_sphere_colors_array[iNode * 3 + 1] = 0;
                control_sphere_colors_array[iNode * 3 + 2] = 0;
            }
        }

        for(size_t iLine=0;iLine<num_lines;iLine++){
            kvs::UInt32 node0 = iLine;
            kvs::UInt32 node1 = node0 + 1;
            if(iLine == num_division - 1){
                node1 = 0;
            } else if(iLine == num_division * 2 - 1){
                node1 = num_division;
            } else if(iLine == num_division * 3 - 1){
                node1 = num_division * 2;
            }

            control_sphere_connections_array[iLine * 2 + 0] = node0;
            control_sphere_connections_array[iLine * 2 + 1] = node1;
        }

        m_control_sphere.swap();
        m_control_sphere->clear();
        //m_control_sphere->setName(OBJ_NAME_CONTROL_SPHERE);
        m_control_sphere->setCoords(control_sphere_coords_array);
        m_control_sphere->setLineType(kvs::LineObject::Segment);
        m_control_sphere->setConnections(control_sphere_connections_array);
        m_control_sphere->setColorType(kvs::LineObject::VertexColor);
        m_control_sphere->setColors(control_sphere_colors_array);
        m_control_sphere->setSize(line_thickness);
        my_scene->replaceObject(m_control_sphere_id, m_control_sphere, false);

//        // call my_scene->replaceObject() : 2021/06/17 nakamura
//        m_control_sphere = new kvs::LineObject();
//        //m_control_sphere->clear();
//        m_control_sphere->setName(OBJ_NAME_CONTROL_SPHERE);
//        m_control_sphere->setLineType(kvs::LineObject::Segment);
//        m_control_sphere->setColorType(kvs::LineObject::VertexColor);
//        m_control_sphere->setCoords(control_sphere_coords_array);
//        m_control_sphere->setConnections(control_sphere_connections_array);
//        m_control_sphere->setColors(control_sphere_colors_array);
//        m_control_sphere->setSize(line_thickness);
//        my_scene->replaceObject(OBJ_NAME_CONTROL_SPHERE, m_control_sphere);
    } else {
        // VR hands

#ifdef DEBUG_TOUCH_CONTROLLER
        std::cout << "hand position viewer called " << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

        const float hand_size = 0.05f;

        // position of hands
        float rx = right_p.x() + base_position_x;
        float ry = right_p.y() + base_position_y;
        float rz = right_p.z() + base_position_z;
        float lx = left_p.x() + base_position_x;
        float ly = left_p.y() + base_position_y;
        float lz = left_p.z() + base_position_z;

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
            rx_min, ry_min, rz_min,
            rx_max, ry_min, rz_min,
            rx_max, ry_max, rz_min,
            rx_min, ry_max, rz_min,
            rx_min, ry_min, rz_max,
            rx_max, ry_min, rz_max,
            rx_max, ry_max, rz_max,
            rx_min, ry_max, rz_max,

            lx_min, ly_min, lz_min,
            lx_max, ly_min, lz_min,
            lx_max, ly_max, lz_min,
            lx_min, ly_max, lz_min,
            lx_min, ly_min, lz_max,
            lx_max, ly_min, lz_max,
            lx_max, ly_max, lz_max,
            lx_min, ly_max, lz_max,
        };

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
#else // USE_WIREFRAME_VR_HANDS
//        m_vr_hands->show();
//        m_control_sphere->hide();

        const int ncoords = 8;
        const int ntriangles = 8;

        const kvs::Vec4 nvx(hand_size, 0, 0, 0);
        const kvs::Vec4 nvy(0, hand_size, 0, 0);
        const kvs::Vec4 nvz(0, 0, -hand_size*2.0f, 0);

        kvs::Vec4 lvx = left_rot * nvx;
        kvs::Vec4 lvy = left_rot * nvy;
        kvs::Vec4 lvz = left_rot * nvz;

        kvs::Vec4 rvx = right_rot * nvx;
        kvs::Vec4 rvy = right_rot * nvy;
        kvs::Vec4 rvz = right_rot * nvz;

        // coords
        kvs::Real32 coords[ncoords*3] {
            lx,         ly,         lz,          // 0 : left[0]
            lx+lvx.x(), ly+lvx.y(), lz+lvx.z(),  // 1 : left[1]
            lx+lvy.x(), ly+lvy.y(), lz+lvy.z(),  // 2 : left[2]
            lx+lvz.x(), ly+lvz.y(), lz+lvz.z(),  // 3 : left[3]
            rx,         ry,         rz,          // 4 : right[0]
            rx+rvx.x(), ry+rvx.y(), rz+rvx.z(),  // 5 : right[1]
            rx+rvy.x(), ry+rvy.y(), rz+rvy.z(), // 6 : right[2]
            rx+rvz.x(), ry+rvz.y(), rz+rvz.z(),  // 7 : right[3]
        };

        // connections
        kvs::UInt32 connections[ntriangles*3] {
//                        0,2,1,
//                        0,1,3,
//                        0,3,2,
//                        1,2,3,
//                        4,6,5,
//                        4,5,7,
//                        4,7,6,
//                        5,6,7,
                        0,1,2,
                        0,3,1,
                        0,2,3,
                        1,3,2,
                        4,5,6,
                        4,7,5,
                        4,6,7,
                        5,7,6,
        };
        // polygon colors
        kvs::UInt8 colors[ntriangles*3] {
            255,  0,  0,  // left[0]
            255,  0,  0,  // left[1]
            255,  0,  0,  // left[2]
            255,  0,  0,  // left[3]
              0,  0,255,  // right[0]
              0,  0,255,  // right[1]
              0,  0,255,  // right[2]
              0,  0,255,  // right[3]
        };
        // normals
        kvs::Real32 normals[ntriangles*3];
        for (int itri = 0; itri < ntriangles; itri++) {
            UInt32 n[3] {connections[itri*3+0], connections[itri*3+1], connections[itri*3+2]};
            kvs::Vector3f v0 (coords[n[0]*3+0],coords[n[0]*3+1],coords[n[0]*3+2]);
            kvs::Vector3f v1 (coords[n[1]*3+0],coords[n[1]*3+1],coords[n[1]*3+2]);
            kvs::Vector3f v2 (coords[n[2]*3+0],coords[n[2]*3+1],coords[n[2]*3+2]);
            kvs::Vector3f v01 = v1 - v0;
            kvs::Vector3f v02 = v2 - v0;
            kvs::Vector3f normal = v01.cross(v02).normalized();
            normals[itri*3+0] = -normal.x();
            normals[itri*3+1] = -normal.y();
            normals[itri*3+2] = -normal.z();
        }
        kvs::ValueArray<kvs::Real32> vr_hands_coords(coords, ncoords*3);
        kvs::ValueArray<kvs::UInt32> vr_hands_connections(connections, ntriangles*3);
        kvs::ValueArray<kvs::UInt8> vr_hands_colors(colors, ntriangles*3);
        kvs::ValueArray<kvs::Real32> vr_hands_normals(normals, ntriangles*3);

        m_vr_hands.swap();
        //m_vr_hands->clear();
        m_vr_hands->setCoords(vr_hands_coords);
        m_vr_hands->setConnections(vr_hands_connections);
        m_vr_hands->setColors(vr_hands_colors);
        m_vr_hands->setOpacity(100);
        m_vr_hands->setNormals(vr_hands_normals);
        my_scene->replaceObject(m_vr_hands_id, m_vr_hands, false);
        //        my_scene->replaceObject(OBJ_NAME_VR_HANDS, m_vr_hands, false);

        m_vr_hands.showAll();

        m_control_sphere.hideAll();

//        m_vr_hands->clear();
//        m_vr_hands->setPolygonTypeToTriangle();
//        m_vr_hands->setColorTypeToPolygon();
//        m_vr_hands->setNormalTypeToPolygon();
//        m_vr_hands->setCoords(vr_hands_coords);
//        m_vr_hands->setConnections(vr_hands_connections);
//        m_vr_hands->setColors(vr_hands_colors);
//        m_vr_hands->setOpacity(100);
//        m_vr_hands->setNormals(vr_hands_normals);

//        //std::cout << "rebuildContrlSphere() : rebuild m_vr_hands finished." << std::endl;

//        // if not replace, rebuilt values of m_vr_hands are not recognized.
//        //m_vr_hands_renderer = new kvs::PolygonRenderer();
//        m_vr_hands_renderer = new kvs::StochasticPolygonRenderer();
////        m_vr_hands_renderer->setRepetitionLevel(VR_HANDS_RENDERER_REPETITION_LEVEL);
//        my_scene->replaceRenderer( m_vr_hands_renderer_id, m_vr_hands_renderer);

        //std::cout << "rebuildContrlSphere() : replace vr_hands_renderer finished." << std::endl;

#endif
    }
}

void TouchController::frameEvent()
{
    static const float GRAB_THRESHOLD = 0.4f;

    kvs::oculus::jaea::Scene* my_scene = static_cast<kvs::oculus::jaea::Scene*>(scene());

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "frameEvent() is being called" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    const ovrTrackingState ts = inputDevice().trackingState( 0 );
    ovrPosef head =  ts.HeadPose.ThePose;
    ovrPosef hands[2] = { ts.HandPoses[ovrHand_Left].ThePose, ts.HandPoses[ovrHand_Right].ThePose };
    const kvs::Vec3 left_p = kvs::oculus::jaea::ToVec3( hands[ovrHand_Left].Position );
    const kvs::Vec3 right_p = kvs::oculus::jaea::ToVec3( hands[ovrHand_Right].Position );

    ovrInputState input_state = inputDevice().inputState( ovrControllerType_Touch );
#ifdef GRAB_USING_INDEX_TRIGGER
    const bool left_grabbed = input_state.IndexTrigger[ovrHand_Left] > GRAB_THRESHOLD;
    const bool right_grabbed = input_state.IndexTrigger[ovrHand_Right] > GRAB_THRESHOLD;
#else
    const bool left_grabbed = input_state.HandTrigger[ovrHand_Left] > GRAB_THRESHOLD;
    const bool right_grabbed = input_state.HandTrigger[ovrHand_Right] > GRAB_THRESHOLD;
#endif

//    if ( m_is_first_frame ){
//        m_initial_head_p = kvs::oculus::jaea::ToVec3( head.Position );
//        m_is_first_frame = false;
//    }
    m_is_first_frame = false;

#ifdef TRACE_COORDINATES
    trace_log
            << frame_count << ", "
            << m_initial_head_p.x() << ", " << m_initial_head_p.y() << ", " << m_initial_head_p.z() << ", "
            << head.Position.x << ", " << head.Position.y << ", " << head.Position.z << ", "
            << left_p.x() << ", " << left_p.y() << ", " << left_p.z() << ", "
            << right_p.x() << ", " << right_p.y() << ", " << right_p.z() << std::endl;
    frame_count++;
#endif // TRACE_COORDINATES


#ifdef DEBUG_TOUCH_CONTROLLER
    // if ( !inputDevice().hasRight() ) { std::cout << "Oculus Touch (R): Disable" << std::endl; }
    // if ( inputDevice().hasRight() ) { std::cout << "Oculus Touch (R): Available" << std::endl; }
    // if ( !inputDevice().hasLeft() ) { std::cout << "Oculus Touch (L): Disable" << std::endl; }
    //std::cout << "  Position (R): " << kvs::oculus::jaea::ToVec3( hands[ovrHand_Left].Position ) << std::endl;
    //std::cout << "  Position (L): " << kvs::oculus::jaea::ToVec3( hands[ovrHand_Right].Position ) << std::endl;
    if ( input_state.Buttons & ovrButton_A ) { std::cout << "Pressed A" << std::endl; }
    if ( input_state.Buttons & ovrButton_B ) { std::cout << "Pressed B" << std::endl; }
    if ( input_state.Buttons & ovrButton_X ) { std::cout << "Pressed X" << std::endl; }
    if ( input_state.Buttons & ovrButton_Y ) { std::cout << "Pressed Y" << std::endl; }
    if ( input_state.HandTrigger[ovrHand_Left] > GRAB_THRESHOLD ) { std::cout << "Grip Left HandTrigger" << std::endl; }
    if ( input_state.HandTrigger[ovrHand_Right] > GRAB_THRESHOLD ) { std::cout << "Grip Right HandTrigger" << std::endl; }
    if ( input_state.IndexTrigger[ovrHand_Left] > GRAB_THRESHOLD ) { std::cout << "Grip Left IndexTrigger" << std::endl; }
    if ( input_state.IndexTrigger[ovrHand_Right] > GRAB_THRESHOLD ) { std::cout << "Grip Right IndexTrigger" << std::endl; }
#endif

    //            kvs::Vec3 eyePos = ::ToVec3(ts.HeadPose.ThePose.Position);
    //            OVR::Matrix4f R = OVR::Matrix4f(ts.HeadPose.ThePose.Orientation);
    //            OVR::Vector3f forward = R.Transform(OVR::Vector3f(0.0,0.0,-6.0));
    kvs::Vec3 hmdPos = ::ToVec3(head.Position);
    OVR::Matrix4f R = OVR::Matrix4f(head.Orientation);
    OVR::Vector3f forward = R.Transform(OVR::Vector3f(ToVector3f(Screen::KVS_CAMERA_INITIAL_POSITION)));
    kvs::Vec3 hmdDir = ::ToVec3(forward);

    if ( right_grabbed && left_grabbed )
    {
        if ( m_both_is_grabbed )
        {
            my_scene->threedcontrollerPressFunction(hmdPos, m_initial_head_p, hmdDir, left_p, right_p, m_prev_left_p, m_prev_right_p);
            m_prev_right_p = right_p;
            m_prev_left_p = left_p;
        }
        else {
            // start grabbing
            m_both_is_grabbed = true;
            m_start_right_p = right_p;
            m_start_left_p = left_p;
            m_prev_right_p = right_p;
            m_prev_left_p = left_p;
        } // if ( m_both_is_grabbed )
    } // if (right_grabbed && left_grabbed)
    else {
        // Released.
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

    if ((!m_button_a_pressed && prev_button_a_pressed) ||
        (!m_button_b_pressed && prev_button_b_pressed) ||
        (!m_button_x_pressed && prev_button_x_pressed) ||
        (!m_button_y_pressed && prev_button_y_pressed))
    {
        std::stringstream msg;
        msg << "Released Button :";
        if (!m_button_a_pressed && prev_button_a_pressed) { msg << " A"; }
        if (!m_button_b_pressed && prev_button_b_pressed) { msg << " B"; }
        if (!m_button_x_pressed && prev_button_x_pressed) { msg << " X"; }
        if (!m_button_y_pressed && prev_button_y_pressed) { msg << " Y"; }
        msg << " : " << prev_button_a_pressed << m_button_a_pressed
            << " "  << prev_button_b_pressed << m_button_b_pressed
            << " "  << prev_button_x_pressed << m_button_x_pressed
            << " "  << prev_button_y_pressed << m_button_y_pressed;
        std::cout << msg.str() << std::endl;
    }
#endif // DEBUG_SCENE


    // update hands or control sphere.
    rebuildControlSphere(false);

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "TouchController::frameEvent(): rebuildControlSphere() called." << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    // update bounding box
    if (PBVRWidgetHandlerInstance()->isEnabledBoundingBox()) {
        kvs::Real32 currentScalingRatio = my_scene->getScalingRatioAccum();
        bool force_update =!m_bounding_box->isShown(); // force update == check box is checked but bounding box is hidden.
        checkScalingRatio(currentScalingRatio, force_update);

        m_bounding_box->show();
        m_bounding_box_submesh->show();
        m_bounding_box_labels->show();
    } else {
        m_bounding_box->hide();
        m_bounding_box_submesh->hide();
        m_bounding_box_labels->hide();
    }

    // update mouse status for 2D widgets.
    PBVRWidgetHandlerInstance()->setMouseStatus(getMouseStatus());
//    if (m_widget_handler != nullptr) {
//        m_widget_handler->setMouseStatus(getMouseStatus());
//    }

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "end of frame event" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER
}

float calcCursorMove (float x) {
    const static float MIN_STICK = 0.3f;
    const static float MAX_STICK = 1.0f;
    const static float D = MAX_STICK - MIN_STICK;
    const static float MAX_MOVE = 5.0f;

    if(x <= -MIN_STICK) {
        return - MAX_MOVE / D /D * (-x - MIN_STICK) * (-x - MIN_STICK);
    } else if(x >= MIN_STICK) {
        return MAX_MOVE / D /D * (x - MIN_STICK) * (x - MIN_STICK);
    }
    return 0.0f;
}

MouseStatus TouchController::getMouseStatus () {
    static int prev_buttons = 0;

    MouseStatus status = MouseStatus();
    status.updated = false;

    ovrInputState input_state = inputDevice().inputState(ovrControllerType_Touch);
    bool lefty_mode = PBVRWidgetHandlerInstance()->leftyMode();
    int cursor_hand = lefty_mode ? ovrHand_Right : ovrHand_Left;
    int click_button = lefty_mode ? ovrButton_X : ovrButton_A;

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cout << "TouchController::getMouseStatus() : lefty_mode = " << lefty_mode << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER


    // this class handles left thumb stick as d-pad
    ovrVector2f dpad = input_state.Thumbstick[cursor_hand];
    status.cursor_dx = calcCursorMove (dpad.x);
    status.cursor_dy = calcCursorMove (dpad.y);
    if (status.cursor_dx != 0.0f || status.cursor_dy != 0.0f) {
        status.updated = true;
    }

    int buttons = input_state.Buttons;
//    if ((buttons & click_button) != 0) {
//        status.buttonPressed[0] = true;
//    }
    status.buttonPressed[0] = ((buttons & click_button) != 0);
    status.buttonPressed[1] = false;
    status.buttonPressed[2] = false;
    if (prev_buttons != buttons) {
        status.updated = true;
    }
    prev_buttons = buttons;

    return status;
}

} // end of namespace jaea
} // end of namespace oculus
} // end of namespace kvs
