#ifndef VIS_MODULE_GENERATE_PLOT_OVER_LINE_H_INCLDE
#define VIS_MODULE_GENERATE_PLOT_OVER_LINE_H_INCLDE

#include <vismodule/ParticleProperty>
#include <vismodule/PlotOverLineProperty>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/KVSMLObjectPlotOverLine>

void SetPOLParameterCS(
    PlotOverLineProperty& pol_property
);

std::unique_ptr<vismodule::KVSMLObjectPlotOverLine> GeneratePOLCS(
    ParticleProperty& particle_property,
    const PlotOverLineProperty& pol_property,
    MultiVolumePropertyList& mvpl
);

#endif
