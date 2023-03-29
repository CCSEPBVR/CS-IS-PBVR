#pragma once
#include "Scene.h"
#include "ControllerBase.h"
#include "headmounteddisplay.h"
#include "widgethandler.h"
#include "TexturedPolygonObject.h"

#include <kvs/PointObject>
#include <kvs/LineObject>
#include <kvs/PolygonObject>
#include <kvs/PolygonRenderer>

//#include <kvs/Scene>

//#include <kvs/VolumeObjectBase>
//#include <kvs/ArrowGlyph>
//#include <kvs/LineRenderer>
//#include <kvs/StochasticLineRenderer>
//#include <kvs/StochasticPolygonRenderer>
//#include "StochasticTexturedPolygonRenderer.h"

#include "StochasticLineRendererForMeasure.h"
#include "StochasticPolygonRendererForMeasure.h"
#include "StochasticTexturedPolygonRendererForMeasure.h"

#include <map>
#include <set>

namespace kvs
{
namespace oculus
{
namespace jaea
{

class TexturedPolygonObject;
class StochasticTexturedPolygonRenderer;

class LineObjectProxy : public kvs::LineObject {
private:
    LineObject list[2];
    int m_index = 0;
//    std::string m_name;

    LineObject* active() {
        return &list[m_index];
    }

public:
    LineObjectProxy() : m_index(0) {
        list[0] = LineObject();
        list[1] = LineObject();
        list[0].setName("proxy");
        list[1].setName("proxy");
    }

    LineObject* swap() {
        m_index = 1-m_index;
        return active();
    }
    LineObject *operator->() {
        return active();
    }
    operator LineObject*() {
        return active();
    }

    void showAll () {
        list[0].show();
        list[1].show();
    }
    void hideAll () {
        list[0].hide();
        list[1].hide();
    }
    int index() {
        return m_index;
    }
};

class PolygonObjectProxy : public kvs::PolygonObject {
private:
    PolygonObject list[2];
    int m_index = 0;
//    std::string m_name;

    PolygonObject* active() {
        return &list[m_index];
    }

public:
    PolygonObjectProxy() : m_index(0) {
        list[0] = PolygonObject();
        list[1] = PolygonObject();
        list[0].setName("proxy");
        list[1].setName("proxy");
    }

    PolygonObject* swap() {
        m_index = 1-m_index;
        return active();
    }
    PolygonObject *operator->() {
        return active();
    }
    operator PolygonObject*() {
        return active();
    }
    void showAll () {
        list[0].show();
        list[1].show();
    }
    void hideAll () {
        list[0].hide();
        list[1].hide();
    }
    int index() {
        return m_index;
    }
};

class TexturedPolygonObjectProxy : public kvs::jaea::TexturedPolygonObject {
private:
    TexturedPolygonObject list[2];
    int m_index = 0;
//    std::string m_name;

    TexturedPolygonObject* active() {
        return &list[m_index];
    }

public:
    TexturedPolygonObjectProxy() : m_index(0) {
        list[0] = TexturedPolygonObject();
        list[1] = TexturedPolygonObject();
        list[0].setName("proxy");
        list[1].setName("proxy");
    }

    TexturedPolygonObject* swap() {
        m_index = 1-m_index;
        return active();
    }
    TexturedPolygonObject *operator->() {
        return active();
    }
    operator TexturedPolygonObject*() {
        return active();
    }
    void showAll () {
        list[0].show();
        list[1].show();
    }
    void hideAll () {
        list[0].hide();
        list[1].hide();
    }
    int index() {
        return m_index;
    }
};

//class TouchController : public kvs::oculus::jaea::ControllerBase
class TouchController : public ControllerBase
{
private:
    // added 20210111
    bool m_is_first_frame;

    // mapping tables for char->texId and texId->filename
    std::map<char, int> m_char_index_map;
    std::map<int, std::string> m_index_filename_map;

    //bool m_is_grabbed;
    bool m_both_is_grabbed;

    bool m_button_a_pressed;
    bool m_button_b_pressed;
    bool m_button_x_pressed;
    bool m_button_y_pressed;

