#include <fstream>
#include <cstdio>
#include <vector>
#include <sstream>
#include <iomanip>
#include <vismodule/Vector3>
#include <vismodule/AnyValueArray>
#include <vismodule/ValueArray>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/PointObject>
#include <vismodule/PointExporter>
#include <vismodule/KVSMLObjectPoint>
#include <vismodule/TransferFunction>
#include <vismodule/RGBColor>
#include <vismodule/Timer>
#include <vismodule/MersenneTwister>

#include <vismodule/GlyphSeed>
#include <vismodule/PlotOverLine>
#include "kvs_wrapper.h"
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/CellByCellRejectionSampling>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/ParamInfo>
#include <vismodule/TrilinearInterpolator>
#include "float.h"

#ifndef CPU_VER
#include <mpi.h>
#endif

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

// Asynchronous io, using worker thread pwt.
#include "particle_write_thread.h"
bool async_io_enabled = false;
static bool is_initial_step = false;
static size_t start_time_step = 0;

pbvr::ParticleWriteThread pwt;
/**
 * @brief begin_wrapper_async_io , call to begin async wrapper output
 */
void begin_wrapper_async_io()
{
    async_io_enabled=true;
}
/**
 * @brief end_wrapper_async_io , call to end async wrapper ouput - and wait for last worker thread to finish.
 */
void end_wrapper_async_io()
{
    if(async_io_enabled)
    {
        pwt.join(true);
        async_io_enabled=false;
    }
}

namespace Generator = vismodule::CellByCellParticleGenerator;

namespace
{
inline size_t CalculateSubpixelLevel( const int particle_limit,
                                      const vismodule::Camera& camera,
                                      const float sampling_step,
                                      const double total_volume,
                                      const vismodule::ObjectBase* volume )
{
    namespace Generator = vismodule::CellByCellParticleGenerator;
    double d_nparticles = 0.0;//particle density for subpixel_level=1
    d_nparticles = Generator::CalculateGreatDensity( &camera, *volume, 1,
                                                      sampling_step ) * total_volume;

    //Calculation of optimized subpixel level
    float plimit = static_cast<float>( particle_limit );
    float nparticles = static_cast<float>( d_nparticles );
    float subpixel_level = sqrt( plimit / nparticles );

    if ( subpixel_level < 1 ) subpixel_level = 1;

    return static_cast<size_t>( subpixel_level + 0.5f );
}

bool LoadParameterFile( ParamInfo*  param_info,
                        const std::string& filename,
                        const std::string& old_filename )
{
    ParamInfo& param = (*param_info);
    bool opend;
    int size = 0;
    char* buf;

    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    if( mpi_rank == 0 )
    {
        bool opened;

        std::ifstream fin( filename.c_str(), std::ios::in );
        opened = fin.is_open();

        if( opened )
        { 
            std::cout << "load parameter file" << std::endl;
            /*ãã¡ã¤ã«ãä¸å®å¨ã®å ´åãå®å¨ã«ãªãã¾ã§ä¸æåæ­¢ããã*/
            param.LoadIN( filename );
            while( param.getString( "END_PARAMETER_FILE" ) != "SUCCESS" )
            {
                param.LoadIN( filename );
            }
            size = param.byteSize();
            if( size > 0 )
            {
                buf = new char [size];
                param.pack( buf );
            }
        }
        else
        {
            size = 0;
        }
    }

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    if( size > 0 )
    {
        if( mpi_rank > 0 ) buf = new char [size];
#ifndef CPU_VER
        MPI_Bcast( buf, size, MPI_CHARACTER, 0, MPI_COMM_WORLD );
#endif
        param.unpack( buf );
        delete[] buf;

        if( mpi_rank == 0 )
        {
            std::rename( filename.c_str(), old_filename.c_str() );
        }
        opend = true;
    }
    else
    {
        opend = false;
    }

    return opend;
}
}//end of unnamed namespace

void SetPOLParameter(jpv::ParticleTransferClientMessage* clntMes  ,const int time_step, plot_over_line_parameters& pol_param )
{



#if 1
    std::string visParamDir;
    std::string POLParamPath;
    std::string POLParamPath_old;
    std::string POLFilePath;

    const char *envBuf = NULL;
    envBuf = std::getenv( "VIS_PARAM_DIR" );
    if (envBuf == NULL) {
        visParamDir = "./";
    }
    else {
        visParamDir = envBuf;
        if (visParamDir[visParamDir.size() - 1] != '/') {
            visParamDir += "/";
        }
    }
    envBuf = std::getenv( "PARTICLE_DIR" );
    if (envBuf == NULL) {
        POLFilePath = "./p_";
    }
    else {
        POLFilePath = envBuf;
        if (POLFilePath[POLFilePath.size() - 1] != '/') {
            POLFilePath += "/p_";
        }
        else {
            POLFilePath += "p_";
        }
    }

    POLParamPath = visParamDir + "parameter.pol";
    POLParamPath_old = visParamDir + "parameter_old.pol";

    pol_param.m_POLParamPath = POLParamPath;
    pol_param.m_POLFilePath = POLFilePath;


    PlotOverLineProperty plot_over_line_property;

    bool read_flag;
    while( plot_over_line_property.getString( "END_PARAMETER_FILE" ) != "SUCCESS" )
    {
        read_flag =  plot_over_line_property.LoadIN(POLParamPath) ;
    }

    int mpi_rank;
#ifndef CPU_VER 
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    if(read_flag)
    {

        bool plot_flag;
        std::string              p_flag                    = plot_over_line_property.getString( "PLOT_FLAG" );
        int sampling_size                                     = plot_over_line_property.getInt("SAMPLING_SIZE");
        std::string              p_variable                = plot_over_line_property.getString( "PLOT_VARIABLE" );
        clntMes->m_plot_variable = p_variable;
//        clntMes->m_plot_variable = std::atoi(p_variable.substr(1).c_str()) -1;
//        std::cout << "p_variable = " << p_variable << std::endl; 
        std::cout << "clntMes->m_plot_variable = " << clntMes->m_plot_variable  << std::endl;
        std::vector<float> s_table;
        s_table = plot_over_line_property.getTableFloat( "START_POINT" );
        std::vector<float> e_table;
        e_table = plot_over_line_property.getTableFloat( "END_POINT" );

        if(strcmp(p_flag.c_str(), "TRUE") ==0 ) plot_flag = true;
        else plot_flag = false;
        clntMes->m_sampling_size = sampling_size;
        clntMes->m_start_point[0] = s_table[0];
        clntMes->m_start_point[1] = s_table[1];
        clntMes->m_start_point[2] = s_table[2];
        clntMes->m_end_point[0] = e_table[0];
        clntMes->m_end_point[1] = e_table[1];
        clntMes->m_end_point[2] = e_table[2];
        //if (plot_flag || clntMes->m_sampling_size > 0)this->setResolution( clntMes->m_sampling_size );
        clntMes->m_plot_flag = plot_flag;
    }
    else
    {
        clntMes->m_plot_flag = false;
    }
    //return m_plot_flag;
#endif
}

