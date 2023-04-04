/****************************************************************************/
/**
 *  @file CellByCellUniformSampling.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellByCellUniformSampling.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__CELL_BY_CELL_UNIFORM_SAMPLING_H_INCLUDE
#define KVS__CELL_BY_CELL_UNIFORM_SAMPLING_H_INCLUDE

#include <kvs/MapperBase>
#include <kvs/Camera>
#include <kvs/PointObject>
#include <kvs/VolumeObjectBase>
#include <kvs/StructuredVolumeObject>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/ClassName>
#include <kvs/Module>
#include "CellByCellParticleGenerator.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Cell-by-cell particle generation class.
 */
/*===========================================================================*/
class CellByCellUniformSampling : public kvs::MapperBase, public kvs::PointObject
{
    // Class name.
    kvsClassName( kvs::CellByCellUniformSampling );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::MapperBase );
    kvsModuleSuperClass( kvs::PointObject );

private:

    const kvs::Camera*     m_camera;         ///< camera (reference)
    size_t                 m_subpixel_level; ///< subpixel level
    float                  m_sampling_step;  ///< sampling step in the object coordinate
    float                  m_object_depth;   ///< object depth
    kvs::ValueArray<float> m_density_map;    ///< density map

public:

    CellByCellUniformSampling( void );

    CellByCellUniformSampling(
        const kvs::VolumeObjectBase* volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const kvs::TransferFunction& transfer_function,
        const float                  object_depth = 0.0f );

    CellByCellUniformSampling(
        const kvs::Camera*           camera,
        const kvs::VolumeObjectBase* volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const kvs::TransferFunction& transfer_function,
        const float                  object_depth = 0.0f );

    virtual ~CellByCellUniformSampling( void );

public:

//    kvs::ObjectBase* exec( const kvs::ObjectBase* object );
    SuperClass* exec( const kvs::ObjectBase* object );

public:

    const size_t subpixelLevel( void ) const;

    const float samplingStep( void ) const;

    const float objectDepth( void ) const;

    void attachCamera( const kvs::Camera* camera );

    void setSubpixelLevel( const size_t subpixel_level );

    void setSamplingStep( const float sampling_step );

    void setObjectDepth( const float object_depth );

private:

    void mapping( const kvs::Camera* camera, const kvs::StructuredVolumeObject* volume );

    void mapping( const kvs::Camera* camera, const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void generate_particles( const kvs::StructuredVolumeObject* volume );

    template <typename T>
    void generate_particles( const kvs::UnstructuredVolumeObject* volume );
};

} // end of namespace kvs

#endif // KVS__CELL_BY_CELL_UNIFORM_SAMPLING_H_INCLUDE
