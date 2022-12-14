#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include "ExtendedTransferFunctionMessage_IS.h"
#include "ExtendedTransferFunctionParameter.h"

using kvs::visclient::ExtendedTransferFunctionMessage;

ExtendedTransferFunctionMessage::ExtendedTransferFunctionMessage() :
    m_color_transfer_function(NPRTF), m_opacity_transfer_function(NPRTF)
{
    ExTransFuncParaSet();
}

ExtendedTransferFunctionMessage::~ExtendedTransferFunctionMessage()
{
}

void ExtendedTransferFunctionMessage::ExTransFuncParaSet()
{
    m_extend_transfer_function_resolution = 256;
    m_transfer_function_number=NPRTF;

    for ( size_t i = 0; i < m_color_transfer_function.size(); i++ )
    {
        std::stringstream ss;
        ss << "C" << i + 1;
        m_color_transfer_function[i] = NamedTransferFunctionParameter();
        m_color_transfer_function[i].m_name            = ss.str();
        m_color_transfer_function[i].m_color_variable        = "q1";
        m_color_transfer_function[i].m_range_initialized = false;
    }
    for ( size_t i = 0; i < m_opacity_transfer_function.size(); i++ )
    {
        std::stringstream ss;
        ss << "O" << i + 1;
        m_opacity_transfer_function[i] = NamedTransferFunctionParameter();
        m_opacity_transfer_function[i].m_name       = ss.str();
        m_opacity_transfer_function[i].m_opacity_variable   = "q1";
        m_opacity_transfer_function[i].m_range_initialized = false;
    }

    m_extend_volume_synthesis           = "";
    m_color_transfer_function_synthesis = "C1";
    m_opacity_transfer_function_synthesis = "O1";

    return;
}

void ExtendedTransferFunctionMessage::ExTransFuncParaDump()
{
    std::cout << "m_transfer_functionResolution " << m_extend_transfer_function_resolution << std::endl;
    std::cout << "m_color_transfer_function_synthesis " << m_color_transfer_function_synthesis << std::endl;
    std::cout << "m_opacity_transfer_function_synthesis " << m_opacity_transfer_function_synthesis << std::endl;

    for ( size_t i = 0; i < m_color_transfer_function.size(); i++ )
    {
        std::cout << "m_color_transfer_function[" << i << "].m_name          " << m_color_transfer_function[i].m_name << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_color_variable      " << m_color_transfer_function[i].m_color_variable  << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_color_variable_min   " << m_color_transfer_function[i].m_color_variable_min << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_color_variable_max   " << m_color_transfer_function[i].m_color_variable_max << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_equationR     " << m_color_transfer_function[i].m_equation_red << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_equationG     " << m_color_transfer_function[i].m_equation_green << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_equationB     " << m_color_transfer_function[i].m_equation_blue << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_equationA     " << m_color_transfer_function[i].m_equation_opacity << std::endl;
    }

    for ( size_t i = 0; i < m_opacity_transfer_function.size(); i++ )
    {
        std::cout << "m_opacity_transfer_function[" << i << "].m_name          " << m_opacity_transfer_function[i].m_name << std::endl;
        std::cout << "m_opacity_transfer_function[" << i << "].m_opacity_variable    " << m_opacity_transfer_function[i].m_opacity_variable  << std::endl;
        std::cout << "m_opacity_transfer_function[" << i << "].m_opacity_variable_min " << m_opacity_transfer_function[i].m_opacity_variable_min << std::endl;
        std::cout << "m_opacity_transfer_function[" << i << "].m_opacity_variable_max " << m_opacity_transfer_function[i].m_opacity_variable_max << std::endl;
    }

    std::cout << "volumeSynthesis " << m_extend_volume_synthesis << std::endl;
}

