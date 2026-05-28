#include "kvs_wrapper_common.h"
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>
#include <vismodule/KVSMLObjectPoint>
#include <vismodule/KVSMLObjectPlotOverLine>
#include <vismodule/KVSMLObjectPlotOverTime>
#include <vismodule/ParameterFileReader>
#include <vismodule/ParameterFileWriter>
#include <vismodule/TransferFunction>
#include <vismodule/TransferFunctionSynthesizer>

#ifndef CPU_VER
#include <mpi.h>
#endif

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

namespace Generator = vismodule::CellByCellParticleGenerator;
bool async_io_enabled = false;
pbvr::ParticleWriteThread pwt;

namespace
{

std::string ToTableString( const std::vector<int32_t>& values )
{
    std::stringstream table;
    for ( size_t i = 0; i < values.size(); i++ )
    {
        table << values[i] << ",";
    }
    return table.str();
}

std::vector<int32_t> DefaultGlyphColorMapTable()
{
    std::vector<int32_t> table = {
        59,76,192,60,78,194,61,80,195,62,81,197,64,83,198,65,85,200,66,86,201,67,88,203,68,90,204,69,92,206,71,93,207,72,95,209,73,97,210,74,99,211,75,100,213,77,102,214,78,104,215,79,105,217,80,107,218,82,109,219,83,110,221,84,112,222,85,114,223,87,115,224,88,117,225,89,119,226,90,120,228,92,122,229,93,124,230,94,125,231,95,127,232,97,128,233,98,130,234,99,132,235,101,133,236,102,135,237,103,136,238,105,138,239,106,139,239,107,141,240,109,142,241,110,144,242,111,145,243,113,147,243,114,148,244,115,150,245,117,151,246,118,153,246,119,154,247,121,156,248,122,157,248,124,159,249,125,160,249,126,161,250,128,163,250,129,164,251,130,166,251,132,167,252,133,168,252,135,170,252,136,171,253,137,172,253,139,173,253,140,175,254,141,176,254,143,177,254,144,178,254,146,180,254,147,181,255,148,182,255,150,183,255,151,184,255,153,185,255,154,187,255,155,188,255,157,189,255,158,190,255,159,191,255,161,192,255,162,193,255,164,194,254,165,195,254,166,196,254,168,197,254,169,198,254,170,199,253,172,200,253,173,201,253,174,201,252,176,202,252,177,203,252,178,204,251,180,205,251,181,206,250,182,206,250,183,207,249,185,208,249,186,209,248,187,209,248,189,210,247,190,211,246,191,211,246,192,212,245,193,212,244,195,213,244,196,214,243,197,214,242,198,215,241,200,215,241,201,216,240,202,216,239,203,216,238,204,217,237,205,217,236,206,218,235,208,218,234,209,218,233,210,219,232,211,219,231,212,219,230,213,219,229,214,220,228,215,220,227,216,220,226,217,220,225,218,220,224,219,221,222,220,221,221,221,220,220,222,220,219,223,219,217,225,219,216,226,218,214,227,218,213,228,217,211,229,217,210,229,216,209,230,216,207,231,215,206,232,214,204,233,214,203,234,213,201,235,212,200,235,211,198,236,211,197,237,210,195,238,209,194,238,208,192,239,207,191,239,206,189,240,206,187,241,205,186,241,204,184,242,203,183,242,202,181,243,201,180,243,200,178,244,199,177,244,198,175,244,197,173,245,196,172,245,195,170,245,193,169,246,192,167,246,191,166,246,190,164,246,189,162,247,188,161,247,186,159,247,185,158,247,184,156,247,183,155,247,181,153,247,180,151,247,179,150,247,178,148,247,176,147,247,175,145,247,173,143,247,172,142,247,171,140,247,169,139,247,168,137,247,166,136,246,165,134,246,163,132,246,162,131,246,160,129,245,159,128,245,157,126,245,156,125,244,154,123,244,153,122,244,151,120,243,149,119,243,148,117,242,146,116,242,144,114,241,143,112,241,141,111,240,139,109,240,138,108,239,136,106,239,134,105,238,133,104,237,131,102,237,129,101,236,127,99,235,125,98,234,124,96,234,122,95,233,120,93,232,118,92,231,116,90,230,114,89,229,112,88,229,111,86,228,109,85,227,107,83,226,105,82,225,103,81,224,101,79,223,99,78,222,97,77,221,95,75,220,93,74,219,91,73,218,89,71,216,86,70,215,84,69,214,82,67,213,80,66,212,78,65,211,76,63,209,73,62,208,71,61,207,69,60,206,67,59,204,64,57,203,62,56,202,59,55,200,57,54,199,54,52,198,52,51,196,49,50,195,46,49,193,43,48,192,40,47,191,37,46,189,34,44,188,30,43,186,26,42,185,22,41,183,17,40,182,11,39,180,4,38
    };
    return table;
}

NameListFile DefaultGlyphParameterNameListFile()
{
    NameListFile nameListFile;
    nameListFile.setLine( "GLYPH_FLAG", "FALSE" );
    nameListFile.setLine( "SCALE_FACTOR", 1 );
    nameListFile.setLine( "STRIDE", 3 );
    nameListFile.setLine( "SEED", 1 );
    nameListFile.setLine( "NUMBER_OF_SMAPLING_POINT", 1000 );
    nameListFile.setLine( "GLYPH_COLOR_MAX", 1 );
    nameListFile.setLine( "GLYPH_COLOR_MIN", 0 );
    nameListFile.setLine( "GLYPH_TYPE", "Arrow" );
    nameListFile.setLine( "SIZE_VARIABLES", "q1," );
    nameListFile.setLine( "COLOR_VARIABLES", "q1," );
    nameListFile.setLine( "DIRECTION_VARIABLES", "q1,q2,q3," );
    nameListFile.setLine( "DISTRIBUTION_MODE", "UniformDistribution" );
    nameListFile.setLine( "SIZE_SAMPLING_METHOD", "Constant" );
    nameListFile.setLine( "COLOR_DATA_SAMPLING_METHOD", "Constant" );
    nameListFile.setLine( "GLYPH_COLOR_MAP_TABLE", ToTableString( DefaultGlyphColorMapTable() ) );
    nameListFile.setLine( "END_PARAMETER_FILE", "SUCCESS" );
    return nameListFile;
}

NameListFile DefaultPlotOverLineParameterNameListFile()
{
    NameListFile nameListFile;
    nameListFile.setLine( "PLOT_FLAG", "FALSE" );
    nameListFile.setLine( "PLOT_VARIABLE", "q1" );
    nameListFile.setLine( "SAMPLING_SIZE", 256 );
    nameListFile.setLine( "START_POINT", "0,0,0," );
    nameListFile.setLine( "END_POINT", "1,1,1," );
    nameListFile.setLine( "END_PARAMETER_FILE", "SUCCESS" );
    return nameListFile;
}

NameListFile DefaultPlotOverTimeParameterNameListFile()
{
    NameListFile nameListFile;
    nameListFile.setLine( "PLOT_FLAG", "FALSE" );
    nameListFile.setLine( "TARGET_POINT", "0,0,0," );
    nameListFile.setLine( "END_PARAMETER_FILE", "SUCCESS" );
    return nameListFile;
}

void BroadcastNameListFile( NameListFile& nameListFile, const int mpi_rank )
{
    int size = 0;
    char* buf = NULL;

    if ( mpi_rank == 0 )
    {
        size = static_cast<int>( nameListFile.byteSize() );
        if ( size > 0 )
        {
            buf = new char[size];
            nameListFile.pack( buf );
        }
    }

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    if ( size > 0 )
    {
        if ( mpi_rank > 0 ) buf = new char [size];
#ifndef CPU_VER
        MPI_Bcast( buf, size, MPI_CHARACTER, 0, MPI_COMM_WORLD );
#endif
        if ( mpi_rank > 0 )
        {
            nameListFile = NameListFile();
            nameListFile.unpack( buf );
        }
        delete[] buf;
    }
}

} // namespace

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

