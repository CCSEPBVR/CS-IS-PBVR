#include <fstream>
#include <cstdio>
#include <vector>
#include <vismodule/Vector3>
#include <vismodule/AnyValueArray>
#include <vismodule/ValueArray>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/TrilinearInterpolator>
#include <vismodule/PointObject>
#include <vismodule/PointExporter>
#include <vismodule/KVSMLObjectPoint>
#include <vismodule/MersenneTwister>
#include <vismodule/TransferFunction>
#include <vismodule/RGBColor>
#include <vismodule/Timer>

#include <sstream>
#include <iomanip>
#include <memory>

#include "kvs_wrapper.h"
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/ParamInfo>
#include "float.h"
#include <vismodule/UnstructuredVolumeObject>
#ifndef CPU_VER
#include <mpi.h>
#endif
#include <vismodule/CellBase>
#include <vismodule/TetrahedralCell>
#include <vismodule/QuadraticTetrahedralCell>
#include <vismodule/HexahedralCell>
#include <vismodule/QuadraticHexahedralCell>
#include <vismodule/PrismaticCell>
#include <vismodule/PyramidalCell>

#ifdef VTK
//VTK
#include <vtkSmartPointerBase.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <FileFormat/VtkUnstructuredFileFormat.h>
#include <FileFormat/VTK/VtkXmlUnstructuredGrid.h>
#include <vtkUnstructuredGrid.h>

//kvsmlImporter
#include "CvtTypeTraits.h"
#include "Importer/VtkImporter.h"
#endif

//PlotOverLine
#include <vismodule/PlotOverLine>

// add FJ start
#ifndef SIMD_BLK_SIZE
#define SIMD_BLK_SIZE 128
#endif
// add FJ  end


#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

#define RANK 1

// Asynchronous io, using worker thread pwt.
#include "particle_write_thread.h"
bool async_io_enabled = false;
static bool is_initial_step = true;
static size_t  start_time_step = 0;

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
    d_nparticles = Generator::CalculateGreatDensity( camera, *volume, 1, sampling_step ) * total_volume;

    //Calculation of optimized subpixel level
    float plimit = static_cast<float>( particle_limit );
    float nparticles = static_cast<float>( d_nparticles );
    float subpixel_level = sqrt( plimit / nparticles );

    if ( subpixel_level < 1 ) subpixel_level = 1;

    return static_cast<size_t>( subpixel_level + 0.5f );
}

void SetDefalutParameter( TransferFunctionSynthesizer* tfs,
                          pbvr_parameters* particleBase,
                          const int nvariables, Type** values, const int ncoords)
{
    //Read TFS
    std::vector<int> i_table;
    std::vector<float> f_table;
    EquationToken eq;
    std::vector<EquationToken> var1;
    int tf_number;
    // get TF_NUMBER
    tf_number = nvariables;
    std::vector<vismodule::TransferFunction> tf;

    //Read 1D tf
    int resolution = 256;
    float min, max;

    tf.clear();

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        // 各変量のminmax
        // calc_minmax
        min = values[i][0];
        max = values[i][0];
        
        for(int j = 1; j < ncoords; j++)
        {
            min = min < values[i][j] ? min : values[i][j]; 
            max = max > values[i][j] ? max : values[i][j]; 
        }

        vismodule::ColorMap color_map( 256, min, max );
        vismodule::OpacityMap opacity_map( 256, min, max );

        vismodule::TransferFunction tfBuf;
        tfBuf.setColorMap( color_map );
        tfBuf.setOpacityMap( opacity_map );
        tf.push_back(tfBuf);
    }
    particleBase ->m_tf = tf;

    // add by shimomura 2024/03/25
    std::string  equation;

    equation = "a1";
    eq = tfs->convert_token(equation);
    tfs->setOpacityFunction( eq );

    equation = "c1" ;
    eq = tfs->convert_token(equation);
    tfs->setColorFunction( eq );

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "q" << i + 1 ;
        const std::string tag_base = tss.str();

        equation = tag_base;
        eq = tfs->convert_token(equation);
        var1.push_back( eq );
    }

    tfs->setOpacityVariable( var1 );
    tfs->setColorVariable( var1 );

    var1.clear();
    int nbin = 256;
    particleBase->m_tf_number = nvariables;
    particleBase->m_O_max.allocate(particleBase->m_tf_number);
    particleBase->m_O_min.allocate(particleBase->m_tf_number);
    particleBase->m_C_max.allocate(particleBase->m_tf_number);
    particleBase->m_C_min.allocate(particleBase->m_tf_number);
    particleBase->m_O_max.fill(0x00);
    particleBase->m_O_min.fill(0x00);
    particleBase->m_C_max.fill(0x00);
    particleBase->m_C_min.fill(0x00);
    particleBase->m_o_histogram.allocate(particleBase->m_tf_number*nbin);
    particleBase->m_c_histogram.allocate(particleBase->m_tf_number*nbin);
    particleBase->m_o_histogram.fill(0x00);
    particleBase->m_c_histogram.fill(0x00);
}

}//end of unnamed namespace

