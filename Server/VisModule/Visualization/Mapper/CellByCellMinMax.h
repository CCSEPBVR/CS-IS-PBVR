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

#include <vector>
#include <vismodule/MapperBase>
#include <vismodule/PointObject>
#include <vismodule/VolumeObjectBase>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/TransferFunctionSynthesizerCreator>
#include "ExtendedTransferFunction.h"

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

    TransferFunctionSynthesizer* m_transfer_function_synthesizer;
    std::vector<vismodule::TransferFunction> m_transfer_function_array; 

public:

    //constructor for unstruct 
    CellByCellMinMax(
        Type** values, int nvariables,
        float* coordinates, int ncoords,
        unsigned int* connections, int ncells,
        const  vismodule::VolumeObjectBase::CellType& celltype ,
        const vismodule::TransferFunction& transfer_function,
        const std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer
    );

    //constructor for struct 
    CellByCellMinMax(
        domain_parameters_struct dom, 
        Type** values,  
        int nvariables, 
        const vismodule::TransferFunction& transfer_function,
        const std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer
    );

    virtual ~CellByCellMinMax();

public:

     // MapperBaseクラスのvirtual 関数をオーバーライドするため空関数を宣言
    SuperClass* exec( const vismodule::ObjectBase& object ){ return NULL; };

public:

    // struct
    void generate_minmax_struct(
        domain_parameters_struct dom, 
        Type** values,
        int nvariables
    );

    // unstruct
    void generate_minmax_unstruct(
        Type** values,
        int nvariables,
        float* coordinates,
        int ncoords,
        unsigned int* connections,
        int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype
    );
   
};

} // end of namespace vismodule

#endif // VIS_MODULE__CELL_BY_CELL_HISTOGRAM_H_INCLUDE
