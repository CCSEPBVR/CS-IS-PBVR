/****************************************************************************/
/**
 *  @file PlotOverLineGenerator.h
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

#ifndef VIS_MODULE__POL_GENERATOR_H_INCLUDE
#define VIS_MODULE__POL_GENERATOR_H_INCLUDE

#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/VolumeObjectBase>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/KVSMLObjectPlotOverLine>
#include <vismodule/PlotOverLineProperty>

class PlotOverLineGenerator
{
public:

    PlotOverLineGenerator(){}
    ~PlotOverLineGenerator(){}

public:
    void GeneratePOLStruct(
        const PlotOverLineProperty& pol_property,
        const domain_parameters_struct& dom,
        Type** values,
        int nvariables,
        vismodule::KVSMLObjectPlotOverLine* object
    );

    void GeneratePOLUnstruct(
        const PlotOverLineProperty& pol_property,
        Type** values,
        int nvariables,
        float* coordinates,
        int ncoords,
        unsigned int* connections,
        int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype,
        vismodule::KVSMLObjectPlotOverLine* object
    );
};

#endif
