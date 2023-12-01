#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include "ExtendedTransferFunctionMessage_CS.h"
#include "ExtendedTransferFunctionParameter.h"
#include "Token.h"
#include <QDebug>
using kvs::visclient::ExtendedTransferFunctionMessage;

ExtendedTransferFunctionMessage::ExtendedTransferFunctionMessage() :
    m_color_transfer_function(NPRTF),
    m_opacity_transfer_function(NPRTF)
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
        m_color_transfer_function[i].m_name       = ss.str();
        m_color_transfer_function[i].m_color_variable   = "q1";
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

    // 色関数
    const size_t n_color_func = this->m_color_transfer_function.size();
    v0 &= ( n_color_func == s.m_color_transfer_function.size() );
    if ( v0 ) {
        for ( size_t n = 0; n < n_color_func; n++ ) {
            v0 &= ( this->m_color_transfer_function[n].operator == ( s.m_color_transfer_function[n] ) );
        }
    }

    // 不透明度関数
    const size_t n_opacity_func = this->m_opacity_transfer_function.size();
    v0 &= ( n_opacity_func == s.m_opacity_transfer_function.size() );
    if ( v0 ) {
        for ( size_t n = 0; n < n_opacity_func; n++ ) {
            v0 &= ( this->m_opacity_transfer_function[n].operator == ( s.m_opacity_transfer_function[n] ) );
        }
    }
    // 1次伝達関数：色
    v0 &= ( this->m_color_transfer_function_synthesis == s.m_color_transfer_function_synthesis );
    // 1次伝達関数：不透明度
    v0 &= ( this->m_opacity_transfer_function_synthesis == s.m_opacity_transfer_function_synthesis );

    return v0;
}

//add by shimomura 2023/1/26
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

    // 1次伝達関数
    message->m_color_transfer_function_synthesis = this->m_color_transfer_function_synthesis;
    message->m_opacity_transfer_function_synthesis = this->m_opacity_transfer_function_synthesis;

    // 色関数リスト
    for ( size_t i = 0; i < this->m_color_transfer_function.size(); i++ )
    {
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation veq;

        const NamedTransferFunctionParameter& tf = this->m_color_transfer_function[i];
        etf = tf;
        int func_num = etf.getNameNumber();
        std::stringstream ss;
        ss << "_F" << func_num;
        etf.m_color_variable   = ss.str() + "_VAR_C";
        veq.m_name     = etf.m_color_variable;
        veq.m_equation = this->m_color_transfer_function[i].m_color_variable;
        message->m_transfer_function.push_back( etf );
        message->m_volume_equation.push_back( veq );
        message->show();
    }

    // 不透明度関数リスト
    for ( size_t i = 0; i < this->m_opacity_transfer_function.size(); i++ )
    {
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation veq;

        const NamedTransferFunctionParameter& tf = this->m_opacity_transfer_function[i];
        etf = tf;
        int func_num = etf.getNameNumber();
        std::stringstream ss;
        ss << "_F" << func_num;
        etf.m_opacity_variable   = ss.str() + "_VAR_O";
        veq.m_name     = etf.m_opacity_variable;
        veq.m_equation = this->m_opacity_transfer_function[i].m_opacity_variable;
        message->m_transfer_function.push_back( etf );
        message->m_volume_equation.push_back( veq );
    }

//      //2023 shimomura   stab data
//       jpv::ParticleTransferClientMessage::EquationToken opa_func = {
//            {VARIABLE, END}, // A1+A2 -> A1 A2 +
//            {A1},// A1, A2
//            {} //nothing
//        };

//        message->opacity_func = opa_func;

//        jpv::ParticleTransferClientMessage::EquationToken col_func = {
//            {VARIABLE, END},// C1
//            {C1},// C1
//            {}//nothing
//        };

//        message->color_func = col_func;

//        jpv::ParticleTransferClientMessage::EquationToken opa_var_1 = {
//    //        {VARIABLE, VARIABLE, PLUS, END}, //q1+q1 -> q1 q1 +
//    //        {Q1,Q1},
//            {VARIABLE, END}, //q1+q1 -> q1 q1 +
//            {Q1},
//            {}
//        };

//        jpv::ParticleTransferClientMessage::EquationToken col_var_1 = {
//    //        {VARIABLE,VARIABLE, PLUS, END}, //q1+q1 -> q1 q1 +
//    //        {Q1, Q1},
//            {VARIABLE, END}, //q1+q1 -> q1 q1 +
//            {Q1},
//            {}
//        };

//        std::vector<jpv::ParticleTransferClientMessage::EquationToken> opa_var;//ex) q1+q2
//        std::vector<jpv::ParticleTransferClientMessage::EquationToken> col_var;//ex) q3*q4
//        opa_var.push_back( opa_var_1 );
//        col_var.push_back( col_var_1 );

