#ifndef __JUPITER_PLOT_OVER_TIME_FILE_H__
#define __JUPITER_PLOT_OVER_TIME_FILE_H__

#include <string>
#include <vismodule/KVSMLObjectPlotOverTime>

class PlotOverTimeFile
{
private:
    vismodule::UInt32 m_subvolume_number;
    vismodule::UInt32 m_initial_step;
    vismodule::UInt32 m_final_step;
    vismodule::UInt32 m_kvsml_file_number;
    std::string m_file_prefix;

public:
    void setFilePrefix( const std::string& prefix );
    void setParameterFromFile();
    bool generatePOTObject( const int time_step, vismodule::KVSMLObjectPlotOverTime *object );
};

#endif