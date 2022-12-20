#include <iostream>
#include <string>

#include "kvs/LineExporter"
#include "kvs/UnstructuredVolumeObject"

#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "FileFormat/AVS/AvsUcd.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/Pfl.h"
#include "PBVRFileInformation/UnstructuredPfi.h"

void Avs2Kvsml( const char* directory, const char* base, const char* src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::AvsUcd input_avs( src );

    cvt::Pfl pfl;

    for ( auto vtu : input_avs.eachCellType() )
    {
        int time_step = 0;
        int last_time_step = 0;
        int sub_volume_id = 1;
        int sub_volume_count = 1;

        cvt::VtkImporter<cvt::AvsUcd> importer( &vtu );
        std::cout << "  cell type: " << importer.cellType() << std::endl;

        kvs::UnstructuredVolumeObject* object = &importer;
        object->print( std::cout, kvs::Indent( 4 ) );

        std::cout << "  Writing to " << directory << " ..." << std::endl;
        auto local_base = std::string( base ) + "_" + std::to_string( object->cellType() );

        cvt::UnstructuredVolumeObjectExporter exporter( &importer );
        exporter.setWritingDataTypeToExternalBinary();
        exporter.write( directory, local_base, time_step, sub_volume_id, sub_volume_count );

        cvt::UnstructuredPfi pfi( object->veclen(), last_time_step, sub_volume_count );
        pfi.registerObject( &exporter, time_step, sub_volume_id );
        pfi.write( directory, local_base );

        pfl.registerPfi( directory, local_base );
    }

    pfl.write( directory, base );
}