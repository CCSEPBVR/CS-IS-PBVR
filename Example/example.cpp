#include "Exporter/PolygonExporter.h"
#include "FileFormat/STL/Stl.h"
#include "Importer/PolygonImporter.h"
#include "kvs/KVSMLPolygonObject"
#include "kvs/PolygonExporter"
#include "kvs/Stl"

#include <iostream>
#include <memory>
#include <string>

int main( int argc, char** argv )
{
    std::cout << argc << std::endl;
    if ( argc < 5 )
    {
        return -1;
    }

    std::cout << "input file format: " << argv[1] << std::endl;

    std::unique_ptr<cvt::PolygonImporter> importer;

    if ( std::string( argv[1] ) == "stl" )
    {
        std::cout << "reading " << argv[3] << " ..." << std::endl;
        auto input_stl = cvt::Stl( std::string( argv[3] ) );
        importer.reset( new cvt::PolygonImporter( &input_stl ) );

        kvs::PolygonObject* polygon_object = importer.get();
        std::cout << "#vertices: " << importer->numberOfVertices() << std::endl;
        std::cout << "#normals: " << importer->numberOfNormals() << std::endl;
    }
    else
    {
        std::cerr << "unknown format: " << argv[1] << std::endl;
        return -1;
    }

    std::string output_format = std::string( argv[2] );
    std::cout << "output file format: " << argv[2] << std::endl;

    if ( output_format == "stl" )
    {
        std::cout << "writing " << argv[4] << " ..." << std::endl;
        cvt::PolygonExporter<cvt::Stl> exporter(
            dynamic_cast<kvs::PolygonObject*>( importer.get() ) );
        exporter.write( std::string( argv[4] ) );
    }
    else if ( output_format == "kvsml" )
    {
        std::cout << "writing " << argv[4] << " ..." << std::endl;
        kvs::PolygonExporter<kvs::KVSMLPolygonObject> exporter(
            dynamic_cast<kvs::PolygonObject*>( importer.get() ) );
        exporter.write( std::string( argv[4] ) );
    }
    else
    {
        std::cerr << "unknown output format: " << output_format << std::endl;
        return -1;
    }

    return 0;
}
