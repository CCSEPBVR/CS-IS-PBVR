#ifndef  __GLYPH_PROPERTY__
#define  __GLYPH_PROPERTY__

#include <string>
#include <vector>
#include <vismodule/ParticleProperty>

enum GlyphType
{
    Invalid = -1,
    Arrow   =  0, // Default
    Diamond =  1,
    Sphere  =  2,
};

enum class GlyphMode : int32_t
{
    Invalid             = -1,
    UniformDistribution =  0, // max sampepoints, seed
    AllPoints           =  1, // No UI
    EveryNthPoints      =  2  // Stride
};

enum class DataDefines : int32_t
{
    Invalid             = -1,
    Constant            =  0,
    VariableArray       =  1
};

struct GlyphProperty
{
    GlyphType m_glyph_type;
    bool m_glyph_flag;
    int m_stride;
    int m_seed;
    float m_scale_factor;
    float m_number_of_sampling_point;
    float m_glyph_color_min;
    float m_glyph_color_max;
    float m_glyph_size_min;
    float m_glyph_size_max;
    std::vector<int32_t> m_glyph_color_map_table;
    vismodule::ColorMap m_color_map; // pack unpackしない
    std::string m_direction_variable[3];
    DataDefines m_size_sampling_method;
    std::vector<std::string> m_size_variable;
    GlyphMode m_distribution_mode;
    DataDefines m_color_data_sampling_method;
    std::vector<std::string> m_color_data_variable;

    int32_t byteSize() const
    {
        int32_t size = 0;
        TaskSignal task_signal = TaskSignal::UPDATE_GLYPH_PROPERTY;

        size += vismodule::Serializer::byteSize( task_signal );
        size += vismodule::Serializer::byteSize( m_glyph_type );
        size += vismodule::Serializer::byteSize( m_glyph_flag );
        size += vismodule::Serializer::byteSize( m_seed );
        size += vismodule::Serializer::byteSize( m_scale_factor );
        size += vismodule::Serializer::byteSize( m_number_of_sampling_point );
        size += vismodule::Serializer::byteSize( m_glyph_color_min );
        size += vismodule::Serializer::byteSize( m_glyph_color_max );
        size += vismodule::Serializer::byteSize( m_glyph_size_min );
        size += vismodule::Serializer::byteSize( m_glyph_size_max );

        size += vismodule::Serializer::byteSize( m_glyph_color_map_table.size() );
        for ( std::size_t i = 0; i < m_glyph_color_map_table.size(); i++ )
        {
            size += vismodule::Serializer::byteSize( m_glyph_color_map_table[i] );
        }

        for ( std::size_t i = 0; i < 3; i++ )
        {
            size += vismodule::Serializer::byteSize( m_direction_variable[i] );
        }

        size += vismodule::Serializer::byteSize( m_size_sampling_method );

        size += vismodule::Serializer::byteSize( m_size_variable.size() );
        for ( std::size_t i = 0; i < m_size_variable.size(); i++ )
        {
            size += vismodule::Serializer::byteSize( m_size_variable[i] );
        }

        size += vismodule::Serializer::byteSize( m_distribution_mode );
        size += vismodule::Serializer::byteSize( m_color_data_sampling_method );

        size += vismodule::Serializer::byteSize( m_color_data_variable.size() );
        for ( std::size_t i = 0; i < m_color_data_variable.size(); i++ )
        {
            size += vismodule::Serializer::byteSize( m_color_data_variable[i] );
        }

        return size;
    }

