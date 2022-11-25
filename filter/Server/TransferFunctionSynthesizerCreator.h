#ifndef PBVR__TRANSFER_FUNCTION_SYNTHESIZER_CREATOR_H_INCLUDE
#define PBVR__TRANSFER_FUNCTION_SYNTHESIZER_CREATOR_H_INCLUDE

#include <map>
#include <string>
#include <sstream>
#include <vector>

#include "TransferFunction.h"
#include "TransferFunctionSynthesizer.h"
#include "ParticleTransferProtocol.h"
#include "ExtendedTransferFunction.h"
#include "ParameterFile.h"
#include "FilterInformation.h"

// const static size_t TF_COUNT = 5;

class TransferFunctionSynthesizerCreator
{
public:
    class VolumeEquation
    {
    public:
        std::string m_name;
        std::string m_equation;
    };

public:
    TransferFunctionSynthesizerCreator();
    ~TransferFunctionSynthesizerCreator();
    TransferFunctionSynthesizer* create();

    void setFilterInfo( const FilterInformationFile* fi );
    void initQuantityMap( const int32_t m_number_ingredients );
    void setRange( const std::string name, const float min_value, const float max_value );
    void setColorRange( const std::string& name, const float min_value, const float max_value );
    void setOpacityRange( const std::string& name, const float min_value, const float max_value );
    void setProtocol(  const jpv::ParticleTransferClientMessage& clntMes );
    void setParameterFile(  const ParameterFile& pa );
    void setAsisTransferFunction( const pbvr::TransferFunction& tf );
    void setTransferFunction( const std::string& name, const pbvr::TransferFunction& tf );
    // add by @hira at 2016/12/01
    void setInitializeRange( const float min_value, const float max_value );

private:
    TransferFunctionSynthesizer* m_synthesizer;

    // delete by @hira at 2016/12/01
    // std::string m_transfunc_synthesis;

    std::vector<NamedTransferFunction> m_transfunc;
    std::vector<VolumeEquation>        m_voleqn;

    // add by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）
    std::string m_color_transfunc_synthesis;
    std::string m_opacity_transfunc_synthesis;

private:
    void assign();
    void set_protocol(  const jpv::ParticleTransferClientMessage& clntMes );
    void set_param_info(  const ParameterFile& pa );

    QuantityMap m_qmap;
};


#endif //PBVR__TRANSFER_FUNCTION_CONFIGURATOR_H_INCLUDE
