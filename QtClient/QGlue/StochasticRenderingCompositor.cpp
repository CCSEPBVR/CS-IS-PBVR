/*****************************************************************************/
/**
 *  @file   StochasticRenderingCompositor.cpp
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
#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "StochasticRenderingCompositor.h"

#include <kvs/Assert>
#include <kvs/Background>
#include <kvs/Camera>
#include <kvs/EventHandler>
#include <kvs/IDManager>
#include <kvs/Light>
#include <kvs/OpenGL>
#include <kvs/PaintEvent>
#include <kvs/ParticleBasedRenderer>
#include <kvs/RendererManager>
#include <kvs/ScreenBase>
#include <kvs/StochasticRendererBase>

#include "ObjectManager.h"
#include "Scene.h"
#include "TimeMeasureUtility.h"
#include "objnameutil.h"
#include "Client/PBRProxy.h"

#ifdef DISP_MODE_VR
#include "pbvrwidgethandler.h"
//#include "ovr.h"
#endif

extern kvs::Int64 frame_index;

namespace kvs {
namespace jaea {
namespace pbvr {

/*===========================================================================*/
/**
 *  @brief  Constructs a new StochasticRenderingCompositor class.
 *  @param  scene [in] pointer to the scene
 */
/*===========================================================================*/
StochasticRenderingCompositor::StochasticRenderingCompositor(kvs::jaea::pbvr::Scene *scene)
    : m_scene(scene), m_window_width(0), m_window_height(0), m_repetition_level(1), m_coarse_level(1), m_enable_lod(false),
#ifdef MR_MODE
    m_enable_camera_image(true), m_stereo_camera(nullptr),