void SetGlyphParameter(jpv::ParticleTransferClientMessage* clntMes  ,const int time_step, glyph_parameters& glyph_param)
{
    // glyph_parameの並列初期化
        glyph_param.m_glyph_vectors.clear();  
        glyph_param.m_glyph_coords.clear();  
        glyph_param.m_glyph_colors.clear();  
        glyph_param.m_glyph_sizes.clear();


    std::string visParamDir;
    std::string glyphParamPath;
    std::string glyphFilePath;

    const char *envBuf = NULL;
    envBuf = std::getenv( "VIS_PARAM_DIR" );
    if (envBuf == NULL) {
        visParamDir = "./";
    }
    else {
        visParamDir = envBuf;
        if (visParamDir[visParamDir.size() - 1] != '/') {
            visParamDir += "/";
        }
    }
    envBuf = std::getenv( "PARTICLE_DIR" );
    if (envBuf == NULL) {
        glyphFilePath = "./g_";
    }
    else {
        glyphFilePath = envBuf;
        if (glyphFilePath[glyphFilePath.size() - 1] != '/') {
             glyphFilePath += "/g_";
        }
        else {
            glyphFilePath += "g_";
        }
    }

    glyphParamPath = visParamDir + "parameter.gly";

    glyph_param.m_glyphParamPath = glyphParamPath;
    glyph_param.m_glyphFilePath = glyphFilePath;

    GlyphProperty glyph_property;

    bool read_flag = glyph_property.LoadIN(glyphParamPath) ;

    bool glyph_flag;
    std::string              g_flag                = glyph_property.getString( "GLYPH_FLAG" );
    std::vector<std::string> direction_variables   = glyph_property.getTableString( "DIRECTION_VARIABLES" );
    std::string              size_sampling_method  = glyph_property.getString("SIZE_SAMPLING_METHOD");
    std::vector<std::string> size_variables        = glyph_property.getTableString( "SIZE_VARIABLES" );
    std::string distribution_modes                 = glyph_property.getString("DISTRIBUTION_MODE");
    int stride                                     = glyph_property.getInt("STRIDE");
    int seed                                       = glyph_property.getInt("SEED");
    float number_of_sample_points                    = glyph_property.getInt("NUMBER_OF_SMAPLING_POINT");
    std::string color_sampling_method              = glyph_property.getString("COLOR_DATA_SAMPLING_METHOD");
    std::vector<std::string> color_data_variables  = glyph_property.getTableString( "COLOR_VARIABLES" );


    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif


#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif


    float glyph_min=0;
    float glyph_max=0;
    glyph_min = glyph_property.getFloat("GLYPH_COLOR_MIN");
    glyph_max = glyph_property.getFloat("GLYPH_COLOR_MAX");
    std::vector<int> i_table;
    i_table = glyph_property.getTableInt( "GLYPH_COLOR_MAP_TABLE" );
    vismodule::ValueArray<vismodule::UInt8> u_table( i_table.size() );
    for( size_t j = 0; j<i_table.size(); j++ ) u_table[j] = (vismodule::UInt8)i_table[j];
//    for( size_t j = 0; j<i_table.size(); j++ ) (vismodule::UInt8)clntMes.m_glyph_color_map_table[j] = (vismodule::UInt8)i_table[j];
    vismodule::ColorMap color_map( u_table, glyph_min, glyph_max);

    clntMes->m_color_map = color_map;
    if(strcmp(g_flag.c_str(), "TRUE") ==0 ) glyph_flag = true;
    else glyph_flag = false;

    if(direction_variables.size() < 3)
    {
        std::cout << "variables number is less 3 numbers !!! Skip glyph generate process !!!" << std::endl;
        clntMes->m_glyph_flag = false;
        return ;
    }
    for (int i = 0; i< 3 ; i++)
    {
        clntMes->m_direction_variable[i] = direction_variables[i];
    }

    if     (size_sampling_method == "Constant"       ) clntMes->m_size_sampling_method    = jpv::DataDefines::Constant;
    else if(size_sampling_method == "SingleVariable" ) clntMes->m_size_sampling_method    = jpv::DataDefines::SingleVariable;
    else if(size_sampling_method == "VariableArray" )  clntMes->m_size_sampling_method    = jpv::DataDefines::VariableArray;
    else
    {
       std::cout << "No selecting Sampling method !!! Skip glyph generate process !!!" << std::endl;
        clntMes->m_glyph_flag = false;
       return ;
    }

    clntMes->m_size_variable.resize(size_variables.size());
    for (int i =0 ; i< size_variables.size(); i++)
    {
        //clntMes->m_size_variables.push_back( std::atoi(size_variables[i].substr(1).c_str()) -1);
        clntMes->m_size_variable[i] = size_variables[i];
    }

    if     (distribution_modes == "AllPoints"           ) clntMes->m_distribution_mode  = jpv::GlyphMode::AllPoints;
    else if(distribution_modes == "EveryNthPoints"      ) clntMes->m_distribution_mode  = jpv::GlyphMode::EveryNthPoints;
    else if(distribution_modes == "UniformDistribution" ) clntMes->m_distribution_mode  = jpv::GlyphMode::UniformDistribution;
    else
    {
       std::cout << "Not selecting Distribution mode !!! Skip glyph generate process !!!" << std::endl;
        clntMes->m_glyph_flag = false;
       return ;
    }

    clntMes->m_stride                  = stride;
    clntMes->m_seed                    = seed;
    if (clntMes->m_distribution_mode  == jpv::GlyphMode::AllPoints ) clntMes->m_stride = 1;
    clntMes->m_number_of_sampling_point = number_of_sample_points;
    if     (color_sampling_method == "Constant"       ) clntMes->m_color_data_sampling_method    = jpv::DataDefines::Constant;
    else if(color_sampling_method == "SingleVariable" ) clntMes->m_color_data_sampling_method    = jpv::DataDefines::SingleVariable;
    else if(color_sampling_method == "VariableArray"  ) clntMes->m_color_data_sampling_method    = jpv::DataDefines::VariableArray;
    else
    {
       std::cout << "No selecting Sampling method !!! Skip glyph generate process !!!" << std::endl;
       clntMes->m_glyph_flag = false;
       return;
    }

    clntMes->m_color_data_variable.resize(color_data_variables.size());
    for (int i =0 ; i< color_data_variables.size(); i++)
    {
        //clntMes->m_color_data_variables.push_back( std::atoi(color_data_variables[i].substr(1).c_str()) - 1);
        clntMes->m_color_data_variable[i] = color_data_variables[i];
    }

#if 1
    std::cout << "m_direction_variables        = " << clntMes->m_direction_variable[0] << ", " << clntMes->m_direction_variable[1]   << std::endl; 
    std::cout << "m_size_sampling_method       = " << static_cast<int>(clntMes->m_size_sampling_method)      << std::endl; 
    if(clntMes->m_size_variable.size() > 0) std::cout << "m_size_variables             = " << clntMes->m_size_variable[0]    << std::endl; 
    std::cout << "m_distribution_modes         = " << static_cast<int>(clntMes->m_distribution_mode )       << std::endl; 
    std::cout << "m_stride                     = " << clntMes->m_stride                    << std::endl; 
    std::cout << "m_seed                       = " << clntMes->m_seed                      << std::endl; 
    std::cout << "m_number_of_sample_points    = " << clntMes->m_number_of_sampling_point   << std::endl; 
    std::cout << "m_color_sampling_method      = " << static_cast<int>(clntMes->m_color_data_sampling_method )    << std::endl; 
    if(clntMes->m_color_data_variable.size() > 0) std::cout << "m_color_data_variables       = " << clntMes->m_color_data_variable[0] <<  std::endl; 
#endif
      //return glyph_flag;
      clntMes->m_glyph_flag = glyph_flag;
}

