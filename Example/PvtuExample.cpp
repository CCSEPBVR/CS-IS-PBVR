#include <iostream>
#include <string>

#include "kvs/UnstructuredVolumeObject"

#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "Exporter/VtkExporter.h"
#include "FileFormat/NumeralSequenceFiles.h"
#include "FileFormat/Pfl.h"
#include "FileFormat/UnstructuredPfi.h"
#include "FileFormat/VTK/VtkXmlPUnstructuredGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "Importer/VtkImporter.h"

void Pvtu2Kvsml( const char* directory, const char* base, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlPUnstructuredGrid input_pvtu2( src );
    const cvt::VtkXmlPUnstructuredGrid& input_pvtu = input_pvtu2;
    std::cout << "#piece: " << input_pvtu.number_of_pieces() << std::endl;

    int last_time_step = 0;
    int time_step = 0;
    int sub_volume_count = input_pvtu.number_of_pieces();
    int sub_volume_id = 1;

    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;

    // Piece reading
    for ( auto vtu : input_pvtu.eachPiece() )
    {
        for ( auto file_format : vtu.eachCellType() )
        {
            std::cout << "importing ..." << std::endl;
            cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &file_format );

            kvs::UnstructuredVolumeObject* object = &importer;
            std::cout << "sub volume id: " << sub_volume_id << std::endl;
            std::cout << "#nodes: " << object->numberOfNodes() << std::endl;
            std::cout << "#cells: " << object->numberOfCells() << std::endl;
            std::cout << "type: " << object->cellType() << std::endl;

            std::cout << "writing (" << object->cellType() << " , " << sub_volume_id << ") ..."
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

        pfl.registerPfi( directory, local_base );
    }
    pfl.write( directory, base );
}

void SeriesPvtu2Kvsml( const char* directory, const char* base, const char* src )
{

    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::NumeralSequenceFiles<cvt::VtkXmlPUnstructuredGrid> time_series( src );
    int last_time_step = time_series.numberOfFiles() - 1;
    int time_step = 0;

    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;

    for ( auto input_pvtu : time_series.eachTimeStep() )
    {
        int sub_volume_count = input_pvtu.number_of_pieces();
        int sub_volume_id = 1;

        // Piece reading
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
                exporter.write( directory, local_base, time_step, sub_volume_id, sub_volume_count );
                // or
                // exporter.write( "<directory/<local_base>_00000_0000001_0000001.kvsml" );

                if ( time_step == 0 )
                {
                    pfi_map.emplace( static_cast<int>( object->cellType() ),
                                     cvt::UnstructuredPfi( object->veclen(), last_time_step,
                                                           sub_volume_count ) );
                }
                pfi_map.at( static_cast<int>( object->cellType() ) )
                    .registerObject( &exporter, time_step, sub_volume_id );
            }

            ++sub_volume_id;
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
}

void SeriesPvtu2KvsmlWhole( const char* directory, const char* base, const char* src )
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
