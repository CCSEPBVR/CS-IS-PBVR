/*
 * Copyright (c) 2022 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "AvsField.h"

#include <regex>
#include <string>

#include "kvs/Message"
#include <vtkAOSDataArrayTemplate.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredGrid.h>
#include <vtkUniformGrid.h>

#include "FileFormat/VTK/VtkXmlImageData.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "Filesystem.h"

namespace
{

std::string RemoveExcessWhitespace( const std::string& line )
{
    auto l0 = std::regex_replace( line, std::regex( "[ \\t]+" ), " " );
    auto l1 = std::regex_replace( l0, std::regex( "^[ \\t]+" ), "" );
    auto l2 = std::regex_replace( l1, std::regex( "[ \\t]+$" ), "" );

    return l2;
}

bool ReadLine( char* dst, std::ifstream& stream, std::size_t n )
{
    std::size_t h = 0;
    bool has_separator = false;

    while ( stream.good() )
    {
        char c;
        stream.read( &c, sizeof( char ) );

        if ( c == '\n' || c == '\r' )
        {
            auto current = stream.tellg();
            stream.read( &c, sizeof( char ) );

            // check \n\r
            if ( c != '\r' )
            {
                stream.seekg( current );
            }

            break;
        }
        else if ( c == 0x0C )
        {
            // skip ^L^L
            stream.read( &c, sizeof( char ) );
            has_separator = true;
            break;
        }
        else
        {
            if ( h < n - 1 )
            {
                dst[h++] = c;
            }
        }
    }
    dst[h] = 0;

    return has_separator;
}

bool SetHeader( cvt::detail::AvsFieldHeader& header, const std::string& line )
{
    auto pos = line.find( "=" );

    if ( pos == std::string::npos )
    {
        return false;
    }
    auto t = line.substr( 0, pos );
    auto v = line.substr( pos + 1 );

    if ( std::regex_match( line, std::regex( "^#" ) ) )
    {
        // comment
        return true;
    }
    else if ( std::regex_match( t, std::regex( "nstep" ) ) )
    {
        header.nstep = std::stoi( v );
    }
    else if ( std::regex_match( t, std::regex( "ndim" ) ) )
    {
        header.ndim = std::stoi( v );
    }
    else if ( std::regex_match( t, std::regex( "dim1" ) ) )
    {
        header.dim1 = std::stoi( v );
    }
    else if ( std::regex_match( t, std::regex( "dim2" ) ) )
    {
        header.dim2 = std::stoi( v );
    }
    else if ( std::regex_match( t, std::regex( "dim3" ) ) )
    {
        header.dim3 = std::stoi( v );
    }
    else if ( std::regex_match( t, std::regex( "nspace" ) ) )
    {
        header.nspace = std::stoi( v );
    }
    else if ( std::regex_match( t, std::regex( "veclen" ) ) )
    {
        header.veclen = std::stoi( v );
    }
    else if ( std::regex_match( t, std::regex( "data" ) ) )
    {
        header.data = v;
    }
    else if ( std::regex_match( t, std::regex( "field" ) ) )
    {
        header.field = v;
    }
    else if ( std::regex_match( t, std::regex( "label" ) ) )
    {
        // ignore
    }
    else if ( std::regex_match( t, std::regex( "unit" ) ) )
    {
        // ignore
    }
    else if ( std::regex_match( t, std::regex( "min_val" ) ) )
    {
        // ignore
    }
    else if ( std::regex_match( t, std::regex( "max_val" ) ) )
    {
        // ignore
    }
    else if ( std::regex_match( t, std::regex( "min_ext" ) ) )
    {
        // ignore
    }
    else if ( std::regex_match( t, std::regex( "max_ext" ) ) )
    {
        // ignore
    }
    else
    {
        // Maybe non-header line
        return false;
    }

    return true;
}

std::shared_ptr<cvt::detail::AvsFieldDataMeta> GetTimeDependentFiled( const std::string& line )
{
    auto meta = std::make_shared<cvt::detail::AvsFieldDataMeta>();

    std::stringstream ss( line );
    std::string block;

    int n = 0;
    while ( std::getline( ss, block, ' ' ) )
    {
        switch ( n )
        {
        case 0:
            meta->type = block;
            if ( std::regex_match( block, std::regex( "time" ) ) )
            {
                n = 2;
            }
            else
            {
                ++n;
            }
            break;
        case 1:
            if ( std::regex_match( line, std::regex( "value" ) ) )
            {
                auto pos = block.find( "=" );

                if ( pos != std::string::npos )
                {
                    auto k = block.substr( 0, pos );
                    auto v = block.substr( pos + 1 );
                    meta->value = std::make_shared<std::string>( v );
                }
            }
            else
            {

                meta->n = std::stoi( block );
            }
            ++n;
            break;
        default:
            auto pos = block.find( "=" );

            if ( pos != std::string::npos )
            {
                auto k = block.substr( 0, pos );
                auto v = block.substr( pos + 1 );

                if ( k == "file" )
                {
                    meta->file = std::make_shared<std::string>( v );
                }
                else if ( k == "filetype" )
                {
                    meta->filetype = v;
                }
                else if ( k == "skip" )
                {
                    meta->skip = std::stoi( v );
                }
                else if ( k == "offset" )
                {
                    meta->offset = std::stoi( v );
                }
                else if ( k == "stride" )
                {
                    meta->stride = std::stoi( v );
                }
                else if ( k == "close" )
                {
                    meta->close = std::stoi( v );
                }
            }
        }
    }

    return meta;
}

std::shared_ptr<std::string> WriteFldDataField( std::ifstream& in )
{
    std::string dst_name = std::tmpnam( nullptr );
    std::ofstream dst( dst_name, std::ios::binary );

    while ( in.good() )
    {
        char c;
        in.read( &c, sizeof( char ) );

        dst.write( &c, 1 );
    }

    return std::make_shared<std::string>( dst_name );
}

bool ReadFirstAvsLine( std::ifstream& stream )
{
    constexpr std::size_t LINE_LENGTH = 512;
    char line[LINE_LENGTH];
    ::ReadLine( line, stream, LINE_LENGTH );

    return std::regex_match( line, std::regex( "# AVS.*" ) );
}

std::shared_ptr<std::string> ReadHeader( cvt::detail::AvsFieldHeader& header,
                                         std::ifstream& stream )
{
    constexpr std::size_t LINE_LENGTH = 512;
    char line[LINE_LENGTH];
    bool has_separator = false;

    while ( stream.good() )
    {
        auto old = stream.tellg();
        has_separator = ::ReadLine( line, stream, LINE_LENGTH );

        if ( !has_separator )
        {
            std::string shrunk = ::RemoveExcessWhitespace( line );
            if ( !::SetHeader( header, shrunk ) )
            {
                stream.seekg( old );
                // end of a header section
                break;
            }
        }
        else
        {
            return ::WriteFldDataField( stream );
        }
    }

    return nullptr;
}

std::shared_ptr<std::string> ReadTimeDependField(
    std::vector<cvt::detail::AvsFieldTimeDependentField>& tdf, std::ifstream& stream, int veclen,
    int nstep )
{
    constexpr std::size_t LINE_LENGTH = 512;
    std::shared_ptr<std::string> data_field = nullptr;

    char line[LINE_LENGTH];

    int time_step = 0;
    tdf[time_step].SetVeclen( veclen );
    bool has_do_loop = false;
    bool has_separator = false;

    while ( stream.good() )
    {
        has_separator = ::ReadLine( line, stream, LINE_LENGTH );
        std::string shrunk = ::RemoveExcessWhitespace( line );

        if ( shrunk[0] == '#' )
        {
            // comment. ignore
        }
        else if ( shrunk == "EOT" )
        {
            tdf[++time_step].SetVeclen( veclen );
        }
        else if ( shrunk == "DO" )
        {
            has_do_loop = true;
            tdf[time_step] = tdf[time_step - 1];
        }
        else if ( shrunk == "ENDDO" )
        {
            /* ignore */
        }
        else
        {
            auto meta = ::GetTimeDependentFiled( shrunk );

            if ( meta->type == "coord" )
            {
                tdf[time_step].coords[meta->n - 1] = meta;
            }
            else if ( meta->type == "variable" )
            {
                tdf[time_step].variables[meta->n - 1] = meta;
            }
            else if ( meta->type == "time" )
            {
                tdf[time_step].time = meta;
            }
        }

        if ( has_separator )
        {
            data_field = ::WriteFldDataField( stream );
            break;
        }
    }

    if ( has_do_loop )
    {
        auto last = time_step - 1;
        for ( ; time_step < nstep; ++time_step )
        {
            tdf[time_step] = tdf[last];
        }
    }

    return data_field;
}

