#include "PBRProxy.h"
#include <assert.h>
using namespace  kvs::visclient;

/*
// from KVS sample
kvs::Mat4 init_mvmat (
            +1.935484e-01,  +0.000000e+00,  +0.000000e+00,  -3.000000e+00,
            +0.000000e+00,  +1.935484e-01,  +0.000000e+00,  -3.000000e+00,
            +0.000000e+00,  +0.000000e+00,  +1.935484e-01,  -1.500000e+01,
            +0.000000e+00,  +0.000000e+00,  +0.000000e+00,  +1.000000e+00
            );
// from OPBVR test code
kvs::Mat4 init_pmat (
            +1.200088e+00,  +0.000000e+00,  -1.470005e-01,  +0.000000e+00,
            +0.000000e+00,  +1.008082e+00,  -1.125376e-01,  +0.000000e+00,
            +0.000000e+00,  +0.000000e+00,  -1.000200e+00,  -2.000400e-01,
            +0.000000e+00,  +0.000000e+00,  -1.000000e+00,  +0.000000e+00
            );
//// from OPBVR test code
//kvs::Vec4 init_viewport (+0.000000e+00, +0.000000e+00, +1.328000e+03, +1.584000e+03);
*/

// from CS-PBVR 1.17.3
kvs::Mat4 init_mvmat (
        +6.386375e-01  +0.000000e+00  +0.000000e+00  -4.489836e+00
        +0.000000e+00  +6.386375e-01  +0.000000e+00  -2.218173e+00
        +0.000000e+00  +0.000000e+00  +6.386375e-01  -9.463013e+00
        +0.000000e+00  +0.000000e+00  +0.000000e+00  +1.000000e+00
        );
kvs::Mat4 init_pmat (
        +2.414213e+00  +0.000000e+00  +0.000000e+00  +0.000000e+00
        +0.000000e+00  +2.414213e+00  +0.000000e+00  +0.000000e+00
        +0.000000e+00  +0.000000e+00  -1.001001e+00  -2.001000e+00
        +0.000000e+00  +0.000000e+00  -1.000000e+00  +0.000000e+00
            );
kvs::Vec4 init_viewport ( +0.000000e+00, +0.000000e+00, +6.200000e+02, +6.200000e+02);

/**
 * @brief PBRProxy  - constructor
 * @param mode_selection , Selects the mode of operation {CPU|GPU}
 * @param other ,  other ojbect to replicate state from.
 */
PBRProxy::PBRProxy(   PBR_MODE mode_selection, PBRProxy* other):
    GPU_BASE(),
//    GPU_BASE(init_mvmat, init_pmat, init_viewport),
    selected_mode(mode_selection)
{
    if(other!=NULL){
        this->setSubpixelLevel(other->subpixelLevel());
        this->setShadingString(other->pbvr_shader_params.c_str());
    }
#ifdef USE_ZED_CAMERA_RENDERING
    this->setRepetitionLevel (9);
    this->enableLODControl();
#endif
}

/**
 * @brief recreateImageBuffer, recreates the Image Buffer  for CPU renderer.
 */
void PBRProxy::recreateImageBuffer()
{
//    for(int nindent = 0; nindent < nstart; nindent++) std::cout << "    ";
//    std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , start" << std::endl;
//    nstart++;

    if(selected_mode==GPU){
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.05.28
        this->deleteParticleBuffer();
        this->createParticleBuffer(    CPU_BASE::windowWidth(), CPU_BASE::windowHeight(),  m_subpixel_level );
    }

//    for(int nindent = 0; nindent < nstart-1; nindent++) std::cout << "    ";
//    std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , end" << std::endl;
//    nstart--;
}

bool PBRProxy::isEnabledShading(){
    return selected_mode==GPU?GPU_BASE::isEnabledShading():CPU_BASE::isEnabledShading();
}
/**
 * @brief setRepetitionLevel, set renderer repetition level.
 * @param rep_level
 */
