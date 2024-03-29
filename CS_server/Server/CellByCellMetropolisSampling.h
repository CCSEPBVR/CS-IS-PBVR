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
#ifndef PBVR__CELL_BY_CELL_METROPOLIS_SAMPLING_H_INCLUDE
#define PBVR__CELL_BY_CELL_METROPOLIS_SAMPLING_H_INCLUDE

#include "MapperBase.h"
#include <kvs/Camera>
#include "PointObject.h"
#include "VolumeObjectBase.h"
#include "StructuredVolumeObject.h"
#include "UnstructuredVolumeObject.h"
#include "ClassName.h"
#include <kvs/MersenneTwister> 
#include <kvs/Module>
#include "CellByCellParticleGenerator.h"
#include "CropRegion.h"
#include "TransferFunctionSynthesizer.h"
#include "TransferFunctionSynthesizerCreator.h"
#include "ExtendedTransferFunction.h"

#include "timer.h"

//add by shimomura 2022/12/22
#define RANK 1
//   typedef struct 
//    {   
//        float x_global_min;
//        float y_global_min;
//        float z_global_min;
//
//        float x_global_max;
//        float y_global_max;
//        float z_global_max;
//    } domain_parameters;
    typedef unsigned char Byte;

namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  Cell-by-cell particle generation class.
 */
/*===========================================================================*/
class CellByCellMetropolisSampling : public pbvr::MapperBase, public pbvr::PointObject
{
    // Class name.
    kvsClassName( pbvr::CellByCellMetropolisSampling );

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
    CropRegion             m_crop;
    //add by shimomura 2022/12/19
    TransferFunctionSynthesizer* m_transfer_function_synthesizer;
    std::vector<pbvr::TransferFunction> m_transfer_function_array; 

    const size_t m_normal_ingredient;

    float                  m_particle_density;
    bool                   m_batch;

public:

    CellByCellMetropolisSampling();

    CellByCellMetropolisSampling(
        const pbvr::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const pbvr::TransferFunction& transfer_function,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const size_t                 normal_ingredient,
        const CropRegion&            crop,
        const float                  object_depth = 0.0f );

    CellByCellMetropolisSampling(
        const kvs::Camera&           camera,
        const pbvr::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const pbvr::TransferFunction& transfer_function,
        std::vector<pbvr::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const size_t                 normal_ingredient,
        const CropRegion&            crop,
        const float                  particle_density,
        const bool                   batch,
        const float                  object_depth = 0.0f );

    virtual ~CellByCellMetropolisSampling();

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

    template <typename T>
    void generate_particles( const pbvr::StructuredVolumeObject& volume );

    template <typename T>
    void generate_particles( const pbvr::UnstructuredVolumeObject& volume );

    const float calculate_density( const float scalar );

    const size_t calculate_number_of_particles( const float density, const float volume_of_cell );

    //add by shimomura  2022/12/19
    const size_t calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    kvs::MersenneTwister* MT ); 

    void calculate_histogram( kvs::ValueArray<int>&   th_o_histogram,
                          kvs::ValueArray<int>&   th_c_histogram,
                          kvs::ValueArray<float>& th_O_min,
                          kvs::ValueArray<float>& th_O_max,
                          kvs::ValueArray<float>& th_C_min,
                          kvs::ValueArray<float>& th_C_max,
                          // ここまでoutput, 以下input
                          const int nbins, // TFSから読み込む最大最小値
                          const kvs::ValueArray<float>& o_min,
                          const kvs::ValueArray<float>& o_max,
                          const kvs::ValueArray<float>& c_min,
                          const kvs::ValueArray<float>& c_max,
                          const float o_scalars[][SIMDW], // åæå¤
                          const float c_scalars[][SIMDW],
                          const int tf_number  );
    
    kvs::Vector3f RandomSamplingInCube( const kvs::Vector3f vertex, kvs::MersenneTwister* MT );

    /*** added by MHIR on /12/12/12/ ***/
    /*
    const float calculate_density( const float scalar );
    const float calculate_volume ();
    const size_t calculate_number_of_particles(
    				       const float density,
    				       const float volume_of_cell
    				       );
    */
    /***********************************/
};

} // end of namespace pbvr

#endif // PBVR__CELL_BY_CELL_METROPOLIS_SAMPLING_H_INCLUDE
