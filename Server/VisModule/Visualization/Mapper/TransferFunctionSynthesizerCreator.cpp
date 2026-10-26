
#include <vismodule/TransferFunctionSynthesizerCreator>
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

void TransferFunctionSynthesizerCreator::setFilterInfo( const MultiVolumeProperty& mvp )
{
    this->initQuantityMap( mvp.m_number_ingredients );
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
    for ( std::size_t i = 0; i < m_transfunc.size(); i++ )
    {
            m_transfunc[i].m_color_variable_min = min_value;
            m_transfunc[i].m_color_variable_max = max_value;
    }
}

void TransferFunctionSynthesizerCreator::setOpacityRange( const float min_value, const float max_value )
{
    for ( std::size_t i = 0; i < m_transfunc.size(); i++ )
    {
            m_transfunc[i].m_opacity_variable_min = min_value;
            m_transfunc[i].m_opacity_variable_max = max_value;
    }
}

void TransferFunctionSynthesizerCreator::setParameterFile( const TransferFunctionProperty& tfp )
{
    read_TF_from_ParamInfo( tfp ); 
}

void TransferFunctionSynthesizerCreator::setProtocol( const jpv::ParticleTransferClientMessage& clntMes )
{
    set_protocol( clntMes );
}

void TransferFunctionSynthesizerCreator::setAsisTransferFunction( const vismodule::TransferFunction& tf )
{
}

