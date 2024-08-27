#ifndef THREEDS_H
#define THREEDS_H

#include <iostream>
#include <fstream>
#include <string>
#include <kvs/ValueArray>
#include <kvs/FileFormatBase>
#include <kvs/Type>
#include <kvs/Indent>
#include <kvs/Deprecated>

namespace kvs
{
class ThreeDS : public kvs::FileFormatBase
{
public:

    typedef kvs::FileFormatBase BaseClass;
    enum FileType
    {
        Ascii, // ascii type
        Binary // binary type
    };

protected:
    FileType m_file_type; ///< file type
    kvs::ValueArray<kvs::Real32> m_normals; /// normal vector array
    kvs::ValueArray<kvs::Real32> m_coords; /// coordinate value array
    kvs::ValueArray<kvs::UInt32> m_connections; /// polygon's vertexes connection array
    kvs::ValueArray<kvs::Real32> m_texture2DCoords; /// texture 2d coordinate (UV coordinate) value array
    kvs::ValueArray<kvs::UInt32> m_textureIds; /// texture ID for each vertex
    kvs::ValueArray<kvs::ValueArray<kvs::UInt8>> m_color_arrays; /// color component array
    kvs::ValueArray<kvs::UInt32> m_image_widths; /// image width
    kvs::ValueArray<kvs::UInt32> m_image_heights; /// image height

public:
    static bool CheckExtension( const std::string& filename );

public:
    ThreeDS();
    ThreeDS( const std::string& filename );
    virtual ~ThreeDS();

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
    void setTexture2DCoords( const kvs::ValueArray<kvs::Real32>& texture2DCoords ) { m_texture2DCoords = texture2DCoords; }
    void setTextureIds( const kvs::ValueArray<kvs::UInt32>& textureIds ) { m_textureIds = textureIds; }
    void setColorArrays( const kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>& color_arrays ) { m_color_arrays = color_arrays; }
    void setImageWidths( const kvs::ValueArray<kvs::UInt32>& image_width ) { m_image_widths = image_width; }
    void setImageHeights( const kvs::ValueArray<kvs::UInt32>& image_height ) { m_image_heights = image_height; }

    void print( std::ostream& os, const kvs::Indent& indent = kvs::Indent(0) ) const;
    bool read( const std::string& filename );
    bool write( const std::string& filename );

private:
    bool is_ascii_type( FILE* ifs );
    bool read_ascii( FILE* ifs );
    bool read_binary( FILE* ifs );
    bool write_ascii( FILE* ifs );
    bool write_binary( FILE* ifs );
};

}
#endif
