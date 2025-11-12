// FBX.h
#ifndef KVS__FBX_H_INCLUDE
#define KVS__FBX_H_INCLUDE

#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <kvs/File>
#include <cassert>
#include <map>
#include <filesystem>

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
    static bool CheckExtension( const std::string& filename )
    {
        const kvs::File file( filename );
        return ( file.extension() == "fbx" );
    }

    FBX()
        : m_file_type( FBX::Binary )
    {
    }

    FBX( const std::string& filename )
        : m_file_type( FBX::Binary )
    {
        this->read( filename );
    }

    virtual ~FBX()
    {
    }

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

    void print( std::ostream& os, const kvs::Indent& indent = kvs::Indent(0) ) const
    {
        os << indent << "Filename : "    << BaseClass::filename() << std::endl;
        os << indent << "File type : "  << ( m_file_type == Ascii ? "ascii" : "binary" ) << std::endl;
        os << indent << "Number of triangles : " << m_normals.size() / 3 << std::endl;
    }

    bool read( const std::string& filename )
    {
        setFilename( filename );

        std::filesystem::path file_path( filename );
        std::filesystem::path base_dir = file_path.parent_path();

        Assimp::Importer importer;
        const unsigned int flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices;
        const aiScene* scene = importer.ReadFile( filename.c_str(), flags );
        if( !scene )
        {
            setSuccess( false );
            std::cerr << "Failed to load model: " << filename << std::endl;
            return false;
        }

        m_file_type = ( CheckExtension( filename ) ? Ascii : Binary );

        std::vector<kvs::Real32> coords;
        std::vector<kvs::Real32> normals;
        std::vector<kvs::UInt32> connections;
        std::vector<kvs::Real32> uvs;
        std::vector<kvs::UInt32> textureIds;

        std::map<std::string,kvs::UInt32> texture_map;
        std::vector<std::filesystem::path> texture_files;

        // メッシュ毎にテクスチャIDを保持
        std::vector<kvs::UInt32> mesh_texture_ids( scene->mNumMeshes );

        for( unsigned int m = 0; m < scene->mNumMeshes; ++m )
        {
            const aiMesh* mesh = scene->mMeshes[m];
            const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

            aiString tex_path;
            kvs::UInt32 tid = 0;
            if( mat->GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
                mat->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path) == AI_SUCCESS )
            {
                std::string rel = tex_path.C_Str();
                auto it = texture_map.find( rel );
                if( it == texture_map.end() )
                {
                    tid = static_cast<kvs::UInt32>( texture_files.size() );
                    texture_map[rel] = tid;
                    texture_files.emplace_back( rel );
                }
                else
                {
                    tid = it->second;
                }
            }
            mesh_texture_ids[m] = tid;
        }

        // ノードを再帰的にトラバースしてデータを収集
        std::function<void(const aiNode*, const aiMatrix4x4&)> traverse =
            [&]( const aiNode* node, const aiMatrix4x4& parent_transform )
        {
            aiMatrix4x4 transform = parent_transform * node->mTransformation;

            for( unsigned int mi = 0; mi < node->mNumMeshes; ++mi )
            {
                unsigned int mesh_idx = node->mMeshes[mi];
                const aiMesh* mesh = scene->mMeshes[mesh_idx];
                size_t base_idx = coords.size() / 3;
                kvs::UInt32 tid = mesh_texture_ids[mesh_idx];

                // 頂点・法線・UV・テクスチャID（頂点単位）
                for( unsigned int v = 0; v < mesh->mNumVertices; ++v )
                {
                    aiVector3D p = transform * mesh->mVertices[v];
                    aiVector3D n = mesh->HasNormals() ? mesh->mNormals[v] : aiVector3D( 0 );
                    coords.push_back( p.x );
                    coords.push_back( p.y );
                    coords.push_back( p.z );
                    normals.push_back( n.x );
                    normals.push_back( n.y );
                    normals.push_back( n.z );

                    if( mesh->HasTextureCoords( 0 ) )
                    {
                        aiVector3D uv = mesh->mTextureCoords[0][v];
                        uvs.push_back( uv.x );
                        uvs.push_back( uv.y );
                    }
                    else
                    {
                        uvs.push_back( 0.0f );
                        uvs.push_back( 0.0f );
                    }

                    textureIds.push_back( tid );
                }

                // 三角フェイスのインデックス
                for( unsigned int f = 0; f < mesh->mNumFaces; ++f )
                {
                    const aiFace& face = mesh->mFaces[f];
                    assert( face.mNumIndices == 3 );
                    for( int k = 0; k < 3; ++k )
                    {
                        unsigned int idx = face.mIndices[k];
                        connections.push_back( static_cast<kvs::UInt32>( base_idx + idx ) );
                    }
                }
            }

            for( unsigned int ci = 0; ci < node->mNumChildren; ++ci )
            {
                traverse(node->mChildren[ci], transform);
            }
        };
        traverse( scene->mRootNode, aiMatrix4x4() );

        m_coords = kvs::ValueArray<kvs::Real32>( coords );
        m_normals = kvs::ValueArray<kvs::Real32>( normals );
        m_connections = kvs::ValueArray<kvs::UInt32>( connections );
        m_texture2DCoords = kvs::ValueArray<kvs::Real32>( uvs );
        m_textureIds = kvs::ValueArray<kvs::UInt32>( textureIds );

        // テクスチャ画像の読み込み
        std::vector<kvs::ValueArray<kvs::UInt8>> color_arrays;
        std::vector<kvs::UInt32> image_widths;
        std::vector<kvs::UInt32> image_heights;

        for( const auto& rel_path : texture_files )
        {
            int w=0,h=0,n=0;
            stbi_uc* data = stbi_load( rel_path.string().c_str(), &w, &h, &n, 0 );
            if( !data )
            {
                auto full = ( base_dir / rel_path ).string();
                data = stbi_load( full.c_str(), &w, &h, &n, 0 );
            }
            if( !data )
            {
                std::cerr << "Failed to load texture: " << rel_path.string() << std::endl;
                continue;
            }

            size_t pix_cnt = static_cast<size_t>( w ) * h;
            kvs::ValueArray<kvs::UInt8> pixels( pix_cnt * 4 );
            if( n == 4 )
            {
                memcpy( pixels.data(), data, pix_cnt * 4 );
            }
            else if( n == 3 )
            {
                for( size_t i = 0; i < pix_cnt; ++i )
                {
                    pixels[i*4+0] = data[i*3+0];
                    pixels[i*4+1] = data[i*3+1];
                    pixels[i*4+2] = data[i*3+2];
                    pixels[i*4+3] = 255;
                }
            }
            stbi_image_free( data );

            color_arrays.push_back( pixels );
            image_widths.push_back( static_cast<kvs::UInt32>( w ) );
            image_heights.push_back( static_cast<kvs::UInt32>( h ) );
        }

        m_color_arrays = kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>( color_arrays );
        m_image_widths = kvs::ValueArray<kvs::UInt32>( image_widths );
        m_image_heights = kvs::ValueArray<kvs::UInt32>( image_heights );

        setSuccess( true );
        return true;
    }

    bool write( const std::string& filename )
    {
        setSuccess( false );
        return false;
    }
};
}

#endif // KVS__FBX_H_INCLUDE
