/****************************************************************************/
/**
 *  @file CellByCellRejectionSampling.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellByCellRejectionSampling.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__CELL_BY_CELL_REJECTION_SAMPLING_H_INCLUDE
#define PBVR__CELL_BY_CELL_REJECTION_SAMPLING_H_INCLUDE

#include "MapperBase.h"
#include <vismodule/Camera>
#include "PointObject.h"
#include "VolumeObjectBase.h"
#include "StructuredVolumeObject.h"
#include "UnstructuredVolumeObject.h"
#include "ClassName.h"
#include <vismodule/Module>
#include <vismodule/MersenneTwister> 
#include "CellByCellParticleGenerator.h"
#include "CropRegion.h"
#include "TransferFunctionSynthesizer.h"
//#include "TransferFunctionSynthesizer_IS.h"
#include "TransferFunctionSynthesizerCreator.h"
#include "ExtendedTransferFunction.h"
//#include "SFMT/SFMT.h" 

#include "timer.h"

#define RANK 1
   typedef struct 
    {   
        float x_global_min;
        float y_global_min;
        float z_global_min;

        float x_global_max;
        float y_global_max;
        float z_global_max;
    } domain_parameters;
    
   typedef unsigned char Byte;

namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  Cell-by-cell particle generation class.
 */
/*===========================================================================*/
class CellByCellRejectionSampling : public pbvr::MapperBase, public pbvr::PointObject
{
    // Class name.
    visModuleClassName( pbvr::CellByCellRejectionSampling );

    // Module information.
    typedef pbvr::MapperBase::ModuleTag ModuleCategory;
    visModuleBaseClass( pbvr::MapperBase );
    visModuleSuperClass( pbvr::PointObject );

private:

    const vismodule::Camera*     m_camera;         ///< camera (reference)
    size_t                 m_subpixel_level; ///< subpixel level
    float                  m_sampling_step;  ///< sampling step in the object coordinate
    float                  m_object_depth;   ///< object depth
    vismodule::ValueArray<float> m_density_map;    ///< density map
    CropRegion             m_crop;
    //add by shimomura 2022/12/19
    TransferFunctionSynthesizer* m_transfer_function_synthesizer;
    std::vector<pbvr::TransferFunction> m_transfer_function_array; 
    //std::vector<NamedTransferFunction> m_transfer_function_array; 

    float                  m_particle_density;

    bool                   m_batch;

public:

    CellByCellRejectionSampling();

    CellByCellRejectionSampling(
        const pbvr::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const pbvr::TransferFunction& transfer_function,
        //const NamedTransferFunction& transfer_function,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const CropRegion&            crop,
        const float                  object_depth = 0.0f );

    CellByCellRejectionSampling(
        const vismodule::Camera&           camera,
        const pbvr::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const pbvr::TransferFunction& transfer_function,
        //const NamedTransferFunction& transfer_function,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const CropRegion&            crop,
        const float                  density_factor,
        const float                  object_depth = 0.0f );

    CellByCellRejectionSampling(
        const vismodule::Camera&           camera,
        const pbvr::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const pbvr::TransferFunction& transfer_function,
        //const NamedTransferFunction& transfer_function,
        //std::vector<NamedTransferFunction>& transfer_function_array,
        std::vector<pbvr::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const CropRegion&            crop,
        const float                  density_factor,
        const bool                   batch,
        const float                  object_depth = 0.0f );

    virtual ~CellByCellRejectionSampling();

public:

    SuperClass* exec( const pbvr::ObjectBase& object );

public:

    const size_t subpixelLevel() const;

    const float samplingStep() const;

    const float objectDepth() const;

    void attachCamera( const vismodule::Camera& camera );

    void setSubpixelLevel( const size_t subpixel_level );

    void setSamplingStep( const float sampling_step );

    void setObjectDepth( const float object_depth );

private:

    void mapping( const vismodule::Camera& camera, const pbvr::StructuredVolumeObject& volume );

    void mapping( const vismodule::Camera& camera, const pbvr::UnstructuredVolumeObject& volume );

    template <typename T>
    void generate_particles( const pbvr::StructuredVolumeObject& volume );

    template <typename T>
    void generate_particles( const pbvr::UnstructuredVolumeObject& volume );

    const float calculate_density( const float scalar );

    const size_t calculate_number_of_particles( const float density, const float volume_of_cell );

    void calculate_histogram( vismodule::ValueArray<int>&   th_o_histogram,
                          vismodule::ValueArray<int>&   th_c_histogram,
                          vismodule::ValueArray<float>& th_O_min,
                          vismodule::ValueArray<float>& th_O_max,
                          vismodule::ValueArray<float>& th_C_min,
                          vismodule::ValueArray<float>& th_C_max,
                          // ここまでoutput, 以下input
                          const int nbins, // TFSから読み込む最大最小値
                          const vismodule::ValueArray<float>& o_min,
                          const vismodule::ValueArray<float>& o_max,
                          const vismodule::ValueArray<float>& c_min,
                          const vismodule::ValueArray<float>& c_max,
 //                         const float o_scalars[][SIMDW], // åæå¤
 //                         const float c_scalars[][SIMDW],
                          float** o_scalars, // åæå¤
                          float** c_scalars,
                          const int tf_number,
                          const int ncells_remain  );
    
    vismodule::Vector3f RandomSamplingInCube( const vismodule::Vector3f vertex, vismodule::MersenneTwister* MT );


    //add by shimomura  2022/12/19
    const size_t calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    vismodule::MersenneTwister* MT ); 

    const float calculate_maximum_density( const float scalar0, const float scalar1 );
};

} // end of namespace pbvr

#endif // PBVR__CELL_BY_CELL_REJECTION_SAMPLING_H_INCLUDE
