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
#ifndef PBVR__CELL_BY_CELL_UNIFORM_SAMPLING_H_INCLUDE
#define PBVR__CELL_BY_CELL_UNIFORM_SAMPLING_H_INCLUDE

#include "MapperBase.h"
#include <kvs/Camera>
#include "PointObject.h"
#include "VolumeObjectBase.h"
#include "StructuredVolumeObject.h"
#include "UnstructuredVolumeObject.h"
#include "ClassName.h"
#include <kvs/Module>
#include <kvs/MersenneTwister> 
#include "CellByCellParticleGenerator.h"
#include "CropRegion.h"
#include "TransferFunctionSynthesizer.h"
//#include "TransferFunctionSynthesizer_IS.h"
#include "TransferFunctionSynthesizerCreator.h"
#include "ExtendedTransferFunction.h"
//#include "SFMT/SFMT.h" 

#include "timer.h"

#define RANK 1

typedef unsigned char Byte;

namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  Cell-by-cell particle generation class.
 */
/*===========================================================================*/
class CellByCellUniformSampling : public pbvr::MapperBase, public pbvr::PointObject
{
    // Class name.
    kvsClassName( pbvr::CellByCellUniformSampling );

    // Module information.
    typedef pbvr::MapperBase::ModuleTag ModuleCategory;
    kvsModuleBaseClass( pbvr::MapperBase );
    kvsModuleSuperClass( pbvr::PointObject );

private:

    const kvs::Camera*     m_camera;         ///< camera (reference)
    size_t                 m_subpixel_level; ///< subpixel level
    float                  m_sampling_step;  ///< sampling step in the object coordinate
    float                  m_object_depth;   ///< object depth
    kvs::ValueArray<float> m_density_map;    ///< density map
    bool                   m_gt5d_full;
    CropRegion             m_crop;
    TransferFunctionSynthesizer* m_transfer_function_synthesizer;
    std::vector<pbvr::TransferFunction> m_transfer_function_array; 

    const size_t m_normal_ingredient;

    float                  m_particle_density;

    bool                   m_batch;


public:
    CellByCellUniformSampling();

    CellByCellUniformSampling(
        const pbvr::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const pbvr::TransferFunction& transfer_function,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const size_t                 normal_ingredient,
        const CropRegion&            crop,
        const float                  object_depth = 0.0f );

    CellByCellUniformSampling(
        const kvs::Camera&           camera,
        const pbvr::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const pbvr::TransferFunction& transfer_function,
        std::vector<pbvr::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        //TransferFunctionSynthesizer* transfunc_synthesizer,
        const size_t                 normal_ingredient,
        const CropRegion&            crop,
        const float                  particle_density,
        const bool                   batch,
        const float                  object_depth = 0.0f );

    CellByCellUniformSampling(
        const kvs::Camera&           camera,
        const pbvr::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const pbvr::TransferFunction& transfer_function,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const size_t                 normal_ingredient,
        const CropRegion&            crop,
        const bool                   gt5d_full,
        const float                  object_depth = 0.0f );

    virtual ~CellByCellUniformSampling();

public:

    SuperClass* exec( const pbvr::ObjectBase& object );

public:

    const size_t subpixelLevel() const;

    const float samplingStep() const;

    const float objectDepth() const;

    void attachCamera( const kvs::Camera& camera );

    void setSubpixelLevel( const size_t subpixel_level );

    void setSamplingStep( const float sampling_step );

    void setObjectDepth( const float object_depth );

private:

    void mapping( const kvs::Camera& camera, const pbvr::StructuredVolumeObject& volume );

    void mapping( const kvs::Camera& camera, const pbvr::UnstructuredVolumeObject& volume );
    
    const size_t calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    kvs::MersenneTwister* MT ); 
    
    template <typename T>
    void generate_particles( const pbvr::StructuredVolumeObject& volume );

    template <typename T>
    void generate_particles( const pbvr::UnstructuredVolumeObject& volume );
    
#ifdef ENABLE_MPI
    void generate_particles_gt5d( const pbvr::UnstructuredVolumeObject* volume );
#endif

};

} // end of namespace pbvr

#endif // PBVR__CELL_BY_CELL_UNIFORM_SAMPLING_H_INCLUDE