std::string FileNameOnly( const std::string& file_path )
{
    const std::string::size_type pos = file_path.find_last_of( "/\\" );
    if ( pos == std::string::npos ) return file_path;
    return file_path.substr( pos + 1 );
}

std::string EnvValueOrUnset( const char* name )
{
    const char* value = std::getenv( name );
    return value ? std::string( value ) : std::string( "(unset)" );
}

void PrintMissingParameterFileWarning(
    const std::string& parameterPath,
    const std::string& parameterPath_old,
    const std::string& defaultParameterMessage
)
{
    std::cout << "================================================================" << std::endl;
    std::cout << "[WARN] " << FileNameOnly( parameterPath ) << " and "
              << FileNameOnly( parameterPath_old ) << " do not exist." << std::endl;
    std::cout << "[INFO] VIS_PARAM_DIR = " << EnvValueOrUnset( "VIS_PARAM_DIR" ) << std::endl;
    std::cout << "[INFO] PARTICLE_DIR  = " << EnvValueOrUnset( "PARTICLE_DIR" ) << std::endl;
    std::cout << "[INFO] " << defaultParameterMessage << std::endl;
    std::cout << "================================================================" << std::endl;
}

void SetDefaultParticleParameter(
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

size_t CalculateSubpixelLevel(
    const int particle_limit,
    const vismodule::Camera& camera,
    const float sampling_step,
    const double total_volume,
    const vismodule::ObjectBase* volume
)
{
    double d_nparticles = 0.0;//particle density for subpixel_level=1
    d_nparticles = Generator::CalculateGreatDensity( camera, *volume, 1, sampling_step ) * total_volume;

    //Calculation of optimized subpixel level
    float plimit = static_cast<float>( particle_limit );
    float nparticles = static_cast<float>( d_nparticles );
    float subpixel_level = sqrt( plimit / nparticles );

    if ( subpixel_level < 1 ) subpixel_level = 1;

    return static_cast<size_t>( subpixel_level + 0.5f );
}

void SetParameterFilePath(
    const int time_step,
    std::string& historyFilePath,
    std::string& stateFilePath,
    std::string& coordMinMaxFilePath,
    std::string& particleFilePrefix,
    std::string& glyphFilePrefix,
    std::string& plotOverLineFilePrefix,
    std::string& plotOverTimeFilePrefix,
    std::string& tfJsonPath,
    std::string& tfJsonPath_old,
    std::string& tfFilePath_step,
    std::string& glyphParameterPath,
    std::string& glyphParameterPath_old,
    std::string& plotOverLineParameterPath,
    std::string& plotOverLineParameterPath_old,
    std::string& plotOverTimeParameterPath,
    std::string& plotOverTimeParameterPath_old
)
{
    std::string visParamDir;
    std::string tfFilename;

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

    std::stringstream step;
    step << '_' << std::setw( 5 ) << std::setfill( '0' ) << time_step;
    historyFilePath = visParamDir + "history" + step.str() + ".txt";
    stateFilePath     = visParamDir + "state.txt";
    
    envBuf = std::getenv( "PARTICLE_DIR" );
    if (envBuf == NULL) {
        coordMinMaxFilePath    = "./t_pfi_coords_minmax.txt";
        particleFilePrefix     = "./t_";
        glyphFilePrefix        = "./g_";
        plotOverLineFilePrefix = "./p_";
        plotOverTimeFilePrefix = "./pt_";
    }
    else {
        coordMinMaxFilePath    = envBuf;
        particleFilePrefix     = envBuf;
        glyphFilePrefix        = envBuf;
        plotOverLineFilePrefix = envBuf;
        plotOverTimeFilePrefix = envBuf;
        if (coordMinMaxFilePath[coordMinMaxFilePath.size() - 1] != '/') {
            coordMinMaxFilePath    += "/t_pfi_coords_minmax.txt";
            particleFilePrefix     += "/t_";
            glyphFilePrefix        += "/g_";
            plotOverLineFilePrefix += "/p_";
            plotOverTimeFilePrefix += "/pt_";
        }
        else {
            coordMinMaxFilePath    += "t_pfi_coords_minmax.txt";
            particleFilePrefix     += "t_";
            glyphFilePrefix        += "g_";
            plotOverLineFilePrefix += "p_";
            plotOverTimeFilePrefix += "pt_";
        }
    }

    tfJsonPath                    = visParamDir + tfFilename + ".json";
    tfJsonPath_old                = visParamDir + tfFilename + "_old.json";
    tfFilePath_step               = visParamDir + tfFilename + step.str() + ".tf";
    glyphParameterPath            = visParamDir + "parameter.gly";
    glyphParameterPath_old        = visParamDir + "parameter_old.gly";
    plotOverLineParameterPath     = visParamDir + "parameter.pol";
    plotOverLineParameterPath_old = visParamDir + "parameter_old.pol";
    plotOverTimeParameterPath     = visParamDir + "parameter.pot";
    plotOverTimeParameterPath_old = visParamDir + "parameter_old.pot";

    is_first_setting = false;
    return true;
}

bool SetGlyphParameter(
    const std::string& glyphParameterPath,
    const std::string& glyphParameterPath_old,
    GlyphProperty& glyph_property,
    NameListFile& nameListFile
)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    ParameterFileReader ppr;

    if ( mpi_rank == 0 )
    {
        std::ifstream glyphParameterFile( glyphParameterPath );
        std::ifstream glyphParameterFileOld( glyphParameterPath_old );

        if ( glyphParameterFile.good() )
        {
            ppr.readGlyphParameterFile( glyphParameterPath.c_str() );
            nameListFile = ppr.getNameListFile();
            std::rename( glyphParameterPath.c_str(), glyphParameterPath_old.c_str() );
        }
        else if ( glyphParameterFileOld.good() )
        {
            ppr.readGlyphParameterFile( glyphParameterPath_old.c_str() );
            nameListFile = ppr.getNameListFile();
        }
        else
        {
            PrintMissingParameterFileWarning(
                glyphParameterPath,
                glyphParameterPath_old,
                "Set default glyph parameters."
            );
            nameListFile = DefaultGlyphParameterNameListFile();
        }
    }

    BroadcastNameListFile( nameListFile, mpi_rank );
    ppr.setNameListFile( nameListFile );
    ppr.setGlyphParameter( glyph_property );

    return true;
}

