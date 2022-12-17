#include <iostream>
#include <string>

void Stl2Stl( const char* dst, const char* src );
void Stl2Kvsml( const char* dst, const char* src );
void Vtp2Kvsml( const char* dst, const char* src );
void Vtr2Vtr( const char* dst, const char* src );
void Vti2Vti( const char* dst, const char* src );
void Vti2Kvsml( const char* directory, const char* base, const char* src );
void UniformPvts2Vts( const char* dst, const char* src );
void Vts2Vts( const char* dst, const char* src );
void Vts2Kvsml( const char* directory, const char* base, const char* src );
void Vtu2Kvsml( const char* directory, const char* base, const char* src );
void SeriesVtu2Kvsml( const char* directory, const char* base, const char* src );
void LineVtu2Kvsml( const char* dst, const char* src );
void Pvtu2Kvsml( const char* directory, const char* base, const char* src );
void Vtm2Vtm( const char* dst, const char* src );
void Case2Kvsml( const char* directory, const char* base, const char* src );
void Cgns2Kvsml( const char* directory, const char* base, const char* src );

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
    else if ( input_format == "vtp" && output_format == "kvsml" )
    {
        Vtp2Kvsml( argv[4], argv[3] );
    }
    else if ( input_format == "vtr" && output_format == "vtr" )
    {
        Vtr2Vtr( argv[4], argv[3] );
    }
    else if ( input_format == "vti" && output_format == "vti" )
    {
        Vti2Vti( argv[4], argv[3] );
    }
    else if ( input_format == "uniform-pvts" && output_format == "vts" )
    {
        UniformPvts2Vts( argv[4], argv[3] );
    }
    else if ( input_format == "vti" && output_format == "kvsml" )
    {
        Vti2Kvsml( argv[4], argv[5], argv[3] );
    }
    else if ( input_format == "vts" && output_format == "vts" )
    {
        Vts2Vts( argv[4], argv[3] );
    }
    else if ( input_format == "vts" && output_format == "kvsml" )
    {
        Vts2Kvsml( argv[4], argv[5], argv[3] );
    }
    else if ( input_format == "vtu" && output_format == "kvsml" )
    {
        Vtu2Kvsml( argv[4], argv[5], argv[3] );
    }
    else if ( input_format == "series-vtu" && output_format == "kvsml" )
    {
        SeriesVtu2Kvsml( argv[4], argv[5], argv[3] );
    }
    else if ( input_format == "line-vtu" && output_format == "kvsml" )
    {
        LineVtu2Kvsml( argv[4], argv[3] );
    }
    else if ( input_format == "pvtu" && output_format == "kvsml" )
    {
        Pvtu2Kvsml( argv[4], argv[5], argv[3] );
    }
    else if ( std::string( argv[1] ) == "vtm" )
    {
        Vtm2Vtm( argv[4], argv[3] );
        return -1;
    }
    else if ( std::string( argv[1] ) == "case" )
    {
        Case2Kvsml( argv[4], argv[5], argv[3] );
        return -1;
    }
    else if ( std::string( argv[1] ) == "cgns" )
    {
        Cgns2Kvsml( argv[4], argv[5], argv[3] );
        return -1;
    }
    else
    {
        std::cerr << "unknown format: " << input_format << " " << output_format << std::endl;
        return -1;
    }

    return 0;
}