void generate_particles(
    int time_step,
    domain_parameters_struct dom,
    Type** values, 
    int nvariables
)
{
    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif

    if ( is_initial_step == true )
    {
        is_initial_step = false;
        start_time_step = time_step;
    }

    std::string stateFilePath;
    std::string coordMinMaxFilePath;
    std::string particleFilePrefix;
    std::string glyphFilePrefix;
    std::string plotOverLineFilePrefix;
    std::string tfFilePath;
    std::string tfFilePath_old;
    std::string glyphParameterPath;
    std::string glyphParameterPath_old;
    std::string plotOverLineParameterPath;
    std::string plotOverLineParameterPath_old; 

    SetParameterFilePath(
        time_step,
        historyFilePath,
        stateFilePath,
        coordMinMaxFilePath,
        particleFilePrefix,
        glyphFilePrefix,
        plotOverLineFilePrefix,
        tfFilePath,
        tfFilePath_old,
        tfFilePath_step,
        glyphParameterPath,
        glyphParameterPath_old,
        plotOverLineParameterPath,
        plotOverLineParameterPath_old
    );

    Argument param;
    MultiVolumePropertyList mvpl;
    NameListFile nameListFile;
    param.m_transfunc_synthesizer = new TransferFunctionSynthesizer();

    SetParticleParameter( dom, tfFilePath, tfFilePath_old, param, mvpl, nameListFile );
    SetGlyphParameter( glyphParameterPath, glyphParameterPath_old, param );
    SetPlotOverLineParameter( plotOverLineParameterPath, plotOverLineParameterPath_old, param );

    const int tf_number  = param.m_transfunc_array.size();
    const int resolution = param.m_sampling_size;

    // particle parameters
    std::vector<float> particle_coords;
    std::vector<Byte>  particle_colors;
    std::vector<float> particle_normals;
    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    float* tmp_max;
    float* tmp_min;

    tmp_c_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_o_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_max = new float[tf_number * 2]; // color, opacity
    tmp_min = new float[tf_number * 2]; // color, opacity

    for ( size_t i = 0; i < (DEFAULT_NBINS * tf_number); i++ )
    {
        tmp_c_bins[tf] = 0;
        tmp_o_bins[tf] = 0;
    }

    for ( int i = 0; i < (tf_number * 2); i++ )
    {
        tmp_max[i] = FLT_MIN;
        tmp_min[i] = FLT_MAX;
    }

    // glyph parameters
    std::vector<float>         glyph_coords;
    std::vector<float>         glyph_vectors;
    std::vector<float>         glyph_sizes;
    std::vector<unsigned char> glyph_colors;

    // plot over line parameters
    vismodule::ValueArray<float> values_on_line( resolution );
    vismodule::ValueArray<bool>  mask( resolution, 0 );
    vismodule::ValueArray<float> x_axis( resolution );

    values_on_line.fill( 0x00 );
    mask.fill( false );
    x_axis.fill( 0x00 );

    jpv::ServerMode server_mode = jpv::ServerMode::IS;
    vismodule::PointObject* point_object = nullptr;
    vismodule::CS_PointObjectGenerator point_object_generator;
    point_object = point_object_generator.GenerateParticleStruct(
        param, dom, values, nvariables, server_mode
    );

    MakeParticle( point_object, particle_coords, particle_colors, particle_normals ); // InSitu only
    MakeHistgram( point_object, tf_number, tmp_c_bins, tmp_o_bins ); // CS common
    MakeParticleMinMax( param.m_transfunc_synthesizer, tf_number, tmp_max, tmp_min ); // CS common

    delete point_object;

    if ( param.m_glyph_flag )
    {
        vismodule::KVSMLObjectGlyph* glyph_object = new vismodule::KVSMLObjectGlyph;
        vismodule::GlyphSeedGenerator glyph_creator;
        int number_of_divide = mpi_size;
        glyph_creator.GenerateGlyphStruct(
            param, number_of_divide, dom,
            values, nvariables, glyph_object
        );

        MakeGlyph( glyph_object, glyph_coords, glyph_vectors, glyph_sizes, glyph_colors ); // InSitu only

        delete glyph_object;
    }

    if ( param.m_plot_flag )
    {
        vismodule::KVSMLObjectPlotOverLine* pol_object = new vismodule::KVSMLObjectPlotOverLine;
        PlotOverLineGenerator pol_generator;
        pol_generator.GeneratePOLSstruct(
            param, dom, values, nvariables, pol_object
        );

        MakePlotOverLine( pol_object, resolution, values_on_line, mask, x_axis ); // CS common

        delete pol_object;
    }

    OutputCoordMinMaxFile( dom, coordMinMaxFilePath );

    if ( mpi_rank == 0 )
    {
        nameListFile.setFileName( tfFilePath_step );
        nameListFile.write();
    }

    // データ出力
    OutputParticles(
        param, mvpl, time_step, tf_number, nvariables, particleFilePrefix,
        particleFilePrefix, stateFilePath, histryFilePath, coords, colors,
        normals, tmp_c_bins, tmp_o_bins, tmp_max, tmp_min
    );

    if ( param.m_glyph_flag )
    {
        OutputGlyphs(
            time_step, glyphFilePrefix, glyph_coords,
            glyph_vectors, glyph_sizes, glyph_colors
        );
    }

    if ( param.m_plot_flag )
    {
        OutputLine( time_step, plotOverLineFilePrefix, values_on_line, mask, x_axis );
    }
   
    delete tmp_c_bins;
    delete tmp_o_bins
    delete tmp_max;
    delete tmp_min;
    delete param.m_transfunc_synthesizer;
}

