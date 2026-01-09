#include "SetDefaultTransferFunction.h"
#include <vismodule/SetDefaultTransferFunction>
#include <vismodule/ExtendedTransferFunctionParameter>

/*　多地点対応後削除予定
void setDefalutTransferFunction(jpv::ParticleTransferServerMessage* servMes, const int tf_number )
{
    servMes->m_transfer_function.clear();
    servMes->m_transfer_function.resize(tf_number);
    int TF_resolution = 256;
    servMes->m_color_transfer_function_synthesis = "C1"; 
    servMes->m_opacity_transfer_function_synthesis = "O1"; 


    // set defalut opacity & color  parameter
    std::vector<float> o_table ={0, 0.00392157, 0.00784314, 0.0117647, 0.0156863, 0.0196078, 0.0235294, 0.027451, 0.0313726, 0.0352941, 0.0392157, 0.0431373, 0.0470588, 0.0509804, 0.054902, 0.0588235, 0.0627451, 0.0666667, 0.0705882, 0.0745098, 0.0784314, 0.0823529, 0.0862745, 0.0901961, 0.0941177, 0.0980392, 0.101961, 0.105882, 0.109804, 0.113725, 0.117647, 0.121569, 0.12549, 0.129412, 0.133333, 0.137255, 0.141176, 0.145098, 0.14902, 0.152941, 0.156863, 0.160784, 0.164706, 0.168627, 0.172549, 0.176471, 0.180392, 0.184314, 0.188235, 0.192157, 0.196078, 0.2, 0.203922, 0.207843, 0.211765, 0.215686, 0.219608, 0.223529, 0.227451, 0.231373, 0.235294, 0.239216, 0.243137, 0.247059, 0.25098, 0.254902, 0.258824, 0.262745, 0.266667, 0.270588, 0.27451, 0.278431, 0.282353, 0.286275, 0.290196, 0.294118, 0.298039, 0.301961, 0.305882, 0.309804, 0.313726, 0.317647, 0.321569, 0.32549, 0.329412, 0.333333, 0.337255, 0.341176, 0.345098, 0.34902, 0.352941, 0.356863, 0.360784, 0.364706, 0.368627, 0.372549, 0.376471, 0.380392, 0.384314, 0.388235, 0.392157, 0.396078, 0.4, 0.403922, 0.407843, 0.411765, 0.415686, 0.419608, 0.423529, 0.427451, 0.431373, 0.435294, 0.439216, 0.443137, 0.447059, 0.45098, 0.454902, 0.458824, 0.462745, 0.466667, 0.470588, 0.47451, 0.478431, 0.482353, 0.486275, 0.490196, 0.494118, 0.498039, 0.501961, 0.505882, 0.509804, 0.513726, 0.517647, 0.521569, 0.52549, 0.529412, 0.533333, 0.537255, 0.541176, 0.545098, 0.54902, 0.552941, 0.556863, 0.560784, 0.564706, 0.568627, 0.572549, 0.576471, 0.580392, 0.584314, 0.588235, 0.592157, 0.596078, 0.6, 0.603922, 0.607843, 0.611765, 0.615686, 0.619608, 0.623529, 0.627451, 0.631373, 0.635294, 0.639216, 0.643137, 0.647059, 0.65098, 0.654902, 0.658824, 0.662745, 0.666667, 0.670588, 0.67451, 0.678431, 0.682353, 0.686275, 0.690196, 0.694118, 0.698039, 0.701961, 0.705882, 0.709804, 0.713726, 0.717647, 0.721569, 0.72549, 0.729412, 0.733333, 0.737255, 0.741176, 0.745098, 0.74902, 0.752941, 0.756863, 0.760784, 0.764706, 0.768628, 0.772549, 0.776471, 0.780392, 0.784314, 0.788235, 0.792157, 0.796079, 0.8, 0.803922, 0.807843, 0.811765, 0.815686, 0.819608, 0.823529, 0.827451, 0.831373, 0.835294, 0.839216, 0.843137, 0.847059, 0.85098, 0.854902, 0.858824, 0.862745, 0.866667, 0.870588, 0.87451, 0.878431, 0.882353, 0.886275, 0.890196, 0.894118, 0.898039, 0.901961, 0.905882, 0.909804, 0.913726, 0.917647, 0.921569, 0.92549, 0.929412, 0.933333, 0.937255, 0.941177, 0.945098, 0.94902, 0.952941, 0.956863, 0.960784, 0.964706, 0.968628, 0.972549, 0.976471, 0.980392, 0.984314, 0.988235, 0.992157, 0.996078, 1};

    std::vector<vismodule::UInt8> c_table = {5,48,97,6,50,100,7,52,102,8,54,105,9,56,108,10,58,111,11,60,114,12,62,116,14,64,119,15,66,122,16,68,125,17,70,128,18,72,131,19,74,134,20,76,136,21,78,139,22,80,142,23,83,145,24,85,148,25,87,151,27,89,154,28,91,157,29,93,160,30,95,163,31,98,166,32,100,169,33,102,172,35,104,173,37,105,174,38,107,175,40,109,176,41,111,177,43,113,178,45,114,178,46,116,179,47,118,180,49,120,181,50,121,182,51,123,183,53,125,184,54,127,185,55,129,186,57,130,187,58,132,188,59,134,189,60,136,189,61,138,190,63,140,191,64,141,192,65,143,193,66,145,194,67,147,195,71,149,196,74,151,197,78,153,198,81,154,199,85,156,200,88,158,201,91,160,202,95,162,203,98,164,204,101,166,205,104,168,206,107,170,207,110,172,209,113,174,210,116,175,211,118,177,212,121,179,213,124,181,214,127,183,215,130,185,216,132,187,217,135,189,218,138,191,219,141,193,220,143,195,221,146,197,222,149,198,223,151,200,223,154,201,224,157,202,225,159,203,226,162,205,226,164,206,227,167,207,228,169,208,228,172,210,229,174,211,230,177,212,231,179,214,231,182,215,232,184,216,233,187,217,234,189,219,234,192,220,235,194,221,236,197,223,236,199,224,237,202,225,238,204,226,239,207,228,239,209,229,240,210,230,240,212,230,241,213,231,241,215,232,241,216,232,241,218,233,242,219,234,242,221,235,242,222,235,242,224,236,243,225,237,243,227,237,243,228,238,244,230,239,244,231,239,244,233,240,244,234,241,245,235,241,245,237,242,245,238,243,245,240,244,246,241,244,246,243,245,246,244,246,246,246,246,247,247,247,247,247,246,245,248,245,243,248,244,241,248,243,240,249,242,238,249,241,236,249,239,234,250,238,232,250,237,230,250,236,228,250,235,227,251,234,225,251,233,223,251,232,221,251,231,219,251,230,217,252,229,215,252,228,214,252,227,212,252,225,210,252,224,208,252,223,206,253,222,204,253,221,203,253,220,201,253,219,199,253,217,196,253,215,193,252,212,191,252,210,188,252,208,185,252,206,182,252,204,179,251,202,177,251,200,174,251,197,171,250,195,168,250,193,165,250,191,163,249,189,160,249,187,157,248,184,154,248,182,152,248,180,149,247,178,146,247,176,143,246,174,141,246,171,138,245,169,135,245,167,133,244,165,130,243,162,128,242,160,126,241,157,124,240,155,122,239,152,119,238,149,117,237,147,115,235,144,113,234,142,111,233,139,109,232,136,107,231,134,105,230,131,103,229,128,101,227,126,99,226,123,97,225,120,95,224,118,93,223,115,91,221,112,89,220,110,87,219,107,85,218,104,83,217,102,81,215,99,79,214,96,77,213,94,76,211,91,74,210,89,73,208,86,71,207,84,70,206,82,68,204,79,67,203,77,66,201,74,64,200,72,63,198,69,62,197,66,60,196,64,59,194,61,57,193,58,56,191,55,55,190,53,53,188,50,52,187,46,51,185,43,49,184,40,48,182,36,47,181,33,46,179,29,44,178,24,43,175,23,43,172,22,42,169,21,42,166,20,41,162,19,41,159,18,40,156,17,40,153,15,39,150,14,39,147,13,38,144,12,38,141,11,37,138,10,37,135,9,36,132,8,36,129,7,35,126,6,35,123,5,34,120,4,34,117,3,33,115,2,33,112,2,33,109,1,32,106,1,32,103,0,31};


    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream cc, qq, tt;
        cc << "C" << i + 1;
        qq << "q" << i + 1;
        tt << "t" << i + 1;  
        servMes->m_transfer_function[i].m_name          = tt.str();
        servMes->m_transfer_function[i].m_color_variable       = qq.str();
        servMes->m_transfer_function[i].m_user_color_variable_min   = servMes->m_server_side_variable_range.min( tt.str() + "_var_c" );
        servMes->m_transfer_function[i].m_user_color_variable_max   = servMes->m_server_side_variable_range.max( tt.str() + "_var_c" ); 
        servMes->m_transfer_function[i].m_opacity_variable     = qq.str();
        servMes->m_transfer_function[i].m_user_opacity_variable_min = servMes->m_server_side_variable_range.min( tt.str() + "_var_o" );
        servMes->m_transfer_function[i].m_user_opacity_variable_max = servMes->m_server_side_variable_range.max( tt.str() + "_var_o" ); 
        servMes->m_transfer_function[i].m_resolution           = TF_resolution;
        servMes->m_transfer_function[i].m_equation_red         = ""; 
        servMes->m_transfer_function[i].m_equation_green       = ""; 
        servMes->m_transfer_function[i].m_equation_blue        = ""; 
        servMes->m_transfer_function[i].m_equation_opacity     = "";
        vismodule::ValueArray<vismodule::UInt8> cc_table(c_table);
        vismodule::ValueArray<float> oo_table(o_table);
        vismodule::ColorMap color_map( cc_table    , servMes->m_server_side_variable_range.min( tt.str() + "_var_c" ), servMes->m_server_side_variable_range.max( tt.str() + "_var_c" )  );
        vismodule::OpacityMap opacity_map( oo_table, servMes->m_server_side_variable_range.min( tt.str() + "_var_o" ), servMes->m_server_side_variable_range.max( tt.str() + "_var_o" )  );
        //vismodule::ColorMap color_map( TF_resolution*3, m_transfunc[i].m_user_color_variable_min, m_transfunc[i].m_user_color_variable_max  );
        //vismodule::OpacityMap opacity_map( TF_resolution, m_transfunc[i].m_user_color_variable_min, m_transfunc[i].m_user_color_variable_max  );
        //for (int n =0 ; n < TF_resolution*3 ; n++ ) color_map.table.at(n) = 1; // dummy data
        //for (int n =0 ; n < TF_resolution ; n++ ) opacity_map.table.at(n) = 1; // dummy data
        servMes->m_transfer_function[i].setColorMap( color_map );
        servMes->m_transfer_function[i].setOpacityMap( opacity_map );

        //servMes->m_transfer_function[i].m_selection = NamedTransferFunctionParameter::SelectExtendTransferFunction;
        servMes->m_transfer_function[i].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
//        servMes->m_transfer_function.push_back( tf );
    }
}
*/

