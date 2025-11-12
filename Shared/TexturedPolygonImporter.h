#pragma once
#include <filesystem>
#include <kvs/DebugNew>
#include <kvs/KVSMLPolygonObject>
#include <kvs/Math>
#include <kvs/Vector3>
#include <kvs/ImporterBase>
#include <kvs/Module>
#include <kvs/TexturedPolygonObject>

#ifdef ASSIMP
#include "FBX.h"
#include "ThreeDS.h"
#endif

namespace kvs
{
class TexturedPolygonImporter : public kvs::ImporterBase, public kvs::TexturedPolygonObject
{
    kvsModule( kvs::TexturedPolygonImporter, Importer );
    kvsModuleBaseClass( kvs::ImporterBase );
    kvsModuleSuperClass( kvs::TexturedPolygonObject );

public:
    TexturedPolygonImporter();
    TexturedPolygonImporter( const std::string& filename )
    {
        std::filesystem::path filepath( filename );
        std::string ext = filepath.extension().string();
#ifdef ASSIMP
        if( ext == ".fbx" )
        {
            if( kvs::FBX::CheckExtension( filename ) )
            {
                kvs::FBX* file_format = new kvs::FBX( filename );
                if( !file_format )
                {
                    BaseClass::setSuccess( false );
                    kvsMessageError( "Cannot read '%s'.",filename.c_str() );
                    return;
                }

                if( file_format->isFailure() )
                {
                    BaseClass::setSuccess( false );
                    kvsMessageError( "Cannot read '%s'.",filename.c_str() );
                    delete file_format;
                    return;
                }

                this->import( file_format );
                delete file_format;
            }
            else
            {
                BaseClass::setSuccess( false );
                kvsMessageError( "Cannot import '%s'.",filename.c_str() );
                return;
            }
        }

        if( ext == ".3ds" )
        {
            if( kvs::ThreeDS::CheckExtension( filename ) )
            {
                kvs::ThreeDS* file_format = new kvs::ThreeDS( filename );
                if ( !file_format )
                {
                    BaseClass::setSuccess( false );
                    kvsMessageError( "Cannot read '%s'.",filename.c_str() );
                    return;
                }

                if ( file_format->isFailure() )
                {
                    BaseClass::setSuccess( false );
                    kvsMessageError( "Cannot read '%s'.",filename.c_str() );
                    delete file_format;
                    return;
                }

                this->import( file_format );
                delete file_format;
            }
            else
            {
                BaseClass::setSuccess( false );
                kvsMessageError( "Cannot import '%s'.",filename.c_str() );
                return;
            }
        }
#endif
    }

    TexturedPolygonImporter( const kvs::FileFormatBase* file_format )
    {
        this->exec( file_format );
    }

    virtual ~TexturedPolygonImporter()
    {
    }

    SuperClass* exec( const kvs::FileFormatBase* file_format )
    {
        if( !file_format )
        {
            BaseClass::setSuccess( false );
            kvsMessageError("Input file format is NULL.");
            return NULL;
        }

#ifdef ASSIMP
        if ( const kvs::FBX* texturedPolygon = dynamic_cast<const kvs::FBX*>(file_format) )
        {
            this->import( texturedPolygon );
        }
        else
        {
            BaseClass::setSuccess( false );
            kvsMessageError("Input file format is not supported.");
            return NULL;
        }

        if ( const kvs::ThreeDS* texturedPolygon = dynamic_cast<const kvs::ThreeDS*>(file_format) )
        {
            this->import( texturedPolygon );
        }
        else
        {
            BaseClass::setSuccess( false );
            kvsMessageError("Input file format is not supported.");
            return NULL;
        }
#endif

        return this;
    }

private:
#ifdef ASSIMP
    void import( const kvs::FBX* fbx )
    {
        this->setPolygonTypeToTriangle();
        this->setNormalTypeToVertex();

        const kvs::ValueArray<kvs::Real32>& coords = fbx->coords();
        this->setCoords( coords );

        const kvs::ValueArray<kvs::Real32>& normals = fbx->normals();
        this->setNormals( normals );

        const kvs::ValueArray<kvs::UInt32>& connections = fbx->connections();
        this->setConnections( connections );

        const kvs::ValueArray<kvs::Real32>& texture2DCoords = fbx->texture2DCoords();
        this->setTexture2DCoords( texture2DCoords );

        const kvs::ValueArray<kvs::UInt32>& textureIds = fbx->textureIds();
        this->setTextureIds( textureIds );

        this->updateMinMaxCoords();

        const kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>& color_arrays = fbx->colorArrays();
        const kvs::ValueArray<kvs::UInt32>& image_widths = fbx->imageWidths();
        const kvs::ValueArray<kvs::UInt32>& image_heights = fbx->imageHeights();
        for( kvs::UInt32 id=0; id<color_arrays.size(); id++ )
        {
            this->addColorArray( id, color_arrays[id], image_widths[id], image_heights[id] );
        }
    }

    void import( const kvs::ThreeDS* threeDS )
    {
        this->setPolygonTypeToTriangle();
        this->setNormalTypeToVertex();

        const kvs::ValueArray<kvs::Real32>& coords = threeDS->coords();
        this->setCoords(coords);

        const kvs::ValueArray<kvs::Real32>& normals = threeDS->normals();
        this->setNormals(normals);

        const kvs::ValueArray<kvs::UInt32>& connections = threeDS->connections();
        this->setConnections(connections);

        const kvs::ValueArray<kvs::Real32>& texture2DCoords = threeDS->texture2DCoords();
        this->setTexture2DCoords(texture2DCoords);

        const kvs::ValueArray<kvs::UInt32>& textureIds = threeDS->textureIds();
        this->setTextureIds(textureIds);

        this->updateMinMaxCoords();

        const kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>& color_arrays = threeDS->colorArrays();
        const kvs::ValueArray<kvs::UInt32>& image_widths = threeDS->imageWidths();
        const kvs::ValueArray<kvs::UInt32>& image_heights = threeDS->imageHeights();
        for( kvs::UInt32 id=0; id<color_arrays.size(); id++ )
        {
            this->addColorArray(id, color_arrays[id], image_widths[id], image_heights[id]);
        }
    }
#endif
};
}
