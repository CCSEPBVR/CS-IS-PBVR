#include "Converter/ConverterTask.h"

#include <algorithm>
#include <cctype>
#include <memory>

#include "kvs/PolygonExporter"
#include "kvs/PolygonObject"

#include "Converter/ConverterTaskInput.h"
#include "Converter/ConverterTaskOutput.h"
#include "Exporter/StructuredVolumeObjectExporter.h"
#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "FileFormat/AVS/AvsUcd.h"
#include "FileFormat/CGNS/Cgns.h"
#include "FileFormat/KVSML/KvsmlUnstructuredVolumeObject.h"
#include "FileFormat/NetCDF/Netcdf.h"
#include "FileFormat/PLOT3D/Plot3d.h"
#include "FileFormat/STL/Stl.h"
#include "FileFormat/VTK/VtkStructuredGrid.h"
#include "FileFormat/VTK/VtkUnstructuredGrid.h"
#include "FileFormat/VTK/VtkXmlImageData.h"
#include "FileFormat/VTK/VtkXmlMultiBlock.h"
#include "FileFormat/VTK/VtkXmlPStructuredGrid.h"
#include "FileFormat/VTK/VtkXmlPUnstructuredGrid.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "Filesystem.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/Pfl.h"
#include "PBVRFileInformation/UnstructuredPfi.h"
#include "TimeSeriesFiles/EnSight/EnSightGold.h"
#include "TimeSeriesFiles/EnSight/EnSightGoldBinary.h"
#include "TimeSeriesFiles/SeriesFileResolver.h"

namespace
{
template <typename FileFormat>
std::optional<cvt::ConverterTaskOutput> ConvertUnstructuredGridToKvsml(
    const std::string& directory, const std::string& base, const std::vector<std::string>& src,
    int target_index, int time_step, int last_time_step, int has_mesh_deformation )
{
    std::unordered_map<int, int> veclens;
    std::unordered_map<int, int> sub_volume_counts;
    int ghost_cell_count = 0;
    // one pass
    for ( auto& path : src )
    {
        auto vtu = FileFormat( path );

        for ( auto file_format : vtu.eachCellType() )
        {
            cvt::VtkImporter<FileFormat> importer( &file_format );
            kvs::UnstructuredVolumeObject* object = &importer;

            auto cell_type = object->cellType();
            sub_volume_counts[cell_type] = ( sub_volume_counts.count( cell_type ) == 0 )
                                               ? 1
                                               : ( sub_volume_counts[cell_type] + 1 );
            veclens[cell_type] = object->veclen();
            ghost_cell_count += file_format.getGhostCellCount();
        }
    }

    // Check if the sub volume has same data arrays.
    if ( !std::all_of( veclens.begin(), veclens.end(),
                       [&]( auto& v ) { return veclens.begin()->second == v.second; } ) )
    {
        return std::nullopt;
    }

    std::unordered_map<std::string, int> sub_volume_counts0;
    for ( auto& e : sub_volume_counts )
    {
        auto local_base = std::string( base ) + "_" + std::to_string( e.first );
        sub_volume_counts0[local_base] = e.second;
    }

    // two pass
    cvt::ConverterTaskOutput output( target_index, time_step, last_time_step, has_mesh_deformation,
                                     sub_volume_counts0, ghost_cell_count );
    std::unordered_map<int, int> sub_volume_ids;
    for ( auto& e : sub_volume_counts )
    {
        sub_volume_ids[e.first] = 1;
    }
    for ( auto& path : src )
    {
        FileFormat input_vtu( path );

        for ( auto vtu : input_vtu.eachCellType() )
        {
            cvt::VtkImporter<FileFormat> importer( &vtu );
            kvs::UnstructuredVolumeObject* object = &importer;

            auto local_base = std::string( base ) + "_" + std::to_string( object->cellType() );

            cvt::UnstructuredVolumeObjectExporter exporter( &importer );
            exporter.setWritingDataTypeToExternalBinary();
            exporter.write( directory, local_base, time_step, sub_volume_ids[object->cellType()],
                            sub_volume_counts[object->cellType()], has_mesh_deformation == 0 );

            output.registerObject( &exporter, local_base, sub_volume_ids[object->cellType()] );

            ++sub_volume_ids[object->cellType()];
        }
    }

    return output;
}

template <typename FileFormat>
std::optional<cvt::ConverterTaskOutput> ConvertMultiBlockToKvsml(
    const std::string& directory, const std::string& base, const std::vector<std::string>& src,
    int target_index, int time_step, int last_time_step, int has_mesh_deformation,
    std::string grid_type )
{
    std::unordered_map<int, int> sub_volume_counts;
    int ghost_cell_count = 0;
    auto update = [&]( int cell_type ) {
        sub_volume_counts[cell_type] = ( sub_volume_counts.count( cell_type ) == 0 )
                                           ? 1
                                           : ( sub_volume_counts[cell_type] + 1 );
    };

    // one pass
    for ( auto& path : src )
    {
        FileFormat input_vtm( path );

        for ( auto format : input_vtm.eachBlock() )
        {
            if ( grid_type == "structured" )
            {
                if ( auto structured_volume_format =
                         dynamic_cast<cvt::VtkXmlRectilinearGrid*>( format.get() ) )
                {
                    update( 2 );
                    ghost_cell_count += structured_volume_format->getGhostCellCount();
                }
                else if ( auto structured_volume_format =
                              dynamic_cast<cvt::VtkXmlImageData*>( format.get() ) )
                {
                    update( 2 );
                    ghost_cell_count += structured_volume_format->getGhostCellCount();
                }
                else if ( auto structured_volume_format =
                              dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) )
                {
                    update( 2 );
                    ghost_cell_count += structured_volume_format->getGhostCellCount();
                }
            }
            else if ( grid_type == "unstructured" )
            {
                if ( auto unstructured_volume_format =
                         dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
                {
                    for ( auto file_format : unstructured_volume_format->eachCellType() )
                    {
                        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &file_format );
                        kvs::UnstructuredVolumeObject* object = &importer;

                        update( object->cellType() );
                        ghost_cell_count += unstructured_volume_format->getGhostCellCount();
                    }
                }
            }
        }
    }

