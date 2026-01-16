/****************************************************************************/
/**
 *  @file CellByCellMinMax.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellByCellMinMax.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__CELL_BY_CELL_MINMAX_H_INCLUDE
#define VIS_MODULE__CELL_BY_CELL_MINMAX_H_INCLUDE

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
class CellByCellMinMax : public vismodule::MapperBase, public vismodule::PointObject
{
    // Class name.
    visModuleClassName( vismodule::CellByCellMinMax );

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
//    CropRegion             m_crop;
    TransferFunctionSynthesizer* m_transfer_function_synthesizer;
    std::vector<vismodule::TransferFunction> m_transfer_function_array; 

    const size_t m_normal_ingredient;

    vismodule::CoordSynthesizerStrings* m_coord_synthesizer_strings;

public:
    CellByCellMinMax();

    //constructor for unstruct 
    CellByCellMinMax(
        domain_parameters_unstruct dom,
        Type** values, int nvariables,
        float* coordinates, int ncoords,
        unsigned int* connections, int ncells,
        const  vismodule::VolumeObjectBase::CellType& celltype ,
        const vismodule::TransferFunction& transfer_function,
        const std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        vismodule::CoordSynthesizerStrings* coord_synthesizer_strings);

    //constructor for struct 
        CellByCellMinMax(
        domain_parameters_struct dom, 
        Type** values,  
        int nvariables, 
        const vismodule::TransferFunction& transfer_function,
        const std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        vismodule::CoordSynthesizerStrings* coord_synthesizer_strings);

    virtual ~CellByCellMinMax();

public:

     // MapperBaseクラスのvirtual 関数をオーバーライドするため空関数を宣言
    SuperClass* exec( const vismodule::ObjectBase& object ){ return NULL; };

public:

    const size_t subpixelLevel() const;

    const float samplingStep() const;

    const float objectDepth() const;

    void attachCamera( const vismodule::Camera& camera );

    void setSubpixelLevel( const size_t subpixel_level );

    void setSamplingStep( const float sampling_step );

    void setObjectDepth( const float object_depth );

    // struct
    void generate_minmax_struct( domain_parameters_struct dom, 
            Type** values, int nvariables);
    // unstruct
    void generate_minmax_unstruct(  domain_parameters_unstruct dom,Type** values, int nvariables,
        float* coordinates, int ncoords,
        unsigned int* connections, int ncells,
        const  vismodule::VolumeObjectBase::CellType& celltype) ;
   
};

} // end of namespace vismodule

#endif // VIS_MODULE__CELL_BY_CELL_HISTOGRAM_H_INCLUDE
