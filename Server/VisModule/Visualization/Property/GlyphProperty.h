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
    SingleVariable      =  1,
    VariableArray       =  2
};

struct GlyphProperty
{
    ServerMode server_mode;
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
    vismodule::ColorMap m_color_map;
    std::string m_direction_variable[3];
    DataDefines m_size_sampling_method;
    std::vector<std::string> m_size_variable;
    GlyphMode m_distribution_mode;
    DataDefines m_color_data_sampling_method;
    std::vector<std::string> m_color_data_variable;
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
    case 1: return DataDefines::SingleVariable;
    case 2: return DataDefines::VariableArray;
    default:
        std::cout << "ERROR:DataDefines is invalid." << std::endl;
        return DataDefines::Invalid;
    }
}

#endif // __PARAM_INFO_H__

