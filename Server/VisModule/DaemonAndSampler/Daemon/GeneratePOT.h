#ifndef VIS_MODULE_GENERATE_PLOT_OVER_TIME_H_INCLDE
#define VIS_MODULE_GENERATE_PLOT_OVER_TIME_H_INCLDE

#include <vector>
#include <memory>

#include <vismodule/PlotOverTimeProperty>
#include <vismodule/KVSMLObjectPlotOverTime>

class MultiVolumePropertyList;

// A missing point is a successful sample with mask=false. A read/compute
// failure is never eligible for reuse, even if another subvolume has a value.
struct POTSample
{
    std::vector<float> values;
    bool mask = false;
    bool success = false;
};

POTSample GeneratePOTCS(
    const std::string& file_path, int time_step,
    const PlotOverTimeProperty& pot_property, MultiVolumePropertyList& mvpl );

void SetDefaultPOTParameter( PlotOverTimeProperty& pot_property );

void SetDefaultPOTParameterIS( PlotOverTimeProperty& pot_property );

bool GeneratePOTIS(
    const int time_step,
    const PlotOverTimeProperty& pot_property,
    std::unique_ptr<vismodule::KVSMLObjectPlotOverTime>& kvsml_object_pot
);

#endif
