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
#ifndef VIS_MODULE__CELL_BY_CELL_REJECTION_SAMPLING_H_INCLUDE
#define VIS_MODULE__CELL_BY_CELL_REJECTION_SAMPLING_H_INCLUDE

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
#include <vismodule/TransferFunctionSynthesizerCreator>
#include "ExtendedTransferFunction.h"

#define RANK 1
   
   typedef unsigned char Byte;

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Cell-by-cell particle generation class.
 */
/*===========================================================================*/
class CellByCellRejectionSampling : public vismodule::MapperBase, public vismodule::PointObject
{
    // Class name.
    visModuleClassName( vismodule::CellByCellRejectionSampling );

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
    //add by shimomura 2022/12/19
    TransferFunctionSynthesizer* m_transfer_function_synthesizer;
    std::vector<vismodule::TransferFunction> m_transfer_function_array; 

    float                  m_particle_density;
    vismodule::CoordSynthesizerStrings* m_coord_synthesizer_strings;

public:

    CellByCellRejectionSampling();

    CellByCellRejectionSampling(
        const vismodule::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const float                  object_depth = 0.0f );

    CellByCellRejectionSampling(
        const vismodule::Camera&           camera,
        const vismodule::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const float                  density_factor,
        const float                  object_depth = 0.0f );

    CellByCellRejectionSampling(
        const vismodule::Camera&           camera,
        const vismodule::VolumeObjectBase& volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const float                  density_factor,
        const float                  object_depth = 0.0f );

//    //constructor for unstruct 
//    CellByCellRejectionSampling(
//        const vismodule::Camera&           camera,
//        Type** values, int nvariables,
//        float* coordinates, int ncoords,
//        unsigned int* connections, int ncells,
//        const  vismodule::VolumeObjectBase::CellType& celltype ,
//        const size_t                 subpixel_level,
//        const float                  sampling_step,
//        const vismodule::TransferFunction& transfer_function,
//        std::vector<vismodule::TransferFunction>& transfer_function_array,
//        TransferFunctionSynthesizer* transfunc_synthesizer,
//        const float                  density_factor,
//        const float                  object_depth = 0.0f );

    //constructor for unstruct 
    CellByCellRejectionSampling(
        const vismodule::Camera&  camera,
        domain_parameters_unstruct dom,
        Type** values, int nvariables,
        float* coordinates, int ncoords,
        unsigned int* connections, int ncells,
        const  vismodule::VolumeObjectBase::CellType& celltype ,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const float                  paritcle_density,
        vismodule::CoordSynthesizerStrings* coord_synthesizer_strings);

    //constructor for struct 
        CellByCellRejectionSampling(
        const vismodule::Camera&  camera,
        domain_parameters_struct dom, 
        Type** values,  
        int nvariables, 
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const float                  paritcle_density,
        vismodule::CoordSynthesizerStrings* coord_synthesizer_strings);


// for IS 
    CellByCellRejectionSampling( Type** values, int nvariables,
            float* coordinates, int ncoords,
            unsigned int* connections, int ncells, const  vismodule::VolumeObjectBase::CellType& celltype, //ISPBVR
            std::vector<vismodule::TransferFunction>& transfer_function_array,
            TransferFunctionSynthesizer* transfunc_synthesizer,
            const size_t                 subpixel_level,
            const float                  density_factor );


    virtual ~CellByCellRejectionSampling();

public:

    SuperClass* exec( const vismodule::ObjectBase& object );
    SuperClass* exec_IS( const vismodule::ObjectBase& object );

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

    template <typename T>
    void generate_particles( const vismodule::StructuredVolumeObject& volume );

    SuperClass* generate_particles_struct( domain_parameters_struct dom, 
            Type** values, int nvariables);

    template <typename T>
    void generate_particles( const vismodule::UnstructuredVolumeObject& volume );

    SuperClass* generate_particles_unstruct(  domain_parameters_unstruct dom,Type** values, int nvariables,
        float* coordinates, int ncoords,
        unsigned int* connections, int ncells,
        const  vismodule::VolumeObjectBase::CellType& celltype) ;
 
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

} // end of namespace vismodule

#endif // VIS_MODULE__CELL_BY_CELL_REJECTION_SAMPLING_H_INCLUDE
