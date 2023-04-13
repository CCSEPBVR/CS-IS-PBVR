/*
 * Created by Japan Atomic Energy Agency
 *
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legal code along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
#include <iostream>
#include <string>

#include "kvs/UnstructuredVolumeObject"

#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "FileFormat/VTK/VtkXmlPUnstructuredGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/Pfl.h"
#include "PBVRFileInformation/UnstructuredPfi.h"
#include "TimeSeriesFiles/NumeralSequenceFiles.h"

void Pvtu2Kvsml( const std::string& directory, const std::string& base, const std::string& src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlPUnstructuredGrid input_pvtu( src );
    std::cout << "#piece: " << input_pvtu.numberOfPieces() << std::endl;

    int last_time_step = 0;
    int time_step = 0;
    int sub_volume_count = input_pvtu.numberOfPieces();
    int sub_volume_id = 1;

    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;

    // Piece reading
    for ( auto vtu : input_pvtu.eachPiece() )
    {
        for ( auto file_format : vtu.eachCellType() )
        {
            std::cout << "Importing ..." << std::endl;
            cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &file_format );

            kvs::UnstructuredVolumeObject* object = &importer;
            object->print( std::cout, kvs::Indent( 2 ) );

            std::cout << "Writing (" << object->cellType() << " , " << sub_volume_id << ") ..."
                      << std::endl;
            std::string local_base =
                std::string( base ) + "_" + std::to_string( object->cellType() );
            cvt::UnstructuredVolumeObjectExporter exporter( &importer );
            exporter.setWritingDataTypeToExternalBinary();
            exporter.write( directory, local_base, time_step, sub_volume_id, sub_volume_count );
            // or
            // exporter.write( "<directory/<local_base>_00000_0000001_0000001.kvsml" );

            if ( time_step == 0 )
            {
                pfi_map.emplace(
                    static_cast<int>( object->cellType() ),
                    cvt::UnstructuredPfi( object->veclen(), last_time_step, sub_volume_count ) );
            }
            pfi_map.at( static_cast<int>( object->cellType() ) )
                .registerObject( &exporter, time_step, sub_volume_id );
        }

        ++sub_volume_id;
    }

    cvt::Pfl pfl;
    for ( auto& e : pfi_map )
    {
        std::string local_base = std::string( base ) + "_" + std::to_string( e.first );
        e.second.write( directory, local_base );
        e.second.print( std::cout );

        pfl.registerPfi( directory, local_base );
    }
    pfl.write( directory, base );
}

void SeriesPvtu2Kvsml( const std::string& directory, const std::string& base,
                       const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::NumeralSequenceFiles<cvt::VtkXmlPUnstructuredGrid> time_series( src.c_str() );
    int last_time_step = time_series.numberOfFiles() - 1;
    int time_step = 0;
    std::unordered_map<int, int> sub_volume_counts;

    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;

    for ( auto input_pvtu : time_series.eachTimeStep() )
    {
        int sub_volume_id = 1;

        // One-pass to collect information.
        if ( time_step == 0 )
        {
            std::cout << "Reading 0 step objects to collect information" << std::endl;
            std::unordered_map<int, int> veclens;

            for ( auto vtu : input_pvtu.eachPiece() )
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
                }
            }

            for ( auto& e : sub_volume_counts )
            {
                auto cell_type = e.first;

                pfi_map.emplace( static_cast<int>( cell_type ),
                                 cvt::UnstructuredPfi( veclens[cell_type], last_time_step,
                                                       sub_volume_counts[cell_type] ) );
            }
        }

        // Two-pass
        // Piece reading
        std::unordered_map<int, int> sub_volume_ids;
        for ( auto& e : sub_volume_counts )
        {
            sub_volume_ids[e.first] = 1;
        }
        for ( auto vtu : input_pvtu.eachPiece() )
        {
            for ( auto file_format : vtu.eachCellType() )
            {
                std::cout << "  Importing ..." << std::endl;
                cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &file_format );

                kvs::UnstructuredVolumeObject* object = &importer;
                std::cout << "    sub volume id: " << sub_volume_id << std::endl;
                object->print( std::cout, kvs::Indent( 4 ) );

                std::cout << "  Writing (" << object->cellType() << " , " << sub_volume_id
                          << ") ..." << std::endl;
                std::string local_base =
                    std::string( base ) + "_" + std::to_string( object->cellType() );
                cvt::UnstructuredVolumeObjectExporter exporter( &importer );
                exporter.setWritingDataTypeToExternalBinary();
                exporter.write( directory, local_base, time_step,
                                sub_volume_ids[object->cellType()],
                                sub_volume_counts[object->cellType()] );

                pfi_map.at( static_cast<int>( object->cellType() ) )
                    .registerObject( &exporter, time_step, sub_volume_ids[object->cellType()] );

                ++sub_volume_ids[object->cellType()];
            }
        }

        ++time_step;
    }

    cvt::Pfl pfl;
    for ( auto& e : pfi_map )
    {
        std::string local_base = std::string( base ) + "_" + std::to_string( e.first );
        e.second.write( directory, local_base );
        e.second.print( std::cout );

        pfl.registerPfi( directory, local_base );
    }
    pfl.write( directory, base );
}

void SeriesPvtu2KvsmlWhole( const std::string& directory, const std::string& base,
                            const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::NumeralSequenceFiles<cvt::VtkXmlPUnstructuredGrid> time_series( src );
    int last_time_step = time_series.numberOfFiles() - 1;
    int time_step = 0;

    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;

    for ( auto input_pvtu : time_series.eachTimeStep() )
    {
        cvt::VtkXmlUnstructuredGrid vtu = input_pvtu.get();
        int sub_volume_count = 1;
        int sub_volume_id = 1;

        // Whole reading
        for ( auto file_format : vtu.eachCellType() )
        {
            std::cout << "  Importing ..." << std::endl;
            cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &file_format );

            kvs::UnstructuredVolumeObject* object = &importer;
            std::cout << "    sub volume id: " << sub_volume_id << std::endl;
            object->print( std::cout, kvs::Indent( 4 ) );

            std::cout << "  Writing (" << object->cellType() << " , " << sub_volume_id << ") ..."
                      << std::endl;
            std::string local_base =
                std::string( base ) + "_" + std::to_string( object->cellType() );
            cvt::UnstructuredVolumeObjectExporter exporter( &importer );
            exporter.setWritingDataTypeToExternalBinary();
            exporter.write( directory, local_base, time_step, sub_volume_id, sub_volume_count );

            if ( time_step == 0 )
            {
                pfi_map.emplace(
                    static_cast<int>( object->cellType() ),
                    cvt::UnstructuredPfi( object->veclen(), last_time_step, sub_volume_count ) );
            }
            pfi_map.at( static_cast<int>( object->cellType() ) )
                .registerObject( &exporter, time_step, sub_volume_id );
        }

        ++time_step;
    }

    cvt::Pfl pfl;
    for ( auto& e : pfi_map )
    {
        std::string local_base = std::string( base ) + "_" + std::to_string( e.first );
        e.second.write( directory, local_base );
        e.second.print( std::cout );

        pfl.registerPfi( directory, local_base );
    }
    pfl.write( directory, base );
}