bool ExtendedTransferFunctionMessage::operator==( const ExtendedTransferFunctionMessage& s ) const
{
    bool v0 = true;

    v0 &= ( this->m_extend_transfer_function_resolution == s.m_extend_transfer_function_resolution );
    v0 &= ( this->m_extend_volume_synthesis == s.m_extend_volume_synthesis );

    // ?????????
    const size_t n_color_func = this->m_color_transfer_function.size();
    v0 &= ( n_color_func == s.m_color_transfer_function.size() );
    if ( v0 ) {
        for ( size_t n = 0; n < n_color_func; n++ ) {
            v0 &= ( this->m_color_transfer_function[n].operator == ( s.m_color_transfer_function[n] ) );
        }
    }

    // ??????????????????
    const size_t n_opacity_func = this->m_opacity_transfer_function.size();
    v0 &= ( n_opacity_func == s.m_opacity_transfer_function.size() );
    if ( v0 ) {
        for ( size_t n = 0; n < n_opacity_func; n++ ) {
            v0 &= ( this->m_opacity_transfer_function[n].operator == ( s.m_opacity_transfer_function[n] ) );
        }
    }
    // 1?????????????????????
    v0 &= ( this->m_color_transfer_function_synthesis == s.m_color_transfer_function_synthesis );
    // 1??????????????????????????????
    v0 &= ( this->m_opacity_transfer_function_synthesis == s.m_opacity_transfer_function_synthesis );

    return v0;
}

//2019 kawamura
jpv::ParticleTransferClientMessage::EquationToken ExtendedTransferFunctionMessage::convertToken( std::string expression ) const
{
    FuncParser::ExpressionTokenizer tokenizer;
    FuncParser::ExpressionConverter exprconv;

    jpv::ParticleTransferClientMessage::EquationToken eq_token;

    tokenizer.tokenizeString( expression );
    exprconv.convertExpToken( tokenizer.m_exp_token );
    int size = exprconv.token_array.size();
    if( size > 128 ){ printf("Equation length too long\n");}

    for( int i = 0; i < 128; i++ )
    {
        if( i < size )
        {
            eq_token.exp_token[i]   = exprconv.token_array[i];
            eq_token.var_name[i]    = exprconv.var_array[i];
            eq_token.value_array[i] = exprconv.value_array[i];
        }
        else
        {
            eq_token.exp_token[i]   = 0;
            eq_token.var_name[i]    = 0;
            eq_token.value_array[i] = 0;
        }
    }

    return eq_token;
}

