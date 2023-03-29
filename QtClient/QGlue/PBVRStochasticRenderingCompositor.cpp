/*****************************************************************************/
/**
 *  @file   PBVRStochasticRenderingCompositor.cpp
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
#include "PBVRStochasticRenderingCompositor.h"
#include <kvs/Assert>
#include <kvs/OpenGL>
#include <kvs/PaintEvent>
#include <kvs/EventHandler>
#include <kvs/ScreenBase>
//#include <kvs/Scene>
#include <kvs/Camera>
#include <kvs/Light>
#include <kvs/Background>
//#include <kvs/ObjectManager>
#include <kvs/RendererManager>
#include <kvs/IDManager>
#include <kvs/StochasticRendererBase>
#include <kvs/ParticleBasedRenderer>

#include "Scene.h"
#include "PBVRObjectManager.h"
#include "objnameutil.h"
#include "TimeMeasure.h"
#include "TimeMeasureUtility.h"

/*===========================================================================*/
/**
 *  @brief  Constructs a new StochasticRenderingCompositor class.
 *  @param  scene [in] pointer to the scene
 */
/*===========================================================================*/
PBVRStochasticRenderingCompositor::PBVRStochasticRenderingCompositor( kvs::oculus::jaea::Scene* scene ):
    m_scene( scene ),
    m_window_width( 0 ),
    m_window_height( 0 ),
    m_repetition_level( 1 ),
    m_coarse_level( 1 ),
    m_enable_lod( false ),
    m_enable_refinement( false ),
    m_eye_index(-1)
{
}

/*===========================================================================*/
/**
 *  @brief  Updates the scene.
 */
/*===========================================================================*/
void PBVRStochasticRenderingCompositor::update()
{
    KVS_ASSERT( m_scene );

    //kvs::OpenGL::WithPushedMatrix p( GL_MODELVIEW );
    //p.loadIdentity();
    {
//        m_scene->updateGLProjectionMatrix();
//        m_scene->updateGLViewingMatrix();
//        m_scene->updateGLLightParameters();
//        m_scene->background()->apply();

        if ( m_scene->objectManager()->hasObject() )
        {
            this->draw();
        }
        else
        {
            m_scene->updateGLModelingMatrix();
        }
    }
}

static size_t prev_repetition_level = 0;
static bool repLevelChanged = false;

/*===========================================================================*/
/**
 *  @brief  Draws the objects with stochastic renderers.
 */
