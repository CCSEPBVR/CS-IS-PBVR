#include "TexturedPolygonImporter.h"
#include <kvs/DebugNew>
#include <kvs/KVSMLPolygonObject>
#include <kvs/Math>
#include <kvs/Vector3>
namespace kvs
{

TexturedPolygonImporter::TexturedPolygonImporter()
{
}

TexturedPolygonImporter::TexturedPolygonImporter( const std::string& filename )
{
    if( kvs::FBX::CheckExtension( filename ) )
    {
        kvs::FBX* file_format = new kvs::FBX( filename );
        if ( !file_format )
        {
            BaseClass::setSuccess( false );
            kvsMessageError("Cannot read '%s'.",filename.c_str());
            return;
        }

        if ( file_format->isFailure() )
        {
            BaseClass::setSuccess( false );
            kvsMessageError("Cannot read '%s'.",filename.c_str());
            delete file_format;
            return;
        }

        this->import( file_format );
        delete file_format;
    }
    else
    {
        BaseClass::setSuccess( false );
        kvsMessageError("Cannot import '%s'.",filename.c_str());
        return;
    }
}

TexturedPolygonImporter::TexturedPolygonImporter( const kvs::FileFormatBase* file_format )
{
    this->exec( file_format );
}

TexturedPolygonImporter::~TexturedPolygonImporter()
{
}

TexturedPolygonImporter::SuperClass* TexturedPolygonImporter::exec( const kvs::FileFormatBase* file_format )
{
    if( !file_format )
    {
        BaseClass::setSuccess( false );
        kvsMessageError("Input file format is NULL.");
        return NULL;
    }

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

    return this;
}

void TexturedPolygonImporter::import( const kvs::FBX* fbx )
{
    this->setPolygonTypeToTriangle();
    this->setNormalTypeToVertex();

    const kvs::ValueArray<kvs::Real32>& coords = fbx->coords();
    this->setCoords(coords);

    const kvs::ValueArray<kvs::Real32>& normals = fbx->normals();
    this->setNormals(normals);

    const kvs::ValueArray<kvs::UInt32>& connections = fbx->connections();
    this->setConnections(connections);

    const kvs::ValueArray<kvs::Real32>& texture2DCoords = fbx->texture2DCoords();
    this->setTexture2DCoords(texture2DCoords);

    const kvs::ValueArray<kvs::UInt32>& textureIds = fbx->textureIds();
    this->setTextureIds(textureIds);

    //const kvs::ValueArray<kvs::Texture2D>& textures = fbx->textures();
    //for(kvs::UInt32 id=0; id<textures.size(); id++){
    //    this->addTexture2D(id, textures[id]);
    //}
    //const kvs::ValueArray<kvs::ColorImage>& images = fbx->images();
    //for(kvs::UInt32 id=0; id<images.size(); id++){
    //    this->addImage(id, images[id]);
    //}
    const kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>& color_arrays = fbx->colorArrays();
    const kvs::ValueArray<kvs::UInt32>& image_widths = fbx->imageWidths();
    const kvs::ValueArray<kvs::UInt32>& image_heights = fbx->imageHeights();
    for(kvs::UInt32 id=0; id<color_arrays.size(); id++){
        this->addColorArray(id, color_arrays[id], image_widths[id], image_heights[id]);
    }
}

}