void ExtendedTransferFunctionMessage::applyToClientMessage( jpv::ParticleTransferClientMessage* message ) const
{
    message->m_transfer_function.clear();
    message->m_volume_equation.clear();
    message->opacity_var.clear();
    message->color_var.clear();

    // 1???????????????
#if 0
    message->color_func = this->convertToken( this->m_color_transfer_function_synthesis );
    message->opacity_func = this->convertToken( this->m_opacity_transfer_function_synthesis );
#else
    std::string colorSynthBuf = this->m_color_transfer_function_synthesis;
    std::replace(colorSynthBuf.begin(), colorSynthBuf.end(), 'C', 'c');
    message->color_func = this->convertToken( colorSynthBuf );
    std::string opacitySynthBuf = this->m_opacity_transfer_function_synthesis;
    std::replace(opacitySynthBuf.begin(), opacitySynthBuf.end(), 'O', 'a');
    message->opacity_func = this->convertToken( opacitySynthBuf );

    message->color_tf_synthesis =  this->m_color_transfer_function_synthesis;
    message->opacity_tf_synthesis = this->m_opacity_transfer_function_synthesis;

#endif
    std::cout<<"color_func="<<colorSynthBuf<<",opacity_func="<<opacitySynthBuf<<std::endl;

#if 0
    // ??????????????????
    for ( size_t i = 0; i < this->m_color_transfer_function.size(); i++ )
    {
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation veq;

        const NamedTransferFunctionParameter& tf = this->m_color_transfer_function[i];
        etf = tf;
        int func_num = etf.getNameNumber();
        std::stringstream ss;
        ss << "_F" << func_num;
        etf.ColorVar   = ss.str() + "_VAR_C";
        etf.OpacityVar = "";
        veq.Name     = etf.ColorVar;
        veq.Equation = this->m_color_transfer_function[i].ColorVar;
        message->transfunc.push_back( etf );
        message->voleqn.push_back( veq );

        message->color_var.push_back( this->convertToken( this->m_color_transfer_function[i].ColorVar ) );
    }

    // ???????????????????????????
    for ( size_t i = 0; i < this->m_opacity_transfer_function.size(); i++ )
    {
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation veq;

        const NamedTransferFunctionParameter& tf = this->m_opacity_transfer_function[i];
        etf = tf;
        int func_num = etf.getNameNumber();
        std::stringstream ss;
        ss << "_F" << func_num;
        etf.OpacityVar   = ss.str() + "_VAR_O";
        etf.ColorVar     = "";
        veq.Name     = etf.OpacityVar;
        veq.Equation = this->m_opacity_transfer_function[i].OpacityVar;
        message->transfunc.push_back( etf );
        message->voleqn.push_back( veq );

        message->opacity_var.push_back( this->convertToken( this->m_opacity_transfer_function[i].OpacityVar ) );
    }
#else
    for ( size_t i = 0; i < this->m_color_transfer_function.size(); i++ )
    {
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation veq_c, veq_o;

        const NamedTransferFunctionParameter& c_tf = this->m_color_transfer_function[i];
        const NamedTransferFunctionParameter& o_tf = this->m_opacity_transfer_function[i];

        etf = c_tf;

        std::stringstream ss;
        ss << "_F" << i;

        etf.m_color_variable   = ss.str() + "_VAR_C";
        etf.m_opacity_variable = ss.str() + "_VAR_O";
        veq_c.m_name     = etf.m_color_variable;
        veq_o.m_name     = etf.m_opacity_variable;
        veq_c.m_equation = this->m_color_transfer_function[i].m_color_variable;
        veq_o.m_equation = this->m_opacity_transfer_function[i].m_opacity_variable;

        std::replace(etf.m_name.begin(), etf.m_name.end(), 'C', 't');
        etf.setOpacityMap( o_tf.opacityMap() );
        etf.m_equation_opacity = o_tf.m_equation_opacity;
        etf.m_opacity_variable_min = o_tf.m_opacity_variable_min;
        etf.m_opacity_variable_max = o_tf.m_opacity_variable_max;

        message->m_transfer_function.push_back( etf );
        message->m_volume_equation.push_back( veq_c );
        message->m_volume_equation.push_back( veq_o );

        message->color_var.push_back( this->convertToken( this->m_color_transfer_function[i].m_color_variable ) );
        message->opacity_var.push_back( this->convertToken( this->m_opacity_transfer_function[i].m_opacity_variable ) );
    }

#endif

    return;
}



/**
 * ????????????????????????
 * ???????????????????????????????????????????????????????????????
 * @param name			??????????????????
 * @param variable		?????????
 */
void ExtendedTransferFunctionMessage::setColorTransferFunction(
        const std::string &name,
        const std::string &variable)
{
    // ????????????????????????????????????????????????
    NamedTransferFunctionParameter *trans = this->getColorTransferFunction(name);
    if (trans != NULL) {
        trans->m_name       = name;
        trans->m_color_variable   = variable;
        return;
    }

    // ?????????????????????
    NamedTransferFunctionParameter transfer_function;
    if (this->m_color_transfer_function.size() > 0) {
        transfer_function = NamedTransferFunctionParameter(this->m_color_transfer_function[0]);
    }
    transfer_function.m_name       = name;
    transfer_function.m_color_variable   = variable;
    transfer_function.m_range_initialized = false;

    this->m_color_transfer_function.push_back(transfer_function);

    // ?????????????????????
    std::sort(this->m_color_transfer_function.begin(), this->m_color_transfer_function.end(), NamedTransferFunctionParameter::compareName);

    return;
}