#endif // ifdef MR_MODE
    m_enable_refinement(false), m_clear_depth(1.0f) {

#ifdef ENABLE_TIME_MEASURE
    //    m_lengthTimerArray_camera = 100;
    //    m_timerCount_camera = 0;
    //    m_array_time_camera = new double[m_lengthTimerArray_camera];

    m_max_num_timers = 20;
    m_max_repetition_level = 20;
    m_temp_timer = new kvs::Timer[m_max_num_timers];
    m_temp_array_time = new double[m_max_num_timers];
    m_lengthTimerArray = 100; // <- 100 times * 2 eyes
    // m_timerCount_renderer = 0;
    // m_array_time_renderer = (double**)malloc(sizeof(double*) * m_max_num_timers_renderer);
    m_array_time = new double ***[m_max_num_timers];
    for (int iRenderer = 0; iRenderer < m_max_num_timers; iRenderer++) {
        m_array_time[iRenderer] = new double **[2];
        for (int iEye = 0; iEye < 2; iEye++) {
            m_array_time[iRenderer][iEye] = new double *[m_max_repetition_level];
            for (int iRepetition = 0; iRepetition < m_max_repetition_level; iRepetition++) {
                m_array_time[iRenderer][iEye][iRepetition] = new double[m_lengthTimerArray];
            }
        }
    }
#endif // ENABLE_TIME_MEASURE
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new StochasticRenderingCompositor class.
 *  @param  scene [in] pointer to the scene
 */
/*===========================================================================*/
StochasticRenderingCompositor::~StochasticRenderingCompositor() {

#ifdef ENABLE_TIME_MEASURE
    //    delete[] m_array_time_camera;

    delete[] m_temp_timer;
    delete[] m_temp_array_time;

    for (int iRenderer = 0; iRenderer < m_max_num_timers; iRenderer++) {
        for (int iEye = 0; iEye < 2; iEye++) {
            for (int iRepetition = 0; iRepetition < m_max_repetition_level; iRepetition++) {
                delete[] m_array_time[iRenderer][iEye][iRepetition];
            }
            delete[] m_array_time[iRenderer][iEye];
        }
        delete[] m_array_time[iRenderer];
    }
    delete[] m_array_time;
#endif // ENABLE_TIME_MEASURE
}

/*===========================================================================*/
/**
 *  @brief  Set back ground color.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::setBackgroundColor(const kvs::RGBColor& c) {
    kvs::Vec3 v = c.toVec3();
    setBackgroundColor(v.x(), v.y(), v.z(), 1.0f);
}

/*===========================================================================*/
/**
 *  @brief  Set back ground color.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::setBackgroundColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    m_background_color[0] = r;
    m_background_color[1] = g;
    m_background_color[2] = b;
    m_background_color[3] = a;
}

/*===========================================================================*/
/**
 *  @brief  Set clear depth value.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::setClearDepth(GLfloat depth) { m_clear_depth = depth; }

/*===========================================================================*/
/**
 *  @brief  Updates the scene.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::update() {
    KVS_ASSERT(m_scene);

#ifdef DISP_MODE_2D
    kvs::OpenGL::WithPushedMatrix p(GL_MODELVIEW);
    p.loadIdentity();

    m_scene->updateGLProjectionMatrix();
    m_scene->updateGLViewingMatrix();
    m_scene->updateGLLightParameters();
    m_scene->background()->apply();
#endif

    {
#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
        this->draw();
#else
        if (m_scene->objectManager()->hasObject()) {
            this->draw();
        } else {
            m_scene->updateGLModelingMatrix();
        }
#endif
    }
}

static size_t prev_repetition_level = 0;
static bool repetition_level_changed = false;

/*===========================================================================*/
/**
 *  @brief  Draws the objects with stochastic renderers.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::draw() {
    MAKE_AND_START_TIMER(A_6_1or2_2_1);
#ifdef MR_MODE
    // update camera image rendering flag
    m_enable_camera_image = PBVRWidgetHandlerInstance()->isEnabledCameraImage();
#endif // MR_MODE

    const size_t size = m_scene->IDManager()->size();
#ifdef ENABLE_FEATURE_DRAW_PARTICLE
    static bool pbrproxy_found = false;
    if(!pbrproxy_found){
        for (size_t i = 0; i < size; i++) {
            kvs::IDManager::IDPair id = m_scene->IDManager()->id(i);
            kvs::RendererBase *renderer = m_scene->rendererManager()->renderer(id.second);
            if (typeid(*renderer) == typeid(kvs::visclient::PBRProxy)) {
                pbrproxy_found = true;
                break;
            }
        }
    }
    if (!pbrproxy_found) {
        return;
    }
#endif // ENABLE_FEATURE_DRAW_PARTICLE

    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_1, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_1)  "));
    MAKE_AND_START_TIMER(A_6_1or2_2_2);

    // check repetition level is changed or not.
    repetition_level_changed = false;
    if (prev_repetition_level != m_repetition_level) {

        std::cerr << "Compositor::draw(): repetiton_level is changed to " << m_repetition_level << ". " <<  std::endl;

        prev_repetition_level = m_repetition_level;
        repetition_level_changed = true;
    }

    kvs::OpenGL::WithPushedAttrib p(GL_ALL_ATTRIB_BITS);

#ifdef DISP_MODE_VR
    // set viewport
    int bufferWidth = m_buffer_size[0];
    int bufferHeight = m_buffer_size[1];
    glViewport(0, 0, bufferWidth / 2, bufferHeight);
#endif

    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_2, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_2)  "));
    MAKE_AND_START_TIMER(A_6_1or2_2_3);
    this->check_window_created();
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_3, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_3)  "));
    MAKE_AND_START_TIMER(A_6_1or2_2_4);
    this->check_window_resized();
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_4, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_4)  "));
    MAKE_AND_START_TIMER(A_6_1or2_2_5);
    this->check_object_changed();
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_5, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_5)  "));

    MAKE_AND_START_TIMER(A_6_1or2_2_6);
    // LOD control.
    size_t repetitions = m_repetition_level;
    kvs::Vec3 camera_position = m_scene->camera()->position();
    kvs::Vec3 light_position = m_scene->light()->position();
    kvs::Mat4 object_xform = this->object_xform();
    if (m_camera_position != camera_position || m_light_position != light_position || m_object_xform != object_xform) {
        if (m_enable_lod) {
            repetitions = m_coarse_level;
        }
        m_camera_position = camera_position;
        m_light_position = light_position;
        m_object_xform = object_xform;
        m_ensemble_buffer.clear();
    }
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_6, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_6)  "));

    MAKE_AND_START_TIMER(A_6_1or2_2_7);
    // Setup engine.
    this->engines_setup();
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_7, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_7)  "));

    MAKE_AND_START_TIMER(A_6_1or2_2_8);
    // Ensemble rendering.
    const bool reset_count = !m_enable_refinement;
    if (reset_count) {
        m_ensemble_buffer.clear();
    }
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_8, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_8)  "));

#ifdef ENABLE_TIME_MEASURE
    // render other stochastic objects
    // for ( size_t i = 0; i < size+1; i++ ){
    //    m_array_time_renderer[i][m_timerCount_renderer % m_lengthTimerArray_renderer] = 0;
    //}
    for (size_t iRenderer = 0; iRenderer < m_max_num_timers; iRenderer++) {
        m_temp_array_time[iRenderer] = 0;
    }
#endif // ENABLE_TIME_MEASURE

    MAKE_TIMER(A_6_1or2_2_9);
    MAKE_TIMER(A_6_1or2_2_10);
    MAKE_TIMER(A_6_1or2_2_11);

    for (size_t i = 0; i < repetitions; i++) {
        //        START_TIMER(A_6_1or2_2_9);
        m_ensemble_buffer.bind();
        STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_9, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_9)  "));

        //        START_TIMER(A_6_1or2_2_10);
        this->engines_draw();
        STOP_AND_RECORD_TIMER_WITH_ID(
            A_6_1or2_2_10, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_10) class-kvs::StochasticRenderingCompositor engines_draw"));

        //        START_TIMER(A_6_1or2_2_11);
        m_ensemble_buffer.unbind();
        m_ensemble_buffer.add();
        STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_11, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_11)  "));

#ifdef ENABLE_TIME_MEASURE
        for (size_t iRenderer = 0; iRenderer < m_max_num_timers; iRenderer++) {
            m_array_time[iRenderer][m_eye][i][frame_index % m_lengthTimerArray] = m_temp_array_time[iRenderer];
        }
#endif // ENABLE_TIME_MEASURE
    }

#ifdef ENABLE_TIME_MEASURE
#if 0
    m_timerCount_renderer++;
    for ( size_t i = 0; i < size; i++ ){
        double elapsedTimeAverage = 0;
        if(m_timerCount_renderer < m_lengthTimerArray_renderer){
            for(int iTime=0;iTime<m_timerCount_renderer;iTime++){
                elapsedTimeAverage += m_array_time_renderer[i][iTime];
            }
            elapsedTimeAverage /= m_timerCount_renderer;
        } else {
             for(int iTime=0;iTime<m_lengthTimerArray_renderer;iTime++){
                 elapsedTimeAverage += m_array_time_renderer[i][iTime];
             }
             elapsedTimeAverage /= m_lengthTimerArray_renderer;
        }
        std::cout << "ElapsedTimeAverage : draw() : renderer[" << i << "] : " << elapsedTimeAverage << std::endl;

    }
#endif

    // m_timerCount_renderer++;

    if (frame_index % m_lengthTimerArray == 0 && m_eye == 1) {
        for (int iRenderer = 0; iRenderer < size + 1; iRenderer++) {
            std::cerr << "[TIME_MEASURE][" << (frame_index / m_lengthTimerArray) << "]";
            if (iRenderer == 0) {
                std::cerr << " class CameraImageRendererEx CameraImage";
            } else if (iRenderer - 1 < size) {
                kvs::IDManager::IDPair id = m_scene->IDManager()->id(iRenderer - 1);
                kvs::ObjectBase *object = m_scene->objectManager()->object(id.first);
                kvs::RendererBase *renderer = m_scene->rendererManager()->renderer(id.second);
                std::cerr << " " << typeid(*renderer).name() << " " << object->name();
            } else {
                continue;
            }

            // std::cerr << " eye=" << m_eye;
            // std::cerr << std::endl;
            // for(int iRepetition=0;iRepetition<m_repetition_level;iRepetition++){
            //     std::cerr << " repetition=" << iRepetition;
            //     for (int iTimer = 0; iTimer < m_lengthTimerArray; iTimer++) {
            //         std::cerr << " " << m_array_time[iRenderer][m_eye][iRepetition][iTimer];
            //     }
            //     std::cerr << std::endl;
            // }

            for (int iTimer = 0; iTimer < m_lengthTimerArray; iTimer++) {
                double temp_time = 0.0;
                for (int iEye = 0; iEye < 2; iEye++) {
                    for (int iRepetition = 0; iRepetition < m_repetition_level; iRepetition++) {
                        temp_time += m_array_time[iRenderer][iEye][iRepetition][iTimer];
                    }
                }
                std::cerr << " " << temp_time;
            }
            std::cerr << std::endl;
        }

        /*
    std::cerr << "[TIME_MEASURE][" << (frame_index / m_lengthTimerArray_renderer) << "] CameraImageRendererEx CameraImage";
    for (int iTimer = 0; iTimer < m_lengthTimerArray_renderer/2; iTimer++) {
        std::cerr << " " << (m_array_time_renderer[0][iTimer*2] +  m_array_time_renderer[0][iTimer*2+1]);
    }
    std::cerr << std::endl;

    for (int i = 0; i < size; i++) {
        kvs::IDManager::IDPair id = m_scene->IDManager()->id( i );
        kvs::ObjectBase* object = m_scene->objectManager()->object( id.first );
        kvs::RendererBase* renderer = m_scene->rendererManager()->renderer( id.second );
        std::cerr << "[TIME_MEASURE][" << (frame_index / m_lengthTimerArray_renderer) << "] " << typeid(*renderer).name() << " " << object->name();
        for (int iTimer = 0; iTimer < m_lengthTimerArray_renderer/2; iTimer++) {
            std::cerr << " " << (m_array_time_renderer[i+1][iTimer*2]+m_array_time_renderer[i+1][iTimer*2+1]);
        }
        std::cerr << std::endl;
    }
    */
    }

