#include <iostream>
#include <string>

#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/StructuredVolumeExporter"
#include "kvs/StructuredVolumeObject"

#include "Exporter/VtkExporter.h"
#include "FileFormat/VTK/VtkXmlImageData.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/UnstructuredPfi.h"

void Vti2Kvsml( const char* directory, const char* base, const char* src )
{
    int last_time_step = 0;
    int time_step = 0;
    int sub_volume_id = 1;
    int sub_volume_count = 1;

    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::VtkXmlImageData input_vti( src );
    cvt::VtkImporter<cvt::VtkXmlImageData> importer( &input_vti );

    kvs::StructuredVolumeObject* object = &importer;
    object->updateMinMaxCoords();
    object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );
    object->print( std::cout, kvs::Indent( 2 ) );

    auto dst = std::string( directory ) + "/" + base + "_00000_0000001_0000001.kvsml";
    std::cout << "Writing " << dst << " ..." << std::endl;
    kvs::StructuredVolumeExporter<kvs::KVSMLStructuredVolumeObject> exporter( &importer );
    exporter.setWritingDataTypeToExternalBinary();
    exporter.write( dst );

    cvt::UnstructuredPfi pfi( last_time_step, sub_volume_count );
    pfi.registerObject( &exporter, time_step, sub_volume_id );
    pfi.write( directory, base );
}