/**
 * ?????????????????????????????????
 * ????????????????????????????????????????????????????????????????????????
 * @param name			???????????????????????????
 * @param variable		??????????????????
 */
void ExtendedTransferFunctionMessage::setOpacityTransferFunction(
        const std::string &name,
        const std::string &variable)
{

    // ????????????????????????????????????????????????
    NamedTransferFunctionParameter *trans = this->getOpacityTransferFunction(name);
    if (trans != NULL) {
        trans->m_name       = name;
        trans->m_opacity_variable   = variable;
        return;
    }

    // ?????????????????????
    NamedTransferFunctionParameter transfer_function;
    if (this->m_color_transfer_function.size() > 0) {
        transfer_function = NamedTransferFunctionParameter(this->m_opacity_transfer_function[0]);
    }
    transfer_function.m_name       = name;
    transfer_function.m_opacity_variable   = variable;
    transfer_function.m_range_initialized = false;

    this->m_opacity_transfer_function.push_back(transfer_function);

    // ?????????????????????
    std::sort(this->m_opacity_transfer_function.begin(), this->m_opacity_transfer_function.end(), NamedTransferFunctionParameter::compareName);

    return;
}

/**
 * ????????????????????????
 * @param name		????????????
 * @return		?????????
 */
NamedTransferFunctionParameter* ExtendedTransferFunctionMessage::getColorTransferFunction(const std::string &name)
{
    std::vector<NamedTransferFunctionParameter>::iterator itr;
    for (itr = this->m_color_transfer_function.begin(); itr != this->m_color_transfer_function.end(); itr++) {
        NamedTransferFunctionParameter trans = (*itr);
        if (trans.m_name == name) {
            return &(*itr);
        }
    }

    return NULL;
}

/**
 * ????????????????????????
 * @param n		???????????????
 * @return		?????????
 */
NamedTransferFunctionParameter* ExtendedTransferFunctionMessage::getColorTransferFunction(const int n)
{
    char name[8] = {0x00};
    sprintf(name, "C%d", n);
    return this->getColorTransferFunction(name);
}

/**
 * ?????????????????????????????????
 * @param name		?????????????????????
 * @return		??????????????????
 */
NamedTransferFunctionParameter* ExtendedTransferFunctionMessage::getOpacityTransferFunction(const std::string &name)
{
    std::vector<NamedTransferFunctionParameter>::iterator itr;
    for (itr = this->m_opacity_transfer_function.begin(); itr != this->m_opacity_transfer_function.end(); itr++) {
        NamedTransferFunctionParameter trans = (*itr);
        if (trans.m_name == name) {
            return &(*itr);
        }
    }

    return NULL;
}

/**
 * ?????????????????????????????????
 * @param n		????????????????????????
 * @return		??????????????????
 */
NamedTransferFunctionParameter* ExtendedTransferFunctionMessage::getOpacityTransferFunction(const int n)
{
    char name[8] = {0x00};
    sprintf(name, "O%d", n);
    return this->getOpacityTransferFunction(name);
}


/**
 * ?????????::????????????????????????????????????
 * @param name			??????????????????
 * @param min 			?????????
 * @param max			?????????
 * @param variable		?????????
 */
void ExtendedTransferFunctionMessage::setColorTransferRange(
        const std::string &name, float min, float max)
{
    // ???????????????????????????
    NamedTransferFunctionParameter *trans = this->getColorTransferFunction(name);
    if (trans == NULL) return;

    trans->m_color_variable_min = min;
    trans->m_color_variable_max = max;

    return;
}

/**
 * ??????????????????::????????????????????????????????????
 * @param name			???????????????????????????
 * @param min 			?????????
 * @param max			?????????
 * @param variable		??????????????????
 */
