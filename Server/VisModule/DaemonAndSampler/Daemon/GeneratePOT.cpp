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

bool GeneratePOTIS(
    const int time_step,
    const PlotOverTimeProperty& pot_property,
    std::unique_ptr<vismodule::KVSMLObjectPlotOverTime>& kvsml_object_pot
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
        // pm.setTimeStep_pot(0);
        std::cerr << __FILE__ << "," << __func__ << "," << __LINE__ << "ERROR: Time step is not exist." << std::endl;
        return false;
    }
    
    // get plot over line
    bool result = false;
    result = pm.readPlotOverTimeFile();
    if ( !result ) return false; // データファイルが存在しないタイムステップはスキップ
    pm.getPlotOverTime( kvsml_object_pot.get() );

    return true;
}