//        message->color_var = opa_var;
//        message->opacity_var = col_var;

        std::string colorSynthBuf = this->m_color_transfer_function_synthesis;
        std::replace(colorSynthBuf.begin(), colorSynthBuf.end(), 'C', 'c');
        message->color_func = this->convertToken( colorSynthBuf );
        std::string opacitySynthBuf = this->m_opacity_transfer_function_synthesis;
        std::replace(opacitySynthBuf.begin(), opacitySynthBuf.end(), 'O', 'a');
        message->opacity_func = this->convertToken( opacitySynthBuf );

        for ( size_t i = 0; i < this->m_color_transfer_function.size(); i++ )
        {
        message->color_var.push_back( this->convertToken( this->m_color_transfer_function[i].m_color_variable ) );
        message->opacity_var.push_back( this->convertToken( this->m_opacity_transfer_function[i].m_opacity_variable ) );
        }

        //2023 shimomura
        std::cout <<" message->m_x_synthesis ="  << message->m_x_synthesis << std::endl;
        if( !message->m_x_synthesis.empty() ) message->x_synthesis_token = this->convertToken( message->m_x_synthesis );
        if( !message->m_y_synthesis.empty() ) message->y_synthesis_token = this->convertToken( message->m_y_synthesis );
        if( !message->m_z_synthesis.empty() ) message->z_synthesis_token = this->convertToken( message->m_z_synthesis );

    return;
}



/**
 * 色関数を設定する
 * 色関数定義名が存在していなければ追加する。
 * @param name			色関数定義名
 * @param variable		色変数
 */
void ExtendedTransferFunctionMessage::setColorTransferFunction(
        const std::string &name,
        const std::string &variable)
{
    // 既存定義が存在するかチェックする
    NamedTransferFunctionParameter *trans = this->getColorTransferFunction(name);
    if (trans != NULL) {
        trans->m_name       = name;
        trans->m_color_variable   = variable;
        return;
    }

    // 新規に追加する
    NamedTransferFunctionParameter transfer_function;
    if (this->m_color_transfer_function.size() > 0) {
        transfer_function = NamedTransferFunctionParameter(this->m_color_transfer_function[0]);
    }
    transfer_function.m_name       = name;
    transfer_function.m_color_variable   = variable;
    transfer_function.m_range_initialized = false;

    this->m_color_transfer_function.push_back(transfer_function);

    // 定義名でソート
    std::sort(this->m_color_transfer_function.begin(), this->m_color_transfer_function.end(), NamedTransferFunctionParameter::compareName);

    return;
}

/**
 * 不透明度関数を設定する
 * 不透明度関数定義名が存在していなければ追加する。
 * @param name			不透明度関数定義名
 * @param variable		不透明度変数
 */
void ExtendedTransferFunctionMessage::setOpacityTransferFunction(
        const std::string &name,
        const std::string &variable)
{

    // 既存定義が存在するかチェックする
    NamedTransferFunctionParameter *trans = this->getOpacityTransferFunction(name);
    if (trans != NULL) {
        trans->m_name       = name;
        trans->m_opacity_variable   = variable;
        return;
    }

    // 新規に追加する
    NamedTransferFunctionParameter transfer_function;
    if (this->m_color_transfer_function.size() > 0) {
        transfer_function = NamedTransferFunctionParameter(this->m_opacity_transfer_function[0]);
    }
    transfer_function.m_name       = name;
    transfer_function.m_opacity_variable   = variable;
    transfer_function.m_range_initialized = false;

    this->m_opacity_transfer_function.push_back(transfer_function);

    // 定義名でソート
    std::sort(this->m_opacity_transfer_function.begin(), this->m_opacity_transfer_function.end(), NamedTransferFunctionParameter::compareName);

    return;
}

/**
 * 色関数を取得する
 * @param name		色関数名
 * @return		色関数
 */
