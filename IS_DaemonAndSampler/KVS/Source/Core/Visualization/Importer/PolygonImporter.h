/****************************************************************************/
/**
 *  @file PolygonImporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonImporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__POLYGON_IMPORTER_H_INCLUDE
#define KVS__POLYGON_IMPORTER_H_INCLUDE

#include "ImporterBase.h"
#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/PolygonObject>
#include <kvs/KVSMLObjectPolygon>
#include <kvs/Stl>
#include <kvs/Ply>


namespace kvs
{

/*==========================================================================*/
/**
 *  Polygon importer class.
 */
/*==========================================================================*/
class PolygonImporter
    : public kvs::ImporterBase
    , public kvs::PolygonObject
{
    // Class name.
    kvsClassName( kvs::PolygonImporter );

    // Module information.
    kvsModuleCategory( Importer );
    kvsModuleBaseClass( kvs::ImporterBase );
    kvsModuleSuperClass( kvs::PolygonObject );

public:

    PolygonImporter( void );

    PolygonImporter( const std::string& filename );

    PolygonImporter( const kvs::FileFormatBase* file_format );

    virtual ~PolygonImporter( void );

public:

    SuperClass* exec( const kvs::FileFormatBase* file_format );

private:

    void import( const kvs::KVSMLObjectPolygon* kvsml );

    void import( const kvs::Stl* stl );

    void import( const kvs::Ply* ply );

    void set_min_max_coord( void );
};

} // end of namespace kvs

#endif // KVS__POLYGON_IMPORTER_H_INCLUDE
