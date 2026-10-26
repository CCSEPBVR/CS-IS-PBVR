/*****************************************************************************/
/**
 *  @file   StochasticRendererBase.h
 *  @author Jun Nishimura, Naohisa Sakamoto
 */
/*****************************************************************************/
#pragma once
#include <kvs/RendererBase>
#include <kvs/Shader>
#include <kvs/Matrix44>
#include <kvs/Module>
#include <kvs/Deprecated>
#include "EnsembleAverageBuffer.h"
#include "StochasticRenderingEngine.h"


namespace kvs
{

class ObjectBase;
class Camera;
class Light;

/*===========================================================================*/
/**
 *  @brief  Stochastic renderer base class.
 */
/*===========================================================================*/
class StochasticRendererBase : public kvs::RendererBase
{
    kvsModule( kvs::StochasticRendererBase, Renderer );

    friend class StochasticRenderingCompositor;

private:
    std::size_t m_window_width = 0; ///< window width
    std::size_t m_window_height = 0; ///< window height
    float m_device_pixel_ratio = 1.0f; ///< device pixel ratio
    std::size_t m_repetition_level = 1; ///< repetition level
    std::size_t m_coarse_level = 1; ///< repetition level for the coarse rendering (LOD)
    bool m_enable_lod = false; ///< flag for LOD rendering
    bool m_enable_refinement = false; ///< flag for progressive refinement rendering
    kvs::Mat4 m_modelview{}; ///< modelview matrix used for LOD control
    kvs::Vec3 m_light_position{}; ///< light position used for LOD control
    kvs::EnsembleAverageBuffer m_ensemble_buffer{}; ///< ensemble averaging buffer
    kvs::Shader::ShadingModel* m_shader = nullptr; ///< shading method
    kvs::StochasticRenderingEngine* m_engine = nullptr; ///< rendering engine

public:
    StochasticRendererBase( kvs::StochasticRenderingEngine* engine );
    virtual ~StochasticRendererBase();

    virtual void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );
    void release();
    std::size_t windowWidth() const { return m_window_width; }
    std::size_t windowHeight() const { return m_window_height; }
    std::size_t framebufferWidth() const { return static_cast<size_t>( m_window_width * m_device_pixel_ratio ); }
    std::size_t framebufferHeight() const { return static_cast<size_t>( m_window_height * m_device_pixel_ratio ); }
    float devicePixelRatio() const { return m_device_pixel_ratio; }
    std::size_t repetitionLevel() const { return m_repetition_level; }

    void setWindowSize( const std::size_t width, const std::size_t height ) { m_window_width = width; m_window_height = height; }
    void setDevicePixelRatio( const float dpr ) { m_device_pixel_ratio = dpr; }
    void setRepetitionLevel( const std::size_t repetition_level ) { m_repetition_level = repetition_level; }

    void setLODControlEnabled( const bool enable = true ) { m_enable_lod = enable; }
    bool isLODControlEnabled() const { return m_enable_lod; }
    void enableLODControl() { this->setLODControlEnabled( true ); }
    void disableLODControl() { this->setLODControlEnabled( false ); }

    void setRefinementEnabled( const bool enable = true ) { m_enable_refinement = enable; }
    bool isRefinementEnabled() const { return m_enable_refinement; }
    void enableRefinement() { this->setRefinementEnabled( true ); }
    void disableRefinement() { this->setRefinementEnabled( false ); }

    void setTwoSideLightingEnabled( const bool enable = true ) { m_shader->two_side_lighting = enable; }
    bool isTwoSideLightingEnabled() const { return m_shader->two_side_lighting; }
    void enableTwoSideLighting() { this->setTwoSideLightingEnabled( true ); }
    void disableTwoSideLighting() { this->setTwoSideLightingEnabled( false ); }

    const kvs::Shader::ShadingModel& shader() const { return *m_shader; }
    const kvs::StochasticRenderingEngine& engine() const { return *m_engine; }
    template <typename ShadingType>
    void setShader( const ShadingType shader );

protected:
    const kvs::Mat4& modelView() const { return m_modelview; }
    const kvs::Vec3& lightPosition() const { return m_light_position; }
    void setModelView( const kvs::Mat4& modelview ) { m_modelview = modelview; }
    void setLightPosition( const kvs::Vec3& position ) { m_light_position = position; }

    kvs::Shader::ShadingModel& shader() { return *m_shader; }
    kvs::StochasticRenderingEngine& engine() { return *m_engine; }
    kvs::EnsembleAverageBuffer& ensembleBuffer() { return m_ensemble_buffer; }

    bool isWindowCreated() { return m_window_width == 0 && m_window_height == 0; }
    bool isWindowResized( std::size_t w, std::size_t h ) { return m_window_width != w || m_window_height != h; }
    bool isObjectChanged( const kvs::ObjectBase* o ) { return m_engine->object() != o; }

    std::size_t controllledRepetitions( const kvs::Mat4& modelview, const kvs::Vec3& light_position );
    void createEnsembleBuffer( const std::size_t frame_width, const std::size_t frame_height );
    void createEngine( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );
    void setupEngine( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

public:
    KVS_DEPRECATED( void setEnabledLODControl( const bool enable ) ) { this->setLODControlEnabled( enable ); }
    KVS_DEPRECATED( bool isEnabledLODControl() const ) { return this->isLODControlEnabled(); }
    KVS_DEPRECATED( void setEnabledRefinement( const bool enable ) ) { this->setRefinementEnabled( enable ); }
    KVS_DEPRECATED( bool isEnabledRefinement() const ) { return this->isRefinementEnabled(); }
};

template <typename ShadingType>
inline void StochasticRendererBase::setShader( const ShadingType shader )
{
    if ( m_shader ) { delete m_shader; }
    m_shader = new ShadingType( shader );
    m_engine->setShader( m_shader );
}

} // end of namespace kvs
