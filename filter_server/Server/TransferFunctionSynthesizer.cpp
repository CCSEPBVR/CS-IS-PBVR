
#include "TransferFunctionSynthesizer.h"
#include <fstream>
#include <limits>
#include <sstream>

#include <cstdlib>

#if (defined(KVS_PLATFORM_LINUX) || defined(KVS_PLATFORM_MACOSX))
#include <execinfo.h>
#endif

using FuncParser::Variable;
using FuncParser::Variables;
using FuncParser::Function;
using FuncParser::FunctionParser;

static const std::string str_coordX( "X" );
static const std::string str_coordY( "Y" );
static const std::string str_coordZ( "Z" );

/****  delete by @hira at 2016/12/01
TransferFunctionSynthesizer::TransferFunctionSynthesizer()
    : m_synthesozer_function_string( "t1" )
{
    m_transfer_function_map_color["t1"] = pbvr::TransferFunction();
    m_transfer_function_map_opacity["t1"] = pbvr::TransferFunction();
    m_transfer_function_quantity_map_color["t1"] = "q1";
    m_transfer_function_quantity_map_opacity["t1"] = "q1";
    m_quantity_map["q1"]    = 0;

    this->create();
}
*/

/**
 * コンストラクタ
 */
TransferFunctionSynthesizer::TransferFunctionSynthesizer()
{
    this->create();
}

/**
 * コピーコンストラクタ
 * @param s
 */
TransferFunctionSynthesizer::TransferFunctionSynthesizer( const TransferFunctionSynthesizer& s )
{
    if ( &s )
    {
        copy( s );
    }
    else
    {
        TransferFunctionSynthesizer();
    }
}

TransferFunctionSynthesizer::~TransferFunctionSynthesizer()
{
}

const TransferFunctionMap& TransferFunctionSynthesizer::colorTransferFunctionMap() const
{
    return m_transfer_function_map_color;
}

const TransferFunctionMap& TransferFunctionSynthesizer::opacityTransferFunctionMap() const
{
    return m_transfer_function_map_opacity;
}

const TransfuncQuantityBindMap& TransferFunctionSynthesizer::colorTransferFunctionBindMap() const
{
    return m_transfer_function_quantity_map_color;
}

const TransfuncQuantityBindMap& TransferFunctionSynthesizer::opacityTransferFunctionBindMap() const
{
    return m_transfer_function_quantity_map_opacity;
}

const FunctionStringMap& TransferFunctionSynthesizer::volumeFunctionStringMap() const
{
    return m_volume_function_string_map;
}

const QuantityMap& TransferFunctionSynthesizer::quantityMap() const
{
    return m_quantity_map;
}

const FunctionMap& TransferFunctionSynthesizer::functionMap() const
{
    return m_volume_function_map;
}

/**** delete by @hira at 2016/12/01
const std::string& TransferFunctionSynthesizer::synthesizerFunctionString() const
{
    return m_synthesozer_function_string;
}
****/

// add by @hira at 2016/12/01
const std::string& TransferFunctionSynthesizer::colorTransfuncSynthesis() const
{
    return this->m_color_transfunc_synthesis;
}

// add by @hira at 2016/12/01
const std::string& TransferFunctionSynthesizer::opacityTransfuncSynthesis() const
{
    return this->m_opacity_transfunc_synthesis;
}

void TransferFunctionSynthesizer::setMaxOpacity( const float opacity )
{
    m_max_opacity = opacity;
}

void TransferFunctionSynthesizer::setMaxDensity( const float density )
{
    m_max_density = density;
}

void TransferFunctionSynthesizer::setSamplingVolumeInverse( const float sampling_volume_inverse )
{
    m_sampling_volume_inverse = sampling_volume_inverse;
}

float TransferFunctionSynthesizer::getMaxOpacity() const
{
    return m_max_opacity;
}

float TransferFunctionSynthesizer::getMaxDensity() const
{
    return m_max_density;
}

float TransferFunctionSynthesizer::getSamplingVolumeInverse() const
{
    return m_sampling_volume_inverse;
}

VariableRange& TransferFunctionSynthesizer::variableRange()
{
    return m_variable_range;
}

const VariableRange& TransferFunctionSynthesizer::variableRange() const
{
    return m_variable_range;
}

void TransferFunctionSynthesizer::mergeVarRange( const TransferFunctionSynthesizer& tfs )
{
    m_variable_range.merge( tfs.m_variable_range );
}

void TransferFunctionSynthesizer::copy( const TransferFunctionSynthesizer& s )
{
    this->setColorTransferFunctionMap( s.colorTransferFunctionMap() );
    this->setOpacityTransferFunctionMap( s.opacityTransferFunctionMap() );
    this->setColorTransfuncQuantityBindMap( s.colorTransferFunctionBindMap() );
    this->setOpacityTransfuncQuantityBindMap( s.opacityTransferFunctionBindMap() );
    this->setVolumeFunctionStringMap( s.volumeFunctionStringMap() );
    this->setQuantityMap( s.quantityMap() );
    // delete by @hira at 2016/12/01
    // this->setSynthFunctionString( s.synthesizerFunctionString() );
    // add by @hira at 2016/12/01
    this->setColorSynthFunctionString( s.colorTransfuncSynthesis() );
    this->setOpacitySynthFunctionString( s.opacityTransfuncSynthesis() );
    this->create();

    this->setMaxOpacity( s.getMaxOpacity() );
    this->setMaxDensity( s.getMaxDensity() );
    this->setSamplingVolumeInverse( s.getSamplingVolumeInverse() );

    m_variable_range = s.m_variable_range;
}

TransferFunctionSynthesizer& TransferFunctionSynthesizer::operator= ( const TransferFunctionSynthesizer& s )
{
    copy( s );
    return *this;
}

void TransferFunctionSynthesizer::setValue( const std::string& nm, const float value )
{
    VariableMap::const_iterator i = m_variable_map.find( nm );
    if ( i != m_variable_map.end() )
    {
        m_variable_map[nm] = value;
    }
}

