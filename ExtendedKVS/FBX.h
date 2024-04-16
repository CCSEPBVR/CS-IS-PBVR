#ifndef _FBX_H_
#define _FBX_H_

//#ifdef CGFORMATEXT4KVS_SUPPORT_FBXSDK

#include <Core/FileFormat/FileFormatBase.h>
#include <kvs/ValueArray>
#include <kvs/ColorImage>
//#include <kvs/Texture2D>

namespace kvs{
namespace jaea{

class FBX : public kvs::FileFormatBase{
protected:
    kvs::ValueArray<kvs::Real32> m_coords; /// coordinate value array
    kvs::ValueArray<kvs::Real32> m_normals; /// normal vector array
    kvs::ValueArray<kvs::UInt32> m_connections; /// polygon's vertexes connection array
    kvs::ValueArray<kvs::Real32> m_texture2DCoords; /// texture 2d coordinate (UV coordinate) value array
    kvs::ValueArray<kvs::UInt32> m_textureIds; /// texture ID for each vertex
    kvs::ValueArray<kvs::ValueArray<kvs::UInt8>> m_color_arrays; /// color component array
    kvs::ValueArray<kvs::UInt32> m_image_widths; /// image width
    kvs::ValueArray<kvs::UInt32> m_image_heights; /// image height

public:
    virtual ~FBX(){};

    virtual bool read( const std::string& filename ) override;
    virtual bool write( const std::string& filename ) override;

    const kvs::ValueArray<kvs::Real32>& coords() const { return m_coords; }
    const kvs::ValueArray<kvs::Real32>& normals() const { return m_normals; }
    const kvs::ValueArray<kvs::UInt32>& connections() const { return m_connections; }
    const kvs::ValueArray<kvs::Real32>& texture2DCoords() const { return m_texture2DCoords; }
    const kvs::ValueArray<kvs::UInt32>& textureIds() const { return m_textureIds; }
    const kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>& colorArrays() const { return m_color_arrays; }
    const kvs::ValueArray<kvs::UInt32>& imageWidths() const { return m_image_widths; }
    const kvs::ValueArray<kvs::UInt32>& imageHeights() const { return m_image_heights; }
};

} // namespace jaea
} // kvs

//#endif // CGFORMATEXT4KVS_SUPPORT_FBXSDK

#endif // _FBX_H_