    std::unordered_map<std::string, int> sub_volume_counts0;
    for ( auto& e : sub_volume_counts )
    {
        auto local_base = std::string( base ) + "_" + std::to_string( e.first );
        sub_volume_counts0[local_base] = e.second;
    }

    // two pass
    cvt::ConverterTaskOutput output( target_index, time_step, last_time_step, has_mesh_deformation,
                                     sub_volume_counts0, ghost_cell_count );
    std::unordered_map<int, int> sub_volume_ids;
    for ( auto& e : sub_volume_counts )
    {
        sub_volume_ids[e.first] = 1;
    }
    for ( auto& path : src )
    {
        FileFormat input_vtm( path );

        for ( auto format : input_vtm.eachBlock() )
        {
            if ( grid_type == "structured" )
            {
                auto write = [&]( auto& importer ) {
                    kvs::StructuredVolumeObject* object = &importer;
                    object->updateMinMaxCoords();
                    object->setMinMaxExternalCoords( object->minObjectCoord(),
                                                     object->maxObjectCoord() );

                    cvt::StructuredVolumeObjectExporter exporter( &importer );
                    exporter.setWritingDataTypeToExternalBinary();
                    exporter.write( directory, base, time_step, sub_volume_ids[2],
                                    sub_volume_counts[2], has_mesh_deformation == 0 );
                    auto local_base = std::string( base ) + "_2";
                    output.registerObject( &exporter, local_base, sub_volume_ids[2] );
                    ++sub_volume_ids[2];
                };

                if ( auto structured_volume_format =
                         dynamic_cast<cvt::VtkXmlRectilinearGrid*>( format.get() ) )
                {
                    cvt::VtkImporter<cvt::VtkXmlRectilinearGrid> importer(
                        structured_volume_format );
                    write( importer );
                }
                else if ( auto structured_volume_format =
                              dynamic_cast<cvt::VtkXmlImageData*>( format.get() ) )
                {
                    cvt::VtkImporter<cvt::VtkXmlImageData> importer( structured_volume_format );
                    write( importer );
                }
                else if ( auto structured_volume_format =
                              dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) )
                {
                    cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer(
                        structured_volume_format );
                    write( importer );
                }
            }
            else if ( grid_type == "unstructured" )
            {
                if ( auto unstructured_volume_format =
                         dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
                {
                    cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer(
                        unstructured_volume_format );
                    kvs::UnstructuredVolumeObject* object = &importer;

                    auto local_base =
                        std::string( base ) + "_" + std::to_string( object->cellType() );

                    cvt::UnstructuredVolumeObjectExporter exporter( &importer );
                    exporter.setWritingDataTypeToExternalBinary();
                    exporter.write(
                        directory, local_base, time_step, sub_volume_ids[object->cellType()],
                        sub_volume_counts[object->cellType()], has_mesh_deformation == 0 );

                    output.registerObject( &exporter, local_base,
                                           sub_volume_ids[object->cellType()] );

                    ++sub_volume_ids[object->cellType()];
                }
            }
        }
    }

    return output;
}

