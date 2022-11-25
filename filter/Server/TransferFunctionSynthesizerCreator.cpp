
#include "TransferFunctionSynthesizerCreator.h"
#include <stdio.h>
#include <string.h>

TransferFunctionSynthesizerCreator::TransferFunctionSynthesizerCreator()
{
    // 色関数
    {
        VolumeEquation ve_c;
        NamedTransferFunction tf_c;
        tf_c.m_selection  = NamedTransferFunction::SelectTransferFunction;
        tf_c.m_name       = "C1";
        tf_c.m_color_variable   = "_C1_var_c";
        ve_c.m_name     = tf_c.m_color_variable;
        ve_c.m_equation = "q1";
        m_transfunc.push_back( tf_c );
        m_voleqn.push_back( ve_c );
        m_color_transfunc_synthesis = "C1";
    }

    // 不透明度関数
    {
        VolumeEquation ve_o;
        NamedTransferFunction tf_o;
        tf_o.m_selection  = NamedTransferFunction::SelectTransferFunction;
        tf_o.m_name       = "O1";
        tf_o.m_opacity_variable   = "_O1_var_o";
        ve_o.m_name     = tf_o.m_opacity_variable;
        ve_o.m_equation = "q1";
        m_transfunc.push_back( tf_o );
        m_voleqn.push_back( ve_o );
        m_opacity_transfunc_synthesis = "O1";
    }
/*
    NamedTransferFunction tf;
    VolumeEquation ve_c, ve_o;
    tf.m_selection  = NamedTransferFunction::SelectTransferFunction;
    tf.m_name       = "t1";
    tf.m_color_variable   = "_t1_var_c";
    tf.m_opacity_variable = "_t1_var_o";
    ve_c.m_name     = tf.m_color_variable;
    ve_o.m_name     = tf.m_opacity_variable;
    ve_c.m_equation = "q1";
    ve_o.m_equation = "q1";
    m_transfunc.push_back( tf );

    m_voleqn.push_back( ve_c );
    m_voleqn.push_back( ve_o );
    //m_transfunc_synthesis = "t1";
    m_color_transfunc_synthesis = "t1";
    m_opacity_transfunc_synthesis = "t1";
*/
}

void TransferFunctionSynthesizerCreator::setFilterInfo( const FilterInformationFile *fi )
{
    this->initQuantityMap( fi->m_number_ingredients );
}

void TransferFunctionSynthesizerCreator::initQuantityMap( const int32_t m_number_ingredients )
{
    m_qmap.clear();
    for ( int32_t i = 0; i < m_number_ingredients; i++ )
    {
        std::stringstream ss;
        ss << "q" << ( i + 1 );

        const std::string name = ss.str();
        const size_t idx = i;

        m_qmap[name] = idx;
    }
}

void TransferFunctionSynthesizerCreator::setRange( const std::string name, const float min_value, const float max_value )
{
    setColorRange( name, min_value, max_value );
    setOpacityRange( name, min_value, max_value );
}

void TransferFunctionSynthesizerCreator::setColorRange( const std::string& name, const float min_value, const float max_value )
{
    for ( size_t i = 0; i < m_transfunc.size(); i++ )
    {
        if ( m_transfunc[i].m_name == name )
        {
            m_transfunc[i].m_color_variable_min = min_value;
            m_transfunc[i].m_color_variable_max = max_value;
        }
    }
}

void TransferFunctionSynthesizerCreator::setOpacityRange( const std::string& name, const float min_value, const float max_value )
{
    for ( size_t i = 0; i < m_transfunc.size(); i++ )
    {
        if ( m_transfunc[i].m_name == name )
        {
            m_transfunc[i].m_opacity_variable_min = min_value;
            m_transfunc[i].m_opacity_variable_max = max_value;
        }
    }
}

void TransferFunctionSynthesizerCreator::setParameterFile( const ParameterFile& pa )
{
    set_param_info( pa );
}

void TransferFunctionSynthesizerCreator::setProtocol( const jpv::ParticleTransferClientMessage& clntMes )
{
    set_protocol( clntMes );
}

void TransferFunctionSynthesizerCreator::setAsisTransferFunction( const pbvr::TransferFunction& tf )
{
}

