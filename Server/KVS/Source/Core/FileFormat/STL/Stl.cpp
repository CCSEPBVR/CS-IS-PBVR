/*****************************************************************************/
/**
 *  @file   Stl.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Stl.cpp 842 2011-06-10 07:48:28Z naohisa.sakamoto@gmail.com $
 */
/*****************************************************************************/
#include "Stl.h"
#include <kvs/File>


namespace
{

const int MaxLineLength = 256;

const char* const Delimiter = " \t\n\r";

const std::string FileTypeToString[2] =
{
    "ascii",
    "binary"
};

}

namespace
{

const bool IsAsciiType( FILE* ifs )
{
    // Go back file-pointer to head.
    fseek( ifs, 0, SEEK_SET );

    // Check file type.
    char buffer[ ::MaxLineLength ];
    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        if ( buffer[0] == '\n' ) continue;

        const char* head = strtok( buffer, ::Delimiter );
        if ( !strcmp( head, "solid" ) ) break;
        else if ( !strcmp( head, "facet" ) ) return( true );
        else return( false );
    }

    size_t counter = 0;
    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        if ( counter > 5 ) return( false );
        if ( buffer[0] == '\n' ) continue;
        if ( strstr( buffer, "endsolid" ) ) return( true );
        if ( strstr( buffer, "facet" ) ) return( true );
        counter++;
    }

    return( false );
}

} // end of namespace

namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new STL class.
 */
/*===========================================================================*/
Stl::Stl( void ):
    m_file_type( Stl::Ascii )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new STL class and reads a file.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