void GlyphObjectGenerator( int time_step,
                         domain_parameters_struct dom,
                         Type** values, int nvariables,
                         const jpv::ParticleTransferClientMessage& clntMes, glyph_parameters& glyph_param) //celltype  enum 型に変更
{

auto safe_append = [](auto& dst, auto const& src, char const* what){
    using size_type = typename std::decay_t<decltype(dst)>::size_type;
    const size_type dsz = dst.size();
    const size_type ssz = src.size();
    const size_type mx  = dst.max_size();

    //　受け渡しの際にセグフォエラーが発生する？これの予防および、調査のためのラムダ関数 
    // オーバーフロー防止（mx - dsz の形で比較）
    if (ssz > mx - dsz) {
        std::ostringstream oss;
        oss << what << ": length_error: size(" << dsz << ") + add(" << ssz
            << ") > max_size(" << mx << ")";
        throw std::length_error(oss.str());
    }
    dst.reserve(dsz + ssz);           // 先に確保しておくと例外の発生箇所がここに来る
    dst.insert(dst.end(), src.begin(), src.end());
};



    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_size = 1;
#endif

     //　グリフ生成処理
        GlyphSeed glyph_seed( clntMes, mpi_size, dom, values, nvariables, true);
    auto const& v = glyph_seed.glyph_directions();
    auto const& c = glyph_seed.glyph_coords();
    auto const& k = glyph_seed.glyph_colors();
    auto const& s = glyph_seed.glyph_sizes();

#if 0
// デバッグ用
     std::cout << "glyph_param.m_glyph_sizes = " << glyph_param.m_glyph_sizes.size() << std::endl; 
     std::cout << " glyph_seed.glyph_sizes() = " <<  glyph_seed.glyph_sizes().size() << std::endl; 
     std::cout << "glyph_param.m_glyph_vectors = " << glyph_param.m_glyph_vectors.size() << std::endl; 
     std::cout << "glyph_seed.glyph_directions() = " << glyph_seed.glyph_directions().size() << std::endl; 
     std::cout << "m_glyph_coords.size() = " << glyph_param.m_glyph_coords.size() << std::endl; 
     std::cout << "glyph_seed.glyph_coords().size() = " << glyph_seed.glyph_coords().size() << std::endl; 
     std::cout << "m_glyph_coords.size() = " << glyph_param.m_glyph_colors.size()     << std::endl; 
     std::cout << "glyph_seed.glyph_colors() = " << glyph_seed.glyph_colors().size()  << std::endl; 

    std::cout << "vectors: cur=" << glyph_param.m_glyph_vectors.size()
              << " add=" << v.size() << " max=" << glyph_param.m_glyph_vectors.max_size() << "\n";
    std::cout << "coords : cur=" << glyph_param.m_glyph_coords.size()
              << " add=" << c.size() << " max=" << glyph_param.m_glyph_coords.max_size() << "\n";
    std::cout << "colors : cur=" << glyph_param.m_glyph_colors.size()
              << " add=" << k.size() << " max=" << glyph_param.m_glyph_colors.max_size() << "\n";
    std::cout << "sizes  : cur=" << glyph_param.m_glyph_sizes.size()
              << " add=" << s.size() << " max=" << glyph_param.m_glyph_sizes.max_size() << "\n";

#endif
        // 集約処理
    safe_append(glyph_param.m_glyph_vectors, v, "m_glyph_vectors");
    safe_append(glyph_param.m_glyph_coords , c, "m_glyph_coords");
    safe_append(glyph_param.m_glyph_colors , k, "m_glyph_colors");
    safe_append(glyph_param.m_glyph_sizes  , s, "m_glyph_sizes");
}