std::optional<cvt::ConverterTaskOutput> Plot3d2Kvsml(
    const std::string& directory, const std::string& base, const std::vector<std::string>& src,
    const std::vector<std::string>& q, const std::vector<std::string>& f, int target_index,
    int time_step, int last_time_step, int has_mesh_deformation, std::string grid_type )
{
    std::unordered_map<int, int> sub_volume_counts;
    int ghost_cell_count = 0;

    auto update = [&]( int cell_type ) {
        sub_volume_counts[cell_type] = ( sub_volume_counts.count( cell_type ) == 0 )
                                           ? 1
                                           : ( sub_volume_counts[cell_type] + 1 );
    };

    // one pass
    int src_i = -1;
    for ( auto& path : src )
    {
        ++src_i;
        cvt::Plot3d input_vtm( path, [&]( vtkMultiBlockPLOT3DReader* reader ) {
            reader->AutoDetectFormatOn();
            if ( q[src_i] != "" )
            {
                reader->SetQFileName( q[src_i].c_str() );
            }
            if ( f[src_i] != "" )
            {
                reader->SetFunctionFileName( f[src_i].c_str() );
            }
        } );

        for ( auto format : input_vtm.eachBlock() )
        {
            if ( grid_type == "structured" )
            {
                if ( auto structured_volume_format =
                         dynamic_cast<cvt::VtkXmlRectilinearGrid*>( format.get() ) )
                {
                    update( 2 );
                    ghost_cell_count += structured_volume_format->getGhostCellCount();
                }
                else if ( auto structured_volume_format =
                              dynamic_cast<cvt::VtkXmlImageData*>( format.get() ) )
                {
                    update( 2 );
                    ghost_cell_count += structured_volume_format->getGhostCellCount();
                }
                else if ( auto structured_volume_format =
                              dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) )
                {
                    update( 2 );
                    ghost_cell_count += structured_volume_format->getGhostCellCount();
                }
            }
            else if ( grid_type == "unstructured" )
            {
                if ( auto unstructured_volume_format =
                         dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
                {
                    for ( auto file_format : unstructured_volume_format->eachCellType() )
                    {
                        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &file_format );
                        kvs::UnstructuredVolumeObject* object = &importer;

                        update( object->cellType() );
                        ghost_cell_count += unstructured_volume_format->getGhostCellCount();
                    }
                }
            }
        }
    }

    std::unordered_map<std::string, int> sub_volume_counts0;
    for ( auto& e : sub_volume_counts )
    {
        auto local_base = std::string( base ) + "_" + std::to_string( e.first );
        sub_volume_counts0[local_base] = e.second;
    }

    // two pass
    cvt::ConverterTaskOutput output( target_index, time_step, last_time_step, has_mesh_deformation,
                                     sub_volume_counts0, ghost_cell_count );
    std::unordered_map<int, int> sub_volume_ids;
    for ( auto& e : sub_volume_counts )
    {
        sub_volume_ids[e.first] = 1;
    }
    src_i = -1;
    for ( auto& path : src )
    {
        ++src_i;
        cvt::Plot3d input_vtm( path, [&]( vtkMultiBlockPLOT3DReader* reader ) {
            reader->AutoDetectFormatOn();
            if ( q[src_i] != "" )
            {
                reader->SetQFileName( q[src_i].c_str() );
            }
            if ( f[src_i] != "" )
            {
                reader->SetFunctionFileName( f[src_i].c_str() );
            }
        } );

        for ( auto format : input_vtm.eachBlock() )
        {
            if ( grid_type == "structured" )
            {
                auto write = [&]( auto& importer ) {
                    kvs::StructuredVolumeObject* object = &importer;
                    object->updateMinMaxCoords();
                    object->setMinMaxExternalCoords( object->minObjectCoord(),
                                                     object->maxObjectCoord() );

                    cvt::StructuredVolumeObjectExporter exporter( &importer );
                    exporter.setWritingDataTypeToExternalBinary();
                    exporter.write( directory, base, time_step, sub_volume_ids[2],
                                    sub_volume_counts[2], has_mesh_deformation == 0 );
                    auto local_base = std::string( base ) + "_2";
                    output.registerObject( &exporter, local_base, sub_volume_ids[2] );
                    ++sub_volume_ids[2];
                };

                if ( auto structured_volume_format =
                         dynamic_cast<cvt::VtkXmlRectilinearGrid*>( format.get() ) )
                {
                    cvt::VtkImporter<cvt::VtkXmlRectilinearGrid> importer(
                        structured_volume_format );
                    write( importer );
                }
                else if ( auto structured_volume_format =
                              dynamic_cast<cvt::VtkXmlImageData*>( format.get() ) )
                {
                    cvt::VtkImporter<cvt::VtkXmlImageData> importer( structured_volume_format );
                    write( importer );
                }
                else if ( auto structured_volume_format =
                              dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) )
                {
                    cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer(
                        structured_volume_format );
                    write( importer );
                }
            }
            else if ( grid_type == "unstructured" )
            {
                if ( auto unstructured_volume_format =
                         dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
                {
                    cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer(
                        unstructured_volume_format );
                    kvs::UnstructuredVolumeObject* object = &importer;

                    auto local_base =
                        std::string( base ) + "_" + std::to_string( object->cellType() );

                    cvt::UnstructuredVolumeObjectExporter exporter( &importer );
                    exporter.setWritingDataTypeToExternalBinary();
                    exporter.write(
                        directory, local_base, time_step, sub_volume_ids[object->cellType()],
                        sub_volume_counts[object->cellType()], has_mesh_deformation == 0 );

                    output.registerObject( &exporter, local_base,
                                           sub_volume_ids[object->cellType()] );

                    ++sub_volume_ids[object->cellType()];
                }
            }
        }
    }

    return output;
}