template <typename T>
void ReadBinaryDataFile( T* dst, int count, const std::string& filename,
                         const std::deque<int>& skips, int stride )
{
    std::ifstream stream( filename, std::ios::binary );
    T c;

    for ( int i = 0; i < skips.size(); ++i )
    {
        for ( int j = 0; j < skips[i] && stream.good(); ++j )
        {
            char x;
            stream.read( &x, sizeof( char ) );
        }

        int p = 0;
        for ( int j = 0; j < count * stride && stream.good(); ++j )
        {
            stream.read( reinterpret_cast<char*>( &c ), sizeof( T ) );
            if ( j % stride == 0 )
            {
                if ( p < count )
                {
                    dst[p++] = c;
                }
            }
        }
    }
}

template <typename T>
T from_string( const std::string& str )
{
    return static_cast<T>( std::stoi( str ) );
}

template <>
float from_string<float>( const std::string& str )
{
    return std::stof( str );
}

template <>
double from_string<double>( const std::string& str )
{
    return std::stod( str );
}

template <typename T>
void ReadAsciiDataFile( T* dst, int count, const std::string& filename,
                        const std::deque<int>& skips, const std::deque<int>& offsets, int stride )
{
    std::ifstream stream( filename );
    std::string str;

    for ( int i = 0; i < skips.size(); ++i )
    {
        for ( int j = 0; j < skips[i] && std::getline( stream, str ); ++j )
        {
        }
        for ( int j = 0; j < offsets[i] && ( stream >> str ); ++j )
        {
        }

        int p = 0;
        for ( int j = 0; j < ( count * stride - offsets[i] ) && ( stream >> str ); ++j )
        {
            T v = 0;
            try
            {
                v = ::from_string<T>( str );
            }
            catch ( ... )
            {
                std::string message =
                    "Failed to convert '" + str + "' from the ascii data. Set the component to 0.";
                kvsMessageWarning( message.c_str() );
            }
            if ( j % stride == 0 )
            {
                if ( p < count )
                {
                    dst[p++] = v;
                }
            }
        }
        std::getline( stream, str );
    }
}

