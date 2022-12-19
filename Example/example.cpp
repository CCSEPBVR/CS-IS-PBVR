#include <algorithm>
#include <cctype>
#include <experimental/filesystem>
#include <iostream>
#include <string>

namespace fs = std::experimental::filesystem;

void Stl2Stl( const char* dst, const char* src );
void Stl2Kvsml( const char* dst, const char* src );
void Vtp2Kvsml( const char* dst, const char* src );
void Vtr2Kvsml( const char* directory, const char* base, const char* src );
void Vti2Kvsml( const char* directory, const char* base, const char* src );
void Vts2Kvsml( const char* directory, const char* base, const char* src );
void Vtu2Kvsml( const char* directory, const char* base, const char* src );
void UniformPvts2Vts( const char* dst, const char* src );
void SeriesVtu2Kvsml( const char* directory, const char* base, const char* src );
void LineVtu2Kvsml( const char* dst, const char* src );
void SeriesPvtu2Kvsml( const char* directory, const char* base, const char* src );
void SeriesPvtu2KvsmlWhole( const char* directory, const char* base, const char* src );
void Vtm2Vtm( const char* dst, const char* src );
void Case2Kvsml( const char* directory, const char* base, const char* src );
void Cgns2Kvsml( const char* directory, const char* base, const char* src );

int main( int argc, char** argv )
{
    if ( argc < 2 )
    {
        std::cerr << "kvsml-converter example_name [input_directory] [output_directory]"
                  << std::endl;
        return -1;
    }

    std::string example_name = std::string( argv[1] );
    std::transform( example_name.begin(), example_name.end(), example_name.begin(), ::tolower );
    std::cout << "Example: " << example_name << std::endl;

    std::string input_directory;
    if ( argc < 3 )
    {
        if ( auto dir = std::getenv( "KVSML_CONVERTER_EXAMPLE_DIR" ) )
        {
            input_directory = dir;
        }
        else
        {
            input_directory = ".";
        }
    }
    else
    {
        input_directory = argv[2];
    }
    std::cout << "Input directory path: " << input_directory << std::endl;

    std::string output_directory;
    if ( argc < 4 )
    {
        output_directory = ".";
    }
    else
    {
        output_directory = argv[3];
    }
    std::cout << "Output directory path: " << output_directory << std::endl;

    auto mkdir = []( const fs::path& d ) {
        if ( !fs::exists( d ) )
        {
            if ( !fs::create_directories( d ) )
            {
                std::cerr << "Failed to create an output directory" << std::endl;
                return false;
            }
        }
        return true;
    };
    if ( !mkdir( output_directory ) )
    {
        return -1;
    }

    if ( example_name == "stl2stl" )
    {
        fs::path src = input_directory;
        src /= "VTKExamples";
        src /= "gears.stl";
        fs::path dst = output_directory;
        dst /= "gears.stl";
        Stl2Stl( dst.c_str(), src.c_str() );
    }
    else if ( example_name == "stl2kvsml" )
    {
        fs::path src = input_directory;
        src /= "VTKExamples";
        src /= "gears.stl";
        fs::path dst = output_directory;
        dst /= "gears.kvsml";
        Stl2Kvsml( dst.c_str(), src.c_str() );
    }
    else if ( example_name == "vtp2kvsml" )
    {
        fs::path src_t = input_directory;
        src_t /= "Polygon";
        src_t /= "triangle.vtp";
        fs::path dst_t = output_directory;
        dst_t /= "triangle.kvsml";
        Vtp2Kvsml( dst_t.c_str(), src_t.c_str() );
        fs::path src_q = input_directory;
        src_q /= "Polygon";
        src_q /= "triangle.vtp";
        fs::path dst_q = output_directory;
        dst_q /= "triangle.kvsml";
        Vtp2Kvsml( dst_q.c_str(), src_q.c_str() );
    }
    else if ( example_name == "vtr2kvsml" )
    {
        fs::path src = input_directory;
        src /= "VTKExamples";
        src /= "RectilinearGrid.vtr";
        fs::path dst = output_directory;
        dst /= "RectilinearGrid.kvsml";
        Vtr2Kvsml( output_directory.c_str(), "RectilinearGrid", src.c_str() );
    }
    else if ( example_name == "vti2kvsml" )
    {
        fs::path src = input_directory;
        src /= "mandelbrot.vti";
        Vti2Kvsml( output_directory.c_str(), "mandelbrot", src.c_str() );
    }
    else if ( example_name == "vts2kvsml" )
    {
        fs::path src = input_directory;
        src /= "VTKExamples";
        src /= "StructuredGrid.vts";
        Vts2Kvsml( output_directory.c_str(), "StructuredGrid", src.c_str() );
    }
    else if ( example_name == "pvts2vts" )
    {
        UniformPvts2Vts( argv[4], argv[3] );
    }
    else if ( example_name == "vtu2kvsml" )
    {
        fs::path src = input_directory;
        src /= "periodicPiece.vtu";
        fs::path dst = output_directory;
        dst /= "periodicPiece";
        mkdir( dst );
        Vtu2Kvsml( dst.c_str(), "periodicPiece", src.c_str() );
    }
    else if ( example_name == "seriesvtu2kvsml" )
    {
        fs::path pattern_t = input_directory;
        pattern_t /= "Tetra";
        pattern_t /= "tetra_only_*.vtu";
        fs::path dst_t = output_directory;
        dst_t /= "Tetra";
        mkdir( dst_t );
        SeriesVtu2Kvsml( dst_t.c_str(), "tetra_only", pattern_t.c_str() );

        fs::path pattern_h = input_directory;
        pattern_h /= "Hex";
        pattern_h /= "hex_only_*.vtu";
        fs::path dst_h = output_directory;
        dst_h /= "Hex";
        mkdir( dst_h );
        SeriesVtu2Kvsml( dst_h.c_str(), "hex_only", pattern_h.c_str() );

        fs::path pattern_th = input_directory;
        pattern_th /= "TetraAndHex";
        pattern_th /= "tetra_and_hex_*.vtu";
        fs::path dst_th = output_directory;
        dst_th /= "TetraAndHex";
        mkdir( dst_th );
        SeriesVtu2Kvsml( dst_th.c_str(), "tetra_and_hex", pattern_th.c_str() );
    }
    else if ( example_name == "line2kvsml" )
    {
        fs::path src = input_directory;
        src /= "helix.vtu";
        fs::path dst = output_directory;
        dst /= "helix.kvsml";
        LineVtu2Kvsml( dst.c_str(), src.c_str() );
    }
    else if ( example_name == "seriespvtu2kvsml" )
    {
        fs::path src = input_directory;
        src /= "Pvtu";
        src /= "example_*.pvtu";
        fs::path dst_p = output_directory;
        dst_p /= "Piece";
        mkdir( dst_p );
        SeriesPvtu2Kvsml( dst_p.c_str(), "example", src.c_str() );

        fs::path dst_w = output_directory;
        dst_w /= "Whole";
        mkdir( dst_w );
        SeriesPvtu2KvsmlWhole( dst_w.c_str(), "example", src.c_str() );
    }
    else if ( example_name == "vtm2kvsml" )
    {
        Vtm2Vtm( argv[3], argv[2] );
        return -1;
    }
    else if ( example_name == "case2kvsml" )
    {
        Case2Kvsml( argv[3], argv[4], argv[2] );
    }
    else if ( example_name == "cgns2kvsml" )
    {
        Cgns2Kvsml( argv[3], argv[4], argv[2] );
    }
    else
    {
        std::cerr << "Unknown example" << std::endl;
        return -1;
    }

    return 0;
}
