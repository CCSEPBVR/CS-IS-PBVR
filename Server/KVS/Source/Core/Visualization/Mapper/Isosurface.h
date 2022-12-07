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
#ifndef KVS__ISOSURFACE_H_INCLUDE
#define KVS__ISOSURFACE_H_INCLUDE

#include <kvs/PolygonObject>
#include <kvs/VolumeObjectBase>
#include <kvs/MapperBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*==========================================================================*/
/**
 *  Isosurface class.
 */
/*==========================================================================*/
class Isosurface : public kvs::MapperBase, public kvs::PolygonObject
{
    // Class name.
    kvsClassName( kvs::Isosurface );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::MapperBase );
    kvsModuleSuperClass( kvs::PolygonObject );

private:

    double m_isolevel;    ///< isosurface level
    bool   m_duplication; ///< duplication flag

public:

    Isosurface( void );

    Isosurface(
        const kvs::VolumeObjectBase* volume,
        const double                 isolevel,
        const SuperClass::NormalType normal_type = SuperClass::PolygonNormal );

    Isosurface(
        const kvs::VolumeObjectBase* volume,
        const double                 isolevel,
        const SuperClass::NormalType normal_type,
        const bool                   duplication,
        const kvs::TransferFunction& transfer_function );

    virtual ~Isosurface( void );

public:

    void setIsolevel( const double isolevel );

public:

    SuperClass* exec( const kvs::ObjectBase* object );

private:

    void mapping( const kvs::VolumeObjectBase* volume );
};

} // end of namespace kvs

#endif // KVS__ISOSURFACE_H_INCLUDE
