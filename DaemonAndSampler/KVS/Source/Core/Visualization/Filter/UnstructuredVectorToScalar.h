/*****************************************************************************/
/**
 *  @file   UnstructuredVectorToScalar.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: UnstructuredVectorToScalar.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__UNSTRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE
#define KVS__UNSTRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE

#include <kvs/VolumeObjectBase>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/FilterBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Conversion class from vector to scalar value for unstructured volume.
 */
/*===========================================================================*/
class UnstructuredVectorToScalar : public kvs::FilterBase, public kvs::UnstructuredVolumeObject
{
    // Class name.
    kvsClassName( kvs::UnstructuredVectorToScalar );

    // Module information.
    kvsModuleCategory( Filter );
    kvsModuleBaseClass( kvs::FilterBase );
    kvsModuleSuperClass( kvs::UnstructuredVolumeObject );

public:

    UnstructuredVectorToScalar( void );

    UnstructuredVectorToScalar( const kvs::UnstructuredVolumeObject* volume );

    virtual ~UnstructuredVectorToScalar( void );

public:

    SuperClass* exec( const kvs::ObjectBase* object );

protected:

    template <typename T>
    void calculate_magnitude( const kvs::UnstructuredVolumeObject* volume );
};

} // end of namespace kvs

#endif // KVS__UNSTRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE
