#ifndef VIS_MODULE_GENERATE_PLOT_OVER_LINE_H_INCLDE
#define VIS_MODULE_GENERATE_PLOT_OVER_LINE_H_INCLDE

#include <vismodule/ParticleProperty>
#include <vismodule/PlotOverLineProperty>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/KVSMLObjectPlotOverLine>

void SetDefaultPOLParameterCS( PlotOverLineProperty& pol_property );

std::unique_ptr<vismodule::KVSMLObjectPlotOverLine> GeneratePOLCS(
    std::string& file_path,
    const int time_step,
    const PlotOverLineProperty& pol_property,
    MultiVolumePropertyList& mvpl
);

/*
void SetPOLParameterIS(
    PlotOverLineProperty& pol_property
);
*/

std::unique_ptr<vismodule::KVSMLObjectPlotOverLine> GeneratePOLIS(
    const int time_step,
    const PlotOverLineProperty& pol_property,
    MultiVolumePropertyList& mvpl
);

#endif
