#include "kvs_wrapper_common.h"
#include "kvs_wrapper.h"

#include <cstdio>
#include <cfloat>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <memory>
#include <cmath>

#include <vismodule/ValueArray>
#include <vismodule/PointObject>
#include <vismodule/KVSMLObjectPoint>
#include <vismodule/PointExporter>
#include <vismodule/RGBColor>
#include <vismodule/MersenneTwister>
#include <vismodule/CellBase>
#include <vismodule/TetrahedralCell>
#include <vismodule/HexahedralCell>
#include <vismodule/QuadraticTetrahedralCell>
#include <vismodule/QuadraticHexahedralCell>
#include <vismodule/PrismaticCell>
#include <vismodule/PyramidalCell>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/UnstructuredVolumeObject>

#ifndef CPU_VER
#include <mpi.h>
#endif

#include <vismodule/ParticleProperty>

#include <vismodule/ParameterFileReader>
#include <vismodule/ParameterFileWriter>

// Generate
#include <vismodule/GenerateParticle>
#include <vismodule/PointObjectGenerator>
#include <vismodule/GlyphSeedGenerator>
#include <vismodule/PlotOverLineGenerator>
#include <vismodule/PlotOverTimeGenerator>

#ifdef EXTEND_FILE_FORMAT
#include <vismodule/UnstructuredVolumeImporter>
#include <vtkSmartPointerBase.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/extendedfileformat/VtkUnstructuredFileFormat>
#include <kvs/extendedfileformat/VtkXmlUnstructuredGrid>
#include <kvs/extendedfileformat/VtkImporter>
#endif

namespace Generator = vismodule::CellByCellParticleGenerator;

static bool is_initial_step = true;
static std::size_t start_time_step = 0;

static void CollectParticleMinMax(
    ParticleProperty& particle_property,
    const domain_parameters_unstruct& dom,
    Type** values,
    const int nvariables,
    float* coordinates,
    const int ncoords,
    unsigned int* connections,
    const int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    const int tf_number,
    float* tmp_max,
    float* tmp_min
)
{
    vismodule::PointObjectGenerator point_object_generator;
    const char tmp_sampling_method = particle_property.m_sampling_method;
    particle_property.m_sampling_method = 'x';

    vismodule::PointObject* point_object = point_object_generator.GenerateParticleUnstruct(
        particle_property, dom, values, nvariables, coordinates,
        ncoords, connections, ncells, celltype, ServerMode::IS
    );

    MakeParticleMinMax( particle_property.m_transfunc_synthesizer, tf_number, tmp_max, tmp_min );

    delete point_object;
    particle_property.m_sampling_method = tmp_sampling_method;
}

static void CollectParticleHistogram(
    ParticleProperty& particle_property,
    const domain_parameters_unstruct& dom,
    Type** values,
    const int nvariables,
    float* coordinates,
    const int ncoords,
    unsigned int* connections,
    const int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    const int tf_number,
    vismodule::UInt64* tmp_c_bins,
    vismodule::UInt64* tmp_o_bins
)
{
    vismodule::PointObjectGenerator point_object_generator;
    const char tmp_sampling_method = particle_property.m_sampling_method;
    particle_property.m_sampling_method = 'h';

    vismodule::PointObject* point_object = point_object_generator.GenerateParticleUnstruct(
        particle_property, dom, values, nvariables, coordinates,
        ncoords, connections, ncells, celltype, ServerMode::IS
    );

    MakeHistgram( point_object, tf_number, tmp_c_bins, tmp_o_bins );

    delete point_object;
    particle_property.m_sampling_method = tmp_sampling_method;
}

static void GenerateParticleObject(
    ParticleProperty& particle_property,
    const domain_parameters_unstruct& dom,
    Type** values,
    const int nvariables,
    float* coordinates,
    const int ncoords,
    unsigned int* connections,
    const int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    std::vector<float>& particle_coords,
    std::vector<Byte>& particle_colors,
    std::vector<float>& particle_normals
)
{
    vismodule::PointObjectGenerator point_object_generator;
    vismodule::PointObject* point_object = point_object_generator.GenerateParticleUnstruct(
        particle_property, dom, values, nvariables, coordinates,
        ncoords, connections, ncells, celltype, ServerMode::IS
    );

    MakeParticle( point_object, particle_coords, particle_colors, particle_normals );

    delete point_object;
}

namespace
{

std::string EnsembleParticleFilePrefix(
    const std::string& particleFilePrefix,
    const std::string& statisticPrefix
)
{
    if ( particleFilePrefix.size() >= 2 &&
         particleFilePrefix.compare( particleFilePrefix.size() - 2, 2, "t_" ) == 0 )
    {
        return particleFilePrefix.substr( 0, particleFilePrefix.size() - 2 ) + statisticPrefix;
    }

    return particleFilePrefix + statisticPrefix;
}

void OutputEnsembleStatisticParticles(
    ParticleProperty& particle_property,
    const MultiVolumePropertyList& mvpl,
    const int time_step,
    const std::string& particleFilePrefix,
    const std::vector<float>& coords,
    const std::vector<Byte>& colors,
    const std::vector<float>& normals
)
{
    static bool first_step = true;
    static int count;
    static int num_nodes;

#ifndef CPU_VER
    static MPI_Comm new_comm;
#endif

    if ( first_step )
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
        MPI_Get_processor_name( procname, &resultlen );
        procname_g = new char[MPI_MAX_PROCESSOR_NAME * numprocs];
        MPI_Allgather( procname, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                       procname_g, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                       MPI_COMM_WORLD );

        int color;
        count = 1;
        for ( color = 0; color < numprocs; color++ )
        {
            procname_p = procname_g + MPI_MAX_PROCESSOR_NAME * color;
            if ( color > 0 )
            {
                procname_bak = procname_p - MPI_MAX_PROCESSOR_NAME;
                if ( strcmp( procname_p, procname_bak ) != 0 ) count++;
            }
            if ( strcmp( procname_p, procname ) == 0 ) break;
        }

        delete[] procname_g;

        MPI_Comm_split( MPI_COMM_WORLD, color, myrank, &new_comm );

        int split_numprocs;
        MPI_Comm_size( new_comm, &split_numprocs );
        num_nodes = numprocs / split_numprocs;
        if ( numprocs % split_numprocs > 0 ) num_nodes++;
#else
        count = 1;
        num_nodes = 1;
#endif
        first_step = false;
    }

    std::stringstream ss;
    ss << std::setfill( '0' ) << std::setw( 5 ) << time_step;
    ss << "_";
    ss << std::setfill( '0' ) << std::setw( 7 ) << count;
    ss << "_";
    ss << std::setfill( '0' ) << std::setw( 7 ) << num_nodes;
    ss << ".kvsml";
    const std::string particleFilePath = particleFilePrefix + ss.str();

    int particle_size = coords.size();
    int* recvcounts;
    int* displs;
    int new_rank;
    int new_number_of_process;

#ifndef CPU_VER
    MPI_Comm_rank( new_comm, &new_rank );
    MPI_Comm_size( new_comm, &new_number_of_process );
#else
    new_rank = 0;
    new_number_of_process = 1;
#endif

    displs = new int[new_number_of_process];
    recvcounts = new int[new_number_of_process];

#ifndef CPU_VER
    MPI_Allgather( &particle_size, 1, MPI_INT,
                   recvcounts, 1, MPI_INT,
                   new_comm );
#else
    recvcounts[0] = particle_size;
#endif

    displs[0] = 0;
    for ( int i = 1; i < new_number_of_process; i++ )
    {
        displs[i] = displs[i - 1] + recvcounts[i - 1];
    }

    const int gathered_size = displs[new_number_of_process - 1] + recvcounts[new_number_of_process - 1];
    vismodule::ValueArray<float> new_coords( gathered_size );
    vismodule::ValueArray<Byte> new_colors( gathered_size );
    vismodule::ValueArray<float> new_normals( gathered_size );

#ifndef CPU_VER
    vismodule::ValueArray<float> tmp_coords( coords );
    vismodule::ValueArray<Byte> tmp_colors( colors );
    vismodule::ValueArray<float> tmp_normals( normals );