template <typename T>
void SetIrregularPoints(
    vtkPoints* points, int n, const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field,
    std::filesystem::path dirname )
{
    int number_of_points = header.dim1 * header.dim2 * header.dim3;
    std::vector<std::vector<T>> data_array( 3 );
    for ( auto& a : data_array )
    {
        a.resize( number_of_points );
        std::fill( a.begin(), a.end(), 0 );
    }

    points->SetNumberOfPoints( number_of_points );

    for ( int c = 0; c < header.nspace; ++c )
    {
        cvt::detail::AvsFieldDataMeta* meta = time_dependent_field[0].coords[c].get();
        std::deque<int> skips;
        std::deque<int> offsets;
        skips.push_back( meta->skip );
        offsets.push_back( meta->offset );

        for ( int i = 1; i <= n; ++i )
        {
            if ( time_dependent_field[i].coords[c] )
            {
                meta = time_dependent_field[i].coords[c].get();
            }

            if ( meta->close == 1 )
            {
                skips.clear();
                offsets.clear();
            }

            skips.push_back( meta->skip );
            offsets.push_back( meta->offset );
        }

        if ( meta->file )
        {
            std::filesystem::path file_path;
            if ( meta->is_relative )
            {
                file_path = dirname;
                file_path /= *( meta->file );
            }
            else
            {
                file_path = *( meta->file );
            }

            if ( meta->filetype == "binary" )
            {
                ::ReadBinaryDataFile( data_array[c].data(), number_of_points, file_path.string(),
                                      skips, meta->stride );
            }
            else
            {
                ::ReadAsciiDataFile( data_array[c].data(), number_of_points, file_path.string(),
                                     skips, offsets, meta->stride );
            }
        }
    }

    for ( int i = 0; i < number_of_points; ++i )
    {
        points->SetPoint( i, data_array[0][i], data_array[1][i], data_array[2][i] );
    }
}