void ExtendedTransferFunctionMessage::setOpacityTransferRange(
        const std::string &name, float min, float max)
{
    // ???????????????????????????
    NamedTransferFunctionParameter *trans = this->getOpacityTransferFunction(name);
    if (trans == NULL) return;

    trans->m_opacity_variable_min = min;
    trans->m_opacity_variable_max = max;

    return;
}


/**
 * ????????????????????????
 * ??????????????????????????????????????????????????????????????????
 * @param name			??????????????????
 * @param variable		?????????
 */
void ExtendedTransferFunctionMessage::addColorTransferFunction(
        const std::string &name,
        const std::string &variable)
{
    // ????????????????????????????????????????????????
    NamedTransferFunctionParameter *trans = this->getColorTransferFunction(name);
    if (trans != NULL) {
        return;
    }

    // ?????????????????????
    NamedTransferFunctionParameter transfer_function;
    transfer_function.m_name       = name;
    transfer_function.m_color_variable   = variable;
    // ?????????[0]?????????????????????
    transfer_function.m_resolution = this->m_color_transfer_function[0].m_resolution;
    transfer_function.m_color_variable_min = this->m_color_transfer_function[0].m_color_variable_min;
    transfer_function.m_color_variable_max = this->m_color_transfer_function[0].m_color_variable_max;
    // ???????????????????????????true?????????
    transfer_function.m_range_initialized = true;

    this->m_color_transfer_function.push_back(transfer_function);

    // ?????????????????????
    std::sort(this->m_color_transfer_function.begin(), this->m_color_transfer_function.end(), NamedTransferFunctionParameter::compareName);

    return;
}

/**
 * ?????????????????????????????????
 * ???????????????????????????????????????????????????????????????????????????
 * @param name			???????????????????????????
 * @param variable		??????????????????
 */
void ExtendedTransferFunctionMessage::addOpacityTransferFunction(
        const std::string &name,
        const std::string &variable)
{
    // ????????????????????????????????????????????????
    NamedTransferFunctionParameter *trans = this->getOpacityTransferFunction(name);
    if (trans != NULL) {
        return;
    }

    // ?????????????????????
    NamedTransferFunctionParameter transfer_function;
    transfer_function.m_name       = name;
    transfer_function.m_opacity_variable   = variable;
    // ?????????[0]?????????????????????
    transfer_function.m_resolution = this->m_opacity_transfer_function[0].m_resolution;
    transfer_function.m_opacity_variable_min = this->m_opacity_transfer_function[0].m_opacity_variable_min;
    transfer_function.m_opacity_variable_max = this->m_opacity_transfer_function[0].m_opacity_variable_max;
    // ???????????????????????????true?????????
    transfer_function.m_range_initialized = true;

    this->m_opacity_transfer_function.push_back(transfer_function);

    // ?????????????????????
    std::sort(this->m_opacity_transfer_function.begin(), this->m_opacity_transfer_function.end(), NamedTransferFunctionParameter::compareName);

    return;
}


/**
 * ????????????????????????
 * @param name			??????????????????
 */
void ExtendedTransferFunctionMessage::removeColorTransferFunction(
        const std::string &name)
{
    std::vector<NamedTransferFunctionParameter>::iterator itr;
    for (itr=this->m_color_transfer_function.begin(); itr!= this->m_color_transfer_function.end(); itr++) {
        if (itr->m_name == name) {
            this->m_color_transfer_function.erase(itr);
            break;
        }
    }

    return;
}

/**
 * ?????????????????????????????????
 * @param name			???????????????????????????
 */
void ExtendedTransferFunctionMessage::removeOpacityTransferFunction(
        const std::string &name)
{
    std::vector<NamedTransferFunctionParameter>::iterator itr;
    for (itr=this->m_opacity_transfer_function.begin(); itr!= this->m_opacity_transfer_function.end(); itr++) {
        if (itr->m_name == name) {
            this->m_opacity_transfer_function.erase(itr);
            break;
        }
    }

    return;
}

