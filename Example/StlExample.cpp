#include <iostream>
#include <string>

#include "kvs/PolygonExporter"
#include "kvs/PolygonObject"

#include "Exporter/VtkExporter.h"
#include "FileFormat/STL/Stl.h"
#include "Importer/VtkImporter.h"

void Stl2Stl( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::Stl input_stl( src );
    cvt::VtkImporter<cvt::Stl> importer( &input_stl );

    kvs::PolygonObject* polygon_object = &importer;
    std::cout << "#vertices: " << polygon_object->numberOfVertices() << std::endl;
    std::cout << "#normals: " << polygon_object->numberOfNormals() << std::endl;

    std::cout << "writing " << dst << " ..." << std::endl;
    cvt::VtkExporter<cvt::Stl> exporter( &importer );
    cvt::Stl* output_stl = &exporter;
    output_stl->write( dst );
}

void Stl2Kvsml( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::Stl input_stl( src );
    cvt::VtkImporter<cvt::Stl> importer( &input_stl );

    kvs::PolygonObject* polygon_object = &importer;
    std::cout << "#vertices: " << polygon_object->numberOfVertices() << std::endl;
    std::cout << "#normals: " << polygon_object->numberOfNormals() << std::endl;

    std::cout << "writing " << dst << " ..." << std::endl;
    kvs::PolygonExporter<kvs::KVSMLPolygonObject> exporter( &importer );
    exporter.write( dst );
}