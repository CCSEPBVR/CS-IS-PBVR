#include "PlotOverLineGenerator.h"
#include <vismodule/PlotOverLineProperty>
#include <vismodule/PlotOverLine>

using namespace vismodule;

void PlotOverLineGenerator::GeneratePOLStruct(
    const PlotOverLineProperty& pol_property,
    const domain_parameters_struct& dom,
    Type** values,
    int nvariables,
    vismodule::KVSMLObjectPlotOverLine* object
)
{
    PlotOverLine plot_over_line(
        dom,
        values,
        nvariables,
        pol_property
    );

    object->setValuesOnLine(plot_over_line.values());
    object->setXAxis(plot_over_line.xAxis());
    object->setMask(plot_over_line.mask());
}

void PlotOverLineGenerator::GeneratePOLUnstruct(
    const PlotOverLineProperty& pol_property,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    vismodule::KVSMLObjectPlotOverLine* object
)
{
    PlotOverLine plot_over_line( 
        values,
        nvariables,
        coordinates,
        ncoords,
        connections,
        ncells,
        celltype,
        pol_property
    );

    object->setValuesOnLine(plot_over_line.values());
    object->setXAxis(plot_over_line.xAxis());
    object->setMask(plot_over_line.mask());
}