template <typename T>
void SetRectilinearPoints(
    vtkRectilinearGrid* grid, int n, const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field,
    std::filesystem::path dirname )
{
    vtkSmartPointer<vtkAOSDataArrayTemplate<T>> data_array[3];
    int dim[3] = { header.dim1, header.dim2, header.dim3 };
    for ( auto& d : data_array )
    {
        d = vtkSmartPointer<vtkAOSDataArrayTemplate<T>>::New();
    }
    grid->SetDimensions( header.dim1, header.dim2, header.dim3 );

    for ( int c = 0; c < header.nspace; ++c )
    {
        data_array[c]->SetNumberOfTuples( dim[c] );
        data_array[c]->SetNumberOfComponents( 1 );
        data_array[c]->Fill( 0 );

        cvt::detail::AvsFieldDataMeta* meta = time_dependent_field[0].coords[c].get();
        std::deque<int> skips;
        std::deque<int> offsets;
        skips.push_back( meta->skip );
        offsets.push_back( meta->offset );

        for ( int i = 1; i <= n; ++i )
        {
            if ( time_dependent_field[i].coords[c] )
            {
                meta = time_dependent_field[i].coords[c].get();
            }

            if ( meta->close == 1 )
            {
                skips.clear();
                offsets.clear();
            }

            skips.push_back( meta->skip );
            offsets.push_back( meta->offset );
        }

        if ( meta->file )
        {
            std::filesystem::path file_path;
            if ( meta->is_relative )
            {
                file_path = dirname;
                file_path /= *( meta->file );
            }
            else
            {
                file_path = *( meta->file );
            }

            if ( meta->filetype == "binary" )
            {
                ::ReadBinaryDataFile( data_array[c]->GetPointer( 0 ), dim[c], file_path.string(),
                                      skips, meta->stride );
            }
            else
            {
                ::ReadAsciiDataFile( data_array[c]->GetPointer( 0 ), dim[c], file_path.string(),
                                     skips, offsets, meta->stride );
            }
        }
    }

    grid->SetXCoordinates( data_array[0] );
    grid->SetYCoordinates( data_array[1] );
    grid->SetZCoordinates( data_array[2] );
}