float TransferFunctionSynthesizer::getValue( const std::string& nm )
{
    VariableMap::const_iterator i = m_variable_map.find( nm );
    float ret = std::numeric_limits<float>::quiet_NaN();
    if ( i != m_variable_map.end() )
    {
        ret = i->second.eval();
    }
    return ret;
}

void TransferFunctionSynthesizer::setTransferFunctionMap( const TransferFunctionMap& ptf_map )
{
    setColorTransferFunctionMap( ptf_map );
    setOpacityTransferFunctionMap( ptf_map );
}

void TransferFunctionSynthesizer::setColorTransferFunctionMap( const TransferFunctionMap& ptf_map )
{
    for ( TransferFunctionMap::const_iterator i = ptf_map.begin(); i != ptf_map.end(); i++ )
    {
        m_transfer_function_map_color[i->first] = pbvr::TransferFunction( i->second );
    }
}

void TransferFunctionSynthesizer::setOpacityTransferFunctionMap( const TransferFunctionMap& ptf_map )
{
    for ( TransferFunctionMap::const_iterator i = ptf_map.begin(); i != ptf_map.end(); i++ )
    {
        m_transfer_function_map_opacity[i->first] = pbvr::TransferFunction( i->second );
    }
}

void TransferFunctionSynthesizer::setTransfuncQuantityBindMap( const TransfuncQuantityBindMap& tfq_map )
{
    setColorTransfuncQuantityBindMap( tfq_map );
    setOpacityTransfuncQuantityBindMap( tfq_map );
}

void TransferFunctionSynthesizer::setColorTransfuncQuantityBindMap( const TransfuncQuantityBindMap& tfq_map )
{
    for ( TransfuncQuantityBindMap::const_iterator i = tfq_map.begin(); i != tfq_map.end(); i++ )
    {
        m_transfer_function_quantity_map_color[i->first] = i->second;
    }
}

void TransferFunctionSynthesizer::setOpacityTransfuncQuantityBindMap( const TransfuncQuantityBindMap& tfq_map )
{
    for ( TransfuncQuantityBindMap::const_iterator i = tfq_map.begin(); i != tfq_map.end(); i++ )
    {
        m_transfer_function_quantity_map_opacity[i->first] = i->second;
    }
}

void TransferFunctionSynthesizer::setVolumeFunctionStringMap( const FunctionStringMap& volfuncstr_map )
{
    const std::string sp( " \t" );
    for ( FunctionStringMap::const_iterator i = volfuncstr_map.begin(); i != volfuncstr_map.end(); i++ )
    {
        std::string fs( i->second );
        size_t s = std::string::npos;
        for ( std::string::const_iterator c = sp.begin(); c != sp.end(); c++ )
        {
            while ( ( s = fs.find( *c, 0 ) ) != std::string::npos )
            {
                fs.replace( s, 1, "" );
            }
        }
        m_volume_function_string_map[i->first] = fs;
    }
}

void TransferFunctionSynthesizer::setQuantityMap( const QuantityMap& qmap )
{
    for ( QuantityMap::const_iterator i = qmap.begin(); i != qmap.end(); i++ )
    {
        m_quantity_map[i->first] = i->second;
    }
}


/****  delete by @hira at 2016/12/01
void TransferFunctionSynthesizer::setSynthFunctionString( const std::string& synth_func_str )
{
    m_synthesozer_function_string = synth_func_str;
}
****/

// add by @hira at 2016/12/01
void TransferFunctionSynthesizer::setColorSynthFunctionString( const std::string& synth_func_str )
{
    m_color_transfunc_synthesis = synth_func_str;
}

// add by @hira at 2016/12/01
void TransferFunctionSynthesizer::setOpacitySynthFunctionString( const std::string& synth_func_str )
{
    m_opacity_transfunc_synthesis = synth_func_str;
}

void TransferFunctionSynthesizer::create()
{
    // clear containers
    m_variables.clear();
    m_variable_map.clear();
    m_volume_function_map.clear();

    m_variable_range.clear();

    // Variables for X,Y,Z coord
    add_variant( str_coordX );
    add_variant( str_coordY );
    add_variant( str_coordZ );

    // Vairables for quantities
    for ( QuantityMap::iterator q = m_quantity_map.begin(); q != m_quantity_map.end(); q++ )
    {
        const std::string& nm = q->first;
        add_variant( nm );
    }

    // variables which have evaluation value of primitive transfer function
    for ( TransferFunctionMap::iterator i = m_transfer_function_map_color.begin(); i != m_transfer_function_map_color.end(); i++ )
    {
        const std::string& nm = i->first;
        add_variant( nm );
    }
    for ( TransferFunctionMap::iterator i = m_transfer_function_map_opacity.begin(); i != m_transfer_function_map_opacity.end(); i++ )
    {
        const std::string& nm = i->first;
        add_variant( nm );
    }

    // initialize and parse volume shape functions
    initialize_volume_function();
    register_volume_function();

    // initialize synthesized transfer function
    // delete by @hira at 2016/12/01
    // FunctionParser synth_func_parse( m_synthesozer_function_string, m_synthesozer_function_string.size() + 1 );
    // FunctionParser::Error err = synth_func_parse.express( m_synthesizer_function, m_variables );

    // add by @hira at 2016/12/01
    FunctionParser color_synth_func_parse( this->m_color_transfunc_synthesis, m_color_transfunc_synthesis.size() + 1 );
    FunctionParser::Error color_err = color_synth_func_parse.express( m_color_synthesizer_function, m_variables );
    FunctionParser opacity_synth_func_parse( this->m_opacity_transfunc_synthesis, m_opacity_transfunc_synthesis.size() + 1 );
    FunctionParser::Error opacity_err = opacity_synth_func_parse.express( m_opacity_synthesizer_function, m_variables );

    return;
}

void TransferFunctionSynthesizer::optimize()
{
    TransfuncQuantityBindMap new_tfq_c, new_tfq_o;
    FunctionStringMap new_volfuncstr_map;

    // volume functions
    for ( TransfuncQuantityBindMap::iterator b = m_transfer_function_quantity_map_color.begin(); b != m_transfer_function_quantity_map_color.end(); b++ )
    {
        const std::string& fn = b->first;
        const std::string& qn = b->second;
        // add by @hira at 2016/12/01 : キーが存在するかチェックする
        if (m_volume_function_string_map.find(qn) == m_volume_function_string_map.end() ){
            continue;
        }
        const std::string& qs = m_volume_function_string_map[qn];
        for ( FunctionStringMap::iterator f = m_volume_function_string_map.begin(); f != m_volume_function_string_map.end(); f++ )
        {
            // modify by @hira at 2016/12/01
            // const std::string& fs = f->second;
            // if ( fs == qs )
            const std::string& ff = f->first;
            const std::string& fs = f->second;
            if ( fs == qs && ff == qn)
            {
                new_tfq_c[fn] = f->first;
                break;
            }
        }
    }

    for ( TransfuncQuantityBindMap::iterator b = m_transfer_function_quantity_map_opacity.begin(); b != m_transfer_function_quantity_map_opacity.end(); b++ )
    {
        const std::string& fn = b->first;
        const std::string& qn = b->second;
        // add by @hira at 2016/12/01 : キーが存在するかチェックする
        if (m_volume_function_string_map.find(qn) == m_volume_function_string_map.end() ){
            continue;
        }
        const std::string& qs = m_volume_function_string_map[qn];
        for ( FunctionStringMap::iterator f = m_volume_function_string_map.begin(); f != m_volume_function_string_map.end(); f++ )
        {
            // modify by @hira at 2016/12/01
            // const std::string& fs = f->second;
            // if ( fs == qs )
            const std::string& ff = f->first;
            const std::string& fs = f->second;
            if ( fs == qs && ff == qn)
            {
                new_tfq_o[fn] = f->first;
                break;
            }
        }
    }

    for ( FunctionStringMap::iterator f = m_volume_function_string_map.begin(); f != m_volume_function_string_map.end(); f++ )
    {
        const std::string& fn = f->first;
        const std::string& fs = f->second;
        for ( TransfuncQuantityBindMap::iterator b = new_tfq_c.begin(); b != new_tfq_c.end(); b++ )
        {
            if ( b->second == fn )
            {
                new_volfuncstr_map[fn] = fs;
                break;
            }
        }
        for ( TransfuncQuantityBindMap::iterator b = new_tfq_o.begin(); b != new_tfq_o.end(); b++ )
        {
            if ( b->second == fn )
            {
                new_volfuncstr_map[fn] = fs;
                break;
            }
        }
    }

    m_transfer_function_quantity_map_color.clear();
    m_transfer_function_quantity_map_opacity.clear();
    m_volume_function_string_map.clear();

    setColorTransfuncQuantityBindMap( new_tfq_c );
    setOpacityTransfuncQuantityBindMap( new_tfq_o );
    setVolumeFunctionStringMap( new_volfuncstr_map );
}

void TransferFunctionSynthesizer::optimize2()
{
    std::vector<std::string> v;

    // modify by @hira at 2016/12/01 : 1次伝達関数をパースした変数と比較(==)とする。findは使用しない( t15.find(t1)が一致する為 )
    /*******
    const std::string& fs = m_synthesizer_function.str();
    for ( TransferFunctionMap::iterator t = m_transfer_function_map_color.begin(); t != m_transfer_function_map_color.end(); t++ )
    {
        const std::string& fn = t->first;
        if ( fs.find( fn , 0 ) == std::string::npos )
        {
            v.push_back( fn );
        }
    }
    **********/

    /**** delete by @hira at 2016/12/01
    for ( TransferFunctionMap::iterator t = m_transfer_function_map_color.begin(); t != m_transfer_function_map_color.end(); t++ )
    {
        const std::string& fn = t->first;
        bool find = this->m_synthesizer_function.find(fn.c_str());
        if (!find) {
            v.push_back( fn );
        }
    }
    for ( size_t i = 0; i < v.size(); i++ )
    {
        const std::string& fn = v[i];
        m_transfer_function_map_color.erase( fn );
        m_transfer_function_map_opacity.erase( fn );
        m_transfer_function_quantity_map_color.erase( fn );
        m_transfer_function_quantity_map_opacity.erase( fn );
    }
    ****/

    {
        std::vector<std::string> rm;
        for ( TransferFunctionMap::iterator t = m_transfer_function_map_color.begin(); t != m_transfer_function_map_color.end(); t++ )
        {
            const std::string& fn = t->first;
            bool find = this->m_color_synthesizer_function.find(fn.c_str());
            if (!find) {
                rm.push_back( fn );
            }
        }
        for ( size_t i = 0; i < rm.size(); i++ )
        {
            const std::string& fn = rm[i];
            m_transfer_function_map_color.erase( fn );
            m_transfer_function_quantity_map_color.erase( fn );
        }
    }

    {
        std::vector<std::string> rm;
        for ( TransferFunctionMap::iterator t = m_transfer_function_map_opacity.begin(); t != m_transfer_function_map_opacity.end(); t++ )
        {
            const std::string& fn = t->first;
            bool find = this->m_opacity_synthesizer_function.find(fn.c_str());
            if (!find) {
                rm.push_back( fn );
            }
        }
        for ( size_t i = 0; i < rm.size(); i++ )
        {
            const std::string& fn = rm[i];
            m_transfer_function_map_opacity.erase( fn );
            m_transfer_function_quantity_map_opacity.erase( fn );
        }
    }

// #ifdef _DEBUG		// debug by @hira
    printf("[%s:%d] :: m_color_transfunc_synthesis=%s, m_transfer_function_map_color.size=%zd \n", __FILE__, __LINE__,
            m_color_transfunc_synthesis.c_str(), m_transfer_function_map_color.size());
    printf("[%s:%d] :: m_opacity_transfunc_synthesis=%s, m_transfer_function_map_opacity.size=%zd \n", __FILE__, __LINE__,
            m_opacity_transfunc_synthesis.c_str(), m_transfer_function_map_opacity.size());
// #endif

    return;
}

void TransferFunctionSynthesizer::clear()
{
    m_variables.clear();
    m_variable_map.clear();
    m_volume_function_map.clear();

    m_variable_range.clear();

    m_transfer_function_map_color.clear();
    m_transfer_function_map_opacity.clear();
    m_transfer_function_quantity_map_color.clear();
    m_transfer_function_quantity_map_opacity.clear();
    m_volume_function_string_map.clear();
    m_quantity_map.clear();

    m_time_step_vector.clear();
    // delete by @hira at 2016/12/01
    // m_synthesozer_function_string.clear();

    // add by @hira at 2016/12/01
    m_color_transfunc_synthesis.clear();
    m_opacity_transfunc_synthesis.clear();
}

//kawamura
void TransferFunctionSynthesizer::multivariateMinMaxValues(
    pbvr::CellBase<kvs::Real32>* cell, const size_t cell_index, const size_t nnodes )
{
    //セル頂点について一つづつ最大最小値を計算.
    for ( size_t i = 0; i < nnodes; i++ )
    {
        // qは変量 q = q(X,Y,Z)
        // fは変量の合成式 f = f(X,Y,Z,q1,..,qn)
        // tは１次元伝達関数 t = t(f).

        // 頂点の X,Y,Zをセット
        m_variable_map[str_coordX] = cell->vertices()[i].x();
        m_variable_map[str_coordY] = cell->vertices()[i].y();
        m_variable_map[str_coordZ] = cell->vertices()[i].z();

        // 頂点のq1,q2,..,qnをセット.
        for ( QuantityMap::const_iterator q = m_quantity_map.begin(); q != m_quantity_map.end(); q++ )
        {
            const std::string& qname = q->first;//qの変量名.
            const size_t cid = q->second;//変量qのコンポーネントID
            cell->bindCell( cell_index, cid );//コンポーネントIDを指定してセルをバインド.
            m_variable_map[qname] = cell->scalars()[i];//コンポーネントIDに対応する頂点の変量qをセット.
        }

        // 頂点のX,Y,Z,q1,q2,..qnからfを計算.
        for ( FunctionMap::const_iterator i = m_volume_function_map.begin(); i != m_volume_function_map.end(); i++ )
        {
            const std::string& fname = i->first;//fの変量名.
            const Function& f = i->second;//変量f
            //どこかでexpressコマンドを走らせているならevalコマンドでfの値が得られる
            m_variable_map[fname] = f.eval();
        }

        //不透明度関数に対応したfの値を最大最小値計算メソッドに放り込む
        for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_opacity.begin(); t != m_transfer_function_map_opacity.end(); t++ )
        {
            const std::string& tname = t->first;//１次元伝達関数tの名前
            if (m_transfer_function_quantity_map_opacity.find(tname) == m_transfer_function_quantity_map_opacity.end() ){
                continue;
            }
            const std::string& fname = m_transfer_function_quantity_map_opacity.find( tname )->second;//１次元伝達関数tの引数fの名前
            const float f_value = m_variable_map[fname].eval();//fの値
            const std::string tag = tname + "_var_o";
            m_variable_range.setValue( tag, f_value );
        }

        //色関数に対応したfの値を最大最小値計算メソッドに放り込む
        for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_color.begin(); t != m_transfer_function_map_color.end(); t++ )
        {
            const std::string& tname = t->first;//１次元伝達関数tの名前
            if (m_transfer_function_quantity_map_color.find(tname) == m_transfer_function_quantity_map_color.end() ){
                continue;
            }
            const std::string& fname = m_transfer_function_quantity_map_color.find( tname )->second;//１次元伝達関数tの引数fの名前
            const float f_value = m_variable_map[fname].eval();//fの値
            const std::string tag = tname + "_var_c";
            m_variable_range.setValue( tag, f_value );
        }
    }
}

//kawamura
void TransferFunctionSynthesizer::multivariateMinMaxAveragedValues(
    pbvr::CellBase<kvs::Real32>* cell, const size_t cell_index, const size_t nnodes )
{
    //セル頂点について一つづつ最大最小値を計算.
    // qは変量 q = q(X,Y,Z)
    // fは変量の合成式 f = f(X,Y,Z,q1,..,qn)
    // tは１次元伝達関数 t = t(f).

    // 頂点の X,Y,Zをセット
    m_variable_map[str_coordX] = cell->vertices()[0].x();
    m_variable_map[str_coordY] = cell->vertices()[0].y();
    m_variable_map[str_coordZ] = cell->vertices()[0].z();

    //不透明度関数に対応したfの値を最大最小値計算メソッドに放り込む
    for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_opacity.begin(); t != m_transfer_function_map_opacity.end(); t++ )
    {
        const std::string& tname = t->first;//１次元伝達関数tの名前
        if (m_transfer_function_quantity_map_opacity.find(tname) == m_transfer_function_quantity_map_opacity.end() ){
            continue;
        }
        const std::string& fname = m_transfer_function_quantity_map_opacity.find( tname )->second;//１次元伝達関数tの引数fの名前
        const float f_value = m_variable_map[fname].eval();//fの値
        const std::string tag = tname + "_var_o";
        m_variable_range.setValue( tag, f_value );
    }

    //色関数に対応したfの値を最大最小値計算メソッドに放り込む
    for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_color.begin(); t != m_transfer_function_map_color.end(); t++ )
    {
        const std::string& tname = t->first;//１次元伝達関数tの名前
        if (m_transfer_function_quantity_map_color.find(tname) == m_transfer_function_quantity_map_color.end() ){
            continue;
        }
        const std::string& fname = m_transfer_function_quantity_map_color.find( tname )->second;//１次元伝達関数tの引数fの名前
        const float f_value = m_variable_map[fname].eval();//fの値
        const std::string tag = tname + "_var_c";
        m_variable_range.setValue( tag, f_value );
    }
}


