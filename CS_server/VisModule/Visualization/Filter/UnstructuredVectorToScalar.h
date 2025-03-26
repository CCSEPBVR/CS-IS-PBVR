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
#ifndef VIS_MODULE__UNSTRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE
#define VIS_MODULE__UNSTRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE

#include <vismodule/VolumeObjectBase>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/FilterBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Conversion class from vector to scalar value for unstructured volume.
 */
/*===========================================================================*/
class UnstructuredVectorToScalar : public vismodule::FilterBase, public vismodule::UnstructuredVolumeObject
{
    // Class name.
    visModuleClassName( vismodule::UnstructuredVectorToScalar );

    // Module information.
    visModuleCategory( Filter );
    visModuleBaseClass( vismodule::FilterBase );
    visModuleSuperClass( vismodule::UnstructuredVolumeObject );

public:

    UnstructuredVectorToScalar( void );

    UnstructuredVectorToScalar( const vismodule::UnstructuredVolumeObject& volume );

    virtual ~UnstructuredVectorToScalar( void );

public:

    SuperClass* exec( const vismodule::ObjectBase& object );

protected:

    template <typename T>
    void calculate_magnitude( const vismodule::UnstructuredVolumeObject& volume );
};

} // end of namespace vismodule

#endif // VIS_MODULE__UNSTRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE
