#ifndef  __GLYPH_PROPERTY__
#define  __GLYPH_PROPERTY__

#include <string>
#include <vector>

// CSとISでglyphのsize minmaxの集計方法が異なる
// 定義する場所は変更予定
enum class ServerMode : int32_t
{
    CS = 0,
    IS = 1
};

enum class GlyphMode : int32_t
{
    UniformDistribution = 0, // max sampepoints, seed
    AllPoints           = 1, // No UI
    EveryNthPoints      = 2  // Stride
};

enum class DataDefines : int32_t
{
    Constant            = 0,
    SingleVariable      = 1,
    VariableArray       = 2
};

struct GlyphProperty
{
    ServerMode server_mode;
    bool m_glyph_flag;
    int m_stride;
    int m_seed;
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

#endif // __PARAM_INFO_H__