float TransferFunctionSynthesizer::calculateOpacity(
    pbvr::CellBase<kvs::Real32>* cell,
    const size_t n,
    const kvs::Vector3f& coord )
{
    // qは変量 q = q(X,Y,Z)
    // fは変量の合成式 f = f(X,Y,Z,q1,..,qn)
    // tは１次元伝達関数 t = t(f).

    // set X,Y,Z
    m_variable_map[str_coordX] = coord.x();
    m_variable_map[str_coordY] = coord.y();
    m_variable_map[str_coordZ] = coord.z();

    // X,Y,Zからq1,q2,..,qnを計算.
    for ( QuantityMap::const_iterator q = m_quantity_map.begin(); q != m_quantity_map.end(); q++ )
    {
        const std::string& nm = q->first;
        const size_t qi = q->second;
        cell->bindCell( n, qi );

        m_variable_map[nm] = cell->scalar();
    }

    // pull t1,t2,...,t5 with X,Y,Z,q1,q2,...,qn,f1,f2,...,fn
    for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_opacity.begin(); t != m_transfer_function_map_opacity.end(); t++ )
    {
        const std::string& nm = t->first;//１次元伝達関数tの名前
        const pbvr::TransferFunction& f = t->second;//１次元伝達関数t

        // physical quantity of the cell
        if (m_transfer_function_quantity_map_opacity.find( nm ) == m_transfer_function_quantity_map_opacity.end()) {
            continue;
        }
        const std::string& qnm = m_transfer_function_quantity_map_opacity.find( nm )->second;//１次元伝達関数tの引数fの名前
        const float scalar = m_volume_function_map[qnm].eval();//fの値
        AssertValid( scalar, __FILE__, __LINE__ );

        // set value to stab variable
        if ( scalar < f.minValue() )
        {
            m_variable_map[nm] = f.opacityMap().at( f.minValue() );
        }
        else if ( f.maxValue() < scalar )
        {
            m_variable_map[nm] = f.opacityMap().at( f.maxValue() );
        }
        else
        {
            m_variable_map[nm] = f.opacityMap().at( scalar );
        }
    }

    // eval synthesized transfer function
    // delete by @hira at 2016/12/01
    // float opacity = m_synthesizer_function.eval();
    float opacity = this->m_opacity_synthesizer_function.eval();

    AssertValid( opacity, __FILE__, __LINE__ );
    if ( opacity > 1.0 ) opacity = 1.0;
    if ( opacity < 0.0 ) opacity = 0.0;

    return opacity;
}


float TransferFunctionSynthesizer::calculateAveragedOpacityBatch(
    pbvr::CellBase<kvs::Real32>* cell,
    const size_t n,
    const kvs::Vector3f& coord )
{
    m_variable_map[str_coordX] = coord.x();
    m_variable_map[str_coordY] = coord.y();
    m_variable_map[str_coordZ] = coord.z();

    for ( QuantityMap::const_iterator q = m_quantity_map.begin(); q != m_quantity_map.end(); q++ )
    {
        const std::string& nm = q->first;
        const size_t qi = q->second;
        cell->bindCell( n, qi );

        m_variable_map[nm] = cell->averagedScalar();
    }

    // pull t1,t2,...,t5 with X,Y,Z,q1,q2,...,qn,f1,f2,...,fn
    for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_opacity.begin(); t != m_transfer_function_map_opacity.end(); t++ )
    {
        const std::string& nm = t->first;
        const pbvr::TransferFunction& f = t->second;

        // physical quantity of the cell
        if (m_transfer_function_quantity_map_opacity.find( nm ) == m_transfer_function_quantity_map_opacity.end()) {
            continue;
        }
        const std::string& qnm = m_transfer_function_quantity_map_opacity.find( nm )->second;
        const float scalar = m_volume_function_map[qnm].eval();
        AssertValid( scalar, __FILE__, __LINE__ );

        // set value to stab variable
        if ( scalar < f.minValue() )
        {
            m_variable_map[nm] = f.opacityMap().at( f.minValue() );
        }
        else if ( f.maxValue() < scalar )
        {
            m_variable_map[nm] = f.opacityMap().at( f.maxValue() );
        }
        else
        {
            m_variable_map[nm] = f.opacityMap().at( scalar );
        }
    }

    // eval synthesized transfer function
    // modify by @hira at 2016/12/01
    // float opacity = m_synthesizer_function.eval();
    float opacity = this->m_opacity_synthesizer_function.eval();
    AssertValid( opacity, __FILE__, __LINE__ );
    if ( opacity > 1.0 ) opacity = 1.0;
    if ( opacity < 0.0 ) opacity = 0.0;

    return opacity;
}

float TransferFunctionSynthesizer::calculateAveragedOpacity(
    pbvr::CellBase<kvs::Real32>* cell,
    const size_t n,
    const kvs::Vector3f& coord )
{
    m_variable_map[str_coordX] = coord.x();
    m_variable_map[str_coordY] = coord.y();
    m_variable_map[str_coordZ] = coord.z();

    for ( QuantityMap::const_iterator q = m_quantity_map.begin(); q != m_quantity_map.end(); q++ )
    {
        const std::string& nm = q->first;
        const size_t qi = q->second;
        cell->bindCell( n, qi );

        m_variable_map[nm] = cell->averagedScalar();
    }

#if 0
    for ( FunctionMap::const_iterator i = m_volume_function_map.begin(); i != m_volume_function_map.end(); i++ )
    {
        const std::string& nm = i->first;
        const Function& f = i->second;

        // set evaluation value to variabe
        m_variable_map[nm] = f.eval();
    }
#endif

    // pull t1,t2,...,t5 with X,Y,Z,q1,q2,...,qn,f1,f2,...,fn
    for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_opacity.begin(); t != m_transfer_function_map_opacity.end(); t++ )
    {
        const std::string& nm = t->first;
        const pbvr::TransferFunction& f = t->second;

        // physical quantity of the cell
        if (m_transfer_function_quantity_map_opacity.find( nm ) == m_transfer_function_quantity_map_opacity.end()) {
            continue;
        }
        const std::string& qnm = m_transfer_function_quantity_map_opacity.find( nm )->second;
        const float scalar = m_volume_function_map[qnm].eval();
        const std::string tag = nm + "_var_o";
        AssertValid( scalar, __FILE__, __LINE__ );
        m_variable_range.setValue( tag, scalar );
        m_variable_map[tag] = scalar;

        // set value to stab variable
        if ( scalar < f.minValue() )
        {
            m_variable_map[nm] = f.opacityMap().at( f.minValue() );
        }
        else if ( f.maxValue() < scalar )
        {
            m_variable_map[nm] = f.opacityMap().at( f.maxValue() );
        }
        else
        {
            m_variable_map[nm] = f.opacityMap().at( scalar );
        }
    }

    // eval synthesized transfer function
    // modify by @hira at 2016/12/01
    // float opacity = m_synthesizer_function.eval();
    float opacity = m_opacity_synthesizer_function.eval();
    AssertValid( opacity, __FILE__, __LINE__ );
    if ( opacity > 1.0 ) opacity = 1.0;
    if ( opacity < 0.0 ) opacity = 0.0;

    return opacity;
}

