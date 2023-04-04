#ifndef PBVR__PARAM_EX_TRANSFUNC_IS
#define PBVR__PARAM_EX_TRANSFUNC_IS

#include <string>
#include <vector>
#include "Types.h"
#include "ExtendedTransferFunctionParameter.h"
#include "ParticleTransferProtocol.h"

namespace kvs
{
namespace visclient
{

class ParamExTransFunc
{
public:
    static const size_t NPRTF = 5;
    static const size_t NRGBA = 4;

public:
    int32_t ext_transferFunctionResolution;

    std::vector<NamedTransferFunctionParameter> transferFunction;

    std::string ext_transferFunctionSynthesis;
    std::string ext_volumeSynthesis;

    std::vector<bool> range_initialized;

public:
    ParamExTransFunc();
    virtual ~ParamExTransFunc();

    void ExTransFuncParaSet();
    void ExTransFuncParaDump();

public:
    bool operator==( const ParamExTransFunc& s ) const;
    void applyToClientMessage( jpv::ParticleTransferClientMessage* message ) const;
    void importFromServerMessage( const jpv::ParticleTransferServerMessage& message );

//TODO:THESE SHOULD ALL BE REMOVED
public:
//    void setColorTransferFunction(const std::string &name, const std::string &variable){}
//    void setOpacityTransferFunction(const std::string &name, const std::string &variable){}

//    void setColorTransferRange(const std::string &name, float min, float max){}
//    void setOpacityTransferRange(const std::string &name, float min, float max){}
//    void addColorTransferFunction(const std::string &name, const std::string &variable){}
//    void addOpacityTransferFunction(const std::string &name, const std::string &variable){}
//    void removeColorTransferFunction(const std::string &name){}
//    void removeOpacityTransferFunction(const std::string &name){}

    NamedTransferFunctionParameter* getTransferFunction(const std::string &name);
    NamedTransferFunctionParameter* getTransferFunction(const int n);
//    NamedTransferFunctionParameter* getOpacityTransferFunction(const std::string &name){exit(199); return 0;}
//    NamedTransferFunctionParameter* getOpacityTransferFunction(const int n){exit(199); return 0;}
//
//    std::vector<NamedTransferFunctionParameter> colorTransferFunction;
//    std::vector<NamedTransferFunctionParameter> opacityTransferFunction;
//    int m_transfer_function_number;
//    std::string ext_colorTransferFunctionSynthesis;
//    std::string ext_opacityTransferFunctionSynthesis;

};

}
}

#endif // PBVR__PARAM_EX_TRANSFUNC_IS

