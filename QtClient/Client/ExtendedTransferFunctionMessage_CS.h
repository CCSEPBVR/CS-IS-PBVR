#ifndef PBVR__PARAM_EX_TRANSFUNC_CS_H_INCLUDE
#define PBVR__PARAM_EX_TRANSFUNC_CS_H_INCLUDE

#include <string>
#include <vector>
#include "Types.h"
#include <ExtendedTransferFunctionParameter.h>
#include "ParticleTransferProtocol.h"

namespace kvs
{
namespace visclient
{

class ExtendedTransferFunctionMessage
{
private:
    static const size_t NPRTF = 5;
    static const size_t NRGBA = 4;

public:
    int32_t m_extend_transfer_function_resolution;
    // delete by @hira at 2016/12/01
    // std::vector<NamedTransferFunctionParameter> m_transfer_function;
    // std::string m_extend_transfer_function_synthesis;
    std::string m_extend_volume_synthesis;
    // std::vector<bool> m_range_initialized;

    // add by @hira at 2016/12/01
    /** 色関数定義リスト */
    std::vector<NamedTransferFunctionParameter> m_color_transfer_function;
    /** 不透明度関数定義リスト */
    std::vector<NamedTransferFunctionParameter> m_opacity_transfer_function;
    /** 1次伝達色関数 */
    std::string m_color_transfer_function_synthesis;
    /** 1次伝達不透明度関数 */
    std::string m_opacity_transfer_function_synthesis;
    /** 関数制限数 */
    int m_transfer_function_number;

    // FROM INSITU
    std::vector<NamedTransferFunctionParameter> transferFunction;
    std::string ext_transferFunctionSynthesis;
    std::vector<bool> range_initialized;
    void importFromServerMessage( const jpv::ParticleTransferServerMessage& message );
    // END FROM INSITU
public:
    ExtendedTransferFunctionMessage();
    virtual ~ExtendedTransferFunctionMessage();

    void ExTransFuncParaSet();
    void ExTransFuncParaDump();

public:
    bool operator==( const ExtendedTransferFunctionMessage& s ) const;
    void applyToClientMessage( jpv::ParticleTransferClientMessage* message ) const;
    void setColorTransferFunction(const std::string &name, const std::string &variable);
    void setOpacityTransferFunction(const std::string &name, const std::string &variable);
    NamedTransferFunctionParameter* getColorTransferFunction(const std::string &name);
    NamedTransferFunctionParameter* getColorTransferFunction(const int n);
    NamedTransferFunctionParameter* getOpacityTransferFunction(const std::string &name);
    NamedTransferFunctionParameter* getOpacityTransferFunction(const int n);
    void setColorTransferRange(const std::string &name, float min, float max);
    void setOpacityTransferRange(const std::string &name, float min, float max);
    void addColorTransferFunction(const std::string &name, const std::string &variable);
    void addOpacityTransferFunction(const std::string &name, const std::string &variable);
    void removeColorTransferFunction(const std::string &name);
    void removeOpacityTransferFunction(const std::string &name);

};

}
}

#endif // PBVR__PARAM_EX_TRANSFUNC_CS_H_INCLUDE

