/****************************************************************************/
/**
 *  @file CellByCellMetropolisSampling.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellByCellMetropolisSampling.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__CELL_BY_CELL_METROPOLIS_SAMPLING_H_INCLUDE
#define VIS_MODULE__CELL_BY_CELL_METROPOLIS_SAMPLING_H_INCLUDE

#include <vismodule/MapperBase>
#include <vismodule/Camera>
#include <vismodule/PointObject>
#include <vismodule/VolumeObjectBase>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include "CellByCellParticleGenerator.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Cell-by-cell particle generation class.
 */
/*===========================================================================*/
class CellByCellMetropolisSampling : public vismodule::MapperBase, public vismodule::PointObject
{
    // Class name.
    visModuleClassName( vismodule::CellByCellMetropolisSampling );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PointObject );

private:

    const vismodule::Camera*     m_camera;         ///< camera (reference)
    size_t                 m_subpixel_level; ///< subpixel level
    float                  m_sampling_step;  ///< sampling step in the object coordinate
    float                  m_object_depth;   ///< object depth
    vismodule::ValueArray<float> m_density_map;    ///< density map

public:

    CellByCellMetropolisSampling( void );

    CellByCellMetropolisSampling(
        const vismodule::VolumeObjectBase* volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        const float                  object_depth = 0.0f );

    CellByCellMetropolisSampling(
        const vismodule::Camera*           camera,
        const vismodule::VolumeObjectBase* volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        const float                  object_depth = 0.0f );

    virtual ~CellByCellMetropolisSampling( void );

public:

    SuperClass* exec( const vismodule::ObjectBase* object );

public:

    const size_t subpixelLevel( void ) const;

    const float samplingStep( void ) const;

    const float objectDepth( void ) const;

    void attachCamera( const vismodule::Camera* camera );

    void setSubpixelLevel( const size_t subpixel_level );

    void setSamplingStep( const float sampling_step );

    void setObjectDepth( const float object_depth );

private:

    void mapping( const vismodule::Camera* camera, const vismodule::StructuredVolumeObject* volume );

    void mapping( const vismodule::Camera* camera, const vismodule::UnstructuredVolumeObject* volume );

    template <typename T>
    void generate_particles( const vismodule::StructuredVolumeObject* volume );

    template <typename T>
    void generate_particles( const vismodule::UnstructuredVolumeObject* volume );
};

} // end of namespace vismodule

#endif // VIS_MODULE__CELL_BY_CELL_METROPOLIS_SAMPLING_H_INCLUDE