/*===========================================================================*/
void PBVRStochasticRenderingCompositor::draw()
{
    bool pbrproxy_found = false;
    const size_t size = m_scene->IDManager()->size();
    for ( size_t i = 0; i < size; i++ )
    {
        kvs::IDManager::IDPair id = m_scene->IDManager()->id( i );
        kvs::RendererBase* renderer = m_scene->rendererManager()->renderer( id.second );
        std::string renderer_classname = std::string (typeid(*renderer).name());
        if (renderer_classname == "class kvs::visclient::PBRProxy") {
            pbrproxy_found = true;
            break;
        }
    }

    if (!pbrproxy_found) {
        return;
    }

    repLevelChanged = false;
    if (prev_repetition_level != m_repetition_level) {

        std::cerr << "repetition level changed from " << prev_repetition_level << " to " << m_repetition_level << std::endl;

        prev_repetition_level = m_repetition_level;
        repLevelChanged = true;
    }


#ifdef ENABLE_TIME_MEASURE
    MAKE_AND_START_TIMER(A_1or2_2_1);
#endif // ENABLE_TIME_MEASURE

    m_timer.start();
    kvs::OpenGL::WithPushedAttrib p( GL_ALL_ATTRIB_BITS );

#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_1,m_eye_index==0 ? "(A_1_2_1)   m_timer.start()" : "(A_2_2_1)   m_timer.start()");
    MAKE_AND_START_TIMER(A_1or2_2_2);
#endif // ENABLE_TIME_MEASURE

    if (repLevelChanged) {
        std::cerr << "  call check_window_created()" << std::endl;
    }

    this->check_window_created();


#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_2,m_eye_index==0 ? "(A_1_2_2)   check_window_created()" : "(A_2_2_2)   check_window_created()");
    MAKE_AND_START_TIMER(A_1or2_2_3);
#endif // ENABLE_TIME_MEASURE

    if (repLevelChanged) {
        std::cerr << "  call check_window_resized()" << std::endl;
    }

    this->check_window_resized();

#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_3,m_eye_index==0 ? "(A_1_2_3)   check_window_resized()" : "(A_2_2_3)   check_window_resized()");
    MAKE_AND_START_TIMER(A_1or2_2_4);
#endif // ENABLE_TIME_MEASURE

    if (repLevelChanged) {
        std::cerr << "  call check_object_created()" << std::endl;
    }

    this->check_object_changed();

#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_4,m_eye_index==0 ? "(A_1_2_4)   check_object_changed()" : "(A_2_2_4)   check_object_changed()");
    MAKE_AND_START_TIMER(A_1or2_2_5);
#endif // ENABLE_TIME_MEASURE


    // LOD control.
    size_t repetitions = m_repetition_level;
    kvs::Vec3 camera_position = m_scene->camera()->position();
    kvs::Vec3 light_position = m_scene->light()->position();
    kvs::Mat4 object_xform = this->object_xform();
    if ( m_camera_position != camera_position ||
         m_light_position != light_position ||
         m_object_xform != object_xform )
    {
        if ( m_enable_lod )
        {
            repetitions = m_coarse_level;
        }
        m_camera_position = camera_position;
        m_light_position = light_position;
        m_object_xform = object_xform;
        m_ensemble_buffer.clear();
    }

#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_5,m_eye_index==0 ? "(A_1_2_5)   m_ensemble_buffer.clear()" : "(A_2_2_5)   m_ensemble_buffer.clear()");
    MAKE_AND_START_TIMER(A_1or2_2_6);
#endif // ENABLE_TIME_MEASURE

    if (repLevelChanged) {
        std::cerr << "  call engines_setup()" << std::endl;
    }

    // Setup engine.
    this->engines_setup();

#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_6,m_eye_index==0 ? "(A_1_2_6)   engines_setup()" : "(A_2_2_6)   engines_setup()");
    MAKE_AND_START_TIMER(A_1or2_2_7);
#endif // ENABLE_TIME_MEASURE


    // Ensemble rendering.
    const bool reset_count = !m_enable_refinement;
    if ( reset_count ) m_ensemble_buffer.clear();
    for ( size_t i = 0; i < repetitions; i++ )
    {


        if (repLevelChanged) {
            std::cerr << "  call engines_draw() [" << i << "]" << std::endl;
        }

        m_ensemble_buffer.bind();
        this->engines_draw();
        m_ensemble_buffer.unbind();
        m_ensemble_buffer.add();
    }

#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_7,m_eye_index==0 ? "(A_1_2_7)   m_ensemble_buffer.bind()" : "(A_2_2_7)   m_ensemble_buffer.bind()");
    MAKE_AND_START_TIMER(A_1or2_2_8);
#endif // ENABLE_TIME_MEASURE

    m_ensemble_buffer.draw();

#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_8,m_eye_index==0 ? "(A_1_2_8)   m_ensemble_buffer.draw()" : "(A_2_2_8)   m_ensemble_buffer.draw()");
    MAKE_AND_START_TIMER(A_1or2_2_9);
#endif // ENABLE_TIME_MEASURE

#ifdef CALL_GL_FINISH
    kvs::OpenGL::Finish();
#endif
    m_timer.stop();

#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_9,m_eye_index==0 ? "(A_1_2_9)   kvs::OpenGL::Finish()" : "(A_2_2_9)   kvs::OpenGL::Finish()");
#endif // ENABLE_TIME_MEASURE

}

/*===========================================================================*/
/**
 *  @brief  Check whether the window is created and initialize the parameters.
 */
/*===========================================================================*/
void PBVRStochasticRenderingCompositor::check_window_created()
{
    const bool window_created = m_window_width == 0 && m_window_height == 0;
    if ( window_created )
    {
        const size_t width = m_scene->camera()->windowWidth();
        const size_t height = m_scene->camera()->windowHeight();
        m_window_width = width;
        m_window_height = height;

        const float dpr = m_scene->camera()->devicePixelRatio();
        const size_t framebuffer_width = static_cast<size_t>( width * dpr );
        const size_t framebuffer_height = static_cast<size_t>( height * dpr );
        m_ensemble_buffer.create( framebuffer_width, framebuffer_height );
        m_ensemble_buffer.clear();

        m_object_xform = this->object_xform();
        m_camera_position = m_scene->camera()->position();
        m_light_position = m_scene->light()->position();
        this->engines_create();
    }
}