//void TransferFunctionSynthesizerCreator::setTransferFunction( jpv::ParticleTransferServerMessage* servMes )
void TransferFunctionSynthesizerCreator::setTransferFunction( jpv::ParticleTransferServerMessage* servMes, const VariableRange vr )
{
    servMes->m_transfer_function.clear();
    servMes->m_transfer_function.resize(m_transfunc.size());
    int TF_resolution = 256;

    // set defalut opacity & color  parameter
    std::vector<float> o_table ={0, 0.00392157, 0.00784314, 0.0117647, 0.0156863, 0.0196078, 0.0235294, 0.027451, 0.0313726, 0.0352941, 0.0392157, 0.0431373, 0.0470588, 0.0509804, 0.054902, 0.0588235, 0.0627451, 0.0666667, 0.0705882, 0.0745098, 0.0784314, 0.0823529, 0.0862745, 0.0901961, 0.0941177, 0.0980392, 0.101961, 0.105882, 0.109804, 0.113725, 0.117647, 0.121569, 0.12549, 0.129412, 0.133333, 0.137255, 0.141176, 0.145098, 0.14902, 0.152941, 0.156863, 0.160784, 0.164706, 0.168627, 0.172549, 0.176471, 0.180392, 0.184314, 0.188235, 0.192157, 0.196078, 0.2, 0.203922, 0.207843, 0.211765, 0.215686, 0.219608, 0.223529, 0.227451, 0.231373, 0.235294, 0.239216, 0.243137, 0.247059, 0.25098, 0.254902, 0.258824, 0.262745, 0.266667, 0.270588, 0.27451, 0.278431, 0.282353, 0.286275, 0.290196, 0.294118, 0.298039, 0.301961, 0.305882, 0.309804, 0.313726, 0.317647, 0.321569, 0.32549, 0.329412, 0.333333, 0.337255, 0.341176, 0.345098, 0.34902, 0.352941, 0.356863, 0.360784, 0.364706, 0.368627, 0.372549, 0.376471, 0.380392, 0.384314, 0.388235, 0.392157, 0.396078, 0.4, 0.403922, 0.407843, 0.411765, 0.415686, 0.419608, 0.423529, 0.427451, 0.431373, 0.435294, 0.439216, 0.443137, 0.447059, 0.45098, 0.454902, 0.458824, 0.462745, 0.466667, 0.470588, 0.47451, 0.478431, 0.482353, 0.486275, 0.490196, 0.494118, 0.498039, 0.501961, 0.505882, 0.509804, 0.513726, 0.517647, 0.521569, 0.52549, 0.529412, 0.533333, 0.537255, 0.541176, 0.545098, 0.54902, 0.552941, 0.556863, 0.560784, 0.564706, 0.568627, 0.572549, 0.576471, 0.580392, 0.584314, 0.588235, 0.592157, 0.596078, 0.6, 0.603922, 0.607843, 0.611765, 0.615686, 0.619608, 0.623529, 0.627451, 0.631373, 0.635294, 0.639216, 0.643137, 0.647059, 0.65098, 0.654902, 0.658824, 0.662745, 0.666667, 0.670588, 0.67451, 0.678431, 0.682353, 0.686275, 0.690196, 0.694118, 0.698039, 0.701961, 0.705882, 0.709804, 0.713726, 0.717647, 0.721569, 0.72549, 0.729412, 0.733333, 0.737255, 0.741176, 0.745098, 0.74902, 0.752941, 0.756863, 0.760784, 0.764706, 0.768628, 0.772549, 0.776471, 0.780392, 0.784314, 0.788235, 0.792157, 0.796079, 0.8, 0.803922, 0.807843, 0.811765, 0.815686, 0.819608, 0.823529, 0.827451, 0.831373, 0.835294, 0.839216, 0.843137, 0.847059, 0.85098, 0.854902, 0.858824, 0.862745, 0.866667, 0.870588, 0.87451, 0.878431, 0.882353, 0.886275, 0.890196, 0.894118, 0.898039, 0.901961, 0.905882, 0.909804, 0.913726, 0.917647, 0.921569, 0.92549, 0.929412, 0.933333, 0.937255, 0.941177, 0.945098, 0.94902, 0.952941, 0.956863, 0.960784, 0.964706, 0.968628, 0.972549, 0.976471, 0.980392, 0.984314, 0.988235, 0.992157, 0.996078, 1};

    std::vector<vismodule::UInt8> c_table = {5,48,97,6,50,100,7,52,102,8,54,105,9,56,108,10,58,111,11,60,114,12,62,116,14,64,119,15,66,122,16,68,125,17,70,128,18,72,131,19,74,134,20,76,136,21,78,139,22,80,142,23,83,145,24,85,148,25,87,151,27,89,154,28,91,157,29,93,160,30,95,163,31,98,166,32,100,169,33,102,172,35,104,173,37,105,174,38,107,175,40,109,176,41,111,177,43,113,178,45,114,178,46,116,179,47,118,180,49,120,181,50,121,182,51,123,183,53,125,184,54,127,185,55,129,186,57,130,187,58,132,188,59,134,189,60,136,189,61,138,190,63,140,191,64,141,192,65,143,193,66,145,194,67,147,195,71,149,196,74,151,197,78,153,198,81,154,199,85,156,200,88,158,201,91,160,202,95,162,203,98,164,204,101,166,205,104,168,206,107,170,207,110,172,209,113,174,210,116,175,211,118,177,212,121,179,213,124,181,214,127,183,215,130,185,216,132,187,217,135,189,218,138,191,219,141,193,220,143,195,221,146,197,222,149,198,223,151,200,223,154,201,224,157,202,225,159,203,226,162,205,226,164,206,227,167,207,228,169,208,228,172,210,229,174,211,230,177,212,231,179,214,231,182,215,232,184,216,233,187,217,234,189,219,234,192,220,235,194,221,236,197,223,236,199,224,237,202,225,238,204,226,239,207,228,239,209,229,240,210,230,240,212,230,241,213,231,241,215,232,241,216,232,241,218,233,242,219,234,242,221,235,242,222,235,242,224,236,243,225,237,243,227,237,243,228,238,244,230,239,244,231,239,244,233,240,244,234,241,245,235,241,245,237,242,245,238,243,245,240,244,246,241,244,246,243,245,246,244,246,246,246,246,247,247,247,247,247,246,245,248,245,243,248,244,241,248,243,240,249,242,238,249,241,236,249,239,234,250,238,232,250,237,230,250,236,228,250,235,227,251,234,225,251,233,223,251,232,221,251,231,219,251,230,217,252,229,215,252,228,214,252,227,212,252,225,210,252,224,208,252,223,206,253,222,204,253,221,203,253,220,201,253,219,199,253,217,196,253,215,193,252,212,191,252,210,188,252,208,185,252,206,182,252,204,179,251,202,177,251,200,174,251,197,171,250,195,168,250,193,165,250,191,163,249,189,160,249,187,157,248,184,154,248,182,152,248,180,149,247,178,146,247,176,143,246,174,141,246,171,138,245,169,135,245,167,133,244,165,130,243,162,128,242,160,126,241,157,124,240,155,122,239,152,119,238,149,117,237,147,115,235,144,113,234,142,111,233,139,109,232,136,107,231,134,105,230,131,103,229,128,101,227,126,99,226,123,97,225,120,95,224,118,93,223,115,91,221,112,89,220,110,87,219,107,85,218,104,83,217,102,81,215,99,79,214,96,77,213,94,76,211,91,74,210,89,73,208,86,71,207,84,70,206,82,68,204,79,67,203,77,66,201,74,64,200,72,63,198,69,62,197,66,60,196,64,59,194,61,57,193,58,56,191,55,55,190,53,53,188,50,52,187,46,51,185,43,49,184,40,48,182,36,47,181,33,46,179,29,44,178,24,43,175,23,43,172,22,42,169,21,42,166,20,41,162,19,41,159,18,40,156,17,40,153,15,39,150,14,39,147,13,38,144,12,38,141,11,37,138,10,37,135,9,36,132,8,36,129,7,35,126,6,35,123,5,34,120,4,34,117,3,33,115,2,33,112,2,33,109,1,32,106,1,32,103,0,31};


    for ( std::size_t i = 0; i < m_transfunc.size(); i++ )
    {
        NamedTransferFunction tf;
        std::stringstream cc, qq, tt;
        cc << "C" << i + 1;
        qq << "q" << i + 1;
        tt << "t" << i + 1;  
        servMes->m_transfer_function[i].m_name          = tt.str();
        servMes->m_transfer_function[i].m_color_variable       = qq.str();
        servMes->m_transfer_function[i].m_color_variable_min   = vr.min( tt.str() + "_var_c" );
        servMes->m_transfer_function[i].m_color_variable_max   = vr.max( tt.str() + "_var_c" );
        //servMes->m_transfer_function[i].m_color_variable_min   = m_transfunc[i].m_color_variable_min;
        //servMes->m_transfer_function[i].m_color_variable_max   = m_transfunc[i].m_color_variable_max; 
        servMes->m_transfer_function[i].m_opacity_variable     = qq.str();
        servMes->m_transfer_function[i].m_opacity_variable_min   = vr.min( tt.str() + "_var_o" );
        servMes->m_transfer_function[i].m_opacity_variable_max   = vr.max( tt.str() + "_var_o" );
        //servMes->m_transfer_function[i].m_opacity_variable_min = m_transfunc[i].m_opacity_variable_min;
        //servMes->m_transfer_function[i].m_opacity_variable_max = m_transfunc[i].m_opacity_variable_max; 
        servMes->m_transfer_function[i].m_resolution           = TF_resolution;
        servMes->m_transfer_function[i].m_equation_red         = ""; 
        servMes->m_transfer_function[i].m_equation_green       = ""; 
        servMes->m_transfer_function[i].m_equation_blue        = ""; 
        servMes->m_transfer_function[i].m_equation_opacity     = "";
        vismodule::ValueArray<vismodule::UInt8> cc_table(c_table);
        vismodule::ValueArray<float> oo_table(o_table);
        vismodule::ColorMap color_map( cc_table, m_transfunc[i].m_color_variable_min, m_transfunc[i].m_color_variable_max  );
        vismodule::OpacityMap opacity_map( oo_table, m_transfunc[i].m_color_variable_min, m_transfunc[i].m_color_variable_max  );
        servMes->m_transfer_function[i].setColorMap( color_map );
        servMes->m_transfer_function[i].setOpacityMap( opacity_map );

        servMes->m_transfer_function[i].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    }


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
    return m_synthesizer;
}