void PlotOverLineObjectGenerator( int time_step,
                         domain_parameters_struct dom,
                         Type** values, int nvariables,
                         const jpv::ParticleTransferClientMessage& clntMes, plot_over_line_parameters& pol_param) //celltype  enum 型に変更
{
     //　グリフ生成処理
        PlotOverLine plot_over_line(dom, values, nvariables, clntMes);
        //集約処理
        int size = plot_over_line.sampling_size();
        for(int i =0; i< size; i++)
        {
                    pol_param.m_x_axis[i] = plot_over_line.xAxis()[i];
                if (plot_over_line.mask()[i])
                {
                    pol_param.m_mask[i] = plot_over_line.mask()[i];
                    pol_param.m_values_on_line.at(i) = plot_over_line.values()[i];
                }
        }

}

bool SetParameter(const domain_parameters_struct dom, pbvr_parameters* particleBase, ParamInfo *m_param ,const int time_step)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif
    // Set Transfer function synthesizer.
    particleBase->m_tf.resize(0);
    m_tfs = new TransferFunctionSynthesizer();
    // 20181226 start  環境変数で指定したファイルパスを参照する
    std::string visParamDir;
    std::string tfFilename;
    std::string stateFilePath;
    std::string minmaxFilePath;
    std::string ptcFilePath;

    const char *envBuf = NULL;
    envBuf = std::getenv( "VIS_PARAM_DIR" );
    if (envBuf == NULL) {
        visParamDir = "./";
    }
    else {
        visParamDir = envBuf;
        if (visParamDir[visParamDir.size() - 1] != '/') {
            visParamDir += "/";
        }
    }
    envBuf = std::getenv( "TF_NAME" );
    if (envBuf == NULL) {
        tfFilename = "default";
    }
    else {
        tfFilename = envBuf;
    }
    stateFilePath = visParamDir + "state.txt";
    envBuf = std::getenv( "PARTICLE_DIR" );
    if (envBuf == NULL) {
        minmaxFilePath = "./t_pfi_coords_minmax.txt";
        ptcFilePath = "./t_";
    }
    else {
        minmaxFilePath = envBuf;
        ptcFilePath = envBuf;
        if (minmaxFilePath[minmaxFilePath.size() - 1] != '/') {
            minmaxFilePath += "/t_pfi_coords_minmax.txt";
            ptcFilePath += "/t_";
        }
        else {
            minmaxFilePath += "t_pfi_coords_minmax.txt";
            ptcFilePath += "t_";
        }
    }
    particleBase->m_visParamDir = visParamDir;
    particleBase->m_ptcFilePath = ptcFilePath;
    particleBase->m_stateFilePath = stateFilePath;
    particleBase->m_tfFilename = tfFilename;

    // 20181226 end

    // 20190318 ボリュームサイズのファイル出力
    //全体の最大最小値を示すpfiファイルを生成
    static bool minmaxFlag = false;
    if (minmaxFlag == false && mpi_rank == 0) {
        FILE* fp = fopen( minmaxFilePath.c_str(), "w" );
        if( fp )
        {
            fprintf( fp, "%f %f %f %f %f %f\n",
                     dom.x_min,
                     dom.y_min,
                     dom.z_min,
                     dom.x_max,
                     dom.y_max,
                     dom.z_max );
            fclose( fp );
        }
        minmaxFlag = true;
    }
    // 20190318 end

    //if(mpi->rank==0)std::cout<<"start initializeTFS()\n";
    vismodule::StructuredVolumeObject* object = new vismodule::StructuredVolumeObject();//Global Min Max volume object
    vismodule::Vector3f min_vec(
        dom.x_min,
        dom.y_min,
        dom.z_min);
    vismodule::Vector3f max_vec(
        dom.x_max,
        dom.y_max,
        dom.z_max );
    object->setMinMaxObjectCoords( min_vec, max_vec );
    object->setMinMaxExternalCoords( min_vec, max_vec );
    particleBase->m_min_vec = min_vec;
    particleBase->m_max_vec = max_vec;
    if(mpi_rank==RANK) std::cout<<"max_vec:"<<max_vec<<std::endl;

    std::string sampling_method;

    //TFファイル読み取り
    bool tmp_parameter_file_opened =
        initializeParameters( m_tfs, particleBase->m_tf, m_param, object, &particleBase->m_sampling_volume_inverse, &particleBase->m_max_opacity, &particleBase->m_max_density,
                             &particleBase->m_subpixel_level, &particleBase->m_particle_density, &particleBase->m_particle_limit, &particleBase->m_particle_data_size_limit, visParamDir, tfFilename, &sampling_method, time_step );

    particleBase->m_sampling_method = sampling_method[0];

    int tf_number = particleBase->m_tf.size();

    
    particleBase->m_tf_number = tf_number;
    particleBase->m_parameter_file_opened = true;

    delete object;
    //if(mpi->rank==0)std::cout<<"end initializeTFS()\n";

    //add by shimomura 20240722
    int nbin =256;
    particleBase->m_O_max.allocate(particleBase->m_tf_number);
    particleBase->m_O_min.allocate(particleBase->m_tf_number);
    particleBase->m_C_max.allocate(particleBase->m_tf_number);
    particleBase->m_C_min.allocate(particleBase->m_tf_number);
    for (int i = 0 ; i< particleBase->m_tf_number; i++)
    {
        particleBase->m_O_max[i]=FLT_MIN;
        particleBase->m_O_min[i]=FLT_MAX;
        particleBase->m_C_max[i]=FLT_MIN;
        particleBase->m_C_min[i]=FLT_MAX;
    }
    particleBase->m_o_histogram.allocate(particleBase->m_tf_number*nbin);
    particleBase->m_c_histogram.allocate(particleBase->m_tf_number*nbin);
    particleBase->m_o_histogram.fill(0x00);
    particleBase->m_c_histogram.fill(0x00);

    // TFファイルがないならば、retrun
    if ( generate_flag == false )
    {
        std::cout << "find no .tf!! skipping generate_particle !!!" << std::endl;
        return false;
    }
    // moved by shimomura 20240807

    return true;
}