void ExtendedTransferFunctionMessage::importFromServerMessage( const jpv::ParticleTransferServerMessage& message )
{
    this->m_extend_transfer_function_resolution= 256;

    //this->transferFunction.clear();
    this->m_color_transfer_function.clear();
    this->m_opacity_transfer_function.clear();

    // set tf_number
    this->m_transfer_function_number = message.transfunc.size();

    for ( size_t i = 0, j = 0; i < message.transfunc.size(); i++, j+=2 )
    {
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation veq_c, veq_o;

        const NamedTransferFunctionParameter& tf = message.transfunc[i];

        etf = tf;

        //this->transferFunction.push_back( etf );
        this->m_color_transfer_function.push_back(etf);
        this->m_opacity_transfer_function.push_back(etf);

        std::string* nameBuf = &this->m_color_transfer_function[i].m_name;
        std::replace(nameBuf->begin(), nameBuf->end(), 't', 'C');
        nameBuf = &this->m_opacity_transfer_function[i].m_name;
        std::replace(nameBuf->begin(), nameBuf->end(), 't', 'O');
    }
    //this->ext_transferFunctionSynthesis = message.transferFunctionSynthesis;
    //std::string strbuf = message.transferFunctionSynthesis;
    //std::replace(strbuf.begin(), strbuf.end(), 't', 'C');
    //this->m_color_transfer_function_synthesis = strbuf;
    this->m_color_transfer_function_synthesis = message.color_tf_synthesis;
    //strbuf = message.transferFunctionSynthesis;
    //std::replace(strbuf.begin(), strbuf.end(), 't', 'O');
    //this->m_opacity_transfer_function_synthesis = strbuf;
    this->m_opacity_transfer_function_synthesis = message.opacity_tf_synthesis;
}

void ExtendedTransferFunctionMessage::show( void ) const
{
    std::cout<<std::endl;
    std::cout<<"SHOW PARAMEXTRANSFUNC"<<std::endl;
    //std::cout<<"ext_transferFunctionSynthesis="<<ext_transferFunctionSynthesis<<std::endl;
    std::cout << "m_color_transfer_function_synthesis=" << m_color_transfer_function_synthesis << std::endl;
    std::cout << "m_opacity_transfer_function_synthesis=" << m_opacity_transfer_function_synthesis << std::endl;
    //std::cout<<"ext_volumeSynthesis"<<ext_volumeSynthesis<<std::endl;
    std::cout << "m_extend_volume_synthesis=" << m_extend_volume_synthesis << std::endl;
    //std::cout<<"transferFunction.Name,ColorVar,OpacityVar,ColorVarMin,ColorVarMax"<<std::endl;

    std::cout << "Color" << std::endl;
    std::cout<<"transferFunction.Name,ColorVar,ColorVarMin,ColorVarMax"<<std::endl;
    for(int i=0; i<m_color_transfer_function.size(); i++)
    {
        std::cout<< m_color_transfer_function[i].m_name<<","<< m_color_transfer_function[i].m_color_variable<<","<< m_color_transfer_function[i].m_color_variable_min<<","<< m_color_transfer_function[i].m_color_variable_max<<std::endl;
    }
    std::cout << "Opacity" << std::endl;
    std::cout << "transferFunction.Name,OpacityVar,OpacityVarMin,OpacityVarMax" << std::endl;
    for (int i = 0; i < m_opacity_transfer_function.size(); i++) {
        std::cout << m_opacity_transfer_function[i].m_name << "," << m_opacity_transfer_function[i].m_opacity_variable << "," << m_opacity_transfer_function[i].m_opacity_variable_min << "," << m_opacity_transfer_function[i].m_opacity_variable_max << std::endl;
    }

}