    MPI_Gatherv( tmp_coords.pointer(), particle_size, MPI_FLOAT,
                 new_coords.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, new_comm );
    MPI_Gatherv( tmp_colors.pointer(), particle_size, MPI_BYTE,
                 new_colors.pointer(), recvcounts, displs, MPI_BYTE,
                 0, new_comm );
    MPI_Gatherv( tmp_normals.pointer(), particle_size, MPI_FLOAT,
                 new_normals.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, new_comm );
#else
    for ( int i = 0; i < particle_size; i++ )
    {
        new_coords[i] = coords[i];
        new_colors[i] = colors[i];
        new_normals[i] = normals[i];
    }
#endif

    if ( new_rank == 0 )
    {
        vismodule::PointObject* point_object = new vismodule::PointObject(
            new_coords, new_colors, new_normals, particle_property.m_subpixel_level
        );
        point_object->setMinMaxObjectCoords( mvpl.m_total_min_object_coord, mvpl.m_total_max_object_coord );

        if ( async_io_enabled )
        {
            pbvr::ParticleWriteThread* particle_write_thread = &pwt;
            particle_write_thread->join( true );
            particle_write_thread->setPointObject( point_object );
            particle_write_thread->setFilename( particleFilePath.c_str() );
            particle_write_thread->work( true );
        }
        else
        {
            vismodule::KVSMLObjectPoint* kvsml_object =
                new vismodule::PointExporter<vismodule::KVSMLObjectPoint>( *point_object );
            kvsml_object->setWritingDataType( vismodule::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->write( particleFilePath.c_str() );
            delete kvsml_object;
        }

        delete point_object;
    }

    delete[] displs;
    delete[] recvcounts;
}

void AppendGeneratedParticles(
    vismodule::PointObject* point_object,
    std::vector<float>& coords,
    std::vector<Byte>& colors,
    std::vector<float>& normals
)
{
    if ( point_object )
    {
        MakeParticle( point_object, coords, colors, normals );
        delete point_object;
    }
}

inline const size_t CalculateNumberOfParticlesV35(
    const float density,
    const float volume_of_cell,
    const float repetition,
    vismodule::MersenneTwister* mt
)
{
    const float n_particles = density * volume_of_cell * repetition;
    const float random = static_cast<float>( mt->rand() );

    size_t n = static_cast<size_t>( n_particles );
    if ( n_particles - n > random ) ++n;

    return n;
}

void AppendRejectedStatisticParticle(
    const float scalar,
    const vismodule::Vector3f& coord,
    const vismodule::Vector3f& normal,
    vismodule::TransferFunction& tf,
    const float sampling_volume_inverse,
    const float max_opacity,
    const float max_density,
    vismodule::MersenneTwister* mt,
    std::vector<float>& coords,
    std::vector<Byte>& colors,
    std::vector<float>& normals
)
{
    const float opacity = tf.opacityMap().at( scalar );
    const float density = opacity < max_opacity ?
        -std::log( 1.0f - opacity ) * sampling_volume_inverse :
        max_density;

    if ( density <= max_density * static_cast<float>( mt->rand() ) ) return;

    const vismodule::RGBColor color = tf.colorMap().at( scalar );

    coords.push_back( coord.x() );
    coords.push_back( coord.y() );
    coords.push_back( coord.z() );
    colors.push_back( color.r() );
    colors.push_back( color.g() );
    colors.push_back( color.b() );
    normals.push_back( normal.x() );
    normals.push_back( normal.y() );
    normals.push_back( normal.z() );
}

} // namespace

