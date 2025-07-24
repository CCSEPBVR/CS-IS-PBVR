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
#ifndef VIS_MODULE__CELL_BY_CELL_UNIFORM_SAMPLING_H_INCLUDE
#define VIS_MODULE__CELL_BY_CELL_UNIFORM_SAMPLING_H_INCLUDE

#include <vismodule/MapperBase>
#include <vismodule/Camera>
#include <vismodule/PointObject>
#include <vismodule/VolumeObjectBase>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/MersenneTwister> 
#include <vismodule/CellByCellParticleGenerator>
//#include <vismodule/CropRegion>
#include <vismodule/TransferFunctionSynthesizer>
//#include "TransferFunctionSynthesizer_IS.h"
#include <vismodule/TransferFunctionSynthesizerCreator>
#include "ExtendedTransferFunction.h"
//#include "SFMT/SFMT.h" 

#define RANK 1

typedef unsigned char Byte;

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Cell-by-cell particle generation class.
 */
/*===========================================================================*/
class CellByCellUniformSampling : public vismodule::MapperBase, public vismodule::PointObject
{
    // Class name.
    visModuleClassName( vismodule::CellByCellUniformSampling );

    // Module information.
    typedef vismodule::MapperBase::ModuleTag ModuleCategory;
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PointObject );

private:

    const vismodule::Camera*     m_camera;         ///< camera (reference)
    size_t                 m_subpixel_level; ///< subpixel level
    float                  m_sampling_step;  ///< sampling step in the object coordinate
    float                  m_object_depth;   ///< object depth
    vismodule::ValueArray<float> m_density_map;    ///< density map
    TransferFunctionSynthesizer* m_transfer_function_synthesizer;
    std::vector<vismodule::TransferFunction> m_transfer_function_array; 

    const size_t m_normal_ingredient;

    float                  m_particle_density;

public:
    CellByCellUniformSampling();

    CellByCellUniformSampling(
        const vismodule::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const size_t                 normal_ingredient,
        const float                  object_depth = 0.0f );

    CellByCellUniformSampling(
        const vismodule::Camera&           camera,
        const vismodule::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const size_t                 normal_ingredient,
        const float                  particle_density,
        const float                  object_depth = 0.0f );

    CellByCellUniformSampling(
        const vismodule::Camera&           camera,
        const vismodule::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const size_t                 normal_ingredient,
        const float                  object_depth = 0.0f );

    virtual ~CellByCellUniformSampling();

public:

    SuperClass* exec( const vismodule::ObjectBase& object );

public:

    const size_t subpixelLevel() const;

    const float samplingStep() const;

    const float objectDepth() const;

    void attachCamera( const vismodule::Camera& camera );

    void setSubpixelLevel( const size_t subpixel_level );

    void setSamplingStep( const float sampling_step );

    void setObjectDepth( const float object_depth );

private:

    void mapping( const vismodule::Camera& camera, const vismodule::StructuredVolumeObject& volume );

    void mapping( const vismodule::Camera& camera, const vismodule::UnstructuredVolumeObject& volume );
    
    const size_t calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    vismodule::MersenneTwister* MT ); 
    
    template <typename T>
    void generate_particles( const vismodule::StructuredVolumeObject& volume );

    template <typename T>
    void generate_particles( const vismodule::UnstructuredVolumeObject& volume );
 
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
                          //const float o_scalars[][SIMDW], // åæå¤
                          //const float c_scalars[][SIMDW],
                          float** o_scalars, // åæå¤
                          float** c_scalars,
                          const int tf_number,
                          const int ncells );
    
    vismodule::Vector3f RandomSamplingInCube( const vismodule::Vector3f vertex, vismodule::MersenneTwister* MT );

   
    const float calculate_maximum_density( const float scalar0, const float scalar1 );
#ifdef ENABLE_MPI
    void generate_particles_gt5d( const vismodule::UnstructuredVolumeObject& volume );
#endif
};

} // end of namespace vismodule

#endif // VIS_MODULE__CELL_BY_CELL_UNIFORM_SAMPLING_H_INCLUDE