bool SetPlotOverLineParameter(
    const std::string& plotOverLineParameterPath,
    const std::string& plotOverLineParameterPath_old,
    PlotOverLineProperty& pol_property,
    NameListFile& nameListFile
)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    ParameterFileReader ppr;

    if ( mpi_rank == 0 )
    {
        std::ifstream plotOverLineParameterFile( plotOverLineParameterPath );
        std::ifstream plotOverLineParameterFileOld( plotOverLineParameterPath_old );

        if ( plotOverLineParameterFile.good() )
        {
            ppr.readPlotOverLineParameterFile( plotOverLineParameterPath.c_str() );
            nameListFile = ppr.getNameListFile();
            std::rename( plotOverLineParameterPath.c_str(), plotOverLineParameterPath_old.c_str() );
        }
        else if ( plotOverLineParameterFileOld.good() )
        {
            ppr.readPlotOverLineParameterFile( plotOverLineParameterPath_old.c_str() );
            nameListFile = ppr.getNameListFile();
        }
        else
        {
            PrintMissingParameterFileWarning(
                plotOverLineParameterPath,
                plotOverLineParameterPath_old,
                "Set default plot over line parameters."
            );
            nameListFile = DefaultPlotOverLineParameterNameListFile();
        }
    }

    BroadcastNameListFile( nameListFile, mpi_rank );
    ppr.setNameListFile( nameListFile );
    ppr.setPlotOverLineParameter( pol_property );

    return true;
}

