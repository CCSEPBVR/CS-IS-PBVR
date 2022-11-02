#include <iostream>
#include <string>

void Stl2Stl( const char* dst, const char* src );
void Stl2Kvsml( const char* dst, const char* src );
void Vtr2Vtr( const char* dst, const char* src );
void Vtm2Vtm( const char* dst, const char* src );

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
    else if ( input_format == "vtr" && output_format == "vtr" )
    {
        Vtr2Vtr( argv[4], argv[3] );
    }
    else if ( std::string( argv[1] ) == "vtm" )
    {
        Vtm2Vtm( argv[4], argv[3] );
        return -1;
    }
    else
    {
        std::cerr << "unknown format: " << input_format << " " << output_format << std::endl;
        return -1;
    }

    return 0;
}