/*****************************************************************************/
/**
 *  @file   StructuredVectorToScalar.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StructuredVectorToScalar.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__STRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE
#define KVS__STRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE

#include <kvs/VolumeObjectBase>
#include <kvs/StructuredVolumeObject>
#include <kvs/FilterBase>
#include <kvs/ClassName>
#include <kvs/Module>

namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Conversion class from vector to scalar value for structured volume.
 */
/*===========================================================================*/
class StructuredVectorToScalar : public kvs::FilterBase, public kvs::StructuredVolumeObject
{
    // Class name.
    kvsClassName( kvs::StructuredVectorToScalar );

    // Module information.
    kvsModuleCategory( Filter );
    kvsModuleBaseClass( kvs::FilterBase );
    kvsModuleSuperClass( kvs::StructuredVolumeObject );

public:

    StructuredVectorToScalar( void );

    StructuredVectorToScalar( const kvs::StructuredVolumeObject* volume );

    virtual ~StructuredVectorToScalar( void );

public:

    SuperClass* exec( const kvs::ObjectBase* object );

protected:

    template <typename T>
    void calculate_magnitude( const kvs::StructuredVolumeObject* volume );
};

} // end of namespace kvs

#endif // KVS__STRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE
