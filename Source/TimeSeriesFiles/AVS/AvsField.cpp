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
            ++n;
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

bool ReadFirstAvsLine( std::ifstream& stream )
{
    constexpr std::size_t LINE_LENGTH = 512;
    char line[LINE_LENGTH];
    ::ReadLine( line, stream, LINE_LENGTH );

    return std::regex_match( line, std::regex( "# AVS.*" ) );
}

bool ReadHeader( cvt::detail::AvsFieldHeader& header, std::ifstream& stream )
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
            return has_separator;
        }
    }

    return has_separator;
}

bool ReadTimeDependField( std::vector<cvt::detail::AvsFieldTimeDependentField>& tdf,
                          std::ifstream& stream, int veclen, int nstep )
{
    constexpr std::size_t LINE_LENGTH = 512;
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
        }
        else if ( shrunk == "ENDDO" )
        {
            break;
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

    return has_separator;
}

template <typename T>
void ReadBinaryDataFile( T* dst, int count, const std::string& filename, int skip, int stride )
{
    std::ifstream stream( filename, std::ios::binary );
    T c;

    for ( int i = 0; stream.good() && i < skip; ++i )
    {
        char x;
        stream.read( &x, sizeof( char ) );
    }

    int p = 0;
    for ( int x = 0; p < count && stream.good(); ++x )
    {
        stream.read( reinterpret_cast<char*>( &c ), sizeof( T ) );
        if ( x % stride == 0 )
        {
            dst[p++] = c;
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
void ReadAsciiDataFile( T* dst, int count, const std::string& filename, int skip, int offset,
                        int stride )
{
    std::ifstream stream( filename );
    std::string str;

    for ( int i = 0; i < skip && std::getline( stream, str ); ++i )
    {
    }
    for ( int i = 0; i < offset && stream >> str; ++i )
    {
    }

    int p = 0;
    int x = stride;

    for ( ; p < count && stream >> str; )
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

        if ( x == stride )
        {
            dst[p++] = v;
            x = 1;
        }
        else
        {
            ++x;
        }
    }
}

template <typename T>
void SetIrregularPoints(
    vtkPoints* points, int n, bool has_data_section, const cvt::detail::AvsFieldHeader& header,
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

    if ( has_data_section )
    {
    }
    else
    {
        for ( int c = 0; c < header.nspace; ++c )
        {
            cvt::detail::AvsFieldDataMeta* meta = time_dependent_field[0].coords[c].get();
            int skip = meta->skip;
            int offset = meta->offset;

            for ( int i = 1; i <= n; ++i )
            {
                if ( time_dependent_field[i].coords[c] )
                {
                    meta = time_dependent_field[i].coords[c].get();

                    if ( meta->filetype == "binary" )
                    {
                        skip = ( meta->close == 0 )
                                   ? ( skip + number_of_points * sizeof( T ) * meta->stride )
                                   : meta->skip;
                    }
                    else
                    {
                        offset = ( meta->close == 0 ) ? ( offset + number_of_points * meta->stride )
                                                      : meta->offset;
                    }
                }
            }

            if ( meta->file )
            {
                std::filesystem::path file_path = dirname;
                file_path /= *( meta->file );

                if ( meta->filetype == "binary" )
                {
                    ::ReadBinaryDataFile( data_array[c].data(), number_of_points,
                                          file_path.string(), skip, meta->stride );
                }
                else
                {
                    ::ReadAsciiDataFile( data_array[c].data(), number_of_points, file_path.string(),
                                         meta->skip, offset, meta->stride );
                }
            }
        }

        for ( int i = 0; i < number_of_points; ++i )
        {
            points->SetPoint( i, data_array[0][i], data_array[1][i], data_array[2][i] );
        }
    }
}

template <typename T>
void SetRectilinearPoints(
    vtkRectilinearGrid* grid, int n, bool has_data_section,
    const cvt::detail::AvsFieldHeader& header,
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

    if ( has_data_section )
    {
        kvsMessageError( "A native filed input was unsupported." );
    }
    else
    {
        for ( int c = 0; c < header.nspace; ++c )
        {
            data_array[c]->SetNumberOfTuples( dim[c] );
            data_array[c]->SetNumberOfComponents( 1 );
            data_array[c]->Fill( 0 );

            cvt::detail::AvsFieldDataMeta* meta = time_dependent_field[0].coords[c].get();
            int skip = meta->skip;
            int offset = meta->offset;

            for ( int i = 1; i <= n; ++i )
            {
                if ( time_dependent_field[i].coords[c] )
                {
                    meta = time_dependent_field[i].coords[c].get();

                    if ( meta->filetype == "binary" )
                    {
                        skip = ( meta->close == 0 ) ? ( skip + dim[c] * sizeof( T ) * meta->stride )
                                                    : meta->skip;
                    }
                    else
                    {
                        offset = ( meta->close == 0 ) ? ( offset + dim[c] * meta->stride )
                                                      : meta->offset;
                    }
                }
            }

            if ( meta->file )
            {
                std::filesystem::path file_path = dirname;
                file_path /= *( meta->file );

                if ( meta->filetype == "binary" )
                {
                    ::ReadBinaryDataFile( data_array[c]->GetPointer( 0 ), dim[c],
                                          file_path.string(), skip, meta->stride );
                }
                else
                {
                    ::ReadAsciiDataFile( data_array[c]->GetPointer( 0 ), dim[c], file_path.string(),
                                         meta->skip, offset, meta->stride );
                }
            }
        }

        grid->SetXCoordinates( data_array[0] );
        grid->SetYCoordinates( data_array[1] );
        grid->SetZCoordinates( data_array[2] );
    }
}

template <typename T>
void SetPointData( vtkPointData* point_data, int n, bool has_data_section,
                   const cvt::detail::AvsFieldHeader& header,
                   const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field,
                   std::filesystem::path dirname )
{
    int number_of_points = header.dim1 * header.dim2 * header.dim3;

    if ( has_data_section )
    {
        kvsMessageError( "A native filed input was unsupported." );
    }
    else
    {
        for ( int c = 0; c < header.veclen; ++c )
        {
            vtkNew<vtkAOSDataArrayTemplate<T>> data_array;
            data_array->SetNumberOfTuples( number_of_points );
            data_array->SetNumberOfComponents( 1 );
            std::string array_name = "variable_" + std::to_string( c );
            data_array->SetName( array_name.c_str() );

            cvt::detail::AvsFieldDataMeta* meta = time_dependent_field[0].variables[c].get();
            int skip = meta->skip;
            int offset = meta->offset;

            for ( int i = 1; i <= n; ++i )
            {
                if ( time_dependent_field[i].variables[c] )
                {
                    meta = time_dependent_field[i].variables[c].get();

                    if ( meta->filetype == "binary" )
                    {
                        skip = ( meta->close == 0 )
                                   ? ( skip + number_of_points * sizeof( T ) * meta->stride )
                                   : meta->skip;
                    }
                    else
                    {
                        offset = ( meta->close == 0 ) ? ( offset + number_of_points * meta->stride )
                                                      : meta->offset;
                    }
                }
            }

            if ( meta->file )
            {
                std::filesystem::path file_path = dirname;
                file_path /= *( meta->file );

                if ( meta->filetype == "binary" )
                {
                    ::ReadBinaryDataFile( data_array->GetPointer( 0 ), number_of_points,
                                          file_path.string(), skip, meta->stride );
                }
                else
                {
                    ::ReadAsciiDataFile( data_array->GetPointer( 0 ), number_of_points,
                                         file_path.string(), meta->skip, offset, meta->stride );
                }

                point_data->AddArray( data_array );
            }
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
        fld =
            std::make_shared<std::ifstream>( filename, std::ios_base::in | std::ios_base::binary );
        has_data_section = false;
        ::ReadFirstAvsLine( *fld );
        if ( ::ReadHeader( header, *fld ) )
        {
            header.nstep = 1;
            has_data_section = true;
            data_section_head = fld->tellg();
        }
        else
        {
            time_dependent_field.resize( header.nstep );

            if ( ::ReadTimeDependField( time_dependent_field, *fld, header.veclen, header.nstep ) )
            {
                has_data_section = true;
                data_section_head = fld->tellg();
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
    int n, std::filesystem::path dirname, bool has_data_section,
    const cvt::detail::AvsFieldHeader& header,
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
            ::SetIrregularPoints<float>( points, n, has_data_section, header, time_dependent_field,
                                         dirname );
            ::SetPointData<float>( grid->GetPointData(), n, has_data_section, header,
                                   time_dependent_field, dirname );
        }
        else if ( header.data == "double" )
        {
            points->SetDataTypeToDouble();
            ::SetIrregularPoints<double>( points, n, has_data_section, header, time_dependent_field,
                                          dirname );
            ::SetPointData<double>( grid->GetPointData(), n, has_data_section, header,
                                    time_dependent_field, dirname );
        }
        else if ( header.data == "integer" )
        {
            points->SetDataTypeToInt();
            ::SetIrregularPoints<int>( points, n, has_data_section, header, time_dependent_field,
                                       dirname );
            ::SetPointData<int>( grid->GetPointData(), n, has_data_section, header,
                                 time_dependent_field, dirname );
        }
        else if ( header.data == "short" )
        {
            points->SetDataTypeToShort();
            ::SetIrregularPoints<short>( points, n, has_data_section, header, time_dependent_field,
                                         dirname );
            ::SetPointData<short>( grid->GetPointData(), n, has_data_section, header,
                                   time_dependent_field, dirname );
        }
        else if ( header.data == "byte" )
        {
            points->SetDataTypeToChar();
            ::SetIrregularPoints<char>( points, n, has_data_section, header, time_dependent_field,
                                        dirname );
            ::SetPointData<char>( grid->GetPointData(), n, has_data_section, header,
                                  time_dependent_field, dirname );
        }

        grid->SetPoints( points );
    }

    return cvt::VtkXmlStructuredGrid( grid );
}

cvt::VtkXmlImageData cvt::detail::GetVtkDataFromUniformAvsField(
    int n, std::filesystem::path dirname, bool has_data_section,
    const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field )
{
    auto grid = vtkSmartPointer<vtkImageData>::New();

    if ( header.field == "uniform" )
    {
        vtkNew<vtkPoints> points;

        grid->SetDimensions( header.dim1, header.dim2, ( header.nspace == 3 ) ? header.dim3 : 1 );

        if ( header.data == "float" )
        {
            ::SetPointData<float>( grid->GetPointData(), n, has_data_section, header,
                                   time_dependent_field, dirname );
        }
        else if ( header.data == "double" )
        {
            ::SetPointData<double>( grid->GetPointData(), n, has_data_section, header,
                                    time_dependent_field, dirname );
        }
        else if ( header.data == "integer" )
        {
            ::SetPointData<int>( grid->GetPointData(), n, has_data_section, header,
                                 time_dependent_field, dirname );
        }
        else if ( header.data == "short" )
        {
            ::SetPointData<short>( grid->GetPointData(), n, has_data_section, header,
                                   time_dependent_field, dirname );
        }
        else if ( header.data == "byte" )
        {
            ::SetPointData<char>( grid->GetPointData(), n, has_data_section, header,
                                  time_dependent_field, dirname );
        }
    }

    return cvt::VtkXmlImageData( grid );
}

cvt::VtkXmlRectilinearGrid cvt::detail::GetVtkDataFromRectilinearAvsField(
    int n, std::filesystem::path dirname, bool has_data_section,
    const cvt::detail::AvsFieldHeader& header,
    const std::vector<cvt::detail::AvsFieldTimeDependentField>& time_dependent_field )
{
    auto grid = vtkSmartPointer<vtkRectilinearGrid>::New();

    if ( header.field == "rectilinear" )
    {
        vtkNew<vtkPoints> points;

        grid->SetDimensions( header.dim1, header.dim2, ( header.nspace == 3 ) ? header.dim3 : 1 );

        if ( header.data == "float" )
        {
            ::SetRectilinearPoints<float>( grid, n, has_data_section, header, time_dependent_field,
                                           dirname );
            ::SetPointData<float>( grid->GetPointData(), n, has_data_section, header,
                                   time_dependent_field, dirname );
        }
        else if ( header.data == "double" )
        {
            ::SetRectilinearPoints<double>( grid, n, has_data_section, header, time_dependent_field,
                                            dirname );
            ::SetPointData<double>( grid->GetPointData(), n, has_data_section, header,
                                    time_dependent_field, dirname );
        }
        else if ( header.data == "integer" )
        {
            ::SetRectilinearPoints<int>( grid, n, has_data_section, header, time_dependent_field,
                                         dirname );
            ::SetPointData<int>( grid->GetPointData(), n, has_data_section, header,
                                 time_dependent_field, dirname );
        }
        else if ( header.data == "short" )
        {
            ::SetRectilinearPoints<short>( grid, n, has_data_section, header, time_dependent_field,
                                           dirname );
            ::SetPointData<short>( grid->GetPointData(), n, has_data_section, header,
                                   time_dependent_field, dirname );
        }
        else if ( header.data == "byte" )
        {
            ::SetRectilinearPoints<char>( grid, n, has_data_section, header, time_dependent_field,
                                          dirname );
            ::SetPointData<char>( grid->GetPointData(), n, has_data_section, header,
                                  time_dependent_field, dirname );
        }
    }

    return cvt::VtkXmlRectilinearGrid( grid );
}
