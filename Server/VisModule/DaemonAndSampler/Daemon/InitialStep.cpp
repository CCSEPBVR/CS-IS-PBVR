#include <array>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include <vismodule/InitialStep>
#include <vismodule/Calculate>
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileReader>
#include <vismodule/JobDispatcher>
#include <vismodule/PointObjectGenerator>
#include <vismodule/GenerateParticle>

#ifndef CPU_VER
    #include <vismodule/JobCollector>
#endif

namespace
{
void ApplyStatisticHistoryToTransferFunctions(
    const std::vector<std::vector<int>>& color_histograms,
    const std::vector<std::vector<int>>& opacity_histograms,
    VariableRange& variable_range,
    std::vector<EnsembleTransferFunction>& transfer_functions,
    const char* statistic_name )
{
    const int tf_number = static_cast<int>( opacity_histograms.size() );
    if ( tf_number <= 0 ) return;

    if ( transfer_functions.size() < static_cast<size_t>( tf_number ) )
    {
        transfer_functions.resize( tf_number );
    }

    for ( int i = 0; i < tf_number; ++i )
    {
        std::fill_n( transfer_functions[i].m_opacity_histogram, DEFAULT_NBINS, 0 );
        const std::vector<int>& opacity_histogram = opacity_histograms[i];
        const int opacity_bins = static_cast<int>( opacity_histogram.size() );
        for ( int b = 0; b < DEFAULT_NBINS && b < opacity_bins; ++b )
        {
            transfer_functions[i].m_opacity_histogram[b] =
                static_cast<vismodule::UInt64>( opacity_histogram[b] );
        }

        std::fill_n( transfer_functions[i].m_color_histogram, DEFAULT_NBINS, 0 );
        if ( i < static_cast<int>( color_histograms.size() ) )
        {
            const std::vector<int>& color_histogram = color_histograms[i];
            const int color_bins = static_cast<int>( color_histogram.size() );
            for ( int b = 0; b < DEFAULT_NBINS && b < color_bins; ++b )
            {
                transfer_functions[i].m_color_histogram[b] =
                    static_cast<vismodule::UInt64>( color_histogram[b] );
            }
        }

        std::stringstream ss;
        ss << ( i + 1 );
        const std::string idxbuf = ss.str();
        transfer_functions[i].m_server_variable_min =
            variable_range.min( "t" + idxbuf + "_var_c" );
        transfer_functions[i].m_server_variable_max =
            variable_range.max( "t" + idxbuf + "_var_c" );
    }

    std::cout << "[InitialStepIS] applied ensemble statistic history statistic="
              << statistic_name << ", tf_number=" << tf_number << std::endl;
}
}