void OutputCoordMinMaxFile(
    const domain_parameters_unstruct& dom,
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
    std::string plotOverTimeFilePrefix;
    std::string tfJsonPath;
    std::string tfJsonPath_old;
    std::string tfJsonPath_step;
    std::string glyphParameterPath;
    std::string glyphParameterPath_old;
    std::string plotOverLineParameterPath;
    std::string plotOverLineParameterPath_old; 
    std::string plotOverTimeParameterPath;
    std::string plotOverTimeParameterPath_old;

    SetParameterFilePath(
        time_step,
        historyFilePath,
        stateFilePath,
        coordMinMaxFilePath,
        particleFilePrefix,
        glyphFilePrefix,
        plotOverLineFilePrefix,
        plotOverTimeFilePrefix,
        tfJsonPath,
        tfJsonPath_old,
        tfJsonPath_step,
        glyphParameterPath,
        glyphParameterPath_old,
        plotOverLineParameterPath,
        plotOverLineParameterPath_old,
        plotOverTimeParameterPath,
        plotOverTimeParameterPath_old
    );

    char  arg_dummy0[] = "dummy";
    char* arg_dummy[]  = { arg_dummy0, NULL };

    ParticleProperty particle_property;
    GlyphProperty glyph_property;
    PlotOverLineProperty pol_property;
    PlotOverTimeProperty pot_property;
    MultiVolumePropertyList mvpl;
    particle_property.m_transfunc_synthesizer = new TransferFunctionSynthesizer();
    particle_property.m_camera                = new vismodule::Camera();

    bool object_generation_enabled = false;
    SetParticleParameter(
        dom, tfJsonPath, tfJsonPath_old, particle_property, mvpl,
        nvariables, object_generation_enabled
    );
    if ( object_generation_enabled )
    {
        SetGlyphParameter( glyphParameterPath, glyphParameterPath_old, glyph_property );
        SetPlotOverLineParameter( plotOverLineParameterPath, plotOverLineParameterPath_old, pol_property );
        SetPlotOverTimeParameter( plotOverTimeParameterPath, plotOverTimeParameterPath_old, pot_property );
    }

    const int tf_number  = particle_property.m_transfunc_array.size();
    const int resolution = object_generation_enabled ? pol_property.m_sampling_size : 0;

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

    for ( std::size_t i = 0; i < (DEFAULT_NBINS * tf_number); i++ )
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

    // plot over time paraemters
    std::vector<float> value_on_time( nvariables, 0 );
    bool pot_mask = false;

    ServerMode server_mode = ServerMode::IS;

    CollectParticleMinMax(
        particle_property, dom, values, nvariables, coordinates,
        ncoords, connections, ncells, celltype, tf_number, tmp_max, tmp_min
    );

#ifndef CPU_VER
    if ( mpi_size > 1 )
    {
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, ( tf_number * 2 ), MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, ( tf_number * 2 ), MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    }
#endif

    for( std::size_t i = 0; i < tf_number; i++ )
    {
        const float color_variable_min   = tmp_min[2 * i + 1];
        const float color_variable_max   = tmp_max[2 * i + 1];
        const float opacity_variable_min = tmp_min[2 * i    ];
        const float opacity_variable_max = tmp_max[2 * i    ];

        particle_property.m_transfunc_array[i].m_server_color_variable_min   = color_variable_min;
        particle_property.m_transfunc_array[i].m_server_color_variable_max   = color_variable_max;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_min = opacity_variable_min;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_max = opacity_variable_max;

        if( particle_property.m_transfunc_array[i].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[i].setColorRange( color_variable_min, color_variable_max );
        }

        if( particle_property.m_transfunc_array[i].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[i].setOpacityRange( opacity_variable_min, opacity_variable_max );
        }
    }

    CollectParticleHistogram(
        particle_property, dom, values, nvariables, coordinates,
        ncoords, connections, ncells, celltype, tf_number, tmp_c_bins, tmp_o_bins
    );

    if ( object_generation_enabled )
    {
        GenerateParticleObject(
            particle_property, dom, values, nvariables, coordinates,
            ncoords, connections, ncells, celltype,
            particle_coords, particle_colors, particle_normals
        );

        if ( glyph_property.m_glyph_flag )
        {
            vismodule::KVSMLObjectGlyph* glyph_object = new vismodule::KVSMLObjectGlyph;
            vismodule::GlyphSeedGenerator glyph_creator;
            int number_of_divide = mpi_size;
            glyph_creator.GenerateGlyphUnstruct(
                glyph_property, number_of_divide, values, nvariables, coordinates,
                ncoords, connections, ncells, celltype, server_mode, glyph_object
            );

            MakeGlyph( glyph_object, glyph_coords, glyph_vectors, glyph_sizes, glyph_colors ); // InSitu only

            delete glyph_object;
        }

        // if ( pol_property.m_plot_flag )
        if ( true ) // 常に生成し続ける、将来的に変わる可能性あり
        {
            vismodule::KVSMLObjectPlotOverLine* pol_object = new vismodule::KVSMLObjectPlotOverLine;
            PlotOverLineGenerator pol_generator;
            pol_generator.GeneratePOLUnstruct(
                pol_property, values, nvariables, coordinates,
                ncoords, connections, ncells, celltype, pol_object
            );

            for( std::size_t i = 0; i < resolution; i++ )
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

        // if ( pot_property.m_plot_flag )
        if ( true ) // 常に生成し続ける、将来的に変わる可能性あり
        {
            PlotOverTimeGenerator pot_generator;
            pot_mask = pot_generator.GeneratePOTUnstruct(
                pot_property, values, nvariables, coordinates,
                ncoords, connections, ncells, celltype, value_on_time
            );
        }
    }

    OutputCoordMinMaxFile( dom, coordMinMaxFilePath );

    if ( object_generation_enabled )
    {
        OutputParticleFiles(
            particle_property, mvpl, time_step, particleFilePrefix,
            particle_coords, particle_colors, particle_normals
        );
    }

    OutputParticleHistory(
        particle_property, tf_number, nvariables, historyFilePath,
        object_generation_enabled, tmp_c_bins, tmp_o_bins, tmp_max, tmp_min
    );

    if ( object_generation_enabled && glyph_property.m_glyph_flag )
    {
        OutputGlyphs(
            time_step, glyphFilePrefix, glyph_coords,
            glyph_vectors, glyph_sizes, glyph_colors
        );
    }

    // if ( pol_property.m_plot_flag )
    if ( object_generation_enabled ) // 常に出力し続ける、将来的に変わる可能性あり
    {
        OutputLine( time_step, plotOverLineFilePrefix, values_on_line, mask, x_axis );
    }

    // if ( pot_property.m_plot_flag )
    if ( object_generation_enabled ) // 常に出力し続ける、将来的に変わる可能性あり
    {
        OutputPOT( time_step, plotOverTimeFilePrefix, pot_mask, value_on_time );
    }

    // OutputParticlesで更新したdefault_old.jsonをtimestep別JSONに保存する
    if ( object_generation_enabled && mpi_rank == 0 )
    {
        std::ifstream src( tfJsonPath_old.c_str(), std::ios::binary );
        std::ofstream dst( tfJsonPath_step.c_str(), std::ios::binary );
        dst << src.rdbuf();
    }

    // 粒子ファイル書き込みスレッドが終了するまで待機
    // async_io_enabled, pwtはkvs_wrapper_common.cppに宣言
    if ( async_io_enabled )
    {
        std::cout << "Particle write thread is active."             << std::endl;
        std::cout << "Waiting for particle write thread to finish." << std::endl;
        pbvr::ParticleWriteThread* particle_write_thread = &pwt;
        particle_write_thread->join( true );
        std::cout << "Particle write thread is finished." << std::endl;
    }
    else
    {
        std::cout << "Particle write thread is not active." << std::endl;
    }

    std::cout << "Waiting for all processes to finish." << std::endl;

#ifndef CPU_VER
    MPI_Barrier( MPI_COMM_WORLD ); // すべてのプロセスでファイルの書き込む処理が終了するまで待機
#endif

    std::cout << "All processes have finished." << std::endl;

    if ( mpi_rank == 0 )
    {
        std::ofstream ofs( stateFilePath.c_str(), std::ios::out );
        if( !ofs.is_open() ) std::cout << "Cannot open state.txt" << std::endl;

        ofs << "START_STEP  = " << start_time_step << std::endl;
        ofs << "LATEST_STEP = " << time_step       << std::endl;

        ofs.close();
    }
   
    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
    delete[] tmp_max;
    delete[] tmp_min;
    delete particle_property.m_transfunc_synthesizer;
    delete particle_property.m_camera;

}

bool ensemble_generate_particles(
    int time_step,
    domain_parameters_unstruct dom,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype
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
    std::string plotOverTimeFilePrefix;
    std::string tfFilePath;
    std::string tfFilePath_old;
    std::string tfFilePath_step;
    std::string glyphParameterPath;
    std::string glyphParameterPath_old;
    std::string plotOverLineParameterPath;
    std::string plotOverLineParameterPath_old;
    std::string plotOverTimeParameterPath;
    std::string plotOverTimeParameterPath_old;

    result = SetParameterFilePath(
        time_step,
        historyFilePath,
        stateFilePath,
        coordMinMaxFilePath,
        particleFilePrefix,
        glyphFilePrefix,
        plotOverLineFilePrefix,
        plotOverTimeFilePrefix,
        tfFilePath,
        tfFilePath_old,
        tfFilePath_step,
        glyphParameterPath,
        glyphParameterPath_old,
        plotOverLineParameterPath,
        plotOverLineParameterPath_old,
        plotOverTimeParameterPath,
        plotOverTimeParameterPath_old
    );

    if ( !result ) return false;

    ParticleProperty particle_property;
    MultiVolumePropertyList mvpl;
    particle_property.m_transfunc_synthesizer = new TransferFunctionSynthesizer();
    particle_property.m_camera = new vismodule::Camera();

    SetParticleParameter( dom, tfFilePath, tfFilePath_old, particle_property, mvpl );

    const int tf_number = particle_property.m_transfunc_array.size();
    std::vector<vismodule::TransferFunction> transfer_functions( tf_number );
    for ( int i = 0; i < tf_number; i++ )
    {
        transfer_functions[i] = particle_property.m_transfunc_array[i];
    }

    const std::string averageFilePrefix = EnsembleParticleFilePrefix( particleFilePrefix, "ave_" );
    const std::string varianceFilePrefix = EnsembleParticleFilePrefix( particleFilePrefix, "var_" );
    const std::string coefficientFilePrefix = EnsembleParticleFilePrefix( particleFilePrefix, "cov_" );

    std::vector<float> average_coords;
    std::vector<Byte> average_colors;
    std::vector<float> average_normals;
    std::vector<float> variance_coords;
    std::vector<Byte> variance_colors;
    std::vector<float> variance_normals;
    std::vector<float> coefficient_coords;
    std::vector<Byte> coefficient_colors;
    std::vector<float> coefficient_normals;

    std::vector<vismodule::UInt64> tmp_c_bins( DEFAULT_NBINS * tf_number, 0 );
    std::vector<vismodule::UInt64> tmp_o_bins( DEFAULT_NBINS * tf_number, 0 );
    std::vector<float> tmp_max( tf_number * 2, FLT_MIN );
    std::vector<float> tmp_min( tf_number * 2, FLT_MAX );

#ifndef CPU_VER
    if ( mpi_size <= 1 )
    {
        std::cout << "ensemble_generate_particles requires MPI ensemble ranks." << std::endl;
        delete particle_property.m_transfunc_synthesizer;
        delete particle_property.m_camera;
        return false;
    }

#if _OPENMP
    const int max_threads = omp_get_max_threads();
#else
    const int max_threads = 1;
#endif

    std::vector<std::vector<vismodule::CellBase<Type>*> > cell( max_threads );
    for ( int thread = 0; thread < max_threads; thread++ )
    {
        cell[thread].resize( nvariables, nullptr );
        for ( int variable = 0; variable < nvariables; variable++ )
        {
            switch ( celltype )
            {
            case vismodule::VolumeObjectBase::Tetrahedra:
                cell[thread][variable] = new vismodule::TetrahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                break;
            case vismodule::VolumeObjectBase::Hexahedra:
                cell[thread][variable] = new vismodule::HexahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                break;
            case vismodule::VolumeObjectBase::QuadraticTetrahedra:
                cell[thread][variable] = new vismodule::QuadraticTetrahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                break;
            case vismodule::VolumeObjectBase::QuadraticHexahedra:
                cell[thread][variable] = new vismodule::QuadraticHexahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                break;
            case vismodule::VolumeObjectBase::Prism:
                cell[thread][variable] = new vismodule::PrismaticCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                break;
            case vismodule::VolumeObjectBase::Pyramid:
                cell[thread][variable] = new vismodule::PyramidalCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                break;
            default:
                std::cout << "Unsupported cell type." << std::endl;
                for ( int i = 0; i <= thread; i++ )
                {
                    for ( int j = 0; j < nvariables; j++ ) delete cell[i][j];
                }
                delete particle_property.m_transfunc_synthesizer;
                delete particle_property.m_camera;
                return false;
            }
        }
    }

    const float sampling_volume_inverse = particle_property.m_transfunc_synthesizer->getSamplingVolumeInverse();
    const float max_opacity = particle_property.m_transfunc_synthesizer->getMaxOpacity();
    const float max_density = particle_property.m_transfunc_synthesizer->getMaxDensity();
    const float particle_density = 1.0f;
    const int ens_number = mpi_size;
    const float repetitions = 1.0f / static_cast<float>( ens_number );

    std::vector<vismodule::Real32> vertex_coords;
    std::vector<vismodule::Real32> vertex_scalars;
    std::vector<vismodule::Real32> vertex_normals;
    std::vector<int> vertex_cellids;
    std::vector<vismodule::Real32> sq_scalars;
    std::vector<vismodule::Real32> tmp_term;

    std::cout << __LINE__ << std::endl; 
#pragma omp parallel
    {
#if _OPENMP
        const int nthreads = omp_get_num_threads();
        const int thid = omp_get_thread_num();
#else
        const int nthreads = 1;
        const int thid = 0;
#endif
        vismodule::UInt32 cell_index[SIMD_BLK_SIZE];
        vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
        vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];
        float volume_array[SIMD_BLK_SIZE];
        int nparticles_array[SIMD_BLK_SIZE];
        float scalar_array[SIMD_BLK_SIZE];
        float grad_scalar[SIMD_BLK_SIZE];
        float grad_array_x[SIMD_BLK_SIZE];
        float grad_array_y[SIMD_BLK_SIZE];
        float grad_array_z[SIMD_BLK_SIZE];
        std::vector<float> o_scalars_array[SIMD_BLK_SIZE];
        for ( int i = 0; i < SIMD_BLK_SIZE; i++ ) o_scalars_array[i].resize( tf_number );

        std::vector<vismodule::Real32> th_vertex_coords;
        std::vector<vismodule::Real32> th_vertex_scalars;
        std::vector<vismodule::Real32> th_vertex_normals;
        std::vector<int> th_vertex_cellids;
        std::vector<vismodule::Real32> th_sq_scalars;
        std::vector<vismodule::Real32> th_tmp_term;
        vismodule::MersenneTwister mt( thid + mpi_rank * nthreads );

#pragma omp for schedule( dynamic ) nowait
        for ( size_t index = 0; index < static_cast<size_t>( ncells ); index += SIMD_BLK_SIZE )
        {
            const int remain = ( ncells - index > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : ncells - index;
            for ( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = static_cast<vismodule::UInt32>( index + cell_BLK );
            }
            for ( int variable = 0; variable < nvariables; variable++ )
            {
                cell[thid][variable]->bindCellArray( remain, cell_index );
            }
            for ( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell[thid][0]->bindCell( cell_index[cell_BLK] );
                volume_array[cell_BLK] = cell[thid][0]->volume();
            }

            for ( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                nparticles_array[cell_BLK] = static_cast<int>(
                    CalculateNumberOfParticlesV35( max_density, volume_array[cell_BLK], particle_density * repetitions, &mt )
                );
            }

            int p_id = 0;
            for ( int cell_BLK = 0; cell_BLK < remain + 1; cell_BLK++ )
            {
                const int nparticles_in_cell = cell_BLK < remain ? nparticles_array[cell_BLK] : 1;
                for ( int i = 0; i < nparticles_in_cell; i += SIMD_BLK_SIZE )
                {
                    const int remain_BLK = ( nparticles_in_cell - i > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : nparticles_in_cell - i;
                    if ( cell_BLK < remain )
                    {
                        for ( int j = 0; j < remain_BLK; j++ )
                        {
                            cell_index[p_id] = static_cast<vismodule::UInt32>( index + cell_BLK );
                            local_coord_array[p_id] = cell[thid][0]->randomSampling_MT( &mt );
                            p_id++;
                            if ( p_id == SIMD_BLK_SIZE )
                            {
                                for ( int k = 0; k < nvariables; k++ ) cell[thid][k]->bindCellArray( p_id, cell_index );
                                cell[thid][0]->setLocalPointArray( p_id, local_coord_array );
                                cell[thid][0]->transformLocalToGlobalArray( p_id, local_coord_array, global_coord_array );
                                particle_property.m_transfunc_synthesizer->CalculateScalarsArray(
                                    cell[thid], p_id, local_coord_array, global_coord_array, transfer_functions, scalar_array
                                );
                                cell[thid][0]->CalcAveragedScalarGrad( p_id, grad_scalar, grad_array_x, grad_array_y, grad_array_z );
                                for ( int k = 0; k < p_id; k++ )
                                {
                                    th_vertex_scalars.push_back( scalar_array[k] );
                                    th_vertex_coords.push_back( local_coord_array[k].x() );
                                    th_vertex_coords.push_back( local_coord_array[k].y() );
                                    th_vertex_coords.push_back( local_coord_array[k].z() );
                                    th_vertex_cellids.push_back( cell_index[k] );
                                    th_vertex_normals.push_back( -grad_array_x[k] );
                                    th_vertex_normals.push_back( -grad_array_y[k] );
                                    th_vertex_normals.push_back( -grad_array_z[k] );
                                    th_sq_scalars.push_back( scalar_array[k] * scalar_array[k] );
                                    th_tmp_term.push_back( scalar_array[k] * grad_array_x[k] );
                                    th_tmp_term.push_back( scalar_array[k] * grad_array_y[k] );
                                    th_tmp_term.push_back( scalar_array[k] * grad_array_z[k] );
                                }
                                p_id = 0;
                            }
                        }
                    }
                    else if ( p_id > 0 )
                    {
                        for ( int k = 0; k < nvariables; k++ ) cell[thid][k]->bindCellArray( p_id, cell_index );
                        cell[thid][0]->setLocalPointArray( p_id, local_coord_array );
                        cell[thid][0]->transformLocalToGlobalArray( p_id, local_coord_array, global_coord_array );
                        particle_property.m_transfunc_synthesizer->CalculateScalarsArray(
                            cell[thid], p_id, local_coord_array, global_coord_array, transfer_functions, scalar_array
                        );
                        cell[thid][0]->CalcAveragedScalarGrad( p_id, grad_scalar, grad_array_x, grad_array_y, grad_array_z );
                        for ( int k = 0; k < p_id; k++ )
                        {
                            th_vertex_scalars.push_back( scalar_array[k] );
                            th_vertex_coords.push_back( local_coord_array[k].x() );
                            th_vertex_coords.push_back( local_coord_array[k].y() );
                            th_vertex_coords.push_back( local_coord_array[k].z() );
                            th_vertex_cellids.push_back( cell_index[k] );
                            th_vertex_normals.push_back( -grad_array_x[k] );
                            th_vertex_normals.push_back( -grad_array_y[k] );
                            th_vertex_normals.push_back( -grad_array_z[k] );
                            th_sq_scalars.push_back( scalar_array[k] * scalar_array[k] );
                            th_tmp_term.push_back( scalar_array[k] * grad_array_x[k] );
                            th_tmp_term.push_back( scalar_array[k] * grad_array_y[k] );
                            th_tmp_term.push_back( scalar_array[k] * grad_array_z[k] );
                        }
                        p_id = 0;
                    }
                }
            }
        }

#pragma omp critical
        {
            vertex_coords.insert( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            vertex_scalars.insert( vertex_scalars.end(), th_vertex_scalars.begin(), th_vertex_scalars.end() );
            vertex_normals.insert( vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end() );
            vertex_cellids.insert( vertex_cellids.end(), th_vertex_cellids.begin(), th_vertex_cellids.end() );
            sq_scalars.insert( sq_scalars.end(), th_sq_scalars.begin(), th_sq_scalars.end() );
            tmp_term.insert( tmp_term.end(), th_tmp_term.begin(), th_tmp_term.end() );
        }
    }

    std::cout << __LINE__ << std::endl; 
    std::vector<std::vector<float> > v_scalars( 2 );
    std::vector<std::vector<float> > v_coords( 2 );
    std::vector<std::vector<float> > v_normals( 2 );
    std::vector<std::vector<int> > v_cellids( 2 );
    std::vector<std::vector<float> > v_sq( 2 );
    std::vector<std::vector<float> > v_tmp( 2 );
    v_scalars[0].swap( vertex_scalars );
    v_coords[0].swap( vertex_coords );
    v_normals[0].swap( vertex_normals );
    v_cellids[0].swap( vertex_cellids );
    v_sq[0].swap( sq_scalars );
    v_tmp[0].swap( tmp_term );

    int cur = 0;
    int nxt = 1;
    for ( int shift = 1; shift < ens_number; shift++ )
    {
        const int send_to = ( mpi_rank + 1 ) % mpi_size;
        const int recv_from = ( mpi_rank - 1 + mpi_size ) % mpi_size;
        const int send_size = static_cast<int>( v_scalars[cur].size() );
        int recv_size = 0;
        MPI_Request reqs[2];
        MPI_Isend( &send_size, 1, MPI_INT, send_to, 0, MPI_COMM_WORLD, &reqs[0] );
        MPI_Irecv( &recv_size, 1, MPI_INT, recv_from, 0, MPI_COMM_WORLD, &reqs[1] );
        MPI_Waitall( 2, reqs, MPI_STATUSES_IGNORE );

        std::vector<float> recv_scalars( recv_size );
        std::vector<float> recv_coords( 3 * recv_size );
        std::vector<float> recv_normals( 3 * recv_size );
        std::vector<int> recv_cellids( recv_size );
        std::vector<float> recv_sq_scalars( recv_size );
        std::vector<float> recv_tmp_term( 3 * recv_size );
        MPI_Request req_recv[6];
        MPI_Request req_send[6];
        MPI_Irecv( recv_cellids.data(), recv_size, MPI_INT, recv_from, 12, MPI_COMM_WORLD, &req_recv[0] );
        MPI_Irecv( recv_scalars.data(), recv_size, MPI_FLOAT, recv_from, 10, MPI_COMM_WORLD, &req_recv[1] );
        MPI_Irecv( recv_coords.data(), 3 * recv_size, MPI_FLOAT, recv_from, 11, MPI_COMM_WORLD, &req_recv[2] );
        MPI_Irecv( recv_normals.data(), 3 * recv_size, MPI_FLOAT, recv_from, 13, MPI_COMM_WORLD, &req_recv[3] );
        MPI_Irecv( recv_sq_scalars.data(), recv_size, MPI_FLOAT, recv_from, 14, MPI_COMM_WORLD, &req_recv[4] );
        MPI_Irecv( recv_tmp_term.data(), 3 * recv_size, MPI_FLOAT, recv_from, 15, MPI_COMM_WORLD, &req_recv[5] );
        MPI_Isend( v_cellids[cur].data(), send_size, MPI_INT, send_to, 12, MPI_COMM_WORLD, &req_send[0] );
        MPI_Isend( v_scalars[cur].data(), send_size, MPI_FLOAT, send_to, 10, MPI_COMM_WORLD, &req_send[1] );
        MPI_Isend( v_coords[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 11, MPI_COMM_WORLD, &req_send[2] );
        MPI_Isend( v_normals[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 13, MPI_COMM_WORLD, &req_send[3] );
        MPI_Isend( v_sq[cur].data(), send_size, MPI_FLOAT, send_to, 14, MPI_COMM_WORLD, &req_send[4] );
        MPI_Isend( v_tmp[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 15, MPI_COMM_WORLD, &req_send[5] );
        MPI_Waitall( 6, req_recv, MPI_STATUSES_IGNORE );

#pragma omp parallel
        {
#if _OPENMP
            const int thid = omp_get_thread_num();
#else
            const int thid = 0;
#endif
            vismodule::UInt32 cell_index[SIMD_BLK_SIZE];
            vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
            vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];
            std::vector<float> o_scalars_array[SIMD_BLK_SIZE];
            float scalar_array[SIMD_BLK_SIZE];
            float grad_scalar[SIMD_BLK_SIZE];
            float grad_array_x[SIMD_BLK_SIZE];
            float grad_array_y[SIMD_BLK_SIZE];
            float grad_array_z[SIMD_BLK_SIZE];
            for ( int i = 0; i < SIMD_BLK_SIZE; i++ ) o_scalars_array[i].resize( tf_number );

#pragma omp for schedule( dynamic )
            for ( int i = 0; i < recv_size; i += SIMD_BLK_SIZE )
            {
                const int remain_BLK = ( recv_size - i > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : recv_size - i;
                for ( int j = 0; j < remain_BLK; j++ )
                {
                    cell_index[j] = static_cast<vismodule::UInt32>( recv_cellids[i + j] );
                    local_coord_array[j] = vismodule::Vector3f(
                        recv_coords[3 * ( i + j )],
                        recv_coords[3 * ( i + j ) + 1],
                        recv_coords[3 * ( i + j ) + 2]
                    );
                }
                for ( int k = 0; k < nvariables; k++ ) cell[thid][k]->bindCellArray( remain_BLK, cell_index );
                cell[thid][0]->setLocalPointArray( remain_BLK, local_coord_array );
                cell[thid][0]->transformLocalToGlobalArray( remain_BLK, local_coord_array, global_coord_array );
                particle_property.m_transfunc_synthesizer->CalculateScalarsArray(
                    cell[thid], remain_BLK, local_coord_array, global_coord_array, transfer_functions, scalar_array
                );
                cell[thid][0]->CalcAveragedScalarGrad( remain_BLK, grad_scalar, grad_array_x, grad_array_y, grad_array_z );
                for ( int j = 0; j < remain_BLK; j++ )
                {
                    const float scalar = scalar_array[j];
                    recv_scalars[i + j] += scalar;
                    recv_normals[3 * ( i + j )] += -grad_array_x[j];
                    recv_normals[3 * ( i + j ) + 1] += -grad_array_y[j];
                    recv_normals[3 * ( i + j ) + 2] += -grad_array_z[j];
                    recv_sq_scalars[i + j] += scalar * scalar;
                    recv_tmp_term[3 * ( i + j )] += scalar * grad_array_x[j];
                    recv_tmp_term[3 * ( i + j ) + 1] += scalar * grad_array_y[j];
                    recv_tmp_term[3 * ( i + j ) + 2] += scalar * grad_array_z[j];
                }
            }
        }

        MPI_Waitall( 6, req_send, MPI_STATUSES_IGNORE );
        v_scalars[nxt].swap( recv_scalars );
        v_coords[nxt].swap( recv_coords );
        v_normals[nxt].swap( recv_normals );
        v_cellids[nxt].swap( recv_cellids );
        v_sq[nxt].swap( recv_sq_scalars );
        v_tmp[nxt].swap( recv_tmp_term );
        std::swap( cur, nxt );
    }

    vertex_scalars.swap( v_scalars[cur] );
    vertex_coords.swap( v_coords[cur] );
    vertex_normals.swap( v_normals[cur] );
    vertex_cellids.swap( v_cellids[cur] );
    sq_scalars.swap( v_sq[cur] );
    tmp_term.swap( v_tmp[cur] );

    std::vector<float> tmp_varience( vertex_scalars.size() );
    std::vector<float> tmp_varience_normals( 3 * vertex_scalars.size() );
    const float invert_num = 1.0f / static_cast<float>( ens_number );
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        vertex_scalars[i] *= invert_num;
        sq_scalars[i] *= invert_num;
    }
    for ( size_t i = 0; i < tmp_term.size(); i++ )
    {
        tmp_term[i] = -2.0f * invert_num * tmp_term[i];
        vertex_normals[i] *= -invert_num;
    }
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        tmp_varience[i] = sq_scalars[i] - vertex_scalars[i] * vertex_scalars[i];
        if ( tmp_varience[i] < 0.0f ) tmp_varience[i] = 0.0f;
        tmp_varience_normals[3 * i] = tmp_term[3 * i] - ( -2.0f * vertex_scalars[i] * vertex_normals[3 * i] );
        tmp_varience_normals[3 * i + 1] = tmp_term[3 * i + 1] - ( -2.0f * vertex_scalars[i] * vertex_normals[3 * i + 1] );
        tmp_varience_normals[3 * i + 2] = tmp_term[3 * i + 2] - ( -2.0f * vertex_scalars[i] * vertex_normals[3 * i + 2] );
    }

    const float delta = 1.0e-30f;
    const float eps = 1.0e-5f;
    std::vector<float> co_varietion( vertex_scalars.size() );
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        co_varietion[i] = vertex_scalars[i] > eps ? std::sqrt( tmp_varience[i] ) / vertex_scalars[i] : delta;
    }

    std::cout << __LINE__ << std::endl; 
#pragma omp parallel
    {
#if _OPENMP
        const int thid = omp_get_thread_num();
#else
        const int thid = 0;
#endif
        std::vector<float> th_average_coords;
        std::vector<Byte> th_average_colors;
        std::vector<float> th_average_normals;
        std::vector<float> th_variance_coords;
        std::vector<Byte> th_variance_colors;
        std::vector<float> th_variance_normals;
        std::vector<float> th_coefficient_coords;
        std::vector<Byte> th_coefficient_colors;
        std::vector<float> th_coefficient_normals;
        vismodule::MersenneTwister mt( 10 + mpi_rank + thid );
        vismodule::UInt32 cell_index[SIMD_BLK_SIZE];
        vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
        vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];

#pragma omp for schedule( dynamic )
        for ( size_t i = 0; i < vertex_scalars.size(); i += SIMD_BLK_SIZE )
        {
            const int remain_BLK = ( vertex_scalars.size() - i > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : vertex_scalars.size() - i;
            for ( int j = 0; j < remain_BLK; j++ )
            {
                cell_index[j] = static_cast<vismodule::UInt32>( vertex_cellids[i + j] );
                local_coord_array[j] = vismodule::Vector3f(
                    vertex_coords[3 * ( i + j )],
                    vertex_coords[3 * ( i + j ) + 1],
                    vertex_coords[3 * ( i + j ) + 2]
                );
            }
            for ( int k = 0; k < nvariables; k++ ) cell[thid][k]->bindCellArray( remain_BLK, cell_index );
            cell[thid][0]->setLocalPointArray( remain_BLK, local_coord_array );
            cell[thid][0]->transformLocalToGlobalArray( remain_BLK, local_coord_array, global_coord_array );

            for ( int j = 0; j < remain_BLK; j++ )
            {
                const size_t idx = i + j;
                const vismodule::Vector3f average_normal(
                    vertex_normals[3 * idx],
                    vertex_normals[3 * idx + 1],
                    vertex_normals[3 * idx + 2]
                );
                const vismodule::Vector3f variance_normal(
                    tmp_varience_normals[3 * idx],
                    tmp_varience_normals[3 * idx + 1],
                    tmp_varience_normals[3 * idx + 2]
                );
                AppendRejectedStatisticParticle(
                    vertex_scalars[idx], global_coord_array[j], average_normal, transfer_functions[0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_average_coords, th_average_colors, th_average_normals
                );
                AppendRejectedStatisticParticle(
                    tmp_varience[idx], global_coord_array[j], variance_normal, transfer_functions[0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_variance_coords, th_variance_colors, th_variance_normals
                );
                AppendRejectedStatisticParticle(
                    co_varietion[idx], global_coord_array[j], variance_normal, transfer_functions[0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_coefficient_coords, th_coefficient_colors, th_coefficient_normals
                );
            }
        }

#pragma omp critical
        {
            average_coords.insert( average_coords.end(), th_average_coords.begin(), th_average_coords.end() );
            average_colors.insert( average_colors.end(), th_average_colors.begin(), th_average_colors.end() );
            average_normals.insert( average_normals.end(), th_average_normals.begin(), th_average_normals.end() );
            variance_coords.insert( variance_coords.end(), th_variance_coords.begin(), th_variance_coords.end() );
            variance_colors.insert( variance_colors.end(), th_variance_colors.begin(), th_variance_colors.end() );
            variance_normals.insert( variance_normals.end(), th_variance_normals.begin(), th_variance_normals.end() );
            coefficient_coords.insert( coefficient_coords.end(), th_coefficient_coords.begin(), th_coefficient_coords.end() );
            coefficient_colors.insert( coefficient_colors.end(), th_coefficient_colors.begin(), th_coefficient_colors.end() );
            coefficient_normals.insert( coefficient_normals.end(), th_coefficient_normals.begin(), th_coefficient_normals.end() );
        }
    }
#else
    std::cout << "ensemble_generate_particles requires MPI; CPU_VER path is disabled." << std::endl;
    delete particle_property.m_transfunc_synthesizer;
    delete particle_property.m_camera;
    return false;
#endif

    std::cout << __LINE__ << std::endl; 
#ifndef CPU_VER
    //MakeParticleMinMax( particle_property.m_transfunc_synthesizer, tf_number, tmp_max.data(), tmp_min.data() );
    std::cout << __LINE__ << std::endl; 

    OutputCoordMinMaxFile( dom, coordMinMaxFilePath );

    OutputParticles(
        particle_property, mvpl, start_time_step, time_step, tf_number, nvariables, averageFilePrefix,
        stateFilePath, historyFilePath, average_coords, average_colors,
        average_normals, tmp_c_bins.data(), tmp_o_bins.data(), tmp_max.data(), tmp_min.data()
    );

    OutputEnsembleStatisticParticles(
        particle_property, mvpl, time_step, varianceFilePrefix,
        variance_coords, variance_colors, variance_normals
    );

    OutputEnsembleStatisticParticles(
        particle_property, mvpl, time_step, coefficientFilePrefix,
        coefficient_coords, coefficient_colors, coefficient_normals
    );

    if ( mpi_rank == 0 )
    {
        ParameterFileReader ppr;
        NameListFile nameListFile;
        ppr.readParticleParameterFile( tfFilePath_old.c_str() );
        nameListFile = ppr.getNameListFile();
        nameListFile.setFileName( tfFilePath_step );
        nameListFile.write();
    }

    if ( async_io_enabled )
    {
        std::cout << "Particle write thread is active." << std::endl;
        std::cout << "Waiting for particle write thread to finish." << std::endl;
        pbvr::ParticleWriteThread* particle_write_thread = &pwt;
        particle_write_thread->join( true );
        std::cout << "Particle write thread is finished." << std::endl;
    }
    else
    {
        std::cout << "Particle write thread is not active." << std::endl;
    }

    std::cout << "Waiting for all processes to finish." << std::endl;
#ifndef CPU_VER
    MPI_Barrier( MPI_COMM_WORLD );
#endif
    std::cout << "All processes have finished." << std::endl;

    if ( mpi_rank == 0 )
    {
        std::ofstream ofs( stateFilePath.c_str(), std::ios::out );
        if ( !ofs.is_open() ) std::cout << "Cannot open state.txt" << std::endl;

        ofs << "START_STEP  = " << start_time_step << std::endl;
        ofs << "LATEST_STEP = " << time_step << std::endl;
        ofs.close();
    }

    for ( int thread = 0; thread < max_threads; thread++ )
    {
        for ( int variable = 0; variable < nvariables; variable++ )
        {
            delete cell[thread][variable];
        }
    }
    delete particle_property.m_transfunc_synthesizer;
    delete particle_property.m_camera;

    return true;
#endif
}

bool SetParticleParameter( 
    const domain_parameters_unstruct& dom,
    const std::string& tfJsonPath,
    const std::string& tfJsonPath_old,
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    const int nvariables,
    bool& object_generation_enabled
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
    char* buf = nullptr;
    int object_generation_enabled_int = 0;
    static bool hasCachedParticleProperty = false;
    static std::vector<char> cachedParticlePropertyBuffer;

    if ( mpi_rank == 0 )
    {
        std::ifstream tfJson( tfJsonPath );
        std::ifstream tfJsonOld( tfJsonPath_old );

        bool loaded = false;
        if ( tfJson.good() )
        {
            loaded = ppr.readTransferFunctionFromJson( tfJsonPath.c_str(), particle_property );
            if ( loaded )
            {
                std::rename( tfJsonPath.c_str(), tfJsonPath_old.c_str() );
            }
        }
        if ( !loaded && !tfJson.good() && hasCachedParticleProperty )
        {
            size = static_cast<int>( cachedParticlePropertyBuffer.size() );
            if ( size > 0 )
            {
                buf = new char[size];
                for ( int i = 0; i < size; ++i )
                {
                    buf[i] = cachedParticlePropertyBuffer[i];
                }
                particle_property.unpack( buf );
                delete[] buf;
                buf = nullptr;
            }
            loaded = size > 0;
        }
        if ( !loaded && tfJsonOld.good() )
        {
            loaded = ppr.readTransferFunctionFromJson( tfJsonPath_old.c_str(), particle_property );
        }

        if ( loaded )
        {
            object_generation_enabled_int = 1;
        }
        else if ( size == 0 )
        {
            std::cout << "================================================================" << std::endl;
            std::cout << "[WARN] Failed to load transfer function json." << std::endl;
            std::cout << "[WARN] Files: " << FileNameOnly( tfJsonPath ) << " and "
                      << FileNameOnly( tfJsonPath_old ) << std::endl;
            std::cout << "[INFO] VIS_PARAM_DIR = " << EnvValueOrUnset( "VIS_PARAM_DIR" ) << std::endl;
            std::cout << "[INFO] PARTICLE_DIR  = " << EnvValueOrUnset( "PARTICLE_DIR" ) << std::endl;
            std::cout << "[INFO] Set default particle parameters and skip object generation." << std::endl;
            std::cout << "================================================================" << std::endl;

            SetDefaultParticleParameter( particle_property, mvpl, nvariables );
            object_generation_enabled_int = 0;
        }

        size = particle_property.byteSize();
        std::cout << "size = " << size << std::endl;

        if ( size > 0 )
        {
            buf = new char[size];
            particle_property.pack( buf );
            if ( object_generation_enabled_int != 0 )
            {
                cachedParticlePropertyBuffer.assign( buf, buf + size );
                hasCachedParticleProperty = true;
            }
        }

    }

#ifndef CPU_VER
    MPI_Bcast( &object_generation_enabled_int, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
    object_generation_enabled = object_generation_enabled_int != 0;

    if ( size > 0 )
    {
        if ( mpi_rank > 0 ) buf = new char [size];
#ifndef CPU_VER
        MPI_Bcast( buf, size, MPI_CHARACTER, 0, MPI_COMM_WORLD );
#endif
        if( mpi_rank > 0 ) particle_property.unpack( buf );
        if ( mpi_rank > 0 && object_generation_enabled )
        {
            cachedParticlePropertyBuffer.assign( buf, buf + size );
            hasCachedParticleProperty = true;
        }
        particle_property.UpdateTransferFunctionSynthesizer();
        delete[] buf;
    }
    else
    {
        return false;
    }

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

    vismodule::StructuredVolumeObject object;
    object.setMinMaxObjectCoords( min_object_coords, max_object_coords );
    object.setMinMaxExternalCoords( min_object_coords, max_object_coords );

    const double total_volume = ( dom.x_global_max - dom.x_global_min )
                              * ( dom.y_global_max - dom.y_global_min )
                              * ( dom.z_global_max - dom.z_global_min );

    const float max_opacity           = 0.98;
    const int particle_limit          = particle_property.m_particle_limit;
    const float extra_opacity_factor  = particle_property.m_extra_opacity_factor;
    particle_property.m_sampling_step = ( max - min ) / 1E1 / extra_opacity_factor;
    const float sampling_step         = particle_property.m_sampling_step;
    const int subpixel_level          = CalculateSubpixelLevel(
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
        fprintf( stdout , "---------initialize Parameters--------------------------------------------\n" );
        fprintf( stdout , "particle_limit       = %20d\n"   , particle_limit                             );
        fprintf( stdout , "extra_opacity_factor = %20f\n"   , extra_opacity_factor                       );
        fprintf( stdout , "resolutin_height     = %20d\n"   , particle_property.m_camera->windowHeight() );
        fprintf( stdout , "resolutin_width      = %20d\n"   , particle_property.m_camera->windowWidth()  );
        fprintf( stdout , "total_volume         = %20.3e\n" , total_volume                               );
        fprintf( stdout , "  |-X                = %20f\n"   , object.maxObjectCoord().x()                );
        fprintf( stdout , "  |-Y                = %20f\n"   , object.maxObjectCoord().y()                );
        fprintf( stdout , "  |-Z                = %20f\n"   , object.maxObjectCoord().z()                );
        fprintf( stdout , "max_opacity          = %20.3e\n" , max_opacity                                );
        fprintf( stdout , "max_density          = %20.3e\n" , max_density                                );
        fprintf( stdout , "sampling_step        = %20.3e\n" , sampling_step                              );
        fprintf( stdout , "subpixel_level       = %20d\n"   , subpixel_level                             );
        fprintf( stdout , "--------------------------------------------------------------------------\n" );
    }

    return true;
}

#ifdef EXTEND_FILE_FORMAT
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

    std::string historyFilePath;
    std::string stateFilePath;
    std::string coordMinMaxFilePath;
    std::string particleFilePrefix;
    std::string glyphFilePrefix;
    std::string plotOverLineFilePrefix;
    std::string plotOverTimeFilePrefix;
    std::string tfJsonPath;
    std::string tfJsonPath_old;
    std::string tfJsonPath_step;
    std::string glyphParameterPath;
    std::string glyphParameterPath_old;
    std::string plotOverLineParameterPath;
    std::string plotOverLineParameterPath_old; 
    std::string plotOverTimeParameterPath;
    std::string plotOverTimeParameterPath_old;

    SetParameterFilePath(
        time_step,
        historyFilePath,
        stateFilePath,
        coordMinMaxFilePath,
        particleFilePrefix,
        glyphFilePrefix,
        plotOverLineFilePrefix,
        plotOverTimeFilePrefix,
        tfJsonPath,
        tfJsonPath_old,
        tfJsonPath_step,
        glyphParameterPath,
        glyphParameterPath_old,
        plotOverLineParameterPath,
        plotOverLineParameterPath_old,
        plotOverTimeParameterPath,
        plotOverTimeParameterPath_old
    );

    char  arg_dummy0[] = "dummy";
    char* arg_dummy[]  = { arg_dummy0, NULL };

    ParticleProperty particle_property;
    GlyphProperty glyph_property;
    PlotOverLineProperty pol_property;
    PlotOverTimeProperty pot_property;
    MultiVolumePropertyList mvpl;
    particle_property.m_transfunc_synthesizer = new TransferFunctionSynthesizer();
    particle_property.m_camera                = new vismodule::Camera();

    bool object_generation_enabled = false;
    SetParticleParameter(
        dom, tfJsonPath, tfJsonPath_old, particle_property, mvpl,
        nvariables, object_generation_enabled
    );
    if ( object_generation_enabled )
    {
        SetGlyphParameter( glyphParameterPath, glyphParameterPath_old, glyph_property );
        SetPlotOverLineParameter( plotOverLineParameterPath, plotOverLineParameterPath_old, pol_property );
        SetPlotOverTimeParameter( plotOverTimeParameterPath, plotOverTimeParameterPath_old, pot_property );
    }

    const int tf_number  = particle_property.m_transfunc_array.size();
    const int resolution = object_generation_enabled ? pol_property.m_sampling_size : 0;

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

    for ( std::size_t i = 0; i < (DEFAULT_NBINS * tf_number); i++ )
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

    // plot over time paraemters
    std::vector<float> value_on_time;
    bool pot_mask = false;

    int nvariables = 0;

    // 先にセルタイプを持っておく
    kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid input_vtu( ucd );
    std::vector<kvs::UnstructuredVolumeObject::CellType> kvs_cell_type_vector;

    for( auto vtu : input_vtu.eachCellType() )
    {
        kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( &vtu );
        kvs::UnstructuredVolumeObject* volume = &importer;
        kvs_cell_type_vector.push_back( volume->cellType() );
    }

    auto convert_cell_type = [&](
        kvs::UnstructuredVolumeObject::CellType kvs_cell_type,
        std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
        int& nvariables,
        std::unique_ptr<float[]>& coordinates,
        int& ncoords,
        std::unique_ptr<unsigned int[]>& connections,
        int& ncells,
        vismodule::VolumeObjectBase::CellType& celltype
    )
    {
        vismodule::UnstructuredVolumeImporter importer;
        importer.import( input_vtu, static_cast<int>(kvs_cell_type) );
        vismodule::UnstructuredVolumeObject* volume = &importer;
        domain_parameters_unstruct tmp_dom; // not use
        nvariables = 0;

        store_volume_in_variables_array_unstruct(
            volume, tmp_dom, values, nvariables, coordinates,
            ncoords, connections, ncells, celltype
        );
    };

    auto collect_cell_type_min_max = [&](
        std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
        const int nvariables,
        std::unique_ptr<float[]>& coordinates,
        const int ncoords,
        std::unique_ptr<unsigned int[]>& connections,
        const int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype
    )
    {
        std::vector<Type*> raw_pointers_vector( nvariables );
        for ( std::size_t j = 0; j < nvariables; ++j )
        {
            raw_pointers_vector[j] = values.get()[j].get();
        }

        CollectParticleMinMax(
            particle_property, dom, raw_pointers_vector.data(), nvariables, coordinates.get(),
            ncoords, connections.get(), ncells, celltype, tf_number, tmp_max, tmp_min
        );
    };

    auto process_cell_type = [&](
        std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
        const int nvariables,
        std::unique_ptr<float[]>& coordinates,
        const int ncoords,
        std::unique_ptr<unsigned int[]>& connections,
        const int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype
    )
    {

        if ( value_on_time.size() != nvariables ) value_on_time.resize( nvariables ); // Plot Over Time

        std::vector<Type*> raw_pointers_vector( nvariables );
        for ( std::size_t j = 0; j < nvariables; ++j )
        {
            raw_pointers_vector[j] = values.get()[j].get();
        }

        ServerMode server_mode = ServerMode::IS;

        CollectParticleHistogram(
            particle_property, dom, raw_pointers_vector.data(), nvariables, coordinates.get(),
            ncoords, connections.get(), ncells, celltype, tf_number, tmp_c_bins, tmp_o_bins
        );

        if ( object_generation_enabled )
        {
            GenerateParticleObject(
                particle_property, dom, raw_pointers_vector.data(), nvariables, coordinates.get(),
                ncoords, connections.get(), ncells, celltype,
                particle_coords, particle_colors, particle_normals
            );

            if ( glyph_property.m_glyph_flag )
            {
                vismodule::KVSMLObjectGlyph* glyph_object = new vismodule::KVSMLObjectGlyph;
                vismodule::GlyphSeedGenerator glyph_creator;
                int number_of_divide = mpi_size;
                glyph_creator.GenerateGlyphUnstruct(
                    glyph_property, number_of_divide, raw_pointers_vector.data(), nvariables, coordinates.get(),
                    ncoords, connections.get(), ncells, celltype, server_mode, glyph_object
                );

                MakeGlyph( glyph_object, glyph_coords, glyph_vectors, glyph_sizes, glyph_colors ); // InSitu only

                delete glyph_object;
            }

            if ( pol_property.m_plot_flag )
            {
                vismodule::KVSMLObjectPlotOverLine* pol_object = new vismodule::KVSMLObjectPlotOverLine;
                PlotOverLineGenerator pol_generator;
                pol_generator.GeneratePOLUnstruct(
                    pol_property, raw_pointers_vector.data(), nvariables, coordinates.get(),
                    ncoords, connections.get(), ncells, celltype, pol_object
                );

                for( std::size_t j = 0; j < resolution; j++ )
                {
                    x_axis[j] = pol_object->x_axis()[j];
                    if ( pol_object->mask()[j] )
                    {
                        mask[j]           = 1;
                        values_on_line[j] = pol_object->values_on_line()[j];
                    }
                }

                delete pol_object;
            }

            // if ( pot_property.m_plot_flag )
            if ( true ) // 常に生成し続ける、将来的に変わる可能性あり
            {
                PlotOverTimeGenerator pot_generator;

                // すでにPlotOverTimeを取得している場合はスキップ
                if ( !pot_mask )
                {
                    pot_mask = pot_generator.GeneratePOTUnstruct(
                        pot_property, raw_pointers_vector.data(), nvariables, coordinates.get(),
                        ncoords, connections.get(), ncells, celltype, value_on_time
                    );
                }
            }
        }
    };

    for ( std::size_t i = 0; i < kvs_cell_type_vector.size(); i++ )
    {
        std::unique_ptr<std::unique_ptr<Type[]>[]> values;
        nvariables = 0;
        std::unique_ptr<float[]> coordinates;
        int ncoords = 0;
        std::unique_ptr<unsigned int[]> connections;
        int ncells = 0;
        vismodule::VolumeObjectBase::CellType celltype;

        convert_cell_type(
            kvs_cell_type_vector[i], values, nvariables, coordinates,
            ncoords, connections, ncells, celltype
        );

        collect_cell_type_min_max( values, nvariables, coordinates, ncoords, connections, ncells, celltype );
    } // for ( std::size_t i = 0; i < kvs_cell_type_vector.size(); i++ )

#ifndef CPU_VER
    if ( mpi_size > 1 )
    {
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, ( tf_number * 2 ), MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, ( tf_number * 2 ), MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    }
#endif

    for( std::size_t i = 0; i < tf_number; i++ )
    {
        const float color_variable_min   = tmp_min[2 * i + 1];
        const float color_variable_max   = tmp_max[2 * i + 1];
        const float opacity_variable_min = tmp_min[2 * i    ];
        const float opacity_variable_max = tmp_max[2 * i    ];

        particle_property.m_transfunc_array[i].m_server_color_variable_min   = color_variable_min;
        particle_property.m_transfunc_array[i].m_server_color_variable_max   = color_variable_max;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_min = opacity_variable_min;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_max = opacity_variable_max;

        if( particle_property.m_transfunc_array[i].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[i].setColorRange( color_variable_min, color_variable_max );
        }

        if( particle_property.m_transfunc_array[i].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[i].setOpacityRange( opacity_variable_min, opacity_variable_max );
        }
    }

    for ( std::size_t i = 0; i < kvs_cell_type_vector.size(); i++ )
    {
        std::unique_ptr<std::unique_ptr<Type[]>[]> values;
        nvariables = 0;
        std::unique_ptr<float[]> coordinates;
        int ncoords = 0;
        std::unique_ptr<unsigned int[]> connections;
        int ncells = 0;
        vismodule::VolumeObjectBase::CellType celltype;

        convert_cell_type(
            kvs_cell_type_vector[i], values, nvariables, coordinates,
            ncoords, connections, ncells, celltype
        );

        process_cell_type( values, nvariables, coordinates, ncoords, connections, ncells, celltype );
    }

    OutputCoordMinMaxFile( dom, coordMinMaxFilePath );

    if ( object_generation_enabled )
    {
        OutputParticleFiles(
            particle_property, mvpl, time_step, particleFilePrefix,
            particle_coords, particle_colors, particle_normals
        );
    }

    OutputParticleHistory(
        particle_property, tf_number, nvariables, historyFilePath,
        object_generation_enabled, tmp_c_bins, tmp_o_bins, tmp_max, tmp_min
    );

    if ( object_generation_enabled && glyph_property.m_glyph_flag )
    {
        OutputGlyphs(
            time_step, glyphFilePrefix, glyph_coords,
            glyph_vectors, glyph_sizes, glyph_colors
        );
    }

    if ( object_generation_enabled && pol_property.m_plot_flag )
    {
        OutputLine( time_step, plotOverLineFilePrefix, values_on_line, mask, x_axis );
    }

    // if ( pot_property.m_plot_flag )
    if ( object_generation_enabled ) // 常にファイルを出力し続ける、将来的に変更する可能性あり
    {
        OutputPOT( time_step, plotOverTimeFilePrefix, pot_mask, value_on_time );
    }

    // OutputParticlesで更新したdefault_old.jsonをtimestep別JSONに保存する
    if ( object_generation_enabled && mpi_rank == 0 )
    {
        std::ifstream src( tfJsonPath_old.c_str(), std::ios::binary );
        std::ofstream dst( tfJsonPath_step.c_str(), std::ios::binary );
        dst << src.rdbuf();
    }

    // 粒子ファイル書き込みスレッドが終了するまで待機
    // async_io_enabled, pwtはkvs_wrapper_common.cppに宣言
    if ( async_io_enabled )
    {
        std::cout << "Particle write thread is active."             << std::endl;
        std::cout << "Waiting for particle write thread to finish." << std::endl;
        pbvr::ParticleWriteThread* particle_write_thread = &pwt;
        particle_write_thread->join( true );
        std::cout << "Particle write thread is finished." << std::endl;
    }
    else
    {
        std::cout << "Particle write thread is not active." << std::endl;
    }

    std::cout << "Waiting for all processes to finish." << std::endl;
    
#ifndef CPU_VER
    MPI_Barrier( MPI_COMM_WORLD ); // すべてのプロセスでファイルの書き込む処理が終了するまで待機
#endif
    
    std::cout << "All processes have finished." << std::endl;

    if ( mpi_rank == 0 )
    {
        std::ofstream ofs( stateFilePath.c_str(), std::ios::out );
        if( !ofs.is_open() ) std::cout << "Cannot open state.txt" << std::endl;

        ofs << "START_STEP  = " << start_time_step << std::endl;
        ofs << "LATEST_STEP = " << time_step       << std::endl;

        ofs.close();
    }

    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
    delete[] tmp_max;
    delete[] tmp_min;
    delete particle_property.m_transfunc_synthesizer;
    delete particle_property.m_camera;

}

void SetDomain( vtkUnstructuredGrid* ucd, domain_parameters_unstruct* dom )
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
#endif // EXTEND_FILE_FORMAT
