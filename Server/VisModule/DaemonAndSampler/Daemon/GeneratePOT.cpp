#include "GeneratePOT.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileReader>

namespace
{

std::string EnvValueOrUnsetIS( const char* name )
{
    const char* value = std::getenv( name );
    return value ? std::string( value ) : std::string( "(unset)" );
}

void PrintMissingParameterFileWarning(
    const std::string& parameter_name,
    const std::string& file_name,
    const std::string& default_parameter_message
)
{
    std::cout << "================================================================" << std::endl;
    std::cout << "[WARN] " << parameter_name << " does not exist." << std::endl;
    std::cout << "[WARN] File: " << file_name << std::endl;
    std::cout << "[INFO] VIS_PARAM_DIR = " << EnvValueOrUnsetIS( "VIS_PARAM_DIR" ) << std::endl;
    std::cout << "[INFO] PARTICLE_DIR  = " << EnvValueOrUnsetIS( "PARTICLE_DIR" ) << std::endl;
    std::cout << "[INFO] " << default_parameter_message << std::endl;
    std::cout << "================================================================" << std::endl;
}

} // namespace

void SetDefaultPOTParameter( PlotOverTimeProperty& pot_property )
{
    pot_property.m_plot_flag = false;
    pot_property.m_target_point[0] = 0;
    pot_property.m_target_point[1] = 0;
    pot_property.m_target_point[2] = 0;
}

void SetDefaultPOTParameterIS( PlotOverTimeProperty& pot_property )
{
    const char *envBuf = NULL;
    std::string visParamDir;
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

    plotOverTimeParameterPath_old =  visParamDir;
    plotOverTimeParameterPath_old += "parameter_old.pot";

    std::ifstream plotOverTimeParameterFileOld( plotOverTimeParameterPath_old );
    if ( !plotOverTimeParameterFileOld.good() )
    {
        PrintMissingParameterFileWarning(
            "Plot over time parameter file",
            "parameter_old.pot",
            "Set default plot over time parameters."
        );
        SetDefaultPOTParameter( pot_property );
        return;
    }

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