    std::size_t pack( char* buf ) const
    {
        std::size_t index = 0;
        TaskSignal task_signal = TaskSignal::UPDATE_GLYPH_PROPERTY;

        index += vismodule::Serializer::write( buf + index, task_signal );
        index += vismodule::Serializer::write( buf + index, m_glyph_type );
        index += vismodule::Serializer::write( buf + index, m_glyph_flag );
        index += vismodule::Serializer::write( buf + index, m_seed );
        index += vismodule::Serializer::write( buf + index, m_scale_factor );
        index += vismodule::Serializer::write( buf + index, m_number_of_sampling_point );
        index += vismodule::Serializer::write( buf + index, m_glyph_color_min );
        index += vismodule::Serializer::write( buf + index, m_glyph_color_max );
        index += vismodule::Serializer::write( buf + index, m_glyph_size_min );
        index += vismodule::Serializer::write( buf + index, m_glyph_size_max );

        index += vismodule::Serializer::write( buf + index, m_glyph_color_map_table.size() );
        for ( std::size_t i = 0; i < m_glyph_color_map_table.size(); i++ )
        {
            index += vismodule::Serializer::write( buf + index, m_glyph_color_map_table[i] );
        }

        for ( std::size_t i = 0; i < 3; i++ )
        {
            index += vismodule::Serializer::write( buf + index, m_direction_variable[i] );
        }

        index += vismodule::Serializer::write( buf + index, m_size_sampling_method );

        index += vismodule::Serializer::write( buf + index, m_size_variable.size() );
        for ( std::size_t i = 0; i < m_size_variable.size(); i++ )
        {
            index += vismodule::Serializer::write( buf + index, m_size_variable[i] );
        }

        index += vismodule::Serializer::write( buf + index, m_distribution_mode );
        index += vismodule::Serializer::write( buf + index, m_color_data_sampling_method );

        index += vismodule::Serializer::write( buf + index, m_color_data_variable.size() );
        for ( std::size_t i = 0; i < m_color_data_variable.size(); i++ )
        {
            index += vismodule::Serializer::write( buf + index, m_color_data_variable[i] );
        }
        
        return index;
    }

    std::size_t unpack( const char* buf )
    {
        std::size_t index = 0;
        int64_t tmp_char_size;
        char* tmp_char = NULL;
        std::size_t size;
        TaskSignal task_signal;

        index += vismodule::Serializer::read( buf + index, &task_signal );
        index += vismodule::Serializer::read( buf + index, &m_glyph_type );
        index += vismodule::Serializer::read( buf + index, &m_glyph_flag );
        index += vismodule::Serializer::read( buf + index, &m_seed );
        index += vismodule::Serializer::read( buf + index, &m_scale_factor );
        index += vismodule::Serializer::read( buf + index, &m_number_of_sampling_point );
        index += vismodule::Serializer::read( buf + index, &m_glyph_color_min );
        index += vismodule::Serializer::read( buf + index, &m_glyph_color_max );
        index += vismodule::Serializer::read( buf + index, &m_glyph_size_min );
        index += vismodule::Serializer::read( buf + index, &m_glyph_size_max );

        m_glyph_color_map_table.clear();
        index += vismodule::Serializer::read( buf + index, &size );
        for ( std::size_t i = 0; i < size; i++ )
        {
            int32_t value = 0;
            index += vismodule::Serializer::read( buf + index, &value );
            m_glyph_color_map_table.push_back( value );
        }

        for ( std::size_t i = 0; i < 3; i++ )
        {
            index += vismodule::Serializer::read( buf + index, &m_direction_variable[i] );
        }

        index += vismodule::Serializer::read( buf + index, &m_size_sampling_method );

        index += vismodule::Serializer::read( buf + index, &size );
        for ( std::size_t i = 0; i < size; i++ )
        {
            std::string value = "";
            index += vismodule::Serializer::read( buf + index, &value );
            m_size_variable.push_back( value );
        }

        index += vismodule::Serializer::read( buf + index, &m_distribution_mode );
        index += vismodule::Serializer::read( buf + index, &m_color_data_sampling_method );

        m_color_data_variable.clear();
        index += vismodule::Serializer::read( buf + index, &size );
        for ( std::size_t i = 0; i < size; i++ )
        {
            std::string value = "";
            index += vismodule::Serializer::read( buf + index, &value );
            m_color_data_variable.push_back( value );
        }

        return index;
    }

