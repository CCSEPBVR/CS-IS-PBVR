/****************************************************************************/
/**
 *  @file KVSMLObjectStructuredVolume.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectStructuredVolume.cpp 867 2011-07-24 06:12:29Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#include "KVSMLObjectStructuredVolume.h"
#include "StructuredVolumeObjectTag.h"
#include "NodeTag.h"
#include "ValueTag.h"
#include "DataArrayTag.h"
#include "CoordTag.h"
#include <kvs/File>
#include <kvs/XMLDocument>
#include <kvs/XMLDeclaration>
#include <kvs/XMLElement>
#include <kvs/XMLComment>
#include <kvs/AnyValueArray>
#include <kvs/Type>
#include <kvs/String>
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML object structured volume object.
 */
/*===========================================================================*/
KVSMLObjectStructuredVolume::KVSMLObjectStructuredVolume( void ):
    m_writing_type( kvs::KVSMLObjectStructuredVolume::Ascii )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML object structured volume object class by reading the given file.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
KVSMLObjectStructuredVolume::KVSMLObjectStructuredVolume( const std::string& filename ):
    m_writing_type( kvs::KVSMLObjectStructuredVolume::Ascii )
{
    if( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*===========================================================================*/
/**
 *  @brief  Destructs the KVSML object structured volume object class.
 */
/*===========================================================================*/
KVSMLObjectStructuredVolume::~KVSMLObjectStructuredVolume( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the KVSML tag.
 *  @return KVSML tag
 */
/*===========================================================================*/
const kvs::kvsml::KVSMLTag& KVSMLObjectStructuredVolume::KVSMLTag( void ) const
{
    return( m_kvsml_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns the object tag.
 *  @return object tag
 */
/*===========================================================================*/
const kvs::kvsml::ObjectTag& KVSMLObjectStructuredVolume::objectTag( void ) const
{
    return( m_object_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns the grid type.
 *  @return grid type
 */
/*===========================================================================*/
const std::string& KVSMLObjectStructuredVolume::gridType( void ) const
{
    return( m_grid_type );
}

const bool KVSMLObjectStructuredVolume::hasLabel( void ) const
{
    return( m_has_label );
}

const std::string& KVSMLObjectStructuredVolume::label( void ) const
{
    return( m_label );
}

/*===========================================================================*/
/**
 *  @brief  Returns the vector length.
 *  @return vector length
 */
/*===========================================================================*/
const size_t KVSMLObjectStructuredVolume::veclen( void ) const
{
    return( m_veclen );
}

/*===========================================================================*/
/**
 *  @brief  Returns the grid resolution.
 *  @return grid resolution
 */
/*===========================================================================*/
const kvs::Vector3ui& KVSMLObjectStructuredVolume::resolution( void ) const
{
    return( m_resolution );
}

const bool KVSMLObjectStructuredVolume::hasMinValue( void ) const
{
    return( m_has_min_value );
}

const bool KVSMLObjectStructuredVolume::hasMaxValue( void ) const
{
    return( m_has_max_value );
}

const double KVSMLObjectStructuredVolume::minValue( void ) const
{
    return( m_min_value );
}

const double KVSMLObjectStructuredVolume::maxValue( void ) const
{
    return( m_max_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the value array.
 *  @return value array
 */
/*===========================================================================*/
const kvs::AnyValueArray& KVSMLObjectStructuredVolume::values( void ) const
{
    return( m_values );
}

const kvs::ValueArray<float>& KVSMLObjectStructuredVolume::coords( void ) const
{
    return( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Sets a writing data type.
 *  @param  writing_type [in] wirting data type
 */
/*===========================================================================*/
void KVSMLObjectStructuredVolume::setWritingDataType( const WritingDataType writing_type )
{
    m_writing_type = writing_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets a grid type.
 *  @param  grid_type [in] grid type
 */
/*===========================================================================*/
void KVSMLObjectStructuredVolume::setGridType( const std::string& grid_type )
{
    m_grid_type = grid_type;
}

void KVSMLObjectStructuredVolume::setLabel( const std::string& label )
{
    m_has_label = true;
    m_label = label;
}

/*===========================================================================*/
/**
 *  @brief  Sets a vector length.
 *  @param  veclen [in] vector length
 */
/*===========================================================================*/
void KVSMLObjectStructuredVolume::setVeclen( const size_t veclen )
{
    m_veclen = veclen;
}

/*===========================================================================*/
/**
 *  @brief  Sets a grid resolution.
 *  @param  resolution [in] grid resolution
 */
/*===========================================================================*/
void KVSMLObjectStructuredVolume::setResolution( const kvs::Vector3ui& resolution )
{
    m_resolution = resolution;
}

void KVSMLObjectStructuredVolume::setMinValue( const double min_value )
{
    m_has_min_value = true;
    m_min_value = min_value;
}

void KVSMLObjectStructuredVolume::setMaxValue( const double max_value )
{
    m_has_max_value = true;
    m_max_value = max_value;
}

/*===========================================================================*/
/**
 *  @brief  Sets a value array.
 *  @param  values [in] value array
 */
/*===========================================================================*/
void KVSMLObjectStructuredVolume::setValues( const kvs::AnyValueArray& values )
{
    m_values = values;
}

void KVSMLObjectStructuredVolume::setCoords( const kvs::ValueArray<float>& coords )
{
    m_coords = coords;
}

/*===========================================================================*/
/**
 *  @brief  Reads the KVSML structured volume object from the given file.
 *  @param  filename [in] filename
 *  @return ture, if the reading process is done successfully
 */
/*===========================================================================*/
const bool KVSMLObjectStructuredVolume::read( const std::string& filename )
{
    m_filename = filename;

    // XML document
    kvs::XMLDocument document;
    if ( !document.read( filename ) )
    {
        kvsMessageError( "%s", document.ErrorDesc().c_str() );
        return( false );
    }

    // <KVSML>
    if ( !m_kvsml_tag.read( &document ) )
    {
        kvsMessageError( "Cannot read <%s>.", m_kvsml_tag.name().c_str() );
        return( false );
    }

    // <Object>
    if ( !m_object_tag.read( m_kvsml_tag.node() ) )
    {
        kvsMessageError( "Cannot read <%s>.", m_object_tag.name().c_str() );
        return( false );
    }

    // <StructuredVolumeObject>
    kvs::kvsml::StructuredVolumeObjectTag volume_tag;
    if ( !volume_tag.read( m_object_tag.node() ) )
    {
        kvsMessageError( "Cannot read <%s>.", volume_tag.name().c_str() );
        return( false );
    }

    if ( !volume_tag.hasResolution() )
    {
        kvsMessageError( "'resolution' is not specified in <%s>.", volume_tag.name().c_str() );
        return( false );
    }
    m_resolution = volume_tag.resolution();

    if ( !volume_tag.hasGridType() )
    {
        kvsMessageError( "'grid_type' is not specified in <%s>.", volume_tag.name().c_str() );
        return( false );
    }
    m_grid_type = volume_tag.gridType();

    // <Node>
    kvs::kvsml::NodeTag node_tag;
    if ( !node_tag.read( volume_tag.node() ) )
    {
        kvsMessageError( "Cannot read <%s>.", node_tag.name().c_str() );
        return( false );
    }

    // <Value>
    kvs::kvsml::ValueTag value_tag;
    if ( !value_tag.read( node_tag.node() ) )
    {
        kvsMessageError( "Cannot read <%s>.", value_tag.name().c_str() );
        return( false );
    }

    m_has_label = value_tag.hasLabel();
    if ( m_has_label ) { m_label = value_tag.label(); }

    if ( !value_tag.hasVeclen() )
    {
        kvsMessageError( "'veclen' is not specified in <%s>.", value_tag.name().c_str() );
        return( false );
    }
    m_veclen = value_tag.veclen();

    m_has_min_value = value_tag.hasMinValue();
    if ( m_has_min_value ) { m_min_value = value_tag.minValue(); }

    m_has_max_value = value_tag.hasMaxValue();
    if ( m_has_max_value ) { m_max_value = value_tag.maxValue(); }

    // <DataArray>
    const kvs::Vector3ui resolution = volume_tag.resolution();
    const size_t nnodes = resolution.x() * resolution.y() * resolution.z();
    const size_t veclen = value_tag.veclen();
    const size_t nelements = nnodes * veclen;
    kvs::kvsml::DataArrayTag values;
    if ( !values.read( value_tag.node(), nelements, &m_values ) )
    {
        kvsMessageError( "Cannot read <%s> for <%s>.",
                         values.name().c_str(),
                         value_tag.name().c_str() );
        return( false );
    }

    // <Coord>
    if ( m_grid_type == "rectilinear" )
    {
        kvs::kvsml::CoordTag coord_tag;
        if ( !coord_tag.read( node_tag.node() ) )
        {
            kvsMessageError( "Cannot read <%s>.", coord_tag.name().c_str() );
            return( false );
        }

        // <DataArray>
        kvs::kvsml::DataArrayTag coords;
        const size_t dimension = 3;
        size_t coord_nelements = 0;
        for ( size_t i = 0; i < dimension; i++ ) coord_nelements += resolution[i];
        if ( !coords.read( coord_tag.node(), coord_nelements, &m_coords ) )
        {
            kvsMessageError( "Cannot read <%s> for <%s>.",
                             coords.name().c_str(),
                             coord_tag.name().c_str() );
            return( false );
        }
    }
    else if ( m_grid_type == "curvilinear" )
    {
        kvs::kvsml::CoordTag coord_tag;
        if ( !coord_tag.read( node_tag.node() ) )
        {
            kvsMessageError( "Cannot read <%s>.", coord_tag.name().c_str() );
            return( false );
        }

        // <DataArray>
        kvs::kvsml::DataArrayTag coords;
        const size_t dimension = 3;
        const size_t coord_nelements = nnodes * dimension;
        if ( !coords.read( coord_tag.node(), coord_nelements, &m_coords ) )
        {
            kvsMessageError( "Cannot read <%s> for <%s>.",
                             coords.name().c_str(),
                             coord_tag.name().c_str() );
            return( false );
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the KVSML structured volume object to the given file.
 *  @param  filename [in] filename
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool KVSMLObjectStructuredVolume::write( const std::string& filename )
{
    m_filename = filename;

    // XML document
    kvs::XMLDocument document;
    document.InsertEndChild( kvs::XMLDeclaration( "1.0" ) );
    document.InsertEndChild( kvs::XMLComment(" Generated by kvs::KVSMLObjectStructuredVolume::write() ") );

    // <KVSML>
    kvs::kvsml::KVSMLTag kvsml_tag;
    if ( !kvsml_tag.write( &document ) )
    {
        kvsMessageError( "Cannot write <%s>.", m_kvsml_tag.name().c_str() );
        return( false );
    }

    // <Object type="StructuredVolumeObject">
    kvs::kvsml::ObjectTag object_tag;
    object_tag.setType( "StructuredVolumeObject" );
    if ( !object_tag.write( kvsml_tag.node() ) )
    {
        kvsMessageError( "Cannot write <%s>.", object_tag.name().c_str() );
        return( false );
    }

    // <StructuredVolumeObject resolution="xxx xxx xxx" grid_type="xxx">
    kvs::kvsml::StructuredVolumeObjectTag volume_tag;
    volume_tag.setResolution( m_resolution );
    volume_tag.setGridType( m_grid_type );
    if ( !volume_tag.write( object_tag.node() ) )
    {
        kvsMessageError( "Cannot write <%s>.", volume_tag.name().c_str() );
        return( false );
    }

    // <Node>
    kvs::kvsml::NodeTag node_tag;
    if ( !node_tag.write( volume_tag.node() ) )
    {
        kvsMessageError( "Cannot write <%s>.", node_tag.name().c_str() );
        return( false );
    }

    // <Value label="xxx" veclen="xxx" min_value="xxx" max_value="xxx">
    kvs::kvsml::ValueTag value_tag;
    value_tag.setVeclen( m_veclen );

    if ( m_has_label ) { value_tag.setLabel( m_label ); }
    if ( m_has_min_value ) { value_tag.setMinValue( m_min_value ); }
    if ( m_has_max_value ) { value_tag.setMaxValue( m_max_value ); }

    if ( !value_tag.write( node_tag.node() ) )
    {
        kvsMessageError( "Cannot write <%s>.", value_tag.name().c_str() );
        return( false );
    }

    // <DataArray>
    kvs::kvsml::DataArrayTag values;
    if ( m_writing_type == ExternalAscii )
    {
        values.setFile( kvs::kvsml::DataArray::GetDataFilename( m_filename, "value" ) );
        values.setFormat( "ascii" );
    }
    else if ( m_writing_type == ExternalBinary )
    {
        values.setFile( kvs::kvsml::DataArray::GetDataFilename( m_filename, "value" ) );
        values.setFormat( "binary" );
    }

    const std::string pathname = kvs::File( m_filename ).pathName();
    if ( !values.write( value_tag.node(), m_values, pathname ) )
    {
        kvsMessageError( "Cannot write <%s> for <%s>.",
                         values.name().c_str(),
                         value_tag.name().c_str() );
        return( false );
    }

    if ( m_grid_type == "rectilinear" || m_grid_type == "curvilinear" )
    {
        // <Coord>
        kvs::kvsml::CoordTag coord_tag;
        if ( !coord_tag.write( node_tag.node() ) )
        {
            kvsMessageError( "Cannot write <%s>.", coord_tag.name().c_str() );
            return( false );
        }

        // <DataArray>
        kvs::kvsml::DataArrayTag coords;
        if ( m_writing_type == ExternalAscii )
        {
            coords.setFile( kvs::kvsml::DataArray::GetDataFilename( m_filename, "coord" ) );
            coords.setFormat( "ascii" );
        }
        else if ( m_writing_type == ExternalBinary )
        {
            coords.setFile( kvs::kvsml::DataArray::GetDataFilename( m_filename, "coord" ) );
            coords.setFormat( "binary" );
        }

        if ( !coords.write( coord_tag.node(), m_coords, pathname ) )
        {
            kvsMessageError( "Cannot write <%s> for <%s>.",
                             coords.name().c_str(),
                             coord_tag.name().c_str() );
            return( false );
        }
    }

    return( document.write( m_filename ) );
}

/*===========================================================================*/
/**
 *  @brief  Checks the file extension.
 *  @param  filename [in] filename
 *  @return true, if the given filename has the supported extension
 */
/*===========================================================================*/
const bool KVSMLObjectStructuredVolume::CheckFileExtension( const std::string& filename )
{
    const kvs::File file( filename );
    if ( file.extension() == "kvsml" ||
         file.extension() == "KVSML" ||
         file.extension() == "xml"   ||
         file.extension() == "XML" )
    {
        return( true );
    }

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Check the file format.
 *  @param  filename [in] filename
 *  @return true, if the KVSMLObjectStructuredVolume class can read the given file
 */
/*===========================================================================*/
const bool KVSMLObjectStructuredVolume::CheckFileFormat( const std::string& filename )
{
    kvs::XMLDocument document;
    if ( !document.read( filename ) ) return( false );

    // <KVSML>
    kvs::kvsml::KVSMLTag kvsml_tag;
    if ( !kvsml_tag.read( &document ) ) return( false );

    // <Object>
    kvs::kvsml::ObjectTag object_tag;
    if ( !object_tag.read( kvsml_tag.node() ) ) return( false );

    if ( object_tag.type() != "StructuredVolumeObject" ) return( false );

    // <StructuredVolumeObject>
    kvs::kvsml::StructuredVolumeObjectTag volume_tag;
    if ( !volume_tag.read( object_tag.node() ) ) return( false );

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Output operator.
 *  @param  os [out] output stream
 *  @param  rhs [in] KVSML structured volume object
 */
/*===========================================================================*/
std::ostream& operator <<( std::ostream& os, const KVSMLObjectStructuredVolume& rhs )
{
    os << "Grid type: " << rhs.m_grid_type << std::endl;
    os << "Veclen: " << rhs.m_veclen << std::endl;
    os << "Resolution: " << rhs.m_resolution << std::endl;
    os << "Value type: " << rhs.m_values.typeInfo()->typeName();

    return( os );
}

} // end of namespace kvs
