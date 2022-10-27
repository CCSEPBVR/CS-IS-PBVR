#include "Exporter/VtkExporter.h"
#include "FileFormat/STL/Stl.h"
#include "Importer/VtkImporter.h"
#include "kvs/KVSMLPolygonObject"
#include "kvs/PolygonExporter"
#include "kvs/Stl"

#include <iostream>
#include <memory>
#include <string>

void Stl2Stl( const char* dst, const char* src );
void Stl2Kvsml( const char* dst, const char* src );

int main( int argc, char** argv )
{
    if ( argc < 5 )
    {
        std::cerr << "kvsml-converter input_format output_format src dst" << std::endl;
        return -1;
    }

    std::string input_format = std::string( argv[1] );
    std::cout << "input file format: " << argv[1] << std::endl;
    std::string output_format = std::string( argv[2] );
    std::cout << "output file format: " << argv[2] << std::endl;

    if ( input_format == "stl" && output_format == "stl" )
    {
        Stl2Stl( argv[4], argv[3] );
    }
    else if ( input_format == "stl" && output_format == "kvsml" )
    {
        Stl2Kvsml( argv[4], argv[3] );
    }
    else
    {
        std::cerr << "unknown format: " << input_format << " " << output_format << std::endl;
        return -1;
    }

    return 0;
}