void GenerateParticles( int time_step,
                         domain_parameters_struct dom,
                         Type** values,
                         int nvariables,  pbvr_parameters& particleBase)
{
#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif

    vismodule::PointObject* tmp_obj = NULL; 
    switch(particleBase.m_sampling_method)
    {
        case 'u':
            std::cout << "Uniform sampling" << std::endl;
            tmp_obj =  new vismodule::CellByCellUniformSampling(  dom, values, nvariables,
                    particleBase.m_tf[0], particleBase.m_tf,
                    m_tfs, particleBase.m_particle_density , NULL);
             break;

        case 'r':
            std::cout << "Rejection sampling" << std::endl;
            tmp_obj =  new vismodule::CellByCellRejectionSampling(  dom, values, nvariables,
                    particleBase.m_tf[0], particleBase.m_tf,
                    m_tfs, particleBase.m_particle_density , NULL);
             break;

        case 'm':
            std::cout << "Metolopolis sampling" << std::endl;
            tmp_obj =  new vismodule::CellByCellMetropolisSampling(  dom, values, nvariables,
                    particleBase.m_tf[0], particleBase.m_tf,
                    m_tfs, particleBase.m_particle_density , NULL);
             break;

        case 'h':
            std::cout << "Histogram " << std::endl;
            tmp_obj =  new vismodule::CellByCellHistogram(  dom, values, nvariables,
                    particleBase.m_tf[0], particleBase.m_tf,
                    m_tfs, particleBase.m_particle_density , NULL);
             break;

        default:
            std::cout << "Unknown sampling method:" << particleBase.m_sampling_method << std::endl;
            return ;
    }

    // 粒子の受け渡し
    size_t nmemb = tmp_obj->nvertices() * 3;
    std::cout << "nmemb = " << nmemb << std::endl;
    vismodule::ValueArray<vismodule::Real32> coords_array ( tmp_obj->coords().pointer(), nmemb );
    vismodule::ValueArray<vismodule::UInt8>  colors_array ( tmp_obj->colors().pointer(), nmemb );
    vismodule::ValueArray<vismodule::Real32> normals_array( tmp_obj->normals().pointer(), nmemb );
    particleBase.m_sample_coords.insert(particleBase.m_sample_coords.end(), coords_array.begin(), coords_array.end());
    particleBase.m_sample_colors.insert(particleBase.m_sample_colors.end(), colors_array.begin(), colors_array.end());
    particleBase.m_sample_normals.insert(particleBase.m_sample_normals.end(), normals_array.begin(), normals_array.end());

    //particleBase.m_sample_coords .assign(coords_array .pointer(), coords_array.pointer() + coords_array.size());
    //particleBase.m_sample_colors .assign(colors_array .pointer(), colors_array.pointer() + colors_array.size());
    //particleBase.m_sample_normals.assign(normals_array.pointer(),normals_array.pointer() +normals_array.size());

    // ヒストグラム、minmaxの受け渡し
    int   tf_number                = particleBase.m_tf_number;
    if (particleBase.m_sampling_method == 'h') tf_number = nvariables;
    for( int i = 0; i < tf_number; i++ )
    {
        //不透明度
        particleBase.m_O_min[i] = m_tfs -> m_o_min[i];
        particleBase.m_O_max[i] = m_tfs -> m_o_max[i];
        //色
        particleBase.m_C_min[i] = m_tfs -> m_c_min[i];
        particleBase.m_C_max[i] = m_tfs -> m_c_max[i];
        std::cout << mpi_rank <<" : particleBase.m_C_min["<< i << "] = " << particleBase.m_C_min[i] << std::endl;
        std::cout << mpi_rank <<" : particleBase.m_C_max["<< i << "] = " << particleBase.m_C_max[i] << std::endl;
    }


    int nbins = 256;
    for( int n = 0; n < tf_number * nbins; n++ )
    {
        particleBase.m_o_histogram[n] += tmp_obj->getOHistogram()[n];
        particleBase.m_c_histogram[n] += tmp_obj->getCHistogram()[n];
    }

}