bool SetPlotOverTimeParameter(
    const std::string& plotOverTimeParameterPath,
    const std::string& plotOverTimeParameterPath_old,
    PlotOverTimeProperty& pot_property,
    NameListFile& nameListFile
)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    ParameterFileReader ppr;

    if ( mpi_rank == 0 )
    {
        std::ifstream plotOverTimeParameterFile( plotOverTimeParameterPath );
        std::ifstream plotOverTimeParameterFileOld( plotOverTimeParameterPath_old );

        if ( plotOverTimeParameterFile.good() )
        {
            ppr.readPlotOverTimeParameterFile( plotOverTimeParameterPath.c_str() );
            nameListFile = ppr.getNameListFile();
            std::rename( plotOverTimeParameterPath.c_str(), plotOverTimeParameterPath_old.c_str() );
        }
        else if ( plotOverTimeParameterFileOld.good() )
        {
            ppr.readPlotOverTimeParameterFile( plotOverTimeParameterPath_old.c_str() );
            nameListFile = ppr.getNameListFile();
        }
        else
        {
            PrintMissingParameterFileWarning(
                plotOverTimeParameterPath,
                plotOverTimeParameterPath_old,
                "Set default plot over time parameters."
            );
            nameListFile = DefaultPlotOverTimeParameterNameListFile();
        }
    }

    BroadcastNameListFile( nameListFile, mpi_rank );
    ppr.setNameListFile( nameListFile );
    ppr.setPlotOverTimeParameter( pot_property );

    return true;
}