#endif // ENABLE_TIME_MEASURE

    MAKE_AND_START_TIMER(A_6_1or2_2_12);

    MAKE_AND_START_TIMER(A_6_1or2_2_12_1);
#ifdef DISP_MODE_VR
    // set viewport
    if (m_eye == 0) {
        glViewport(0, 0, bufferWidth / 2, bufferHeight);
    } else {
        glViewport(bufferWidth / 2, 0, bufferWidth / 2, bufferHeight);
    }
#endif
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_12_1, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_12_1)  "));

    MAKE_AND_START_TIMER(A_6_1or2_2_12_2);
    m_ensemble_buffer.draw();
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_12_2, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_12_2)  "));

    MAKE_AND_START_TIMER(A_6_1or2_2_12_3);
#ifdef CALL_GL_FINISH
    kvs::OpenGL::Finish();
#endif
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_12_3, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_12_3)  kvs::OpenGL::Finish()"));

    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_12, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_12)  "));
}

/*===========================================================================*/
/**
 *  @brief  Check whether the window is created and initialize the parameters.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::check_window_created() {
    const bool window_created = m_window_width == 0 && m_window_height == 0;
    if (window_created) {
        MAKE_AND_START_TIMER(A_6_1or2_2_3_1);

        const size_t width = m_scene->camera()->windowWidth();
        const size_t height = m_scene->camera()->windowHeight();
        const float dpr = m_scene->camera()->devicePixelRatio();
        const size_t framebuffer_width = static_cast<size_t>(width * dpr);
        const size_t framebuffer_height = static_cast<size_t>(height * dpr);

        m_window_width = width;
        m_window_height = height;
        m_ensemble_buffer.create(framebuffer_width, framebuffer_height);
        m_ensemble_buffer.clear();

        m_object_xform = this->object_xform();
        m_camera_position = m_scene->camera()->position();
        m_light_position = m_scene->light()->position();

        STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_3_1, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_3_1)  "));

        MAKE_AND_START_TIMER(A_6_1or2_2_3_2);
        this->engines_create();
        STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_3_2, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_3_2)  "));
    } else {
        // If ENABLE_TIME_MEASURE is defined, this block will become empty and may be deleted by compiler optimization.
        RECORD_TIMER(std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_3_1)  "), 0.0);
        RECORD_TIMER(std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_3_2)  "), 0.0);
    }
}

/*===========================================================================*/
/**
 *  @brief  Check whether the window is resized and update the parameters.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::check_window_resized() {
    MAKE_AND_START_TIMER(A_6_1or2_2_4_1);
    const size_t width = m_scene->camera()->windowWidth();
    const size_t height = m_scene->camera()->windowHeight();
    const bool window_resized = m_window_width != width || m_window_height != height;
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_4_1, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_4_1)  "));
    if (window_resized) {
        MAKE_AND_START_TIMER(A_6_1or2_2_4_2);
        const float dpr = m_scene->camera()->devicePixelRatio();
        const size_t framebuffer_width = static_cast<size_t>(width * dpr);
        const size_t framebuffer_height = static_cast<size_t>(height * dpr);

        m_window_width = width;
        m_window_height = height;
        m_ensemble_buffer.release();
        m_ensemble_buffer.create(framebuffer_width, framebuffer_height);
        m_ensemble_buffer.clear();
        STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_4_2, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_4_2)  "));

        MAKE_AND_START_TIMER(A_6_1or2_2_4_3);
        this->engines_update();
        STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_4_3, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_4_3)  "));
    } else {
        // If ENABLE_TIME_MEASURE is defined, this block will become empty and may be deleted by compiler optimization.
        RECORD_TIMER(std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_4_2)  "), 0.0);
        RECORD_TIMER(std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_4_3)  "), 0.0);
    }
}

/*===========================================================================*/
/**
 *  @brief  Check whether the object is changed and recreated the engine.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::check_object_changed() {
    typedef kvs::StochasticRendererBase Renderer;

    const size_t size = m_scene->IDManager()->size();
    for (size_t i = 0; i < size; i++) {
        MAKE_AND_START_TIMER(A_6_1or2_2_5_x_1);
        kvs::IDManager::IDPair id = m_scene->IDManager()->id(i);
        kvs::ObjectBase *object = m_scene->objectManager()->object(id.first);
        kvs::RendererBase *renderer = m_scene->rendererManager()->renderer(id.second);
        STOP_AND_RECORD_TIMER_WITH_ID(
            A_6_1or2_2_5_x_1,
            std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_5_").append(std::to_string(i + 1)).append("_1)  ").append(object->name()));

        if (Renderer *stochastic_renderer = Renderer::DownCast(renderer)) {
            const bool object_changed = stochastic_renderer->engine().object() != object;
            if (object_changed || repetition_level_changed) {
                MAKE_AND_START_TIMER(A_6_1or2_2_5_x_2);
                m_ensemble_buffer.clear();

                if (stochastic_renderer->engine().object()) {
                    stochastic_renderer->engine().release();
                }
                stochastic_renderer->engine().setDepthTexture(m_ensemble_buffer.currentDepthTexture());
                stochastic_renderer->engine().setShader(&stochastic_renderer->shader());
                stochastic_renderer->engine().setRepetitionLevel(m_repetition_level);
                stochastic_renderer->engine().setEnabledShading(stochastic_renderer->isEnabledShading());

                kvs::OpenGL::PushMatrix();
                STOP_AND_RECORD_TIMER_WITH_ID(
                    A_6_1or2_2_5_x_2, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_5_").append(std::to_string(i + 1)).append("_2)  "));

                MAKE_AND_START_TIMER(A_6_1or2_2_5_x_3);
                stochastic_renderer->engine().create(object, m_scene->camera(), m_scene->light());
                STOP_AND_RECORD_TIMER_WITH_ID(
                    A_6_1or2_2_5_x_3, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_5_").append(std::to_string(i + 1)).append("_3)  "));

                MAKE_AND_START_TIMER(A_6_1or2_2_5_x_4);
                kvs::OpenGL::PopMatrix();
                STOP_AND_RECORD_TIMER_WITH_ID(
                    A_6_1or2_2_5_x_4, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_5_").append(std::to_string(i + 1)).append("_4)  "));
            } else {
                // If ENABLE_TIME_MEASURE is defined, this block will become empty and may be deleted by compiler optimization.
                RECORD_TIMER(std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_5_").append(std::to_string(i + 1).append("_2)  ")), 0.0);
                RECORD_TIMER(std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_5_").append(std::to_string(i + 1).append("_3)  ")), 0.0);
                RECORD_TIMER(std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_5_").append(std::to_string(i + 1).append("_4)  ")), 0.0);
            }
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Returns the xform matrix of the active object.
 *  @return xform matrix
 */
/*===========================================================================*/
kvs::Mat4 StochasticRenderingCompositor::object_xform() {
    return m_scene->objectManager()->hasActiveObject() ? m_scene->objectManager()->activeObject()->xform().toMatrix()
                                                       : m_scene->objectManager()->xform().toMatrix();
}

/*===========================================================================*/
/**
 *  @brief  Calls the create method of each engine.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::engines_create() {
    typedef kvs::StochasticRendererBase Renderer;

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cerr << "StochasticRenderingCompositor::engines_create() start." << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    const size_t size = m_scene->IDManager()->size();
    for (size_t i = 0; i < size; i++) {
        MAKE_AND_START_TIMER(A_6_1or2_2_3_2_x_1);
        kvs::IDManager::IDPair id = m_scene->IDManager()->id(i);
        kvs::ObjectBase *object = m_scene->objectManager()->object(id.first);
        kvs::RendererBase *renderer = m_scene->rendererManager()->renderer(id.second);
        STOP_AND_RECORD_TIMER_WITH_ID(
            A_6_1or2_2_3_2_x_1, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_3_2_").append(std::to_string(i + 1)).append("_1)  "));

        if (Renderer *stochastic_renderer = Renderer::DownCast(renderer)) {
            MAKE_AND_START_TIMER(A_6_1or2_2_3_2_x_2);
#ifdef DEBUG_TOUCH_CONTROLLER
            const std::type_info &renderer_info = typeid(*stochastic_renderer);
            const std::type_info &object_info = typeid(*object);
            const kvs::GeometryObjectBase *geo_obj = static_cast<kvs::GeometryObjectBase *>(object);
            std::cerr << "StochasticRenderingCompositor::engines_create() : [" << i << "/" << size << "] renderer=" << renderer_info.name()
                      << " / object=" << object_info.name() << "(" << (object == nullptr ? "NULL" : object->name()) << ") "
                      << (object->isShown() ? "show" : "hide") << "/ nverts=" << geo_obj->numberOfVertices() << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

            stochastic_renderer->engine().setDepthTexture(m_ensemble_buffer.currentDepthTexture());
            stochastic_renderer->engine().setShader(&stochastic_renderer->shader());
            stochastic_renderer->engine().setRepetitionLevel(m_repetition_level);
            stochastic_renderer->engine().setEnabledShading(stochastic_renderer->isEnabledShading());

            kvs::OpenGL::PushMatrix();
            if (!ObjectNameUtil::isToIgnoreModelingMatrix(object->name())) {
                m_scene->updateGLModelingMatrix(object);
            } else {
                kvs::OpenGL::LoadIdentity();
            }

#ifdef DEBUG_TOUCH_CONTROLLER
            std::cerr << "StochasticRenderingCompositor::engines_create() : [" << i << "/" << size << "] call Engine::create()" << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER
            STOP_AND_RECORD_TIMER_WITH_ID(
                A_6_1or2_2_3_2_x_2, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_3_2_").append(std::to_string(i + 1)).append("_2)  "));

            MAKE_AND_START_TIMER(A_6_1or2_2_3_2_x_3);
            stochastic_renderer->engine().create(object, m_scene->camera(), m_scene->light());
            STOP_AND_RECORD_TIMER_WITH_ID(
                A_6_1or2_2_3_2_x_3, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_3_2_").append(std::to_string(i + 1)).append("_3)  "));

#ifdef MR_MODE
#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
#ifdef DEBUG_TOUCH_CONTROLLER
            std::cerr << "StochasticRenderingCompositor::engines_create() : [" << i << "+/" << size
                      << "] m_camera_image_renderer=" << m_camera_image_renderer << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER
#endif // ENABLE_FEATURE_DRAW_CAMERA_IMAGE
#endif // MR_MODE

            MAKE_AND_START_TIMER(A_6_1or2_2_3_2_x_5);
            kvs::OpenGL::PopMatrix();
            STOP_AND_RECORD_TIMER_WITH_ID(
                A_6_1or2_2_3_2_x_5, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_3_2_").append(std::to_string(i + 1)).append("_5)  "));
        }
    }

#ifdef MR_MODE
#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
    MAKE_AND_START_TIMER(A_6_1or2_2_3_2_x_4);
    if (m_camera_image_renderer) {
        m_camera_image_renderer->engine().setDepthTexture(m_ensemble_buffer.currentDepthTexture());
        m_camera_image_renderer->engine().setShader(&m_camera_image_renderer->shader());
        m_camera_image_renderer->engine().setRepetitionLevel(m_repetition_level);
        m_camera_image_renderer->engine().setEnabledShading(false);
        m_camera_image_renderer->engine().create(nullptr, m_scene->camera(), m_scene->light());
    }
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_3_2_x_4, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_3_2_4)  "));
#endif
#endif // MR_MODE

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cerr << "StochasticRenderingCompositor::engines_create() end." << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER
}

/*===========================================================================*/
/**
 *  @brief  Calls the update method of each engine.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::engines_update() {
    typedef kvs::StochasticRendererBase Renderer;
//    // #ifdef MR_MODE
//    //     bool is_stereo_camera_done = false;
//    // #endif // MR_MODE
//    bool is_stereo_camera_done = false;

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cerr << "StochasticRenderingCompositor::engines_update() start." << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER

    kvs::Camera *camera = m_scene->camera();
    kvs::Light *light = m_scene->light();

#ifdef MR_MODE
#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
    MAKE_AND_START_TIMER(A_6_1or2_2_4_3_x_4);
    //            if(m_camera_image_renderer && !is_stereo_camera_done){
    if (m_camera_image_renderer) {
        static_cast<CameraImageRenderer::Engine &>(m_camera_image_renderer->engine()).update(nullptr, camera, light);
        //                is_stereo_camera_done = true;
    }
    STOP_AND_RECORD_TIMER_WITH_ID(
        A_6_1or2_2_4_3_x_4, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_4_3_").append(std::to_string(0)).append("_4)  "));
#endif // ENABLE_FEATURE_DRAW_CAMERA_IMAGE
#endif // MR_MODE

    const size_t size = m_scene->IDManager()->size();
    for (size_t i = 0; i < size; i++) {
        MAKE_AND_START_TIMER(A_6_1or2_2_4_3_x_1);
        kvs::IDManager::IDPair id = m_scene->IDManager()->id(i);
        kvs::ObjectBase *object = m_scene->objectManager()->object(id.first);
        kvs::RendererBase *renderer = m_scene->rendererManager()->renderer(id.second);
        STOP_AND_RECORD_TIMER_WITH_ID(
            A_6_1or2_2_4_3_x_1, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_4_3_").append(std::to_string(i + 1)).append("_1)  "));

        if (Renderer *stochastic_renderer = Renderer::DownCast(renderer)) {
            MAKE_AND_START_TIMER(A_6_1or2_2_4_3_x_2);
            kvs::OpenGL::PushMatrix();
            if (!ObjectNameUtil::isToIgnoreModelingMatrix(object->name())) {
                m_scene->updateGLModelingMatrix(object);
            } else {
                kvs::OpenGL::LoadIdentity();
            }
            STOP_AND_RECORD_TIMER_WITH_ID(
                A_6_1or2_2_4_3_x_2, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_4_3_").append(std::to_string(i + 1)).append("_2)  "));

            MAKE_AND_START_TIMER(A_6_1or2_2_4_3_x_3);
            stochastic_renderer->engine().update(object, camera, light);
            STOP_AND_RECORD_TIMER_WITH_ID(
                A_6_1or2_2_4_3_x_3, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_4_3_").append(std::to_string(i + 1)).append("_3)  "));

            MAKE_AND_START_TIMER(A_6_1or2_2_4_3_x_5);
            kvs::OpenGL::PopMatrix();
            STOP_AND_RECORD_TIMER_WITH_ID(
                A_6_1or2_2_4_3_x_5, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_4_3_").append(std::to_string(i + 1)).append("_5)  "));
        }
    }

#ifdef DEBUG_TOUCH_CONTROLLER
    std::cerr << "StochasticRenderingCompositor::engines_update() end." << std::endl;
#endif // DEBUG_TOUCH_CONTROLLER
}

/*===========================================================================*/
/**
 *  @brief  Calls the setup method of each engine.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::engines_setup() {
    MAKE_AND_START_TIMER(A_6_1or2_2_7_0);
    MAKE_AND_START_TIMER(A_6_1or2_2_7_0_1);

    typedef kvs::StochasticRendererBase Renderer;
    // #ifdef MR_MODE
    //     bool is_stereo_camera_done = false;
    // #endif // MR_MODE

    // #ifdef DEBUG_TOUCH_CONTROLLER
    //    std::cerr << "StochasticRenderingCompositor::engines_setup() start." << std::endl;
    // #endif // DEBUG_TOUCH_CONTROLLER

    kvs::Camera *camera = m_scene->camera();
    kvs::Light *light = m_scene->light();
    const bool reset_count = !m_enable_refinement;
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_7_0_1, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_7_0_1)  "));

#ifdef MR_MODE
#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
    if (m_enable_camera_image &&
        //        m_camera_image_renderer &&
        //            !is_stereo_camera_done){
        m_camera_image_renderer) {
        MAKE_AND_START_TIMER(A_6_1or2_2_7_0_2);
        if (reset_count)
            static_cast<CameraImageRenderer::Engine &>(m_camera_image_renderer->engine()).resetRepetitions();
        static_cast<CameraImageRenderer::Engine &>(m_camera_image_renderer->engine()).setup(nullptr, camera, light);
        STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_7_0_2, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_7_0_2)  "));

        //        is_stereo_camera_done = true;
    }
#endif // ENABLE_FEATURE_DRAW_CAMERA_IMAGE
#endif // MR_MODE

    const size_t size = m_scene->IDManager()->size();
    STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_7_0, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_7_0)  "));
    for (size_t i = 0; i < size; i++) {
        MAKE_AND_START_TIMER(A_6_1or2_2_7_x);
        MAKE_AND_START_TIMER(A_6_1or2_2_7_x_1);
        kvs::IDManager::IDPair id = m_scene->IDManager()->id(i);
        kvs::ObjectBase *object = m_scene->objectManager()->object(id.first);
        kvs::RendererBase *renderer = m_scene->rendererManager()->renderer(id.second);
        STOP_AND_RECORD_TIMER_WITH_ID(
            A_6_1or2_2_7_x_1, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_7_").append(std::to_string(i + 1)).append("_1)  "));

        if (Renderer *stochastic_renderer = Renderer::DownCast(renderer)) {
            MAKE_AND_START_TIMER(A_6_1or2_2_7_x_2);
            kvs::OpenGL::PushMatrix();
            if (!ObjectNameUtil::isToIgnoreModelingMatrix(object->name())) {
                m_scene->updateGLModelingMatrix(object);
            } else {
                // kvs::OpenGL::LoadIdentity();
            }
            if (reset_count)
                stochastic_renderer->engine().resetRepetitions();
            STOP_AND_RECORD_TIMER_WITH_ID(
                A_6_1or2_2_7_x_2, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_7_").append(std::to_string(i + 1)).append("_2)  "));

            MAKE_AND_START_TIMER(A_6_1or2_2_7_x_3);
            stochastic_renderer->engine().setup(object, camera, light);
            STOP_AND_RECORD_TIMER_WITH_ID(
                A_6_1or2_2_7_x_3, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_7_").append(std::to_string(i + 1)).append("_3)  "));

            MAKE_AND_START_TIMER(A_6_1or2_2_7_x_4);
            kvs::OpenGL::PopMatrix();
            STOP_AND_RECORD_TIMER_WITH_ID(
                A_6_1or2_2_7_x_4, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_7_").append(std::to_string(i + 1)).append("_4)  "));
        }

#ifdef ENABLE_TIME_MEASURE
        std::string renderer_classname = std::string(typeid(*renderer).name());
        std::replace(renderer_classname.begin(), renderer_classname.end(), ' ', '-');
        const std::string &object_name = object->name();
        STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_7_x, std::string("(A_6_")
                                                          .append(std::to_string(m_eye + 1))
                                                          .append("_2_7_")
                                                          .append(std::to_string(i + 1))
                                                          .append(") ")
                                                          .append(renderer_classname)
                                                          .append(" ")
                                                          .append(object_name));
#endif
    }

    // #ifdef DEBUG_TOUCH_CONTROLLER
    //     std::cerr << "StochasticRenderingCompositor::engines_setup() end." << std::endl;
    // #endif // DEBUG_TOUCH_CONTROLLER
}

/*===========================================================================*/
/**
 *  @brief  Calls the draw method of each engine.
 */