/*===========================================================================*/
/**
 *  @brief  Check whether the window is resized and update the parameters.
 */
/*===========================================================================*/
void PBVRStochasticRenderingCompositor::check_window_resized()
{
    const size_t width = m_scene->camera()->windowWidth();
    const size_t height = m_scene->camera()->windowHeight();
    const bool window_resized = m_window_width != width || m_window_height != height;
    if ( window_resized )
    {
        m_window_width = width;
        m_window_height = height;

        const float dpr = m_scene->camera()->devicePixelRatio();
        const size_t framebuffer_width = static_cast<size_t>( width * dpr );
        const size_t framebuffer_height = static_cast<size_t>( height * dpr );
        m_ensemble_buffer.release();
        m_ensemble_buffer.create( framebuffer_width, framebuffer_height );
        m_ensemble_buffer.clear();

        this->engines_update();
    }
}

/*===========================================================================*/
/**
 *  @brief  Check whether the object is changed and recreated the engine.
 */
/*===========================================================================*/
void PBVRStochasticRenderingCompositor::check_object_changed()
{
    typedef kvs::StochasticRendererBase Renderer;

    if (repLevelChanged) {
        std::cerr << "    check_object_changed : repLevelChanged." << std::endl;
    }

    const size_t size = m_scene->IDManager()->size();
    for ( size_t i = 0; i < size; i++ )
    {

#ifdef ENABLE_TIME_MEASURE
        MAKE_AND_START_TIMER(A_1or2_2_4_i);
        MAKE_AND_START_TIMER(A_1or2_2_4_i_1);
#endif // ENABLE_TIME_MEASURE

        kvs::IDManager::IDPair id = m_scene->IDManager()->id( i );
        kvs::ObjectBase* object = m_scene->objectManager()->object( id.first );
        kvs::RendererBase* renderer = m_scene->rendererManager()->renderer( id.second );

#ifdef ENABLE_TIME_MEASURE
        std::string renderer_name = std::string(typeid(*renderer).name());
        const std::string& obj_name = object->name();
        std::string eye_id = std::to_string(m_eye_index+1);
        std::string renderer_index = std::to_string(i+1);
        STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_4_i_1, std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_1)   "));
#endif // ENABLE_TIME_MEASURE

        if ( Renderer* stochastic_renderer = Renderer::DownCast( renderer ) )
        {
            const bool object_changed = stochastic_renderer->engine().object() != object;
            if ( object_changed || repLevelChanged )
//            if ( object_changed)
            {
#ifdef ENABLE_TIME_MEASURE
        MAKE_AND_START_TIMER(A_1or2_2_4_i_2);
#endif // ENABLE_TIME_MEASURE

                m_ensemble_buffer.clear();

#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_4_i_2, std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_2)   "));
        MAKE_AND_START_TIMER(A_1or2_2_4_i_3);
#endif // ENABLE_TIME_MEASURE

                if ( stochastic_renderer->engine().object() ) {
                    if (repLevelChanged) {
                        std::cerr << "      renderer[" << i << "] (" << typeid(*stochastic_renderer).name() << ") call release()" << std::endl;
                    }

                    stochastic_renderer->engine().release();
                }

#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_4_i_3, std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_3)   "));
        MAKE_AND_START_TIMER(A_1or2_2_4_i_4);
#endif // ENABLE_TIME_MEASURE

                if (repLevelChanged) {
                    std::cerr << "      renderer[" << i << "] (" << typeid(*stochastic_renderer).name() << ") rep level=" << stochastic_renderer->engine().repetitionLevel() << std::endl;
                }

                stochastic_renderer->engine().setDepthTexture( m_ensemble_buffer.currentDepthTexture() );
                stochastic_renderer->engine().setShader( &stochastic_renderer->shader() );
                stochastic_renderer->engine().setRepetitionLevel( m_repetition_level );
                stochastic_renderer->engine().setEnabledShading( stochastic_renderer->isEnabledShading() );

                if (repLevelChanged) {
                    std::cerr << "      renderer[" << i << "] (" << typeid(*stochastic_renderer).name() << ") rep level=" << stochastic_renderer->engine().repetitionLevel() << std::endl;
                }

#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_4_i_4, std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_4)   "));
        MAKE_AND_START_TIMER(A_1or2_2_4_i_5);
#endif // ENABLE_TIME_MEASURE


                kvs::OpenGL::PushMatrix();

#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_4_i_5, std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_5)   "));
        MAKE_AND_START_TIMER(A_1or2_2_4_i_6);
#endif // ENABLE_TIME_MEASURE

                if (repLevelChanged) {
                    std::cerr << "      renderer[" << i << "] (" << typeid(*stochastic_renderer).name() << ")->engine().create()" << std::endl;
                }

                stochastic_renderer->engine().create( object, m_scene->camera(), m_scene->light() );

#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_4_i_6, std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_6)   "));
        MAKE_AND_START_TIMER(A_1or2_2_4_i_7);
#endif // ENABLE_TIME_MEASURE

                kvs::OpenGL::PopMatrix();
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_4_i_7, std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_7)   "));
#endif // ENABLE_TIME_MEASURE

            }  // if(object_changed)

#ifdef ENABLE_TIME_MEASURE
            else { // (object_changed)
                RECORD_TIMER(std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_2)   "), 0.0);
                RECORD_TIMER(std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_3)   "), 0.0);
                RECORD_TIMER(std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_4)   "), 0.0);
                RECORD_TIMER(std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_5)   "), 0.0);
                RECORD_TIMER(std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_6)   "), 0.0);
                RECORD_TIMER(std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append("_7)   "), 0.0);
            }
#endif // ENABLE_TIME_MEASURE

        }  // if (stochastic_renderer)

