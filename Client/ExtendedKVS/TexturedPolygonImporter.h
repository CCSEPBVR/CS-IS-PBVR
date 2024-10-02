#pragma once
#include <kvs/ImporterBase>
#include <kvs/Module>

//#include <kvs/TexturedPolygonObject>
#include <kvs/TexturedPolygonObject>

//#include <kvs/KVSMLPolygonObject>

//#include <kvs/FBX>
#ifdef PBVR_SUPPORT_FBX
#include "FBX.h"
#endif
#ifdef PBVR_SUPPORT_3DS
#include "ThreeDS.h"
#endif

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
#ifdef PBVR_SUPPORT_FBX
    void import( const kvs::FBX* fbx );
#endif
#ifdef PBVR_SUPPORT_3DS
    void import( const kvs::ThreeDS* threeDS );
#endif
    void set_min_max_coord();
};
}