    void show( int rank ) const
    {
        std::string glyph_type_string;
        std::string size_sampling_method_string;
        std::string distribution_mode_string;
        std::string color_data_sampling_method_string;

        if ( m_glyph_type == GlyphType::Arrow )
        {
            glyph_type_string = "Arrow";
        }
        else if ( m_glyph_type == GlyphType::Diamond )
        {
            glyph_type_string = "Diamond";
        }
        else if ( m_glyph_type == GlyphType::Sphere )
        {
            glyph_type_string = "Sphere";
        }
        else
        {
            std::cout << "ERROR: unknown glyph type" << std::endl;
            glyph_type_string = "UNKNOWN";
        }

        if ( m_size_sampling_method == DataDefines::Constant )
        {
            size_sampling_method_string = "Constant";
        }
        else if ( m_size_sampling_method == DataDefines::VariableArray )
        {
            size_sampling_method_string = "VariableArray";
        }
        else
        {
            std::cout << "ERROR: unknown size sampling method" << std::endl;
            size_sampling_method_string = "UNKNOWN";
        }

        if ( m_distribution_mode == GlyphMode::UniformDistribution )
        {
            distribution_mode_string = "UniformDistribution";
        }
        else if ( m_distribution_mode == GlyphMode::AllPoints )
        {
            distribution_mode_string = "AllPoints";
        }
        else if ( m_distribution_mode == GlyphMode::EveryNthPoints )
        {
            distribution_mode_string = "EveryNthPoints";
        }
        else
        {
            std::cout << "ERROR: unknown distribution mode" << std::endl;
            distribution_mode_string = "UNKNOWN";
        }

        if ( m_color_data_sampling_method == DataDefines::Constant )
        {
            color_data_sampling_method_string = "Constant";
        }
        else if ( m_color_data_sampling_method == DataDefines::VariableArray )
        {
            color_data_sampling_method_string = "VariableArray";
        }
        else
        {
            std::cout << "ERROR: unknown color data sampling method" << std::endl;
            color_data_sampling_method_string = "UNKNOWN";
        }

        std::cout << "===================== Glyph Property ( rank : " << rank << ") START ====================" << std::endl;
        std::cout << "glyph_type                 : " << glyph_type_string                 << std::endl;
        std::cout << "seed                       : " << m_seed                            << std::endl;
        std::cout << "scale_factor               : " << m_scale_factor                    << std::endl;
        std::cout << "number_of_sampling_point   : " << m_number_of_sampling_point        << std::endl;
        std::cout << "glyph_color_min            : " << m_glyph_color_min                 << std::endl;
        std::cout << "glyph_color_max            : " << m_glyph_color_max                 << std::endl;
        std::cout << "glyph_size_min             : " << m_glyph_size_min                  << std::endl;
        std::cout << "glyph_size_max             : " << m_glyph_size_max                  << std::endl;
        std::cout << "direction_variable[0]      : " << m_direction_variable[0]           << std::endl;
        std::cout << "direction_variable[1]      : " << m_direction_variable[1]           << std::endl;
        std::cout << "direction_variable[2]      : " << m_direction_variable[2]           << std::endl;
        std::cout << "size_sampling_method       : " << size_sampling_method_string       << std::endl;
        std::cout << "distribution_mode          : " << distribution_mode_string          << std::endl;
        std::cout << "color_data_sampling_method : " << color_data_sampling_method_string << std::endl;

        for ( std::size_t i = 0; i < m_size_variable.size(); i++ )
        {
            std::cout << "size_variable[" << i << "]           : " << m_size_variable[i] << std::endl;
        }

        for ( std::size_t i = 0; i < m_color_data_variable.size(); i++ )
        {
            std::cout << "color_data_variable[" << i << "]     : " << m_color_data_variable[i] << std::endl;
        }

        std::cout << "===================== Glyph Property ( rank : " << rank << ") END ====================" << std::endl;        
    }
};

inline GlyphType ConvertIntToGlyphType( int int_glyph_mode )
{
    switch ( int_glyph_mode )
    {
    case 0: return GlyphType::Arrow;
    case 1: return GlyphType::Diamond;
    case 2: return GlyphType::Sphere;
    default:
        std::cout << "ERROR:GlyphType is invalid." << std::endl;
        return GlyphType::Invalid;
    }
}

inline GlyphMode ConvertIntToGlyphMode( int glyph_mode_int )
{
    switch ( glyph_mode_int )
    {
    case 0: return GlyphMode::UniformDistribution;
    case 1: return GlyphMode::AllPoints;
    case 2: return GlyphMode::EveryNthPoints;
    default:
        std::cout << "ERROR:GlyphMode is invalid." << std::endl;
        return GlyphMode::Invalid;
    }
}

inline DataDefines ConvertIntToDataDefines( int data_defines_int )
{
    switch ( data_defines_int )
    {
    case 0: return DataDefines::Constant;
    case 1: return DataDefines::VariableArray;
    default:
        std::cout << "ERROR:DataDefines is invalid." << std::endl;
        return DataDefines::Invalid;
    }
}

#endif // __PARAM_INFO_H__

