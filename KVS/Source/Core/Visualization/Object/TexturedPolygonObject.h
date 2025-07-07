/****************************************************************************/
/**
 *  @file   PolygonObject.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonObject.h 1815 2014-10-02 01:17:28Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#pragma once
#include <map>
#include <ostream>
#include <kvs/GeometryObjectBase>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector3>
#include <kvs/RGBColor>
#include <kvs/Module>
#include <kvs/Indent>
#include <kvs/Deprecated>
#include <kvs/ColorImage>
//#include <kvs/Texture2D>

namespace kvs
{
/*==========================================================================*/
/**
 *  Polygon object class.
 */
/*==========================================================================*/
class TexturedPolygonObject : public kvs::GeometryObjectBase
{
    kvsModule( kvs::TexturedPolygonObject, Object );
    kvsModuleBaseClass( kvs::GeometryObjectBase );

public:

    enum PolygonType
    {
        Triangle   = 3,
        Quadrangle = 4,
        Tri        = Triangle,
        Quad       = Quadrangle,
        UnknownPolygonType
    };

    enum ColorType
    {
        VertexColor  = 0,
        PolygonColor = 1,
        UnknownColorType
    };

    enum NormalType
    {
        VertexNormal  = 0,
        PolygonNormal = 1,
        UnknownNormalType
    };

private:

    PolygonType m_polygon_type; ///< polygon type
    ColorType m_color_type; ///< polygon color type
    NormalType m_normal_type; ///< polygon normal type
    kvs::ValueArray<kvs::UInt32> m_connections; ///< connection array
    kvs::ValueArray<kvs::UInt8> m_opacities; ///< opacity array
    kvs::ValueArray<kvs::Real32> m_texture_2d_coords; ///< texture 2d coord array
    kvs::ValueArray<kvs::UInt32> m_texture_ids; ///< texture index

    std::map<kvs::UInt32, kvs::ValueArray<kvs::UInt8>> m_map_id_to_color_array; ///< map texture id to color component array of RGBA
    std::map<kvs::UInt32, kvs::UInt32> m_map_id_to_image_width; ///< map texture id to image width
    std::map<kvs::UInt32, kvs::UInt32> m_map_id_to_image_height; ///< map texture id to image height

public:

    TexturedPolygonObject();

    void shallowCopy( const TexturedPolygonObject& object );
    void deepCopy( const TexturedPolygonObject& object );
    void clear();
    void print( std::ostream& os, const kvs::Indent& indent = kvs::Indent(0) ) const;
    bool read( const std::string& filename ){return false;};
    bool write( const std::string& filename, const bool ascii = true, const bool external = false ) const{return false;};

    void setPolygonType( const PolygonType polygon_type ) { m_polygon_type = polygon_type; }
    void setPolygonTypeToTriangle() { this->setPolygonType( Triangle ); }
    void setPolygonTypeToQuadrangle() { this->setPolygonType( Quadrangle ); }
    void setColorType( const ColorType color_type ) { m_color_type = color_type; }
    void setColorTypeToVertex() { this->setColorType( VertexColor ); }
    void setColorTypeToPolygon() { this->setColorType( PolygonColor ); }
    void setNormalType( const NormalType normal_type ) { m_normal_type = normal_type; }
    void setNormalTypeToVertex() { this->setNormalType( VertexNormal ); }
    void setNormalTypeToPolygon() { this->setNormalType( PolygonNormal ); }
    void setConnections( const kvs::ValueArray<kvs::UInt32>& connections ) { m_connections = connections; }
    void setOpacities( const kvs::ValueArray<kvs::UInt8>& opacities ) { m_opacities = opacities; }

    void setTexture2DCoords( const kvs::ValueArray<kvs::Real32>& texture2DCoords ) { m_texture_2d_coords = texture2DCoords; }
    void setTextureIds( const kvs::ValueArray<kvs::UInt32>& textureIds ) { m_texture_ids = textureIds; }

    void addColorArray( const kvs::UInt32 id, const kvs::ValueArray<kvs::UInt8>& color_array,
                       const kvs::UInt32 width, const kvs::UInt32 height );

    void setColor( const kvs::RGBColor& color );
    void setOpacity( const kvs::UInt8 opacity );

    PolygonType polygonType() const { return m_polygon_type; }
    ColorType colorType() const { return m_color_type; }
    NormalType normalType() const { return m_normal_type; }
    size_t numberOfConnections() const;
    size_t numberOfOpacities() const { return m_opacities.size(); }
    kvs::UInt8 opacity( const size_t index = 0 ) const { return m_opacities[index]; }

    const kvs::ValueArray<kvs::UInt32>& connections() const { return m_connections; }
    const kvs::ValueArray<kvs::UInt8>& opacities() const { return m_opacities; }
    const kvs::ValueArray<kvs::Real32>& texture2DCoords() const { return m_texture_2d_coords; }
    const kvs::ValueArray<kvs::UInt32>& textureIds() const { return m_texture_ids; }

    const std::map<kvs::UInt32, kvs::ValueArray<kvs::UInt8>>& mapIdToColorArray() const { return m_map_id_to_color_array; }
    const std::map<kvs::UInt32, kvs::UInt32>& mapIdToImageWidth() const { return m_map_id_to_image_width; }
    const std::map<kvs::UInt32, kvs::UInt32>& mapIdToImageHeight() const { return m_map_id_to_image_height; }

private:

    void flip( const kvs::UInt32 id );

};

} // end of namespace kvs