void MakeParticle(
    const vismodule::PointObject* point_object,
    std::vector<float>& coords,
    std::vector<Byte>&  colors,
    std::vector<float>& normals
)
{
    size_t nmemb = point_object->nvertices() * 3;
    vismodule::ValueArray<vismodule::Real32> coords_array ( point_object->coords().pointer() , nmemb );
    vismodule::ValueArray<vismodule::UInt8>  colors_array ( point_object->colors().pointer() , nmemb );
    vismodule::ValueArray<vismodule::Real32> normals_array( point_object->normals().pointer(), nmemb );

    coords.insert( coords.end(), coords_array.begin(), coords_array.end() );
    colors.insert( colors.end(), colors_array.begin(), colors_array.end() );
    normals.insert( normals.end(), normals_array.begin(), normals_array.end() );
}

void MakeGlyph(
    const vismodule::KVSMLObjectGlyph* glyph_object,
    std::vector<float>& coords,
    std::vector<float>& vectors,
    std::vector<float>& sizes,
    std::vector<unsigned char>& colors
)
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

    auto const& c = glyph_object->coords();
    auto const& v = glyph_object->directions();
    auto const& s = glyph_object->sizes();
    auto const& k = glyph_object->colors();

#if 0 // for debug

    std::cout << "vectors:"
              << " cur = " << vectors.size()
              << " add = " << v.size() 
              << " max = " << vectors.max_size() << std::endl;
    std::cout << "coords :"
              << " cur = " << coords.size()
              << " add = " << c.size()
              << " max = " << coords.max_size() << std::endl;
    std::cout << "colors :"
              << " cur = " << colors.size()
              << " add = " << k.size()
              << " max = " << colors.max_size() << std::endl;
    std::cout << "sizes  :"
              << " cur = " << sizes.size()
              << " add = " << s.size()
              << " max = " << sizes.max_size() << std::endl;
#endif // for debug

    // 集約処理
    safe_append(coords , c, "m_glyph_coords");
    safe_append(vectors, v, "m_glyph_vectors");
    safe_append(sizes  , s, "m_glyph_sizes");
    safe_append(colors , k, "m_glyph_colors");
}