// 初回通信用 デフォルトパラメータを設定する(CS)
bool SetDefaultParticleParameterCS(
    const std::string& transfer_function_file_name,
    const MultiVolumePropertyList& mvpl,
    ParticleProperty& particle_property
)
{
    int rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    rank = 0;
	mpi_size = 1;
#endif

    particle_property.m_level_index              = 1;
    particle_property.m_repeat_level             = 4;
    particle_property.m_sampling_method          = 'u';
    particle_property.m_particle_data_size_limit = 20;
    particle_property.m_particle_limit           = 10000000;
    particle_property.m_extra_opacity_factor     = 1;
    particle_property.m_latency_threshold        = -1.0;
    particle_property.m_job_id_pack_size         = 1;


    particle_property.m_sampling_step  = CalculateSamplingStep( mvpl ) / particle_property.m_extra_opacity_factor;
    particle_property.m_subpixel_level = CalculateSubpixelLevel( particle_property, mvpl, *particle_property.m_camera );

    // ユーザーが伝達関数を指定している場合
    if ( transfer_function_file_name != "" )
    {
        std::cout << "user define parameter " << std::endl;

        // 伝達関数ファイルから伝達関数を設定する処理の実装する, クライアントにサンプルがあるはず
        ParameterFileReader ppr;
        ppr.readTransferFunctionFile( transfer_function_file_name.c_str() );
        ppr.setTransferFunctionParameter( particle_property );
    }
    // ユーザーが伝達関数を指定していない場合
    else
    {
        const int tf_number     = mvpl.m_total_number_ingredients;
        const int TF_resolution = 256;

        std::cout << "default parameter " << std::endl;
        
        particle_property.m_transfunc_array.clear();
        particle_property.m_transfunc_array.resize( tf_number );
 
        // particle_property.m_voleqn.clear();
        // particle_property.m_voleqn.resize( tf_number );

        particle_property.m_color_transfer_function_synthesis   = "C1"; 
        particle_property.m_opacity_transfer_function_synthesis = "O1"; 

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

        for ( std::size_t i = 0; i < tf_number; i++ )
        {
            std::stringstream cc, qq, tt, ff;
            cc << "C"  << i + 1;
            qq << "q"  << i + 1;
            tt << "t"  << i + 1;
            ff << "_F" << i + 1 << "_VAR_";

            particle_property.m_transfunc_array[i].m_name                 = tt.str();
            particle_property.m_transfunc_array[i].m_color_variable       = qq.str();
            particle_property.m_transfunc_array[i].m_opacity_variable     = qq.str();

            particle_property.m_transfunc_array[i].m_server_color_variable_min   = 0;
            particle_property.m_transfunc_array[i].m_server_color_variable_max   = 1; 
            particle_property.m_transfunc_array[i].m_server_opacity_variable_min = 0;
            particle_property.m_transfunc_array[i].m_server_opacity_variable_max = 1;
            particle_property.m_transfunc_array[i].m_user_color_variable_min     = 0;
            particle_property.m_transfunc_array[i].m_user_color_variable_max     = 1; 
            particle_property.m_transfunc_array[i].m_user_opacity_variable_min   = 0;
            particle_property.m_transfunc_array[i].m_user_opacity_variable_max   = 1; 
            particle_property.m_transfunc_array[i].m_resolution                  = TF_resolution;
            particle_property.m_transfunc_array[i].m_equation_red                = ""; 
            particle_property.m_transfunc_array[i].m_equation_green              = ""; 
            particle_property.m_transfunc_array[i].m_equation_blue               = ""; 
            particle_property.m_transfunc_array[i].m_equation_opacity            = "";

            vismodule::ColorMap color_map( cc_table    , 0, 1 );
            vismodule::OpacityMap opacity_map( oo_table, 0, 1 );

            particle_property.m_transfunc_array[i].setColorMap( color_map );
            particle_property.m_transfunc_array[i].setOpacityMap( opacity_map );

            particle_property.m_transfunc_array[i].m_server_color_range_mode   = NamedTransferFunction::ServerRangeMode::ServerSide;
            particle_property.m_transfunc_array[i].m_server_opacity_range_mode = NamedTransferFunction::ServerRangeMode::ServerSide;

            // particle_property.m_voleqn[i].m_name     = ff.str() + "C";
            // particle_property.m_voleqn[i].m_equation = qq.str();

            var_o.push_back( particle_property.m_transfunc_synthesizer->convert_token( qq.str() ) );
            var_c.push_back( particle_property.m_transfunc_synthesizer->convert_token( qq.str() ) );
        }

        particle_property.m_transfunc_synthesizer->setOpacityVariable( var_o );
        particle_property.m_transfunc_synthesizer->setColorVariable( var_c );
    }

    return true;
}

