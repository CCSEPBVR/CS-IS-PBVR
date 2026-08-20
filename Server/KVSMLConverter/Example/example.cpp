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
#include <iostream>
#include <string>
#include <vector>

#include "Filesystem.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "TimeSeriesFiles/SeriesFileResolver.h"

bool contains_wildcard( const std::string& path );
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
                      const std::vector<std::string>& file_paths );
void Vts2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void Pvts2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void AvsUcd2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void Vtu2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void SeriesVtu2Kvsml( const std::string& directory, const std::string& base,
                      const std::vector<std::string>& file_paths );
void PointVtu2Kvsml( const std::string& dst, const std::string& src );
void LineVtu2Kvsml( const std::string& dst, const std::string& src );
void TriangleVtu2Kvsml( const std::string& dst, const std::string& src );
void Pvtu2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void SeriesPvtu2Kvsml( const std::string& directory, const std::string& base,
                       const std::vector<std::string>& file_paths );
void SeriesPvtu2KvsmlWhole( const std::string& directory, const std::string& base,
                            const std::vector<std::string>& file_paths );
void AccessToVtm( const std::string& src );
void Vtm2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void SeriesVtm2Kvsml( const std::string& directory, const std::string& base,
                      const std::vector<std::string>& file_paths );
void MergeBlock( const std::string& dst, const std::string& src, const std::string& config_path );
void MergeBlockAsPolygon( const std::string& dst_vtk, const std::string& dst_kvsml,
                          const std::string& dst_stl, const std::string& src,
                          const std::string& config_path );
void Case2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void Cgns2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void Netcdf2Kvsml( const std::string& directory, const std::string& base, const std::string& src );
void SeriesNetcdf2Kvsml( const std::string& directory, const std::string& base,
                         const std::vector<std::string>& file_paths );

int main( int argc, char** argv )
{
    using fs = cvt::filesystem;

    if ( argc != 3 )
    {
        std::cout << "kvsml-converter [input_file] [output_directory]" << std::endl;
        std::cout << "If input_file is time-series files, use a wildcard." << std::endl;
        std::cout << "On MacOS or Linux, filename containing a wildcard should be enclosed in single quotation." << std::endl;
        std::cout << "EXAMPLE:kvsml-converter 'input_folder/input_subfolder/example_*.vtu' output_folder/output_subfolder" << std::endl;
        return -1;
    }

    std::string input_file;
    input_file = argv[1];
    fs::path input_file_path = input_file;

    std::string output_directory;
    output_directory = argv[2];
    fs::path output_directory_path = output_directory;

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
    if ( contains_wildcard( input_file ) )
    {
        cvt::ResolvedSeries series;
        std::string error;
        if ( !cvt::ResolveSeries( input_file_path.generic_string(), series, error ) )
        {
            std::cerr << error << std::endl;
            return -1;
        }
        if ( !mkdir( output_directory_path ) ) return -1;

        switch ( series.format )
        {
        case cvt::SeriesFormat::Vti:
            SeriesVti2Kvsml( output_directory_path.string(), series.output_base,
                             series.file_paths );
            break;
        case cvt::SeriesFormat::Vtu:
            SeriesVtu2Kvsml( output_directory_path.string(), series.output_base,
                             series.file_paths );
            break;
        case cvt::SeriesFormat::Vtm:
            SeriesVtm2Kvsml( output_directory_path.string(), series.output_base,
                             series.file_paths );
            break;
        case cvt::SeriesFormat::Pvtu:
            SeriesPvtu2Kvsml( output_directory_path.string(), series.output_base,
                              series.file_paths );
            break;
        case cvt::SeriesFormat::Netcdf:
            SeriesNetcdf2Kvsml( output_directory_path.string(), series.output_base,
                                series.file_paths );
            break;
        }
        return 0;
    }

    if ( !mkdir( output_directory_path ) ) return -1;

    fs::path input_file_extension = input_file_path.extension();
    fs::path input_filename_without_extension = input_file_path.filename().stem();

    if ( input_file_extension == ".stl" )
    {
        std::string separator(1, fs::path::preferred_separator);
        std::string output_file_path = output_directory_path.string() + separator + input_filename_without_extension.string() + ".kvsml";
        Stl2Kvsml( output_file_path, input_file_path.string() );
    }
    else if ( input_file_extension == ".vtp" )
    {
        std::string separator(1, fs::path::preferred_separator);
        std::string output_file_path = output_directory_path.string() + separator + input_filename_without_extension.string() + ".kvsml";
        Vtp2Kvsml( output_file_path, input_file_path.string() );
    }
    /*
    else if ( input_file_extension == ".xyz" || input_file_extension == ".q" || input_file_extension == ".f" )
    {
        fs::path input_file_path_xyz = input_file_path.replace_extension(".xyz");
        fs::path input_file_path_q = input_file_path.replace_extension(".q");
        fs::path input_file_path_f = input_file_path.replace_extension(".f");
        Plot3d2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path_xyz.string(), input_file_path_q.string(), input_file_path_f.string() );
    }
    else if ( input_file_extension == ".vtr" )
    {
        Vtr2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path.string() );
    }
    */
    else if ( input_file_extension == ".vtk" )
    {
        StructuredPoints2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path.string() );
    }
    else if ( input_file_extension == ".vti" )
    {
        Vti2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path.string() );
    }
    else if ( input_file_extension == ".vts" )
    {
        Vts2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path.string() );
    }
    else if ( input_file_extension == ".pvts" )
    {
        Pvts2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path.string() );
    }

    else if ( input_file_extension == ".inp" )
    {
        AvsUcd2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path.string() );
    }
    else if ( input_file_extension == ".nc" )
    {
        Netcdf2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path.string() );
    }
    else if ( input_file_extension == ".vtu" )
    {
        cvt::VtkXmlUnstructuredGrid input_vtu( input_file_path.string() );

        if ( input_vtu.isPointObjectConvertible() )
        {
            std::string separator(1, fs::path::preferred_separator);
            std::string output_file_path = output_directory_path.string() + separator + input_filename_without_extension.string() + ".kvsml";
            PointVtu2Kvsml( output_file_path, input_file_path.string() );
        }
        else if ( input_vtu.isLineObjectConvertible() )
        {
            std::string separator(1, fs::path::preferred_separator);
            std::string output_file_path = output_directory_path.string() + separator + input_filename_without_extension.string() + ".kvsml";
            LineVtu2Kvsml( output_file_path, input_file_path.string() );
        }
        else if ( input_vtu.isPolygonObjectConvertible() )
        {
            std::string separator(1, fs::path::preferred_separator);
            std::string output_file_path = output_directory_path.string() + separator + input_filename_without_extension.string() + ".kvsml";
            TriangleVtu2Kvsml( output_file_path, input_file_path.string() );
        }
        else
        {
            Vtu2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path.string() );
        }
    }
    else if ( input_file_extension == ".pvtu" )
    {
        Pvtu2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path.string() );
    }
    else if ( input_file_extension == ".vtm" )
    {
        Vtm2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path.string() );
    }
    else if ( input_file_extension == ".case" )
    {
        if ( contains_wildcard( input_file_path.string() ) )
        {
            std::cout << ".case does not yet support a wildcard." << std::endl;
        }
        else
        {
            Case2Kvsml( output_directory_path.string(), input_filename_without_extension.string(), input_file_path.string() );
        }
    }
    else
    {
        std::cout << "This file extension is not yet supported" << std::endl;
    }
    
    return 0;
}

bool contains_wildcard( const std::string& path )
{
    const std::string wildcard_chars = "*";
    return path.find_first_of(wildcard_chars) != std::string::npos;
}