void setDefalutTransferFunctionToArgument( ParticleProperty& particle_property, const VariableRange vr, const int tf_number )
{
    const int TF_resolution = 256;
    particle_property.m_transfunc_array.clear();
    particle_property.m_transfunc_array.resize( tf_number );
    particle_property.m_color_transfer_function_synthesis = "C1"; 
    particle_property.m_opacity_transfer_function_synthesis = "O1"; 
    // particle_property->m_voleqn.clear();
    // particle_property->m_voleqn.resize( tf_number );

    // set defalut opacity & color  parameter
    std::vector<float> o_table ={0, 0.00392157, 0.00784314, 0.0117647, 0.0156863, 0.0196078, 0.0235294, 0.027451, 0.0313726, 0.0352941, 0.0392157, 0.0431373, 0.0470588, 0.0509804, 0.054902, 0.0588235, 0.0627451, 0.0666667, 0.0705882, 0.0745098, 0.0784314, 0.0823529, 0.0862745, 0.0901961, 0.0941177, 0.0980392, 0.101961, 0.105882, 0.109804, 0.113725, 0.117647, 0.121569, 0.12549, 0.129412, 0.133333, 0.137255, 0.141176, 0.145098, 0.14902, 0.152941, 0.156863, 0.160784, 0.164706, 0.168627, 0.172549, 0.176471, 0.180392, 0.184314, 0.188235, 0.192157, 0.196078, 0.2, 0.203922, 0.207843, 0.211765, 0.215686, 0.219608, 0.223529, 0.227451, 0.231373, 0.235294, 0.239216, 0.243137, 0.247059, 0.25098, 0.254902, 0.258824, 0.262745, 0.266667, 0.270588, 0.27451, 0.278431, 0.282353, 0.286275, 0.290196, 0.294118, 0.298039, 0.301961, 0.305882, 0.309804, 0.313726, 0.317647, 0.321569, 0.32549, 0.329412, 0.333333, 0.337255, 0.341176, 0.345098, 0.34902, 0.352941, 0.356863, 0.360784, 0.364706, 0.368627, 0.372549, 0.376471, 0.380392, 0.384314, 0.388235, 0.392157, 0.396078, 0.4, 0.403922, 0.407843, 0.411765, 0.415686, 0.419608, 0.423529, 0.427451, 0.431373, 0.435294, 0.439216, 0.443137, 0.447059, 0.45098, 0.454902, 0.458824, 0.462745, 0.466667, 0.470588, 0.47451, 0.478431, 0.482353, 0.486275, 0.490196, 0.494118, 0.498039, 0.501961, 0.505882, 0.509804, 0.513726, 0.517647, 0.521569, 0.52549, 0.529412, 0.533333, 0.537255, 0.541176, 0.545098, 0.54902, 0.552941, 0.556863, 0.560784, 0.564706, 0.568627, 0.572549, 0.576471, 0.580392, 0.584314, 0.588235, 0.592157, 0.596078, 0.6, 0.603922, 0.607843, 0.611765, 0.615686, 0.619608, 0.623529, 0.627451, 0.631373, 0.635294, 0.639216, 0.643137, 0.647059, 0.65098, 0.654902, 0.658824, 0.662745, 0.666667, 0.670588, 0.67451, 0.678431, 0.682353, 0.686275, 0.690196, 0.694118, 0.698039, 0.701961, 0.705882, 0.709804, 0.713726, 0.717647, 0.721569, 0.72549, 0.729412, 0.733333, 0.737255, 0.741176, 0.745098, 0.74902, 0.752941, 0.756863, 0.760784, 0.764706, 0.768628, 0.772549, 0.776471, 0.780392, 0.784314, 0.788235, 0.792157, 0.796079, 0.8, 0.803922, 0.807843, 0.811765, 0.815686, 0.819608, 0.823529, 0.827451, 0.831373, 0.835294, 0.839216, 0.843137, 0.847059, 0.85098, 0.854902, 0.858824, 0.862745, 0.866667, 0.870588, 0.87451, 0.878431, 0.882353, 0.886275, 0.890196, 0.894118, 0.898039, 0.901961, 0.905882, 0.909804, 0.913726, 0.917647, 0.921569, 0.92549, 0.929412, 0.933333, 0.937255, 0.941177, 0.945098, 0.94902, 0.952941, 0.956863, 0.960784, 0.964706, 0.968628, 0.972549, 0.976471, 0.980392, 0.984314, 0.988235, 0.992157, 0.996078, 1};
    std::vector<vismodule::UInt8> c_table = {5,48,97,6,50,100,7,52,102,8,54,105,9,56,108,10,58,111,11,60,114,12,62,116,14,64,119,15,66,122,16,68,125,17,70,128,18,72,131,19,74,134,20,76,136,21,78,139,22,80,142,23,83,145,24,85,148,25,87,151,27,89,154,28,91,157,29,93,160,30,95,163,31,98,166,32,100,169,33,102,172,35,104,173,37,105,174,38,107,175,40,109,176,41,111,177,43,113,178,45,114,178,46,116,179,47,118,180,49,120,181,50,121,182,51,123,183,53,125,184,54,127,185,55,129,186,57,130,187,58,132,188,59,134,189,60,136,189,61,138,190,63,140,191,64,141,192,65,143,193,66,145,194,67,147,195,71,149,196,74,151,197,78,153,198,81,154,199,85,156,200,88,158,201,91,160,202,95,162,203,98,164,204,101,166,205,104,168,206,107,170,207,110,172,209,113,174,210,116,175,211,118,177,212,121,179,213,124,181,214,127,183,215,130,185,216,132,187,217,135,189,218,138,191,219,141,193,220,143,195,221,146,197,222,149,198,223,151,200,223,154,201,224,157,202,225,159,203,226,162,205,226,164,206,227,167,207,228,169,208,228,172,210,229,174,211,230,177,212,231,179,214,231,182,215,232,184,216,233,187,217,234,189,219,234,192,220,235,194,221,236,197,223,236,199,224,237,202,225,238,204,226,239,207,228,239,209,229,240,210,230,240,212,230,241,213,231,241,215,232,241,216,232,241,218,233,242,219,234,242,221,235,242,222,235,242,224,236,243,225,237,243,227,237,243,228,238,244,230,239,244,231,239,244,233,240,244,234,241,245,235,241,245,237,242,245,238,243,245,240,244,246,241,244,246,243,245,246,244,246,246,246,246,247,247,247,247,247,246,245,248,245,243,248,244,241,248,243,240,249,242,238,249,241,236,249,239,234,250,238,232,250,237,230,250,236,228,250,235,227,251,234,225,251,233,223,251,232,221,251,231,219,251,230,217,252,229,215,252,228,214,252,227,212,252,225,210,252,224,208,252,223,206,253,222,204,253,221,203,253,220,201,253,219,199,253,217,196,253,215,193,252,212,191,252,210,188,252,208,185,252,206,182,252,204,179,251,202,177,251,200,174,251,197,171,250,195,168,250,193,165,250,191,163,249,189,160,249,187,157,248,184,154,248,182,152,248,180,149,247,178,146,247,176,143,246,174,141,246,171,138,245,169,135,245,167,133,244,165,130,243,162,128,242,160,126,241,157,124,240,155,122,239,152,119,238,149,117,237,147,115,235,144,113,234,142,111,233,139,109,232,136,107,231,134,105,230,131,103,229,128,101,227,126,99,226,123,97,225,120,95,224,118,93,223,115,91,221,112,89,220,110,87,219,107,85,218,104,83,217,102,81,215,99,79,214,96,77,213,94,76,211,91,74,210,89,73,208,86,71,207,84,70,206,82,68,204,79,67,203,77,66,201,74,64,200,72,63,198,69,62,197,66,60,196,64,59,194,61,57,193,58,56,191,55,55,190,53,53,188,50,52,187,46,51,185,43,49,184,40,48,182,36,47,181,33,46,179,29,44,178,24,43,175,23,43,172,22,42,169,21,42,166,20,41,162,19,41,159,18,40,156,17,40,153,15,39,150,14,39,147,13,38,144,12,38,141,11,37,138,10,37,135,9,36,132,8,36,129,7,35,126,6,35,123,5,34,120,4,34,117,3,33,115,2,33,112,2,33,109,1,32,106,1,32,103,0,31};

    vismodule::ValueArray<vismodule::UInt8> cc_table(c_table);
    vismodule::ValueArray<float> oo_table(o_table);

    EquationToken eq;
    std::vector<EquationToken> var_o;
    std::vector<EquationToken> var_c;

    eq = particle_property.m_transfunc_synthesizer->convert_token( "a1" );
    particle_property.m_transfunc_synthesizer->setOpacityFunction( eq );

    eq = particle_property.m_transfunc_synthesizer->convert_token( "c1" );
    particle_property.m_transfunc_synthesizer->setColorFunction( eq );

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream cc, qq, tt, ff;
        cc << "C"  << i + 1;
        qq << "q"  << i + 1;
        tt << "t"  << i + 1;
        ff << "_F" << i + 1 << "_VAR_";

        particle_property.m_transfunc_array[i].m_name                 = tt.str();
        particle_property.m_transfunc_array[i].m_color_variable       = qq.str();
        particle_property.m_transfunc_array[i].m_opacity_variable     = qq.str();

        particle_property.m_transfunc_array[i].m_user_color_variable_min   = vr.min( tt.str() + "_var_c" );
        particle_property.m_transfunc_array[i].m_user_color_variable_max   = vr.max( tt.str() + "_var_c" ); 
        particle_property.m_transfunc_array[i].m_user_opacity_variable_min = vr.min( tt.str() + "_var_o" );
        particle_property.m_transfunc_array[i].m_user_opacity_variable_max = vr.max( tt.str() + "_var_o" ); 
        particle_property.m_transfunc_array[i].m_resolution                = TF_resolution;
        particle_property.m_transfunc_array[i].m_equation_red              = ""; 
        particle_property.m_transfunc_array[i].m_equation_green            = ""; 
        particle_property.m_transfunc_array[i].m_equation_blue             = ""; 
        particle_property.m_transfunc_array[i].m_equation_opacity          = "";

        vismodule::ColorMap color_map( cc_table    , vr.min( tt.str() + "_var_c" ), vr.max( tt.str() + "_var_c" ) );
        vismodule::OpacityMap opacity_map( oo_table, vr.min( tt.str() + "_var_o" ), vr.max( tt.str() + "_var_o" ) );
        
        // particle_property.m_transfunc_array[i].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;

        var_o.push_back( particle_property.m_transfunc_synthesizer->convert_token( qq.str() ) );
        var_c.push_back( particle_property.m_transfunc_synthesizer->convert_token( qq.str() ) );

        // particle_property.m_voleqn[i].m_name     = ff.str() + "C";
        // particle_property.m_voleqn[i].m_equation = qq.str();
    }

    particle_property.m_transfunc_synthesizer->setOpacityVariable( var_o );
    particle_property.m_transfunc_synthesizer->setColorVariable( var_c );
}