std::vector<NamedTransferFunction> TransferFunctionSynthesizerCreator::transfunc()
{
    return m_transfunc;
}

void TransferFunctionSynthesizerCreator::setInitialProtocol( const int nvariable, const VariableRange vr)
{
    // メンバ変数の初期化
    m_transfunc.clear();
    m_voleqn.clear();

    // 領域確保
    m_synthesizer = new TransferFunctionSynthesizer();
    
    // 例外処理 
    if (nvariable == 0)
    {
        std::cout << "TF_number is 0 !!" << std::endl;
        return;
    }

    int TF_resolution = 256;
    for ( std::size_t i = 0; i < nvariable; i++ )
    {
        NamedTransferFunction tf;
        std::stringstream cc, qq, tt;
        cc << "t" << i + 1;
        qq << "q" << i + 1;
        tt << "t" << i + 1;  
        //NamedTransferFunction変数
        tf.m_name          = cc.str();
        tf.m_color_variable       = qq.str();
        tf.m_opacity_variable     = qq.str();
        // ExtendTransferFunction変数
        tf.m_color_variable_min   = vr.min( tt.str() + "_var_c" );
        tf.m_color_variable_max   = vr.max( tt.str() + "_var_c" ); 
        tf.m_opacity_variable_min = vr.min( tt.str() + "_var_o" );
        tf.m_opacity_variable_max = vr.max( tt.str() + "_var_o" );
        tf.m_resolution           = TF_resolution;
        tf.m_equation_red         = ""; 
        tf.m_equation_green       = ""; 
        tf.m_equation_blue        = ""; 
        tf.m_equation_opacity     = "";

        // TransferFunction変数
        //vismodule::ColorMap color_map( TF_resolution, tf.m_color_variable_min, tf.m_color_variable_max  );
        vismodule::ColorMap color_map( TF_resolution*3, tf.m_color_variable_min, tf.m_color_variable_max  );
        vismodule::OpacityMap opacity_map( TF_resolution, tf.m_opacity_variable_min, tf.m_opacity_variable_max  );
        tf.setColorMap( color_map );
        tf.setOpacityMap( opacity_map );

        tf.m_selection = NamedTransferFunction::SelectTransferFunction;
        m_transfunc.push_back( tf );
    }

    // overwrite opacitymap add by shimomura  2023/1/24    
    std::size_t cnt = nvariable;
    for ( std::size_t i = 0; i < cnt; i++ )
    {
        m_transfunc[i].setColorRange( m_transfunc[i].m_color_variable_min, m_transfunc[i].m_color_variable_max );
        m_transfunc[i].setOpacityRange( m_transfunc[i].m_opacity_variable_min, m_transfunc[i].m_opacity_variable_max );
    }    


    std::cout << "nvariables = " << m_transfunc.size() << std::endl;

        // add by shimomura at 2022/12/12
        // TransferFunction Synthesizer用変数の設定
        EquationToken eq;
        std::vector<EquationToken> var; 
        std::vector<EquationToken> var_o; 
        std::vector<EquationToken> var_c; 

        std::string opacitySynthBuf = "a1"; 
        std::replace(opacitySynthBuf.begin(), opacitySynthBuf.end(), 'O', 'a');
        eq = m_synthesizer -> convert_token(opacitySynthBuf);

        m_synthesizer -> setOpacityFunction( eq );

        std::string colorSynthBuf = "c1"; 
        std::replace(colorSynthBuf.begin(), colorSynthBuf.end(), 'C', 'c');
        eq = m_synthesizer -> convert_token(colorSynthBuf);
        m_synthesizer -> setColorFunction( eq );

        for ( std::size_t i = 0; i < nvariable ; i++ )
        {
            std::stringstream cc, qq;
            qq << "q" << i + 1;
            std::string OSynthBuf = qq.str();
            std::string CSynthBuf = qq.str() ;

            var_o.push_back( m_synthesizer ->  convert_token(OSynthBuf ));
            var_c.push_back( m_synthesizer ->  convert_token(CSynthBuf ));
        }

        m_synthesizer -> setOpacityVariable( var_o );
        m_synthesizer -> setColorVariable( var_c );

        //vol_eqの設定        
    for ( std::size_t i = 0; i < nvariable; i++ )
    {
        std::stringstream ff, qq;
        ff << "_F" << i << "_VAR_";
        qq << "q" << i ; 
        VolumeEquation ve;
        ve.m_name     = ff.str() + "C";
        ve.m_equation = qq.str();
        m_voleqn.push_back( ve );

        VolumeEquation vo;
        vo.m_name     = ff.str() + "O";
        vo.m_equation = qq.str();
        m_voleqn.push_back( vo );
    }
}


