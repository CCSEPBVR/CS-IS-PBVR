//#ifndef _TEXTURED_POLYGON_IMPORTER_H_
//#define _TEXTURED_POLYGON_IMPORTER_H_

//#include <Core/Visualization/Importer/ImporterBase.h>
//#include "TexturedPolygonObject.h"

//namespace kvs{
//namespace jaea{

//class FBX;
//#ifdef CGFORMATEXT4KVS_SUPPORT_ASSIMP
//class ThreeDS;
//#endif // CGFORMATEXT4KVS_SUPPORT_ASSIMP

//class TexturedPolygonImporter : public kvs::ImporterBase, public TexturedPolygonObject {
//    kvsModule( kvs::jaea::TexturedPolygonImporter, Importer );
//    kvsModuleBaseClass( kvs::ImporterBase );
//    kvsModuleSuperClass( kvs::jaea::TexturedPolygonImporter );

//public:
//    TexturedPolygonImporter(){};
//    virtual ~TexturedPolygonImporter(){};

//    SuperClass* exec( const kvs::FileFormatBase* file_format ) override;

//private:
//#ifdef CGFORMATEXT4KVS_SUPPORT_ASSIMP
//    void import3DS(const kvs::jaea::ThreeDS* threeDS);
//#endif // CGFORMATEXT4KVS_SUPPORT_ASSIMP

//    void importFBX( const kvs::jaea::FBX* fbx);
//};

//} // namespace jaea
//} // kvs

//#endif // _THREE_DS_H_

#pragma once
#include <kvs/ImporterBase>
#include <kvs/Module>

//#include <kvs/TexturedPolygonObject>
#include "TexturedPolygonObject.h"

//#include <kvs/KVSMLPolygonObject>

//#include <kvs/FBX>
#include "FBX.h"

//#include <kvs/Stl>
//#include <kvs/Ply>

namespace kvs
{
//class TexturedPolygonImporter : public kvs::ImporterBase, public kvs::TexturedPolygonObject
class TexturedPolygonImporter : public kvs::ImporterBase, public kvs::TexturedPolygonObject
{
    kvsModule( kvs::TexturedPolygonImporter, Importer );
    kvsModuleBaseClass( kvs::ImporterBase );
    kvsModuleSuperClass( kvs::TexturedPolygonObject );

public:
    TexturedPolygonImporter();
    TexturedPolygonImporter( const std::string& filename );
    TexturedPolygonImporter( const kvs::FileFormatBase* file_format );
    virtual ~TexturedPolygonImporter();

    SuperClass* exec( const kvs::FileFormatBase* file_format );

private:
    void import( const kvs::FBX* fbx );
//    void import( const kvs::3DS* 3ds );
    void set_min_max_coord();
};
}
