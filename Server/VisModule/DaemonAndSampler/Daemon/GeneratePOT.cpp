#include "GeneratePOT.h"

#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileReader>

void SetDefaultPOTParameterIS( PlotOverTimeProperty& pot_property )
{
    const char *envBuf = NULL;
    std::string visParamDir;
    std::string plotOverTimeParameterPath;
    std::string plotOverTimeParameterPath_old;
    ParameterFileReader ppr;

    envBuf = std::getenv( "VIS_PARAM_DIR" );

    if (envBuf == NULL) {
        visParamDir = "./";
    }
    else {
        visParamDir = envBuf;
        if (visParamDir[visParamDir.size() - 1] != '/') {
            visParamDir += "/";
        }
    }

    plotOverTimeParameterPath     =  visParamDir;
    plotOverTimeParameterPath_old =  visParamDir;
    plotOverTimeParameterPath     += "parameter.pot";
    plotOverTimeParameterPath_old += "parameter_old.pot";

    ppr.readPlotOverTimeParameterFile( plotOverTimeParameterPath_old.c_str() );
    ppr.setPlotOverTimeParameter( pot_property );
}

std::unique_ptr<vismodule::KVSMLObjectPlotOverTime> GeneratePOTIS(
    const int time_step,
    const PlotOverTimeProperty& pot_property
)
{
    ParticleMonitor pm;
    pm.check();

    if( pm.stepExisted() )
    {
        pm.setTimeStep_pot( time_step );
    }
    else
    {
        pm.setTimeStep_pot(0);
    }

    vismodule::KVSMLObjectPlotOverTime* tmp_obj = new vismodule::KVSMLObjectPlotOverTime;
    bool mask = false;
    vismodule::ValueArray<float> values_on_time;
    
    // get plot over line
    pm.readPlotOverTimeFile();
    pm.getPlotOverTime( tmp_obj );

    mask = tmp_obj->mask();

    if ( mask )
    {
        values_on_time.allocate( tmp_obj->values_on_time().size() );
        values_on_time.fill( 0x00 );
        memcpy( values_on_time.pointer(), tmp_obj->values_on_time().pointer(), tmp_obj->values_on_time().byteSize() );
    }

    delete tmp_obj;

    return std::make_unique<vismodule::KVSMLObjectPlotOverTime>( values_on_time, mask );
}