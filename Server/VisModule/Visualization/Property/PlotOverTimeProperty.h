#ifndef  __PLOT_OVER_TIME_PROPERTY__
#define  __PLOT_OVER_TIME_PROPERTY__

#include <cstdint>
#include "TaskSignal.h"
#include <vismodule/Serializer>

struct PlotOverTimeProperty
{
    bool m_plot_flag;
    float m_target_point[3];

    int32_t byteSize() const
    {
        return sizeof(TaskSignal) + sizeof(m_plot_flag) + sizeof(m_target_point);
    }

    size_t pack( char* buf ) const
    {
        size_t index = 0;
        const TaskSignal signal = TaskSignal::UPDATE_PLOT_OVER_TIME_PROPERTY;
        index += vismodule::Serializer::write( buf + index, signal );
        index += vismodule::Serializer::write( buf + index, m_plot_flag );
        for ( const float coordinate : m_target_point )
            index += vismodule::Serializer::write( buf + index, coordinate );
        return index;
    }

    size_t unpack( const char* buf )
    {
        size_t index = 0;
        TaskSignal signal;
        index += vismodule::Serializer::read( buf + index, &signal );
        index += vismodule::Serializer::read( buf + index, &m_plot_flag );
        for ( float& coordinate : m_target_point )
            index += vismodule::Serializer::read( buf + index, &coordinate );
        return index;
    }
};

#endif // __PLOT_OVER_TIME_PROPERTY__