void InitialStepCS(
    std::string& file_path,
    const int time_step, 
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl
)
{
    int rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    rank = 0;
	mpi_size = 1;
#endif

    int st, vl, wid = 0;
    int tf_number;
    VariableRange vr; // jobcollectorで使用
    JobDispatcher jd;
    bool nan_error = false;
    char tmp_sampling_method;

#ifndef CPU_VER
    JobCollector jc( &jd );
#endif

    tf_number = particle_property.m_transfunc_array.size();

    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    float* tmp_max;
    float* tmp_min;

    tmp_c_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_o_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    std::fill_n( tmp_c_bins, DEFAULT_NBINS * tf_number, 0 );
    std::fill_n( tmp_o_bins, DEFAULT_NBINS * tf_number, 0 );

    tmp_max = new float[tf_number * 2]; // color, opacity
    tmp_min = new float[tf_number * 2]; // color, opacity
    std::fill_n( tmp_max, tf_number * 2, FLT_MIN );
    std::fill_n( tmp_min, tf_number * 2, FLT_MAX );

    // サンプリングメソッドをMinMax用に一時的に変更
    tmp_sampling_method = particle_property.m_sampling_method;
    particle_property.m_sampling_method ='x';

    jd.initialize(
        time_step,
        time_step,
        mvpl.m_total_number_subvolumes,
        mvpl.m_total_min_subvolume_coord,
        mvpl.m_total_max_subvolume_coord,
        particle_property.m_latency_threshold,
        particle_property.m_job_id_pack_size
    );

    while ( jd.dispatchNext( wid, &st, &vl ) )
    {
        // calculate min max
        float* job_max = new float[tf_number * 2];
        float* job_min = new float[tf_number * 2];
        std::fill_n( job_max, tf_number * 2, FLT_MIN );
        std::fill_n( job_min, tf_number * 2, FLT_MAX );

        if ( ( rank > 0 ) || ( mpi_size == 1 ) )
        {
            int xvl, fidx;
            fidx = mvpl.getFileIndex( vl, &xvl );
            MultiVolumeProperty& mvp = mvpl.m_list[fidx];
            mvp.setFilePath( file_path, st, xvl );

            // generate point object start
            try
            {
                vismodule::VolumeObjectBase* volume = nullptr;
                vismodule::PointObjectGenerator point_object_generator;
                vismodule::PointObject* tmp_obj = nullptr;

                // generate volume object
                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                {
                    generate_volume( file_path, mvp, st, xvl, volume );
                }
                else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                {
                    generate_volume( file_path, mvp, st, xvl, volume );
                }
                else // filetype: kvsml
                {
                    generate_volume( file_path, mvp, volume );
                }

                if ( !volume )
                {
                    throw std::runtime_error("Failed to generate volume object.");
                }

                float max_opacity;
                float max_density;
                float sampling_volume_inverse;

                if ( particle_property.m_camera == nullptr )
                {
                    std::cout << "ERROR: particle_property.m_camera is NULL." << std::endl;
                }

                vismodule::CellByCellParticleGenerator::CalculateDensityConstaint(
                    *particle_property.m_camera,
                    *volume,
                    static_cast<float>( particle_property.m_subpixel_level ),
                    particle_property.m_sampling_step,
                    &sampling_volume_inverse,
                    &max_opacity,
                    &max_density
                );

                if ( particle_property.m_transfunc_synthesizer == nullptr )
                {
                    std::cout << "ERROR: particle_property.m_transfunc_synthesizer is NULL." << std::endl;
                }

                particle_property.m_transfunc_synthesizer->setMaxOpacity( max_opacity );
                particle_property.m_transfunc_synthesizer->setMaxDensity( max_density );
                particle_property.m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );

                std::unique_ptr<std::unique_ptr<Type[]>[]> values;
                int nvariables = 0;
                int ncoords = 0;
                vismodule::VolumeObjectBase::VolumeType voltype = volume->volumeType();

                if( voltype == vismodule::VolumeObjectBase::VolumeType::Unstructured )
                {
                    domain_parameters_unstruct dom;
                    std::unique_ptr<float[]> coordinates;
                    std::unique_ptr<unsigned int[]> connections;
                    int ncells = 0;
                    vismodule::VolumeObjectBase::CellType celltype;

                    store_volume_in_variables_array_unstruct( volume, dom, values, nvariables, coordinates, ncoords, connections, ncells, celltype );

                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( std::size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }

                    // generate particle
                    tmp_obj = point_object_generator.GenerateParticleUnstruct( particle_property, dom, raw_pointers_vector.data(), nvariables, coordinates.get(), ncoords, connections.get(), ncells, celltype, ServerMode::CS );
                }
                else // ( voltype == vismodule::VolumeObjectBase::VolumeType::Structured )
                {
                    domain_parameters_struct dom; 
                    std::array<int, 3> resolution;

                    store_volume_in_variables_array_struct( volume, dom, resolution, values, nvariables, ncoords );


                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( std::size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }                        

                    // generate particle
                    tmp_obj = point_object_generator.GenerateParticleStruct( particle_property, dom, raw_pointers_vector.data(), nvariables, ServerMode::CS );
                }

                delete volume;
                delete tmp_obj;
            }
            catch ( const std::runtime_error& e )
            {
#ifdef _DEBUG // debug by @hira
                printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
                std::cerr << e.what();
                nan_error = true;
            }
            // generate point object end

            MakeParticleMinMax( particle_property.m_transfunc_synthesizer, tf_number, job_max, job_min );

            for ( int i = 0; i < tf_number * 2; ++i )
            {
                tmp_max[i] = std::max( tmp_max[i], job_max[i] );
                tmp_min[i] = std::min( tmp_min[i], job_min[i] );
            }
        } // calculate minmax

#ifndef CPU_VER
        if ( mpi_size > 1 )
        {
            jc.jobCollect_done( &nan_error, &wid );
        }
#endif

        delete[] job_max;
        delete[] job_min;
    } // end of while(DispatchNext)

