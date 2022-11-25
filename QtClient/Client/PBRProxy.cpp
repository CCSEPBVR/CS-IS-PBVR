#include "PBRProxy.h"
#include <assert.h>
using namespace  kvs::visclient;


/**
 * @brief PBRProxy  - constructor
 * @param mode_selection , Selects the mode of operation {CPU|GPU}
 * @param other ,  other ojbect to replicate state from.
 */
PBRProxy::PBRProxy(   PBR_MODE mode_selection, PBRProxy* other):
    selected_mode(mode_selection)
{
    if(other!=NULL){
        this->setSubpixelLevel(other->subpixelLevel());
        this->setShadingString(other->pbvr_shader_params.c_str());
    }
}

/**
 * @brief recreateImageBuffer, recreates the Image Buffer  for CPU renderer.
 */
void PBRProxy::recreateImageBuffer()
{
#ifdef CPU_MODE
#endif
#ifdef GPU_MODE
    //必要かどうか不明
//    this->deleteParticleBuffer();
//    this->createParticleBuffer(    CPU_BASE::windowWidth(), CPU_BASE::windowHeight(),  m_subpixel_level );
#endif
//    if(selected_mode==GPU){
//        //KVS2.7.0
//        //MOD BY)T.Osaki 2020.05.28
//        this->deleteParticleBuffer();
//        this->createParticleBuffer(    CPU_BASE::windowWidth(), CPU_BASE::windowHeight(),  m_subpixel_level );
//    }
}

bool PBRProxy::isEnabledShading(){
#ifdef CPU_MODE
    return CPU_BASE::isEnabledShading();
#endif
#ifdef GPU_MODE
    return GPU_BASE::isEnabledShading();
#endif
}
/**
 * @brief setRepetitionLevel, set renderer repetition level.
 * @param rep_level
 */
void PBRProxy::setRepetitionLevel(size_t rep_level){
#ifdef CPU_MODE
    CPU_BASE::setSubpixelLevel( (size_t) sqrt(rep_level));
#endif
#ifdef GPU_MODE
    GPU_BASE::setRepetitionLevel(rep_level);
#endif
}
/**
 * @brief setSubpixelLevel, set subpixel level
 * @param sp_level
 */
void PBRProxy::setSubpixelLevel(size_t sp_level){
#ifdef CPU_MODE
    CPU_BASE::setSubpixelLevel( sp_level);
#endif
#ifdef GPU_MODE
    GPU_BASE::setRepetitionLevel(sp_level*sp_level);
#endif
}

/**
 * @brief setShader, set shader
 * @param shader
 */
template <typename ShadingType>
void PBRProxy::setShader( const ShadingType shader ){
#ifdef CPU_MODE
    CPU_BASE::setShader(shader);
#endif
#ifdef GPU_MODE
    GPU_BASE::setShader(shader);
#endif
}

/**
 * @brief disableShading, disable shading
 */
void PBRProxy::disableShading(){
#ifdef CPU_MODE
    CPU_BASE::disableShading();
#endif
#ifdef GPU_MODE
    GPU_BASE::disableShading();
#endif
}

/**
 * @brief enableShading, enable shading
 */
void PBRProxy::enableShading(){
#ifdef CPU_MODE
    CPU_BASE::enableShading();
#endif
#ifdef GPU_MODE
    GPU_BASE::enableShading();
#endif
}

/**
 * @brief repetitionLevel, return the repetitionLevel
 * @return
 */
size_t PBRProxy::repetitionLevel()
{
#ifdef CPU_MODE
    return CPU_BASE::subpixelLevel() *  CPU_BASE::subpixelLevel();
#endif
#ifdef GPU_MODE
    return GPU_BASE::repetitionLevel();
#endif
}
/**
 * @brief subpixelLevel, return the sub pixel level
 * @return
 */
size_t PBRProxy::subpixelLevel()
{
#ifdef CPU_MODE
    return CPU_BASE::subpixelLevel();
#endif
#ifdef GPU_MODE
    return sqrt(GPU_BASE::repetitionLevel());
#endif
}
/**
 * @brief setShadingString , sets PBVR type shading string in format L|P|B[Ka,Kd][,Ks,S]
 * @param pbvr_shading_string , L for Lambert, P for Phong, B for BlinnPhong
 *                              Ka for ambient, Kd for diffuse,  (Available for L|P|B)
 *                              Ks For Specular, S for Shinyness (Available for P|B)
 */
