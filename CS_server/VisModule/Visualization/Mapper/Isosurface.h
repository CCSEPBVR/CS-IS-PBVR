/****************************************************************************/
/**
 *  @file Isosurface.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Isosurface.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__ISOSURFACE_H_INCLUDE
#define VIS_MODULE__ISOSURFACE_H_INCLUDE

#include <vismodule/PolygonObject>
#include <vismodule/VolumeObjectBase>
#include <vismodule/MapperBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Isosurface class.
 */
/*==========================================================================*/
class Isosurface : public vismodule::MapperBase, public vismodule::PolygonObject
{
    // Class name.
    visModuleClassName( vismodule::Isosurface );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PolygonObject );

private:

    double m_isolevel;    ///< isosurface level
    bool   m_duplication; ///< duplication flag

public:

    Isosurface( void );

    Isosurface(
        const vismodule::VolumeObjectBase& volume,
        const double                 isolevel,
        const SuperClass::NormalType normal_type = SuperClass::PolygonNormal );

    Isosurface(
        const vismodule::VolumeObjectBase& volume,
        const double                 isolevel,
        const SuperClass::NormalType normal_type,
        const bool                   duplication,
        const vismodule::TransferFunction& transfer_function );

    virtual ~Isosurface( void );

public:

    void setIsolevel( const double isolevel );

public:

    SuperClass* exec( const vismodule::ObjectBase& object );

private:

    void mapping( const vismodule::VolumeObjectBase& volume );
};

} // end of namespace vismodule

#endif // VIS_MODULE__ISOSURFACE_H_INCLUDE