std::optional<cvt::ConverterTaskOutput> PVtu2Kvsml( const std::string& directory,
                                                    const std::string& base,
                                                    const std::vector<std::string>& src,
                                                    int target_index, int time_step,
                                                    int last_time_step, int has_mesh_deformation )
{
    std::unordered_map<int, int> veclens;
    std::unordered_map<int, int> sub_volume_counts;
    int ghost_cell_count = 0;

    // one pass
    for ( auto& path : src )
    {
        auto pvtu = cvt::VtkXmlPUnstructuredGrid( path );

        for ( auto vtu : pvtu.eachPiece() )
        {
            for ( auto file_format : vtu.eachCellType() )
            {
                cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &file_format );
                kvs::UnstructuredVolumeObject* object = &importer;

                auto cell_type = object->cellType();
                sub_volume_counts[cell_type] = ( sub_volume_counts.count( cell_type ) == 0 )
                                                   ? 1
                                                   : ( sub_volume_counts[cell_type] + 1 );
                veclens[cell_type] = object->veclen();
                ghost_cell_count += file_format.getGhostCellCount();
            }
        }
    }
    // Check if the sub volume has same data arrays.
    if ( !std::all_of( veclens.begin(), veclens.end(),
                       [&]( auto& v ) { return veclens.begin()->second == v.second; } ) )
    {
        return std::nullopt;
    }

    std::unordered_map<std::string, int> sub_volume_counts0;
    for ( auto& e : sub_volume_counts )
    {
        auto local_base = std::string( base ) + "_" + std::to_string( e.first );
        sub_volume_counts0[local_base] = e.second;
    }

    // two pass
    cvt::ConverterTaskOutput output( target_index, time_step, last_time_step, has_mesh_deformation,
                                     sub_volume_counts0, ghost_cell_count );
    std::unordered_map<int, int> sub_volume_ids;
    for ( auto& e : sub_volume_counts )
    {
        sub_volume_ids[e.first] = 1;
    }
    for ( auto& path : src )
    {
        cvt::VtkXmlPUnstructuredGrid input_pvtu( path );

        for ( auto vtu : input_pvtu.eachPiece() )
        {
            for ( auto file_format : vtu.eachCellType() )
            {
                cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &file_format );

                kvs::UnstructuredVolumeObject* object = &importer;

                std::string local_base =
                    std::string( base ) + "_" + std::to_string( object->cellType() );
                cvt::UnstructuredVolumeObjectExporter exporter( &importer );
                exporter.setWritingDataTypeToExternalBinary();
                exporter.write( directory, local_base, time_step,
                                sub_volume_ids[object->cellType()],
                                sub_volume_counts[object->cellType()], has_mesh_deformation == 0 );

                output.registerObject( &exporter, local_base, sub_volume_ids[object->cellType()] );

                ++sub_volume_ids[object->cellType()];
            }
        }
    }

    return output;
}

template <typename FileFormat>
std::optional<cvt::ConverterTaskOutput> ConvertStructuredGridToKvsml(
    const std::string& directory, const std::string& base, const std::vector<std::string>& src,
    int target_index, int time_step, int last_time_step, int has_mesh_deformation )
{
    int sub_volume_id = 1;
    int sub_volume_count = src.size();
    std::unordered_map<std::string, int> sub_volume_counts;
    sub_volume_counts[base] = sub_volume_count;

    cvt::ConverterTaskOutput output( target_index, time_step, last_time_step, has_mesh_deformation,
                                     sub_volume_counts, 0 );
    for ( auto& path : src )
    {
        FileFormat file_format( path );
        cvt::VtkImporter<FileFormat> importer( &file_format );

        kvs::StructuredVolumeObject* object = &importer;
        object->updateMinMaxCoords();
        object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );

        cvt::StructuredVolumeObjectExporter exporter( &importer );
        exporter.setWritingDataTypeToExternalBinary();
        exporter.write( directory, base, time_step, sub_volume_id, sub_volume_count,
                        has_mesh_deformation == 0 );
        output.registerObject( &exporter, base, sub_volume_id++ );
        output.ghost_cell_count += file_format.getGhostCellCount();
    }

    return output;
}

std::optional<cvt::ConverterTaskOutput> Pvts2Kvsml( const std::string& directory,
                                                    const std::string& base,
                                                    const std::vector<std::string>& src,
                                                    int target_index, int time_step,
                                                    int last_time_step, int has_mesh_deformation )
{
    int sub_volume_count = 0;
    // one pass
    for ( auto& path : src )
    {
        cvt::VtkXmlPStructuredGrid input_pvts( path );
        sub_volume_count += input_pvts.numberOfPieces();
    }

    // two pass
    std::unordered_map<std::string, int> sub_volume_counts;
    sub_volume_counts[base] = sub_volume_count;

    cvt::ConverterTaskOutput output( target_index, time_step, last_time_step, has_mesh_deformation,
                                     sub_volume_counts, 0 );
    for ( auto& path : src )
    {
        cvt::VtkXmlPStructuredGrid input_pvts( path );

        int sub_volume_id = 1;

        for ( auto file_format : input_pvts.eachPiece() )
        {
            cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &file_format );

            kvs::StructuredVolumeObject* object = &importer;
            object->updateMinMaxCoords();
            object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );

            cvt::StructuredVolumeObjectExporter exporter( &importer );
            exporter.setWritingDataTypeToExternalBinary();
            exporter.write( directory, base, time_step, sub_volume_id, sub_volume_count,
                            has_mesh_deformation == 0 );

            output.registerObject( &exporter, base, sub_volume_id++ );
            output.ghost_cell_count += file_format.getGhostCellCount();
        }
    }

    return output;
}

