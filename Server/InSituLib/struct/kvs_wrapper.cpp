#include "kvs_wrapper_common.h"
#include "kvs_wrapper.h"

#include <cstdio>
#include <fstream>
#include <vector>

#include <vismodule/ValueArray>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/PointObject>
#include <vismodule/TransferFunction>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/TransferFunctionSynthesizer>

#include <vismodule/ParameterFileReader>

// Generate
#include <vismodule/PointObjectGenerator>
#include <vismodule/GlyphSeedGenerator>
#include <vismodule/PlotOverLineGenerator>

#include <vismodule/GenerateParticle>
#include <vismodule/GenerateGlyph>
#include <vismodule/GeneratePOL>

namespace Generator = vismodule::CellByCellParticleGenerator;

static bool is_initial_step = false;
static size_t start_time_step = 0;

void OutputCoordMinMaxFile(
    const domain_parameters_struct& dom,
    const std::string& coordMinMaxFilePath
)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

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

bool generate_particles(
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

    bool result = false;
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

    result = SetParameterFilePath(
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

    if ( !result ) return false;

    char  arg_dummy0[] = "dummy";
    char* arg_dummy[]  = { arg_dummy0, NULL };

    ParticleProperty particle_property;
    GlyphProperty glyph_property;
    PlotOverLineProperty pol_property;
    MultiVolumePropertyList mvpl;
    static NameListFile particleNameListFile;
    static NameListFile glyphNameListFile;
    static NameListFile POLNameListFile;
    particle_property.m_transfunc_synthesizer = new TransferFunctionSynthesizer();
    particle_property.m_camera                = new vismodule::Camera();

    SetParticleParameter( dom, tfFilePath, tfFilePath_old, particle_property, mvpl, particleNameListFile );
    SetGlyphParameter( glyphParameterPath, glyphParameterPath_old, glyph_property, glyphNameListFile );
    SetPlotOverLineParameter( plotOverLineParameterPath, plotOverLineParameterPath_old, pol_property, POLNameListFile );

    const int tf_number  = particle_property.m_transfunc_array.size();
    const int resolution = pol_property.m_sampling_size;

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
    std::vector<float> values_on_line( resolution, 0 );
    std::vector<int>   mask( resolution, 0 );
    std::vector<float> x_axis( resolution, 0 );
    
    ServerMode server_mode = ServerMode::IS;
    vismodule::PointObject* point_object = nullptr;
    vismodule::PointObjectGenerator point_object_generator;
    point_object = point_object_generator.GenerateParticleStruct(
        particle_property, dom, values, nvariables
    );

    MakeParticle( point_object, particle_coords, particle_colors, particle_normals ); // InSitu only
    MakeHistgram( point_object, tf_number, tmp_c_bins, tmp_o_bins ); // CS common
    MakeParticleMinMax( particle_property.m_transfunc_synthesizer, tf_number, tmp_max, tmp_min ); // CS common

    delete point_object;

    if ( glyph_property.m_glyph_flag )
    {
        vismodule::KVSMLObjectGlyph* glyph_object = new vismodule::KVSMLObjectGlyph;
        vismodule::GlyphSeedGenerator glyph_creator;
        int number_of_divide = mpi_size;
        glyph_creator.GenerateGlyphStruct(
            glyph_property, number_of_divide, dom,
            values, nvariables, server_mode, glyph_object
        );

        MakeGlyph( glyph_object, glyph_coords, glyph_vectors, glyph_sizes, glyph_colors ); // InSitu only

        delete glyph_object;
    }

    if ( pol_property.m_plot_flag )
    {
        vismodule::KVSMLObjectPlotOverLine* pol_object = new vismodule::KVSMLObjectPlotOverLine;
        PlotOverLineGenerator pol_generator;
        pol_generator.GeneratePOLStruct(
            pol_property, dom, values, nvariables, pol_object
        );

        for( size_t i = 0; i < resolution; i++ )
        { 
            x_axis[i] = pol_object->x_axis()[i];
            if ( pol_object->mask()[i] )
            {
                mask[i]           = 1;
                values_on_line[i] = pol_object->values_on_line()[i];
            }
        }

        delete pol_object;
    }

    OutputCoordMinMaxFile( dom, coordMinMaxFilePath );

    if ( mpi_rank == 0 )
    {
        particleNameListFile.setFileName( tfFilePath_step );
        particleNameListFile.write();
    }

    // データ出力
    OutputParticles(
        particle_property, mvpl, start_time_step, time_step, tf_number, nvariables, particleFilePrefix,
        stateFilePath, historyFilePath, particle_coords, particle_colors,
        particle_normals, tmp_c_bins, tmp_o_bins, tmp_max, tmp_min
    );

    if ( glyph_property.m_glyph_flag )
    {
        OutputGlyphs(
            time_step, glyphFilePrefix, glyph_coords,
            glyph_vectors, glyph_sizes, glyph_colors
        );
    }

    if ( pol_property.m_plot_flag )
    {
        OutputLine( time_step, plotOverLineFilePrefix, values_on_line, mask, x_axis );
    }
   
    delete tmp_c_bins;
    delete tmp_o_bins;
    delete tmp_max;
    delete tmp_min;
    delete particle_property.m_transfunc_synthesizer;
    delete particle_property.m_camera;

    return true;
}

bool SetParticleParameter( 
    const domain_parameters_struct& dom,
    const std::string& tfFilePath,
    const std::string& tfFilePath_old,
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
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
    int size = 0;
    char* buf;

    if ( mpi_rank == 0 )
    {
        bool is_file_exist = false;
        std::ifstream tfFile( tfFilePath );

        if ( tfFile.good() )
        {
            is_file_exist = true;
            ppr.readParticleParameterFile( tfFilePath.c_str() );
            nameListFile = ppr.getNameListFile();
            std::rename( tfFilePath.c_str(), tfFilePath_old.c_str() );
        }
        else
        {
            ppr.setNameListFile( nameListFile );
        }

        if ( is_file_exist ) size = nameListFile.byteSize();
        else size = 0;

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
        if( mpi_rank > 0 ) nameListFile.unpack( buf );
        delete[] buf;
    }

    if ( mpi_rank > 0 ) ppr.setNameListFile( nameListFile );
    ppr.setParticleParameter( particle_property );

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

    particle_property.m_sampling_step = ( max - min ) / 1E1;
    const float sampling_step = particle_property.m_sampling_step;

    vismodule::StructuredVolumeObject object;
    object.setMinMaxObjectCoords( min_object_coords, max_object_coords );
    object.setMinMaxExternalCoords( min_object_coords, max_object_coords );

    const double total_volume = ( dom.x_global_max - dom.x_global_min )
                              * ( dom.y_global_max - dom.y_global_min )
                              * ( dom.z_global_max - dom.z_global_min );

    const float max_opacity      = 0.98;
    const int particle_limit     = particle_property.m_particle_limit;
    const float particle_density = particle_property.m_particle_density;
    const int subpixel_level     = CalculateSubpixelLevel(
        particle_limit,
        *particle_property.m_camera,
        sampling_step,
        total_volume,
        &object
    );

    float sampling_volume_inverse;
    float max_density;

    Generator::CalculateDensityParameters(
        particle_property.m_camera,
        &object,
        (float)subpixel_level,
        sampling_step,
        max_opacity,
        &sampling_volume_inverse,
        &max_density
    );

    particle_property.m_transfunc_synthesizer->setMaxOpacity( max_opacity );
    particle_property.m_transfunc_synthesizer->setMaxDensity( max_density );
    particle_property.m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );

    if( mpi_rank == 0 )
    {
        fprintf( stdout , "---------initialize Parameters----------------------------------------\n" );
        fprintf( stdout , "particle_limit    = %20d\n"  , particle_limit                             );
        fprintf( stdout , "particle_density  = %20f\n"  , particle_density                           );
        fprintf( stdout , "resolutin_height  = %20d\n"  , particle_property.m_camera->windowHeight() );
        fprintf( stdout , "resolutin_width   = %20d\n"  , particle_property.m_camera->windowWidth()  );
        fprintf( stdout , "total_volume      = %20.3e\n", total_volume                               );
        fprintf( stdout , "  |-X             = %20f\n"  , object.maxObjectCoord().x()                );
        fprintf( stdout , "  |-Y             = %20f\n"  , object.maxObjectCoord().y()                );
        fprintf( stdout , "  |-Z             = %20f\n"  , object.maxObjectCoord().z()                );
        fprintf( stdout , "max_opacity       = %20.3e\n", max_opacity                                );
        fprintf( stdout , "max_density       = %20.3e\n", max_density                                );
        fprintf( stdout , "sampling_step     = %20.3e\n", sampling_step                              );
        fprintf( stdout , "subpixel_level    = %20d\n"  , subpixel_level                             );
        fprintf( stdout , "----------------------------------------------------------------------\n" );
    }

    return true;
}