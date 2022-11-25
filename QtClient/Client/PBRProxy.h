#ifndef PBVR_PBR_PROXY_H_INCLUDE
#define PBVR_PBR_PROXY_H_INCLUDE

#include <kvs/ParticleBasedRenderer>
#include <map>

namespace kvs
{
namespace visclient
{

#ifdef CPU_MODE
typedef  kvs::ParticleBasedRenderer CPU_BASE;
enum PBR_MODE { CPU};
#endif
#ifdef GPU_MODE
typedef  kvs::glsl::ParticleBasedRenderer GPU_BASE;
enum PBR_MODE { GPU};
#endif
//typedef  kvs::glsl::ParticleBasedRenderer GPU_BASE;
//typedef  kvs::ParticleBasedRenderer CPU_BASE;
//enum PBR_MODE { CPU,GPU};

/**
 * @brief The PBRProxy class, this acts a a proxy (in lack of a better term) class for the
 *          kvs::ParticleBasedRenderer, and kvs::glsl::ParticleBasedRenderer classes.
 *
 *        Constructor takes a PBR_MODE to decide the mode of operation, and any subsequent calls
 *        to underlying classes will be performed on the base class that corrsponds to the
 *        mode of operation.
 */
#ifdef CPU_MODE
class PBRProxy : public CPU_BASE
#endif
#ifdef GPU_MODE
class PBRProxy : public GPU_BASE
#endif
//class PBRProxy : public CPU_BASE, public GPU_BASE
{
public:
#ifdef CPU_MODE
    PBR_MODE selected_mode = CPU;
#endif
#ifdef GPU_MODE
    PBR_MODE selected_mode = GPU;
#endif
    kvs::Shader::Lambert L;
    kvs::Shader::BlinnPhong B;
    kvs::Shader::Phong P;
    std::map<char, kvs::Shader::Base*> shader_map ={ {'L',&L}, {'B',&B},  {'P',&P}};
    std::string pbvr_shader_params;
    PBRProxy(   PBR_MODE mode_selection, PBRProxy* other=NULL);
    void recreateImageBuffer();
    bool isEnabledShading();
    void setRepetitionLevel(size_t rep_level);
    void setSubpixelLevel(size_t sp_level);
    template <typename ShadingType>
    void setShader( const ShadingType shader );
    void disableShading();
    void enableShading();
    size_t repetitionLevel();
    size_t subpixelLevel();
    void setShadingString(  const char* pbvr_shader_string);
};

}
}

/**
 * @brief  The SwitchablePBRProxy class, this class provides an interface to be able to switch between the two states of
 *        the PBRProxy class.
 *         Additionally, it automatically recreates the PBRProxy object when the RenderRepetitionLevel,
 *        or GPU|CPU mode changes.
 */
class SwitchablePBRProxy{
private:
    kvs::visclient::PBRProxy* pbr; // ParticleBasedRenderer (proxy)
    bool shading_enabled;
#ifdef CPU_MODE
    bool _use_gpu=false;
#endif
#ifdef GPU_MODE
    bool _use_gpu=true;
#endif
public:
#ifdef CPU_MODE
    SwitchablePBRProxy(bool use_gpu=false);
#endif
#ifdef GPU_MODE
    SwitchablePBRProxy(bool use_gpu=true);
#endif
    void use_gpu();
    void use_cpu();
    void enableShading();
    void disableShading();
    void setShadingString(  const char* pbvr_shading_string);
    void recreatePBR();
    size_t getRepetitionLevel();
    void setRepetitionLevel( const size_t rep_level);
    kvs::RendererBase* pbr_pointer();
#ifdef GPU_MODE
    void updateModelView();
#endif
};
#endif    // PBVR_PBR_PROXY_H_INCLUDE
