/****************************************************************************/
/**
 *  @file PlotOverTimeGenerator.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PlotOverLineGenerator.h 634 2025-01-31 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/

#ifndef VIS_MODULE__POT_GENERATOR_H_INCLUDE
#define VIS_MODULE__POT_GENERATOR_H_INCLUDE

#include <vismodule/VolumeObjectBase>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/PlotOverTimeProperty>

class PlotOverTimeGenerator
{
public:

    PlotOverTimeGenerator(){}
    ~PlotOverTimeGenerator(){}

public:
    bool GeneratePOTStruct(
        const PlotOverTimeProperty& pot_property,
        const domain_parameters_struct& dom,
        Type** values,
        int nvariables,
        std::vector<float>& value_on_time
    );

    bool GeneratePOTUnstruct(
        const PlotOverTimeProperty& pot_property,
        Type** values,
        int nvariables,
        float* coordinates,
        int ncoords,
        unsigned int* connections,
        int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype,
        std::vector<float>& value_on_time
    );
};

#endif