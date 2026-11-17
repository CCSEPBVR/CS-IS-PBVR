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
#include <vismodule/MersenneTwister> 
#include <vismodule/Module>
#include <vismodule/CellByCellParticleGenerator>
//#include <vismodule/CropRegion>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/TransferFunctionSynthesizerCreator>
#include "ExtendedTransferFunction.h"

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
    typedef vismodule::MapperBase::ModuleTag ModuleCategory;
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PointObject );

private:

    const vismodule::Camera*     m_camera;         ///< camera (reference)
    std::size_t                 m_subpixel_level; ///< subpixel level
    /*  粒子数の倍率。既定 1.0f で従来と同一（* 1.0f は IEEE754 で値を変えない）。
     *  アンサンブルメンバーを重ね描きするとき 1/N を渡し、全メンバー合計を
     *  1メンバー分に抑える。アンサンブル版 EnsembleParticleGenerator の
     *  repetition と同じ役割。 */
    float                       m_repetition = 1.0f;
    float                  m_sampling_step;  ///< sampling step in the object coordinate
    float                  m_object_depth;   ///< object depth
    vismodule::ValueArray<float> m_density_map;    ///< density map
//    CropRegion             m_crop;
    //add by shimomura 2022/12/19
    TransferFunctionSynthesizer* m_transfer_function_synthesizer;
    std::vector<vismodule::TransferFunction> m_transfer_function_array; 

    const std::size_t m_normal_ingredient;

    vismodule::CoordSynthesizerStrings* m_coord_synthesizer_strings;

public:

    CellByCellMetropolisSampling();
    //constructor for unstruct 
    CellByCellMetropolisSampling(
        domain_parameters_unstruct dom,
        Type** values, int nvariables,
        float* coordinates, int ncoords,
        unsigned int* connections, int ncells,
        const  vismodule::VolumeObjectBase::CellType& celltype ,
        const vismodule::TransferFunction& transfer_function,
        const std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        vismodule::CoordSynthesizerStrings* coord_synthesizer_strings,
        /*  粒子数の倍率。既定 1.0f で従来と同一。コンストラクタ内で粒子生成が
         *  走るため、setter ではなくここで受け取る必要がある。 */
        const float repetition = 1.0f);

    //constructor for struct 
        CellByCellMetropolisSampling(
        domain_parameters_struct dom, 
        Type** values,  
        int nvariables, 
        const vismodule::TransferFunction& transfer_function,
        const std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        vismodule::CoordSynthesizerStrings* coord_synthesizer_strings,
        /*  粒子数の倍率。既定 1.0f で従来と同一。コンストラクタ内で粒子生成が
         *  走るため、setter ではなくここで受け取る必要がある。 */
        const float repetition = 1.0f);

    virtual ~CellByCellMetropolisSampling();

public:

     // MapperBaseクラスのvirtual 関数をオーバーライドするため空関数を宣言
    SuperClass* exec( const vismodule::ObjectBase& object ){ return NULL; };

public:

    const std::size_t subpixelLevel() const;

    const float samplingStep() const;

    const float objectDepth() const;

    void attachCamera( const vismodule::Camera& camera );

    void setSubpixelLevel( const std::size_t subpixel_level );

    /*  粒子数の倍率。既定は 1.0f（従来と同一の粒子数）。
     *  注意: コンストラクタ内で粒子生成が走るため、生成後に呼んでも効かない。
     *  生成時に効かせるにはコンストラクタ引数で渡すこと。 */
    void setRepetition( const float repetition ) { m_repetition = repetition; }
    float repetition() const { return m_repetition; }

    void setSamplingStep( const float sampling_step );

    void setObjectDepth( const float object_depth );

private:

    void generate_particles_struct( domain_parameters_struct dom, 
            Type** values, int nvariables);
   
    void generate_particles_unstruct(  domain_parameters_unstruct dom,Type** values, int nvariables,
        float* coordinates, int ncoords,
        unsigned int* connections, int ncells,
        const  vismodule::VolumeObjectBase::CellType& celltype) ;


    const float calculate_density( const float scalar );

    const std::size_t calculate_number_of_particles( const float density, const float volume_of_cell );

    //add by shimomura  2022/12/19
    const std::size_t calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    vismodule::MersenneTwister* MT ); 

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

};

} // end of namespace vismodule

#endif // VIS_MODULE__CELL_BY_CELL_METROPOLIS_SAMPLING_H_INCLUDE
