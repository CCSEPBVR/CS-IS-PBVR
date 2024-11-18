/****************************************************************************/
/**
 *  @file KVSMLObjectUnstructuredVolume.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectUnstructuredVolume.cpp 848 2011-06-29 11:35:52Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "KVSMLObjectUnstructuredVolume.h"
#include "UnstructuredVolumeObjectTag.h"
#include "NodeTag.h"
#include "CellTag.h"
#include "ValueTag.h"
#include "CoordTag.h"
#include "ConnectionTag.h"
#include "DataArrayTag.h"
#include <kvs/File>
#include <kvs/XMLDocument>
#include <kvs/XMLDeclaration>
#include <kvs/XMLElement>
#include <kvs/XMLComment>
#include <kvs/AnyValueArray>
#include <kvs/Type>
#include <kvs/IgnoreUnusedVariable>


namespace
{

/*===========================================================================*/
/**
 *  @brief  Returns the number of nodes per element.
 *  @param  cell_type [in] cell type as string
 *  @return number of nodes per element
 */
/*===========================================================================*/
inline const size_t GetNumberOfNodesPerElement( const std::string& cell_type )
{
    size_t nnodes = 0;
    if ( cell_type == "tetrahedra" ) nnodes = 4;
    else if ( cell_type == "quadratic tetrahedra" ) nnodes = 10;
    else if ( cell_type == "hexahedra" ) nnodes = 8;
    else if ( cell_type == "quadratic hexahedra" ) nnodes = 20;
    else if ( cell_type == "pyramid" ) nnodes = 5;
    else if ( cell_type == "point" ) nnodes = 1;

    return( nnodes );
}

} // end of namespace


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML object unstructured volume object class.
 */
/*===========================================================================*/
KVSMLObjectUnstructuredVolume::KVSMLObjectUnstructuredVolume( void ):
    m_writing_type( kvs::KVSMLObjectUnstructuredVolume::Ascii )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML object unstructured volume object class by reading the given file.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
