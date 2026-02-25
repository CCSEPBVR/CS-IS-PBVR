#ifndef VIS_MODULE_GENERATE_PLOT_OVER_TIME_H_INCLDE
#define VIS_MODULE_GENERATE_PLOT_OVER_TIME_H_INCLDE

#include <vector>
#include <memory>

#include <vismodule/PlotOverTimeProperty>
#include <vismodule/KVSMLObjectPlotOverTime>

void SetDefaultPOTParameterIS( PlotOverTimeProperty& pot_property );

std::unique_ptr<vismodule::KVSMLObjectPlotOverTime> GeneratePOTIS(
    const int time_step,
    const PlotOverTimeProperty& pot_property
);

#endif
