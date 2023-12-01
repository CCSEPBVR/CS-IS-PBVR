/****************************************************************************/
/**
 *  @file ExtractVertices.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ExtractVertices.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__EXTRACT_VERTICES_H_INCLUDE
#define KVS__EXTRACT_VERTICES_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/Module>

#include "MapperBase.h"
#include <kvs/PointObject>


namespace kvs
{

class VolumeObjectBase;

/*==========================================================================*/
/**
 *  ExtractVertices class.
 */
/*==========================================================================*/
class ExtractVertices
    : public kvs::MapperBase
    , public kvs::PointObject
{
    // Class name.
    kvsClassName( kvs::ExtractVertices );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::MapperBase );
    kvsModuleSuperClass( kvs::PointObject );

public:

    ExtractVertices( void );

    ExtractVertices(
        const kvs::VolumeObjectBase* volume );

    ExtractVertices(
        const kvs::VolumeObjectBase* volume,
        const kvs::TransferFunction& transfer_function );

    virtual ~ExtractVertices( void );

public:

//    kvs::ObjectBase* exec( const kvs::ObjectBase* object );
    SuperClass* exec( const kvs::ObjectBase* object );

private:

    void mapping( const kvs::VolumeObjectBase* volume );

//    void pre_process( void );

    void calculate_coords( void );

    void calculate_uniform_coords( void );

    void calculate_rectiliner_coords( void );

    template <typename T>
    void calculate_colors( void );
};

}

#endif // KVS__EXTRACT_VERTICES_H_INCLUDE
