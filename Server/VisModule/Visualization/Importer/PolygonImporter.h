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
#ifndef VIS_MODULE__POLYGON_IMPORTER_H_INCLUDE
#define VIS_MODULE__POLYGON_IMPORTER_H_INCLUDE

#include <vismodule/ImporterBase>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/PolygonObject>
#include <vismodule/KVSMLObjectPolygon>
#include <vismodule/Stl>
#include <vismodule/Ply>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Polygon importer class.
 */
/*==========================================================================*/
class PolygonImporter
    : public vismodule::ImporterBase
    , public vismodule::PolygonObject
{
    // Class name.
    visModuleClassName( vismodule::PolygonImporter );

    // Module information.
    visModuleCategory( Importer );
    visModuleBaseClass( vismodule::ImporterBase );
    visModuleSuperClass( vismodule::PolygonObject );

public:

    PolygonImporter( void );

    PolygonImporter( const std::string& filename );

    PolygonImporter( const vismodule::FileFormatBase& file_format );

    virtual ~PolygonImporter( void );

public:

    SuperClass* exec( const vismodule::FileFormatBase& file_format );

private:

    void import( const vismodule::KVSMLObjectPolygon* kvsml );

    void import( const vismodule::Stl* stl );

    void import( const vismodule::Ply* ply );

    void set_min_max_coord( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__POLYGON_IMPORTER_H_INCLUDE