/*===========================================================================*/
void StochasticRenderingCompositor::engines_draw() {
    typedef kvs::StochasticRendererBase Renderer;
    // #ifdef MR_MODE
    //     bool is_stereo_camera_done = false;
    // #endif // MR_MODE
    kvs::Camera *camera = m_scene->camera();
    kvs::Light *light = m_scene->light();
    const size_t size = m_scene->IDManager()->size();

#ifdef DISP_MODE_VR
    KVS_GL_CALL(glClearColor(m_background_color[0], m_background_color[1], m_background_color[2], m_background_color[3]));
#endif // DISP_MODE_VR

#ifdef MR_MODE
#ifdef ENABLE_FEATURE_DRAW_CAMERA_IMAGE
    if (m_enable_camera_image &&
        //       m_camera_image_renderer &&
        //       !is_stereo_camera_done){
        m_camera_image_renderer) {
        // {
        //     std::cerr << "[" << i << "+/" << size << "] obj=[(none)], renderer=CameraImageRenderer" << std::endl;
        // }

        MAKE_AND_START_TIMER(A_6_1or2_2_10_0);

        // draw camera image -> initialize depth to m_clear_depth
        KVS_GL_CALL( glClearDepth(m_clear_depth));
        KVS_GL_CALL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        KVS_GL_CALL(glDepthFunc(GL_ALWAYS));

        static_cast<CameraImageRenderer::Engine &>(m_camera_image_renderer->engine()).drawStereoCameraImage(m_eye);

        STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_10_0, std::string("(A_6_").append(std::to_string(m_eye + 1)).append("_2_10_0)  drawStereoCameraImage"));

//        is_stereo_camera_done = true;

#ifdef ENABLE_TIME_MEASURE
#if 0
        {
            m_array_time_camera[m_timerCount_camera % m_lengthTimerArray_camera] = m_timer_camera.msec();
            m_timerCount_camera++;

            double elapsedTimeAverage = 0;
            if(m_timerCount_camera < m_lengthTimerArray_camera){
                for(int iTime=0;iTime<m_timerCount_camera;iTime++){
                    elapsedTimeAverage += m_array_time_camera[iTime];
                }
                elapsedTimeAverage /= m_timerCount_camera;
            } else {
                for(int iTime=0;iTime<m_lengthTimerArray_camera;iTime++){
                    elapsedTimeAverage += m_array_time_camera[iTime];
                }
                elapsedTimeAverage /= m_lengthTimerArray_camera;
            }

            std::cout << "ElapsedTimeAverage : engines_draw:CameraImageRenderer() : " << elapsedTimeAverage << std::endl;
        }
#endif
        //        m_array_time_camera[m_timerCount_camera % m_lengthTimerArray_camera] += m_timer_camera.msec();
        //        m_timerCount_camera++;

#endif // ENABLE_TIME_MEASURE
    } else {
        // not draw camera image -> initialize depth to 1.0
        KVS_GL_CALL(glClearDepth(1.0));
        KVS_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }
#endif // ENABLE_FEATURE_DRAW_CAMERA_IMAGE
#elif defined(VR_MODE) // MR_MODE
    KVS_GL_CALL(glClearDepth(1.0));
    KVS_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
#endif // MR_MODE

#ifdef DEBUG_TOUCH_CONTROLLER
    static bool first_draw = true;
    if (first_draw) {
        for (size_t i = 0; i < size; i++) {
            kvs::IDManager::IDPair id = m_scene->IDManager()->id(i);
            kvs::ObjectBase *object = m_scene->objectManager()->object(id.first);
            kvs::RendererBase *renderer = m_scene->rendererManager()->renderer(id.second);

            std::cerr << "[" << i << "/" << size << "] object=" << object->name() << (object->isShown() ? " (show)" : " (hide)") << std::endl;
        }
        first_draw = false;
    }
#endif // DEBUG_TOUCH_CONTROLLER

#ifdef DISP_MODE_VR
    KVS_GL_CALL(glDepthFunc(GL_LESS));
#endif // DISP_MODE_VR

    for (size_t i = 0; i < size; i++) {
        MAKE_AND_START_TIMER(A_6_1or2_2_10_x);

        kvs::IDManager::IDPair id = m_scene->IDManager()->id(i);
        kvs::ObjectBase *object = m_scene->objectManager()->object(id.first);
        kvs::RendererBase *renderer = m_scene->rendererManager()->renderer(id.second);

        // {
        //     std::cerr << "[" << i << "/" << size << "] obj=[" << object->name() << "], renderer=" << typeid(*renderer).name() << std::endl;
        // }

        //std::cerr << "##--------## " << object->name() << " isShown()=" << object->isShown() << std::endl;
        //std::cerr << "##        ## object->minObjectCoord()  =" << object->minObjectCoord() << std::endl;
        //std::cerr << "##        ## object->maxObjectCoord()  =" << object->maxObjectCoord() << std::endl;
        //std::cerr << "##        ## object->minExternalCoord()=" << object->minExternalCoord() << std::endl;
        //std::cerr << "##        ## object->maxExternalCoord()=" << object->maxExternalCoord() << std::endl;
        if (Renderer *stochastic_renderer = Renderer::DownCast(renderer)) {
            if (object->isShown()) {
                kvs::OpenGL::PushMatrix();
                if (!ObjectNameUtil::isToIgnoreModelingMatrix(object->name())) {
                    m_scene->updateGLModelingMatrix(object);
                } else {
                    // kvs::OpenGL::LoadIdentity();
                }
                stochastic_renderer->engine().draw(object, camera, light);
                stochastic_renderer->engine().countRepetitions();
                kvs::OpenGL::PopMatrix();
            }
        } // if (stochastic_renderer)

#ifdef ENABLE_TIME_MEASURE
        std::string renderer_classname = std::string(typeid(*renderer).name());
        std::replace(renderer_classname.begin(), renderer_classname.end(), ' ', '-');
        const std::string &object_name = object->name();
        STOP_AND_RECORD_TIMER_WITH_ID(A_6_1or2_2_10_x, std::string("(A_6_")
                                                           .append(std::to_string(m_eye + 1))
                                                           .append("_2_10_")
                                                           .append(std::to_string(i + 1))
                                                           .append(") ")
                                                           .append(renderer_classname)
                                                           .append(" ")
                                                           .append(object_name));
#endif
    } // for(i, size)
}

} // end of namespace pbvr
} // end of namespace jaea
} // end of namespace kvs
