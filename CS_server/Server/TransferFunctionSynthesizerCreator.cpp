
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

void TransferFunctionSynthesizerCreator::setFilterInfo( const FilterInformationFile& fi )
{
    this->initQuantityMap( fi.m_number_ingredients );
}

void TransferFunctionSynthesizerCreator::initQuantityMap( const int32_t m_number_ingredients )
{
      m_component = m_number_ingredients;    
}

void TransferFunctionSynthesizerCreator::setRange( const std::string name, const float min_value, const float max_value )
{
    setColorRange( min_value, max_value );
    setOpacityRange( min_value, max_value );
}

void TransferFunctionSynthesizerCreator::setColorRange( const float min_value, const float max_value )
{
    for ( size_t i = 0; i < m_transfunc.size(); i++ )
    {
            m_transfunc[i].m_color_variable_min = min_value;
            m_transfunc[i].m_color_variable_max = max_value;
    }
}

void TransferFunctionSynthesizerCreator::setOpacityRange( const float min_value, const float max_value )
{
    for ( size_t i = 0; i < m_transfunc.size(); i++ )
    {
            m_transfunc[i].m_opacity_variable_min = min_value;
            m_transfunc[i].m_opacity_variable_max = max_value;
    }
}

void TransferFunctionSynthesizerCreator::setParameterFile( const ParameterFile& pa )
{
    read_TF_from_ParamInfo( pa ); 
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
//    for ( size_t i = 0; i < m_transfunc.size(); i++ )
//    {
//        if ( m_transfunc[i].m_name == name )
//        {
//            NamedTransferFunction ntf( tf );
//            ntf.m_selection = NamedTransferFunction::SelectTransferFunction;
//            ntf.m_name       = name;
//            ntf.m_color_variable   = "_" + name + "_var_c";
//            ntf.m_opacity_variable = "_" + name + "_var_o";
//            m_transfunc[i] = ntf;
//        }
//    }
}

TransferFunctionSynthesizerCreator::~TransferFunctionSynthesizerCreator()
{
}

TransferFunctionSynthesizer* TransferFunctionSynthesizerCreator::create()
{
    //m_synthesizer = new TransferFunctionSynthesizer();
    //m_synthesizer->clear();

    //this->assign();
    //m_synthesizer->setQuantityMap( m_qmap );
    //m_synthesizer.m_component = m_component;

    //m_synthesizer->optimize();
    //m_synthesizer->create();

    return m_synthesizer;
}

std::vector<NamedTransferFunction> TransferFunctionSynthesizerCreator::transfunc()
{
return m_transfunc;
}