    float m_touch_distance;
    float m_rotation_factor;
    float m_translation_factor;
    float m_scaling_factor;
    kvs::Vec3 m_rot_start_pos_right;

    kvs::Vec3 m_prev_left_p;
    kvs::Vec3 m_prev_right_p;
    kvs::Vec3 m_start_left_p;
    kvs::Vec3 m_start_right_p;

    kvs::Vec3 m_initial_head_p;

    kvs::oculus::jaea::WidgetHandler *m_widget_handler;


    // vr hands
//    kvs::PolygonObject* m_vr_hands;
    static PolygonObjectProxy m_vr_hands;
    static int m_vr_hands_id;
    kvs::StochasticPolygonRenderer* m_vr_hands_renderer;
    //kvs::PolygonRenderer* m_vr_hands_renderer;
//    int m_vr_hands_renderer_id;

    // control sphere
    static LineObjectProxy m_control_sphere;
    static int m_control_sphere_id;
//    kvs::LineObject* m_control_sphere;
    kvs::StochasticLineRenderer* m_control_sphere_renderer;

    // bounding box
//    kvs::LineObject* m_bounding_box;
    static LineObjectProxy m_bounding_box;
    static int m_bounding_box_id;
    kvs::StochasticLineRenderer* m_bounding_box_renderer;

    // submesh of bounding box
//    kvs::LineObject* m_bounding_box_submesh;
    static LineObjectProxy m_bounding_box_submesh;
    static int m_bounding_box_submesh_id;
    kvs::StochasticLineRenderer* m_bounding_box_submesh_renderer;

    // scale labels for submesh
    //kvs::jaea::TexturedPolygonObject* m_test_scale_box;
    //kvs::jaea::StochasticTexturedPolygonRenderer* m_test_scale_box_renderer;
    static TexturedPolygonObjectProxy m_bounding_box_labels;
    static int m_bounding_box_labels_id;
    kvs::jaea::StochasticTexturedPolygonRenderer* m_bounding_box_labels_renderer;
    bool m_numberTexturesAreInitialized;
//    kvs::jaea::TexturedPolygonObject* m_bounding_box_labels;
//    kvs::jaea::StochasticTexturedPolygonRenderer* m_bounding_box_labels_renderer;
//    int m_bounding_box_labels_renderer_id;

    // bounding box submesh parameters.
    kvs::Real32 m_bouniding_box_scaling_ratio_min;
    kvs::Real32 m_bouniding_box_scaling_ratio_max;
    kvs::Real32 m_bounding_box_scaling_ratio;


public:
    // default values for rescale submesh of bounding box
    constexpr static kvs::Real32 DEAFULT_SCALING_RATIO = 1.0f;
    constexpr static kvs::Real32 DEFAULT_THRESHOLD_MIN = 1.0f;
    constexpr static kvs::Real32 DEFAULT_THRESHOLD_MAX = 2.0;

private:
    void initializeNumberTextureMaps();
    MouseStatus getMouseStatus ();

protected:
    virtual void initializeScaleLabel();
    //    virtual void rebuildControlSphere();
    virtual void rebuildControlSphere(bool force_rebuild_submesh);
    virtual void rebuildBoundingBox(bool force_rebuild_submesh);
    virtual void checkScalingRatio(kvs::Real32 scaling_ratio, bool force_update = false);

public:
    //TouchController( kvs::oculus::jaea::Screen* screen );
//    TouchController( Screen* screen );
    TouchController( kvs::oculus::jaea::HeadMountedDisplay& hmd, kvs::oculus::jaea::Scene *scene );

    float rotationFactor() const { return m_rotation_factor; }
    float translationFactor() const { return m_translation_factor; }
    float scalingFactor() const { return m_scaling_factor; }
    void setRotationFactor( const float factor ) { m_rotation_factor = factor; }
    void setTranslationFactor( const float factor ) { m_translation_factor = factor; }
    void setScalingFactor( const float factor ) { m_scaling_factor = factor; }

    void setWidgetHandler (kvs::oculus::jaea::WidgetHandler *handler) {
        m_widget_handler = handler;
    }

    virtual void initializeEvent();
    virtual void frameEvent();
};

} // end of namespace jaea

} // end of namespace oculus

} // end of namespace kvs
