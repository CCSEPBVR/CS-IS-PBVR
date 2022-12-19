#include <iostream>
#include <string>

#include "kvs/KVSMLPolygonObject"
#include "kvs/PolygonExporter"
#include "kvs/PolygonObject"

#include "Exporter/VtkExporter.h"
#include "FileFormat/VTK/VtkXmlPolyData.h"
#include "Importer/VtkImporter.h"

void Vtp2Kvsml( const char* dst, const char* src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::VtkXmlPolyData input_vtp( src );
    cvt::VtkImporter<cvt::VtkXmlPolyData> importer( &input_vtp );

    kvs::PolygonObject* polygon_object = &importer;
    polygon_object->print( std::cout, kvs::Indent( 2 ) );

    std::cout << "Writing " << dst << " ..." << std::endl;
    kvs::PolygonExporter<kvs::KVSMLPolygonObject> exporter( &importer );
    exporter.setWritingDataTypeToExternalBinary();
    exporter.write( dst );
}