template <typename T>
void SetPointData( vtkPointData* point_data, int n, const cvt::detail::AvsFieldHeader& header,
                   const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field,
                   std::filesystem::path dirname )
{
    int number_of_points = header.dim1 * header.dim2 * header.dim3;

    for ( int c = 0; c < header.veclen; ++c )
    {
        vtkNew<vtkAOSDataArrayTemplate<T>> data_array;
        data_array->SetNumberOfTuples( number_of_points );
        data_array->SetNumberOfComponents( 1 );
        std::string array_name = "variable_" + std::to_string( c );
        data_array->SetName( array_name.c_str() );

        cvt::detail::AvsFieldDataMeta* meta = time_dependent_field[0].variables[c].get();
        std::deque<int> skips;
        std::deque<int> offsets;
        skips.push_back( meta->skip );
        offsets.push_back( meta->offset );

        for ( int i = 1; i <= n; ++i )
        {
            if ( time_dependent_field[i].variables[c] )
            {
                meta = time_dependent_field[i].variables[c].get();
            }

            if ( meta->close == 1 )
            {
                skips.clear();
                offsets.clear();
            }

            skips.push_back( meta->skip );
            offsets.push_back( meta->offset );
        }

        if ( meta->file )
        {
            std::filesystem::path file_path;
            if ( meta->is_relative )
            {
                file_path = dirname;
                file_path /= *( meta->file );
            }
            else
            {
                file_path = *( meta->file );
            }

            if ( meta->filetype == "binary" )
            {
                ::ReadBinaryDataFile( data_array->GetPointer( 0 ), number_of_points,
                                      file_path.string(), skips, meta->stride );
            }
            else
            {
                ::ReadAsciiDataFile( data_array->GetPointer( 0 ), number_of_points,
                                     file_path.string(), skips, offsets, meta->stride );
            }

            point_data->AddArray( data_array );
        }
    }
}
} // namespace

bool cvt::AvsField::read( const std::string& filename )
{
    setSuccess( false );
    std::filesystem::path f( filename );
    dirname = f.parent_path();

    try
    {
        std::ifstream fld( filename, std::ios_base::in | std::ios_base::binary );
        ::ReadFirstAvsLine( fld );
        if ( auto fp = ::ReadHeader( header, fld ) )
        {
            header.nstep = 1;
            time_dependent_field.resize( 1 );
            for ( int i = 0; i < header.veclen; ++i )
            {
                auto c = std::make_shared<cvt::detail::AvsFieldDataMeta>();

                c->type = "variable";
                c->n = i + 1;
                c->file = fp;
                c->is_relative = false;
                c->filetype = "binary";
                c->skip = 0;
                c->offset = i;
                c->stride = header.veclen;
                c->close = 0;

                time_dependent_field[0].variables[i] = c;
            }
            for ( int i = 0; i < header.ndim; ++i )
            {
                auto c = std::make_shared<cvt::detail::AvsFieldDataMeta>();

                c->type = "coord";
                c->n = i + 1;
                c->file = fp;
                c->is_relative = false;
                c->filetype = "binary";
                c->skip = header.veclen * header.dim1 * header.dim2 * header.dim3;
                c->offset = i;
                c->stride = header.ndim;
                c->close = 0;

                time_dependent_field[0].coords[i] = c;
            }
        }
        else
        {
            time_dependent_field.resize( header.nstep );

            if ( auto fp = ::ReadTimeDependField( time_dependent_field, fld, header.veclen,
                                                  header.nstep ) )
            {
                for ( auto& tdf : time_dependent_field )
                {
                    for ( auto& c : tdf.coords )
                    {
                        if ( c )
                        {
                            c->file = fp;
                            c->is_relative = false;
                        }
                    }
                    for ( auto& v : tdf.variables )
                    {
                        if ( v )
                        {
                            v->file = fp;
                            v->is_relative = false;
                        }
                    }
                }
            }
        }

        setSuccess( true );
        return true;
    }
    catch ( std::exception& e )
    {
        kvsMessageError( e.what() );
        return false;
    }
}