//min,max　のMPIプロセス間集約
#ifndef CPU_VER
    if ( mpi_size > 1 )
    {
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, ( tf_number * 2 ), MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, ( tf_number * 2 ), MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    }
#endif

    vr = setVariablerange2( tmp_max, tmp_min, tf_number );
    vr.show();

    for( std::size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream ss; 
        ss << (i + 1); 
        const std::string idxbuf = ss.str();

        float color_variable_min   = vr.min( "t" + idxbuf + "_var_c" );
        float color_variable_max   = vr.max( "t" + idxbuf + "_var_c" );
        float opacity_variable_min = vr.min( "t" + idxbuf + "_var_o" );
        float opacity_variable_max = vr.max( "t" + idxbuf + "_var_o" );

        // MinMaxの格納
        particle_property.m_transfunc_array[i].m_server_color_variable_min   = color_variable_min;
        particle_property.m_transfunc_array[i].m_server_color_variable_max   = color_variable_max;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_min = opacity_variable_min;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_max = opacity_variable_max;

        // ColorMap,OpacityMapの更新
        if( particle_property.m_transfunc_array[i].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        particle_property.m_transfunc_array[i].setColorRange( color_variable_min, color_variable_max);
        if( particle_property.m_transfunc_array[i].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        particle_property.m_transfunc_array[i].setOpacityRange( opacity_variable_min, opacity_variable_max );
    }

    // 本命の粒子生成のためjob dispatch のパラメータ初期化 
    wid = 0; st = 0; vl = 0;

    // サンプリングメソッドをHistogram用に一時的に変更
    particle_property.m_sampling_method = 'h';

    jd.initialize(
        time_step,
        time_step,
        mvpl.m_total_number_subvolumes,
        mvpl.m_total_min_subvolume_coord,
        mvpl.m_total_max_subvolume_coord,
        particle_property.m_latency_threshold,
        particle_property.m_job_id_pack_size
    );

    while ( jd.dispatchNext( wid, &st, &vl ) )
    {
        vismodule::PointObject* send_obj = nullptr;
        vismodule::UInt64* job_c_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
        vismodule::UInt64* job_o_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
        std::fill_n( job_c_bins, DEFAULT_NBINS * tf_number, 0 );
        std::fill_n( job_o_bins, DEFAULT_NBINS * tf_number, 0 );

        // make point object and histgram and range
        if ( ( rank > 0 ) || ( mpi_size == 1 ) )
        {
            int xvl, fidx;
            fidx = mvpl.getFileIndex( vl, &xvl );
            MultiVolumeProperty& mvp = mvpl.m_list[fidx];
            mvp.setFilePath( file_path, st, xvl );

            // generate point object start
            try
            {
                vismodule::VolumeObjectBase* volume = nullptr;
                vismodule::PointObjectGenerator point_object_generator;

                // generate volume object
                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                {
                    generate_volume( file_path, mvp, st, xvl, volume );
                }
                else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                {
                    generate_volume( file_path, mvp, st, xvl, volume );
                }
                else // filetype: kvsml
                {
                    generate_volume( file_path, mvp, volume );
                }

                if ( !volume )
                {
                    throw std::runtime_error("Failed to generate volume object.");
                }

                float max_opacity;
                float max_density;
                float sampling_volume_inverse;

                if ( particle_property.m_camera == nullptr )
                {
                    std::cout << "ERROR: particle_property.m_camera is NULL." << std::endl;
                }

                vismodule::CellByCellParticleGenerator::CalculateDensityConstaint(
                    *particle_property.m_camera,
                    *volume,
                    static_cast<float>( particle_property.m_subpixel_level ),
                    particle_property.m_sampling_step,
                    &sampling_volume_inverse,
                    &max_opacity,
                    &max_density
                );

                if ( particle_property.m_transfunc_synthesizer == nullptr )
                {
                    std::cout << "ERROR: particle_property.m_transfunc_synthesizer is NULL." << std::endl;
                }

                particle_property.m_transfunc_synthesizer->setMaxOpacity( max_opacity );
                particle_property.m_transfunc_synthesizer->setMaxDensity( max_density );
                particle_property.m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );

                std::unique_ptr<std::unique_ptr<Type[]>[]> values;
                int nvariables = 0;
                int ncoords = 0;
                vismodule::VolumeObjectBase::VolumeType voltype = volume->volumeType();

                if( voltype == vismodule::VolumeObjectBase::VolumeType::Unstructured )
                {
                    domain_parameters_unstruct dom;
                    std::unique_ptr<float[]> coordinates;
                    std::unique_ptr<unsigned int[]> connections;
                    int ncells = 0;
                    vismodule::VolumeObjectBase::CellType celltype;

                    store_volume_in_variables_array_unstruct( volume, dom, values, nvariables, coordinates, ncoords, connections, ncells, celltype );

                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( std::size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }

                    // generate particle
                    send_obj = point_object_generator.GenerateParticleUnstruct( particle_property, dom, raw_pointers_vector.data(), nvariables, coordinates.get(), ncoords, connections.get(), ncells, celltype, ServerMode::CS );
                }
                else // ( voltype == vismodule::VolumeObjectBase::VolumeType::Structured )
                {
                    domain_parameters_struct dom; 
                    std::array<int, 3> resolution;

                    store_volume_in_variables_array_struct( volume, dom, resolution, values, nvariables, ncoords );

                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( std::size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }                        

                    // generate particle
                    send_obj = point_object_generator.GenerateParticleStruct( particle_property, dom, raw_pointers_vector.data(), nvariables, ServerMode::CS );
                }

                delete volume;
            }
            catch ( const std::runtime_error& e )
            {
#ifdef _DEBUG // debug by @hira
                printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
                std::cerr << e.what();
                nan_error = true;
            }
            // generate point object end

            if ( send_obj )
            {
                MakeHistgram( send_obj, tf_number, job_c_bins, job_o_bins );
                for ( int i = 0; i < DEFAULT_NBINS * tf_number; ++i )
                {
                    tmp_c_bins[i] += job_c_bins[i];
                    tmp_o_bins[i] += job_o_bins[i];
                }
            }
        } // make point object and histgram and range

#ifndef CPU_VER
        if ( mpi_size > 1 )
        {
            jc.jobCollect_done( &nan_error, &wid );
        }
#endif

        delete send_obj;
        delete[] job_c_bins;
        delete[] job_o_bins;
    } // end of while(DispatchNext)

#ifndef CPU_VER
    if ( mpi_size > 1 )
    {
        MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, ( DEFAULT_NBINS * tf_number ), MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, ( DEFAULT_NBINS * tf_number ), MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
    }
#endif

    if ( rank == 0 )
    {
        // histogramの格納
        for( int i = 0; i < tf_number; i++ )
        {
            std::copy( tmp_c_bins + ( DEFAULT_NBINS * i ), tmp_c_bins + ( DEFAULT_NBINS * ( i + 1 ) ), particle_property.m_transfunc_array[i].m_color_histogram );
            std::copy( tmp_o_bins + ( DEFAULT_NBINS * i ), tmp_o_bins + ( DEFAULT_NBINS * ( i + 1 ) ), particle_property.m_transfunc_array[i].m_opacity_histogram );
        }
    }

    nan_error = false;

    // 一時的に変更していたサンプリングメソッドを元に戻す
    particle_property.m_sampling_method = tmp_sampling_method;

    delete[] tmp_min;
    delete[] tmp_max;
    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
}

