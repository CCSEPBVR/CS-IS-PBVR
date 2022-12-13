#include <iostream>
#include <string>

#include "kvs/UnstructuredVolumeObject"

#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "Exporter/VtkExporter.h"
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
    for ( auto vtu : input_pvtu.eachPieces() )
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
            exporter.writeForPbvr( directory, local_base, time_step, sub_volume_id,
                                   sub_volume_count );
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

void Pvtu2Kvsml2( const char* directory, const char* base, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlPUnstructuredGrid input_pvtu( src );
    // Whole reading
    cvt::VtkXmlUnstructuredGrid input_vtu = input_pvtu.get();

    int last_time_step = 0;
    int time_step = 0;
    int sub_volume_count = 1;
    int sub_volume_id = 1;
    cvt::Pfl pfl;

    for ( auto vtu : input_vtu.eachCellType() )
    {
        std::cout << "importing ..." << std::endl;
        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &vtu );

        kvs::UnstructuredVolumeObject* object = &importer;
        std::cout << "#nodes: " << object->numberOfNodes() << std::endl;
        std::cout << "#cells: " << object->numberOfCells() << std::endl;
        std::cout << "cellType: " << object->cellType() << std::endl;

        std::cout << "writing ..." << std::endl;
        auto local_base = std::string( base ) + "_" + std::to_string( object->cellType() );

        cvt::UnstructuredVolumeObjectExporter exporter( &importer );
        exporter.setWritingDataTypeToExternalBinary();
        exporter.writeForPbvr( directory, local_base, time_step, sub_volume_id, sub_volume_count );
        // or
        // exporter.write( "<directory/<local_base>_00000_0000001_0000001.kvsml" );

        cvt::UnstructuredPfi pfi( exporter.veclen(), last_time_step, sub_volume_count );
        pfi.registerObject( &exporter, time_step, sub_volume_id );
        pfi.write( directory, local_base );
        // or
        // pfi.write( "<directory/<local_base>.pfi" );
    }

    pfl.write( directory, base );
    // or
    // pfl.write( "<directory/<base>.pfi" );
}