void OutputParticleFiles(
    ParticleProperty& particle_property,
    const MultiVolumePropertyList& mvpl,
    const int time_step,
    const std::string& particleFilePrefix,
    const std::vector<float>& coords,
    const std::vector<Byte>& colors,
    const std::vector<float>& normals
)
{
    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
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
    // 環境変数で指定したファイルパスを参照する
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << count;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << num_nodes;
    ss << ".kvsml";
    const std::string particleFilePath = particleFilePrefix + ss.str();

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
    vismodule::ValueArray<float> tmp_coords( coords );
    vismodule::ValueArray<Byte>  tmp_colors( colors );
    vismodule::ValueArray<float> tmp_normals( normals );

    MPI_Gatherv( tmp_coords.pointer(),   particle_size, MPI_FLOAT,
                 new_coords.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, new_comm );

    MPI_Gatherv( tmp_colors.pointer(),   particle_size, MPI_BYTE,
                 new_colors.pointer(), recvcounts, displs, MPI_BYTE,
                 0, new_comm );

    MPI_Gatherv( tmp_normals.pointer(),   particle_size, MPI_FLOAT,
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
        vismodule::PointObject* point_object = new vismodule::PointObject( new_coords, new_colors, new_normals, particle_property.m_subpixel_level );
        point_object->setMinMaxObjectCoords( mvpl.m_total_min_object_coord, mvpl.m_total_max_object_coord );
        // If async_io is enabled, use worker thread to write kvsml data
        if (async_io_enabled){
            pbvr::ParticleWriteThread* particle_write_thread = &pwt;
            particle_write_thread->join( true );
            particle_write_thread->setPointObject( point_object );
            particle_write_thread->setFilename( particleFilePath.c_str() );
            particle_write_thread->work( true );
        }// If async_io is disabled, use kvs::PointExporter here in main thread.
        else{
            vismodule::KVSMLObjectPoint* kvsml_object = new vismodule::PointExporter<vismodule::KVSMLObjectPoint>( *point_object );
            kvsml_object->setWritingDataType( vismodule::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->write( particleFilePath.c_str() );
            delete kvsml_object;
        }
        delete point_object;
    }

    delete[] displs;
    delete[] recvcounts;
}

void OutputParticleHistory(
    ParticleProperty& particle_property,
    const int tf_number,
    const int nvariables,
    const std::string& histryFilePath,
    const bool update_parameter_file,
    const vismodule::UInt64* c_bins,
    const vismodule::UInt64* o_bins,
    const float* max_array,
    const float* min_array
)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    // histgram receiver
    vismodule::UInt64* c_bins_recv;
    vismodule::UInt64* o_bins_recv;
    c_bins_recv = new vismodule::UInt64[tf_number * DEFAULT_NBINS];
    o_bins_recv = new vismodule::UInt64[tf_number * DEFAULT_NBINS];

    for ( size_t i = 0; i < (tf_number * DEFAULT_NBINS); i++ )
    {
        c_bins_recv[i] = 0;
        o_bins_recv[i] = 0;
    }

#ifndef CPU_VER
    //ヒストグラムの集計
    MPI_Reduce( c_bins, c_bins_recv, (tf_number * DEFAULT_NBINS), MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD );
    MPI_Reduce( o_bins, o_bins_recv, (tf_number * DEFAULT_NBINS), MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD );
#else
    for( int i = 0; i < (tf_number * DEFAULT_NBINS); i++ )
    {
        c_bins_recv[i] = c_bins[i];
        o_bins_recv[i] = o_bins[i];
    }
#endif

    // min max receiver
    float* max_array_recv;
    float* min_array_recv;
    max_array_recv = new float[tf_number * 2]; // color, opacity
    min_array_recv = new float[tf_number * 2]; // color, opacity

    for ( size_t i = 0; i < (tf_number * 2); i++ )
    {
        max_array_recv[i] = FLT_MIN;
        min_array_recv[i] = FLT_MAX;
    }

#ifndef CPU_VER
    MPI_Reduce( max_array, max_array_recv, (tf_number * 2), MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( min_array, min_array_recv, (tf_number * 2), MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
#else
    for( size_t i = 0; i < (tf_number * 2); i++ )
    {
        max_array_recv[i] = max_array[i];
        min_array_recv[i] = min_array[i];
    }
#endif

    if( mpi_rank == 0 )
    {
        // historyファイルの出力
        std::ofstream ofs2( histryFilePath.c_str(), std::ios::out);
        ofs2 << "TF_NUMBER=" << tf_number << std::endl;

        for( size_t i = 0; i < tf_number; i++ )
        {
            ofs2 << "MIN_O" << (i + 1) << "=" << min_array_recv[2 * i    ] << std::endl;
            ofs2 << "MAX_O" << (i + 1) << "=" << max_array_recv[2 * i    ] << std::endl;
            ofs2 << "MIN_C" << (i + 1) << "=" << min_array_recv[2 * i + 1] << std::endl;
            ofs2 << "MAX_C" << (i + 1) << "=" << max_array_recv[2 * i + 1] << std::endl;
            ofs2 << "RESOLUTION_O" << (i + 1) << "=" << DEFAULT_NBINS << std::endl;
            ofs2 << "HISTOGRAM_O"  << (i + 1) << "=";
            for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
            {
                ofs2 << o_bins_recv[j + (i * DEFAULT_NBINS)] << ",";
            }
            ofs2 << std::endl;
            ofs2 << "RESOLUTION_C" << (i + 1) << "=" << DEFAULT_NBINS << std::endl;
            ofs2 << "HISTOGRAM_C"  << (i + 1) << "=";
            for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
            {
                ofs2 << c_bins_recv[j + (i * DEFAULT_NBINS)] << ",";
            }
            ofs2 << std::endl;
        }

        ofs2 << "N_VARIABLES="             << nvariables                               << std::endl;
        // ofs2 << "EXTRA_OPACITY_FACTOR="    << particle_property.m_extra_opacity_factor << std::endl; // 一時的にコメントアウト
        ofs2 << "PARTICLE_LIMIT="          << particle_property.m_particle_limit       << std::endl;
        ofs2 << "END_HISTORY_FILE=SUCCESS" << std::endl;
        ofs2.close();

        // ServerSideMinMaxを更新してからdefault.tfファイルを出力
        for( size_t i = 0; i < tf_number; i++ )
        {
            particle_property.m_transfunc_array[i].m_server_color_variable_min   = min_array_recv[2 * i + 1];
            particle_property.m_transfunc_array[i].m_server_color_variable_max   = max_array_recv[2 * i + 1];
            particle_property.m_transfunc_array[i].m_server_opacity_variable_min = min_array_recv[2 * i    ];
            particle_property.m_transfunc_array[i].m_server_opacity_variable_max = max_array_recv[2 * i    ];
        }

        if ( update_parameter_file )
        {
            ParameterFileWriter ppw;
            ppw.getParticleParameter( particle_property );
            ppw.writeTF2OldJson( particle_property ); //old_Jsonファイル出力
        }
    }

    delete[] min_array_recv;
    delete[] max_array_recv;
    delete[] c_bins_recv;
    delete[] o_bins_recv;
}

void OutputParticles(
    ParticleProperty& particle_property,
    const MultiVolumePropertyList& mvpl,
    const int,
    const int time_step,
    const int tf_number,
    const int nvariables,
    const std::string& particleFilePrefix,
    const std::string&,
    const std::string& histryFilePath,
    const std::vector<float>& coords,
    const std::vector<Byte>& colors,
    const std::vector<float>& normals,
    const vismodule::UInt64* c_bins,
    const vismodule::UInt64* o_bins,
    const float* max_array,
    const float* min_array
)
{
    OutputParticleFiles(
        particle_property, mvpl, time_step, particleFilePrefix,
        coords, colors, normals
    );
    OutputParticleHistory(
        particle_property, tf_number, nvariables, histryFilePath, true,
        c_bins, o_bins, max_array, min_array
    );
}

void OutputGlyphs(
    const int time_step,
    const std::string& glyphFilePrefix,
    const std::vector<float>& coords,
    const std::vector<float>& vectors,
    const std::vector<float>& sizes,
    const std::vector<unsigned char>& colors
)
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

    vismodule::ValueArray<float> tmp_coords( coords );
    vismodule::ValueArray<float> tmp_vectors( vectors );
    vismodule::ValueArray<Byte>  tmp_colors( colors );
    vismodule::ValueArray<float> tmp_sizes( sizes );

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_rank+1;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_size;
    ss << ".dat";
    std::string glyphFilePath;
    glyphFilePath = glyphFilePrefix + ss.str();
    
    vismodule::KVSMLObjectGlyph glyph_object( tmp_coords, tmp_colors, tmp_vectors, tmp_sizes );
    glyph_object.write( glyphFilePath.c_str() );
}

void OutputLine(
    const int time_step,
    const std::string& plotOverLineFilePrefix,
    const std::vector<float>& values_on_line,
    const std::vector<int>& mask,
    const std::vector<float>& x_axis
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

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_rank+1;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_size;
    ss << ".dat";
    std::string plotOverLineFilePath;
    plotOverLineFilePath = plotOverLineFilePrefix + ss.str();

    vismodule::ValueArray<float> tmp_values_on_line( values_on_line );
    vismodule::ValueArray<float> tmp_x_axis( x_axis );

    // convert mask's data from int to bool
    // MPI does not support bool type so use int type
    // vismodule::KVSMLObjectPlotOverLine::m_mask is defined as bool type
    vismodule::ValueArray<bool> tmp_mask( mask.size() );
    for ( size_t i = 0; i < mask.size(); i++ )
    {
        if ( mask[i] != 0 ) tmp_mask[i] = true;
    }

    vismodule::KVSMLObjectPlotOverLine pol_object( tmp_values_on_line, tmp_x_axis, tmp_mask );
    pol_object.write( plotOverLineFilePath.c_str() );
}

void OutputPOT(
    const int time_step,
    const std::string& plotOverTimeFilePrefix,
    const bool mask,
    const std::vector<float>& value_on_time
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

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << ( mpi_rank + 1 );
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_size;
    ss << ".dat";

    std::string plotOverTimeFilePath;
    plotOverTimeFilePath = plotOverTimeFilePrefix + ss.str();

    vismodule::ValueArray<float> tmp_value_on_time( value_on_time );

    vismodule::KVSMLObjectPlotOverTime pot_object( tmp_value_on_time, mask );
    pot_object.write( plotOverTimeFilePath.c_str() );
}
