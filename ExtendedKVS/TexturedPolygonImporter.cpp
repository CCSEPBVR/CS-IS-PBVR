#include "TexturedPolygonImporter.h"


#include "FBX.h"

#ifdef CGFORMATEXT4KVS_SUPPORT_ASSIMP
#include "ThreeDS.h"
#endif // CGFORMATEXT4KVS_SUPPORT_ASSIMP

namespace kvs{
namespace jaea{

TexturedPolygonImporter::SuperClass* TexturedPolygonImporter::exec(const kvs::FileFormatBase* file_format){
    if(!file_format){
        BaseClass::setSuccess(false);
        kvsMessageError("Input file format is NULL.");
        return NULL;
    }

#ifdef CGFORMATEXT4KVS_SUPPORT_ASSIMP
    if(const kvs::jaea::ThreeDS* texturedPolygon = dynamic_cast<const kvs::jaea::ThreeDS*>(file_format)){
        this->import3DS(texturedPolygon);
    }else
#endif // CGFORMATEXT4KVS_SUPPORT_ASSIMP

        if (const kvs::jaea::FBX* texturedPolygon = dynamic_cast<const kvs::jaea::FBX*>(file_format)){
            this->importFBX(texturedPolygon);
        }
        else
        {
            BaseClass::setSuccess(false);
            kvsMessageError("Input file format is not supported.");
            return NULL;
        }

    return this;
}

#ifdef CGFORMATEXT4KVS_SUPPORT_ASSIMP
void TexturedPolygonImporter::import3DS(const kvs::jaea::ThreeDS* threeDS){
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

    //const kvs::ValueArray<kvs::Texture2D>& textures = threeDS->textures();
    //for(kvs::UInt32 id=0; id<textures.size(); id++){
    //    this->addTexture2D(id, textures[id]);
    //}
    //const kvs::ValueArray<kvs::ColorImage>& images = threeDS->images();
    //for(kvs::UInt32 id=0; id<images.size(); id++){
    //    this->addImage(id, images[id]);
    //}
    const kvs::ValueArray<kvs::ValueArray<kvs::UInt8>>& color_arrays = threeDS->colorArrays();
    const kvs::ValueArray<kvs::UInt32>& image_widths = threeDS->imageWidths();
    const kvs::ValueArray<kvs::UInt32>& image_heights = threeDS->imageHeights();
    for(kvs::UInt32 id=0; id<color_arrays.size(); id++){
        this->addColorArray(id, color_arrays[id], image_widths[id], image_heights[id]);
    }
}
#endif // CGFORMATEXT4KVS_SUPPORT_ASSIMP

void TexturedPolygonImporter::importFBX(const kvs::jaea::FBX* fbx){
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

} // jaea
} // kvs
