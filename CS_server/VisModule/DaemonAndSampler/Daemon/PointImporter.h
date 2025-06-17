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
#ifndef PBVR__POINT_IMPORTER_H_INCLUDE
#define PBVR__POINT_IMPORTER_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/KVSMLObjectPoint>
#include "ImporterBase.h"
#include "PointObject.h"


namespace pbvr
{

/*==========================================================================*/
/**
 *  Point object importer class.
 */
/*==========================================================================*/
class PointImporter
    : public vismodule::ImporterBase
    , public pbvr::PointObject
{
    // Class name.
    vismoduleClassName( vismodule::PointImporter );

    // Module information.
    vismoduleModuleCategory( Importer );
    vismoduleModuleBaseClass( vismodule::ImporterBase );
    vismoduleModuleSuperClass( pbvr::PointObject );

public:

    PointImporter( void );

    PointImporter( const std::string& filename );

    PointImporter( const vismodule::FileFormatBase* file_format );

    virtual ~PointImporter( void );

public:

    SuperClass* exec( const vismodule::FileFormatBase* file_format );

private:

    void import( const vismodule::KVSMLObjectPoint* kvsml );

    void set_min_max_coord( void );
};

} // end of namespace vismodule

#endif // KVS__POINT_IMPORTER_H_INCLUDE
