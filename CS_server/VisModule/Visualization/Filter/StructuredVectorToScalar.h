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
#ifndef VIS_MODULE__STRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE
#define VIS_MODULE__STRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE

#include <vismodule/VolumeObjectBase>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/FilterBase>
#include <vismodule/ClassName>
#include <vismodule/Module>

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Conversion class from vector to scalar value for structured volume.
 */
/*===========================================================================*/
class StructuredVectorToScalar : public vismodule::FilterBase, public vismodule::StructuredVolumeObject
{
    // Class name.
    visModuleClassName( vismodule::StructuredVectorToScalar );

    // Module information.
    visModuleCategory( Filter );
    visModuleBaseClass( vismodule::FilterBase );
    visModuleSuperClass( vismodule::StructuredVolumeObject );

public:

    StructuredVectorToScalar( void );

    StructuredVectorToScalar( const vismodule::StructuredVolumeObject* volume );

    virtual ~StructuredVectorToScalar( void );

public:

    SuperClass* exec( const vismodule::ObjectBase* object );

protected:

    template <typename T>
    void calculate_magnitude( const vismodule::StructuredVolumeObject* volume );
};

} // end of namespace vismodule

#endif // VIS_MODULE__STRUCTURED_VECTOR_TO_SCALAR_H_INCLUDE
