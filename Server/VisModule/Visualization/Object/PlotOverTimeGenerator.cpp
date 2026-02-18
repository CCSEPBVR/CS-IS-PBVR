#include <vismodule/PlotOverTimeGenerator>
#include <vismodule/PlotOverTimeProperty>
#include <vismodule/PlotOverTime>

bool PlotOverTimeGenerator::GeneratePOTStruct(
    const PlotOverTimeProperty& pot_property,
    const domain_parameters_struct& dom,
    Type** values,
    int nvariables,
    std::vector<float>& value_on_time
)
{
    PlotOverTime plot_over_time(
        dom,
        values,
        nvariables,
        pot_property
    );

    memcpy( value_on_time.data(), plot_over_time.values().pointer(), sizeof( float ) * nvariables );

    return plot_over_time.mask();
}

bool PlotOverTimeGenerator::GeneratePOTUnstruct(
    const PlotOverTimeProperty& pot_property,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    std::vector<float>& value_on_time
)
{
    PlotOverTime plot_over_time(
        values,
        nvariables,
        coordinates,
        ncoords,
        connections,
        ncells,
        celltype,
        pot_property
    );

    memcpy( value_on_time.data(), plot_over_time.values().pointer(), sizeof( float ) * nvariables );

    return plot_over_time.mask();
}