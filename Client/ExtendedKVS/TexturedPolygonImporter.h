#pragma once
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
    TexturedPolygonImporter( const std::string& filename );
    TexturedPolygonImporter( const kvs::FileFormatBase* file_format );
    virtual ~TexturedPolygonImporter();

    SuperClass* exec( const kvs::FileFormatBase* file_format );

private:
#ifdef ASSIMP
    void import( const kvs::FBX* fbx );
    void import( const kvs::ThreeDS* threeDS );
#endif

    void set_min_max_coord();
};
}