void TransferFunctionSynthesizerCreator::set_protocol( const jpv::ParticleTransferClientMessage& clntMes )
{
    m_transfunc.clear();
    m_voleqn.clear();

    m_synthesizer = new TransferFunctionSynthesizer();
    
    if (clntMes.m_transfer_function.size() == 0)
    {
        std::cout << "TF_number is 0 !!" << std::endl;
        return;
    }

    for ( std::size_t i = 0; i < clntMes.m_transfer_function.size(); i++ )
    {
        NamedTransferFunction tf;
        if ( clntMes.m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            tf.m_selection = NamedTransferFunction::SelectExtendTransferFunction;
        }
        else if ( clntMes.m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            tf.m_selection = NamedTransferFunction::SelectTransferFunction;
        }
        // NamedTransferFunction変数
        tf.m_name          = clntMes.m_transfer_function[i].m_name;
        tf.m_color_variable       = clntMes.m_transfer_function[i].m_color_variable;
        tf.m_opacity_variable     = clntMes.m_transfer_function[i].m_opacity_variable;

        // ExtendTransferFunction変数
        tf.m_color_variable_min   = clntMes.m_transfer_function[i].m_color_variable_min;
        tf.m_color_variable_max   = clntMes.m_transfer_function[i].m_color_variable_max;
        tf.m_opacity_variable_min = clntMes.m_transfer_function[i].m_opacity_variable_min;
        tf.m_opacity_variable_max = clntMes.m_transfer_function[i].m_opacity_variable_max;
        tf.m_resolution    = clntMes.m_transfer_function[i].m_resolution;
        tf.m_equation_red     = clntMes.m_transfer_function[i].m_equation_red;
        tf.m_equation_green     = clntMes.m_transfer_function[i].m_equation_green;
        tf.m_equation_blue     = clntMes.m_transfer_function[i].m_equation_blue;
        tf.m_equation_opacity     = clntMes.m_transfer_function[i].m_equation_opacity;
        
        // TransferFunction変数
        tf.setColorMap( clntMes.m_transfer_function[i].colorMap() );
        tf.setOpacityMap( clntMes.m_transfer_function[i].opacityMap() );
           
        m_transfunc.push_back( tf );
    }

    // add by shimomura at 2022/12/12
    EquationToken eq;
    std::vector<EquationToken> var; 
    std::vector<EquationToken> var_o; 
    std::vector<EquationToken> var_c; 

    std::string opacitySynthBuf = clntMes.m_opacity_transfer_function_synthesis;
    std::replace( opacitySynthBuf.begin(), opacitySynthBuf.end(), 'O', 'a' );
    eq = m_synthesizer->convert_token( opacitySynthBuf );

    m_synthesizer->setOpacityFunction( eq );

    std::string colorSynthBuf = clntMes.m_color_transfer_function_synthesis;
    std::replace( colorSynthBuf.begin(), colorSynthBuf.end(), 'C', 'c' );
    eq = m_synthesizer->convert_token( colorSynthBuf );
    m_synthesizer->setColorFunction( eq );

    int vloeqsize2 = clntMes.m_volume_equation.size() / 2;
    for ( std::size_t i = 0; i < vloeqsize2 ; i++ )
    {
        std::string OSynthBuf = clntMes.m_volume_equation[2 * i + 1].m_equation;
        std::replace( OSynthBuf.begin(), OSynthBuf.end(), 'X', 'x' );
        std::replace( OSynthBuf.begin(), OSynthBuf.end(), 'Y', 'y' );
        std::replace( OSynthBuf.begin(), OSynthBuf.end(), 'Z', 'z' );

        std::string CSynthBuf = clntMes.m_volume_equation[2 * i].m_equation ;
        std::replace( CSynthBuf.begin(), CSynthBuf.end(), 'X', 'x' );
        std::replace( CSynthBuf.begin(), CSynthBuf.end(), 'Y', 'y' );
        std::replace( CSynthBuf.begin(), CSynthBuf.end(), 'Z', 'z' );

        var_o.push_back( m_synthesizer->convert_token( OSynthBuf ) );
        var_c.push_back( m_synthesizer->convert_token( CSynthBuf ) );
        //var_o.push_back( m_synthesizer -> convert_token(clntMes.m_volume_equation[i+vloeqsize2].m_equation) );
        //var_c.push_back( m_synthesizer -> convert_token(clntMes.m_volume_equation[i].m_equation ));
    }

    m_synthesizer -> setOpacityVariable( var_o );
    m_synthesizer -> setColorVariable( var_c );
        
    // overwrite opacitymap add by shimomura  2023/1/24    
    //size_t cnt = clntMes.m_transfer_function.size()/2;
    std::size_t cnt = clntMes.m_transfer_function.size();
    for ( std::size_t i = 0; i < cnt; i++ )
    {
        m_transfunc[i].m_color_variable     = clntMes.m_transfer_function[i].m_color_variable;
        m_transfunc[i].m_color_variable_min = clntMes.m_transfer_function[i].m_color_variable_min;
        m_transfunc[i].m_color_variable_max = clntMes.m_transfer_function[i].m_color_variable_max;
        m_transfunc[i].setColorMap( clntMes.m_transfer_function[i].colorMap() );
        m_transfunc[i].setColorRange( m_transfunc[i].m_color_variable_min, m_transfunc[i].m_color_variable_max );
	
        m_transfunc[i].m_opacity_variable     = clntMes.m_transfer_function[i].m_opacity_variable;
        m_transfunc[i].m_opacity_variable_min = clntMes.m_transfer_function[i].m_opacity_variable_min;
        m_transfunc[i].m_opacity_variable_max = clntMes.m_transfer_function[i].m_opacity_variable_max;
        m_transfunc[i].setOpacityMap( clntMes.m_transfer_function[i].opacityMap() );
        m_transfunc[i].setOpacityRange( m_transfunc[i].m_opacity_variable_min, m_transfunc[i].m_opacity_variable_max );
    }    

    for ( std::size_t i = 0; i < clntMes.m_volume_equation.size(); i++ )
    {
        VolumeEquation ve;
        ve.m_name     = clntMes.m_volume_equation[i].m_name;
        ve.m_equation = clntMes.m_volume_equation[i].m_equation;
        m_voleqn.push_back( ve );
    }
}

