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

#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/KVSMLObjectPoint>
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
    : public kvs::ImporterBase
    , public pbvr::PointObject
{
    // Class name.
    kvsClassName( kvs::PointImporter );

    // Module information.
    kvsModuleCategory( Importer );
    kvsModuleBaseClass( kvs::ImporterBase );
    kvsModuleSuperClass( pbvr::PointObject );

public:

    PointImporter( void );

    PointImporter( const std::string& filename );

    PointImporter( const kvs::FileFormatBase* file_format );

    virtual ~PointImporter( void );

public:

    SuperClass* exec( const kvs::FileFormatBase* file_format );

private:

    void import( const kvs::KVSMLObjectPoint* kvsml );

    void set_min_max_coord( void );
};

} // end of namespace kvs

#endif // KVS__POINT_IMPORTER_H_INCLUDE
