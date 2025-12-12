#ifndef  __PLOT_OVER_LINE_PROPERTY__
#define  __PLOT_OVER_LINE_PROPERTY__

#include <string>
#include <vector>

struct PlotOverLineProperty 
{
    bool m_plot_flag;
    std::string m_plot_variable;
    float m_start_point[3];
    float m_end_point[3];
    int32_t m_sampling_size; // resolution
};

#endif // __PARAM_INFO_H__

