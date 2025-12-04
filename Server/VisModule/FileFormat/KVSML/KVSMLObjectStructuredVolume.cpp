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
#include <vismodule/DataArrayTag>
#include "CoordTag.h"
#include <vismodule/File>
#include <vismodule/XMLDocument>
#include <vismodule/XMLDeclaration>
#include <vismodule/XMLElement>
#include <vismodule/XMLComment>
#include <vismodule/AnyValueArray>
#include <vismodule/Type>
#include <vismodule/String>
#include <vismodule/IgnoreUnusedVariable>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML object structured volume object.
 */
/*===========================================================================*/
KVSMLObjectStructuredVolume::KVSMLObjectStructuredVolume( void ):
    m_writing_type( vismodule::KVSMLObjectStructuredVolume::Ascii )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML object structured volume object class by reading the given file.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
KVSMLObjectStructuredVolume::KVSMLObjectStructuredVolume( const std::string& filename ):
    m_writing_type( vismodule::KVSMLObjectStructuredVolume::Ascii )
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
const vismodule::kvsml::KVSMLTag& KVSMLObjectStructuredVolume::KVSMLTag( void ) const
{
    return( m_kvsml_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns the object tag.
 *  @return object tag
 */
/*===========================================================================*/
const vismodule::kvsml::ObjectTag& KVSMLObjectStructuredVolume::objectTag( void ) const
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
const vismodule::Vector3ui& KVSMLObjectStructuredVolume::resolution( void ) const
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
const vismodule::AnyValueArray& KVSMLObjectStructuredVolume::values( void ) const
{
    return( m_values );
}

const vismodule::ValueArray<float>& KVSMLObjectStructuredVolume::coords( void ) const
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
void KVSMLObjectStructuredVolume::setResolution( const vismodule::Vector3ui& resolution )
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
void KVSMLObjectStructuredVolume::setValues( const vismodule::AnyValueArray& values )
{
    m_values = values;
}

void KVSMLObjectStructuredVolume::setCoords( const vismodule::ValueArray<float>& coords )
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
    vismodule::XMLDocument document;
    if ( !document.read( filename ) )
    {
        visModuleMessageError( "%s", document.ErrorDesc().c_str() );
        return( false );
    }

    // <KVSML>
    if ( !m_kvsml_tag.read( &document ) )
    {
        visModuleMessageError( "Cannot read <%s>.", m_kvsml_tag.name().c_str() );
        return( false );
    }

    // <Object>
    if ( !m_object_tag.read( m_kvsml_tag.node() ) )
    {
        visModuleMessageError( "Cannot read <%s>.", m_object_tag.name().c_str() );
        return( false );
    }

    // <StructuredVolumeObject>
    vismodule::kvsml::StructuredVolumeObjectTag volume_tag;
    if ( !volume_tag.read( m_object_tag.node() ) )
    {
        visModuleMessageError( "Cannot read <%s>.", volume_tag.name().c_str() );
        return( false );
    }

    if ( !volume_tag.hasResolution() )
    {
        visModuleMessageError( "'resolution' is not specified in <%s>.", volume_tag.name().c_str() );
        return( false );
    }
    m_resolution = volume_tag.resolution();

    if ( !volume_tag.hasGridType() )
    {
        visModuleMessageError( "'grid_type' is not specified in <%s>.", volume_tag.name().c_str() );
        return( false );
    }
    m_grid_type = volume_tag.gridType();

    // <Node>
    vismodule::kvsml::NodeTag node_tag;
    if ( !node_tag.read( volume_tag.node() ) )
    {
        visModuleMessageError( "Cannot read <%s>.", node_tag.name().c_str() );
        return( false );
    }

    // <Value>
    vismodule::kvsml::ValueTag value_tag;
    if ( !value_tag.read( node_tag.node() ) )
    {
        visModuleMessageError( "Cannot read <%s>.", value_tag.name().c_str() );
        return( false );
    }

    m_has_label = value_tag.hasLabel();
    if ( m_has_label ) { m_label = value_tag.label(); }

    if ( !value_tag.hasVeclen() )
    {
        visModuleMessageError( "'veclen' is not specified in <%s>.", value_tag.name().c_str() );
        return( false );
    }
    m_veclen = value_tag.veclen();

    m_has_min_value = value_tag.hasMinValue();
    if ( m_has_min_value ) { m_min_value = value_tag.minValue(); }

    m_has_max_value = value_tag.hasMaxValue();
    if ( m_has_max_value ) { m_max_value = value_tag.maxValue(); }

    // <DataArray>
    const vismodule::Vector3ui resolution = volume_tag.resolution();
    const size_t nnodes = resolution.x() * resolution.y() * resolution.z();
    const size_t veclen = value_tag.veclen();
    const size_t nelements = nnodes * veclen;
    vismodule::kvsml::DataArrayTag values;
    if ( !values.read( value_tag.node(), nelements, &m_values ) )
    {
        visModuleMessageError( "Cannot read <%s> for <%s>.",
                         values.name().c_str(),
                         value_tag.name().c_str() );
        return( false );
    }

    // <Coord>
    if ( m_grid_type == "rectilinear" )
    {
        vismodule::kvsml::CoordTag coord_tag;
        if ( !coord_tag.read( node_tag.node() ) )
        {
            visModuleMessageError( "Cannot read <%s>.", coord_tag.name().c_str() );
            return( false );
        }

        // <DataArray>
        vismodule::kvsml::DataArrayTag coords;
        const size_t dimension = 3;
        size_t coord_nelements = 0;
        for ( size_t i = 0; i < dimension; i++ ) coord_nelements += resolution[i];
        if ( !coords.read( coord_tag.node(), coord_nelements, &m_coords ) )
        {
            visModuleMessageError( "Cannot read <%s> for <%s>.",
                             coords.name().c_str(),
                             coord_tag.name().c_str() );
            return( false );
        }
    }
    else if ( m_grid_type == "curvilinear" )
    {
        vismodule::kvsml::CoordTag coord_tag;
        if ( !coord_tag.read( node_tag.node() ) )
        {
            visModuleMessageError( "Cannot read <%s>.", coord_tag.name().c_str() );
            return( false );
        }

        // <DataArray>
        vismodule::kvsml::DataArrayTag coords;
        const size_t dimension = 3;
        const size_t coord_nelements = nnodes * dimension;
        if ( !coords.read( coord_tag.node(), coord_nelements, &m_coords ) )
        {
            visModuleMessageError( "Cannot read <%s> for <%s>.",
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
    vismodule::XMLDocument document;
    document.InsertEndChild( vismodule::XMLDeclaration( "1.0" ) );
    document.InsertEndChild( vismodule::XMLComment(" Generated by vismodule::KVSMLObjectStructuredVolume::write() ") );

    // <KVSML>
    vismodule::kvsml::KVSMLTag kvsml_tag;
    if ( !kvsml_tag.write( &document ) )
    {
        visModuleMessageError( "Cannot write <%s>.", m_kvsml_tag.name().c_str() );
        return( false );
    }

    // <Object type="StructuredVolumeObject">
    vismodule::kvsml::ObjectTag object_tag;
    object_tag.setType( "StructuredVolumeObject" );
    if ( !object_tag.write( kvsml_tag.node() ) )
    {
        visModuleMessageError( "Cannot write <%s>.", object_tag.name().c_str() );
        return( false );
    }

    // <StructuredVolumeObject resolution="xxx xxx xxx" grid_type="xxx">
    vismodule::kvsml::StructuredVolumeObjectTag volume_tag;
    volume_tag.setResolution( m_resolution );
    volume_tag.setGridType( m_grid_type );
    if ( !volume_tag.write( object_tag.node() ) )
    {
        visModuleMessageError( "Cannot write <%s>.", volume_tag.name().c_str() );
        return( false );
    }

    // <Node>
    vismodule::kvsml::NodeTag node_tag;
    if ( !node_tag.write( volume_tag.node() ) )
    {
        visModuleMessageError( "Cannot write <%s>.", node_tag.name().c_str() );
        return( false );
    }

    // <Value label="xxx" veclen="xxx" min_value="xxx" max_value="xxx">
    vismodule::kvsml::ValueTag value_tag;
    value_tag.setVeclen( m_veclen );

    if ( m_has_label ) { value_tag.setLabel( m_label ); }
    if ( m_has_min_value ) { value_tag.setMinValue( m_min_value ); }
    if ( m_has_max_value ) { value_tag.setMaxValue( m_max_value ); }

    if ( !value_tag.write( node_tag.node() ) )
    {
        visModuleMessageError( "Cannot write <%s>.", value_tag.name().c_str() );
        return( false );
    }

    // <DataArray>
    vismodule::kvsml::DataArrayTag values;
    if ( m_writing_type == ExternalAscii )
    {
        values.setFile( vismodule::kvsml::DataArray::GetDataFilename( m_filename, "value" ) );
        values.setFormat( "ascii" );
    }
    else if ( m_writing_type == ExternalBinary )
    {
        values.setFile( vismodule::kvsml::DataArray::GetDataFilename( m_filename, "value" ) );
        values.setFormat( "binary" );
    }

    const std::string pathname = vismodule::File( m_filename ).pathName();
    if ( !values.write( value_tag.node(), m_values, pathname ) )
    {
        visModuleMessageError( "Cannot write <%s> for <%s>.",
                         values.name().c_str(),
                         value_tag.name().c_str() );
        return( false );
    }

    if ( m_grid_type == "rectilinear" || m_grid_type == "curvilinear" )
    {
        // <Coord>
        vismodule::kvsml::CoordTag coord_tag;
        if ( !coord_tag.write( node_tag.node() ) )
        {
            visModuleMessageError( "Cannot write <%s>.", coord_tag.name().c_str() );
            return( false );
        }

        // <DataArray>
        vismodule::kvsml::DataArrayTag coords;
        if ( m_writing_type == ExternalAscii )
        {
            coords.setFile( vismodule::kvsml::DataArray::GetDataFilename( m_filename, "coord" ) );
            coords.setFormat( "ascii" );
        }
        else if ( m_writing_type == ExternalBinary )
        {
            coords.setFile( vismodule::kvsml::DataArray::GetDataFilename( m_filename, "coord" ) );
            coords.setFormat( "binary" );
        }

        if ( !coords.write( coord_tag.node(), m_coords, pathname ) )
        {
            visModuleMessageError( "Cannot write <%s> for <%s>.",
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
    const vismodule::File file( filename );
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
    vismodule::XMLDocument document;
    if ( !document.read( filename ) ) return( false );

    // <KVSML>
    vismodule::kvsml::KVSMLTag kvsml_tag;
    if ( !kvsml_tag.read( &document ) ) return( false );

    // <Object>
    vismodule::kvsml::ObjectTag object_tag;
    if ( !object_tag.read( kvsml_tag.node() ) ) return( false );

    if ( object_tag.type() != "StructuredVolumeObject" ) return( false );

    // <StructuredVolumeObject>
    vismodule::kvsml::StructuredVolumeObjectTag volume_tag;
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

} // end of namespace vismodule