void TransferFunctionSynthesizerCreator::setTransferFunction( const std::string& name, const pbvr::TransferFunction& tf )
{
    for ( size_t i = 0; i < m_transfunc.size(); i++ )
    {
        if ( m_transfunc[i].m_name == name )
        {
            NamedTransferFunction ntf( tf );
            ntf.m_selection = NamedTransferFunction::SelectTransferFunction;
            ntf.m_name       = name;
            ntf.m_color_variable   = "_" + name + "_var_c";
            ntf.m_opacity_variable = "_" + name + "_var_o";
            m_transfunc[i] = ntf;
        }
    }
}

TransferFunctionSynthesizerCreator::~TransferFunctionSynthesizerCreator()
{
}

TransferFunctionSynthesizer* TransferFunctionSynthesizerCreator::create()
{
    m_synthesizer = new TransferFunctionSynthesizer();
    m_synthesizer->clear();

    this->assign();
    m_synthesizer->setQuantityMap( m_qmap );

    m_synthesizer->optimize();
    m_synthesizer->create();

    return m_synthesizer;
}

void TransferFunctionSynthesizerCreator::assign()
{
    TransferFunctionMap      tfm_c, tfm_o;
    TransfuncQuantityBindMap tqm_c, tqm_o;
    FunctionStringMap        vfm;

    // delete by @hira at 2016/12/01
    // const std::string tf_synth = m_transfunc_synthesis;

    for ( size_t i = 0; i < m_transfunc.size(); i++ )
    {
        const NamedTransferFunction::Selection s = m_transfunc[i].m_selection;
        const std::string& name = m_transfunc[i].m_name;
        const std::string& var_c = m_transfunc[i].m_color_variable;
        const std::string& var_o = m_transfunc[i].m_opacity_variable;
        const std::string& R = m_transfunc[i].m_equation_red;
        const std::string& G = m_transfunc[i].m_equation_green;
        const std::string& B = m_transfunc[i].m_equation_blue;
        const std::string& A = m_transfunc[i].m_equation_opacity;
        const int32_t resolution = m_transfunc[i].m_resolution;
        const float min_c = m_transfunc[i].m_color_variable_min;
        const float max_c = m_transfunc[i].m_color_variable_max;
        const float min_o = m_transfunc[i].m_opacity_variable_min;
        const float max_o = m_transfunc[i].m_opacity_variable_max;

        if ( s == NamedTransferFunction::SelectExtendTransferFunction )
        {
            tfm_c[name] = ExtendedTransferFunction( R, G, B, A, resolution, min_c, max_c );
            tfm_o[name] = ExtendedTransferFunction( R, G, B, A, resolution, min_o, max_o );
        }
        else if ( s == NamedTransferFunction::SelectTransferFunction )
        {
            tfm_c[name] = static_cast<pbvr::TransferFunction>( m_transfunc[i] );
            tfm_o[name] = static_cast<pbvr::TransferFunction>( m_transfunc[i] );
            tfm_c[name].setRange( min_c, max_c );
            tfm_o[name].setRange( min_o, max_o );
        }

        tqm_c[name] = var_c;
        tqm_o[name] = var_o;
    }

    for ( size_t i = 0; i < m_voleqn.size(); i++ )
    {
        const std::string& name  = m_voleqn[i].m_name;
        const std::string& value = m_voleqn[i].m_equation;
        vfm[name] = value;
    }

    m_synthesizer->setColorTransferFunctionMap( tfm_c );
    m_synthesizer->setOpacityTransferFunctionMap( tfm_o );
    m_synthesizer->setColorTransfuncQuantityBindMap( tqm_c );
    m_synthesizer->setOpacityTransfuncQuantityBindMap( tqm_o );
    m_synthesizer->setVolumeFunctionStringMap( vfm );

    // delete by @hira at 2016/12/01
    // m_synthesizer->setSynthFunctionString( tf_synth );

    // add by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）
    m_synthesizer->setColorSynthFunctionString( m_color_transfunc_synthesis );
    m_synthesizer->setOpacitySynthFunctionString( m_opacity_transfunc_synthesis );
}

