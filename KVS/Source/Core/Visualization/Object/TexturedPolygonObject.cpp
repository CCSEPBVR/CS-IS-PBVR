/****************************************************************************/
/**
 *  @file   PolygonObject.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonObject.cpp 1759 2014-05-05 06:14:18Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#include "TexturedPolygonObject.h"
#include <string>
#include <kvs/KVSMLPolygonObject>
#include <kvs/Assert>
#include <kvs/Type>

namespace
{

/*==========================================================================*/
/**
 *  @breif  Returns the polygon type from the given string.
 *  @param  polygon_type [in] polygon type string
 *  @return polygon type
 */
/*==========================================================================*/
kvs::TexturedPolygonObject::PolygonType GetPolygonType( const std::string& polygon_type )
{
    if (      polygon_type == "triangle"   ) { return kvs::TexturedPolygonObject::Triangle; }
    else if ( polygon_type == "quadrangle" ) { return kvs::TexturedPolygonObject::Quadrangle; }
    else
    {
        kvsMessageError( "Unknown polygon type '%s'.", polygon_type.c_str() );
        return kvs::TexturedPolygonObject::UnknownPolygonType;
    }
}

/*==========================================================================*/
/**
 *  @brief  Returns the polygon color type from the given string.
 *  @param  color_type [in] polygon color type string
 *  @return polygon color type
 */
/*==========================================================================*/
kvs::TexturedPolygonObject::ColorType GetColorType( const std::string& color_type )
{
    if (      color_type == "vertex"  ) { return kvs::TexturedPolygonObject::VertexColor; }
    else if ( color_type == "polygon" ) { return kvs::TexturedPolygonObject::PolygonColor; }
    else
    {
        kvsMessageError( "Unknown polygon color type '%s'.", color_type.c_str() );
        return kvs::TexturedPolygonObject::UnknownColorType;
    }
}

/*==========================================================================*/
/**
 *  @brief  Returns the polygon normal type from the given string.
 *  @param  normal_type [in] polygon normal type string
 *  @return polygon normal type
 */
/*==========================================================================*/
kvs::TexturedPolygonObject::NormalType GetNormalType( const std::string& normal_type )
{
    if (      normal_type == "vertex"  ) { return kvs::TexturedPolygonObject::VertexNormal; }
    else if ( normal_type == "polygon" ) { return kvs::TexturedPolygonObject::PolygonNormal; }
    else
    {
        kvsMessageError( "Unknown polygon normal type '%s'.", normal_type.c_str() );
        return kvs::TexturedPolygonObject::UnknownNormalType;
    }
}

/*===========================================================================*/
/**
 *  @brief  Returns the name of the polygon type as string.
 *  @param  type [in] polygon type
 *  @return name of the polygon type
 */
/*===========================================================================*/
const std::string GetPolygonTypeName( const kvs::TexturedPolygonObject::PolygonType type )
{
    switch( type )
    {
    case kvs::TexturedPolygonObject::Triangle: return "triangle";
    case kvs::TexturedPolygonObject::Quadrangle: return "quadrangle";
    default: return "unknown";
    }
}

/*===========================================================================*/
/**
 *  @brief  Returns the name of the color type as string.
 *  @param  type [in] color type
 *  @return name of the color type
 */
/*===========================================================================*/
const std::string GetColorTypeName( const kvs::TexturedPolygonObject::ColorType type )
{
    switch( type )
    {
    case kvs::TexturedPolygonObject::VertexColor: return "vertex";
    case kvs::TexturedPolygonObject::PolygonColor: return "polygon";
    default: return "unknown";
    }
}

/*===========================================================================*/
/**
 *  @brief  Returns the name of the normal type as string.
 *  @param  type [in] normal type
 *  @return name of the normal type
 */
/*===========================================================================*/
const std::string GetNormalTypeName( const kvs::TexturedPolygonObject::NormalType type )
{
    switch( type )
    {
    case kvs::TexturedPolygonObject::VertexNormal: return "vertex";
    case kvs::TexturedPolygonObject::PolygonNormal: return "polygon";
    default: return "unknown";
    }
}

} // end of namespace


