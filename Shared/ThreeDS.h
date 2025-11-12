#ifndef THREEDS_H
#define THREEDS_H

#include "stb_image.h"
#include <cstring>
#include <cassert>
#include <kvs/File>
#include <kvs/Assert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <fstream>
#include <string>
#include <kvs/ValueArray>
#include <kvs/FileFormatBase>
#include <kvs/Type>
#include <kvs/Indent>
#include <kvs/Deprecated>

namespace
{
const int MaxLineLength = 256;
const char* const Delimiter = " \t\n\r";
const std::string FileTypeToString[2] = { "ascii", "binary" };
}

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
    static bool CheckExtension( const std::string& filename )
    {
        const kvs::File file( filename );
        if( file.extension() == "3ds" )
        {
            return true;
        }

        return false;
    }

public:
    ThreeDS():
        m_file_type( ThreeDS::Binary )
    {
    }

    ThreeDS( const std::string& filename ):
        m_file_type( ThreeDS::Binary )
    {
        this->read( filename );
    }

    virtual ~ThreeDS()
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
    void setTexture2DCoords( const kvs::ValueArray<kvs::Real32>& texture2DCoords ) { m_texture2DCoords = texture2DCoords; }
    void setTextureIds( const kvs::ValueArray<kvs::UInt32>& textureIds ) { m_textureIds = textureIds; }
    void setColorArrays( const kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>& color_arrays ) { m_color_arrays = color_arrays; }
    void setImageWidths( const kvs::ValueArray<kvs::UInt32>& image_width ) { m_image_widths = image_width; }
    void setImageHeights( const kvs::ValueArray<kvs::UInt32>& image_height ) { m_image_heights = image_height; }

    void print( std::ostream& os, const kvs::Indent& indent = kvs::Indent(0) ) const
    {
        os << indent << "Filename : " << BaseClass::filename() << std::endl;
        os << indent << "File type : " << ::FileTypeToString[ m_file_type ] << std::endl;
        os << indent << "Number of triangles : " << m_normals.size() / 3;
    }

    bool read( const std::string& filename )
    {
        setFilename( filename );

#ifdef _WIN32
        std::string filenameUTF8 = MultibyteToUTF8( filename );
#else
        // Because allmost all Linux distribution may regards that file path is encoded in UTF-8...
        std::string filenameUTF8 = filename;
#endif

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            filenameUTF8,
            aiProcess_CalcTangentSpace |
                aiProcess_Triangulate | // for triangulation
                aiProcess_JoinIdenticalVertices |
                aiProcess_SortByPType);

        if( !scene )
        {
            setSuccess(false);
            return false;
        }

        unsigned int vertexTop = 0;
        std::vector<kvs::Real32> normals;
        std::vector<kvs::Real32> coords;
        std::vector<kvs::UInt32> connections;
        std::vector<kvs::Real32> texture2DCoords;
        std::vector<kvs::UInt32> textureIds;
        std::vector<aiString> texturePaths;

        aiTextureType textureType = aiTextureType::aiTextureType_DIFFUSE;
        unsigned int textureIndex = 0;
        for (unsigned int i=0; i<scene->mNumMeshes; i++ )
        {
            aiMesh* mesh = scene->mMeshes[i];
            if( mesh == NULL )
            {
                continue;
            }

            for( unsigned int j = 0; j < mesh->mNumVertices; j++ )
            {
                // read Normal
                aiVector3D normal = mesh->mNormals[j];
                normals.push_back( static_cast<kvs::Real32>( normal.x ) );
                normals.push_back( static_cast<kvs::Real32>( normal.y ) );
                normals.push_back( static_cast<kvs::Real32>( normal.z ) );

                // read Vertex
                aiVector3D vertex = mesh->mVertices[j];
                coords.push_back( static_cast<kvs::Real32>( vertex.x ) );
                coords.push_back( static_cast<kvs::Real32>( vertex.y ) );
                coords.push_back( static_cast<kvs::Real32>( vertex.z ) );
            }

            // read Polygon's vertex connection
            for( unsigned int j = 0; j < mesh->mNumFaces; j++ )
            {
                aiFace face = mesh->mFaces[j];
                assert( face.mNumIndices == 3 );
                for( unsigned int k = 0; k < face.mNumIndices; k++ )
                {
                    // global vertex index is "vertexTop + face.mIndices[k]"
                    connections.push_back( vertexTop + face.mIndices[k] );
                }
            }

            // read texture image file's file path
            aiMaterial* meshMaterial = scene->mMaterials[mesh->mMaterialIndex];
            if( meshMaterial != NULL )
            {
                unsigned int textureCount = meshMaterial->GetTextureCount( textureType );
                for( unsigned int j = 0; j < textureCount; j++ )
                {
                    aiString path;
                    meshMaterial->GetTexture(textureType, j, &path);

                    auto it = std::find( texturePaths.begin(), texturePaths.end(), path );
                    if( it == texturePaths.end() )
                    {
                        texturePaths.push_back( path );
                    }
                }

                // set texture ID to mesh vertex
                int uvIndex = 0;
                if( 0 < textureCount )
                {
                    // uses only first texture because TexturedPolygonObject not supprot multiple textures
                    aiString path;
                    meshMaterial->GetTexture( textureType, textureIndex, &path );

                    if( meshMaterial->Get(AI_MATKEY_UVWSRC(textureType, textureIndex), uvIndex) != aiReturn_SUCCESS )
                    {
                        // UV-channel for the texture isn't defined explicitly.
                        uvIndex = 0;
                    }

                    // find texture ID of path and set the id to vertex
                    auto it = std::find( texturePaths.begin(), texturePaths.end(), path );
                    if( it != texturePaths.end() )
                    {
                        kvs::UInt32 id = (kvs::UInt32) std::distance( texturePaths.begin(), it );
                        for( unsigned int j = 0; j < mesh->mNumVertices; j++ )
                        {
                            textureIds.push_back( id );
                        }
                    }
                }

                // read texture 2d coord (UV coord)
                if( 0 < mesh->GetNumUVChannels() && mesh->mTextureCoords[uvIndex] != NULL )
                {
                    // uses only first UV coords because TexturedPolygonObject not supprot multiple UV coords
                    aiVector3D* uvsets = mesh->mTextureCoords[uvIndex];
                    for( unsigned int j = 0; j < mesh->mNumVertices; j++ )
                    {
                        texture2DCoords.push_back( uvsets[j].x );
                        texture2DCoords.push_back( uvsets[j].y );
                    }
                }
            }

            vertexTop += mesh->mNumVertices;
        }
        m_normals = kvs::ValueArray<kvs::Real32>( normals );
        m_coords = kvs::ValueArray<kvs::Real32>( coords );
        m_connections = kvs::ValueArray<kvs::UInt32>( connections );
        m_texture2DCoords = kvs::ValueArray<kvs::Real32>( texture2DCoords );
        m_textureIds = kvs::ValueArray<kvs::UInt32>( textureIds );

        // read texture image files
        //std::vector<kvs::Texture2D> textures;
        //std::vector<kvs::ColorImage> images;
        std::vector<kvs::ValueArray<kvs::UInt8>> color_arrays;
        std::vector<kvs::UInt32> image_widths;
        std::vector<kvs::UInt32> image_heights;
        for( int i=0; i<texturePaths.size(); i++ )
        {
            int id = i;
            std::string parentPath = filename.substr( 0, filename.find_last_of("\\/") + 1 ); // +1 to include tail's "/"
#ifdef _WIN32
            std::string textureFilename = MultibyteToUTF8( parentPath + texturePaths[i].C_Str() );
#else \
            // Because allmost all Linux distribution may regards that file path is encoded in UTF-8...
            std::string textureFilename = parentPath + texturePaths[i].C_Str();
#endif \
            // read image file from filesystem and get width, height and data.
            const size_t bytesPerChannel = 1;
            int width = 0;
            int height = 0;
            int numChannels = 0; // It will be 4 for RGBA, 3 for RGB
            stbi_uc* data = stbi_load( textureFilename.c_str(), &width, &height, &numChannels, 0 );
            //std::cout << "numChannels= " << numChannels << ", file=" << textureFilename << std::endl;
            if( data == NULL )
            {
                std::cerr << "Failed to load " << textureFilename << std::endl;
            }
            else
            {

#ifdef _DEBUG
                std::cerr << "  texture[" << i << "] filename=" << textureFilename <<std::endl;
                std::cerr << "     width=" << width << ", height=" << height << std::endl;
#endif // _DEBUG

                if( numChannels == 4 )
                {
                    kvs::ValueArray<kvs::UInt8> pixels( data, width * height * numChannels );
                    //kvs::Texture2D texture;
                    //texture.setPixelFormat(numChannels, bytesPerChannel);
                    //texture.create(width, height, data);
                    //textures.push_back(texture);
                    //kvs::ColorImage image(width, height, pixels);
                    //images.push_back(image);
                    color_arrays.push_back( pixels );
                    image_widths.push_back( width );
                    image_heights.push_back( height );
                }
                else if( numChannels == 3 )
                {
                    kvs::ValueArray<kvs::UInt8> pixels( width * height * 4 );
                    for( size_t iData=0; iData < width * height; iData++)
                    {
                        stbi_uc r = data[iData*3 + 0];
                        stbi_uc g = data[iData*3 + 1];
                        stbi_uc b = data[iData*3 + 2];
                        stbi_uc a = 255;
                        pixels[iData*4 + 0] = r;
                        pixels[iData*4 + 1] = g;
                        pixels[iData*4 + 2] = b;
                        pixels[iData*4 + 3] = a;
                    }

                    //kvs::Texture2D texture;
                    //texture.setPixelFormat(numChannels, bytesPerChannel);
                    //texture.create(width, height, data);
                    //textures.push_back(texture);
                    //kvs::ColorImage image(width, height, pixels);
                    //images.push_back(image);
                    color_arrays.push_back( pixels );
                    image_widths.push_back( width );
                    image_heights.push_back( height );
                }
                else
                {
                    std::cerr << "Unsupported numChannels=" << numChannels << ", file=" << textureFilename << std::endl;
                }
            }
        }
        //m_textures = kvs::ValueArray<kvs::Texture2D>(textures);
        //m_images = kvs::ValueArray<kvs::ColorImage>(images);
        m_color_arrays = kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>( color_arrays );
        m_image_widths = kvs::ValueArray<kvs::UInt32>( image_widths );
        m_image_heights = kvs::ValueArray<kvs::UInt32>( image_heights );

        setSuccess( true );
        return true;
    }

    bool write( const std::string& filename )
    {
        setFilename( filename );
        setSuccess( false );
        return false;
    }

    std::string MultibyteToUTF8( const std::string& text )
    {
#ifdef _WIN32
        // Multibyte to Wide
        int wideLength = MultiByteToWideChar( CP_THREAD_ACP, 0, text.c_str(), -1, NULL, 0 );
        wchar_t* wideBuffer = new wchar_t[wideLength];
        if( 0 == MultiByteToWideChar( CP_THREAD_ACP, 0, text.c_str(), -1, wideBuffer, wideLength ) )
        {
            delete wideBuffer;
            return 0;
        }

        // Wide to UTF8
        int utf8Length = WideCharToMultiByte( CP_UTF8, 0, wideBuffer, -1, NULL, 0, NULL, NULL );
        char* utf8Buffer = new char[utf8Length];
        if( 0 == WideCharToMultiByte( CP_UTF8, 0, wideBuffer, -1, utf8Buffer, utf8Length, NULL, NULL ) )
        {
            delete wideBuffer;
            delete utf8Buffer;
            return 0;
        }

        std::string strUTF8( utf8Buffer );
        delete wideBuffer;
        delete utf8Buffer;
#else
        // get initial locale like "C"
        std::string prevLocale = std::setlocale( LC_ALL, NULL );

        // set default user-locale like "Japanese_Japan.932"
        setlocale( LC_ALL, "" );

        std::mbstate_t state = std::mbstate_t();

        // Multibyte to Wide
        const char* mbstr = text.c_str();
        std::size_t wideLength = std::mbsrtowcs( NULL, &mbstr, 0, &state ) + 1;
        std::vector<wchar_t> wideCharacters( wideLength );
        std::mbsrtowcs( &wideCharacters[0], &mbstr, wideCharacters.size(), &state );

        // Wide to UTF8
        std::setlocale( LC_ALL, ".utf8" );
        const wchar_t* wstr = &wideCharacters[0];
        std::size_t utf8Length = std::wcsrtombs( nullptr, &wstr, 0, &state ) + 1;
        std::vector<char> utf8bytes( utf8Length );
        std::wcsrtombs( &utf8bytes[0], &wstr, utf8bytes.size(), &state );

        std::string strUTF8( &utf8bytes[0] );

        // restore initial locale
        std::setlocale( LC_ALL, prevLocale.c_str() );
#endif
        return strUTF8;
    }
};

}
#endif
