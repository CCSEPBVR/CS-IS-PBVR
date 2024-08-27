/****************************************************************************/
/**
 *  @file AVSField.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: AVSField.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "AVSField.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <kvs/File>
#include <kvs/Macro>
#include <kvs/Platform>
#include <kvs/Version>


namespace
{

const int MaxLineLength = 256;

const std::string FieldTypeToString[kvs::AVSField::NumberOfFieldTypes] =
{
    "unknown",
    "uniform",
    "rectilinear",
    "irregular"
};

const std::string DataTypeToString[kvs::AVSField::NumberOfDataTypes] =
{
    "unknown",
    "byte",
    "short",
    "integer",
    "float",
    "double"
};

}

namespace
{

/*==========================================================================*/
/**
 *  Check the AVS header.
 *  @param ifs [in] input file stream
 */
/*==========================================================================*/
const bool CheckHeader( FILE* ifs )
{
    // Go back file-pointer to head.
    fseek( ifs, 0, SEEK_SET );

    // Check AVS header.
    char buf[ ::MaxLineLength ];

    // Read a head line and check the AVS format.
    if ( !fgets( buf, ::MaxLineLength, ifs ) ) return( false );
    if ( strncmp( buf, "# AVS", 5 ) != 0 ) return( false );

    return( true );
}

/*==========================================================================*/
/**
 *  Skip header region.
 *  @param ifs [in] input file stream
 */
/*==========================================================================*/
void SkipHeader( FILE* ifs )
{
    // Go back file-pointer to head.
    fseek( ifs, 0, SEEK_SET );

    // Search the data separator '\f\f'.
    int c;
    while( ( c = fgetc( ifs ) ) != EOF )
    {
        if( c == '\f' ) if( fgetc( ifs ) == '\f' ) break;
    }
}

}

namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
AVSField::AVSField( void )
{
    this->initialize();
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param filename [in] inpt filename
 */
/*==========================================================================*/
AVSField::AVSField( const std::string& filename )
{
    this->initialize();
    if( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
AVSField::~AVSField( void )
{
    this->clear();
}

/*==========================================================================*/
/**
 *  Initialize.
 */
/*==========================================================================*/
void AVSField::initialize( void )
{
    m_bits              = 0;
    m_is_signed         = true;
    m_veclen            = 1;
    m_nspace            = 0;
    m_ndim              = 0;
    m_dim               = kvs::Vector3ui( 1, 1, 1 );
    m_min_ext           = kvs::Vector3f( 0, 0, 0 );
    m_max_ext           = kvs::Vector3f( 1, 1, 1 );
    m_has_min_max_ext   = false;
    m_field             = Uniform;
    m_type              = UnknownDataType;
    m_labels.clear();
}

/*==========================================================================*/
/**
 *  Clear.
 */
/*==========================================================================*/
void AVSField::clear( void )
{
    m_values.deallocate();
    m_coords.deallocate();
}

/*==========================================================================*/
/**
 *  Substitution operator.
 *  @param fld [in] AVS field data
 */
/*==========================================================================*/
AVSField& AVSField::operator = ( const AVSField& rhs )
{
    m_bits            = rhs.m_bits;
    m_is_signed       = rhs.m_is_signed;
    m_veclen          = rhs.m_veclen;
    m_nspace          = rhs.m_nspace;
    m_ndim            = rhs.m_ndim;
    m_dim             = rhs.m_dim;
    m_min_ext         = rhs.m_min_ext;
    m_max_ext         = rhs.m_max_ext;
    m_has_min_max_ext = rhs.m_has_min_max_ext;
    m_field           = rhs.m_field;
    m_type            = rhs.m_type;

    std::copy( rhs.m_labels.begin(),
               rhs.m_labels.end(),
               m_labels.begin() );

    return( *this );
}

/*==========================================================================*/
/**
 *  Output stream for AVSField class.
 *  @param os [in] output stream
 *  @param fld [in] AVS field data
 */
/*==========================================================================*/
std::ostream& operator << ( std::ostream& os, const AVSField& fld )
{
    os << "bits    : " << fld.m_bits << std::endl;
    os << "signed  : " << ( fld.m_is_signed ? "signed" : "unsigned" ) << std::endl;
    os << "veclen  : " << fld.m_veclen << std::endl;
    os << "nspace  : " << fld.m_nspace << std::endl;
    os << "ndim    : " << fld.m_ndim << std::endl;
    os << "dim     : " << fld.m_dim << std::endl;
    os << "min ext : " << fld.m_min_ext << std::endl;
    os << "max ext : " << fld.m_max_ext << std::endl;
    os << "data    : " << ::DataTypeToString[fld.m_type] << std::endl;
    os << "field   : " << ::FieldTypeToString[fld.m_field] << std::endl;
    os << "label   : ";
    for( size_t i = 0; i < fld.m_labels.size(); i++ )
    {
        os << fld.m_labels[i] << " ";
    }
    os << std::endl;

    return( os );
}

/*==========================================================================*/
/**
 *  Get bit depth.
 *  @return bit depth
 */
/*==========================================================================*/
const int AVSField::bits( void ) const
{
    return( m_bits );
}

/*==========================================================================*/
/**
 *  Check value: signed or unsigned.
 *  @return true, if the value is signed
 */
/*==========================================================================*/
const bool AVSField::isSigned( void ) const
{
    return( m_is_signed );
}

/*==========================================================================*/
/**
 *  Get vector legth.
 *  @return vector length
 */
/*==========================================================================*/
const int AVSField::veclen( void ) const
{
    return( m_veclen );
}

/*==========================================================================*/
/**
 *  Get number of spaces.
 *  @return number of spaces
 */
/*==========================================================================*/
const int AVSField::nspace( void ) const
{
    return( m_nspace );
}

/*==========================================================================*/
/**
 *  Get number of dimensions.
 *  @return number of dimesions
 */
/*==========================================================================*/
const int AVSField::ndim( void ) const
{
    return( m_ndim );
}

/*==========================================================================*/
/**
 *  Get dimension.
 *  @return dimension
 */
/*==========================================================================*/
const kvs::Vector3ui& AVSField::dim( void ) const
{
    return( m_dim );
}

/*==========================================================================*/
/**
 *  Get extent min. coordinate value.
 *  @return extent min. coordinate value
 */
/*==========================================================================*/
const kvs::Vector3f& AVSField::minExt( void ) const
{
    return( m_min_ext );
}

/*==========================================================================*/
/**
 *  Get extent max. coordinate value.
 *  @return extent max. coordinate value
 */
/*==========================================================================*/
const kvs::Vector3f& AVSField::maxExt( void ) const
{
    return( m_max_ext );
}

/*==========================================================================*/
/**
 *  Check for including extent min/max coordinate value.
 *  @return true, if extent min/max coordinate value is included
 */
/*==========================================================================*/
const bool AVSField::hasMinMaxExt( void ) const
{
    return( m_has_min_max_ext );
}

/*==========================================================================*/
/**
 *  Get field type name.
 *  @return fiedl type name
 */
/*==========================================================================*/
const std::string& AVSField::field( void ) const
{
    return( ::FieldTypeToString[m_field] );
}

/*==========================================================================*/
/**
 *  Get data type name.
 *  @return data type name
 */
/*==========================================================================*/
const std::string& AVSField::type( void ) const
{
    return( ::DataTypeToString[m_type] );
}

/*==========================================================================*/
/**
 *  Get field type.
 *  @return field type
 */
/*==========================================================================*/
const AVSField::FieldType AVSField::fieldType( void ) const
{
    return( m_field );
}

/*==========================================================================*/
/**
 *  Get data type.
 *  @return data type
 */
/*==========================================================================*/
const AVSField::DataType AVSField::dataType( void ) const
{
    return( m_type );
}

/*==========================================================================*/
/**
 *  Get label array.
 *  @return label array
 */
/*==========================================================================*/
const std::vector<std::string>& AVSField::labels( void ) const
{
    return( m_labels );
}

/*==========================================================================*/
/**
 *  Get node array.
 *  @return node array
 */
/*==========================================================================*/
const kvs::AnyValueArray& AVSField::values( void ) const
{
    return( m_values );
}

/*==========================================================================*/
/**
 *  Get coordinate value array.
 *  @return coordinate value array
 */
/*==========================================================================*/
//const kvs::AnyValueArray& AVSField::coords( void ) const
const kvs::ValueArray<float>& AVSField::coords( void ) const
{
    return( m_coords );
}

/*==========================================================================*/
/**
 *  Set bit depth.
 *  @param bits [in] bit depth
 */
/*==========================================================================*/
void AVSField::setBits( const int bits )
{
    m_bits = bits;
}

/*==========================================================================*/
/**
 *  Set signed/unsigned.
 *  @param sign [in] signed(true) / unsigned(false)
 */
/*==========================================================================*/
void AVSField::setSigned( const bool sign )
{
    m_is_signed = sign;
}

/*==========================================================================*/
/**
 *  Set vector length.
 *  @param veclen [in] vector length
 */
/*==========================================================================*/
void AVSField::setVeclen( const int veclen )
{
    m_veclen = veclen;
}

/*==========================================================================*/
/**
 *  Set number of spaces.
 *  @param nspace [in] number of spaces
 */
/*==========================================================================*/
void AVSField::setNSpace( const int nspace )
{
    m_nspace = nspace;
}

/*==========================================================================*/
/**
 *  Set number of dimensions.
 *  @param ndim [in] number of dimensions
 */
/*==========================================================================*/
void AVSField::setNDim( const int ndim )
{
    m_ndim = ndim;
}

/*==========================================================================*/
/**
 *  Set dimension
 *  @param dim [in] dimension
 */
/*==========================================================================*/
void AVSField::setDim( const Vector3ui& dim )
{
    m_dim = dim;
}

/*==========================================================================*/
/**
 *  Set extent min. coordinate value.
 *  @param m [in] extent min. coordinate value.
 */
/*==========================================================================*/
void AVSField::setMinExt( const Vector3f& m )
{
    m_has_min_max_ext = true;
    m_min_ext = m;
}

/*==========================================================================*/
/**
 *  Set extent max. coordinate value.
 *  @param m [in] extent max. coordinate value
 */
/*==========================================================================*/
void AVSField::setMaxExt( const Vector3f& m )
{
    m_has_min_max_ext = true;
    m_max_ext = m;
}

/*==========================================================================*/
/**
 *  Set field type.
 *  @param field [in] field type
 */
/*==========================================================================*/
void AVSField::setFieldType( const FieldType field )
{
    m_field = field;
}

/*==========================================================================*/
/**
 *  Set data type.
 *  @param type [in] data type
 */
/*==========================================================================*/
void AVSField::setDataType( const DataType type )
{
    m_type = type;
}

/*==========================================================================*/
/**
 *  Set label array.
 *  @param labels [in] label array
 */
/*==========================================================================*/
void AVSField::setLabels( const std::vector<std::string>& labels )
{
    m_labels.clear();
    for( size_t i = 0; i < labels.size(); i++ )
    {
        m_labels.push_back( labels[i] );
    }
}

/*==========================================================================*/
/**
 *  Set field value array.
 *  @param values [in] field value array
 */
/*==========================================================================*/
void AVSField::setValues( const kvs::AnyValueArray& values )
{
    m_values = values;
}

/*==========================================================================*/
/**
 *  Set coordinate value array.
 *  @param coord [in] coordinate value array
 */
/*==========================================================================*/
void AVSField::setCoords( const kvs::ValueArray<float>& coords )
{
    m_coords = coords;
}

/*==========================================================================*/
/**
 *  Read AVS field data from file.
 *  @param filename [in] filename
 */
/*==========================================================================*/
const bool AVSField::read( const std::string& filename )
{
    m_filename = filename;

    FILE* ifs = fopen( filename.c_str(), "rb" );
    if( !ifs )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        return( false );
    }

    if( !read_header( ifs ) ) return( false );
    if( !read_node( ifs ) )   return( false );
    if( !read_coord( ifs ) )   return( false );

    fclose( ifs );

    return( true );
}

/*==========================================================================*/
/**
 *  Read header information.
 *  @param ifs [in] input file stream
 */
/*==========================================================================*/
bool AVSField::read_header( FILE* ifs )
{
    if( !::CheckHeader( ifs ) ) return( false );

    char buf[ ::MaxLineLength ];

    while( fgets( buf, ::MaxLineLength, ifs ) != NULL )
    {
        if( buf[0] == '#')   continue; // skip comment line
        if( buf[0] == '\f' ) break;    // detected data separator

        const char* seps = " ,\t\n" ;
        char* tag   = strtok( buf,  seps ) ;
        char* equal = strtok( NULL, seps ) ;
        char* value = strtok( NULL, seps ) ;

        if( strcmp( equal, "=" ) ) continue;

        if( !strcmp( tag, "veclen" ) ) m_veclen  = atoi( value );
        if( !strcmp( tag, "nspace" ) ) m_nspace  = atoi( value );
        if( !strcmp( tag, "ndim"   ) ) m_ndim    = atoi( value );
        if( !strcmp( tag, "dim1"   ) ) m_dim.x() = atoi( value );
        if( !strcmp( tag, "dim2"   ) ) m_dim.y() = atoi( value );
        if( !strcmp( tag, "dim3"   ) ) m_dim.z() = atoi( value );

        if( !strcmp( tag, "field"  ) )
        {
            m_field =
                !strcmp( value, "uniform"    ) ? Uniform :
                !strcmp( value, "rectilinear") ? Rectilinear :
                !strcmp( value, "irregular"  ) ? Irregular :
                UnknownFieldType;
        }

        if( !strcmp( tag, "data"  ) )
        {
            m_type =
                !strcmp( value, "byte"     ) ? Byte :
                !strcmp( value, "short"    ) ? Short :
                !strcmp( value, "integer"  ) ? Integer :
                !strcmp( value, "float"    ) ? Float :
                !strcmp( value, "double"   ) ? Double :
                UnknownDataType;
        }

        if( !strcmp( tag, "label" ) )
        {
            m_labels.clear();

            while( value )
            {
                m_labels.push_back( value );
                value = strtok( NULL, seps );
            }
        }

        if( !strcmp( tag, "min_ext" ) )
        {
            m_min_ext.x() = static_cast<float>( atof( value ) ); value = strtok( NULL, seps );
            m_min_ext.y() = static_cast<float>( atof( value ) ); value = strtok( NULL, seps );
            m_min_ext.z() = static_cast<float>( atof( value ) );

            m_has_min_max_ext = true;
        }

        if( !strcmp( tag, "max_ext" ) )
        {
            m_max_ext.x() = static_cast<float>( atof( value ) ); value = strtok( NULL, seps );
            m_max_ext.y() = static_cast<float>( atof( value ) ); value = strtok( NULL, seps );
            m_max_ext.z() = static_cast<float>( atof( value ) );

            m_has_min_max_ext = true;
        }
    }

    read_comment( ifs );

    return( true );
}

/*==========================================================================*/
/**
 *  Read comment.
 *  @param ifs [in] input file stream
 */
/*==========================================================================*/
bool AVSField::read_comment( FILE* ifs )
{
    // Go back file-pointer to head.
    fseek( ifs, 0, SEEK_SET );

    // Set initial value.
    switch( m_type )
    {
    case Byte:
    {
        m_bits = sizeof(unsigned char) << 3;
        m_is_signed = false;
        break;
    }
    case Short:
    {
        m_bits = sizeof(short) << 3;
        m_is_signed = true;
        break;
    }
    case Integer:
    {
        m_bits = sizeof(int) << 3;
        m_is_signed = true;
        break;
    }
    case Float:
    {
        m_bits = sizeof(float) << 3;
        m_is_signed = true;
        break;
    }
    case Double:
    {
        m_bits = sizeof(double) << 3;
        m_is_signed = true;
        break;
    }
    default: break;
    }

    // Read comment.
    char buf[ ::MaxLineLength ];
    while( fgets( buf, ::MaxLineLength, ifs ) != NULL )
    {
        if( buf[0] == '\f' ) break; // detected data separator
        if( buf[0] == '#'  )
        {
            if( strlen( buf ) < 2   ) continue;
            if( !strstr( buf, "=" ) ) continue;

            const char* seps = " ,\t\n" ;
            char* tag   = strtok( buf + 1,  seps ) ;
            char* equal = strtok( NULL, seps ) ;
            char* value = strtok( NULL, seps ) ;
            if( strcmp( equal, "=" ) ) continue;

            if( !strcmp( tag, "bits"   ) ) m_bits      = atoi( value );
            if( !strcmp( tag, "signed" ) ) m_is_signed = strcmp( value, "signed" ) == 0;
        }
    }

    return( true );
}

/*==========================================================================*/
/**
 *  Read node data.
 *  @param ifs [in] input file stream
 */
/*==========================================================================*/
bool AVSField::read_node( FILE* ifs )
{
    ::SkipHeader( ifs );

    const size_t nelems = m_dim.x() * m_dim.y() * m_dim.z() * m_veclen;
    switch( m_type )
    {
    case Byte:    m_values.allocate<unsigned char>( nelems ); break;
    case Short:   m_values.allocate<short>( nelems );         break;
    case Integer: m_values.allocate<int>( nelems );           break;
    case Float:   m_values.allocate<float>( nelems );         break;
    case Double:  m_values.allocate<double>( nelems );        break;
    default: break;
    }

    const size_t byte_size = m_values.byteSize();
    if ( fread( m_values.pointer(), 1, byte_size, ifs ) != byte_size )
    {
        return( false );
    }

#if defined ( KVS_PLATFORM_BIG_ENDIAN )
    m_values.swapByte();
#endif

    return( true );
}

/*==========================================================================*/
/**
 *  Read node data.
 *  @param ifs [in] input file stream
 */
/*==========================================================================*/
bool AVSField::read_coord( FILE* ifs )
{
    if( m_field == AVSField::Uniform ) return true;

    size_t nvertices = 0;
    if( m_field == AVSField::Rectilinear )
    {
        const size_t ndim = static_cast<size_t>( m_ndim );
        for ( size_t i = 0; i < ndim; i++ ) nvertices += m_dim[i];
    }
    else if( m_field == AVSField::Irregular )
    {
        nvertices = 1;
        const size_t ndim = static_cast<size_t>( m_ndim );
        for ( size_t i = 0; i < ndim; i++ ) nvertices *= m_dim[i];
    }
    else
    {
        kvsMessageError("Unknown field type");
        return false;
    }

    switch( m_type )
    {
    case Byte:    read_coord_data<unsigned char>( ifs, nvertices ); break;
    case Short:   read_coord_data<short>( ifs, nvertices ); break;
    case Integer: read_coord_data<int>( ifs, nvertices ); break;
    case Float:   read_coord_data<float>( ifs, nvertices ); break;
    case Double:  read_coord_data<double>( ifs, nvertices ); break;
    default: break;
    }

    return true;
}

template <typename T>
bool AVSField::read_coord_data(  FILE* ifs, const size_t nvertices )
{
    m_coords.allocate( nvertices * m_nspace );

    const size_t nspace = static_cast<size_t>( m_nspace );
    for( size_t nsp = 0; nsp < nspace; nsp++ )
    {
        for( size_t i = 0; i < nvertices; i++ )
        {
            T data = T(0);
            if ( fread( &data, sizeof(T), 1, ifs ) == 1 )
            {
                m_coords[ i * m_nspace + nsp ] = static_cast<float>( data );
            }
        }
    }

    return true;
}

/*==========================================================================*/
/**
 *  Write AVS Field data to file.
 *  @param filename [in] filename
 */
/*==========================================================================*/
const bool AVSField::write( const std::string& filename )
{
    m_filename = filename;

    std::ofstream ofs( filename.c_str(), std::ios::out | std::ios::binary );
    if( !ofs.is_open() )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        return( false );
    }

    if( !write_header( ofs ) ) return( false );
    if( !write_node( ofs ) )   return( false );

    ofs.close();

    return( true );
}

/*==========================================================================*/
/**
 *  Write header information.
 *  @param ofs [in] output file stream
 */
/*==========================================================================*/
bool AVSField::write_header( std::ofstream& ofs ) const
{
    // Output a necessary line.
    std::string version = kvs::Version::Name();
    ofs << "# AVS field generated by KVS (" << version << ")" << std::endl;
    ofs << "#" << std::endl;

#if defined ( KVS_PLATFORM_WINDOWS )
    char* author = getenv( "USERNAME" );
#else
    char* author = getenv( "USER" );
#endif
    time_t tm; time( &tm );
    char*  date = ctime( &tm ); // This string includes a return code: '\n'.
    ofs << "# Author: " << std::string(author) << std::endl;
    ofs << "# Date:   " << std::string(date);
    ofs << "#" << std::endl;

    // Output extended header information.
    ofs << "# bits = " << m_bits << std::endl;
    ofs << "# signed = " << ( m_is_signed ? "signed" : "unsigned" ) << std::endl;
    ofs << "#" << std::endl;

    // Output header information.
    ofs << "nspace = " << m_nspace << std::endl;
    ofs << "veclen = " << m_veclen << std::endl;
    ofs << "ndim   = " << m_ndim   << std::endl;
    ofs << "dim1   = " << m_dim.x()  << std::endl;
    if( m_ndim >= 2 ) ofs << "dim2   = " << m_dim.y()  << std::endl;
    if( m_ndim >= 3 ) ofs << "dim3   = " << m_dim.z()  << std::endl;
    ofs << "data   = " << ::DataTypeToString[m_type] << std::endl;
    ofs << "field  = " << ::FieldTypeToString[m_field] << std::endl;
    if( m_has_min_max_ext )
    {
        ofs << "min_ext = "
            << m_min_ext.x() << " "
            << m_min_ext.y() << " "
            << m_min_ext.z() << std::endl;
        ofs << "max_ext = "
            << m_max_ext.x() << " "
            << m_max_ext.y() << " "
            << m_max_ext.z() << std::endl;
    }
    if( m_labels.size() != 0 )
    {
        ofs << "label  = ";
        for( size_t i = 0; i < m_labels.size(); i++ )
        {
            ofs << m_labels[i] << " ";
        }
        ofs << std::endl;
    }

    return( true );
}

/*==========================================================================*/
/**
 *  Write node data.
 *  @param ofs [in] output file stream
 */
/*==========================================================================*/
bool AVSField::write_node( std::ofstream& ofs ) const
{
    ofs << "\f\f";
    ofs.write( (char*)m_values.pointer(), m_values.byteSize() );

    return( true );
}

const bool AVSField::CheckFileExtension( const std::string& filename )
{
    const kvs::File file( filename );
    if ( file.extension() == "fld" || file.extension() == "FLD" )
    {
        return( true );
    }

    return( false );
}

const bool AVSField::CheckFileFormat( const std::string& filename )
{
    FILE* ifs = fopen( filename.c_str(), "rb" );
    if( !ifs )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        return( false );
    }

    if ( !::CheckHeader( ifs ) )
    {
        fclose( ifs );
        return( false );
    }

    fclose( ifs );
    return( true );
}

} // end of namespace kvs