cvt::VtkXmlStructuredGrid cvt::detail::GetVtkDataFromIrregularAvsField(
    int n, std::filesystem::path dirname, const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field )
{
    auto grid = vtkSmartPointer<vtkStructuredGrid>::New();

    if ( header.field == "irregular" )
    {
        vtkNew<vtkPoints> points;

        grid->SetDimensions( header.dim1, header.dim2, ( header.nspace == 3 ) ? header.dim3 : 1 );

        if ( header.data == "float" )
        {
            points->SetDataTypeToFloat();
            ::SetIrregularPoints<float>( points, n, header, time_dependent_field, dirname );
            ::SetPointData<float>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }
        else if ( header.data == "double" )
        {
            points->SetDataTypeToDouble();
            ::SetIrregularPoints<double>( points, n, header, time_dependent_field, dirname );
            ::SetPointData<double>( grid->GetPointData(), n, header, time_dependent_field,
                                    dirname );
        }
        else if ( header.data == "integer" )
        {
            points->SetDataTypeToInt();
            ::SetIrregularPoints<int>( points, n, header, time_dependent_field, dirname );
            ::SetPointData<int>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }
        else if ( header.data == "short" )
        {
            points->SetDataTypeToShort();
            ::SetIrregularPoints<short>( points, n, header, time_dependent_field, dirname );
            ::SetPointData<short>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }
        else if ( header.data == "byte" )
        {
            points->SetDataTypeToChar();
            ::SetIrregularPoints<char>( points, n, header, time_dependent_field, dirname );
            ::SetPointData<char>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }

        grid->SetPoints( points );
    }

    return cvt::VtkXmlStructuredGrid( grid );
}

cvt::VtkXmlImageData cvt::detail::GetVtkDataFromUniformAvsField(
    int n, std::filesystem::path dirname, const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field )
{
    auto grid = vtkSmartPointer<vtkImageData>::New();

    if ( header.field == "uniform" )
    {
        vtkNew<vtkPoints> points;

        grid->SetDimensions( header.dim1, header.dim2, ( header.nspace == 3 ) ? header.dim3 : 1 );

        if ( header.data == "float" )
        {
            ::SetPointData<float>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }
        else if ( header.data == "double" )
        {
            ::SetPointData<double>( grid->GetPointData(), n, header, time_dependent_field,
                                    dirname );
        }
        else if ( header.data == "integer" )
        {
            ::SetPointData<int>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }
        else if ( header.data == "short" )
        {
            ::SetPointData<short>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }
        else if ( header.data == "byte" )
        {
            ::SetPointData<char>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }
    }

    return cvt::VtkXmlImageData( grid );
}

cvt::VtkXmlRectilinearGrid cvt::detail::GetVtkDataFromRectilinearAvsField(
    int n, std::filesystem::path dirname, const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field )
{
    auto grid = vtkSmartPointer<vtkRectilinearGrid>::New();

    if ( header.field == "rectilinear" )
    {
        vtkNew<vtkPoints> points;

        grid->SetDimensions( header.dim1, header.dim2, ( header.nspace == 3 ) ? header.dim3 : 1 );

        if ( header.data == "float" )
        {
            ::SetRectilinearPoints<float>( grid, n, header, time_dependent_field, dirname );
            ::SetPointData<float>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }
        else if ( header.data == "double" )
        {
            ::SetRectilinearPoints<double>( grid, n, header, time_dependent_field, dirname );
            ::SetPointData<double>( grid->GetPointData(), n, header, time_dependent_field,
                                    dirname );
        }
        else if ( header.data == "integer" )
        {
            ::SetRectilinearPoints<int>( grid, n, header, time_dependent_field, dirname );
            ::SetPointData<int>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }
        else if ( header.data == "short" )
        {
            ::SetRectilinearPoints<short>( grid, n, header, time_dependent_field, dirname );
            ::SetPointData<short>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }
        else if ( header.data == "byte" )
        {
            ::SetRectilinearPoints<char>( grid, n, header, time_dependent_field, dirname );
            ::SetPointData<char>( grid->GetPointData(), n, header, time_dependent_field, dirname );
        }
    }

    return cvt::VtkXmlRectilinearGrid( grid );
}