/* クライアントメッセージをどのように受け取るか未定のため保留
void setClientTransferFunctionToArgument( Argument *param, const jpv::ParticleTransferClientMessage& clntMes )
{
    const int TF_resolution = 256;
    param->m_named_transfunc_array.clear();
    param->m_named_transfunc_array.resize( clntMes.m_transfer_function.size() );
    param->m_voleqn.clear();
    param->m_voleqn.resize( clntMes.m_transfer_function.size() );
    param->m_color_transfer_function_synthesis   = clntMes.m_color_transfer_function_synthesis; 
    param->m_opacity_transfer_function_synthesis = clntMes.m_opacity_transfer_function_synthesis; 

    if (clntMes.m_transfer_function.size() == 0)
    {
        std::cout << "TF_number is 0 !!" << std::endl;
        return;
    }
    
    for ( size_t i = 0; i < clntMes.m_transfer_function.size(); i++ )
    {
        // NamedTransferFunction変数
        param->m_named_transfunc_array[i].m_name                 = clntMes.m_transfer_function[i].m_name;
        param->m_named_transfunc_array[i].m_color_variable       = clntMes.m_transfer_function[i].m_color_variable;
        param->m_named_transfunc_array[i].m_opacity_variable     = clntMes.m_transfer_function[i].m_opacity_variable;

        // ExtendTransferFunction変数
        param->m_named_transfunc_array[i].m_user_color_variable_min   = clntMes.m_transfer_function[i].m_user_color_variable_min;
        param->m_named_transfunc_array[i].m_user_color_variable_max   = clntMes.m_transfer_function[i].m_user_color_variable_max;
        param->m_named_transfunc_array[i].m_user_opacity_variable_min = clntMes.m_transfer_function[i].m_user_opacity_variable_min;
        param->m_named_transfunc_array[i].m_user_opacity_variable_max = clntMes.m_transfer_function[i].m_user_opacity_variable_max;
        param->m_named_transfunc_array[i].m_resolution           = clntMes.m_transfer_function[i].m_resolution;
        param->m_named_transfunc_array[i].m_equation_red         = clntMes.m_transfer_function[i].m_equation_red;
        param->m_named_transfunc_array[i].m_equation_green       = clntMes.m_transfer_function[i].m_equation_green;
        param->m_named_transfunc_array[i].m_equation_blue        = clntMes.m_transfer_function[i].m_equation_blue;
        param->m_named_transfunc_array[i].m_equation_opacity     = clntMes.m_transfer_function[i].m_equation_opacity;
        
        // TransferFunction変数
        param->m_named_transfunc_array[i].setColorMap( clntMes.m_transfer_function[i].colorMap() );
        param->m_named_transfunc_array[i].setOpacityMap( clntMes.m_transfer_function[i].opacityMap() );
        param->m_named_transfunc_array[i].setColorRange(
            param->m_named_transfunc_array[i].m_user_color_variable_min,
            param->m_named_transfunc_array[i].m_user_color_variable_max
        );
        param->m_named_transfunc_array[i].setOpacityRange(
            param->m_named_transfunc_array[i].m_user_opacity_variable_min,
            param->m_named_transfunc_array[i].m_user_opacity_variable_max
        );

        param->m_named_transfunc_array[i].m_selection = clntMes.m_transfer_function[i].m_selection;

        param->m_voleqn[i].m_name     = clntMes.m_volume_equation[i].m_name;
        param->m_voleqn[i].m_equation = clntMes.m_volume_equation[i].m_equation;
    }
}
*/