void TransferFunctionSynthesizer::calculateOpacityOfNode(
    pbvr::CellBase<kvs::Real32>* cell,
    pbvr::CellBase<kvs::Real32>* cell_opacity,
    const size_t n )
{
    for ( size_t node = 0; node < cell_opacity->numberOfNodes(); node++ )
    {
        m_variable_map[str_coordX] = cell->vertices()[node].x();
        m_variable_map[str_coordY] = cell->vertices()[node].y();
        m_variable_map[str_coordZ] = cell->vertices()[node].z();

        for ( QuantityMap::const_iterator q = m_quantity_map.begin(); q != m_quantity_map.end(); q++ )
        {
            const std::string& nm = q->first;
            const size_t qi = q->second;
            cell->bindCell( n, qi );

            m_variable_map[nm] = cell->scalars()[node];
        }

        // pull t1,t2,...,t5 with X,Y,Z,q1,q2,...,qn,f1,f2,...,fn
        for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_opacity.begin(); t != m_transfer_function_map_opacity.end(); t++ )
        {
            const std::string& nm = t->first;
            const pbvr::TransferFunction& f = t->second;

            // physical quantity of the cell
            if (m_transfer_function_quantity_map_opacity.find( nm ) == m_transfer_function_quantity_map_opacity.end()) {
                continue;
            }
            const std::string& qnm = m_transfer_function_quantity_map_opacity.find( nm )->second;

            const float scalar = m_volume_function_map[qnm].eval();
            const std::string tag = nm + "_var_o";
            AssertValid( scalar, __FILE__, __LINE__ );
            m_variable_range.setValue( tag, scalar );
            m_variable_map[tag] = scalar;

            // set value to stab variable
            if ( scalar < f.minValue() )
            {
                m_variable_map[nm] = f.opacityMap().at( f.minValue() );
            }
            else if ( f.maxValue() < scalar )
            {
                m_variable_map[nm] = f.opacityMap().at( f.maxValue() );
            }
            else
            {
                m_variable_map[nm] = f.opacityMap().at( scalar );
            }
        }

        // eval synthesized transfer function
        // modify by @hira at 2016/12/01
        // float opacity = m_synthesizer_function.eval();
        float opacity = m_opacity_synthesizer_function.eval();
        AssertValid( opacity, __FILE__, __LINE__ );
        if ( opacity > 1.0 ) opacity = 1.0;
        if ( opacity < 0.0 ) opacity = 0.0;

        cell_opacity->setScalars()[ node ] = opacity;
    }
}

/**
 * 未使用
 * @param cell
 * @param n
 * @param coord
 * @return
 */
float TransferFunctionSynthesizer::calculateGravityPointOpacity(
    pbvr::CellBase<kvs::Real32>* cell,
    const size_t n,
    const kvs::Vector3f& coord )
{
    // qは変量 q = q(X,Y,Z)
    // fは変量の合成式 f = f(X,Y,Z,q1,..,qn)
    // tは１次元伝達関数 t = t(f).

    // set X,Y,Z
    m_variable_map[str_coordX] = coord.x();
    m_variable_map[str_coordY] = coord.y();
    m_variable_map[str_coordZ] = coord.z();

    // X,Y,Zからq1,q2,..,qnを計算.
    for ( QuantityMap::const_iterator q = m_quantity_map.begin(); q != m_quantity_map.end(); q++ )
    {
        const std::string& nm = q->first;
        const size_t qi = q->second;
        cell->bindCell( n, qi );

        m_variable_map[nm] = cell->localGravityPointValue();
    }

    // X,Y,Z,q1,q2,..qnからfを計算.
    for ( FunctionMap::const_iterator i = m_volume_function_map.begin(); i != m_volume_function_map.end(); i++ )
    {
        const std::string& nm = i->first;
        const Function& f = i->second;

        // set evaluation value to variabe
        m_variable_map[nm] = f.eval();
    }

    // pull t1,t2,...,t5 with X,Y,Z,q1,q2,...,qn,f1,f2,...,fn
    for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_opacity.begin(); t != m_transfer_function_map_opacity.end(); t++ )
    {
        const std::string& nm = t->first;//１次元伝達関数tの名前
        const pbvr::TransferFunction& f = t->second;//１次元伝達関数t

        // physical quantity of the cell
        if (m_transfer_function_quantity_map_opacity.find( nm ) == m_transfer_function_quantity_map_opacity.end()) {
            continue;
        }
        const std::string& qnm = m_transfer_function_quantity_map_opacity.find( nm )->second;//１次元伝達関数tの引数fの名前
        const float scalar = m_variable_map[qnm].eval();//fの値
        const std::string tag = nm + "_var_o";
        AssertValid( scalar, __FILE__, __LINE__ );
        m_variable_range.setValue( tag, scalar );
        m_variable_map[tag] = scalar;

        // set value to stab variable
        if ( scalar < f.minValue() )
        {
            m_variable_map[nm] = f.opacityMap().at( f.minValue() );
        }
        else if ( f.maxValue() < scalar )
        {
            m_variable_map[nm] = f.opacityMap().at( f.maxValue() );
        }
        else
        {
            m_variable_map[nm] = f.opacityMap().at( scalar );
        }
    }

    // eval synthesized transfer function
    // modify by @hira at 2016/12/01
    // float opacity = m_synthesizer_function.eval();
    float opacity = this->m_opacity_synthesizer_function.eval();
    AssertValid( opacity, __FILE__, __LINE__ );
    if ( opacity > 1.0 ) opacity = 1.0;
    if ( opacity < 0.0 ) opacity = 0.0;

    return opacity;
}

