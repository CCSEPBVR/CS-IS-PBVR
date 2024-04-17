#pragma once
#include <kvs/ImporterBase>
#include <kvs/Module>

//#include <kvs/TexturedPolygonObject>
#include "TexturedPolygonObject.h"

//#include <kvs/KVSMLPolygonObject>

//#include <kvs/FBX>
#include "FBX.h"
#include "ThreeDS.h"

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
    void import( const kvs::ThreeDS* threeDS );
    void set_min_max_coord();
};
}