Stl::Stl( const std::string& filename ):
    m_file_type( Stl::Ascii )
{
    if( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*===========================================================================*/
/**
 *  @brief  Destructs the STL class.
 */
/*===========================================================================*/
Stl::~Stl( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns a file type.
 *  @return file type
 */
/*===========================================================================*/
const Stl::FileType Stl::fileType( void ) const
{
    return( m_file_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns a normal vector array.
 *  @return noraml vector array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& Stl::normals( void ) const
{
    return( m_normals );
}

/*===========================================================================*/
/**
 *  @brief  Returns a coordinate value array.
 *  @return coodinate value array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& Stl::coords( void ) const
{
    return( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns a number of triangles.
 *  @return number of triangles
 */
/*===========================================================================*/
const size_t Stl::ntriangles( void ) const
{
    return( m_normals.size() / 3 );
}

/*===========================================================================*/
/**
 *  @brief  Sets file type.
 *  @param  file_type [in] file type (kvs::Stl::Ascii or kvs::Stl::Binary)
 */
/*===========================================================================*/
void Stl::setFileType( const FileType file_type )
{
    m_file_type = file_type;
}

/*===========================================================================*/
/**
 *  @brief  Set a normal vector array.
 *  @param  normals [in] normal vector array
 */
/*===========================================================================*/
void Stl::setNormals( const kvs::ValueArray<kvs::Real32>& normals )
{
    m_normals = normals;
}

/*===========================================================================*/
/**
 *  @brief  Set a coordinate vector array.
 *  @param  coords [in] coordinate vector array
 */
/*===========================================================================*/
void Stl::setCoords( const kvs::ValueArray<kvs::Real32>& coords )
{
    m_coords = coords;
}

/*===========================================================================*/
/**
 *  @brief  Read a STL file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool Stl::read( const std::string& filename )
{
    BaseClass::m_filename = filename;

    FILE* ifs = fopen( BaseClass::m_filename.c_str(), "r" );
    if ( !ifs )
    {
        kvsMessageError( "Cannot open %s.", m_filename.c_str() );
        BaseClass::m_is_success = false;
        return( BaseClass::m_is_success );
    }

    bool status = false;
//    if ( this->is_ascii_type( ifs ) )
    if ( ::IsAsciiType( ifs ) )
    {
        m_file_type = Stl::Ascii;
        status = this->read_ascii( ifs );
    }
    else
    {
        m_file_type = Stl::Binary;
        status = this->read_binary( ifs );
    }

    fclose( ifs );

    return( status );
}

/*===========================================================================*/
/**
 *  @brief  Writes the polygon data to a file as STL format.
 *  @param  filename [in] filename
 *  @return true, if the writting process is done successfully
 */
/*===========================================================================*/
const bool Stl::write( const std::string& filename )
{
    BaseClass::m_filename = filename;

    FILE* ofs = fopen( BaseClass::m_filename.c_str(), "w" );
    if ( !ofs )
    {
        kvsMessageError( "Cannot open %s.", m_filename.c_str() );
        BaseClass::m_is_success = false;
        return( BaseClass::m_is_success );
    }

    bool status = false;
    if ( m_file_type == Stl::Ascii )
    {
        status = this->write_ascii( ofs );
    }
    else
    {
        status = this->write_binary( ofs );
    }

    fclose( ofs );

    return( status );
}

/*===========================================================================*/
/**
 *  @brief  Check file type whether the ascii or the binary.
 *  @param  ifs [in] file pointer
 *  @return true, if the file type is ascii
 */
/*===========================================================================*/
const bool Stl::is_ascii_type( FILE* ifs )
{
    // Go back file-pointer to head.
    fseek( ifs, 0, SEEK_SET );

    // Check file type.
    char buffer[ ::MaxLineLength ];
    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        if ( buffer[0] == '\n' ) continue;

        const char* head = strtok( buffer, ::Delimiter );
        if ( !strcmp( head, "solid" ) ) break;
        else if ( !strcmp( head, "facet" ) ) return( true );
        else return( false );
    }

    size_t counter = 0;
    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        if ( counter > 5 ) return( false );
        if ( buffer[0] == '\n' ) continue;
        if ( strstr( buffer, "endsolid" ) ) return( true );
        if ( strstr( buffer, "facet" ) ) return( true );
        counter++;
    }

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Reads the polygon data as ascii format.
 *  @param  ifs [in] file pointer
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool Stl::read_ascii( FILE* ifs )
{
    // Go back file-pointer to head.
    fseek( ifs, 0, SEEK_SET );

    // Check head line.
    char buffer[ ::MaxLineLength ];
    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        if ( buffer[0] == '\n' ) continue;

        const char* head = strtok( buffer, ::Delimiter );
        if ( !strcmp( head, "solid" ) ) break;

        if ( !strcmp( head, "facet" ) )
        {
            fseek( ifs, 0, SEEK_SET );
            break;
        }
    }

    std::vector<kvs::Real32> normals;
    std::vector<kvs::Real32> coords;
    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        if ( buffer[0] == '\n' ) continue;
        if ( strstr( buffer, "endsolid" ) ) break;

        // facet
        const char* facet = strtok( buffer, ::Delimiter );
        if ( !strcmp( facet, "facet" ) )
        {
            strtok( 0, ::Delimiter ); // 'normal'
            normals.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
            normals.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
            normals.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
        }
        else
        {
            kvsMessageError("Cannot find 'facet'.");
            return( false );
        }

        // outer loop
        if ( fgets( buffer, ::MaxLineLength, ifs ) == 0 ) return( false );
        const char* outer = strtok( buffer, ::Delimiter );
        const char* loop = strtok( 0, ::Delimiter );
        if ( strcmp( outer, "outer" ) || strcmp( loop, "loop" ) )
        {
            kvsMessageError("Cannot find 'outer loop'.");
            return( false );
        }

        // vertex 0
        if ( fgets( buffer, ::MaxLineLength, ifs ) == 0 ) return( false );
        const char* vertex0 = strtok( buffer, ::Delimiter );
        if ( !strcmp( vertex0, "vertex" ) )
        {
            coords.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
            coords.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
            coords.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
        }
        else
        {
            kvsMessageError("Cannot find 'vertex' (0).");
            return( false );
        }

        // vertex 1
        if ( fgets( buffer, ::MaxLineLength, ifs ) == 0 ) return( false );
        const char* vertex1 = strtok( buffer, ::Delimiter );
        if ( !strcmp( vertex1, "vertex" ) )
        {
            coords.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
            coords.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
            coords.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
        }
        else
        {
            kvsMessageError("Cannot find 'vertex' (1).");
            return( false );
        }

        // vertex 2
        if ( fgets( buffer, ::MaxLineLength, ifs ) == 0 ) return( false );
        const char* vertex2 = strtok( buffer, ::Delimiter );
        if ( !strcmp( vertex2, "vertex" ) )
        {
            coords.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
            coords.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
            coords.push_back( static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) ) );
        }
        else
        {
            kvsMessageError("Cannot find 'vertex' (2).");
            return( false );
        }

        // endloop
        if ( fgets( buffer, ::MaxLineLength, ifs ) == 0 ) return( false );
        const char* endloop = strtok( buffer, ::Delimiter );
        if ( strcmp( endloop, "endloop" ) )
        {
            kvsMessageError("Cannot find 'endloop'.");
            return( false );
        }

        // endfacet
        if ( fgets( buffer, ::MaxLineLength, ifs ) == 0 ) return( false );
        const char* endfacet = strtok( buffer, ::Delimiter );
        if ( strcmp( endfacet, "endfacet" ) )
        {
            kvsMessageError("Cannot find 'endfacet'.");
            return( false );
        }
    }

    m_normals = kvs::ValueArray<kvs::Real32>( normals );
    m_coords = kvs::ValueArray<kvs::Real32>( coords );

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Reads the polygon data as binary format.
 *  @param  ifs [in] file pointer
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool Stl::read_binary( FILE* ifs )
{
    // Go back file-pointer to head.
    fseek( ifs, 0, SEEK_SET );

    // Read header string (80bytes).
    const size_t HeaderLength = 80;
    char buffer[ HeaderLength ];
    if ( fread( buffer, sizeof( char ), HeaderLength, ifs ) != HeaderLength )
    {
        kvsMessageError("Cannot read a header string (80byets).");
        return( false );
    }

    // Read a number of triangles (4bytes).
    kvs::UInt32 ntriangles = 0;
    if ( fread( &ntriangles, sizeof( kvs::UInt32 ), 1, ifs ) != 1 )
    {
        kvsMessageError("Cannot read a number of triangles.");
        return( false );
    }

    // Memory allocation.
    if ( !m_normals.allocate( ntriangles * 3 ) )
    {
        kvsMessageError("Cannot allocate memory for the normal vector array.");
        return( false );
    }
    if ( !m_coords.allocate( ntriangles * 9 ) )
    {
        kvsMessageError("Cannot allocate memory for the coordinate value array.");
        return( false );
    }

    // Read trianbles.
    kvs::Real32* normals = m_normals.pointer();
    kvs::Real32* coords = m_coords.pointer();
    kvs::UInt32 index3 = 0;
    kvs::UInt32 index9 = 0;
    for ( kvs::UInt32 i = 0; i < ntriangles; i++, index3 += 3, index9 += 9 )
    {
        // Normal vector (4x3=12byets)
        if ( fread( normals + index3, sizeof( kvs::Real32 ), 3, ifs ) != 3 )
        {
            kvsMessageError("Cannot read a normal vector.");
            m_normals.deallocate();
            return( false );
        }

        // Coordinate value (4x3=12bytes)
        if ( fread( coords + index9, sizeof( kvs::Real32 ), 9, ifs ) != 9 )
        {
            kvsMessageError("Cannot read a coordinate value.");
            m_coords.deallocate();
            return( false );
        }

        // Unused (2bytes)
        // NOTE: This block is sometimes used for storing color infomartion,
        // but we don't currently supported such color STL format.
        char unused[2];
        if ( fread( unused, sizeof( char ), 2, ifs ) != 2 )
        {
            kvsMessageError("Cannot read unused block (2bytes).");
            m_normals.deallocate();
            m_coords.deallocate();
            return( false );
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the polygon data as ascii format.
 *  @param  ifs [in] file pointer
 *  @return true, if the writting process is done successfully
 */
/*===========================================================================*/
const bool Stl::write_ascii( FILE* ofs )
{
    const char* header = "Generated by KVS";
    fprintf( ofs, "solid %s\n", header );

    const size_t ntriangles = m_normals.size() / 3;
    size_t index3 = 0;
    size_t index9 = 0;
    for ( size_t i = 0; i < ntriangles; i++, index3 += 3, index9 += 9 )
    {
        fprintf( ofs, "facet normal %f %f %f\n",
                 m_normals[ index3 + 0 ],
                 m_normals[ index3 + 1 ],
                 m_normals[ index3 + 2 ] );
        fprintf( ofs, "outer loop\n" );
        fprintf( ofs, "vertex %f %f %f\n",
                 m_coords[ index9 + 0 ],
                 m_coords[ index9 + 1 ],
                 m_coords[ index9 + 2 ] );
        fprintf( ofs, "vertex %f %f %f\n",
                 m_coords[ index9 + 3 ],
                 m_coords[ index9 + 4 ],
                 m_coords[ index9 + 5 ] );
        fprintf( ofs, "vertex %f %f %f\n",
                 m_coords[ index9 + 6 ],
                 m_coords[ index9 + 7 ],
                 m_coords[ index9 + 8 ] );
        fprintf( ofs, "endloop\n" );
        fprintf( ofs, "endfacet\n" );
    }

    fprintf( ofs, "endsolid\n" );

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the polygon data as binary format.
 *  @param  ifs [in] file pointer
 *  @return true, if the writting process is done successfully
 */
/*===========================================================================*/
const bool Stl::write_binary( FILE* ofs )
{
    // Header string (80bytes).
    char header[80]; memset( header, 0, 80 );
    sprintf( header, "Generated by KVS" );
    if ( fwrite( header, sizeof( char ), 80, ofs ) != 80 )
    {
        kvsMessageError("Cannot write a header string (80bytes).");
        return( false );
    }

    // Number of triangles (4bytes).
    const kvs::UInt32 ntriangles = static_cast<kvs::UInt32>( m_normals.size() / 3 );
    if ( fwrite( &ntriangles, sizeof( kvs::UInt32 ), 1, ofs ) != 1 )
    {
        kvsMessageError("Cannot write a number of triangles.");
        return( false );
    }

    // Triangles (50*ntriangles bytes)
    const kvs::Real32* normals = m_normals.pointer();
    const kvs::Real32* coords = m_coords.pointer();
    size_t index3 = 0;
    size_t index9 = 0;
    for ( size_t i = 0; i < ntriangles; i++, index3 += 3, index9 += 9 )
    {
        // Normal vector
        if ( fwrite( normals + index3, sizeof( kvs::Real32 ), 3, ofs ) != 3 )
        {
            kvsMessageError("Cannot write a normal vector.");
            return( false );
        }

        // Coordinate values
        if ( fwrite( coords + index9, sizeof( kvs::Real32 ), 9, ofs ) != 9 )
        {
            kvsMessageError("Cannot write a coordinate values (3 vertices = 9 elements).");
            return( false );
        }

        // Unused block (2bytes)
        const char unused[2] = {0,0};
        if ( fwrite( unused, sizeof( char ), 2, ofs ) != 2 )
        {
            kvsMessageError("Cannot write a unused block (2byets).");
            return( false );
        }
    }

    return( true );
}

std::ostream& operator << ( std::ostream& os, const Stl& stl )
{
    os << "file type : " << ::FileTypeToString[ stl.fileType() ] << std::endl;
    os << "number of triangles : " << stl.normals().size() / 3;

    return( os );
}

const bool Stl::CheckFileExtension( const std::string& filename )
{
    const kvs::File file( filename );
    if ( file.extension() == "stl" || file.extension() == "STL" ||
         file.extension() == "stla" || file.extension() == "STLA" ||
         file.extension() == "stlb" || file.extension() == "STLB" ||
         file.extension() == "sla" || file.extension() == "SLA" ||
         file.extension() == "slb" || file.extension() == "SLB" )
    {
        return( true );
    }

    return( false );
}

const bool Stl::CheckFileFormat( const std::string& filename )
{
    FILE* ifs = fopen( filename.c_str(), "rb" );
    if( !ifs )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        return( false );
    }

    if ( !::IsAsciiType( ifs ) )
    {
        // Go back file-pointer to head.
        fseek( ifs, 0, SEEK_SET );

        // Check binary STL
        kvs::File file( filename );
        const size_t byte_size = file.byteSize();

        // Read header string (80bytes).
        const size_t HeaderLength = 80;
        char buffer[ HeaderLength ];
        if ( fread( buffer, sizeof( char ), HeaderLength, ifs ) != HeaderLength )
        {
            kvsMessageError("Cannot read a header string (80byets).");
            fclose( ifs );
            return( false );
        }

        // Read a number of triangles (4bytes).
        kvs::UInt32 ntriangles = 0;
        if ( fread( &ntriangles, sizeof( kvs::UInt32 ), 1, ifs ) != 1 )
        {
            kvsMessageError("Cannot read a number of triangles.");
            fclose( ifs );
            return( false );
        }

        const size_t data_size = 50 * ntriangles + 80 + 4;
        if ( byte_size != data_size )
        {
            fclose( ifs );
            return( false );
        }
    }

    fclose( ifs );

    return( true );
}

} // end of namespace kvs
