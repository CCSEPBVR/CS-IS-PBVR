#ifndef  __PLOT_OVER_LINE_PROPERTY__
#define  __PLOT_OVER_LINE_PROPERTY__

#include <string>
#include <vector>

#include "TaskSignal.h"

#include <vismodule/Serializer>

struct PlotOverLineProperty 
{
    bool m_plot_flag;
    std::string m_plot_variable;
    float m_start_point[3];
    float m_end_point[3];
    int32_t m_sampling_size; // resolution

    int32_t byteSize() const
    {
        int32_t size = 0;
        TaskSignal task_signal = TaskSignal::UPDATE_PLOT_OVER_LINE_PROPERTY;

        size += sizeof( task_signal );
        size += sizeof( m_plot_flag );
        size += sizeof( m_plot_variable );

        for ( size_t i = 0; i < 3; i++ )
        {
            size += sizeof( m_start_point[i] );
        }

        for ( size_t i = 0; i < 3; i++ )
        {
            size += sizeof( m_end_point[i] );
        }

        size += sizeof( m_sampling_size );

        return size;
    }

    size_t pack( char* buf ) const
    {
        size_t index = 0;
        TaskSignal task_signal = TaskSignal::UPDATE_PLOT_OVER_LINE_PROPERTY;

        index += vismodule::Serializer::write( buf + index, task_signal );
        index += vismodule::Serializer::write( buf + index, m_plot_flag );
        index += vismodule::Serializer::write( buf + index, m_plot_variable );

        for ( size_t i = 0; i < 3; i++ )
        {
            index += vismodule::Serializer::write( buf + index, m_start_point[i] );
        }

        for ( size_t i = 0; i < 3; i++ )
        {
            index += vismodule::Serializer::write( buf + index, m_end_point[i] );
        }

        index += vismodule::Serializer::write( buf + index, m_sampling_size );

        return index;
    }

    size_t unpack( char* buf )
    {
        size_t index = 0;
        TaskSignal task_signal;

        index += vismodule::Serializer::read( buf + index, &task_signal );
        index += vismodule::Serializer::read( buf + index, &m_plot_flag );
        index += vismodule::Serializer::read( buf + index, &m_plot_variable );

        for ( size_t i = 0; i < 3; i++ )
        {
            index += vismodule::Serializer::read( buf + index, &m_start_point[i] );
        }

        for ( size_t i = 0; i < 3; i++ )
        {
            index += vismodule::Serializer::read( buf + index, &m_end_point[i] );
        }

        index += vismodule::Serializer::read( buf + index, &m_sampling_size );

        return index;
    }

    void show( int rank ) const
    {
        std::cout << "===================== Plot Over Line Property ( rank : " << rank << ") START ====================" << std::endl;
        std::cout << "plot_flag       : " << m_plot_flag      << std::endl;
        std::cout << "plot_variable   : " << m_plot_variable  << std::endl;
        std::cout << "start_point[0]  : " << m_start_point[0] << std::endl;
        std::cout << "start_point[1]  : " << m_start_point[1] << std::endl;
        std::cout << "start_point[2]  : " << m_start_point[2] << std::endl;
        std::cout << "end_point[0]    : " << m_end_point[0]   << std::endl;
        std::cout << "end_point[1]    : " << m_end_point[1]   << std::endl;
        std::cout << "end_point[2]    : " << m_end_point[2]   << std::endl;
        std::cout << "m_sampling_size : " << m_sampling_size  << std::endl;
        std::cout << "===================== Plot Over Line Property ( rank : " << rank << ") END ====================" << std::endl;
    }
};

#endif // __PARAM_INFO_H__