//void TransferFunctionSynthesizerCreator::assign()
//{
//    TransferFunctionMap      tfm_c, tfm_o;
//    TransfuncQuantityBindMap tqm_c, tqm_o;
//    FunctionStringMap        vfm;
//
//    // delete by @hira at 2016/12/01
//    // const std::string tf_synth = m_transfunc_synthesis;
//
//    for ( size_t i = 0; i < m_transfunc.size(); i++ )
//    {
//        const NamedTransferFunction::Selection s = m_transfunc[i].m_selection;
//        const std::string& name = m_transfunc[i].m_name;
//        const std::string& var_c = m_transfunc[i].m_color_variable;
//        const std::string& var_o = m_transfunc[i].m_opacity_variable;
//        const std::string& R = m_transfunc[i].m_equation_red;
//        const std::string& G = m_transfunc[i].m_equation_green;
//        const std::string& B = m_transfunc[i].m_equation_blue;
//        const std::string& A = m_transfunc[i].m_equation_opacity;
//        const int32_t resolution = m_transfunc[i].m_resolution;
//        const float min_c = m_transfunc[i].m_color_variable_min;
//        const float max_c = m_transfunc[i].m_color_variable_max;
//        const float min_o = m_transfunc[i].m_opacity_variable_min;
//        const float max_o = m_transfunc[i].m_opacity_variable_max;
//
//        if ( s == NamedTransferFunction::SelectExtendTransferFunction )
//        {
//            tfm_c[name] = ExtendedTransferFunction( R, G, B, A, resolution, min_c, max_c );
//            tfm_o[name] = ExtendedTransferFunction( R, G, B, A, resolution, min_o, max_o );
//        }
//        else if ( s == NamedTransferFunction::SelectTransferFunction )
//        {
//            tfm_c[name] = static_cast<pbvr::TransferFunction>( m_transfunc[i] );
//            tfm_o[name] = static_cast<pbvr::TransferFunction>( m_transfunc[i] );
//            tfm_c[name].setRange( min_c, max_c );
//            tfm_o[name].setRange( min_o, max_o );
//        }
//
//        tqm_c[name] = var_c;
//        tqm_o[name] = var_o;
//    }
//
//    for ( size_t i = 0; i < m_voleqn.size(); i++ )
//    {
//        const std::string& name  = m_voleqn[i].m_name;
//        const std::string& value = m_voleqn[i].m_equation;
//        vfm[name] = value;
//    }
//
//    m_synthesizer->setColorTransferFunctionMap( tfm_c );
//    m_synthesizer->setOpacityTransferFunctionMap( tfm_o );
//    m_synthesizer->setColorTransfuncQuantityBindMap( tqm_c );
//    m_synthesizer->setOpacityTransfuncQuantityBindMap( tqm_o );
//    m_synthesizer->setVolumeFunctionStringMap( vfm );
//
//    // delete by @hira at 2016/12/01
//    // m_synthesizer->setSynthFunctionString( tf_synth );
//
//    // add by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）
//    m_synthesizer->setColorSynthFunctionString( m_color_transfunc_synthesis );
//    m_synthesizer->setOpacitySynthFunctionString( m_opacity_transfunc_synthesis );
//}