std::optional<cvt::ConverterTaskOutput> OutputConvertProfile(
    std::optional<cvt::ConverterTaskOutput>&& output, const std::string& destination_directory,
    const std::string& destination_prefix, int output_profile )
{
    if ( output && output_profile > 0 )
    {
        cvt::filesystem::path dst( destination_directory );
        dst /= destination_prefix + "_" + std::to_string( output->time_step ) + ".xml";
        std::ofstream ostream( dst );

        ostream << output->serialize();
    }

    return output;
}

template <typename EnSightFileFormat>
std::optional<cvt::ConverterTaskOutput> Case2Kvsml( const std::string& directory,
                                                    const std::string& base, const std::string& src,
                                                    int target_index, int output_profile )
{
    EnSightFileFormat input_case( src );

    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;

    auto time_steps_container = input_case.eachTimeStep();
    int last_time_step = time_steps_container.lastTimeStep();
    int time_step = 0;
    std::unordered_map<int, int> sub_volume_counts;
    int ghost_cell_count = 0;

    for ( auto time_and_format : time_steps_container )
    {
        auto& multi_block_format = time_and_format.second;

        if ( time_step == 0 )
        {
            // one pass
            for ( auto format : multi_block_format.eachBlock() )
            {
                if ( format )
                {
                    if ( auto unstructured_volume_format =
                             dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
                    {
                        for ( auto file_format : unstructured_volume_format->eachCellType() )
                        {
                            cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &file_format );
                            kvs::UnstructuredVolumeObject* object = &importer;
                            if ( object->cellType() ==
                                 kvs::UnstructuredVolumeObject::CellType::UnknownCellType )
                            {
                                continue;
                            }

                            auto cell_type = object->cellType();
                            sub_volume_counts[cell_type] =
                                ( sub_volume_counts.count( cell_type ) == 0 )
                                    ? 1
                                    : ( sub_volume_counts[cell_type] + 1 );
                            ghost_cell_count = file_format.getGhostCellCount();
                        }
                    }
                    else if ( auto structured_volume_format =
                                  dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) )
                    {
                        int cell_type = 2;
                        sub_volume_counts[cell_type] = ( sub_volume_counts.count( cell_type ) == 0 )
                                                           ? 1
                                                           : ( sub_volume_counts[cell_type] + 1 );
                    }
                }
            }
        }

        // two pass
        std::unordered_map<std::string, int> sub_volume_counts0;
        for ( auto& e : sub_volume_counts )
        {
            auto local_base = std::string( base ) + "_" + std::to_string( e.first );
            sub_volume_counts0[local_base] = e.second;
        }
        cvt::ConverterTaskOutput output( target_index, time_step, last_time_step, 1,
                                         sub_volume_counts0, ghost_cell_count );
        std::unordered_map<int, int> sub_volume_ids;
        for ( auto& e : sub_volume_counts )
        {
            sub_volume_ids[e.first] = 1;
        }
        for ( auto format : multi_block_format.eachBlock() )
        {
            if ( !format )
            {
                std::cout << "      Unsupported VTK data type" << std::endl;
            }
            else if ( auto unstructured_volume_format =
                          dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
            {
                for ( auto vtu : unstructured_volume_format->eachCellType() )
                {
                    cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &vtu );

                    kvs::UnstructuredVolumeObject* object = &importer;

                    if ( object->cellType() ==
                         kvs::UnstructuredVolumeObject::CellType::UnknownCellType )
                    {
                        continue;
                    }

                    auto local_base =
                        std::string( base ) + "_" + std::to_string( object->cellType() );

                    cvt::UnstructuredVolumeObjectExporter exporter( &importer );
                    exporter.setWritingDataTypeToExternalBinary();
                    exporter.write( directory, local_base, time_step,
                                    sub_volume_ids[object->cellType()],
                                    sub_volume_counts[object->cellType()] );
                    output.registerObject( &exporter, local_base,
                                           sub_volume_ids[object->cellType()] );
                    if ( time_step == 0 )
                    {
                        pfi_map.emplace(
                            static_cast<int>( object->cellType() ),
                            cvt::UnstructuredPfi( object->veclen(), last_time_step,
                                                  sub_volume_counts[object->cellType()] ) );
                    }
                    pfi_map.at( static_cast<int>( object->cellType() ) )
                        .registerObject( &exporter, time_step, sub_volume_ids[object->cellType()] );
                    ++sub_volume_ids[object->cellType()];
                }
            }
            else if ( auto structured_volume_format =
                          dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) )
            {
                cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( structured_volume_format );

                kvs::StructuredVolumeObject* object = &importer;
                object->updateMinMaxCoords();
                object->setMinMaxExternalCoords( object->minObjectCoord(),
                                                 object->maxObjectCoord() );

                auto local_base = std::string( base ) + "_2";
                cvt::StructuredVolumeObjectExporter exporter( &importer );
                exporter.setWritingDataTypeToExternalBinary();
                exporter.write( directory, local_base, time_step, sub_volume_ids[2],
                                sub_volume_counts[2] );
                if ( time_step == 0 )
                {
                    pfi_map.emplace( 2, cvt::UnstructuredPfi( object->veclen(), last_time_step,
                                                              sub_volume_counts[2] ) );
                }
                pfi_map.at( static_cast<int>( 2 ) )
                    .registerObject( &exporter, time_step, sub_volume_ids[2] );

                ++sub_volume_ids[2];
            }
            else
            {
                std::cout << "unsupported" << std::endl;
            }

            ::OutputConvertProfile( output, directory, base, output_profile );
        }

        ++time_step;
    }

    cvt::Pfl pfl;
    for ( auto& e : pfi_map )
    {
        std::string local_base = std::string( base ) + "_" + std::to_string( e.first );
        e.second.write( directory, local_base );
        pfl.registerPfi( directory, local_base );
    }
    pfl.write( directory, base );

    // Return empty
    return cvt::ConverterTaskOutput( target_index, time_step );
}