static std::string EnvValueOrUnsetIS( const char* name )
{
    const char* value = std::getenv( name );
    return value ? std::string( value ) : std::string( "(unset)" );
}

void SetFallbackParticleParameterIS(
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    const int nvariables
)
{
    const int tf_number     = nvariables > 0 ? nvariables : 1;
    const int TF_resolution = 256;

    particle_property.m_level_index              = 1;
    particle_property.m_repeat_level             = 4;
    particle_property.m_sampling_method          = 'u';
    particle_property.m_particle_data_size_limit = 20;
    particle_property.m_particle_limit           = 10000000;
    particle_property.m_extra_opacity_factor     = 1;
    particle_property.m_latency_threshold        = -1.0;
    particle_property.m_job_id_pack_size         = 1;
    particle_property.m_color_transfer_function_synthesis   = "C1";
    particle_property.m_opacity_transfer_function_synthesis = "O1";
    particle_property.m_transfunc_array.clear();
    particle_property.m_transfunc_array.resize( tf_number );
    mvpl.m_total_number_ingredients = tf_number;

    vismodule::TransferFunction default_transfer_function( TF_resolution );
    default_transfer_function.setColorRange( 0, 1 );
    default_transfer_function.setOpacityRange( 0, 1 );

    std::vector<EquationToken> var_o;
    std::vector<EquationToken> var_c;

    EquationToken eq = particle_property.m_transfunc_synthesizer->convert_token( "a1" );
    particle_property.m_transfunc_synthesizer->setOpacityFunction( eq );

    eq = particle_property.m_transfunc_synthesizer->convert_token( "c1" );
    particle_property.m_transfunc_synthesizer->setColorFunction( eq );

    for ( int i = 0; i < tf_number; i++ )
    {
        std::stringstream qq, tt;
        qq << "q" << i + 1;
        tt << "t" << i + 1;

        NamedTransferFunction named_transfer_function( default_transfer_function );
        named_transfer_function.m_name                         = tt.str();
        named_transfer_function.m_color_variable               = qq.str();
        named_transfer_function.m_opacity_variable             = qq.str();
        named_transfer_function.m_server_color_variable_min    = 0;
        named_transfer_function.m_server_color_variable_max    = 1;
        named_transfer_function.m_server_opacity_variable_min  = 0;
        named_transfer_function.m_server_opacity_variable_max  = 1;
        named_transfer_function.m_user_color_variable_min      = 0;
        named_transfer_function.m_user_color_variable_max      = 1;
        named_transfer_function.m_user_opacity_variable_min    = 0;
        named_transfer_function.m_user_opacity_variable_max    = 1;
        named_transfer_function.m_server_color_range_mode      = NamedTransferFunction::ServerRangeMode::ServerSide;
        named_transfer_function.m_server_opacity_range_mode    = NamedTransferFunction::ServerRangeMode::ServerSide;
        named_transfer_function.m_resolution                   = TF_resolution;
        particle_property.m_transfunc_array[i] = named_transfer_function;

        var_o.push_back( particle_property.m_transfunc_synthesizer->convert_token( qq.str() ) );
        var_c.push_back( particle_property.m_transfunc_synthesizer->convert_token( qq.str() ) );
    }

    particle_property.m_transfunc_synthesizer->setOpacityVariable( var_o );
    particle_property.m_transfunc_synthesizer->setColorVariable( var_c );
    particle_property.m_camera->setWindowSize( 620, 620 );
}

