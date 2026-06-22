#ifndef  __PLOT_OVER_TIME_PROPERTY__
#define  __PLOT_OVER_TIME_PROPERTY__

#include "TaskSignal.h"

#include <cstddef>
#include <cstdint>

#include <vismodule/Serializer>

struct PlotOverTimeProperty
{
    bool m_plot_flag;
    float m_target_point[3];

    int32_t byteSize() const
    {
        int32_t size = 0;
        TaskSignal task_signal = TaskSignal::UPDATE_PLOT_OVER_TIME_PROPERTY;

        size += vismodule::Serializer::byteSize( task_signal );
        size += vismodule::Serializer::byteSize( m_plot_flag );
        for ( std::size_t i = 0; i < 3; i++ )
        {
            size += vismodule::Serializer::byteSize( m_target_point[i] );
        }

        return size;
    }

    std::size_t pack( char* buf ) const
    {
        std::size_t index = 0;
        TaskSignal task_signal = TaskSignal::UPDATE_PLOT_OVER_TIME_PROPERTY;

        index += vismodule::Serializer::write( buf + index, task_signal );
        index += vismodule::Serializer::write( buf + index, m_plot_flag );
        for ( std::size_t i = 0; i < 3; i++ )
        {
            index += vismodule::Serializer::write( buf + index, m_target_point[i] );
        }

        return index;
    }

    std::size_t unpack( const char* buf )
    {
        std::size_t index = 0;
        TaskSignal task_signal;

        index += vismodule::Serializer::read( buf + index, &task_signal );
        index += vismodule::Serializer::read( buf + index, &m_plot_flag );
        for ( std::size_t i = 0; i < 3; i++ )
        {
            index += vismodule::Serializer::read( buf + index, &m_target_point[i] );
        }

        return index;
    }
};

#endif // __PLOT_OVER_TIME_PROPERTY__