std::optional<cvt::ConverterTaskOutput> Stl2Kvsml( const std::string& directory,
                                                   const std::string& base,
                                                   const std::vector<std::string>& src,
                                                   int target_index, int time_step )
{
    int sub = 1;
    for ( auto& path : src )
    {
        cvt::Stl input_stl( path, []( vtkSTLReader* stl_reader ) { stl_reader->MergingOn(); } );
        cvt::VtkImporter<cvt::Stl> importer( &input_stl );

        kvs::PolygonExporter<kvs::KVSMLPolygonObject> exporter( &importer );
        exporter.setWritingDataTypeToExternalBinary();

        cvt::filesystem::path dst = directory + base + std::to_string( sub );
        exporter.write( dst.u8string().c_str() );

        ++sub;
    }

    // Return empty
    return cvt::ConverterTaskOutput( target_index, time_step );
}

std::optional<cvt::ConverterTaskOutput> Netcdf2Kvsml(
    const std::string& directory, const std::string& base, const std::vector<std::string>& src,
    int target_index, int time_step, int last_time_step, int has_mesh_deformation )
{
    if ( src.empty() )
    {
        return std::nullopt;
    }

    auto import = []( cvt::Netcdf& input ) -> std::unique_ptr<kvs::VolumeObjectBase> {
        if ( auto* format = dynamic_cast<cvt::VtkXmlImageData*>( input.format().get() ) )
        {
            auto importer =
                std::make_unique<cvt::VtkImporter<cvt::VtkXmlImageData>>( format );
            if ( importer->isFailure() )
            {
                return nullptr;
            }
            return importer;
        }
        if ( auto* format =
                 dynamic_cast<cvt::VtkXmlRectilinearGrid*>( input.format().get() ) )
        {
            auto importer =
                std::make_unique<cvt::VtkImporter<cvt::VtkXmlRectilinearGrid>>( format );
            if ( importer->isFailure() )
            {
                return nullptr;
            }
            return importer;
        }
        if ( auto* format =
                 dynamic_cast<cvt::VtkXmlStructuredGrid*>( input.format().get() ) )
        {
            auto importer =
                std::make_unique<cvt::VtkImporter<cvt::VtkXmlStructuredGrid>>( format );
            if ( importer->isFailure() )
            {
                return nullptr;
            }
            return importer;
        }
        if ( auto* format =
                 dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( input.format().get() ) )
        {
            auto importer =
                std::make_unique<cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid>>( format );
            if ( importer->isFailure() )
            {
                return nullptr;
            }
            return importer;
        }
        return nullptr;
    };

    cvt::Netcdf first_input( src.front() );
    if ( first_input.isFailure() )
    {
        return std::nullopt;
    }
    auto first_volume = import( first_input );
    if ( !first_volume )
    {
        return std::nullopt;
    }

    const auto expected_volume_type = first_volume->volumeType();
    const int expected_veclen = static_cast<int>( first_volume->veclen() );
    int expected_cell_type = -1;
    int expected_grid_type = -1;
    if ( auto* unstructured =
             dynamic_cast<kvs::UnstructuredVolumeObject*>( first_volume.get() ) )
    {
        expected_cell_type = static_cast<int>( unstructured->cellType() );
    }
    else if ( auto* structured =
                  dynamic_cast<kvs::StructuredVolumeObject*>( first_volume.get() ) )
    {
        expected_grid_type = static_cast<int>( structured->gridType() );
    }
    else
    {
        return std::nullopt;
    }

    const int sub_volume_count = static_cast<int>( src.size() );
    const std::string local_base =
        expected_volume_type == kvs::VolumeObjectBase::Unstructured
            ? base + "_" + std::to_string( expected_cell_type )
            : base;
    std::unordered_map<std::string, int> sub_volume_counts = {
        { local_base, sub_volume_count }
    };
    cvt::ConverterTaskOutput output( target_index, time_step, last_time_step,
                                     has_mesh_deformation, sub_volume_counts, 0 );

    auto write = [&]( kvs::VolumeObjectBase* volume, const std::string& path,
                      int sub_volume_id ) {
        if ( volume->volumeType() != expected_volume_type ||
             static_cast<int>( volume->veclen() ) != expected_veclen )
        {
            kvsMessageError(
                ( std::string( "NetCDF volume structure differs for " ) + path ).c_str() );
            return false;
        }

        if ( auto* unstructured =
                 dynamic_cast<kvs::UnstructuredVolumeObject*>( volume ) )
        {
            if ( static_cast<int>( unstructured->cellType() ) != expected_cell_type )
            {
                kvsMessageError(
                    ( std::string( "NetCDF cell type differs for " ) + path ).c_str() );
                return false;
            }
            cvt::UnstructuredVolumeObjectExporter exporter( unstructured );
            exporter.setWritingDataTypeToExternalBinary();
            if ( !exporter.write( directory, local_base, time_step, sub_volume_id,
                                  sub_volume_count, has_mesh_deformation == 0 ) )
            {
                return false;
            }
            output.registerObject( &exporter, local_base, sub_volume_id );
            return true;
        }

        if ( auto* structured = dynamic_cast<kvs::StructuredVolumeObject*>( volume ) )
        {
            if ( static_cast<int>( structured->gridType() ) != expected_grid_type )
            {
                kvsMessageError(
                    ( std::string( "NetCDF grid type differs for " ) + path ).c_str() );
                return false;
            }
            structured->updateMinMaxCoords();
            structured->setMinMaxExternalCoords( structured->minObjectCoord(),
                                                 structured->maxObjectCoord() );
            cvt::StructuredVolumeObjectExporter exporter( structured );
            exporter.setWritingDataTypeToExternalBinary();
            if ( !exporter.write( directory, local_base, time_step, sub_volume_id,
                                  sub_volume_count, has_mesh_deformation == 0 ) )
            {
                return false;
            }
            output.registerObject( &exporter, local_base, sub_volume_id );
            return true;
        }
        return false;
    };

    int sub_volume_id = 1;
    if ( !write( first_volume.get(), src.front(), sub_volume_id++ ) )
    {
        return std::nullopt;
    }

    for ( std::size_t i = 1; i < src.size(); ++i )
    {
        const auto& path = src[i];
        cvt::Netcdf input( path );
        if ( input.isFailure() || input.formatName() != first_input.formatName() ||
             input.gridType() != first_input.gridType() )
        {
            return std::nullopt;
        }
        auto volume = import( input );
        if ( !volume || !write( volume.get(), path, sub_volume_id ) )
        {
            kvsMessageError( ( std::string( "Failed to write NetCDF conversion output for " ) +
                               path )
                                 .c_str() );
            return std::nullopt;
        }
        ++sub_volume_id;
    }

    return output;
}
} // namespace