bool SetDefaultParticleParameterIS(
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl
)
{
    const char* env_buf = std::getenv( "VIS_PARAM_DIR" );
    std::string parameter_dir = ( env_buf == nullptr ) ? "./" : env_buf;
    if ( !parameter_dir.empty() && parameter_dir.back() != '/' )
    {
        parameter_dir += "/";
    }

    env_buf = std::getenv( "TF_NAME" );
    const std::string tf_name = ( env_buf == nullptr ) ? "default" : env_buf;
    const std::string json_path = parameter_dir + tf_name + ".json";
    const std::string json_old_path = parameter_dir + tf_name + "_old.json";

    MultiVolumeProperty mvp;

    particle_property.m_level_index              = 1;
    particle_property.m_repeat_level             = 4;
    particle_property.m_particle_data_size_limit = 20;

    // Using environment variables, the constructor of the ParticleMonitor class
    // set particle file, glyph file, plot over line file, status file, history file,
    // and the min/max coordinates of the object.
    ParticleMonitor pm;
    pm.check();

    if( pm.stepExisted() )
    {
        pm.setTimeStep_particle( pm.particleStatusFile().getStartTimeStep() );
    }
    else
    {
        pm.setTimeStep_particle(0);
        std::cout << "WARN:particle status file does not exist" << std::endl;
    }
    pm.readParticleFile();
    pm.readParticleHistoryFile();                

    // store particle monitor in mvpl
    mvpl.m_total_start_steps        = pm.particleStatusFile().getStartTimeStep();
    mvpl.m_total_last_step          = pm.particleStatusFile().getLatestTimeStep();
    mvpl.m_total_number_steps       = mvpl.m_total_last_step - mvpl.m_total_start_steps + 1;
    mvp.m_file_type                 = 0;
    mvp.m_elem_type                 = 0;
    mvpl.m_list.push_back(mvp);
    mvpl.m_total_number_ingredients = pm.particleHistoryFile().nVariables();
    mvpl.m_total_number_elements    = 0;
    mvpl.m_total_number_nodes       = 0;
    mvpl.m_total_number_subvolumes  = 1;
    mvpl.m_total_min_object_coord   = pm.getMinObjectCoords();
    mvpl.m_total_max_object_coord   = pm.getMaxObjectCoords();
    mvpl.m_total_min_value          = 0;
    mvpl.m_total_max_value          = 0;

    // store particle monitor in param
    particle_property.m_is_ensemble = pm.particleHistoryFile().isEnsemble();
    // sampling step is not used in IS mode
    particle_property.m_subpixel_level       = pm.getSubpixelLevel();
    // particle limit and particle density will be overwritten later
    // when transfer function file is readed(ParameterFileReader)
    particle_property.m_particle_limit       = pm.particleHistoryFile().ParticleLimit();
    
    // 一時的にハードコーディング
    // particle_property.m_extra_opacity_factor = pm.particleHistoryFile().ExtraOpacityFactor();
    particle_property.m_extra_opacity_factor = 1;

    ParameterFileReader ppr;
    bool loaded = ppr.readTransferFunctionFromJson( json_path.c_str(), particle_property );
    if ( !loaded )
    {
        loaded = ppr.readTransferFunctionFromJson( json_old_path.c_str(), particle_property );
    }
    if ( !loaded )
    {
        std::cout << "================================================================" << std::endl;
        std::cout << "[WARN] Failed to load transfer function json." << std::endl;
        std::cout << "[WARN] Files: " << json_path << " and " << json_old_path << std::endl;
        std::cout << "[INFO] VIS_PARAM_DIR = " << EnvValueOrUnsetIS( "VIS_PARAM_DIR" ) << std::endl;
        std::cout << "[INFO] PARTICLE_DIR  = " << EnvValueOrUnsetIS( "PARTICLE_DIR" ) << std::endl;
        std::cout << "[INFO] Set default particle parameters." << std::endl;
        std::cout << "================================================================" << std::endl;

        SetFallbackParticleParameterIS(
            particle_property,
            mvpl,
            pm.particleHistoryFile().nVariables()
        );
    }
    return true;
}