kvs::RGBColor TransferFunctionSynthesizer::calculateAveragedColor(
    pbvr::CellBase<kvs::Real32>* cell,
    const size_t n,
    const kvs::Vector3f& coord )
{
    kvs::UInt8 col[3];
    str_color_map c_map;

    m_variable_map[str_coordX] = coord.x();
    m_variable_map[str_coordY] = coord.y();
    m_variable_map[str_coordZ] = coord.z();

    for ( QuantityMap::const_iterator q = m_quantity_map.begin(); q != m_quantity_map.end(); q++ )
    {
        const std::string& nm = q->first;
        const size_t qi = q->second;
        cell->bindCell( n, qi );

        m_variable_map[nm] = cell->averagedScalar();
    }

#if 0
    for ( FunctionMap::const_iterator i = m_volume_function_map.begin(); i != m_volume_function_map.end(); i++ )
    {
        const std::string& nm = i->first;
        const Function& f = i->second;

        // set evaluation value to variabe
        m_variable_map[nm] = f.eval();
    }
#endif

    for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_color.begin(); t != m_transfer_function_map_color.end(); t++ )
    {
        const std::string& nm = t->first;
        const pbvr::TransferFunction& f = t->second;

        // physical quantity of the cell
        if (m_transfer_function_quantity_map_color.find( nm ) == m_transfer_function_quantity_map_color.end()) {
            continue;
        }
        const std::string& qnm = m_transfer_function_quantity_map_color.find( nm )->second;
//      const float scalar = m_variable_map[qnm].eval();
        const float scalar = m_volume_function_map[qnm].eval();
        const std::string tag = nm + "_var_c";
        AssertValid( scalar, __FILE__, __LINE__ );
        m_variable_range.setValue( tag, scalar );
        m_variable_map[tag] = scalar;

        // reference transfer function
        c_map[nm] = ( scalar < f.minValue() ) ? f.colorMap().at( f.minValue() ) :
                    ( scalar > f.maxValue() ) ? f.colorMap().at( f.maxValue() ) :
                    f.colorMap().at( scalar );
    }

    //kawamura
    float color[3];
    // eval synthesized transfer function
    for ( unsigned c = 0; c < 3; c++ ) // for each RGB element
    {
        for ( str_color_map::iterator i = c_map.begin(); i != c_map.end(); i++ )
        {
            const std::string& nm = i->first;
            const kvs::RGBColor& cv = i->second;

            float r = ( float )cv.r() / 255.0;
            float g = ( float )cv.g() / 255.0;
            float b = ( float )cv.b() / 255.0;

            m_variable_map[nm] = ( c == 0 ) ? r :
                                 ( c == 1 ) ? g :
                                 b;
            /*m_variable_map[nm] = (c == 0)? cv.r():
                            (c == 1)? cv.g():
                            cv.b();*/

        }

        // modify by @hira at 2016/12/01
        // color[c] = m_synthesizer_function.eval();
        color[c] = this->m_color_synthesizer_function.eval();
        col[c] = static_cast<kvs::UInt8>( kvs::Math::Clamp<float>( color[c] * 255.0, 0.0, 255.0 ) );
        //col[c] = m_synthesizer_function.eval();
        AssertValid( color[c], __FILE__, __LINE__ );
    }

    return kvs::RGBColor( col );
}

kvs::RGBColor TransferFunctionSynthesizer::calculateColor(
    pbvr::CellBase<kvs::Real32>* cell,
    const size_t n,
    const kvs::Vector3f& coord )
{
    kvs::UInt8 col[3];
    str_color_map c_map;

    m_variable_map[str_coordX] = coord.x();
    m_variable_map[str_coordY] = coord.y();
    m_variable_map[str_coordZ] = coord.z();

    for ( QuantityMap::const_iterator q = m_quantity_map.begin(); q != m_quantity_map.end(); q++ )
    {
        const std::string& nm = q->first;
        const size_t qi = q->second;
        cell->bindCell( n, qi );

        m_variable_map[nm] = cell->scalar();
    }

#if 0
    for ( FunctionMap::const_iterator i = m_volume_function_map.begin(); i != m_volume_function_map.end(); i++ )
    {
        const std::string& nm = i->first;
        const Function& f = i->second;

        // set evaluation value to variabe
        m_variable_map[nm] = f.eval();
    }
#endif

    for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_color.begin(); t != m_transfer_function_map_color.end(); t++ )
    {
        const std::string& nm = t->first;
        const pbvr::TransferFunction& f = t->second;

        // physical quantity of the cell
        if (m_transfer_function_quantity_map_color.find( nm ) == m_transfer_function_quantity_map_color.end()) {
            continue;
        }
        const std::string& qnm = m_transfer_function_quantity_map_color.find( nm )->second;
//      const float scalar = m_variable_map[qnm].eval();
        const float scalar = m_volume_function_map[qnm].eval();
        AssertValid( scalar, __FILE__, __LINE__ );

        // reference transfer function
        c_map[nm] = std::isnan(scalar) ? f.colorMap().at( f.minValue() ) :			// add by @hira at 2016/12/01
                    ( scalar < f.minValue() ) ? f.colorMap().at( f.minValue() ) :
                    ( scalar > f.maxValue() ) ? f.colorMap().at( f.maxValue() ) :
                    f.colorMap().at( scalar );
    }

    //kawamura
    float color[3];
    // eval synthesized transfer function
    for ( unsigned c = 0; c < 3; c++ ) // for each RGB element
    {
        for ( str_color_map::iterator i = c_map.begin(); i != c_map.end(); i++ )
        {
            const std::string& nm = i->first;
            const kvs::RGBColor& cv = i->second;

            float r = ( float )cv.r() / 255.0;
            float g = ( float )cv.g() / 255.0;
            float b = ( float )cv.b() / 255.0;

            m_variable_map[nm] = ( c == 0 ) ? r :
                                 ( c == 1 ) ? g :
                                 b;
            /*m_variable_map[nm] = (c == 0)? cv.r():
                            (c == 1)? cv.g():
                            cv.b();*/

        }

        // modify by @hira at 2016/12/01
        // color[c] = m_synthesizer_function.eval();
        color[c] = this->m_color_synthesizer_function.eval();
        col[c] = static_cast<kvs::UInt8>( kvs::Math::Clamp<float>( color[c] * 255.0, 0.0, 255.0 ) );
        //col[c] = m_synthesizer_function.eval();
        AssertValid( color[c], __FILE__, __LINE__ );
    }

    return kvs::RGBColor( col );
}