std::optional<cvt::ConverterTaskOutput> cvt::Convert( cvt::ConverterTaskInput input )
{
    try
    {
        cvt::filesystem::path path( input.source_file_paths[0] );

        // 拡張子の大文字・小文字にかかわらず、後続の形式判定を行えるようにする。
        std::string extension = path.extension();
        std::transform( extension.begin(), extension.end(), extension.begin(),
                        []( const unsigned char c ) {
                            return static_cast<char>( std::tolower( c ) );
                        } );

        // 従来から拡張子によって変換処理を選択できる形式は、内容の検査を行わずに
        // そのまま後続の変換処理へ渡す。
        const std::vector<std::string> known_extensions = {
            ".vtu", ".pvtu", ".inp", ".nc",   ".vti", ".vtr", ".vts", ".pvts",
            ".vtk", ".case", ".cgns", ".vtm", ".xyz", ".stl"
        };

        // 拡張子がない、または独自の拡張子を持つ時系列ファイルの場合は、先頭の
        // 入力ファイルの内容から形式を検出し、既存の変換分岐で扱える拡張子へ正規化する。
        if ( std::find( known_extensions.begin(), known_extensions.end(), extension ) ==
             known_extensions.end() )
        {
            cvt::SeriesFormat detected_format;
            std::string error;
            if ( !cvt::DetectSeriesFileFormat( path.string(), detected_format, error ) )
            {
                std::cerr << error << std::endl;
                return std::nullopt;
            }
            extension = cvt::CanonicalSeriesExtension( detected_format );
        }

        // Create a destination directory
        cvt::filesystem::path directory( input.destination_directory );

        if ( !cvt::filesystem::exists( directory ) )
        {
            try
            {
                cvt::filesystem::create_directories( directory );
            }
            catch ( ... )
            {
            }
        }
        if ( !cvt::filesystem::exists( directory ) )
        {
            return std::nullopt;
        }

        if ( extension == ".vtu" )
        {
            return ::OutputConvertProfile(
                ::ConvertUnstructuredGridToKvsml<cvt::VtkXmlUnstructuredGrid>(
                    input.destination_directory, input.destination_prefix, input.source_file_paths,
                    input.target_index, input.time_step, input.last_time_step,
                    input.has_mesh_deformation ),
                input.destination_directory, input.destination_prefix, input.output_profile );
        }
        else if ( extension == ".pvtu" )
        {
            return ::OutputConvertProfile(
                ::PVtu2Kvsml( input.destination_directory, input.destination_prefix,
                              input.source_file_paths, input.target_index, input.time_step,
                              input.last_time_step, input.has_mesh_deformation ),
                input.destination_directory, input.destination_prefix, input.output_profile );
        }
        else if ( extension == ".inp" )
        {
            return ::OutputConvertProfile(
                ::ConvertUnstructuredGridToKvsml<cvt::AvsUcd>(
                    input.destination_directory, input.destination_prefix, input.source_file_paths,
                    input.target_index, input.time_step, input.last_time_step,
                    input.has_mesh_deformation ),
                input.destination_directory, input.destination_prefix, input.output_profile );
        }
        else if ( extension == ".nc" )
        {
            return ::OutputConvertProfile(
                ::Netcdf2Kvsml( input.destination_directory, input.destination_prefix,
                                input.source_file_paths, input.target_index, input.time_step,
                                input.last_time_step, input.has_mesh_deformation ),
                input.destination_directory, input.destination_prefix, input.output_profile );
        }
        else if ( extension == ".vti" )
        {
            return ::OutputConvertProfile(
                ::ConvertStructuredGridToKvsml<cvt::VtkXmlImageData>(
                    input.destination_directory, input.destination_prefix, input.source_file_paths,
                    input.target_index, input.time_step, input.last_time_step,
                    input.has_mesh_deformation ),
                input.destination_directory, input.destination_prefix, input.output_profile );
        }
        else if ( extension == ".vtr" )
        {
            return ::OutputConvertProfile(
                ::ConvertStructuredGridToKvsml<cvt::VtkXmlRectilinearGrid>(
                    input.destination_directory, input.destination_prefix, input.source_file_paths,
                    input.target_index, input.time_step, input.last_time_step,
                    input.has_mesh_deformation ),
                input.destination_directory, input.destination_prefix, input.output_profile );
        }
        else if ( extension == ".vts" )
        {
            return ::OutputConvertProfile(
                ::ConvertStructuredGridToKvsml<cvt::VtkXmlStructuredGrid>(
                    input.destination_directory, input.destination_prefix, input.source_file_paths,
                    input.target_index, input.time_step, input.last_time_step,
                    input.has_mesh_deformation ),
                input.destination_directory, input.destination_prefix, input.output_profile );
        }
        else if ( extension == ".pvts" )
        {
            return ::OutputConvertProfile(
                ::Pvts2Kvsml( input.destination_directory, input.destination_prefix,
                              input.source_file_paths, input.target_index, input.time_step,
                              input.last_time_step, input.has_mesh_deformation ),
                input.destination_directory, input.destination_prefix, input.output_profile );
        }
        else if ( extension == ".vtk" )
        {
            if ( input.source_grid_type == "structured" )
            {
                return ::OutputConvertProfile(
                    ::ConvertStructuredGridToKvsml<cvt::VtkStructuredGrid>(
                        input.destination_directory, input.destination_prefix,
                        input.source_file_paths, input.target_index, input.time_step,
                        input.last_time_step, input.has_mesh_deformation ),
                    input.destination_directory, input.destination_prefix, input.output_profile );
            }
            else if ( input.source_grid_type == "unstructured" )
            {
                return ::OutputConvertProfile(
                    ::ConvertUnstructuredGridToKvsml<cvt::VtkUnstructuredGrid>(
                        input.destination_directory, input.destination_prefix,
                        input.source_file_paths, input.target_index, input.time_step,
                        input.last_time_step, input.has_mesh_deformation ),
                    input.destination_directory, input.destination_prefix, input.output_profile );
            }
            else
            {
                return std::nullopt;
            }
        }
        else if ( extension == ".case" )
        {
            if ( input.is_binary > 0 )
            {
                return ::Case2Kvsml<cvt::EnSightGoldBinary>(
                    input.destination_directory, input.destination_prefix,
                    input.source_file_paths[0], input.target_index, input.output_profile );
            }
            else
            {
                return ::Case2Kvsml<cvt::EnSightGold<>>(
                    input.destination_directory, input.destination_prefix,
                    input.source_file_paths[0], input.target_index, input.output_profile );
            }
        }
        else if ( extension == ".cgns" )
        {
            return ::OutputConvertProfile(
                ::ConvertMultiBlockToKvsml<cvt::Cgns>(
                    input.destination_directory, input.destination_prefix, input.source_file_paths,
                    input.target_index, input.time_step, input.last_time_step,
                    input.has_mesh_deformation, input.source_grid_type ),
                input.destination_directory, input.destination_prefix, input.output_profile );
            return std::nullopt;
        }
        else if ( extension == ".vtm" )
        {
            return ::OutputConvertProfile(
                ::ConvertMultiBlockToKvsml<cvt::VtkXmlMultiBlock>(
                    input.destination_directory, input.destination_prefix, input.source_file_paths,
                    input.target_index, input.time_step, input.last_time_step,
                    input.has_mesh_deformation, input.source_grid_type ),
                input.destination_directory, input.destination_prefix, input.output_profile );
        }
        else if ( extension == ".xyz" )
        {
            return ::OutputConvertProfile(
                ::Plot3d2Kvsml( input.destination_directory, input.destination_prefix,
                                input.source_file_paths, input.q, input.f, input.target_index,
                                input.time_step, input.last_time_step, input.has_mesh_deformation,
                                input.source_grid_type ),
                input.destination_directory, input.destination_prefix, input.output_profile );
        }
        else if ( extension == ".stl" )
        {
            return Stl2Kvsml( input.destination_directory, input.destination_prefix,
                              input.source_file_paths, input.target_index, input.time_step );
        }
        else
        {
            return std::nullopt;
        }
    }
    catch ( std::exception& e )
    {
        std::cerr << e.what() << std::endl;
        return std::nullopt;
    }
    catch ( ... )
    {
        return std::nullopt;
    }
}