/* 多地点対応後削除予定
void setParamTransferFunctionToServer( jpv::ParticleTransferServerMessage* servMes, Argument *param )
{
    servMes->m_transfer_function.clear();
    servMes->m_transfer_function.resize( param->m_named_transfunc_array.size() );
    int TF_resolution = 256;
    servMes->m_volume_equation.clear();
    servMes->m_volume_equation.resize( param->m_named_transfunc_array.size() );
    servMes->m_color_transfer_function_synthesis   = param->m_color_transfer_function_synthesis; 
    servMes->m_opacity_transfer_function_synthesis = param->m_opacity_transfer_function_synthesis; 
    
    for ( size_t i = 0; i < param->m_named_transfunc_array.size(); i++ )
    {
        NamedTransferFunction tf;
        std::stringstream cc, qq, tt;
        cc << "C" << i + 1;
        qq << "q" << i + 1;
        tt << "t" << i + 1;

        // NamedTransferFunction変数
        servMes->m_transfer_function[i].m_name                 = tt.str();
        servMes->m_transfer_function[i].m_color_variable       = param->m_named_transfunc_array[i].m_color_variable;
        servMes->m_transfer_function[i].m_opacity_variable     = param->m_named_transfunc_array[i].m_opacity_variable;

        // ExtendTransferFunction変数
        servMes->m_transfer_function[i].m_user_color_variable_min   = param->m_named_transfunc_array[i].m_user_color_variable_min;
        servMes->m_transfer_function[i].m_user_color_variable_max   = param->m_named_transfunc_array[i].m_user_color_variable_max;
        servMes->m_transfer_function[i].m_user_opacity_variable_min = param->m_named_transfunc_array[i].m_user_opacity_variable_min;
        servMes->m_transfer_function[i].m_user_opacity_variable_max = param->m_named_transfunc_array[i].m_user_opacity_variable_max; 
        servMes->m_transfer_function[i].m_resolution           = param->m_named_transfunc_array[i].m_resolution;
        servMes->m_transfer_function[i].m_equation_red         = param->m_named_transfunc_array[i].m_equation_red; 
        servMes->m_transfer_function[i].m_equation_green       = param->m_named_transfunc_array[i].m_equation_green;
        servMes->m_transfer_function[i].m_equation_blue        = param->m_named_transfunc_array[i].m_equation_blue; 
        servMes->m_transfer_function[i].m_equation_opacity     = param->m_named_transfunc_array[i].m_equation_opacity;

        // TransferFunction変数
        servMes->m_transfer_function[i].setColorMap( param->m_named_transfunc_array[i].colorMap() );
        servMes->m_transfer_function[i].setOpacityMap( param->m_named_transfunc_array[i].opacityMap() );
        servMes->m_transfer_function[i].setColorRange(
            servMes->m_transfer_function[i].m_user_color_variable_min,
            servMes->m_transfer_function[i].m_user_color_variable_max
        );
        servMes->m_transfer_function[i].setOpacityRange(
            servMes->m_transfer_function[i].m_user_opacity_variable_min,
            servMes->m_transfer_function[i].m_user_opacity_variable_max
        );

        servMes->m_transfer_function[i].m_selection = param->m_named_transfunc_array[i].m_selection;

        servMes->m_volume_equation[i].m_name     = param->m_voleqn[i].m_name;
        servMes->m_volume_equation[i].m_equation = param->m_voleqn[i].m_equation;
    }    
}
*/