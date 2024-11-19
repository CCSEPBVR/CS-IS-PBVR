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
#ifndef KVS__POINT_IMPORTER_H_INCLUDE
#define KVS__POINT_IMPORTER_H_INCLUDE

#include "ImporterBase.h"
#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/PointObject>
#include <kvs/KVSMLObjectPoint>


namespace kvs
{

/*==========================================================================*/
/**
 *  Point object importer class.
 */
/*==========================================================================*/
class PointImporter
    : public kvs::ImporterBase
    , public kvs::PointObject
{
    // Class name.
    kvsClassName( kvs::PointImporter );

    // Module information.
    kvsModuleCategory( Importer );
    kvsModuleBaseClass( kvs::ImporterBase );
    kvsModuleSuperClass( kvs::PointObject );

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
