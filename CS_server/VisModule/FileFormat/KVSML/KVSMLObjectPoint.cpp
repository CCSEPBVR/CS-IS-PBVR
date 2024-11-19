/****************************************************************************/
/**
 *  @file KVSMLObjectPoint.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectPoint.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "KVSMLObjectPoint.h"
#include "PointObjectTag.h"
#include "VertexTag.h"
#include "CoordTag.h"
#include "ColorTag.h"
#include "NormalTag.h"
#include "SizeTag.h"
#include "DataArrayTag.h"
#include "DataValueTag.h"
#include "DataReader.h"
#include "DataWriter.h"
#include <vismodule/XMLDocument>
#include <vismodule/XMLDeclaration>
#include <vismodule/XMLElement>
#include <vismodule/XMLComment>
#include <vismodule/ValueArray>
#include <vismodule/File>
#include <vismodule/Type>
#include <vismodule/File>
#include <vismodule/IgnoreUnusedVariable>
#include <iostream>
#include <fstream>
#include <sstream>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML point object class.
 */
/*===========================================================================*/
KVSMLObjectPoint::KVSMLObjectPoint( void ):
    m_writing_type( vismodule::KVSMLObjectPoint::Ascii )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML point object class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
KVSMLObjectPoint::KVSMLObjectPoint( const std::string& filename ):
    m_writing_type( vismodule::KVSMLObjectPoint::Ascii )
{
    if ( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*===========================================================================*/
/**
 *  @brief  Destructs the KVSML point object class.
 */
/*===========================================================================*/
KVSMLObjectPoint::~KVSMLObjectPoint( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the KVSML tag.
 *  @return KVSML tag
 */
/*===========================================================================*/
const vismodule::kvsml::KVSMLTag& KVSMLObjectPoint::KVSMLTag( void ) const
{
    return( m_kvsml_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns the object tag.
 *  @return object tag
 */
/*===========================================================================*/
const vismodule::kvsml::ObjectTag& KVSMLObjectPoint::objectTag( void ) const
{
    return( m_object_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate value array.
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& KVSMLObjectPoint::coords( void ) const
{
    return( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns the color value array.
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::UInt8>& KVSMLObjectPoint::colors( void ) const
{
    return( m_colors );
}

/*===========================================================================*/
/**
 *  @brief  Returns the normal vector array.
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& KVSMLObjectPoint::normals( void ) const
{
    return( m_normals );
}

/*===========================================================================*/
/**
 *  @brief  Returns the size value array.
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& KVSMLObjectPoint::sizes( void ) const
{
    return( m_sizes );
}

/*===========================================================================*/
/**
 *  @brief  Set a writing type.
 *  @param  writing_type [in] writing type
 */
/*===========================================================================*/
void KVSMLObjectPoint::setWritingDataType( const WritingDataType writing_type )
{
    m_writing_type = writing_type;
}

/*===========================================================================*/
/**
 *  @brief  Set a coordinate value array.
 *  @param  coords [in] coordinate value array
 */
/*===========================================================================*/
void KVSMLObjectPoint::setCoords( const vismodule::ValueArray<vismodule::Real32>& coords )
{
    m_coords = coords;
}

/*===========================================================================*/
/**
 *  @brief  Set a color value array.
 *  @param  colors [in] color value array
 */
/*===========================================================================*/
void KVSMLObjectPoint::setColors( const vismodule::ValueArray<vismodule::UInt8>& colors )
{
    m_colors = colors;
}

/*===========================================================================*/
/**
 *  @brief  Set a normal vector array.
 *  @param  normals [in] normal vector array
 */
/*===========================================================================*/
void KVSMLObjectPoint::setNormals( const vismodule::ValueArray<vismodule::Real32>& normals )
{
    m_normals = normals;
}

/*===========================================================================*/
/**
 *  @brief  Set a size value array.
 *  @param  sizes [in] size value array
 */
/*===========================================================================*/
void KVSMLObjectPoint::setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes )
{
    m_sizes = sizes;
}

/*===========================================================================*/
/**
 *  @brief  Read a KVSML point object file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is successfully
 */
/*===========================================================================*/
const bool KVSMLObjectPoint::read( const std::string& filename )
{
    m_filename = filename;

    // XML document.
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

    // <PointObject>
    vismodule::kvsml::PointObjectTag point_tag;
    if ( !point_tag.read( m_object_tag.node() ) )
    {
        visModuleMessageError( "Cannot read <%s>.", point_tag.name().c_str() );
        return( false );
    }

    // <Vertex>
    vismodule::kvsml::VertexTag vertex_tag;
    if ( !vertex_tag.read( point_tag.node() ) )
    {
        visModuleMessageError( "Cannot read <%s>.", vertex_tag.name().c_str() );
        return( false );
    }
    else
    {
        // Parent node.
        const vismodule::XMLNode::SuperClass* parent = vertex_tag.node();
        
        // <Coord>
        const size_t ncoords = vertex_tag.nvertices();
        
        if( vertex_tag.nvertices() == 0 ) return( true );

        if ( !vismodule::kvsml::ReadCoordData( parent, ncoords, &m_coords ) ) return( false );
        if ( m_coords.size() == 0 )
        {
            visModuleMessageError( "Cannot read the coord data." );
            return( false );
        }

        // <Color>
        const size_t ncolors = vertex_tag.nvertices();
        if ( !vismodule::kvsml::ReadColorData( parent, ncolors, &m_colors ) ) return( false );
        if ( m_colors.size() == 0 )
        {
            // default value (black).
            m_colors.allocate(3);
            m_colors.at(0) = 0;
            m_colors.at(1) = 0;
            m_colors.at(2) = 0;
        }

        // <Normal>
        const size_t nnormals = vertex_tag.nvertices();
        if ( !vismodule::kvsml::ReadNormalData( parent, nnormals, &m_normals ) ) return( false );

        // <Size>
        const size_t nsizes = vertex_tag.nvertices();
        if ( !vismodule::kvsml::ReadSizeData( parent, nsizes, &m_sizes ) ) return( false );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the KVSML point object.
 *  @param  filename [in] filename
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool KVSMLObjectPoint::write( const std::string& filename )
{
    m_filename = filename;

    vismodule::XMLDocument document;
    document.InsertEndChild( vismodule::XMLDeclaration("1.0") );
    document.InsertEndChild( vismodule::XMLComment(" Generated by vismodule::KVSMLObjectPoint::write() ") );

    // <KVSML>
    vismodule::kvsml::KVSMLTag kvsml_tag;
    if ( !kvsml_tag.write( &document ) )
    {
        visModuleMessageError( "Cannot write <%s>.", kvsml_tag.name().c_str() );
        return( false );
    }

    // <Object type="PointObject">
    vismodule::kvsml::ObjectTag object_tag;
    object_tag.setType( "PointObject" );
    if ( !object_tag.write( kvsml_tag.node() ) )
    {
        visModuleMessageError( "Cannot write <%s>.", object_tag.name().c_str() );
        return( false );
    }

    // <PointObject>
    vismodule::kvsml::PointObjectTag point_tag;
    if ( !point_tag.write( object_tag.node() ) )
    {
        visModuleMessageError( "Cannot write <%s>.", point_tag.name().c_str() );
        return( false );
    }

    // <Vertex nvertices="xxx">
    const size_t dimension = 3;
    vismodule::kvsml::VertexTag vertex_tag;
    vertex_tag.setNVertices( m_coords.size() / dimension );
    if ( !vertex_tag.write( point_tag.node() ) )
    {
        visModuleMessageError( "Cannot write <%s>.", vertex_tag.name().c_str() );
        return( false );
    }
    else
    {
        // Parent node and writing data type.
        vismodule::XMLNode::SuperClass* parent = vertex_tag.node();
        const vismodule::kvsml::WritingDataType type = static_cast<vismodule::kvsml::WritingDataType>(m_writing_type);

        // <Coord>
        if ( !vismodule::kvsml::WriteCoordData( parent, type, m_filename, m_coords ) ) return( false );

        // <Color>
        if ( !vismodule::kvsml::WriteColorData( parent, type, m_filename, m_colors ) ) return( false );

        // <Normal>
        if ( !vismodule::kvsml::WriteNormalData( parent, type, m_filename, m_normals ) ) return( false );

        // <Size>
        if ( !vismodule::kvsml::WriteSizeData( parent, type, m_filename, m_sizes ) ) return( false );
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
const bool KVSMLObjectPoint::CheckFileExtension( const std::string& filename )
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
 *  @return true, if the KVSMLObjectPoint class can read the given file
 */
/*===========================================================================*/
const bool KVSMLObjectPoint::CheckFileFormat( const std::string& filename )
{
    vismodule::XMLDocument document;
    if ( !document.read( filename ) ) return( false );

    // <KVSML>
    vismodule::kvsml::KVSMLTag kvsml_tag;
    if ( !kvsml_tag.read( &document ) ) return( false );

    // <Object>
    vismodule::kvsml::ObjectTag object_tag;
    if ( !object_tag.read( kvsml_tag.node() ) ) return( false );
    if ( object_tag.type() != "PointObject" ) return( false );

    // <PointObject>
    vismodule::kvsml::PointObjectTag point_tag;
    if ( !point_tag.read( object_tag.node() ) ) return( false );

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Output operator.
 *  @param  os [out] output stream
 *  @param  rhs [in] KVSML point object
 */
/*===========================================================================*/
std::ostream& operator <<( std::ostream& os, const KVSMLObjectPoint& rhs )
{
    os << "Num. of vertices: " << rhs.m_coords.size() / 3;

    return( os );
}

} // end of namespace vismodule