void TransferFunctionSynthesizerCreator::set_protocol( const jpv::ParticleTransferClientMessage& clntMes )
{
    m_transfunc.clear();
    m_voleqn.clear();

    // delete by @hira at 2016/12/01
    // m_transfunc_synthesis = clntMes.m_transfer_function_synthesis;
    // add by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）
    m_color_transfunc_synthesis = clntMes.m_color_transfer_function_synthesis;
    m_opacity_transfunc_synthesis = clntMes.m_opacity_transfer_function_synthesis;

    for ( size_t i = 0; i < clntMes.m_transfer_function.size(); i++ )
    {
        NamedTransferFunction tf;

        tf.m_name          = clntMes.m_transfer_function[i].m_name;
        tf.m_color_variable      = clntMes.m_transfer_function[i].m_color_variable;
        tf.m_color_variable_min   = clntMes.m_transfer_function[i].m_color_variable_min;
        tf.m_color_variable_max   = clntMes.m_transfer_function[i].m_color_variable_max;
        tf.m_opacity_variable    = clntMes.m_transfer_function[i].m_opacity_variable;
        tf.m_opacity_variable_min = clntMes.m_transfer_function[i].m_opacity_variable_min;
        tf.m_opacity_variable_max = clntMes.m_transfer_function[i].m_opacity_variable_max;
        tf.m_resolution    = clntMes.m_transfer_function[i].m_resolution;
        tf.m_equation_red     = clntMes.m_transfer_function[i].m_equation_red;
        tf.m_equation_green     = clntMes.m_transfer_function[i].m_equation_green;
        tf.m_equation_blue     = clntMes.m_transfer_function[i].m_equation_blue;
        tf.m_equation_opacity     = clntMes.m_transfer_function[i].m_equation_opacity;
        tf.setColorMap( clntMes.m_transfer_function[i].colorMap() );
        tf.setOpacityMap( clntMes.m_transfer_function[i].opacityMap() );

        if ( clntMes.m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            tf.m_selection = NamedTransferFunction::SelectExtendTransferFunction;
        }
        else if ( clntMes.m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            tf.m_selection = NamedTransferFunction::SelectTransferFunction;
        }

        m_transfunc.push_back( tf );
    }

    for ( size_t i = 0; i < clntMes.m_volume_equation.size(); i++ )
    {
        VolumeEquation ve;
        ve.m_name     = clntMes.m_volume_equation[i].m_name;
        ve.m_equation = clntMes.m_volume_equation[i].m_equation;
        m_voleqn.push_back( ve );
    }
}

