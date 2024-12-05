/****************************************************************************/
/**
 *  @file PointImporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PointImporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__POINT_IMPORTER_H_INCLUDE
#define VIS_MODULE__POINT_IMPORTER_H_INCLUDE

#include <vismodule/ImporterBase>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/PointObject>
#include <vismodule/KVSMLObjectPoint>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Point object importer class.
 */
/*==========================================================================*/
class PointImporter
    : public vismodule::ImporterBase
    , public vismodule::PointObject
{
    // Class name.
    visModuleClassName( vismodule::PointImporter );

    // Module information.
    visModuleCategory( Importer );
    visModuleBaseClass( vismodule::ImporterBase );
    visModuleSuperClass( vismodule::PointObject );

public:

    PointImporter( void );

    PointImporter( const std::string& filename );

    PointImporter( const vismodule::FileFormatBase& file_format );

    virtual ~PointImporter( void );

public:

    SuperClass* exec( const vismodule::FileFormatBase& file_format );

private:

    void import( const vismodule::KVSMLObjectPoint* kvsml );

    void set_min_max_coord( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__POINT_IMPORTER_H_INCLUDE
