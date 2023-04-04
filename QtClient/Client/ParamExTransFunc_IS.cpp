#include <iostream>
#include <sstream>
#include "ParamExTransFunc_IS.h"
#include "ExtendedTransferFunctionParameter.h"

using kvs::visclient::ParamExTransFunc;

ParamExTransFunc::ParamExTransFunc() :
    transferFunction( NPRTF ),
    range_initialized( NPRTF, false )
{
    ExTransFuncParaSet();
}

ParamExTransFunc::~ParamExTransFunc()
{
}

void ParamExTransFunc::ExTransFuncParaSet()
{
    ext_transferFunctionResolution = 256;
    for ( size_t i = 0; i < transferFunction.size(); i++ )
    {
        std::stringstream ss;
        ss << "t" << i + 1;
        transferFunction[i] = NamedTransferFunctionParameter();
        transferFunction[i].Name       = ss.str();
        transferFunction[i].ColorVar   = "q1";
        transferFunction[i].OpacityVar = "q1";

        range_initialized[i] = false;
    }
    ext_transferFunctionSynthesis = "t1";
    ext_volumeSynthesis           = "";
    return;
}

void ParamExTransFunc::ExTransFuncParaDump()
{
    std::cout << "transferFunctionResolution " << ext_transferFunctionResolution << std::endl;
    for ( size_t i = 0; i < transferFunction.size(); i++ )
    {
        std::cout << "transferFunction[" << i << "].Name          " << transferFunction[i].Name << std::endl;
        std::cout << "transferFunction[" << i << "].ColorVar      " << transferFunction[i].ColorVar  << std::endl;
        std::cout << "transferFunction[" << i << "].ColorVarMin   " << transferFunction[i].ColorVarMin << std::endl;
        std::cout << "transferFunction[" << i << "].ColorVarMax   " << transferFunction[i].ColorVarMax << std::endl;
        std::cout << "transferFunction[" << i << "].OpacityVar    " << transferFunction[i].OpacityVar  << std::endl;
        std::cout << "transferFunction[" << i << "].OpacityVarMin " << transferFunction[i].OpacityVarMin << std::endl;
        std::cout << "transferFunction[" << i << "].OpacityVarMax " << transferFunction[i].OpacityVarMax << std::endl;
        std::cout << "transferFunction[" << i << "].EquationR     " << transferFunction[i].EquationR << std::endl;
        std::cout << "transferFunction[" << i << "].EquationG     " << transferFunction[i].EquationG << std::endl;
        std::cout << "transferFunction[" << i << "].EquationB     " << transferFunction[i].EquationB << std::endl;
        std::cout << "transferFunction[" << i << "].EquationA     " << transferFunction[i].EquationA << std::endl;
    }
    std::cout << "transferFunctionSynthesis " << ext_transferFunctionSynthesis << std::endl;
    std::cout << "volumeSynthesis " << ext_volumeSynthesis << std::endl;
}

bool ParamExTransFunc::operator==( const ParamExTransFunc& s ) const
{
    bool v0 = true;

    v0 &= ( this->ext_transferFunctionResolution == s.ext_transferFunctionResolution );
    v0 &= ( this->ext_transferFunctionSynthesis == s.ext_transferFunctionSynthesis );
    v0 &= ( this->ext_volumeSynthesis == s.ext_volumeSynthesis );

    const size_t n_trans_func = this->transferFunction.size();
    const size_t n_range_init = this->range_initialized.size();

    v0 &= ( n_trans_func == s.transferFunction.size() );
    v0 &= ( n_range_init == s.range_initialized.size() );

    if ( v0 )
    {
        for ( size_t n = 0; n < n_trans_func; n++ )
        {
            v0 &= ( this->transferFunction[n].operator == ( s.transferFunction[n] ) );
        }
        for ( size_t n = 0; n < n_range_init; n++ )
        {
            v0 &= ( this->range_initialized[n] == s.range_initialized[n] );
        }
    }

    return v0;
}

void ParamExTransFunc::applyToClientMessage( jpv::ParticleTransferClientMessage* message ) const
{
    message->transfunc.clear();
    message->voleqn.clear();

    for ( size_t i = 0; i < ParamExTransFunc::NPRTF; i++ )
    {
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation veq_c, veq_o;

        const NamedTransferFunctionParameter& tf = this->transferFunction[i];

        etf = tf;

        if ( etf.ColorVar   == "" )
        {
            continue;
        }
        if ( etf.OpacityVar == "" )
        {
            continue;
        }

        std::stringstream ss;
        ss << "_F" << i;

        etf.ColorVar   = ss.str() + "_VAR_C";
        etf.OpacityVar = ss.str() + "_VAR_O";
        veq_c.Name     = etf.ColorVar;
        veq_o.Name     = etf.OpacityVar;
        veq_c.Equation = this->transferFunction[i].ColorVar;
        veq_o.Equation = this->transferFunction[i].OpacityVar;

        message->transfunc.push_back( etf );
        message->voleqn.push_back( veq_c );
        message->voleqn.push_back( veq_o );
    }
    message->transferFunctionSynthesis = this->ext_transferFunctionSynthesis;
}

void ParamExTransFunc::importFromServerMessage( const jpv::ParticleTransferServerMessage& message )
{
    this->ext_transferFunctionResolution= 256;

    this->transferFunction.clear();
    for ( size_t i = 0, j = 0; i < message.transfunc.size(); i++, j+=2 )
    {
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation veq_c, veq_o;

        const NamedTransferFunctionParameter& tf = message.transfunc[i];

        etf = tf;

        this->transferFunction.push_back( etf );
    }
    this->ext_transferFunctionSynthesis = message.transferFunctionSynthesis;
}

/**
 * 色関数を取得する
 * @param name		色関数名
 * @return		色関数
 */
NamedTransferFunctionParameter* ParamExTransFunc::getTransferFunction(const std::string &name)
{

    std::vector<NamedTransferFunctionParameter>::iterator itr;
    for (itr = this->transferFunction.begin(); itr != this->transferFunction.end(); itr++) {
        NamedTransferFunctionParameter trans = (*itr);
        if (trans.Name == name) {
            return &(*itr);
        }
    }

    return NULL;
}

/**
 * 色関数を取得する
 * @param n		色関数番号
 * @return		色関数
 */
NamedTransferFunctionParameter* ParamExTransFunc::getTransferFunction(const int n)
{
    char name[8] = {0x00};
    sprintf(name, "t%d", n);
    return this->getTransferFunction(name);
}
