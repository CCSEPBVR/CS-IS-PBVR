/*
 * Created by Japan Atomic Energy Agency
 *
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legal code along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

#include "Filesystem.h"

void Stl2Stl( const std::string& dst, const std::string& src );
void Stl2Kvsml( const std::string& dst, const std::string& src );
void Vtp2Kvsml( const std::string& dst, const std::string& src );
void Plot3d2Kvsml( const std::string& directory, const std::string& base, const std::string& xyz,
                   const std::string& q, const std::string& f );
void Vtr2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void StructuredPoints2Kvsml( const std::string& directory, const std::string& base,
                             const std::string& src );
void Vti2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void SeriesVti2Kvsml( const std::string& directory, const std::string& base,
                      const std::string& src );
void Vts2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void Pvts2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void AvsUcd2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void Vtu2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void SeriesVtu2Kvsml( const std::string& directory, const std::string& base,
                      const std::string& src );
void PointVtu2Kvsml( const std::string& dst, const std::string& src );
void LineVtu2Kvsml( const std::string& dst, const std::string& src );
void TriangleVtu2Kvsml( const std::string& dst, const std::string& src );
void SeriesPvtu2Kvsml( const std::string& directory, const std::string& base,
                       const std::string& src );
void SeriesPvtu2KvsmlWhole( const std::string& directory, const std::string& base,
                            const std::string& src );
void AccessToVtm( const std::string& src );
void SeriesVtm2Kvsml( const std::string& directory, const std::string& base,
                      const std::string& src );
void Case2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void Cgns2Kvsml( const std::string& directory, const std::string& base, const std::string& src );

int main( int argc, char** argv )
{
    namespace fs = std::filesystem;

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
        src /= "src";
        src /= "Testing";
        src /= "Data";
        src /= "gears.stl";
        fs::path dst = output_directory;
        dst /= "gears.stl";
        Stl2Stl( dst.string(), src.string() );
    }
    else if ( example_name == "stl2kvsml" )
    {
        fs::path src = input_directory;
        src /= "VTKExamples";
        src /= "src";
        src /= "Testing";
        src /= "Data";
        src /= "gears.stl";
        fs::path dst = output_directory;
        dst /= "gears.kvsml";
        Stl2Kvsml( dst.string(), src.string() );
    }
    else if ( example_name == "vtp2kvsml" )
    {
        fs::path src_t = input_directory;
        src_t /= "Polygon";
        src_t /= "triangle.vtp";
        fs::path dst_t = output_directory;
        dst_t /= "triangle.kvsml";
        Vtp2Kvsml( dst_t.string(), src_t.string() );
        fs::path src_q = input_directory;
        src_q /= "Polygon";
        src_q /= "triangle.vtp";
        fs::path dst_q = output_directory;
        dst_q /= "triangle.kvsml";
        Vtp2Kvsml( dst_q.string(), src_q.string() );
    }
    else if ( example_name == "plot3d2kvsml" )
    {
        fs::path xyz = input_directory;
        xyz /= "pbvr_sample_data";
        xyz /= "plot3d";
        fs::path q = xyz;
        fs::path f = xyz;
        xyz /= "singleGrid.xyz";
        q /= "singleGrid.q";
        f /= "singleGrid.f";
        Plot3d2Kvsml( output_directory, "singleGrid", xyz.string(), q.string(), f.string() );
    }
    else if ( example_name == "vtr2kvsml" )
    {
        fs::path src = input_directory;
        src /= "VTKExamples";
        src /= "src";
        src /= "Testing";
        src /= "Data";
        src /= "RectilinearGrid.vtr";
        fs::path dst = output_directory;
        dst /= "RectilinearGrid.kvsml";
        Vtr2Kvsml( output_directory, "RectilinearGrid", src.string() );
    }
    else if ( example_name == "structuredpoints2kvsml" )
    {
        fs::path src = input_directory;
        src /= "VTKExamples";
        src /= "src";
        src /= "Testing";
        src /= "Data";
        src /= "StructuredPoints.vtk";
        StructuredPoints2Kvsml( output_directory, "StructuredPoints", src.string() );
    }
    else if ( example_name == "vti2kvsml" )
    {
        fs::path src = input_directory;
        src /= "VTKExamples";
        src /= "src";
        src /= "Testing";
        src /= "Data";
        src /= "vase.vti";
        Vti2Kvsml( output_directory, "vase", src.string() );
    }
    else if ( example_name == "seriesvti2kvsml" )
    {
        fs::path src = input_directory;
        src /= "Vti";
        src /= "union_*.vti";
        SeriesVti2Kvsml( output_directory, "union", src.string() );
    }
    else if ( example_name == "vts2kvsml" )
    {
        fs::path src = input_directory;
        src /= "VTKExamples";
        src /= "src";
        src /= "Testing";
        src /= "Data";
        src /= "StructuredGrid.vts";
        Vts2Kvsml( output_directory, "StructuredGrid", src.string() );
    }
    else if ( example_name == "pvts2kvsml" )
    {
        fs::path src = input_directory;
        src /= "Pvts";
        src /= "example_0.pvts";
        Pvts2Kvsml( output_directory, "example_0", src.string() );
    }
    else if ( example_name == "avsucd2kvsml" )
    {
        fs::path src = input_directory;
        src /= "frontstr.inp";
        fs::path dst = output_directory;
        dst /= "AVS";
        mkdir( dst );
        AvsUcd2Kvsml( dst.string(), "frontstr", src.string() );
    }
    else if ( example_name == "vtu2kvsml" )
    {
        fs::path src = input_directory;
        src /= "VTKExamples";
        src /= "src";
        src /= "Testing";
        src /= "Data";
        src /= "fire_ug.vtu";
        fs::path dst = output_directory;
        dst /= "fire_ug";
        mkdir( dst );
        Vtu2Kvsml( dst.string(), "fire_ug", src.string() );
    }
    else if ( example_name == "seriesvtu2kvsml" )
    {
        fs::path pattern_t = input_directory;
        pattern_t /= "Tetra";
        pattern_t /= "tetra_only_*.vtu";
        fs::path dst_t = output_directory;
        dst_t /= "Tetra";
        mkdir( dst_t );
        SeriesVtu2Kvsml( dst_t.string(), "tetra_only", pattern_t.string() );

        fs::path pattern_h = input_directory;
        pattern_h /= "Hex";
        pattern_h /= "hex_only_*.vtu";
        fs::path dst_h = output_directory;
        dst_h /= "Hex";
        mkdir( dst_h );
        SeriesVtu2Kvsml( dst_h.string(), "hex_only", pattern_h.string() );

        fs::path pattern_th = input_directory;
        pattern_th /= "TetraAndHex";
        pattern_th /= "tetra_and_hex_*.vtu";
        fs::path dst_th = output_directory;
        dst_th /= "TetraAndHex";
        mkdir( dst_th );
        SeriesVtu2Kvsml( dst_th.string(), "tetra_and_hex", pattern_th.string() );
    }
    else if ( example_name == "point2kvsml" )
    {
        fs::path src = input_directory;
        src /= "Vertex";
        src /= "vertex_only_0.vtu";
        fs::path dst = output_directory;
        dst /= "vertex.kvsml";
        PointVtu2Kvsml( dst.string(), src.string() );
        // Stop by a checker member
        LineVtu2Kvsml( dst.string(), src.string() );
        TriangleVtu2Kvsml( dst.string(), src.string() );
    }
    else if ( example_name == "line2kvsml" )
    {
        fs::path src = input_directory;
        src /= "helix.vtu";
        fs::path dst = output_directory;
        dst /= "helix.kvsml";
        LineVtu2Kvsml( dst.string(), src.string() );
        // Stop by a checker member
        PointVtu2Kvsml( dst.string(), src.string() );
        TriangleVtu2Kvsml( dst.string(), src.string() );
    }
    else if ( example_name == "triangle2kvsml" )
    {
        fs::path src = input_directory;
        src /= "Triangle";
        src /= "triangle_only_0.vtu";
        fs::path dst = output_directory;
        dst /= "triangle.kvsml";
        TriangleVtu2Kvsml( dst.string(), src.string() );
        // Stop by a checker member
        PointVtu2Kvsml( dst.string(), src.string() );
        LineVtu2Kvsml( dst.string(), src.string() );
    }
    else if ( example_name == "seriespvtu2kvsml" )
    {
        fs::path src = input_directory;
        src /= "Pvtu";
        src /= "example_*.pvtu";
        fs::path dst_p = output_directory;
        dst_p /= "Piece";
        mkdir( dst_p );
        SeriesPvtu2Kvsml( dst_p.string(), "example", src.string() );

        fs::path dst_w = output_directory;
        dst_w /= "Whole";
        mkdir( dst_w );
        SeriesPvtu2KvsmlWhole( dst_w.string(), "example", src.string() );
    }
    else if ( example_name == "accessvtm" )
    {
        fs::path src = input_directory;
        src /= "MultiBlock";
        src /= "multiblock_0.vtm";
        AccessToVtm( src.string() );
    }
    else if ( example_name == "seriesvtm2kvsml" )
    {
        fs::path src = input_directory;
        src /= "UnstructuredMultiBlock";
        src /= "multiblock_*.vtm";
        fs::path dst = output_directory;
        dst /= "UnstructuredMultiBlock";
        mkdir( dst );
        SeriesVtm2Kvsml( dst.string(), "multiblock", src.string() );
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