void PBRProxy::setShadingString(  const char* pbvr_shader_string)
{
    const char shaderType=pbvr_shader_string[0];
    kvs::Shader::Base* b = shader_map[shaderType];
    if (b){
        // sscanf will only scan as many strings as matches the format, i.e if only two are available, then only two are scanned.
        // All shaders have all 4 members Ka,Kd,Ks,S, but  Lambert will only use Ka,Kd.
        int nscan=sscanf( &pbvr_shader_string[1],",%f,%f,%f,%f", &b->Ka, &b->Kd, &b->Ks, &b->S );
        if(shaderType=='L') setShader(L);
        if(shaderType=='B') setShader(B);
        if(shaderType=='P') setShader(P);
        enableShading();
        this->pbvr_shader_params=pbvr_shader_string;
    }
    else{
        disableShading();
    }
}


SwitchablePBRProxy::SwitchablePBRProxy(bool use_gpu){
#ifdef CPU_MODE
    this->_use_gpu=false;
    kvs::visclient::PBR_MODE m = kvs::visclient::CPU;
#endif
#ifdef GPU_MODE
    this->_use_gpu=true;
    kvs::visclient::PBR_MODE m = kvs::visclient::GPU;
#endif
//    this->_use_gpu=use_gpu;
//    kvs::visclient::PBR_MODE m = _use_gpu?kvs::visclient::GPU:kvs::visclient::CPU;
    pbr=new kvs::visclient::PBRProxy(m );
}

/**
 * @brief use_gpu, switch to GPU mode - repcreates renderer [ need replacement in scene]
 */
void SwitchablePBRProxy::use_gpu(){
    _use_gpu=true;
    recreatePBR();
}
/**
 * @brief use_cpu, switch to CPU mode - repcreates renderer [ need replacement in scene]
 */
void SwitchablePBRProxy::use_cpu(){
    _use_gpu=false;
    recreatePBR();
}

/**
 * @brief enableShading enables shading
 */
void SwitchablePBRProxy::enableShading(){
    pbr->enableShading();
}

/**
 * @brief disableShading disables shading
 */
void SwitchablePBRProxy::disableShading(){
    pbr->disableShading();
}

/**
 * @brief setShadingString , sets PBVR type shading string in format L|P|B[Ka,Kd][,Ks,S]
 * @param pbvr_shading_string , L for Lambert, P for Phong, B for BlinnPhong
 *                              Ka for ambient, Kd for diffuse,  (Available for L|P|B)
 *                              Ks For Specular, S for Shinyness (Available for P|B)
 */
void SwitchablePBRProxy::setShadingString(  const char* pbvr_shading_string)
{
    pbr->setShadingString(pbvr_shading_string);

}
/**
 * @brief recreatePBR, recreates the PBR instance
 */
void SwitchablePBRProxy::recreatePBR(){
#ifdef CPU_MODE
    kvs::visclient::PBR_MODE m = kvs::visclient::CPU;
#endif
#ifdef GPU_MODE
    kvs::visclient::PBR_MODE m = kvs::visclient::GPU;
#endif
//    kvs::visclient::PBR_MODE m = _use_gpu?kvs::visclient::GPU:kvs::visclient::CPU;
    auto new_pbr=new kvs::visclient::PBRProxy(m,pbr );
    if (pbr){
        delete pbr;
        pbr=new_pbr;
    }
}
/**
 * @brief getRepetitionLevel , get the repetition level from PBR
 * @return
 */
size_t SwitchablePBRProxy::getRepetitionLevel()
{
    assert (pbr->repetitionLevel() == pbr->subpixelLevel()* pbr->subpixelLevel());
    return pbr->repetitionLevel();
}
/**
 * @brief setRepetitionLevel, set the repetition level of PBR
 * @param rep_level
 */
void SwitchablePBRProxy::setRepetitionLevel( const size_t rep_level){
//    recreatePBR();
    pbr->setRepetitionLevel(rep_level);
    assert (pbr->repetitionLevel() == pbr->subpixelLevel()* pbr->subpixelLevel());
}

/**
 * @brief pbr_pointer, get the PBR pointer cast to kvs::RenderBase
 * @return
 */
kvs::RendererBase* SwitchablePBRProxy::pbr_pointer(){
    assert (pbr->repetitionLevel() == pbr->subpixelLevel()* pbr->subpixelLevel());
#ifdef CPU_MODE
    return (kvs::ParticleBasedRenderer*)pbr;
#endif
#ifdef GPU_MODE
    return   (kvs::glsl::ParticleBasedRenderer*)pbr;
#endif
//    if (_use_gpu)
//        return   (kvs::glsl::ParticleBasedRenderer*)pbr;
//    else
//        return (kvs::ParticleBasedRenderer*)pbr;
}

/**
 * @brief updateModelView, Update m_initial_modelview.
 */
#ifdef GPU_MODE
void SwitchablePBRProxy::updateModelView(){
    pbr->updateModelViewMatrixTest();
}
#endif
