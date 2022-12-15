#include <iostream>
#include <string>

#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/StructuredVolumeExporter"
#include "kvs/StructuredVolumeObject"

#include "Exporter/StructuredVolumeObjectExporter.h"
#include "Exporter/VtkExporter.h"
#include "FileFormat/KVSML/KvsmlStructuredVolumeObject.h"
#include "FileFormat/UnstructuredPfi.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "Importer/VtkImporter.h"

void Vts2Vts( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlStructuredGrid input_vts( src );
    cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &input_vts );

    kvs::StructuredVolumeObject* object = &importer;
    object->print( std::cout, kvs::Indent( 2 ) );

    std::cout << "writing " << dst << " ..." << std::endl;
    cvt::VtkExporter<cvt::VtkXmlStructuredGrid> exporter( &importer );
    cvt::VtkXmlStructuredGrid* output_vts = &exporter;
    output_vts->write( dst );
}

void Vts2Kvsml( const char* directory, const char* base, const char* src )
{
    int last_time_step = 0;
    int time_step = 0;
    int sub_volume_id = 1;
    int sub_volume_count = 1;

    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlStructuredGrid input_vts( src );
    cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &input_vts );

    kvs::StructuredVolumeObject* object = &importer;
    object->updateMinMaxCoords();
    object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );
    object->print( std::cout, kvs::Indent( 2 ) );

    std::cout << "writing to " << directory << " ..." << std::endl;
    cvt::StructuredVolumeObjectExporter exporter( &importer );
    exporter.setWritingDataTypeToExternalBinary();
    exporter.write( directory, base, time_step, sub_volume_id, sub_volume_count );

    cvt::UnstructuredPfi pfi( last_time_step, sub_volume_count );
    pfi.registerObject( &exporter, time_step, sub_volume_id );
    pfi.write( directory, base );
}