/**
 * 未使用
 * @param cell
 * @param n
 * @param coord
 * @return
 */
kvs::RGBColor TransferFunctionSynthesizer::calculateGravityPointColor(
    pbvr::CellBase<kvs::Real32>* cell,
    const size_t n,
    const kvs::Vector3f& coord )
{
    kvs::UInt8 col[3];
    str_color_map c_map;

    m_variable_map[str_coordX] = coord.x();
    m_variable_map[str_coordY] = coord.y();
    m_variable_map[str_coordZ] = coord.z();

    for ( QuantityMap::const_iterator q = m_quantity_map.begin(); q != m_quantity_map.end(); q++ )
    {
        const std::string& nm = q->first;
        const size_t qi = q->second;
        cell->bindCell( n, qi );

        m_variable_map[nm] = cell->localGravityPointValue();
    }

    for ( FunctionMap::const_iterator i = m_volume_function_map.begin(); i != m_volume_function_map.end(); i++ )
    {
        const std::string& nm = i->first;
        const Function& f = i->second;

        // set evaluation value to variabe
        m_variable_map[nm] = f.eval();
    }

    for ( TransferFunctionMap::const_iterator t = m_transfer_function_map_color.begin(); t != m_transfer_function_map_color.end(); t++ )
    {
        const std::string& nm = t->first;
        const pbvr::TransferFunction& f = t->second;

        // physical quantity of the cell
        if (m_transfer_function_quantity_map_color.find( nm ) == m_transfer_function_quantity_map_color.end()) {
            continue;
        }
        const std::string& qnm = m_transfer_function_quantity_map_color.find( nm )->second;
        const float scalar = m_variable_map[qnm].eval();
        const std::string tag = nm + "_var_c";
        AssertValid( scalar, __FILE__, __LINE__ );
        m_variable_range.setValue( tag, scalar );
        m_variable_map[tag] = scalar;

        // reference transfer function
        c_map[nm] = ( scalar < f.minValue() ) ? f.colorMap().at( f.minValue() ) :
                    ( scalar > f.maxValue() ) ? f.colorMap().at( f.maxValue() ) :
                    f.colorMap().at( scalar );
    }

    //kawamura
    float color[3];
    // eval synthesized transfer function
    for ( unsigned c = 0; c < 3; c++ ) // for each RGB element
    {
        for ( str_color_map::iterator i = c_map.begin(); i != c_map.end(); i++ )
        {
            const std::string& nm = i->first;
            const kvs::RGBColor& cv = i->second;

            float r = ( float )cv.r() / 255.0;
            float g = ( float )cv.g() / 255.0;
            float b = ( float )cv.b() / 255.0;

            m_variable_map[nm] = ( c == 0 ) ? r :
                                 ( c == 1 ) ? g :
                                 b;
            /*m_variable_map[nm] = (c == 0)? cv.r():
                            (c == 1)? cv.g():
                            cv.b();*/

        }

        // modify by @hira at 2016/12/01
        // color[c] = m_synthesizer_function.eval();
        color[c] = this->m_color_synthesizer_function.eval();
        col[c] = static_cast<kvs::UInt8>( kvs::Math::Clamp<float>( color[c] * 255.0, 0.0, 255.0 ) );
        //col[c] = m_synthesizer_function.eval();
        AssertValid( color[c], __FILE__, __LINE__ );
    }

    return kvs::RGBColor( col );
}

void TransferFunctionSynthesizer::add_variant( const std::string& nm )
{
    // キーチェック : add by @hira at 2016/12/01
    if (m_variable_map.find(nm) != m_variable_map.end()){
        // 既に存在している。
        return;
    }
    m_variable_map[nm] = Variable();
    Variable& var = m_variable_map[nm];

    m_char_pointer_vector.pushBackString( nm );

    var.tag( m_char_pointer_vector.back() );
    m_variables.push_back( var );
}

void TransferFunctionSynthesizer::initialize_volume_function()
{
    for ( FunctionStringMap::iterator i = m_volume_function_string_map.begin(); i != m_volume_function_string_map.end(); i++ )
    {
        const std::string& nm  = i->first;
        const std::string& str = i->second;
        m_volume_function_map[nm] = Function();
        FunctionParser parse( const_cast<std::string&>( str ), str.size() + 1 );

#ifdef _DEBUG
        printf("[%s:%d] nm=%s,str=%s\n", __FILE__, __LINE__,
                nm.c_str(), str.c_str());
#endif

        FunctionParser::Error err = parse.express( m_volume_function_map[nm], m_variables );
        if ( err != FunctionParser::ERR_NONE )
        {
            std::cerr << FunctionParser::error_type_to_string[err] << std::endl;
        }
    }
}

void TransferFunctionSynthesizer::register_volume_function()
{
    for ( FunctionMap::const_iterator i = m_volume_function_map.begin(); i != m_volume_function_map.end(); i++ )
    {
        const std::string& nm = i->first;
        add_variant( nm );
    }
}

void TransferFunctionSynthesizer::AssertValid( const float& v, const char* file, const int line )
{
    if ( std::isnan( v ) || std::isinf( v ) )
    {
        std::stringstream ss;
        ss << "Assertion failed: NaN/Inf detected: "
           << file << ":" << line << std::endl;
#if (defined(KVS_PLATFORM_LINUX) || defined(KVS_PLATFORM_MACOSX))
        void* stk[128];
        int sz = backtrace( stk, 128 );
        char** sstr = backtrace_symbols( stk, sz );
        for ( int i = 0; i < sz; i++ )
        {
            ss << sstr[i] << std::endl;
        }
        std::free( sstr );
#endif
        throw NumericException( ss.str() );
    }
}
