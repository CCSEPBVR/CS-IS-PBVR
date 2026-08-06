#include "kvs_wrapper_common.h"
#include "kvs_wrapper.h"

#include <cstdio>
#include <cfloat>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <climits>

#include <vismodule/ValueArray>
#include <vismodule/PointObject>
#include <vismodule/KVSMLObjectPoint>
#include <vismodule/PointExporter>
#include <vismodule/RGBColor>
#include <vismodule/MersenneTwister>
#include <cstdint>
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
#ifdef ENABLE_ENSEMBLE_TIMER
#include <vismodule/Timer>
#endif

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

#include "ChainRuleNormal.h"
#include "EnsembleCellHistogram.h"
#include "EnsembleStatisticOutput.h"

using namespace pbvr;

namespace
{

void ReduceEnsembleStatisticRange(
    EnsembleStatisticRange& range,
    const int tf_number,
    MPI_Comm comm = MPI_COMM_WORLD
)
{
#ifndef CPU_VER
    std::vector<float> min_recv( tf_number * 2, FLT_MAX );
    std::vector<float> max_recv( tf_number * 2, -FLT_MAX );
    std::vector<vismodule::UInt64> o_bins_recv( tf_number * DEFAULT_NBINS, 0 );
    std::vector<vismodule::UInt64> c_bins_recv( tf_number * DEFAULT_NBINS, 0 );

    MPI_Reduce( range.min_values.data(), min_recv.data(), tf_number * 2, MPI_FLOAT, MPI_MIN, 0, comm );
    MPI_Reduce( range.max_values.data(), max_recv.data(), tf_number * 2, MPI_FLOAT, MPI_MAX, 0, comm );
    MPI_Reduce( range.o_bins.data(), o_bins_recv.data(), tf_number * DEFAULT_NBINS, MPI_UNSIGNED_LONG, MPI_SUM, 0, comm );
    MPI_Reduce( range.c_bins.data(), c_bins_recv.data(), tf_number * DEFAULT_NBINS, MPI_UNSIGNED_LONG, MPI_SUM, 0, comm );

    range.min_values.swap( min_recv );
    range.max_values.swap( max_recv );
    range.o_bins.swap( o_bins_recv );
    range.c_bins.swap( c_bins_recv );
#endif
}

void EnsureEnsembleStatisticRangeSize(
    EnsembleStatisticRange& range,
    const int tf_number
)
{
    const size_t value_size = static_cast<size_t>( tf_number * 2 );
    const size_t histogram_size = static_cast<size_t>( tf_number * DEFAULT_NBINS );

    if ( range.min_values.size() < value_size ) range.min_values.resize( value_size, 0.0f );
    if ( range.max_values.size() < value_size ) range.max_values.resize( value_size, 0.0f );
    if ( range.o_bins.size() < histogram_size ) range.o_bins.resize( histogram_size, 0 );
    if ( range.c_bins.size() < histogram_size ) range.c_bins.resize( histogram_size, 0 );
}


void StoreStatisticRangeToTransferFunctions(
    const EnsembleStatisticRange& range,
    std::vector<NamedTransferFunction>& transfer_functions,
    const int tf_number
)
{
    if ( static_cast<int>( transfer_functions.size() ) < tf_number ) return;
    if ( range.min_values.size() < static_cast<size_t>( tf_number * 2 ) ||
         range.max_values.size() < static_cast<size_t>( tf_number * 2 ) ||
         range.o_bins.size() < static_cast<size_t>( tf_number * DEFAULT_NBINS ) ||
         range.c_bins.size() < static_cast<size_t>( tf_number * DEFAULT_NBINS ) ) return;

    for ( int i = 0; i < tf_number; i++ )
    {
        transfer_functions[i].m_server_opacity_variable_min = range.min_values[2 * i    ];
        transfer_functions[i].m_server_opacity_variable_max = range.max_values[2 * i    ];
        transfer_functions[i].m_server_color_variable_min   = range.min_values[2 * i + 1];
        transfer_functions[i].m_server_color_variable_max   = range.max_values[2 * i + 1];
        std::copy(
            range.o_bins.begin() + i * DEFAULT_NBINS,
            range.o_bins.begin() + ( i + 1 ) * DEFAULT_NBINS,
            transfer_functions[i].m_opacity_histogram );
        std::copy(
            range.c_bins.begin() + i * DEFAULT_NBINS,
            range.c_bins.begin() + ( i + 1 ) * DEFAULT_NBINS,
            transfer_functions[i].m_color_histogram );
    }
}

void StoreStatisticRangeToTransferFunctions(
    const EnsembleStatisticRange& range,
    std::vector<EnsembleTransferFunction>& transfer_functions,
    const int tf_number
)
{
    if ( static_cast<int>( transfer_functions.size() ) < tf_number ) return;
    if ( range.min_values.size() < static_cast<size_t>( tf_number * 2 ) ||
         range.max_values.size() < static_cast<size_t>( tf_number * 2 ) ||
         range.o_bins.size() < static_cast<size_t>( tf_number * DEFAULT_NBINS ) ||
         range.c_bins.size() < static_cast<size_t>( tf_number * DEFAULT_NBINS ) ) return;

    for ( int i = 0; i < tf_number; i++ )
    {
        transfer_functions[i].m_server_variable_min = range.min_values[2 * i + 1];
        transfer_functions[i].m_server_variable_max = range.max_values[2 * i + 1];
        std::copy(
            range.o_bins.begin() + i * DEFAULT_NBINS,
            range.o_bins.begin() + ( i + 1 ) * DEFAULT_NBINS,
            transfer_functions[i].m_opacity_histogram );
        std::copy(
            range.c_bins.begin() + i * DEFAULT_NBINS,
            range.c_bins.begin() + ( i + 1 ) * DEFAULT_NBINS,
            transfer_functions[i].m_color_histogram );
    }
}



void WritePrefixedStatisticHistory(
    std::ofstream& ofs,
    const std::string& prefix,
    const EnsembleStatisticRange& range,
    const int tf_number
)
{
    for ( int i = 0; i < tf_number; i++ )
    {
        ofs << prefix << "_MIN_O" << ( i + 1 ) << "=" << range.min_values[2 * i    ] << std::endl;
        ofs << prefix << "_MAX_O" << ( i + 1 ) << "=" << range.max_values[2 * i    ] << std::endl;
        ofs << prefix << "_MIN_C" << ( i + 1 ) << "=" << range.min_values[2 * i + 1] << std::endl;
        ofs << prefix << "_MAX_C" << ( i + 1 ) << "=" << range.max_values[2 * i + 1] << std::endl;
//        ofs << prefix << "_RESOLUTION_O" << ( i + 1 ) << "=" << DEFAULT_NBINS << std::endl;
        ofs << prefix << "_HISTOGRAM_O" << ( i + 1 ) << "=";
        for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
        {
            ofs << range.o_bins[j + i * DEFAULT_NBINS] << ",";
        }
        ofs << std::endl;
//        ofs << prefix << "_RESOLUTION_C" << ( i + 1 ) << "=" << DEFAULT_NBINS << std::endl;
        ofs << prefix << "_HISTOGRAM_C" << ( i + 1 ) << "=";
        for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
        {
            ofs << range.c_bins[j + i * DEFAULT_NBINS] << ",";
        }
        ofs << std::endl;
    }
}

void WriteStatisticHistory(
    std::ofstream& ofs,
    const EnsembleStatisticRange& range,
    const int tf_number
)
{
    for ( int i = 0; i < tf_number; i++ )
    {
        ofs << "MIN_O" << ( i + 1 ) << "=" << range.min_values[2 * i    ] << std::endl;
        ofs << "MAX_O" << ( i + 1 ) << "=" << range.max_values[2 * i    ] << std::endl;
        ofs << "MIN_C" << ( i + 1 ) << "=" << range.min_values[2 * i + 1] << std::endl;
        ofs << "MAX_C" << ( i + 1 ) << "=" << range.max_values[2 * i + 1] << std::endl;
        ofs << "RESOLUTION_O" << ( i + 1 ) << "=" << DEFAULT_NBINS << std::endl;
        ofs << "HISTOGRAM_O" << ( i + 1 ) << "=";
        for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
        {
            ofs << range.o_bins[j + i * DEFAULT_NBINS] << ",";
        }
        ofs << std::endl;
        ofs << "RESOLUTION_C" << ( i + 1 ) << "=" << DEFAULT_NBINS << std::endl;
        ofs << "HISTOGRAM_C" << ( i + 1 ) << "=";
        for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
        {
            ofs << range.c_bins[j + i * DEFAULT_NBINS] << ",";
        }
        ofs << std::endl;
    }
}

void WriteEnsembleStatisticJson(
    std::ostringstream& history_json,
    const std::string& statistic_name,
    const EnsembleStatisticRange& range,
    const int tf_number
)
{
    history_json << "        \"" << statistic_name << "\": {\n";
    for ( int i = 0; i < tf_number; i++ )
    {
        history_json << "            \"O" << ( i + 1 ) << "\": {\n";
        history_json << "                \"min\": " << range.min_values[2 * i] << ",\n";
        history_json << "                \"max\": " << range.max_values[2 * i] << ",\n";
        history_json << "                \"resolution\": " << DEFAULT_NBINS << ",\n";
        history_json << "                \"histogram\": [";
        for ( std::size_t j = 0; j < DEFAULT_NBINS; j++ )
        {
            if ( j > 0 ) history_json << ", ";
            history_json << range.o_bins[j + i * DEFAULT_NBINS];
        }
        history_json << "]\n";
        history_json << "            },\n";

        history_json << "            \"C" << ( i + 1 ) << "\": {\n";
        history_json << "                \"min\": " << range.min_values[2 * i + 1] << ",\n";
        history_json << "                \"max\": " << range.max_values[2 * i + 1] << ",\n";
        history_json << "                \"resolution\": " << DEFAULT_NBINS << ",\n";
        history_json << "                \"histogram\": [";
        for ( std::size_t j = 0; j < DEFAULT_NBINS; j++ )
        {
            if ( j > 0 ) history_json << ", ";
            history_json << range.c_bins[j + i * DEFAULT_NBINS];
        }
        history_json << "]\n";
        history_json << "            }";
        if ( i + 1 < tf_number ) history_json << ",";
        history_json << "\n";
    }
    history_json << "        }";
}


} // anonymous namespace