void PBRProxy::setRepetitionLevel(size_t rep_level){
    if (selected_mode==GPU)
        GPU_BASE::setRepetitionLevel(rep_level);
    else
        CPU_BASE::setSubpixelLevel( (size_t) sqrt(rep_level));
}
/**
 * @brief setSubpixelLevel, set subpixel level
 * @param sp_level
 */
void PBRProxy::setSubpixelLevel(size_t sp_level){
    if (selected_mode==GPU){
        GPU_BASE::setRepetitionLevel(sp_level*sp_level);
    }
    else{
        CPU_BASE::setSubpixelLevel( sp_level);
    }
}

/**
 * @brief setShader, set shader
 * @param shader
 */
template <typename ShadingType>
void PBRProxy::setShader( const ShadingType shader ){
    selected_mode==GPU?GPU_BASE::setShader(shader):CPU_BASE::setShader(shader);
}

/**
 * @brief disableShading, disable shading
 */
void PBRProxy::disableShading(){
    selected_mode==GPU?GPU_BASE::disableShading():CPU_BASE::disableShading();
}

/**
 * @brief enableShading, enable shading
 */
void PBRProxy::enableShading(){
    selected_mode==GPU?GPU_BASE::enableShading():CPU_BASE::enableShading();
}

/**
 * @brief repetitionLevel, return the repetitionLevel
 * @return
 */
size_t PBRProxy::repetitionLevel()
{
    return (selected_mode==GPU) ? GPU_BASE::repetitionLevel():CPU_BASE::subpixelLevel() *  CPU_BASE::subpixelLevel();

}
/**
 * @brief subpixelLevel, return the sub pixel level
 * @return
 */
size_t PBRProxy::subpixelLevel()
{
    return selected_mode==GPU?(size_t) sqrt(GPU_BASE::repetitionLevel()):CPU_BASE::subpixelLevel();
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
    this->_use_gpu=use_gpu;
    kvs::visclient::PBR_MODE m = _use_gpu?kvs::visclient::GPU:kvs::visclient::CPU;
    pbr=new kvs::visclient::PBRProxy(m );
}

/**
 * @brief use_gpu, switch to GPU mode - repcreates renderer [ need replacement in scene]
 */
void SwitchablePBRProxy::use_gpu(){

//    for(int nindent = 0; nindent < nstart; nindent++) std::cout << "    ";
//    std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , start" << std::endl;
//    nstart++;

    _use_gpu=true;
    recreatePBR();

//    for(int nindent = 0; nindent < nstart-1; nindent++) std::cout << "    ";
//    std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , end" << std::endl;
//    nstart--;
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

//    for(int nindent = 0; nindent < nstart; nindent++) std::cout << "    ";
//    std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , start" << std::endl;
//    nstart++;

    kvs::visclient::PBR_MODE m = _use_gpu?kvs::visclient::GPU:kvs::visclient::CPU;
    auto new_pbr=new kvs::visclient::PBRProxy(m,pbr );
    if (pbr){
        delete pbr;
        pbr=new_pbr;
    }

//    for(int nindent = 0; nindent < nstart-1; nindent++) std::cout << "    ";
//    std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , end" << std::endl;
//    nstart--;
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
    // Kawamura mod 2021/11/16
    // below line comment out. It deletes and new renderer, and caused random texture id bug.
    //recreatePBR();
    pbr->setRepetitionLevel(rep_level);
    assert (pbr->repetitionLevel() == pbr->subpixelLevel()* pbr->subpixelLevel());
}

/**
 * @brief pbr_pointer, get the PBR pointer cast to kvs::RenderBase
 * @return
 */
kvs::RendererBase* SwitchablePBRProxy::pbr_pointer(){
    assert (pbr->repetitionLevel() == pbr->subpixelLevel()* pbr->subpixelLevel());

    if (_use_gpu)
        return   (kvs::glsl::ParticleBasedRenderer*)pbr;
    else
        return (kvs::ParticleBasedRenderer*)pbr;
}