KVSMLObjectUnstructuredVolume::KVSMLObjectUnstructuredVolume( const std::string& filename ):
    m_writing_type( kvs::KVSMLObjectUnstructuredVolume::Ascii )
{
    if( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*===========================================================================*/
/**
 *  @brief  Destructs the KVSML object unstructured volume object class.
 */
/*===========================================================================*/
KVSMLObjectUnstructuredVolume::~KVSMLObjectUnstructuredVolume( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the KVSML tag.
 *  @return KVSML tag
 */
/*===========================================================================*/
const kvs::kvsml::KVSMLTag& KVSMLObjectUnstructuredVolume::KVSMLTag( void ) const
{
    return( m_kvsml_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns the object tag.
 *  @return object tag
 */
/*===========================================================================*/
const kvs::kvsml::ObjectTag& KVSMLObjectUnstructuredVolume::objectTag( void ) const
{
    return( m_object_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns the cell type as string.
 *  @return cell type
 */
/*===========================================================================*/
const std::string& KVSMLObjectUnstructuredVolume::cellType( void ) const
{
    return( m_cell_type );
}

const bool KVSMLObjectUnstructuredVolume::hasLabel( void ) const
{
    return( m_has_label );
}

const std::string& KVSMLObjectUnstructuredVolume::label( void ) const
{
    return( m_label );
}

/*===========================================================================*/
/**
 *  @brief  Returns the vector length.
 *  @return vector length
 */
/*===========================================================================*/
const size_t KVSMLObjectUnstructuredVolume::veclen( void ) const
{
    return( m_veclen );
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of nodes.
 *  @return number of nodes
 */
/*===========================================================================*/
const size_t KVSMLObjectUnstructuredVolume::nnodes( void ) const
{
    return( m_nnodes );
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of cells.
 *  @return number of cells
 */
/*===========================================================================*/
const size_t KVSMLObjectUnstructuredVolume::ncells( void ) const
{
    return( m_ncells );
}

const bool KVSMLObjectUnstructuredVolume::hasMinValue( void ) const
{
    return( m_has_min_value );
}

const bool KVSMLObjectUnstructuredVolume::hasMaxValue( void ) const
{
    return( m_has_max_value );
}

const double KVSMLObjectUnstructuredVolume::minValue( void ) const
{
    return( m_min_value );
}

const double KVSMLObjectUnstructuredVolume::maxValue( void ) const
{
    return( m_max_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the value array.
 *  @return value array
 */
/*===========================================================================*/
const kvs::AnyValueArray& KVSMLObjectUnstructuredVolume::values( void ) const
{
    return( m_values );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate array.
 *  @return coordinate array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& KVSMLObjectUnstructuredVolume::coords( void ) const
{
    return( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns the connection array.
 *  @return connection array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& KVSMLObjectUnstructuredVolume::connections( void ) const
{
    return( m_connections );
}

/*===========================================================================*/
/**
 *  @brief  Sets a writing data type.
 *  @param  writing_type [in] writing data type
 */
/*===========================================================================*/
void KVSMLObjectUnstructuredVolume::setWritingDataType( const WritingDataType writing_type )
{
    m_writing_type = writing_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets a cell type.
 *  @param  cell_type [in] cell type
 */
/*===========================================================================*/
void KVSMLObjectUnstructuredVolume::setCellType( const std::string& cell_type )
{
    m_cell_type = cell_type;
}

void KVSMLObjectUnstructuredVolume::setLabel( const std::string& label )
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
void KVSMLObjectUnstructuredVolume::setVeclen( const size_t veclen )
{
    m_veclen = veclen;
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of nodes.
 *  @param  nnodes [in] number of nodes
 */
/*===========================================================================*/
void KVSMLObjectUnstructuredVolume::setNNodes( const size_t nnodes )
{
    m_nnodes = nnodes;
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of cells.
 *  @param  ncells [in] number of cells
 */
/*===========================================================================*/
void KVSMLObjectUnstructuredVolume::setNCells( const size_t ncells )
{
    m_ncells = ncells;
}

void KVSMLObjectUnstructuredVolume::setMinValue( const double min_value )
{
    m_has_min_value = true;
    m_min_value = min_value;
}

void KVSMLObjectUnstructuredVolume::setMaxValue( const double max_value )
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
void KVSMLObjectUnstructuredVolume::setValues( const kvs::AnyValueArray& values )
{
    m_values = values;
}

/*===========================================================================*/
/**
 *  @brief  Sets a coordinate array.
 *  @param  coords [in] coordinate array
 */
/*===========================================================================*/
void KVSMLObjectUnstructuredVolume::setCoords( const kvs::ValueArray<kvs::Real32>& coords )
{
    m_coords = coords;
}

/*===========================================================================*/
/**
 *  @brief  Sets a connection array.
 *  @param  connections [in] connection array
 */
/*===========================================================================*/
void KVSMLObjectUnstructuredVolume::setConnections( const kvs::ValueArray<kvs::UInt32>& connections )
{
    m_connections = connections;
}

/*===========================================================================*/
/**
 *  @brief  Read a KVSML unstructured volume object from the given file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool KVSMLObjectUnstructuredVolume::read( const std::string& filename )
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

    // <UnstructuredVolumeObject>
    kvs::kvsml::UnstructuredVolumeObjectTag volume_tag;
    if ( !volume_tag.read( m_object_tag.node() ) )
    {
        kvsMessageError( "Cannot read <%s>.", volume_tag.name().c_str() );
        return( false );
    }

    if ( !volume_tag.hasCellType() )
    {
        kvsMessageError( "'cell_type' is not specified in <%s>.", volume_tag.name().c_str() );
        return( false );
    }
    m_cell_type = volume_tag.cellType();

    // <Node>
    kvs::kvsml::NodeTag node_tag;
    if ( !node_tag.read( volume_tag.node() ) )
    {
        kvsMessageError( "Cannot read <%s>.", node_tag.name().c_str() );
        return( false );
    }

    if ( !node_tag.hasNNodes() )
    {
        kvsMessageError( "'nnodes' is not specified in <%s>.", node_tag.name().c_str() );
        return( false );
    }
    m_nnodes = node_tag.nnodes();

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
    const size_t value_nelements = m_nnodes * m_veclen;
    kvs::kvsml::DataArrayTag values;
    if ( !values.read( value_tag.node(), value_nelements, &m_values ) )
    {
        kvsMessageError( "Cannot read <%s> for <%s>.",
                         values.name().c_str(),
                         value_tag.name().c_str() );
        return( false );
    }

    // <Coord>
    kvs::kvsml::CoordTag coord_tag;
    if ( !coord_tag.read( node_tag.node() ) )
    {
        kvsMessageError( "Cannot read <%s>.", coord_tag.name().c_str() );
        return( false );
    }

    // <DataArray>
    const size_t dimension = 3;
    const size_t coord_nelements = m_nnodes * dimension;
    kvs::kvsml::DataArrayTag coords;
    if ( !coords.read( coord_tag.node(), coord_nelements, &m_coords ) )
    {
        kvsMessageError( "Cannot read <%s> for <%s>.",
                         coords.name().c_str(),
                         coord_tag.name().c_str() );
        return( false );
    }

    // <Cell>
    kvs::kvsml::CellTag cell_tag;
    if ( !cell_tag.read( volume_tag.node() ) )
    {
        kvsMessageError( "Cannot read <%s>.", cell_tag.name().c_str() );
        return( false );
    }

    if ( !cell_tag.hasNCells() )
    {
        kvsMessageError( "'ncells' is not specified in <%s>.", cell_tag.name().c_str() );
        return( false );
    }
    m_ncells = cell_tag.ncells();

    // <Connection>
    kvs::kvsml::ConnectionTag connection_tag;
    if ( !connection_tag.read( cell_tag.node() ) )
    {
        kvsMessageError( "Cannot read <%s>.", connection_tag.name().c_str() );
        return( false );
    }

    // <DataArray>
    const size_t nnodes_per_element = ::GetNumberOfNodesPerElement( m_cell_type );
    const size_t connection_nelements = m_ncells * nnodes_per_element;
    kvs::kvsml::DataArrayTag connections;
    if ( !connections.read( connection_tag.node(), connection_nelements, &m_connections ) )
    {
        kvsMessageError( "Cannot read <%s> for <%s>.",
                         connections.name().c_str(),
                         connection_tag.name().c_str() );
        return( false );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the KVSML unstructured volume object to the given file.
 *  @param  filename [in] filename
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool KVSMLObjectUnstructuredVolume::write( const std::string& filename )
{
    m_filename = filename;

    kvs::XMLDocument document;
    document.InsertEndChild( kvs::XMLDeclaration("1.0") );
    document.InsertEndChild( kvs::XMLComment(" Generated by kvs::KVSMLObjectStructuredVolume::write() ") );

    // <KVSML>
    kvs::kvsml::KVSMLTag kvsml_tag;
    if ( !kvsml_tag.write( &document ) )
    {
        kvsMessageError( "Cannot write <%s>.", m_kvsml_tag.name().c_str() );
        return( false );
    }

    // <Object type="UnstructuredVolumeObject">
    kvs::kvsml::ObjectTag object_tag;
    object_tag.setType( "UnstructuredVolumeObject" );
    if ( !object_tag.write( kvsml_tag.node() ) )
    {
        kvsMessageError( "Cannot write <%s>.", object_tag.name().c_str() );
        return( false );
    }

    // <UnstructuredVolumeObject cell_type="xxx">
    kvs::kvsml::UnstructuredVolumeObjectTag volume_tag;
    volume_tag.setCellType( m_cell_type );
    if ( !volume_tag.write( object_tag.node() ) )
    {
        kvsMessageError( "Cannot write <%s>.", volume_tag.name().c_str() );
        return( false );
    }

    // <Node nnodes="xxx">
    kvs::kvsml::NodeTag node_tag;
    node_tag.setNNodes( m_nnodes );
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

    // <Cell ncells="xxx">
    kvs::kvsml::CellTag cell_tag;
    cell_tag.setNCells( m_ncells );
    if ( !cell_tag.write( volume_tag.node() ) )
    {
        kvsMessageError( "Cannot write <%s>.", cell_tag.name().c_str() );
        return( false );
    }

    // <Connection>
    kvs::kvsml::ConnectionTag connection_tag;
    if ( !connection_tag.write( cell_tag.node() ) )
    {
        kvsMessageError( "Cannot write <%s>.", connection_tag.name().c_str() );
        return( false );
    }

    // <DataArray>
    kvs::kvsml::DataArrayTag connections;
    if ( m_writing_type == ExternalAscii )
    {
        connections.setFile( kvs::kvsml::DataArray::GetDataFilename( m_filename, "connect" ) );
        connections.setFormat( "ascii" );
    }
    else if ( m_writing_type == ExternalBinary )
    {
        connections.setFile( kvs::kvsml::DataArray::GetDataFilename( m_filename, "connect" ) );
        connections.setFormat( "binary" );
    }

    if ( !connections.write( connection_tag.node(), m_connections, pathname ) )
    {
        kvsMessageError( "Cannot write <%s> for <%s>.",
                         connections.name().c_str(),
                         connection_tag.name().c_str() );
        return( false );
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
const bool KVSMLObjectUnstructuredVolume::CheckFileExtension( const std::string& filename )
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
 *  @return true, if the KVSMLObjectUnstructuredVolume class can read the given file
 */
/*===========================================================================*/
const bool KVSMLObjectUnstructuredVolume::CheckFileFormat( const std::string& filename )
{
    kvs::XMLDocument document;
    if ( !document.read( filename ) ) return( false );

    // <KVSML>
    kvs::kvsml::KVSMLTag kvsml_tag;
    if ( !kvsml_tag.read( &document ) ) return( false );

    // <Object>
    kvs::kvsml::ObjectTag object_tag;
    if ( !object_tag.read( kvsml_tag.node() ) ) return( false );

    if ( object_tag.type() != "UnstructuredVolumeObject" ) return( false );

    // <UnstructuredVolumeObject>
    kvs::kvsml::UnstructuredVolumeObjectTag volume_tag;
    if ( !volume_tag.read( object_tag.node() ) ) return( false );

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Output operator.
 *  @param  os [out] output stream
 *  @param  rhs [in] KVSML unstructured volume object
 */
/*===========================================================================*/
std::ostream& operator <<( std::ostream& os, const KVSMLObjectUnstructuredVolume& rhs )
{
    os << "Cell type: " << rhs.m_cell_type << std::endl;
    os << "Veclen: " << rhs.m_veclen << std::endl;
    os << "Num. of nodes: " << rhs.m_nnodes << std::endl;
    os << "Num. of cells: " << rhs.m_ncells << std::endl;
    os << "Value type: " << rhs.m_values.typeInfo()->typeName();

    return( os );
}

} // end of namespace kvs