void TransferFunctionSynthesizerCreator::set_param_info( const ParameterFile& pa )
{
    m_transfunc.clear();
    m_voleqn.clear();

    ParameterFile   pa1 = const_cast<ParameterFile&>( pa );

    // value check
    if ( !pa1.getState( "TF_RESOLUTION" ) || !pa1.getState( "TF_SYNTH_C" ) || !pa1.getState( "TF_SYNTH_O" ) )
    {
        return;
    }

    int32_t resolution = static_cast<int32_t>( pa1.getInt( "TF_RESOLUTION" ) );
    // delete by @hira at 2016/12/01
    // m_transfunc_synthesis =   pa1.getString( "TF_SYNTH" );
    int transfer_function_number = pa1.getInt( "TF_NUMBER" );
    this->m_color_transfunc_synthesis = pa1.getString( "TF_SYNTH_C" );
    this->m_opacity_transfunc_synthesis = pa1.getString( "TF_SYNTH_O" );

    // 色関数
    for ( size_t n = 0; n < MAX_TF_NUMBER; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
        const std::string tag_base = ss.str();
        std::string name = tag_base + "C";
        if (!pa1.getState(name)) {
            continue;
        }
        NamedTransferFunction tf;
        VolumeEquation        ve_c;

        // value check
        if (	!pa1.getState( tag_base + "C" ) ||
                !pa1.getState( tag_base + "MIN_C" ) ||
                !pa1.getState( tag_base + "MAX_C" ) ||
                !pa1.getState( tag_base + "VAR_C" ) ||
                !pa1.getState( tag_base + "TABLE_C" ) ) continue;

        tf.create( ( size_t )resolution );
        tf.m_name = pa1.getString( tag_base + "C" );
        tf.m_resolution = resolution;
        tf.m_color_variable_min = pa1.getFloat( tag_base + "MIN_C" );
        tf.m_color_variable_max = pa1.getFloat( tag_base + "MAX_C" );
        ve_c.m_equation = pa1.getString( tag_base + "VAR_C" );

        kvs::ColorMap::Table colorTable( resolution * 3 );
        std::string s_color = pa1.getString( tag_base + "TABLE_C" );
        char* cp_color = ( char* )s_color.c_str();
        for ( int32_t j = 0; j < resolution * 3; j++ )
        {
            int val;
            sscanf( cp_color, "%d,", &val );
            colorTable[j] = val;
            cp_color = strstr( cp_color, "," ) + 1;
        }

        kvs::ColorMap colorMap( colorTable );
        colorMap.setResolution( ( size_t )resolution );
        colorMap.setRange( tf.m_color_variable_min, tf.m_color_variable_max );
        tf.setColorMap( colorMap );

        std::stringstream fss;
        fss << "_F" << (n+1);
        const std::string fname = fss.str();
        const std::string cname = fname + "_VAR_C";
        tf.m_color_variable   = cname;
        ve_c.m_name = cname;
        tf.m_selection  = NamedTransferFunction::SelectTransferFunction;
        m_transfunc.push_back( tf );
        m_voleqn.push_back( ve_c );
    }

    // 不透明度関数
    for ( size_t n = 0; n < MAX_TF_NUMBER; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
        const std::string tag_base = ss.str();
        std::string name = tag_base + "O";
        if (!pa1.getState(name)) {
            continue;
        }
        NamedTransferFunction tf;
        VolumeEquation        ve_o;

        // value check
        if (	!pa1.getState( tag_base + "O" ) ||
                !pa1.getState( tag_base + "MIN_O" ) ||
                !pa1.getState( tag_base + "MAX_O" ) ||
                !pa1.getState( tag_base + "VAR_O" ) ||
                !pa1.getState( tag_base + "TABLE_O" ) ) continue;

        tf.create( ( size_t )resolution );
        tf.m_name = pa1.getString( tag_base + "O" );
        tf.m_resolution = resolution;
        tf.m_opacity_variable_min = pa1.getFloat( tag_base + "MIN_O" );
        tf.m_opacity_variable_max = pa1.getFloat( tag_base + "MAX_O" );
        ve_o.m_equation = pa1.getString( tag_base + "VAR_O" );

        kvs::OpacityMap::Table opacityTable( resolution );
        std::string s_opacity = pa1.getString( tag_base + "TABLE_O" );
        char* cp_opacity = ( char* )s_opacity.c_str();
        for ( int32_t j = 0; j < resolution; j++ )
        {
            float v_opacity;
            sscanf( cp_opacity, "%f,", &v_opacity );
            opacityTable[j] = v_opacity;
            cp_opacity = strstr( cp_opacity, "," ) + 1;
        }

        kvs::OpacityMap opacityMap( opacityTable );
        opacityMap.setResolution( resolution );
        opacityMap.setRange( tf.m_opacity_variable_min, tf.m_opacity_variable_max );
        tf.setOpacityMap( opacityMap );

        std::stringstream fss;
        fss << "_F" << (n+1);

        const std::string fname = fss.str();
        const std::string oname = fname + "_VAR_O";
        tf.m_opacity_variable = oname;
        ve_o.m_name = oname;

        tf.m_selection  = NamedTransferFunction::SelectTransferFunction;

        m_transfunc.push_back( tf );
        m_voleqn.push_back( ve_o );
    }
}


/**
 * すべての関数定義に最小値、最大値を設定する
 */
void TransferFunctionSynthesizerCreator::setInitializeRange( const float min_value, const float max_value )
{
    for ( size_t i = 0; i < m_transfunc.size(); i++ ) {
        m_transfunc[i].m_color_variable_min = min_value;
        m_transfunc[i].m_color_variable_max = max_value;
        m_transfunc[i].m_opacity_variable_min = min_value;
        m_transfunc[i].m_opacity_variable_max = max_value;
    }
}