namespace pbvr
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


void OutputEnsembleStatisticHistory(
    ParticleProperty& particle_property,
    const int tf_number,
    const int nvariables,
    const std::string& historyFilePath,
    EnsembleStatisticRange average_range,
    EnsembleStatisticRange variance_range,
    EnsembleStatisticRange co_variation_range,
    MPI_Comm ensemble_comm
)
{
    EnsureEnsembleStatisticRangeSize( average_range, tf_number );
    EnsureEnsembleStatisticRangeSize( variance_range, tf_number );
    EnsureEnsembleStatisticRangeSize( co_variation_range, tf_number );

    ReduceEnsembleStatisticRange( average_range, tf_number, ensemble_comm );
    ReduceEnsembleStatisticRange( variance_range, tf_number, ensemble_comm );
    ReduceEnsembleStatisticRange( co_variation_range, tf_number, ensemble_comm );

    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( ensemble_comm, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    if ( mpi_rank != 0 ) return;

    std::ostringstream history_json;
    history_json << "{\n";
    history_json << "    \"tf_number\": " << tf_number << ",\n";
    history_json << "    \"nvariables\": " << nvariables << ",\n";
    history_json << "    \"particle_limit\": " << particle_property.m_particle_limit << ",\n";
    history_json << "    \"is_ensemble\": true,\n";
    history_json << "    \"statistics\": {\n";
    WriteEnsembleStatisticJson( history_json, "average", average_range, tf_number );
    history_json << ",\n";
    WriteEnsembleStatisticJson( history_json, "variance", variance_range, tf_number );
    history_json << ",\n";
    WriteEnsembleStatisticJson( history_json, "coefficient_of_variation", co_variation_range, tf_number );
    history_json << "\n    }\n";
    history_json << "}\n";

    const std::string tmp_history_file_path = historyFilePath + ".tmp";
    std::ofstream ofs( tmp_history_file_path.c_str(), std::ios::out );
    if ( !ofs )
    {
        throw std::runtime_error( "Cannot open temporary ensemble history json file for writing: " + tmp_history_file_path );
    }

    ofs << history_json.str();
    ofs.close();
    if ( !ofs )
    {
        throw std::runtime_error( "Cannot write temporary ensemble history json file: " + tmp_history_file_path );
    }

    if ( std::rename( tmp_history_file_path.c_str(), historyFilePath.c_str() ) != 0 )
    {
        throw std::runtime_error(
            "Cannot rename temporary ensemble history json file: " + tmp_history_file_path + " -> " + historyFilePath );
    }

    StoreStatisticRangeToTransferFunctions( average_range, particle_property.m_transfunc_array, tf_number );
    StoreStatisticRangeToTransferFunctions( average_range, particle_property.m_mean_transfer_function_array, tf_number );
    StoreStatisticRangeToTransferFunctions( variance_range, particle_property.m_variance_transfer_function_array, tf_number );
    StoreStatisticRangeToTransferFunctions(
        co_variation_range,
        particle_property.m_coefficient_of_variation_transfer_function_array,
        tf_number );

    ParameterFileWriter ppw;
    ppw.writeTF2OldJson( particle_property );
}


} // namespace pbvr