#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2_4_i, std::string("(A_").append(eye_id).append("_2_4_").append(renderer_index).append(") ").append(renderer_name).append(" ").append(obj_name));
#endif // ENABLE_TIME_MEASURE

    } // for (i)
}

/*===========================================================================*/
/**
 *  @brief  Returns the xform matrix of the active object.
 *  @return xform matrix
 */
/*===========================================================================*/
kvs::Mat4 PBVRStochasticRenderingCompositor::object_xform()
{
    return m_scene->objectManager()->hasActiveObject() ?
        m_scene->objectManager()->activeObject()->xform().toMatrix() :
        m_scene->objectManager()->xform().toMatrix();
}

/*===========================================================================*/
/**
 *  @brief  Calls the create method of each engine.
 */
/*===========================================================================*/
void PBVRStochasticRenderingCompositor::engines_create()
{
    typedef kvs::StochasticRendererBase Renderer;

    const size_t size = m_scene->IDManager()->size();
    for ( size_t i = 0; i < size; i++ )
    {
        kvs::IDManager::IDPair id = m_scene->IDManager()->id( i );
        kvs::ObjectBase* object = m_scene->objectManager()->object( id.first );
        kvs::RendererBase* renderer = m_scene->rendererManager()->renderer( id.second );
        if ( Renderer* stochastic_renderer = Renderer::DownCast( renderer ) )
        {
            stochastic_renderer->engine().setDepthTexture( m_ensemble_buffer.currentDepthTexture() );
            stochastic_renderer->engine().setShader( &stochastic_renderer->shader() );
            stochastic_renderer->engine().setRepetitionLevel( m_repetition_level );
            stochastic_renderer->engine().setEnabledShading( stochastic_renderer->isEnabledShading() );

            kvs::OpenGL::PushMatrix();
            if (!kvs::oculus::jaea::pbvr::ObjectNameUtil::isToIgnoreModelingMatrix(object->name())) {
                m_scene->updateGLModelingMatrix( object );
            }
            stochastic_renderer->engine().create( object, m_scene->camera(), m_scene->light() );
            kvs::OpenGL::PopMatrix();
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Calls the update method of each engine.
 */
/*===========================================================================*/
void PBVRStochasticRenderingCompositor::engines_update()
{
    typedef kvs::StochasticRendererBase Renderer;

    kvs::Camera* camera = m_scene->camera();
    kvs::Light* light = m_scene->light();

    const size_t size = m_scene->IDManager()->size();
    for ( size_t i = 0; i < size; i++ )
    {
        kvs::IDManager::IDPair id = m_scene->IDManager()->id( i );
        kvs::ObjectBase* object = m_scene->objectManager()->object( id.first );
        kvs::RendererBase* renderer = m_scene->rendererManager()->renderer( id.second );
        if ( Renderer* stochastic_renderer = Renderer::DownCast( renderer ) )
        {
            kvs::OpenGL::PushMatrix();
            if (!kvs::oculus::jaea::pbvr::ObjectNameUtil::isToIgnoreModelingMatrix(object->name())) {
                m_scene->updateGLModelingMatrix( object );
            }
            stochastic_renderer->engine().update( object, camera, light );
            kvs::OpenGL::PopMatrix();
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Calls the setup method of each engine.
 */
/*===========================================================================*/
void PBVRStochasticRenderingCompositor::engines_setup()
{
    typedef kvs::StochasticRendererBase Renderer;

    kvs::Camera* camera = m_scene->camera();
    kvs::Light* light = m_scene->light();
    const bool reset_count = !m_enable_refinement;

    const size_t size = m_scene->IDManager()->size();
    for ( size_t i = 0; i < size; i++ )
    {
        kvs::IDManager::IDPair id = m_scene->IDManager()->id( i );
        kvs::ObjectBase* object = m_scene->objectManager()->object( id.first );
        kvs::RendererBase* renderer = m_scene->rendererManager()->renderer( id.second );
        if ( Renderer* stochastic_renderer = Renderer::DownCast( renderer ) )
        {
            kvs::OpenGL::PushMatrix();
            if (!kvs::oculus::jaea::pbvr::ObjectNameUtil::isToIgnoreModelingMatrix(object->name())) {
                m_scene->updateGLModelingMatrix( object );
            }
            if ( reset_count ) stochastic_renderer->engine().resetRepetitions();
            stochastic_renderer->engine().setup( object, camera, light );
            kvs::OpenGL::PopMatrix();
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Calls the draw method of each engine.
 */
/*===========================================================================*/
void PBVRStochasticRenderingCompositor::engines_draw()
{
    typedef kvs::StochasticRendererBase Renderer;

    kvs::Camera* camera = m_scene->camera();
    kvs::Light* light = m_scene->light();

    const size_t size = m_scene->IDManager()->size();
    for ( size_t i = 0; i < size; i++ )
    {
        kvs::IDManager::IDPair id = m_scene->IDManager()->id( i );
        kvs::ObjectBase* object = m_scene->objectManager()->object( id.first );
        kvs::RendererBase* renderer = m_scene->rendererManager()->renderer( id.second );
        if ( Renderer* stochastic_renderer = Renderer::DownCast( renderer ) )
        {

#ifdef DEBUG_COMPOSITOR
            {
                kvs::Vec3 objcrd_min = object->minObjectCoord();
                kvs::Vec3 objcrd_max = object->maxObjectCoord();
                kvs::Vec3 extcrd_min = object->minExternalCoord();
                kvs::Vec3 extcrd_max = object->maxExternalCoord();
                std::cerr << "Compositor::engines_draw() " <<
                             (object->isShown()? "o" : "x") << " [" << i << "/" << size <<
                             "] objcrd=min" << objcrd_min << ", max" << objcrd_max <<
                             " / extcrd=min" << extcrd_min << ", max" << extcrd_max <<
                             std::endl;
                if ( kvs::PointObject* pointObject = dynamic_cast<kvs::PointObject*>( object ) )
                {
                    std::cerr << "Compositor::engines_draw() " <<
                                 (object->isShown()? "o" : "x") << " [" << i << "/" << size <<
                                 "] pointObject->numberOfVertices()=" << pointObject->numberOfVertices() <<
                                 std::endl;
                }
            }
#endif // DEBUG_COMPOSITOR

//#ifdef DEBUG_COMPOSITOR
//            std::cerr << "Compositor::engines_draw() : [" << i << "/" << size << "] obj=" << object << " -> stochastic renderer " << std::endl;
//#endif // DEBUG_COMPOSITOR


            if ( object->isShown() )
            {

                if (repLevelChanged) {
                    std::cerr << "    call renderer[" << i << "](" << typeid(*stochastic_renderer).raw_name() << ")->engine->draw()" << std::endl;
                }


                kvs::OpenGL::PushMatrix();
                if (!kvs::oculus::jaea::pbvr::ObjectNameUtil::isToIgnoreModelingMatrix(object->name())) {
                    m_scene->updateGLModelingMatrix( object );
                }
                stochastic_renderer->engine().draw( object, camera, light );
                stochastic_renderer->engine().countRepetitions();
                kvs::OpenGL::PopMatrix();
            }
        }
//#ifdef DEBUG_COMPOSITOR
//        else {
//            std::cerr << "Compositor::engines_draw() : [" << i << "/" << size << "] obj=" << object << " -> non-stochastic renderer " << std::endl;
//        }
//#endif // DEBUG_COMPOSITOR
    }
}