NamedTransferFunctionParameter* ExtendedTransferFunctionMessage::getColorTransferFunction(const std::string &name)
{
//    qInfo("ParamExTransFunc::getColorTransferFunction %s ",name.c_str() );
    std::vector<NamedTransferFunctionParameter>::iterator itr;
    for (itr = this->m_color_transfer_function.begin(); itr != this->m_color_transfer_function.end(); itr++) {
        NamedTransferFunctionParameter trans = (*itr);
        if (trans.m_name == name) {
//            qInfo("found it");
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
NamedTransferFunctionParameter* ExtendedTransferFunctionMessage::getColorTransferFunction(const int n)
{
//    qInfo("ParamExTransFunc::getColorTransferFunction %d ",n );
    char name[8] = {0x00};
    sprintf(name, "C%d", n);
    return this->getColorTransferFunction(name);
}

/**
 * 不透明度関数を取得する
 * @param name		不透明度関数名
 * @return		不透明度関数
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
 * 不透明度関数を取得する
 * @param n		不透明度関数番号
 * @return		不透明度関数
 */
NamedTransferFunctionParameter* ExtendedTransferFunctionMessage::getOpacityTransferFunction(const int n)
{
    char name[8] = {0x00};
    sprintf(name, "O%d", n);
    return this->getOpacityTransferFunction(name);
}


/**
 * 色関数::最小値、最大値を追加する
 * @param name			色関数定義名
 * @param min 			最小値
 * @param max			最大値
 * @param variable		色変数
 */
void ExtendedTransferFunctionMessage::setColorTransferRange(
        const std::string &name, float min, float max)
{
    // 既存定義を取得する
    NamedTransferFunctionParameter *trans = this->getColorTransferFunction(name);
    if (trans == NULL) return;

    trans->m_color_variable_min = min;
    trans->m_color_variable_max = max;

    return;
}

/**
 * 不透明度関数::最小値、最大値を追加する
 * @param name			不透明度関数定義名
 * @param min 			最小値
 * @param max			最大値
 * @param variable		不透明度変数
 */
void ExtendedTransferFunctionMessage::setOpacityTransferRange(
        const std::string &name, float min, float max)
{
    // 既存定義を取得する
    NamedTransferFunctionParameter *trans = this->getOpacityTransferFunction(name);
    if (trans == NULL) {
        std::cout << "Transfer function " <<name<< " not found\n";
        return;
    }
    trans->m_opacity_variable_min = min;
    trans->m_opacity_variable_max = max;

    return;
}


/**
 * 色関数を追加する
 * 色関数定義名が存在していなければ追加しない。
 * @param name			色関数定義名
 * @param variable		色変数
 */
void ExtendedTransferFunctionMessage::addColorTransferFunction(
        const std::string &name,
        const std::string &variable)
{
    // 既存定義が存在するかチェックする
    NamedTransferFunctionParameter *trans = this->getColorTransferFunction(name);
    if (trans != NULL) {
        return;
    }

    // 新規に追加する
    NamedTransferFunctionParameter transfer_function;
    transfer_function.m_name       = name;
    transfer_function.m_color_variable   = variable;
    // 範囲は[0]からコピーする
    transfer_function.m_resolution = this->m_color_transfer_function[0].m_resolution;
    transfer_function.m_color_variable_min = this->m_color_transfer_function[0].m_color_variable_min;
    transfer_function.m_color_variable_max = this->m_color_transfer_function[0].m_color_variable_max;
    // 更新されないようにtrueとする
    transfer_function.m_range_initialized = true;

    this->m_color_transfer_function.push_back(transfer_function);

    // 定義名でソート
    std::sort(this->m_color_transfer_function.begin(), this->m_color_transfer_function.end(), NamedTransferFunctionParameter::compareName);

    return;
}

/**
 * 不透明度関数を追加する
 * 不透明度関数定義名が存在していなければ追加しない。
 * @param name			不透明度関数定義名
 * @param variable		不透明度変数
 */
void ExtendedTransferFunctionMessage::addOpacityTransferFunction(
        const std::string &name,
        const std::string &variable)
{
    // 既存定義が存在するかチェックする
    NamedTransferFunctionParameter *trans = this->getOpacityTransferFunction(name);
    if (trans != NULL) {
        return;
    }

    // 新規に追加する
    NamedTransferFunctionParameter transfer_function;
    transfer_function.m_name       = name;
    transfer_function.m_opacity_variable   = variable;
    // 範囲は[0]からコピーする
    transfer_function.m_resolution = this->m_opacity_transfer_function[0].m_resolution;
    transfer_function.m_opacity_variable_min = this->m_opacity_transfer_function[0].m_opacity_variable_min;
    transfer_function.m_opacity_variable_max = this->m_opacity_transfer_function[0].m_opacity_variable_max;
    // 更新されないようにtrueとする
    transfer_function.m_range_initialized = true;

    this->m_opacity_transfer_function.push_back(transfer_function);

    // 定義名でソート
    std::sort(this->m_opacity_transfer_function.begin(), this->m_opacity_transfer_function.end(), NamedTransferFunctionParameter::compareName);

    return;
}


/**
 * 色関数を削除する
 * @param name			色関数定義名
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
 * 不透明度関数を削除する
 * @param name			不透明度関数定義名
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
    this->m_color_transfer_function_synthesis = message.color_tf_synthesis;
    this->m_opacity_transfer_function_synthesis = message.opacity_tf_synthesis;

/*#ifdef COMM_MODE_IS
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
#else
    qInfo("ParamExTransFunc::importFromServerMessage only implemented for INSITU");
    exit(1);

#endif
*/
}