void OutputParticles(int time_step, int nvariables, pbvr_parameters& particleBase, ParamInfo *param, bool skip_flag)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif
    int tf_number = particleBase.m_tf_number;
    int nbins = 256;

    if (skip_flag)
    {
    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
    vismodule::ValueArray<float> coords( particleBase.m_sample_coords );
    vismodule::ValueArray<Byte>  colors( particleBase.m_sample_colors );
    vismodule::ValueArray<float> normals(particleBase.m_sample_normals );

    static bool first_step = true;
    static int count;
    static int num_nodes;

#ifndef CPU_VER
    static MPI_Comm new_comm;
#endif

    /* 各ノード毎に粒子データを出力する。 */
    if( first_step )
    {
#ifndef CPU_VER
        int numprocs, myrank;
        int resultlen;
        char procname[MPI_MAX_PROCESSOR_NAME];
        char* procname_bak;
        char* procname_g;
        char* procname_p;

        MPI_Comm_size( MPI_COMM_WORLD, &numprocs );
        MPI_Comm_rank( MPI_COMM_WORLD, &myrank );

        /* ノード名を取得し、各ランクで共有する. */
        MPI_Get_processor_name( procname, &resultlen );
        procname_g = new char[ MPI_MAX_PROCESSOR_NAME * numprocs ];
        MPI_Allgather( procname,   MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                       procname_g, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                       MPI_COMM_WORLD );

        int color;
        count = 1;
        for( color = 0; color < numprocs; color++ )
        {
            procname_p = procname_g + MPI_MAX_PROCESSOR_NAME * color;

            /* 要素の隣同士を比較して差異があった場合にカウントし, *
             * ノード毎に連続した番号を割り当てる.                 */
            if( color > 0 )
            {
                procname_bak = procname_p - MPI_MAX_PROCESSOR_NAME;
                if( strcmp( procname_p, procname_bak ) != 0 )
                    count++;
            }

            /* 自分のノード名が一致した要素番号をコミュニケータ分割のcolorとする */
            if( strcmp( procname_p, procname ) == 0 )
                break;
        }

        delete[] procname_g;
        
        MPI_Comm_split( MPI_COMM_WORLD, color, myrank, &new_comm );
        
        int split_numprocs;
        MPI_Comm_size( new_comm, &split_numprocs );
        
        /*
         * 各ノードに均等にランクが割り当てられることを前提とし,
         * 分割前のプロセス数と分割後のプロセス数の非を粒子ファイル数とする.
         */
        num_nodes = numprocs / split_numprocs;
        if( numprocs % split_numprocs > 0 ) num_nodes++;
#else
        count = 1;
        num_nodes = 1;
#endif
        first_step = false;
    }   
    
    /*
     * ファイル名の粒子データのファイル名を入力する.
     * countが各ファイルで連続でない場合,ファイルが不在と見なしてデーモンでスピンロックがかかる.
     */
#if 0
    char filename[256];
    sprintf(filename, "./jupiter_particle_out/t_%05d_",time_step);
    sprintf(filename,"%s%07d_%07d.kvsml", filename, count, num_nodes );
#else
    // 20181226 start  環境変数で指定したファイルパスを参照する
    std::stringstream ss;
    //add by shimomura 20240614
//    ss << std::setfill('0') << std::setw(2) << static_cast<int>(celltype);
//    ss << "_";
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << count;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << num_nodes;
    ss << ".kvsml";
    particleBase.m_ptcFilePath += ss.str();
    // 20181226 end
#endif

    int particle_size = coords.size();
    int *recvcounts;
    int *displs;

    int new_rank;
    int new_number_of_process;
#ifndef CPU_VER
    MPI_Comm_rank( new_comm, &new_rank );
    MPI_Comm_size( new_comm, &new_number_of_process );
#else
    new_rank = 0;
    new_number_of_process = 1;
#endif

    /*
     *  recvcounts: 各ランク毎の受信バッファサイズ.
     *  displs:     受信先バッファ上の各ランク毎の受信バッファの位置(オフセット)
     */

    displs = new int[ new_number_of_process ];
    recvcounts = new int[ new_number_of_process ];

#ifndef CPU_VER
    MPI_Allgather( &particle_size, 1, MPI_INT,
                   recvcounts,     1, MPI_INT,
                   new_comm );
#else
    recvcounts[0] = particle_size;
#endif

    displs[0] = 0;
    for( int i = 1; i < new_number_of_process; i++ )
        displs[i] = displs[i-1] + recvcounts[i-1];

    vismodule::ValueArray<float> new_coords(  displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );
    vismodule::ValueArray<Byte>  new_colors(  displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );
    vismodule::ValueArray<float> new_normals( displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );

#ifndef CPU_VER
    MPI_Gatherv( coords.pointer(),   particle_size, MPI_FLOAT,
                 new_coords.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, new_comm );

    MPI_Gatherv( colors.pointer(),   particle_size, MPI_BYTE,
                 new_colors.pointer(), recvcounts, displs, MPI_BYTE,
                 0, new_comm );

    MPI_Gatherv( normals.pointer(),   particle_size, MPI_FLOAT,
                 new_normals.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, new_comm );
#else
    for( int i = 0; i < particle_size; i++ )
    {
        new_coords[i]  = coords[i];
        new_colors[i]  = colors[i];
        new_normals[i] = normals[i];
    }
#endif

    /*  分割後コミュニケータのランク0で出力する  */
    if( new_rank == 0 )
    {
        vismodule::PointObject* point_object = new vismodule::PointObject( new_coords, new_colors, new_normals, particleBase.m_subpixel_level );
        point_object->setMinMaxObjectCoords( particleBase.m_min_vec, particleBase.m_max_vec );
        // If async_io is enabled, use worker thread to write kvsml data and state.txt
        if (async_io_enabled){
            pbvr::ParticleWriteThread* particle_write_thread =  &pwt;
            particle_write_thread->join(true);
            particle_write_thread->setPointObject( point_object );
            particle_write_thread->setFilename(particleBase.m_ptcFilePath.c_str());
            particle_write_thread->setTimestep(time_step ,particleBase.m_stateFilePath.c_str());
            particle_write_thread->setStartTimestep(st_time_step); //add by shimomura 20240808
            particle_write_thread->work(true);
        }// If async_io is disabled, use kvs::PointExporter here in main thread.
        else{
            vismodule::KVSMLObjectPoint* kvsml_object = new vismodule::PointExporter<vismodule::KVSMLObjectPoint>( *point_object );
            kvsml_object->setWritingDataType( vismodule::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->write( particleBase.m_ptcFilePath.c_str() );
            delete kvsml_object;
        }
        delete point_object;
    }
    
    }// end if skip_flag
    else 
    {
            std::ofstream ofs( particleBase.m_stateFilePath.c_str(), std::ios::out);
            // 20181226 end
            if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

            ofs<<"START_STEP="<< 0 <<std::endl;
            ofs<<"LATEST_STEP="<<time_step<<std::endl;

            ofs.close();
    }


//    timer.stop();
//    time.writting = timer.sec();
//    timer.start();

    //static bool parameter_file_opened= particleBase.m_parameter_file_opened;
    static bool parameter_file_opened= true;
    //ヒストグラム、最大最小値の集計
    if( parameter_file_opened )
    {
        O_min_recv.allocate(tf_number);
        O_max_recv.allocate(tf_number);
        C_min_recv.allocate(tf_number);
        C_max_recv.allocate(tf_number);
        O_min_recv.fill(0x00);
        O_max_recv.fill(0x00);
        C_min_recv.fill(0x00);
        C_max_recv.fill(0x00);

#ifndef CPU_VER
        // if(mpi_rank==0)std::cout<<"MPI_Reduce"<<std::endl;
        MPI_Reduce( particleBase.m_O_min.pointer(), O_min_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_O_max.pointer(), O_max_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_C_min.pointer(), C_min_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_C_max.pointer(), C_max_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );
        // if(mpi_rank==0) std::cout<<"end MPI_Reduce"<<std::endl;
#else
        for( int i = 0; i < tf_number; i++ )
        {
            O_min_recv[i] = particleBase.m_O_min[i];
            O_max_recv[i] = particleBase.m_O_max[i];
            C_min_recv[i] = particleBase.m_C_min[i];
            C_max_recv[i] = particleBase.m_C_max[i];
        }
#endif

        o_histogram_recv.allocate(tf_number * nbins);
        c_histogram_recv.allocate(tf_number * nbins);
        o_histogram_recv.fill(0x00);
        c_histogram_recv.fill(0x00);

#ifndef CPU_VER
        //ヒストグラムの集計
        MPI_Reduce( particleBase.m_o_histogram.pointer(), o_histogram_recv.pointer(),
                    tf_number*nbins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );

        MPI_Reduce( particleBase.m_c_histogram.pointer(), c_histogram_recv.pointer(),
                    tf_number*nbins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
#else
        for( int i = 0; i < (tf_number * nbins); i++ )
        {
            o_histogram_recv[i] = particleBase.m_o_histogram[i];
            c_histogram_recv[i] = particleBase.m_c_histogram[i];
        }
#endif
    }

//    timer.stop();
//    time.mpi_reduce = timer.sec();
//    timer.start();


    //状態ファイルの出力
    if( mpi_rank == 0 )
    {
        // 20181226 start 環境変数で指定したファイルパスを使用
        //std::ofstream ofs( "state.txt", std::ios::out);
        // If async_io is enabled, state.txt will be written from worker thread.
        // If async_io is disabled, state.txt will be written here.
        if (!async_io_enabled){
            std::ofstream ofs( particleBase.m_stateFilePath.c_str(), std::ios::out);
            // 20181226 end
            if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

            ofs<<"START_STEP="<< st_time_step <<std::endl;
            ofs<<"LATEST_STEP="<<time_step<<std::endl;

            ofs.close();
        }
        std::stringstream step;
        step << '_' << std::setw( 5 ) << std::setfill( '0' ) << time_step;

        // 20181226 start 環境変数で指定したファイルパスを使用
        std::string history_file_name = particleBase.m_visParamDir + "history" + step.str() + ".txt";
        // 20181226 end
        std::ofstream ofs2( history_file_name.c_str(), std::ios::out);

        ofs2<<"TF_NUMBER="<<tf_number<<std::endl;
        for( int i = 0; i < tf_number; i++ )
        {
            ofs2<<"MIN_O"<<i+1<<"="<<O_min_recv[i]<<std::endl;
            ofs2<<"MAX_O"<<i+1<<"="<<O_max_recv[i]<<std::endl;
            ofs2<<"MIN_C"<<i+1<<"="<<C_min_recv[i]<<std::endl;
            ofs2<<"MAX_C"<<i+1<<"="<<C_max_recv[i]<<std::endl;
            ofs2<<"RESOLUTION_O"<<i+1<<"="<<nbins<<std::endl;
            ofs2<<"HISTOGRAM_O"<<i+1<<"=";
            for(int j=0; j<nbins; j++)
            {
                ofs2<<o_histogram_recv[j + i*nbins]<<",";
            }
            ofs2<<std::endl;
            ofs2<<"RESOLUTION_C"<<i+1<<"="<<nbins<<std::endl;
            ofs2<<"HISTOGRAM_C"<<i+1<<"=";
            for(int j=0; j<nbins; j++)
            {
                ofs2<<c_histogram_recv[j + i*nbins]<<",";
            }
            ofs2<<std::endl;
        }
        ofs2<<"N_VARIABLES="<<particleBase.m_nvariables<<std::endl;
        ofs2<<"PARTICLE_DENSITY="<<particleBase.m_particle_density<<std::endl;
        ofs2<<"PARTICLE_LIMIT="<<particleBase.m_particle_limit<<std::endl;
        ofs2 << "END_HISTORY_FILE=SUCCESS" << std::endl;
        ofs2.close();

        if (skip_flag)
        {
            // 20181226 start 環境変数で指定したファイルパスを使用
            std::string jupiter_file_name = particleBase.m_visParamDir + particleBase.m_tfFilename + step.str() + ".tf";
            // 20181226 end
            param->write( jupiter_file_name );
        }
    }
//    timer.stop();
//    time.write_text = timer.sec();
//
//    show_timer( time );
    //if(mpi->rank==0)std::cout<<"end generate_particles\n";

}

void OutputGlyphs(const int time_step, glyph_parameters& glyph_param)
{

    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif
    int nbins = 256;

    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
    vismodule::ValueArray<float> coords( glyph_param.m_glyph_coords  );
    vismodule::ValueArray<float> vectors(glyph_param.m_glyph_vectors );
    vismodule::ValueArray<Byte>  colors( glyph_param.m_glyph_colors   );
    vismodule::ValueArray<float> sizes(  glyph_param.m_glyph_sizes  );

    static bool first_step = true;
    // static MPI_Comm new_comm;
    static int count;
    static int num_nodes;

    std::stringstream ss;
    //add by shimomura 20240614
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_rank+1;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_size;
    ss << ".dat";
    glyph_param.m_glyphFilePath += ss.str();
    // 20181226 end
    
    vismodule::KVSMLObjectGlyph kvsmlobject( coords, colors, vectors, sizes);
    kvsmlobject.write(glyph_param.m_glyphFilePath.c_str());

}

void OutputLine( const int time_step, plot_over_line_parameters& pol_param)
{

    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank=0;
    mpi_size=1;
#endif
    int nbins = 256;

    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
    vismodule::ValueArray<float> values_on_line( pol_param.m_values_on_line  );
    vismodule::ValueArray<float> x_axis(         pol_param.m_x_axis );
    vismodule::ValueArray<bool>  mask (          pol_param.m_mask   );
#if 0
    static bool first_step = true;
    static MPI_Comm new_comm;
    static int count;
    static int num_nodes;
#endif

    std::stringstream ss;
    //add by shimomura 20240614
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_rank+1;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_size;
    ss << ".dat";
    pol_param.m_POLFilePath += ss.str();
    // 20181226 end

    vismodule::KVSMLObjectPlotOverLine vismodulemlobject( values_on_line, x_axis, mask);
    vismodulemlobject.write(pol_param.m_POLFilePath.c_str());

}
void state_txt_writer( void )
{
    std::ofstream ofs( "state.txt", std::ios::out);
        if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

        ofs<<"LATEST_STEP=NO_STEP"<<std::endl;

        ofs.close();
}