void TransferFunctionSynthesizerCreator::set_param_info( const TransferFunctionProperty& tfp )
{
    m_transfunc.clear();
    m_voleqn.clear();

    TransferFunctionProperty   tfp1 = const_cast<TransferFunctionProperty&>( tfp );

    // value check
    if ( !tfp1.getState( "TF_RESOLUTION" ) || !tfp1.getState( "TF_SYNTH_C" ) || !tfp1.getState( "TF_SYNTH_O" ) )
    {
        return;
    }

    int32_t resolution = static_cast<int32_t>( tfp1.getInt( "TF_RESOLUTION" ) );
    // delete by @hira at 2016/12/01
    int transfer_function_number = tfp1.getInt( "TF_NUMBER" );
    this->m_color_transfunc_synthesis = tfp1.getString( "TF_SYNTH_C" );
    this->m_opacity_transfunc_synthesis = tfp1.getString( "TF_SYNTH_O" );

    // 色関数
    for ( std::size_t n = 0; n < MAX_TF_NUMBER; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
        const std::string tag_base = ss.str();
        std::string name = tag_base + "C";
        if (!tfp1.getState(name)) {
            continue;
        }
        NamedTransferFunction tf;
        VolumeEquation        ve_c;

        // value check
        if (	!tfp1.getState( tag_base + "C" ) ||
                !tfp1.getState( tag_base + "MIN_C" ) ||
                !tfp1.getState( tag_base + "MAX_C" ) ||
                !tfp1.getState( tag_base + "VAR_C" ) ||
                !tfp1.getState( tag_base + "TABLE_C" ) ) continue;

        tf.create( ( std::size_t )resolution );
        tf.m_name = tfp1.getString( tag_base + "C" );
        tf.m_resolution = resolution;
        tf.m_color_variable_min = tfp1.getFloat( tag_base + "MIN_C" );
        tf.m_color_variable_max = tfp1.getFloat( tag_base + "MAX_C" );
        ve_c.m_equation = tfp1.getString( tag_base + "VAR_C" );

        vismodule::ColorMap::Table colorTable( resolution * 3 );
        std::string s_color = tfp1.getString( tag_base + "TABLE_C" );
        char* cp_color = ( char* )s_color.c_str();
        for ( int32_t j = 0; j < resolution * 3; j++ )
        {
            int val;
            sscanf( cp_color, "%d,", &val );
            colorTable[j] = val;
            cp_color = strstr( cp_color, "," ) + 1;
        }

        vismodule::ColorMap colorMap( colorTable );
        colorMap.setResolution( ( std::size_t )resolution );
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
    for ( std::size_t n = 0; n < MAX_TF_NUMBER; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
        const std::string tag_base = ss.str();
        std::string name = tag_base + "O";
        if (!tfp1.getState(name)) {
            continue;
        }
        NamedTransferFunction tf;
        VolumeEquation        ve_o;

        // value check
        if (	!tfp1.getState( tag_base + "O" ) ||
                !tfp1.getState( tag_base + "MIN_O" ) ||
                !tfp1.getState( tag_base + "MAX_O" ) ||
                !tfp1.getState( tag_base + "VAR_O" ) ||
                !tfp1.getState( tag_base + "TABLE_O" ) ) continue;

        tf.create( ( std::size_t )resolution );
        tf.m_name = tfp1.getString( tag_base + "O" );
        tf.m_resolution = resolution;
        tf.m_opacity_variable_min = tfp1.getFloat( tag_base + "MIN_O" );
        tf.m_opacity_variable_max = tfp1.getFloat( tag_base + "MAX_O" );
        ve_o.m_equation = tfp1.getString( tag_base + "VAR_O" );

        vismodule::OpacityMap::Table opacityTable( resolution );
        std::string s_opacity = tfp1.getString( tag_base + "TABLE_O" );
        char* cp_opacity = ( char* )s_opacity.c_str();
        for ( int32_t j = 0; j < resolution; j++ )
        {
            float v_opacity;
            sscanf( cp_opacity, "%f,", &v_opacity );
            opacityTable[j] = v_opacity;
            cp_opacity = strstr( cp_opacity, "," ) + 1;
        }

        vismodule::OpacityMap opacityMap( opacityTable );
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
////                          std::vector<vismodule::TransferFunction>& tf,
////                          TransferFunctionSynthesizer* tfs )
void TransferFunctionSynthesizerCreator::read_TF_from_ParamInfo( const TransferFunctionProperty& tfp ) 
{
    m_transfunc.clear();
    m_voleqn.clear();

    TransferFunctionProperty   tfp1 = const_cast<TransferFunctionProperty&>( tfp );

    // value check
    if ( !tfp1.getState( "TF_RESOLUTION" ) || !tfp1.getState( "COLOR_SYNTH" ) || !tfp1.getState( "OPACITY_SYNTH" ) )
    {
        return;
    }

    // add by shimomura 2022/12/19
    float particle_density         = tfp1.getFloat( "PARTICLE_DENSITY" );  
    float particle_data_size_limit = tfp1.getFloat( "PARTICLE_DATA_SIZE_LIMIT" );

    m_synthesizer -> setParticleDensity(particle_density);
    m_synthesizer -> setParticleDataSizeLimit(particle_data_size_limit);


    int32_t resolution = static_cast<int32_t>( tfp1.getInt( "TF_RESOLUTION" ) );
    int transfer_function_number = tfp1.getInt( "TF_NUMBER" );
    this->m_color_transfunc_synthesis = tfp1.getString( "COLOR_SYNTH" );
    this->m_opacity_transfunc_synthesis = tfp1.getString( "OPACITY_SYNTH" );

    //Read TFS
    std::vector<int> i_table;
    std::vector<float> f_table;
    EquationToken eq;
    std::vector<EquationToken> var; 
    int tf_number;

    i_table = tfp1.getTableInt( "OPA_FUNC_EXP_TOKEN" );
    if (i_table.size() < 128){
        std::cerr<<"Error retrieving TF from ParamInfo"<<std::endl<<
        "If you are trying to overwrite an existing job you may need to execute RESET.sh first."<<std::endl;
        exit(1);
    }    

    for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

    i_table = tfp1.getTableInt( "OPA_FUNC_VAR_NAME" );
    for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

    f_table = tfp1.getTableFloat( "OPA_FUNC_VAL_ARRAY" );
    for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

    m_synthesizer ->setOpacityFunction( eq );

    i_table = tfp1.getTableInt( "COL_FUNC_EXP_TOKEN" );
    for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

    i_table = tfp1.getTableInt( "COL_FUNC_VAR_NAME" );
    for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

    f_table = tfp1.getTableFloat( "COL_FUNC_VAL_ARRAY" );
    for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

    //tf.m_selection  = NamedTransferFunction::SelectTransferFunction;

    m_synthesizer -> setColorFunction( eq );
    //tfs->setColorFunction( eq );

    // get TF_NUMBER
    tf_number = tfp1.getInt( "TF_NUMBER" );

    for ( std::size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        i_table = tfp1.getTableInt( tag_base + "O_EXP_TOKEN" );
        for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

        i_table = tfp1.getTableInt( tag_base + "O_VAR_NAME" );
        for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

        f_table = tfp1.getTableFloat( tag_base + "O_VAL_ARRAY" );
        for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

        var.push_back( eq );
    }

    m_synthesizer ->setOpacityVariable( var );

    var.clear();
    for ( std::size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        i_table = tfp1.getTableInt( tag_base + "C_EXP_TOKEN" );
        for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

        i_table = tfp1.getTableInt( tag_base + "C_VAR_NAME" );
        for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

        f_table = tfp1.getTableFloat( tag_base + "C_VAL_ARRAY" );
        for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

        var.push_back( eq );
    }

    m_synthesizer ->setColorVariable( var );

    //Read 1D tf
    float min, max;

    m_transfunc.clear();

    for ( std::size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        min = tfp1.getFloat( tag_base +"MIN_C" );
        max = tfp1.getFloat( tag_base +"MAX_C" );
        i_table = tfp1.getTableInt( tag_base + "TABLE_C" );
        vismodule::ValueArray<vismodule::UInt8> u_table( i_table.size() );
        for( std::size_t j = 0; j<i_table.size(); j++ ) u_table[j] = (vismodule::UInt8)i_table[j];
        vismodule::ColorMap color_map( u_table, min, max );

        min = tfp1.getFloat( tag_base +"MIN_O" );
        max = tfp1.getFloat( tag_base +"MAX_O" );
        f_table = tfp1.getTableFloat( tag_base + "TABLE_O" );
        vismodule::ValueArray<float> ff_table( f_table );
        vismodule::OpacityMap opacity_map( ff_table, min, max );

        vismodule::TransferFunction tfBuf;
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
    for ( std::size_t i = 0; i < m_transfunc.size(); i++ ) {
        m_transfunc[i].m_color_variable_min = min_value;
        m_transfunc[i].m_color_variable_max = max_value;
        m_transfunc[i].m_opacity_variable_min = min_value;
        m_transfunc[i].m_opacity_variable_max = max_value;
    }
}

