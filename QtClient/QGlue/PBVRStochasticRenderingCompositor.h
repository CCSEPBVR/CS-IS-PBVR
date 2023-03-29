/*****************************************************************************/
/**
 *  @file   PBVRStochasticRenderingCompositor.h
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
#pragma once
#include <kvs/Timer>
#include <kvs/PaintEventListener>
#include <kvs/Matrix44>
#include <kvs/Vector3>
#include <kvs/Deprecated>
#include <kvs/EnsembleAverageBuffer>
//#include <kvs/Scene>
#include "Scene.h"

//class kvs::Scene;
namespace kvs {
namespace oculus {
namespace jaea {
class Scene;
}
}
}

/*===========================================================================*/
/**
 *  @brief  Stochastic rendering compositor class.
 */
/*===========================================================================*/
class PBVRStochasticRenderingCompositor : public kvs::PaintEventListener
{
private:
    kvs::Timer m_timer;
    kvs::oculus::jaea::Scene* m_scene; ///< pointer to the scene
    size_t m_window_width; ///< window width
    size_t m_window_height; ///< window height
    size_t m_repetition_level; ///< repetition level
    size_t m_coarse_level; ///< repetition level for the coarse rendering (LOD)
    bool m_enable_lod; ///< flag for LOD rendering
    bool m_enable_refinement; ///< flag for progressive refinement rendering
    kvs::Mat4 m_object_xform; ///< object xform matrix used for LOD control
    kvs::Vec3 m_light_position; ///< light position used for LOD control
    kvs::Vec3 m_camera_position; ///< camera position used for LOD control
    kvs::EnsembleAverageBuffer m_ensemble_buffer; ///< ensemble averaging buffer

    int m_eye_index;

public:
    PBVRStochasticRenderingCompositor( kvs::oculus::jaea::Scene* scene );
    const kvs::Timer& timer() const { return m_timer; }
    size_t repetitionLevel() const { return m_repetition_level; }
    bool isEnabledLODControl() const { return m_enable_lod; }
    bool isEnabledRefinement() const { return m_enable_refinement; }
    void setRepetitionLevel( const size_t repetition_level ) {
        m_repetition_level = repetition_level;
        std::cerr << "set repetition level to " << m_repetition_level <<std::endl;
    }
    void setEnabledLODControl( const bool enable ) { m_enable_lod = enable; }
    void setEnabledRefinement( const bool enable ) { m_enable_refinement = enable; }
    void enableLODControl() { this->setEnabledLODControl( true ); }
    void enableRefinement() { this->setEnabledRefinement( true ); }
    void disableLODControl() { this->setEnabledLODControl( false ); }
    void disableRefinement() { this->setEnabledRefinement( false ); }
    void update();

    void setEyeIndex (int eye) { m_eye_index = eye;}

private:
    PBVRStochasticRenderingCompositor();
    void draw();
    void check_window_created();
    void check_window_resized();
    void check_object_changed();
    kvs::Mat4 object_xform();
    void engines_create();
    void engines_update();
    void engines_setup();
    void engines_draw();

public:
    KVS_DEPRECATED( bool isEnabledShading() const ) { return false; /* do not use */ }
    KVS_DEPRECATED( void setEnabledShading( const bool /* enable */ ) ) { /* not do anything */}
    KVS_DEPRECATED( void enableShading() ) { /* not do anything */ }
    KVS_DEPRECATED( void disableShading() ) { /* not do anything */ }
};