namespace kvs
{
/*===========================================================================*/
/**
 *  @brief  Constructs a new TexturedPolygonObject class.
 */
/*===========================================================================*/
TexturedPolygonObject::TexturedPolygonObject()
{
    BaseClass::setGeometryType( Polygon );
    this->setOpacity( 255 );
}

/*===========================================================================*/
/**
 *  @brief  Shallow copys.
 *  @param  object [in] polygon object
 */
/*===========================================================================*/
void TexturedPolygonObject::shallowCopy( const TexturedPolygonObject& object )
{
    BaseClass::shallowCopy( object );
    m_polygon_type = object.polygonType();
    m_color_type = object.colorType();
    m_normal_type = object.normalType();
    m_connections = object.connections();
    m_opacities = object.opacities();
    m_texture_2d_coords = object.texture2DCoords();
    m_texture_ids = object.textureIds();
    m_map_id_to_color_array = object.mapIdToColorArray();
    m_map_id_to_image_width = object.mapIdToImageWidth();
    m_map_id_to_image_height = object.mapIdToImageHeight();
}

/*===========================================================================*/
/**
 *  @brief  Deep copys.
 *  @param  object [in] polygon object
 */
/*===========================================================================*/
void TexturedPolygonObject::deepCopy( const TexturedPolygonObject& object )
{
    BaseClass::deepCopy( object );
    m_polygon_type = object.polygonType();
    m_color_type = object.colorType();
    m_normal_type = object.normalType();
    m_connections = object.connections().clone();
    m_opacities = object.opacities().clone();
    m_texture_2d_coords = object.texture2DCoords().clone();
    m_texture_ids = object.textureIds().clone();
    m_map_id_to_color_array = std::map<kvs::UInt32, kvs::ValueArray<kvs::UInt8>>(object.mapIdToColorArray());
    m_map_id_to_image_width = std::map<kvs::UInt32, kvs::UInt32>(object.mapIdToImageWidth());
    m_map_id_to_image_height = std::map<kvs::UInt32, kvs::UInt32>(object.mapIdToImageHeight());
}

/*===========================================================================*/
/**
 *  @brief  Clears the data of the polygon object.
 */
/*===========================================================================*/
void TexturedPolygonObject::clear()
{
    BaseClass::clear();
    m_connections.release();
    m_opacities.release();
}

/*===========================================================================*/
/**
 *  @brief  Prints information of the polygon object.
 *  @param  os [in] output stream
 *  @param  indent [in] indent
 */
/*===========================================================================*/
void TexturedPolygonObject::print( std::ostream& os, const kvs::Indent& indent ) const
{
    //TODO implement me
    os << indent << "Object type : " << "polygon object" << std::endl;
    BaseClass::print( os, indent );
    os << indent << "Number of connections : " << this->numberOfConnections() << std::endl;
    os << indent << "Number of opacities : " << this->numberOfOpacities() << std::endl;
    os << indent << "Polygon type : " << ::GetPolygonTypeName( this->polygonType() ) << std::endl;
    os << indent << "Color type : " << ::GetColorTypeName( this->colorType() ) << std::endl;
    os << indent << "Normal type : " << ::GetNormalTypeName( this->normalType() ) << std::endl;
}


/*===========================================================================*/
/**
 *  @brief  Sets a color value.
 *  @param  color [in] color value
 */
/*===========================================================================*/
void TexturedPolygonObject::setColor( const kvs::RGBColor& color )
{
    BaseClass::setColor( color );
    m_color_type = TexturedPolygonObject::PolygonColor;
}

/*===========================================================================*/
/**
 *  @brief  Sets an opacity value.
 *  @param  opacity [in] opacity value
 */
/*===========================================================================*/
void TexturedPolygonObject::setOpacity( const kvs::UInt8 opacity )
{
    m_opacities.allocate( 1 );
    m_opacities[0] = opacity;
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of connections.
 *  @return number of connections
 */
/*===========================================================================*/
size_t TexturedPolygonObject::numberOfConnections() const
{
    const size_t nvertices_per_face = m_polygon_type;
    return nvertices_per_face == 0 ? 0 : m_connections.size() / nvertices_per_face;
}

void TexturedPolygonObject::addColorArray( const kvs::UInt32 id, const kvs::ValueArray<kvs::UInt8>& color_array, const kvs::UInt32 width, const kvs::UInt32 height )
{
    m_map_id_to_color_array[id] = color_array;
    m_map_id_to_image_width[id] = width;
    m_map_id_to_image_height[id] = height;
    flip(id);
}

void TexturedPolygonObject::flip( const kvs::UInt32 id )
{
    const kvs::UInt32 width = m_map_id_to_image_width[id];
    const kvs::UInt32 height = m_map_id_to_image_height[id];

    const size_t nChannels = 4;
    const size_t stride = width * nChannels;

    kvs::UInt8* color_array = m_map_id_to_color_array[id].data();
    const size_t end_line = height / 2;
    for ( size_t i = 0; i < end_line; i++ )
    {
        kvs::UInt8* src = color_array + ( i * stride );
        kvs::UInt8* dst = color_array + ( ( height - i - 1 ) * stride );
        for ( size_t j = 0; j < stride; j++ )
        {
            std::swap( *src, *dst );
            src++; dst++;
        }
    }
}

std::ostream& operator << ( std::ostream& os, const TexturedPolygonObject& object )
{
    os << "Object type:  " << "textured polygon object" << std::endl;
#ifdef KVS_COMPILER_VC
#if KVS_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
#endif
#else
    //    os << static_cast<const kvs::GeometryObjectBase&>( object ) << std::endl;
    static_cast<const kvs::GeometryObjectBase&>( object ).print( os );
#endif
    os << "Number of connections:  " << object.numberOfConnections() << std::endl;
    os << "Number of opacities:  " << object.numberOfOpacities() << std::endl;
    os << "Polygon type:  " << ::GetPolygonTypeName( object.polygonType() ) << std::endl;
    os << "Color type:  " << ::GetColorTypeName( object.colorType() ) << std::endl;
    os << "Normal type:  " << ::GetNormalTypeName( object.normalType() );

    return os;
}

} // end of namespace kvs
