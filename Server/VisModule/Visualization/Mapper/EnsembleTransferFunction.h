#ifndef VIS_MODULE__ENSEMBLE_TRANSFER_FUNCTION_H_INCLUDE
#define VIS_MODULE__ENSEMBLE_TRANSFER_FUNCTION_H_INCLUDE

#include <algorithm>
#include <cstdint>
#include <string>

#include <vismodule/FrequencyTable>
#include <vismodule/Serializer>
#include <vismodule/TransferFunction>
#include <vismodule/Type>

class EnsembleTransferFunction : public vismodule::TransferFunction
{
public:
    enum class ServerRangeMode : std::uint8_t
    {
        Unknown    = 0,
        UserRange  = 1,
        ServerSide = 2,
    };

public:
    int32_t m_resolution;
    int m_id;
    std::string m_label;
    std::string m_variable;
    float m_user_variable_min;
    float m_user_variable_max;
    float m_server_variable_min;
    float m_server_variable_max;
    ServerRangeMode m_server_range_mode;
    vismodule::UInt64 m_color_histogram[DEFAULT_NBINS];
    vismodule::UInt64 m_opacity_histogram[DEFAULT_NBINS];

public:
    EnsembleTransferFunction() :
        vismodule::TransferFunction( 256 ),
        m_resolution( 256 ),
        m_id( 0 ),
        m_user_variable_min( 0.0f ),
        m_user_variable_max( 0.0f ),
        m_server_variable_min( 0.0f ),
        m_server_variable_max( 0.0f ),
        m_server_range_mode( ServerRangeMode::Unknown )
    {
        std::fill_n( m_color_histogram, DEFAULT_NBINS, 0 );
        std::fill_n( m_opacity_histogram, DEFAULT_NBINS, 0 );
    }

    EnsembleTransferFunction( const vismodule::TransferFunction& tf ) :
        vismodule::TransferFunction( tf ),
        m_resolution( 256 ),
        m_id( 0 ),
        m_user_variable_min( 0.0f ),
        m_user_variable_max( 0.0f ),
        m_server_variable_min( 0.0f ),
        m_server_variable_max( 0.0f ),
        m_server_range_mode( ServerRangeMode::Unknown )
    {
        std::fill_n( m_color_histogram, DEFAULT_NBINS, 0 );
        std::fill_n( m_opacity_histogram, DEFAULT_NBINS, 0 );
    }

    const float userMinValue() const { return m_user_variable_min; }
    const float userMaxValue() const { return m_user_variable_max; }
    const float serverMinValue() const { return m_server_variable_min; }
    const float serverMaxValue() const { return m_server_variable_max; }

    const vismodule::UInt64* colorHistogram() const { return m_color_histogram; }
    const vismodule::UInt64* opacityHistogram() const { return m_opacity_histogram; }

    int32_t byteSize() const
    {
        int32_t size = 0;
        size += vismodule::Serializer::byteSize( m_resolution );
        size += vismodule::Serializer::byteSize( m_id );
        size += vismodule::Serializer::byteSize( m_label );
        size += vismodule::Serializer::byteSize( m_server_range_mode );
        size += vismodule::Serializer::byteSize( m_variable );
        size += vismodule::Serializer::byteSize( m_user_variable_min );
        size += vismodule::Serializer::byteSize( m_user_variable_max );
        size += vismodule::Serializer::byteSize( m_server_variable_min );
        size += vismodule::Serializer::byteSize( m_server_variable_max );
        size += sizeof( vismodule::UInt64 ) * DEFAULT_NBINS;
        size += sizeof( vismodule::UInt64 ) * DEFAULT_NBINS;
        size += vismodule::Serializer::byteSize<vismodule::TransferFunction>( *this );
        return size;
    }

    size_t pack( char* buf ) const
    {
        size_t index = 0;
        index += vismodule::Serializer::write( buf + index, m_resolution );
        index += vismodule::Serializer::write( buf + index, m_id );
        index += vismodule::Serializer::write( buf + index, m_label );
        index += vismodule::Serializer::write( buf + index, m_server_range_mode );
        index += vismodule::Serializer::write( buf + index, m_variable );
        index += vismodule::Serializer::write( buf + index, m_user_variable_min );
        index += vismodule::Serializer::write( buf + index, m_user_variable_max );
        index += vismodule::Serializer::write( buf + index, m_server_variable_min );
        index += vismodule::Serializer::write( buf + index, m_server_variable_max );
        index += vismodule::Serializer::writeArray( buf + index, m_color_histogram, DEFAULT_NBINS );
        index += vismodule::Serializer::writeArray( buf + index, m_opacity_histogram, DEFAULT_NBINS );
        index += vismodule::Serializer::pack<vismodule::TransferFunction>( buf + index, *this );
        return index;
    }

    size_t unpack( const char* buf )
    {
        size_t index = 0;
        index += vismodule::Serializer::read( buf + index, &m_resolution );
        index += vismodule::Serializer::read( buf + index, &m_id );
        index += vismodule::Serializer::read( buf + index, &m_label );
        index += vismodule::Serializer::read( buf + index, &m_server_range_mode );
        index += vismodule::Serializer::read( buf + index, &m_variable );
        index += vismodule::Serializer::read( buf + index, &m_user_variable_min );
        index += vismodule::Serializer::read( buf + index, &m_user_variable_max );
        index += vismodule::Serializer::read( buf + index, &m_server_variable_min );
        index += vismodule::Serializer::read( buf + index, &m_server_variable_max );
        index += vismodule::Serializer::readArray( buf + index, m_color_histogram, DEFAULT_NBINS );
        index += vismodule::Serializer::readArray( buf + index, m_opacity_histogram, DEFAULT_NBINS );
        index += vismodule::Serializer::unpack<vismodule::TransferFunction>( buf + index, this );
        return index;
    }
};

#endif // VIS_MODULE__ENSEMBLE_TRANSFER_FUNCTION_H_INCLUDE