void InitialStepIS(
    const int time_step,
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl
)
{
    int tf_number;
    ParticleMonitor pm;
    VariableRange vr;

    std::cout << "time_step:" << time_step << std::endl;

    pm.check();

    if( pm.stepExisted() )
    {
        // pm.setTimeStep_particle( pm.particleStatusFile().getLatestTimeStep() );
        pm.setTimeStep_particle( time_step );
    }
    else
    {
        pm.setTimeStep_particle(0);
    }
    pm.readParticleHistoryFile();

    tf_number = pm.particleHistoryFile().colorHistogramArray().size();

    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    tmp_c_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_o_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    std::fill_n( tmp_c_bins, DEFAULT_NBINS * tf_number, 0 );
    std::fill_n( tmp_o_bins, DEFAULT_NBINS * tf_number, 0 );

    // get histgram start
    int c_count = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        for ( int res = 0; res < DEFAULT_NBINS; res++ )
        {
            tmp_c_bins[c_count] = pm.particleHistoryFile().colorHistogramArray()[tf][res];
            c_count++;
        }
    }
    
    int o_count = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        for ( int res = 0; res < DEFAULT_NBINS; res++ )
        {
            tmp_o_bins[o_count] = pm.particleHistoryFile().opacityHistogramArray()[tf][res];
            o_count++;
        }
    }
    // get histgram end
    
    vr = pm.particleHistoryFile().variableRange();
    vr.show();

    // histogram, minmaxの格納
    for( int i = 0; i < tf_number; i++ )
    {
        // histogram
        std::copy( tmp_c_bins + ( DEFAULT_NBINS * i ), tmp_c_bins + ( DEFAULT_NBINS * ( i + 1 ) ), particle_property.m_transfunc_array[i].m_color_histogram );
        std::copy( tmp_o_bins + ( DEFAULT_NBINS * i ), tmp_o_bins + ( DEFAULT_NBINS * ( i + 1 ) ), particle_property.m_transfunc_array[i].m_opacity_histogram );

        // minmax
        std::stringstream ss; 
        ss << (i + 1); 
        const std::string idxbuf = ss.str();
        particle_property.m_transfunc_array[i].m_server_color_variable_min   = vr.min( "t" + idxbuf + "_var_c" );
        particle_property.m_transfunc_array[i].m_server_color_variable_max   = vr.max( "t" + idxbuf + "_var_c" );
        particle_property.m_transfunc_array[i].m_server_opacity_variable_min = vr.min( "t" + idxbuf + "_var_o" );
        particle_property.m_transfunc_array[i].m_server_opacity_variable_max = vr.max( "t" + idxbuf + "_var_o" );
    }

    if ( pm.particleHistoryFile().hasEnsembleStatisticHistogram() )
    {
        ApplyStatisticHistoryToTransferFunctions(
            pm.particleHistoryFile().averageColorHistogramArray(),
            pm.particleHistoryFile().averageOpacityHistogramArray(),
            pm.particleHistoryFile().averageVariableRange(),
            particle_property.m_mean_transfer_function_array,
            "average" );
        ApplyStatisticHistoryToTransferFunctions(
            pm.particleHistoryFile().varianceColorHistogramArray(),
            pm.particleHistoryFile().varianceOpacityHistogramArray(),
            pm.particleHistoryFile().varianceVariableRange(),
            particle_property.m_variance_transfer_function_array,
            "variance" );
        ApplyStatisticHistoryToTransferFunctions(
            pm.particleHistoryFile().coefficientOfVariationColorHistogramArray(),
            pm.particleHistoryFile().coefficientOfVariationOpacityHistogramArray(),
            pm.particleHistoryFile().coefficientOfVariationVariableRange(),
            particle_property.m_coefficient_of_variation_transfer_function_array,
            "cv" );
    }
    else
    {
        std::cout << "[InitialStepIS] ensemble statistic history is not available." << std::endl;
    }

    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
}
