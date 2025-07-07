// FBX.h
#ifndef KVS__FBX_H_INCLUDE
#define KVS__FBX_H_INCLUDE

#include <iostream>
#include <string>
#include <kvs/ValueArray>
#include <kvs/FileFormatBase>
#include <kvs/Type>
#include <kvs/Indent>

// ASSIMP headers
#include <assimp/scene.h>

namespace kvs
{
class FBX : public kvs::FileFormatBase
{
public:
    typedef kvs::FileFormatBase BaseClass;
    enum FileType
    {
        Ascii,  ///< ascii type
        Binary  ///< binary type
    };

protected:
    FileType m_file_type; ///< file type
    kvs::ValueArray<kvs::Real32> m_normals;         ///< normal vector array
    kvs::ValueArray<kvs::Real32> m_coords;          ///< coordinate value array
    kvs::ValueArray<kvs::UInt32> m_connections;     ///< polygon's vertex connection array
    kvs::ValueArray<kvs::Real32> m_texture2DCoords; ///< texture UV coordinate array
    kvs::ValueArray<kvs::UInt32> m_textureIds;      ///< texture ID per vertex
    kvs::ValueArray<kvs::ValueArray<kvs::UInt8>> m_color_arrays; ///< pixel data arrays
    kvs::ValueArray<kvs::UInt32> m_image_widths;    ///< image widths
    kvs::ValueArray<kvs::UInt32> m_image_heights;   ///< image heights

public:
    static bool CheckExtension( const std::string& filename );

    FBX();
    FBX( const std::string& filename );
    virtual ~FBX();

    FileType fileType() const { return m_file_type; }
    const kvs::ValueArray<kvs::Real32>& normals() const { return m_normals; }
    const kvs::ValueArray<kvs::Real32>& coords() const { return m_coords; }
    const kvs::ValueArray<kvs::UInt32>& connections() const { return m_connections; }
    const kvs::ValueArray<kvs::Real32>& texture2DCoords() const { return m_texture2DCoords; }
    const kvs::ValueArray<kvs::UInt32>& textureIds() const { return m_textureIds; }
    const kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>& colorArrays() const { return m_color_arrays; }
    const kvs::ValueArray<kvs::UInt32>& imageWidths() const { return m_image_widths; }
    const kvs::ValueArray<kvs::UInt32>& imageHeights() const { return m_image_heights; }
    size_t numberOfTriangles() const { return m_normals.size() / 3; }

    void setFileType( const FileType file_type ) { m_file_type = file_type; }
    void setNormals( const kvs::ValueArray<kvs::Real32>& normals ) { m_normals = normals; }
    void setCoords( const kvs::ValueArray<kvs::Real32>& coords ) { m_coords = coords; }
    void setConnections( const kvs::ValueArray<kvs::UInt32>& connections ) { m_connections = connections; }
    void setTexture2DCoords( const kvs::ValueArray<kvs::Real32>& uv ) { m_texture2DCoords = uv; }
    void setTextureIds( const kvs::ValueArray<kvs::UInt32>& ids ) { m_textureIds = ids; }
    void setColorArrays( const kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>& colors ) { m_color_arrays = colors; }
    void setImageWidths( const kvs::ValueArray<kvs::UInt32>& widths ) { m_image_widths = widths; }
    void setImageHeights( const kvs::ValueArray<kvs::UInt32>& heights ) { m_image_heights = heights; }

    void print( std::ostream& os, const kvs::Indent& indent = kvs::Indent(0) ) const;
    bool read( const std::string& filename );
    bool write( const std::string& filename );
};
}

#endif // KVS__FBX_H_INCLUDE