// 変数配列用のソルバー関数
void generate_particles(
    int time_step,
    domain_parameters_unstruct dom,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const  vismodule::VolumeObjectBase::CellType& celltype
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

    std::string historyFilePath;
    std::string stateFilePath;
    std::string coordMinMaxFilePath;
    std::string particleFilePrefix;
    std::string glyphFilePrefix;
    std::string plotOverLineFilePrefix;
    std::string tfFilePath;
    std::string tfFilePath_old;
    std::string tfFilePath_step;
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

    char  arg_dummy0[] = "dummy";
    char* arg_dummy[]  = { arg_dummy0, NULL };

    Argument param( 1, arg_dummy );
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
        tmp_c_bins[i] = 0;
        tmp_o_bins[i] = 0;
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
    vismodule::ValueArray<bool>  mask( resolution );
    vismodule::ValueArray<float> x_axis( resolution );

    values_on_line.fill( 0x00 );
    mask.fill( false );
    x_axis.fill( 0x00 );

    jpv::ServerMode server_mode = jpv::ServerMode::IS;
    vismodule::PointObject* point_object = nullptr;
    vismodule::CS_PointObjectGenerator point_object_generator;
    point_object = point_object_generator.GenerateParticleUnstruct(
        param, dom, values, nvariables, coordinates,
        ncoords, connections, ncells, celltype, server_mode
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
        glyph_creator.GenerateGlyphUnstruct(
            param, number_of_divide, values, nvariables, coordinates,
            ncoords, connections, ncells, celltype, glyph_object
        );

        MakeGlyph( glyph_object, glyph_coords, glyph_vectors, glyph_sizes, glyph_colors ); // InSitu only

        delete glyph_object;
    }

    if ( param.m_plot_flag )
    {
        vismodule::KVSMLObjectPlotOverLine* pol_object = new vismodule::KVSMLObjectPlotOverLine;
        PlotOverLineGenerator pol_generator;
        pol_generator.GeneratePOLUnstruct(
            param, values, nvariables, coordinates,
            ncoords, connections, ncells, celltype, pol_object
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

void SetParameterFilePath(
    const int time_step,
    std::string& historyFilePath,
    std::string& stateFilePath,
    std::string& coordMinMaxFilePath,
    std::string& particleFilePrefix,
    std::string& glyphFilePrefix,
    std::string& plotOverLineFilePrefix,
    std::string& tfFilePath,
    std::string& tfFilePath_old,
    std::string& tfFilePath_step,
    std::string& glyphParameterPath,
    std::string& glyphParameterPath_old,
    std::string& plotOverLineParameterPath,
    std::string& plotOverLineParameterPath_old
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
    history_file_name = visParamDir + "history" + step.str() + ".txt";
    stateFilePath     = visParamDir + "state.txt";
    
    envBuf = std::getenv( "PARTICLE_DIR" );
    if (envBuf == NULL) {
        coordMinMaxFilePath    = "./t_pfi_coords_minmax.txt";
        particleFilePrefix     = "./t_";
        glyphFilePrefix        = "./g_";
        plotOverLineFilePrefix = "./p_";
    }
    else {
        coordMinMaxFilePath    = envBuf;
        particleFilePrefix     = envBuf;
        glyphFilePrefix        = envBuf;
        plotOverLineFilePrefix = envBuf;
        if (coordMinMaxFilePath[coordMinMaxFilePath.size() - 1] != '/') {
            coordMinMaxFilePath    += "/t_pfi_coords_minmax.txt";
            particleFilePrefix     += "/t_";
            glyphFilePrefix        += "/g_";
            plotOverLineFilePrefix += "/p_";
        }
        else {
            coordMinMaxFilePath    += "t_pfi_coords_minmax.txt";
            particleFilePrefix     += "t_";
            glyphFilePrefix        += "g_";
            plotOverLineFilePrefix += "p_";
        }
    }

    tfFilePath                    = visParamDir + tfFilename + ".tf";
    tfFilePath_old                = visParamDir + tfFilename + "_old.tf";
    tfFilePath_step               = visParamDir + tfFilename + step.str() + ".tf";
    glyphParameterPath            = visParamDir + "parameter.gly";
    glyphParameterPath_old        = visParamDir + "parameter_old.gly";
    plotOverLineParameterPath     = visParamDir + "parameter.pol";
    plotOverLineParameterPath_old = visParamDir + "parameter_old.pol";
}

void OutputCoordMinMaxFile(
    const domain_parameters_unstruct& dom,
    const std::string& coordMinMaxFilePath
)
{
    static bool minmaxFlag = false;
    if (minmaxFlag == false && mpi_rank == 0) {
        FILE* fp = fopen( coordMinMaxFilePath.c_str(), "w" );
        if( fp )
        {
            fprintf( fp, "%f %f %f %f %f %f\n",
                     dom.x_global_min,
                     dom.y_global_min,
                     dom.z_global_min,
                     dom.x_global_max,
                     dom.y_global_max,
                     dom.z_global_max );
            fclose( fp );
        }
        minmaxFlag = true;
    }
}

bool SetParticleParameter( 
    const domain_parameters_unstruct& dom,
    const std::string& tfFilePath,
    const std::string& tfFilePath_old,
    Argument& param,
    MultiVolumePropertyList& mvpl,
    NameListFile& nameListFile
)
{
    bool result = false;
    ParameterFileReader ppr;

    vismodule::Vector3f min_object_coords(
        dom.x_global_min,
        dom.y_global_min,
        dom.z_global_min
    );
    vismodule::Vector3f max_object_coords(
        dom.x_global_max,
        dom.y_global_max,
        dom.z_global_max
    );

    mvpl.m_total_min_object_coord  = min_object_coords;
    mvpl.m_total_max_object_coord  = max_object_coords;
    ppr.readParticleParameterFile( tfFilePath.c_str() );
    std::rename( filename.c_str(), old_filename.c_str() );
    ppr.setParticleParameter( param );
    nameListFile = ppr.getNameListFile();

    const float min = vismodule::Math::Min(
        dom.x_global_min,
        dom.y_global_min,
        dom.z_global_min
    );

    const float max = vismodule::Math::Max(
        dom.x_global_max,
        dom.y_global_max,
        dom.z_global_max
    );

    param.m_time
    param.m_sampling_step = ( max - min ) / 1E1;
    const float sampling_step = param.m_sampling_step;

    vismodule::VolumeObjectBase object;
    object.setMinMaxObjectCoords( min_object_coords, max_object_coords );
    object.setMinMaxExternalCoords( min_object_coords, max_object_coords );

    const double total_volume = ( dom.x_global_max - dom.x_global_min )
                              * ( dom.y_global_max - dom.y_global_min )
                              * ( dom.z_global_max - dom.z_global_min );

    const float max_opacity      = 0.98;
    const int particle_limit     = param.m_particle_limit;
    const float particle_density = param.m_particle_density;
    const int subpixel_level     = CalculateSubpixelLevel(
        particle_limit,
        *param.m_camera,
        sampling_step,
        total_volume,
        &object
    );

    float sampling_volume_inverse;
    float max_density;

    Generator::CalculateDensityParameters(
        param.m_camera,
        &object,
        subpixel_level,
        param.m_sampling_step,
        max_opacity,
        &sampling_volume_inverse,
        &max_density
    );

    param.m_transfunc_synthesizer->setMaxOpacity( max_opacity );
    param.m_transfunc_synthesizer->setMaxDensity( max_opacity );
    param.m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );

    // cameraをどこかでnewする必要がある
    // calculate部分をCSと共通化予定
    // param.m_sampling_step  = CalculateSamplingStep( mvpl );
    // param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *param.m_camera );

    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    if( mpi_rank == 0 )
    {
        fprintf( stdout , "---------initialize Parameters--------------------------\n" );
        fprintf( stdout , "particle_limit    = %20d\n"  , particle_limit               );
        fprintf( stdout , "particle_density  = %20f\n"  , particle_density             );
        fprintf( stdout , "resolutin_height  = %20d\n"  , height                       );
        fprintf( stdout , "resolutin_width   = %20d\n"  , width                        );
        fprintf( stdout , "total_volume      = %20.3e\n", total_volume                 );
        fprintf( stdout , "  |-X             = %20f\n"  , object.maxObjectCoord().x()  );
        fprintf( stdout , "  |-Y             = %20f\n"  , object.maxObjectCoord().y()  );
        fprintf( stdout , "  |-Z             = %20f\n"  , object.maxObjectCoord().z()  );
        fprintf( stdout , "max_opacity       = %20.3e\n", max_opacity                  );
        fprintf( stdout , "max_density       = %20.3e\n", max_density                  );
        fprintf( stdout , "sampling_step     = %20.3e\n", sampling_step                );
        fprintf( stdout , "subpixel_level    = %20d\n"  , subpixel_level               );
        fprintf( stdout , "--------------------------------------------------------\n" );
    }

    return true;
}

bool SetGlyphParameter(
    const std::string& glyphParameterPath,
    const std::string& glyphParameterPath_old,
    Argument& param 
)
{
    ParameterFileReader ppr;
    ppr.readGlyphParameterFile( glyphParameterPath.c_str() );
    std::rename( glyphParameterPath.c_str(), glyphParameterPath_old.c_str() );
    ppr.setGlyphParameter( param );
    return true;
}

bool SetPlotOverLineParameter(
    const std::string& plotOverLineParameterPath,
    const std::string& plotOverLineParameterPath_old
    Argument& param
)
{
    ParameterFileReader ppr;
    ppr.readPlotOverLineParameterFile( plotOverLineParameterPath.c_str() );
    std::rename( plotOverLineParameterPath.c_str(), plotOverLineParameterPath_old.c_str() );
    ppr.setPlotOverLineParameter( param );
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

    auto const& c = glyph_object->glyph_coords();
    auto const& v = glyph_object->glyph_directions();
    auto const& s = glyph_object->glyph_sizes();
    auto const& k = glyph_object->glyph_colors();

#if 0 // for debug
     std::cout << "glyph_param.m_glyph_sizes        = " << glyph_param.m_glyph_sizes.size()     << std::endl; 
     std::cout << "glyph_seed.glyph_sizes()         = " << glyph_seed.glyph_sizes().size()      << std::endl; 
     std::cout << "glyph_param.m_glyph_vectors      = " << glyph_param.m_glyph_vectors.size()   << std::endl; 
     std::cout << "glyph_seed.glyph_directions()    = " << glyph_seed.glyph_directions().size() << std::endl; 
     std::cout << "m_glyph_coords.size()            = " << glyph_param.m_glyph_coords.size()    << std::endl; 
     std::cout << "glyph_seed.glyph_coords().size() = " << glyph_seed.glyph_coords().size()     << std::endl; 
     std::cout << "m_glyph_coords.size()            = " << glyph_param.m_glyph_colors.size()    << std::endl; 
     std::cout << "glyph_seed.glyph_colors()        = " << glyph_seed.glyph_colors().size()     << std::endl; 

    std::cout << "vectors:"
              << " cur = " << glyph_param.m_glyph_vectors.size()
              << " add = " << v.size() 
              << " max = " << glyph_param.m_glyph_vectors.max_size() << std::endl;
    std::cout << "coords :"
              << " cur = " << glyph_param.m_glyph_coords.size()
              << " add = " << c.size()
              << " max = " << glyph_param.m_glyph_coords.max_size() << std::endl;
    std::cout << "colors :"
              << " cur = " << glyph_param.m_glyph_colors.size()
              << " add = " << k.size()
              << " max = " << glyph_param.m_glyph_colors.max_size() << std::endl;
    std::cout << "sizes  :"
              << " cur = " << glyph_param.m_glyph_sizes.size()
              << " add = " << s.size()
              << " max = " << glyph_param.m_glyph_sizes.max_size() << std::endl;
#endif // for debug

    // 集約処理
    safe_append(coords , c, "m_glyph_coords");
    safe_append(vectors, v, "m_glyph_vectors");
    safe_append(sizes  , s, "m_glyph_sizes");
    safe_append(colors , k, "m_glyph_colors");
}

void OutputParticles(
    const Argument& param,
    const MultiVolumePropertyList& mvpl,
    const int time_step,
    const int tf_number,
    const int nvariables,
    const std::string& particleFilePrefix,
    const std::string& stateFilePath,
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
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

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
        vismodule::PointObject* point_object = new vismodule::PointObject( new_coords, new_colors, new_normals, param.m_subpixel_level );
        point_object->setMinMaxObjectCoords( mvpl.m_total_min_object_coord, mvpl.m_total_min_object_coord );
        // If async_io is enabled, use worker thread to write kvsml data and state.txt
        if (async_io_enabled){
            pbvr::ParticleWriteThread* particle_write_thread = &pwt;
            particle_write_thread->join( true );
            particle_write_thread->setPointObject( point_object );
            particle_write_thread->setFilename( particleFilePath.c_str() );
            particle_write_thread->setTimestep( time_step , stateFilePath.c_str() );
            particle_write_thread->setStartTimestep( start_time_step ); //add by shimomura 20240808
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

    // histgram receiver
    vismodule::UInt64* c_bins_recv;
    vismodule::UInt64* o_bins_recv;
    c_bins_recv = new vismodule::UInt64[tf_number * DEFAULT_NBINS];
    o_bins_recv = new vismodule::UInt64[tf_number * DEFAULT_NBINS];

#ifndef CPU_VER
    //ヒストグラムの集計
    MPI_Reduce( c_bins.pointer(), c_bins_recv.pointer(),
                (tf_number * DEFAULT_NBINS), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );

    MPI_Reduce( o_bins.pointer(), o_bins_recv.pointer(),
                (tf_number * DEFAULT_NBINS), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
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

#ifndef CPU_VER
    // if(mpi_rank==0)std::cout<<"MPI_Reduce"<<std::endl;
    MPI_Reduce( max_array.pointer(), max_array_recv.pointer(),
                (tf_number * 2), MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( min_array.pointer(), min_array_recv.pointer(),
                (tf_number * 2), MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
#else
    for( size_t i = 0; i < tf_number; i++ )
    {
        max_array_recv[i] = max_array[i];
        min_array_recv[i] = min_array[i];
    }
#endif

    //状態ファイルの出力
    if( mpi_rank == 0 )
    {
        // If async_io is enabled, state.txt will be written from worker thread.
        // If async_io is disabled, state.txt will be written here.
        if ( !async_io_enabled ){
            std::ofstream ofs( stateFilePath.c_str(), std::ios::out );
            if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

            ofs << "START_STEP  = " << start_time_step << std::endl;
            ofs << "LATEST_STEP = " << time_step       << std::endl;

            ofs.close();
        }

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

        ofs2 << "N_VARIABLES="      << nvariables               << std::endl;
        ofs2 << "PARTICLE_DENSITY=" << param.m_particle_density << std::endl;
        ofs2 << "PARTICLE_LIMIT="   << param.m_particle_limit   << std::endl;
        ofs2 << "END_HISTORY_FILE=SUCCESS" << std::endl;
        ofs2.close();
    }
}

void OutputGlyphs(
    const int time_step,
    const std::string& glyphFilePrefix
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

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_rank+1;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_size;
    ss << ".dat";
    std::string glyphFilePath;
    glyphFilePath = glyphFilePrefix + ss.str();
    
    vismodule::KVSMLObjectGlyph glyph_object( coords, colors, vectors, sizes );
    glyph_object.write( glyphFilePath.c_str() );
}

void OutputLine(
    const int time_step,
    const std::string& plotOverLineFilePrefix,
    const vismodule::ValueArray<float>& values_on_line,
    const vismodule::ValueArray<bool>& mask,
    const vismodule::ValueArray<float>& x_axis
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

    vismodule::KVSMLObjectPlotOverLine pol_object( values_on_line, x_axis, mask );
    pol_object.write( plotOverLineFilePath.c_str() );
}

#ifdef VTK
// vtk用のソルバー関数
void generate_particles_vtk( int time_step, vtkUnstructuredGrid* ucd )
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
 
    domain_parameters_unstruct dom;
    SetDomain( ucd, &dom );

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
        tmp_max[ i ] = FLT_MIN;
        tmp_min[ i ] = FLT_MAX;
    }

    // glyph parameters
    std::vector<float>         glyph_coords;
    std::vector<float>         glyph_vectors;
    std::vector<float>         glyph_sizes;
    std::vector<unsigned char> glyph_colors;

    // plot over line parameters
    vismodule::ValueArray<float> values_on_line( resolution );
    vismodule::ValueArray<bool>   mask( resolution, 0 );
    vismodule::ValueArray<float> x_axis( resolution );

    values_on_line.fill( 0x00 );
    mask.fill( false );
    x_axis.fill( 0x00 );

    int nvariables;
    
    // 先にセルタイプを持っておく
    kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid input_vtu( ucd );
    std::vector<kvs::UnstructuredVolumeObject::CellType> kvs_cell_type_vector;

    for( auto vtu : input_vtu.eachCellType() )
    {
        kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( &vtu );
        kvs::UnstructuredVolumeObject* volume = &importer;
        kvs_cell_type_vector.push_back( volume->cellType() );
    }

    for ( size_t i = 0; i < kvs_cell_type_vector.size(); i++ )
    {
        vismodule::UnstructuredVolumeImporter importer;
        importer.import( input_vtu, kvs_cell_type_vector[i] );
        vismodule::UnstructuredVolumeObject* volume = &importer;

        std::unique_ptr<std::unique_ptr<Type[]>[]> values;
        domain_parameters_unstruct tmp_dom; // not use
        nvariables  = 0;
        std::unique_ptr<float[]> coordinates;
        int ncoords = 0;
        std::unique_ptr<unsigned int[]> connections;
        int ncells = 0;
        vismodule::VolumeObjectBase::CellType celltype;

        // CS common
        store_volume_in_variables_array_unstruct(
            volume, tmp_dom, values, nvariables, coordinates,
            ncoords, connections, ncells, celltype
        );

        vismodule::PointObject* point_object = nullptr;
        vismodule::CS_PointObjectGenerator point_object_generator;
        point_object = point_object_generator.GenerateParticleUnstruct(
            param, dom, values, nvariables, coordinates,
            ncoords, connections, ncells, celltype, server_mode
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
            glyph_creator.GenerateGlyphUnstruct(
                param, number_of_divide, values, nvariables, coordinates,
                ncoords, connections, ncells, celltype, glyph_object
            );

            MakeGlyph( glyph_object, glyph_coords, glyph_vectors, glyph_sizes, glyph_colors ); // InSitu only

            delete glyph_object;
        }

        if ( param.m_plot_flag )
        {
            vismodule::KVSMLObjectPlotOverLine* pol_object = new vismodule::KVSMLObjectPlotOverLine;
            PlotOverLineGenerator pol_generator;
            pol_generator.GeneratePOLUnstruct(
                param, values, nvariables, coordinates,
                ncoords, connections, ncells, celltype, pol_object
            );

            MakePlotOverLine( pol_object, resolution, values_on_line, mask, x_axis ); // CS common

            delete pol_object;
        }
    } // for ( size_t i = 0; i < kvs_cell_type_vector.size(); i++ )

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

void SetDomain( vtkUnstructuredGrid* ucd, domain_parameters_unstruct* dom)
{
    double bounds[6];
    ucd->GetPoints()->GetBounds( bounds );
    float recv_Xmin, recv_Xmax;
    float recv_Ymin, recv_Ymax;
    float recv_Zmin, recv_Zmax;

    float Xmin = bounds[0];
    float Xmax = bounds[1];
    float Ymin = bounds[2];
    float Ymax = bounds[3];
    float Zmin = bounds[4];
    float Zmax = bounds[5];

#ifndef CPU_VER
    MPI_Allreduce( &Xmin, &recv_Xmin, 1 , MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( &Ymin, &recv_Ymin, 1 , MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( &Zmin, &recv_Zmin, 1 , MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( &Xmax, &recv_Xmax, 1 , MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
    MPI_Allreduce( &Ymax, &recv_Ymax, 1 , MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
    MPI_Allreduce( &Zmax, &recv_Zmax, 1 , MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#else // CPU_VER
    recv_Xmin = Xmin;
    recv_Ymin = Ymin;
    recv_Zmin = Zmin;
    recv_Xmax = Xmin;
    recv_Ymax = Ymax;
    recv_Zmax = Zmax;
#endif // CPU_VER

    dom->x_global_min = recv_Xmin;
    dom->y_global_min = recv_Ymin;
    dom->z_global_min = recv_Zmin;
    dom->x_global_max = recv_Xmax;
    dom->y_global_max = recv_Ymax;
    dom->z_global_max = recv_Zmax;
}
#endif // VTK

/*

vismodule::Vector3f RandomSamplingInCube( vismodule::MersenneTwister* MT  )
{
    const float x = (float)MT->rand();
    const float y = (float)MT->rand();
    const float z = (float)MT->rand();

    const vismodule::Vector3f vertex( x, y, z );

    return vertex;
}

bool NullSpace( const vismodule::UInt32* indeces,
                Type* fs0, Type* fs1, 
                Type* fl0, Type* fl1 )
{
    bool null = true;

    for( int i=0; i<8; i++ )
    {

        if( fs0[indeces[i]] > 0.4 ) null = false;
        if( fs1[indeces[i]] > 0.4 ) null = false;
        if( fl0[indeces[i]] > 0.4 ) null = false;
        if( fl1[indeces[i]] > 0.4 ) null = false;
    }

    return null;
}

void state_txt_writer( void )
{
    std::ofstream ofs( "state.txt", std::ios::out);
        if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

        ofs<<"LATEST_STEP=NO_STEP"<<std::endl;

        ofs.close();
}

const size_t calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    vismodule::MersenneTwister* MT )
{
    const float N = density * volume_of_cell;
    const float R = MT->rand();

    size_t n = static_cast<size_t>( N );
    if ( N - n > R )
    {
        ++n;
    }

    return ( n );
}

void show_timer( time_parameters time )
{
    double initialize;
    double sampling;
    double writting;
    double mpi_reduce;
    double write_text;
    int    nparticles;

    double time_total =
        time.initialize + time.sampling + time.writting + time.mpi_reduce + time.write_text;
    double total;

#ifndef CPU_VER
    MPI_Reduce( &(time.initialize), &initialize, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.sampling),   &sampling,   1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.writting),   &writting,   1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.mpi_reduce), &mpi_reduce, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.write_text), &write_text, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time_total),      &total,      1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.nparticles), &nparticles, 1, MPI_INTEGER,MPI_SUM, 0, MPI_COMM_WORLD );
#else
    initialize = time.initialize;
    sampling   = time.sampling;
    writting   = time.writting;
    mpi_reduce = time.mpi_reduce;
    write_text = time.write_text;
    total      = time_total;
    nparticles = time.nparticles;
#endif

    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    if( mpi_rank == 0 )
    {
        printf("initialize= %8.3e [sec/step]\n", initialize);
        printf("sampling  = %8.3e [sec/step]\n", sampling  );
        printf("witting   = %8.3e [sec/step]\n", writting  );
        printf("mpi_reduce= %8.3e [sec/step]\n", mpi_reduce);
        printf("write_text= %8.3e [sec/step]\n", write_text);
        printf("total     = %8.3e [sec/step]\n", total     );
        printf("nparticles= %d\n", nparticles );
    }
}

bool initializeParameters(
    TransferFunctionSynthesizer* tfs,
    std::vector<vismodule::TransferFunction>& tf,
    ParamInfo *param_info,
    const vismodule::ObjectBase* object,
    float* sampling_volume_inverse,
    float* max_opacity, float* max_density, int* subpixel_level, float* particle_density, int* particle_limit,
    float* particle_data_size_limit,
    const std::string &visParamDir,
    const std::string &tfFilename, 
    std::string *sampling_method, 
    const int time_step )
{
    //std::cout<<"param.LoadIN()\n";
    ParamInfo& param = (*param_info);
    static bool start_flag = true;
    bool opend;

#if 0
    std::string param_filename     = "jupiter.tf";
    std::string old_param_filename = "jupiter_old.tf";
#else 
    // 20181226 start 環境変数で指定したパスを使用
    std::string param_filename     = visParamDir + tfFilename + ".tf";
    std::string old_param_filename = visParamDir + tfFilename + "_old.tf";
    // 20181226 end
#endif

    opend = LoadParameterFile( param_info, param_filename, old_param_filename );
    // add by shimomura 0808
    if (is_initial_step == true && opend == false)
    {
        //  TFファイル未読み取り判定ならば、return
        return opend; 
    }
    else if (is_initial_step == true && opend == true )
    {
        is_initial_step = false;
        start_time_step = time_step;
    }

    *particle_density         = param.getFloat( "PARTICLE_DENSITY" );
    *particle_data_size_limit = param.getFloat( "PARTICLE_DATA_SIZE_LIMIT" );
    *sampling_method = param.getString("SAMPLING_METHOD");
    //*sampling_method = "u";

    //2019 kawamura
    readTFfromParamInfo( param_info, tf, tfs );


    //std::cout<<"camera\n";
    vismodule::Camera camera;
    int height = param.getInt( "RESOLUTION_HEIGHT" );
    int width  = param.getInt( "RESOLUTION_WIDTH" );
    camera.setWindowSize( height,width );
    float min = vismodule::Math::Min( object->minObjectCoord().x(),
                                object->minObjectCoord().y(),
                                object->minObjectCoord().z() );
    float max = vismodule::Math::Max( object->maxObjectCoord().x(),
                                object->maxObjectCoord().y(),
                                object->maxObjectCoord().z() );
    const float sampling_step = (max - min) / 1E1;
    *particle_limit = param.getInt( "PARTICLE_LIMIT" );
#if 0
#else
    double total_volume = ( object->maxObjectCoord().x() - object->minObjectCoord().x() )
                        * ( object->maxObjectCoord().y() - object->minObjectCoord().y() )
                        * ( object->maxObjectCoord().z() - object->minObjectCoord().z() );
#endif
    *max_opacity = 0.98;
    *subpixel_level = CalculateSubpixelLevel( *particle_limit , camera, sampling_step, total_volume, object );

    //std::cout<<"Generator::\n";
    Generator::CalculateDensityParameters(
        &camera,
        object,
        (float)(*subpixel_level),
        sampling_step,
        *max_opacity,
        sampling_volume_inverse,
        max_density );

    tfs->setMaxOpacity( *max_opacity );
    tfs->setMaxDensity( * max_density );
    tfs->setSamplingVolumeInverse( *sampling_volume_inverse );


    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    //if( mpi_rank == RANK )
    if( mpi_rank == 0 )
    {
        fprintf( stdout , "---------initialize Parameters-------------\n" );
        fprintf( stdout , "particle_limit    = %20d\n", *particle_limit );
        fprintf( stdout , "particle_density  = %20f\n", *particle_density );
        fprintf( stdout , "resolutin_height  = %20d\n", height );
        fprintf( stdout , "resolutin_width   = %20d\n", width );
        fprintf( stdout , "total_volume      = %20.3e\n", total_volume );
        fprintf( stdout , "  |-X             = %20f\n", object->maxObjectCoord().x() );
        fprintf( stdout , "  |-Y             = %20f\n", object->maxObjectCoord().y() );
        fprintf( stdout , "  |-Z             = %20f\n", object->maxObjectCoord().z() );
        fprintf( stdout , "max_opacity       = %20.3e\n", *max_opacity );
        fprintf( stdout , "max_density       = %20.3e\n", *max_density );
        fprintf( stdout , "sampling_step     = %20.3e\n", sampling_step );
        fprintf( stdout , "subpixel_level    = %20d\n", *subpixel_level );
        fprintf( stdout , "------------------------------------\n" );
    }
    
    return opend;
}

void readTFfromParamInfo( ParamInfo* param,
                          std::vector<vismodule::TransferFunction>& tf,
                          TransferFunctionSynthesizer* tfs )
{

    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    //Read TFS
    std::vector<int> i_table;
    std::vector<float> f_table;
    EquationToken eq;
    std::vector<EquationToken> var;
    int tf_number;
    // get TF_NUMBER
    tf_number = param->getInt( "TF_NUMBER" );

    //Read 1D tf
    int resolution = param->getInt( "TF_RESOLUTION" );
    float min, max;

    tf.clear();

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        min = param->Float( tag_base +"MIN_C" );
        max = param->getFloat( tag_base +"MAX_C" );
        i_table = param->getTableInt( tag_base + "TABLE_C" );
        vismodule::ValueArray<vismodule::UInt8> u_table( i_table.size() );
        for( size_t j = 0; j<i_table.size(); j++ ) u_table[j] = (vismodule::UInt8)i_table[j];
        vismodule::ColorMap color_map( u_table, min, max );

        min = param->getFloat( tag_base +"MIN_O" );
        max = param->getFloat( tag_base +"MAX_O" );
        f_table = param->getTableFloat( tag_base + "TABLE_O" );
        vismodule::ValueArray<float> ff_table( f_table );
        vismodule::OpacityMap opacity_map( ff_table, min, max );

        vismodule::TransferFunction tfBuf;
        tfBuf.setColorMap( color_map );
        tfBuf.setOpacityMap( opacity_map );
        tf.push_back(tfBuf);
    }
    

#if 1
    // add by shimomura 2024/03/25
    std::string  equation;

    equation = param->getString( "OPACITY_SYNTH" );
    std::replace(equation.begin(), equation.end(), 'O', 'a');
    eq = tfs->convert_token(equation);
    tfs->setOpacityFunction( eq );

    equation = param->getString( "COLOR_SYNTH" );
    std::replace(equation.begin(), equation.end(), 'C', 'c');
    eq = tfs->convert_token(equation);
    tfs->setColorFunction( eq );


    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        equation = param->getString( tag_base +"VAR_C" );
        eq = tfs->convert_token(equation);


        var.push_back( eq );

    }

    tfs->setOpacityVariable( var );

    var.clear();
    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        equation = param->getString( tag_base +"VAR_O" );
        eq = tfs->convert_token(equation);

        var.push_back( eq );
    }

    tfs->setColorVariable( var );
    var.clear();
#endif 

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
            // ファイルが不完全(書き込み途中)の場合、完全になるまで一時停止する
            // ファイルが完全ならLoadINは1回で終わる
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

bool SetParticleParameter(const domain_parameters_unstruct dom, pbvr_parameters* particleBase, ParamInfo *m_param ,const int time_step)
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
                     dom.x_global_min,
                     dom.y_global_min,
                     dom.z_global_min,
                     dom.x_global_max,
                     dom.y_global_max,
                     dom.z_global_max );
            fclose( fp );
        }
        minmaxFlag = true;
    }
    // 20190318 end

    //if(mpi->rank==0)std::cout<<"start initializeTFS()\n";
    vismodule::StructuredVolumeObject* object = new vismodule::StructuredVolumeObject();//Global Min Max volume object
    vismodule::Vector3f min_vec(
        dom.x_global_min,
        dom.y_global_min,
        dom.z_global_min);
    vismodule::Vector3f max_vec(
        dom.x_global_max,
        dom.y_global_max,
        dom.z_global_max );
    object->setMinMaxObjectCoords( min_vec, max_vec );
    object->setMinMaxExternalCoords( min_vec, max_vec );
    particleBase->m_min_vec = min_vec;
    particleBase->m_max_vec = max_vec;
    // if(mpi_rank==RANK) std::cout<<"max_vec:"<<max_vec<<std::endl;

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
    if ( is_initial_step == true )
    {
        std::cout << "find no .tf!! skipping generate_particle !!!" << std::endl;
        return false;
    }
    // moved by shimomura 20240807

    return true;
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
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
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

void GenerateParticles(
    domain_parameters_unstruct dom,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    const Argument& param
)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif
    // 粒子生成コア関数呼び出し
    vismodule::PointObject* tmp_obj = NULL; 
    switch( param.m_sampling_method )
    {
        case 'u':
            std::cout << "Uniform sampling" << std::endl;
            tmp_obj =  new vismodule::CellByCellUniformSampling(  dom, values, nvariables,
                    coordinates, ncoords, connections, ncells, celltype, param.m_transfunc_array[0], particleBase.m_tf,
                    m_tfs, param.m_particle_density , NULL);
             break;

        case 'r':
            std::cout << "Rejection sampling" << std::endl;
            tmp_obj =  new vismodule::CellByCellRejectionSampling(  dom, values, nvariables,
                    coordinates, ncoords, connections, ncells, celltype, particleBase.m_tf[0], particleBase.m_tf,
                    m_tfs, particleBase.m_particle_density , NULL);
             break;

        case 'm':
            std::cout << "Metolopolis sampling" << std::endl;
            tmp_obj =  new vismodule::CellByCellMetropolisSampling(  dom, values, nvariables,
                    coordinates, ncoords, connections, ncells, celltype, particleBase.m_tf[0], particleBase.m_tf,
                    m_tfs, particleBase.m_particle_density , NULL);
             break;

        case 'h':
            std::cout << "Histogram " << std::endl;
            tmp_obj =  new vismodule::CellByCellHistogram(  dom, values, nvariables,
                    coordinates, ncoords, connections, ncells, celltype, particleBase.m_tf[0], particleBase.m_tf,
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

    //vismodule::VolumeObjectBase volume;

}

void GlyphObjectGenerator( 
    domain_parameters_unstruct dom,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
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
    GlyphSeed glyph_seed( clntMes, mpi_size, values, nvariables, coordinates, ncoords, connections, ncells, celltype, true );

    auto const& v = glyph_seed.glyph_directions();
    auto const& c = glyph_seed.glyph_coords();
    auto const& k = glyph_seed.glyph_colors();
    auto const& s = glyph_seed.glyph_sizes();

#if 0 // デバッグ用
     std::cout << "glyph_param.m_glyph_sizes        = " << glyph_param.m_glyph_sizes.size()     << std::endl; 
     std::cout << "glyph_seed.glyph_sizes()         = " << glyph_seed.glyph_sizes().size()      << std::endl; 
     std::cout << "glyph_param.m_glyph_vectors      = " << glyph_param.m_glyph_vectors.size()   << std::endl; 
     std::cout << "glyph_seed.glyph_directions()    = " << glyph_seed.glyph_directions().size() << std::endl; 
     std::cout << "m_glyph_coords.size()            = " << glyph_param.m_glyph_coords.size()    << std::endl; 
     std::cout << "glyph_seed.glyph_coords().size() = " << glyph_seed.glyph_coords().size()     << std::endl; 
     std::cout << "m_glyph_coords.size()            = " << glyph_param.m_glyph_colors.size()    << std::endl; 
     std::cout << "glyph_seed.glyph_colors()        = " << glyph_seed.glyph_colors().size()     << std::endl; 

    std::cout << "vectors:"
              << " cur = " << glyph_param.m_glyph_vectors.size()
              << " add = " << v.size() 
              << " max = " << glyph_param.m_glyph_vectors.max_size() << std::endl;
    std::cout << "coords :"
              << " cur = " << glyph_param.m_glyph_coords.size()
              << " add = " << c.size()
              << " max = " << glyph_param.m_glyph_coords.max_size() << std::endl;
    std::cout << "colors :"
              << " cur = " << glyph_param.m_glyph_colors.size()
              << " add = " << k.size()
              << " max = " << glyph_param.m_glyph_colors.max_size() << std::endl;
    std::cout << "sizes  :"
              << " cur = " << glyph_param.m_glyph_sizes.size()
              << " add = " << s.size()
              << " max = " << glyph_param.m_glyph_sizes.max_size() << std::endl;

#endif

    // 集約処理
    safe_append(glyph_param.m_glyph_vectors, v, "m_glyph_vectors");
    safe_append(glyph_param.m_glyph_coords , c, "m_glyph_coords");
    safe_append(glyph_param.m_glyph_colors , k, "m_glyph_colors");
    safe_append(glyph_param.m_glyph_sizes  , s, "m_glyph_sizes");
}

void PlotOverLineObjectGenerator(
    int time_step,
    domain_parameters_unstruct dom,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    const jpv::ParticleTransferClientMessage& clntMes,
    plot_over_line_parameters& pol_param
) //celltype  enum 型に変更
{
    //　グリフ生成処理
    PlotOverLine plot_over_line( values, nvariables, coordinates, ncoords, connections, ncells, celltype, clntMes );

    //集約処理
    int size = plot_over_line.sampling_size();
    for( int i = 0; i < size; i++ )
    {
        pol_param.m_x_axis[i] = plot_over_line.xAxis()[i];
        if ( plot_over_line.mask()[i] )
        {
            pol_param.m_mask[i] = plot_over_line.mask()[i];
            pol_param.m_values_on_line.at(i) = plot_over_line.values()[i];
        }
    }
}

void GeneratePlotOverLine( int time_step,
                             domain_parameters_unstruct dom, 
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells,
                             const  vismodule::VolumeObjectBase::CellType& celltype , PlotOverLine* plot_over_line )  
{
//       if(plot_over_line->plot_flag())
//       {
//           plot_over_line->extractPlotLine( volume );
//           plot_over_line->CellTypeReduceing();
//       } 
//
}
void GeneratePlotOverLine( const int time_step,
                           const vismodule::UnstructuredVolumeObject* volume,
                           PlotOverLine* plot_over_line  ) 
{
       if(plot_over_line->plot_flag())
       {
           plot_over_line->extractPlotLine( volume );
           plot_over_line->CellTypeReduceing();
       } 

}

// 使用されていない
void callPlotOverLine( int time_step,
                             domain_parameters_unstruct dom, 
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells,
                             const  vismodule::VolumeObjectBase::CellType& celltype , PlotOverLine* plot_over_line )  
{

        vismodule::UnstructuredVolumeObject* object = new vismodule::UnstructuredVolumeObject;
        object -> setNNodes(ncoords);
        object -> setNCells(ncells);
        object -> setVeclen(nvariables);
        std::vector<float> Values;
        Values.resize(ncoords * nvariables);
        for (int i =0; i<nvariables ; i++) 
        {    
            for (int k=0; k< ncoords; k++) 
            {    
                Values[k+i*ncoords] = values[i][k];
            }    
        }    

        vismodule::AnyValueArray Var(Values);
        object -> setValues(Var);
        vismodule::ValueArray<float> Coords(coordinates, ncoords*3);
        object -> setCoords(Coords);

        vismodule::ValueArray<vismodule::UInt32> Connections;
        switch ( celltype )  // 2次要素は一旦除外
        {
            case 4: // pbvr::VolumeObjectBase::Tetrahedra:
                {
                    int nconnection = ncells * 4;
                    Connections.allocate(nconnection); // hexのみ
                    for (int i =0; i< nconnection ;i ++)
                    {
                        Connections[i] = connections[i];
                    }
                    object -> setCellType(vismodule::VolumeObjectBase::Tetrahedra);
                    break;
                }
            case 8: //  pbvr::VolumeObjectBase::Hexahedra:
                {
                    int nconnection = ncells * 8;
                    Connections.allocate(nconnection); // hexのみ
                    for (int i =0; i< nconnection ;i ++)
                    {
                        Connections[i] = connections[i];
                    }
                    object -> setCellType(vismodule::VolumeObjectBase::Hexahedra);
                    break;
                }
            case 6: // pbvr::VolumeObjectBase::Prism:
                {
                    int nconnection = ncells * 6;
                    Connections.allocate(nconnection); // hexのみ
                    for (int i =0; i< nconnection ;i ++)
                    {
                        Connections[i] = connections[i];
                    }
                    object -> setCellType(vismodule::VolumeObjectBase::Prism);
                    break;
                }
            case 5: //pbvr::VolumeObjectBase::Pyramid:
                {
                    int nconnection = ncells * 5;
                    Connections.allocate(nconnection); // hexのみ
                    for (int i =0; i< nconnection ;i ++)
                    {
                        Connections[i] = connections[i];
                    }
                    object -> setCellType(vismodule::VolumeObjectBase::Pyramid);
                    break;
                }
            default:
                {
                    std::cout << "Unsupported cell type." << std::endl;
                    return;
                }
        }

        object -> setConnections(Connections);

        GeneratePlotOverLine(time_step, object, plot_over_line);
        delete object;
}

#ifdef VTK
void SetVariables(
    const kvs::UnstructuredVolumeObject* volume,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int& nvariables,
    std::unique_ptr<float[]>& coordinates,
    int& ncoords,
    std::unique_ptr<unsigned int[]>& connections,
    int& ncells,
    vismodule::VolumeObjectBase::CellType& celltype
)
{
    std::cout << "volume->cellType() = " << volume->cellType() << std::endl;

    // kvsのセルタイプ別の処理
    switch ( volume->cellType() )
    {
    case 1: // vismodule::VolumeObjectBase::Tetrahedra:
        {
            celltype = vismodule::VolumeObjectBase::Tetrahedra;
            break;
        }
    case 3: //vismodule::VolumeObjectBase::QuadraticTetrahedra:
        {
            celltype = vismodule::VolumeObjectBase::QuadraticTetrahedra;
            break;
        }
    case 2: //  vismodule::VolumeObjectBase::Hexahedra:
        {
            celltype = vismodule::VolumeObjectBase::Hexahedra;
            break;
        }
    case 4: //vismodule::VolumeObjectBase::QuadraticHexahedra:
        {
            celltype = vismodule::VolumeObjectBase::QuadraticHexahedra;
            break;
        }
    case 7: // vismodule::VolumeObjectBase::Prism:
        {
            celltype = vismodule::VolumeObjectBase::Prism;
            break;
        }
    case 5: //vismodule::VolumeObjectBase::Pyramid:
        {
            celltype = vismodule::VolumeObjectBase::Pyramid;
            break;
        }
    default:
        {
            std::cout << "Unsupported cell type." << std::endl; 
            return;
        }
    }

    int nvariables = object -> veclen();    
    int ncoords = object -> nnodes(); 
    for ( int j = 0; j < nvariables; j++ )
    {
        for ( int i = 0; i < ncoords; i++ )
        {
            int  it = j * ncoords  + i;
            values[j][i] = object ->values().at<Type>(it);
        }
    }
}
#endif // VTK
*/