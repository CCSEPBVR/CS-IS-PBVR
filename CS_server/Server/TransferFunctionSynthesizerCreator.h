#ifndef VIS_MODULE__TRANSFER_FUNCTION_SYNTHESIZER_CREATOR_H_INCLUDE
#define VIS_MODULE__TRANSFER_FUNCTION_SYNTHESIZER_CREATOR_H_INCLUDE

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <vismodule/TransferFunction>
#include "TransferFunctionSynthesizer.h"
//#include "TransferFunctionSynthesizer_IS.h"
#include "ParticleTransferProtocol.h"
#include "ExtendedTransferFunction.h"
#include "ParameterFile.h"
#include "FilterInformation.h"

// const static size_t TF_COUNT = 5;
#include "ReversePolishNotation.h"
#include "Token.h"

#include "VariableRange.h"

#define TF_COUNT 5 //������ 
#define VAR_OFFSET_A 112 //Token.h->VarName-A1�������
#define VAR_OFFSET_C 96 //Token.h->VarName->C1�������

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

    std::vector<NamedTransferFunction> transfunc(); 
    //std::vector<vismodule::TransferFunction> transfunc(); 
    //std::vector<VolumeEquation>        voleq();
    void setFilterInfo( const FilterInformationFile& fi );
    void initQuantityMap( const int32_t m_number_ingredients );
    void setRange( const std::string name, const float min_value, const float max_value );
    //void setColorRange( const std::string& name, const float min_value, const float max_value );
    //void setOpacityRange( const std::string& name, const float min_value, const float max_value );
    void setColorRange( const float min_value, const float max_value );
    void setOpacityRange( const float min_value, const float max_value );
    void setProtocol(  const jpv::ParticleTransferClientMessage& clntMes );
    void setInitialProtocol(  const int nvariable, const VariableRange vr );
    void setParameterFile(  const ParameterFile& pa );
    void setAsisTransferFunction( const vismodule::TransferFunction& tf );
    void setTransferFunction( jpv::ParticleTransferServerMessage* servMes , const VariableRange vr);
    // add by @hira at 2016/12/01
    void setInitializeRange( const float min_value, const float max_value );

private:
    TransferFunctionSynthesizer* m_synthesizer;

    // delete by @hira at 2016/12/01
    // std::string m_transfunc_synthesis;

    std::vector<NamedTransferFunction> m_transfunc;
    //std::vector<vismodule::TransferFunction> m_transfunc;
    std::vector<VolumeEquation>        m_voleqn;

    // add by @hira at 2016/12/01 : 1���`�B�֐��i�F�A�s�����x�j
    std::string m_color_transfunc_synthesis;
    std::string m_opacity_transfunc_synthesis;

    // add by @shimomura 2022/12/12 
//    EquationToken m_opacity_func;
//    EquationToken m_color_func;
//    std::vector<EquationToken> m_opacity_var;
//    std::vector<EquationToken> m_color_var;

private:
    void assign();
    void set_protocol(  const jpv::ParticleTransferClientMessage& clntMes );
    void set_param_info(  const ParameterFile& pa );
    void read_TF_from_ParamInfo(  const ParameterFile& pa );
    
//    EquationToken convert_token(std::string expression); 
    
    size_t m_component;
    QuantityMap m_qmap;
};


#endif //VIS_MODULE__TRANSFER_FUNCTION_CONFIGURATOR_H_INCLUDE
