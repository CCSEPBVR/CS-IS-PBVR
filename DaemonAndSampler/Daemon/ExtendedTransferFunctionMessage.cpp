#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include "ExtendedTransferFunctionMessage.h"
#include "ExtendedTransferFunctionParameter.h"

using kvs::visclient::ExtendedTransferFunctionMessage;

ExtendedTransferFunctionMessage::ExtendedTransferFunctionMessage() :
    m_color_transfer_function(m_nprtf), m_opacity_transfer_function(m_nprtf)
{
    ExTransFuncParaSet();
}

ExtendedTransferFunctionMessage::~ExtendedTransferFunctionMessage()
{
}

void ExtendedTransferFunctionMessage::ExTransFuncParaSet()
{
    m_extend_transfer_function_resolution = 256;
    m_transfer_function_number=m_nprtf;

    for ( size_t i = 0; i < m_color_transfer_function.size(); i++ )
    {
        std::stringstream ss;
        ss << "C" << i + 1;
        m_color_transfer_function[i] = NamedTransferFunctionParameter();
        m_color_transfer_function[i].Name            = ss.str();
        m_color_transfer_function[i].ColorVar        = "q1";
        m_color_transfer_function[i].m_range_initialized = false;
    }
    for ( size_t i = 0; i < m_opacity_transfer_function.size(); i++ )
    {
        std::stringstream ss;
        ss << "O" << i + 1;
        m_opacity_transfer_function[i] = NamedTransferFunctionParameter();
        m_opacity_transfer_function[i].Name       = ss.str();
        m_opacity_transfer_function[i].OpacityVar   = "q1";
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
        std::cout << "m_color_transfer_function[" << i << "].m_name          " << m_color_transfer_function[i].Name << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_color_variable      " << m_color_transfer_function[i].ColorVar  << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_color_variable_min   " << m_color_transfer_function[i].ColorVarMin << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_color_variable_max   " << m_color_transfer_function[i].ColorVarMax << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_equationR     " << m_color_transfer_function[i].EquationR << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_equationG     " << m_color_transfer_function[i].EquationG << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_equationB     " << m_color_transfer_function[i].EquationB << std::endl;
        std::cout << "m_color_transfer_function[" << i << "].m_equationA     " << m_color_transfer_function[i].EquationA << std::endl;
    }

    for ( size_t i = 0; i < m_opacity_transfer_function.size(); i++ )
    {
        std::cout << "m_opacity_transfer_function[" << i << "].m_name          " << m_opacity_transfer_function[i].Name << std::endl;
        std::cout << "m_opacity_transfer_function[" << i << "].m_opacity_variable    " << m_opacity_transfer_function[i].OpacityVar  << std::endl;
        std::cout << "m_opacity_transfer_function[" << i << "].m_opacity_variable_min " << m_opacity_transfer_function[i].OpacityVarMin << std::endl;
        std::cout << "m_opacity_transfer_function[" << i << "].m_opacity_variable_max " << m_opacity_transfer_function[i].OpacityVarMax << std::endl;
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
    message->transfunc.clear();
    message->voleqn.clear();
    message->opacity_var.clear();
    message->color_var.clear();

    // 1次伝達関数
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
        etf.ColorVar   = ss.str() + "_VAR_C";
        etf.OpacityVar = "";
        veq.Name     = etf.ColorVar;
        veq.Equation = this->m_color_transfer_function[i].ColorVar;
        message->transfunc.push_back( etf );
        message->voleqn.push_back( veq );

        message->color_var.push_back( this->convertToken( this->m_color_transfer_function[i].ColorVar ) );
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

        etf.ColorVar   = ss.str() + "_VAR_C";
        etf.OpacityVar = ss.str() + "_VAR_O";
        veq_c.Name     = etf.ColorVar;
        veq_o.Name     = etf.OpacityVar;
        veq_c.Equation = this->m_color_transfer_function[i].ColorVar;
        veq_o.Equation = this->m_opacity_transfer_function[i].OpacityVar;

        std::replace(etf.Name.begin(), etf.Name.end(), 'C', 't');
        etf.setOpacityMap( o_tf.opacityMap() );
        etf.EquationA = o_tf.EquationA;
        etf.OpacityVarMin = o_tf.OpacityVarMin;
        etf.OpacityVarMax = o_tf.OpacityVarMax;

        message->transfunc.push_back( etf );
        message->voleqn.push_back( veq_c );
        message->voleqn.push_back( veq_o );

        message->color_var.push_back( this->convertToken( this->m_color_transfer_function[i].ColorVar ) );
        message->opacity_var.push_back( this->convertToken( this->m_opacity_transfer_function[i].OpacityVar ) );
    }

#endif

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
        trans->Name       = name;
        trans->ColorVar   = variable;
        return;
    }

    // 新規に追加する
    NamedTransferFunctionParameter transfer_function;
    if (this->m_color_transfer_function.size() > 0) {
        transfer_function = NamedTransferFunctionParameter(this->m_color_transfer_function[0]);
    }
    transfer_function.Name       = name;
    transfer_function.ColorVar   = variable;
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
        trans->Name       = name;
        trans->OpacityVar   = variable;
        return;
    }

    // 新規に追加する
    NamedTransferFunctionParameter transfer_function;
    if (this->m_color_transfer_function.size() > 0) {
        transfer_function = NamedTransferFunctionParameter(this->m_opacity_transfer_function[0]);
    }
    transfer_function.Name       = name;
    transfer_function.OpacityVar   = variable;
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
    std::vector<NamedTransferFunctionParameter>::iterator itr;
    for (itr = this->m_color_transfer_function.begin(); itr != this->m_color_transfer_function.end(); itr++) {
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
NamedTransferFunctionParameter* ExtendedTransferFunctionMessage::getColorTransferFunction(const int n)
{
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
        if (trans.Name == name) {
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

    trans->ColorVarMin = min;
    trans->ColorVarMax = max;

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
    if (trans == NULL) return;

    trans->OpacityVarMin = min;
    trans->OpacityVarMax = max;

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
    transfer_function.Name       = name;
    transfer_function.ColorVar   = variable;
    // 範囲は[0]からコピーする
    transfer_function.Resolution = this->m_color_transfer_function[0].Resolution;
    transfer_function.ColorVarMin = this->m_color_transfer_function[0].ColorVarMin;
    transfer_function.ColorVarMax = this->m_color_transfer_function[0].ColorVarMax;
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
    transfer_function.Name       = name;
    transfer_function.OpacityVar   = variable;
    // 範囲は[0]からコピーする
    transfer_function.Resolution = this->m_opacity_transfer_function[0].Resolution;
    transfer_function.OpacityVarMin = this->m_opacity_transfer_function[0].OpacityVarMin;
    transfer_function.OpacityVarMax = this->m_opacity_transfer_function[0].OpacityVarMax;
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
        if (itr->Name == name) {
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
        if (itr->Name == name) {
            this->m_opacity_transfer_function.erase(itr);
            break;
        }
    }

    return;
}

void ExtendedTransferFunctionMessage::importFromServerMessage( const jpv::ParticleTransferServerMessage& message )
{
    this->m_extend_transfer_function_resolution = 256;

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

        std::string* nameBuf = &this->m_color_transfer_function[i].Name;
        std::replace(nameBuf->begin(), nameBuf->end(), 't', 'C');
        nameBuf = &this->m_opacity_transfer_function[i].Name;
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
        std::cout<< m_color_transfer_function[i].Name<<","<< m_color_transfer_function[i].ColorVar<<","<< m_color_transfer_function[i].ColorVarMin<<","<< m_color_transfer_function[i].ColorVarMax<<std::endl;
    }
	std::cout << "Opacity" << std::endl;
	std::cout << "transferFunction.Name,OpacityVar,OpacityVarMin,OpacityVarMax" << std::endl;
	for (int i = 0; i < m_opacity_transfer_function.size(); i++) {
		std::cout << m_opacity_transfer_function[i].Name << "," << m_opacity_transfer_function[i].OpacityVar << "," << m_opacity_transfer_function[i].OpacityVarMin << "," << m_opacity_transfer_function[i].OpacityVarMax << std::endl;
	}

}