void TransferFunctionSynthesizerCreator::set_protocol( const jpv::ParticleTransferClientMessage& clntMes )
{
    m_transfunc.clear();
    m_voleqn.clear();

    m_synthesizer = new TransferFunctionSynthesizer();
    
    size_t cnt = clntMes.transfunc.size()/2;
    for ( size_t i = 0; i < clntMes.transfunc.size(); i++ )
    {
        NamedTransferFunction tf;

        tf.m_name          = clntMes.transfunc[i].Name;
        tf.m_color_variable       = clntMes.transfunc[i].ColorVar;
        tf.m_color_variable_min   = clntMes.transfunc[i].ColorVarMin;
        tf.m_color_variable_max   = clntMes.transfunc[i].ColorVarMax;
        tf.m_opacity_variable     = clntMes.transfunc[i].OpacityVar;
        tf.m_opacity_variable_min = clntMes.transfunc[i].OpacityVarMin;
        tf.m_opacity_variable_max = clntMes.transfunc[i].OpacityVarMax;
        tf.m_resolution    = clntMes.transfunc[i].Resolution;
        tf.m_equation_red     = clntMes.transfunc[i].EquationR;
        tf.m_equation_green     = clntMes.transfunc[i].EquationG;
        tf.m_equation_blue     = clntMes.transfunc[i].EquationB;
        tf.m_equation_opacity     = clntMes.transfunc[i].EquationA;
        tf.setColorMap( clntMes.transfunc[i].colorMap() );
        tf.setOpacityMap( clntMes.transfunc[i].opacityMap() );

        // add by shimomura at 2022/12/12
        EquationToken eq;
        std::vector<EquationToken> var; 

        for(size_t j=0; j<128; j++) eq.exp_token[j] = clntMes.opacity_func.exp_token[j];                    
        for(size_t j=0; j<128; j++) eq.var_name[j]  = clntMes.opacity_func.var_name[j];                    
        for(size_t j=0; j<128; j++) eq.val_array[j] = clntMes.opacity_func.value_array[j];                    
        m_synthesizer -> setOpacityFunction( eq );

        for(size_t j=0; j<128; j++) eq.exp_token[j] = clntMes.color_func.exp_token[j];  
        for(size_t j=0; j<128; j++) eq.var_name[j]  = clntMes.color_func.var_name[j];                    
        for(size_t j=0; j<128; j++) eq.val_array[j] = clntMes.color_func.value_array[j];                    
        m_synthesizer -> setColorFunction( eq );

//        std::cout << "clntMes.opacity_var.size()  = " << clntMes.opacity_var.size() <<std::endl;
        for(size_t k=0; k< clntMes.opacity_var.size(); k++)
        {
            for(size_t j=0; j<128; j++) eq.exp_token[j] = clntMes.opacity_var[k].exp_token[j];  
            for(size_t j=0; j<128; j++) eq.var_name[j]  = clntMes.opacity_var[k].var_name[j];                    
            for(size_t j=0; j<128; j++) eq.val_array[j] = clntMes.opacity_var[k].value_array[j];  

            var.push_back( eq );
        }
        m_synthesizer -> setOpacityVariable( var );

	var.clear();

        for(size_t k=0; k< clntMes.color_var.size(); k++)
        {
            for(size_t j=0; j<128; j++) eq.exp_token[j] = clntMes.color_var[k].exp_token[j];  
            for(size_t j=0; j<128; j++) eq.var_name[j]  = clntMes.color_var[k].var_name[j];                    
            for(size_t j=0; j<128; j++) eq.val_array[j] = clntMes.color_var[k].value_array[j];                    

            var.push_back( eq );
        }
        m_synthesizer -> setColorVariable( var );
        
        if ( clntMes.transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            tf.m_selection = NamedTransferFunction::SelectExtendTransferFunction;
        }
        else if ( clntMes.transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            tf.m_selection = NamedTransferFunction::SelectTransferFunction;
        }
           m_transfunc.push_back( tf );
    }

    // overwrite opacitymap add by shimomura  2023/1/24    
    for ( size_t i = 0; i < cnt; i++ )
    {
        m_transfunc[i].m_color_variable     = clntMes.transfunc[i].ColorVar;
        m_transfunc[i].m_color_variable_min = clntMes.transfunc[i].ColorVarMin;
        m_transfunc[i].m_color_variable_max = clntMes.transfunc[i].ColorVarMax;
        m_transfunc[i].setColorMap( clntMes.transfunc[i].colorMap() );
	m_transfunc[i].setColorRange( m_transfunc[i].m_color_variable_min, m_transfunc[i].m_color_variable_max );
	
        m_transfunc[i].m_opacity_variable     = clntMes.transfunc[i+cnt].OpacityVar;
        m_transfunc[i].m_opacity_variable_min = clntMes.transfunc[i+cnt].OpacityVarMin;
        m_transfunc[i].m_opacity_variable_max = clntMes.transfunc[i+cnt].OpacityVarMax;
        m_transfunc[i].setOpacityMap( clntMes.transfunc[i+cnt].opacityMap() );
	m_transfunc[i].setOpacityRange( m_transfunc[i].m_opacity_variable_min, m_transfunc[i].m_opacity_variable_max );

        //m_transfunc[i].setRange(m_transfunc[i].m_color_variable_min,m_transfunc[i].m_color_variable_max);
        //for(int j=0; j<256; j++) std::cout << "m_transfunc["<< i <<"].opacityMap().["<< j <<"] = " << m_transfunc[i].opacityMap()[j] <<std::endl;
    }    
        //for(int i=0; i<clntMes.transfunc.size(); i++) std::cout << "m_transfunc["<< i <<"].colorMap().maxValue() = " << m_transfunc[i].colorMap().maxValue() << std::endl;

    for ( size_t i = 0; i < clntMes.voleqn.size(); i++ )
    {
        VolumeEquation ve;
        ve.m_name     = clntMes.voleqn[i].Name;
        ve.m_equation = clntMes.voleqn[i].Equation;
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

//// add by @shimomura 2022/12/05
////void readTFfromParamInfo( ParamInfo* param,
////                          std::vector<pbvr::TransferFunction>& tf,
////                          TransferFunctionSynthesizer* tfs )
void TransferFunctionSynthesizerCreator::read_TF_from_ParamInfo( const ParameterFile& pa ) 
{
    m_transfunc.clear();
    m_voleqn.clear();

    ParameterFile   pa1 = const_cast<ParameterFile&>( pa );

    // value check
    if ( !pa1.getState( "TF_RESOLUTION" ) || !pa1.getState( "COLOR_SYNTH" ) || !pa1.getState( "OPACITY_SYNTH" ) )
    {
        return;
    }

    // add by shimomura 2022/12/19
    float particle_density         = pa1.getFloat( "PARTICLE_DENSITY" );  
    float particle_data_size_limit = pa1.getFloat( "PARTICLE_DATA_SIZE_LIMIT" );

    m_synthesizer -> setParticleDensity(particle_density);
    m_synthesizer -> setParticleDataSizeLimit(particle_data_size_limit);


    int32_t resolution = static_cast<int32_t>( pa1.getInt( "TF_RESOLUTION" ) );
    int transfer_function_number = pa1.getInt( "TF_NUMBER" );
    this->m_color_transfunc_synthesis = pa1.getString( "COLOR_SYNTH" );
    this->m_opacity_transfunc_synthesis = pa1.getString( "OPACITY_SYNTH" );

    //Read TFS
    std::vector<int> i_table;
    std::vector<float> f_table;
    EquationToken eq;
    std::vector<EquationToken> var; 
    int tf_number;

    i_table = pa1.getTableInt( "OPA_FUNC_EXP_TOKEN" );
    if (i_table.size() < 128){
        std::cerr<<"Error retrieving TF from ParamInfo"<<std::endl<<
        "If you are trying to overwrite an existing job you may need to execute RESET.sh first."<<std::endl;
        exit(1);
    }    

    for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

    i_table = pa1.getTableInt( "OPA_FUNC_VAR_NAME" );
    for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

    f_table = pa1.getTableFloat( "OPA_FUNC_VAL_ARRAY" );
    for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

    m_synthesizer ->setOpacityFunction( eq );

    i_table = pa1.getTableInt( "COL_FUNC_EXP_TOKEN" );
    for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

    i_table = pa1.getTableInt( "COL_FUNC_VAR_NAME" );
    for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

    f_table = pa1.getTableFloat( "COL_FUNC_VAL_ARRAY" );
    for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

    //tf.m_selection  = NamedTransferFunction::SelectTransferFunction;

    m_synthesizer -> setColorFunction( eq );
    //tfs->setColorFunction( eq );

    // get TF_NUMBER
    tf_number = pa1.getInt( "TF_NUMBER" );

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        i_table = pa1.getTableInt( tag_base + "O_EXP_TOKEN" );
        for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

        i_table = pa1.getTableInt( tag_base + "O_VAR_NAME" );
        for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

        f_table = pa1.getTableFloat( tag_base + "O_VAL_ARRAY" );
        for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

        var.push_back( eq );
    }

    m_synthesizer ->setOpacityVariable( var );

    var.clear();
    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        i_table = pa1.getTableInt( tag_base + "C_EXP_TOKEN" );
        for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

        i_table = pa1.getTableInt( tag_base + "C_VAR_NAME" );
        for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

        f_table = pa1.getTableFloat( tag_base + "C_VAL_ARRAY" );
        for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

        var.push_back( eq );
    }

    m_synthesizer ->setColorVariable( var );

    //Read 1D tf
    float min, max;

    m_transfunc.clear();

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        min = pa1.getFloat( tag_base +"MIN_C" );
        max = pa1.getFloat( tag_base +"MAX_C" );
        i_table = pa1.getTableInt( tag_base + "TABLE_C" );
        kvs::ValueArray<kvs::UInt8> u_table( i_table.size() );
        for( size_t j = 0; j<i_table.size(); j++ ) u_table[j] = (kvs::UInt8)i_table[j];
        kvs::ColorMap color_map( u_table, min, max );

        min = pa1.getFloat( tag_base +"MIN_O" );
        max = pa1.getFloat( tag_base +"MAX_O" );
        f_table = pa1.getTableFloat( tag_base + "TABLE_O" );
        kvs::ValueArray<float> ff_table( f_table );
        kvs::OpacityMap opacity_map( ff_table, min, max );

        pbvr::TransferFunction tfBuf;
        tfBuf.setColorMap( color_map );
        tfBuf.setOpacityMap( opacity_map );
        m_transfunc.push_back(tfBuf);
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

