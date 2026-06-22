#include "ParameterFileReader.h"
#include <exception>
#define DEFAULT_TF_NUMBER 5
#define BEFORE_READ_TF_NUMBER 99

namespace
{

std::string JsonPathForTransferFunction( const size_t index, const std::string& field )
{
    std::stringstream stream;
    stream << "transfer_functions[" << index << "]." << field;
    return stream.str();
}

const nlohmann::json& JsonRequiredObject( const nlohmann::json& parent, const std::string& key )
{
    if ( !parent.contains( key ) || !parent.at( key ).is_object() )
    {
        throw std::runtime_error( "Invalid default.json: missing object '" + key + "'" );
    }
    return parent.at( key );
}

const nlohmann::json& JsonRequiredArray( const nlohmann::json& parent, const std::string& key )
{
    if ( !parent.contains( key ) || !parent.at( key ).is_array() )
    {
        throw std::runtime_error( "Invalid default.json: missing array '" + key + "'" );
    }
    return parent.at( key );
}

bool JsonContainsAny( const nlohmann::json& parent, const std::string& key, const std::string& alternate_key )
{
    return parent.contains( key ) || parent.contains( alternate_key );
}

template <typename T>
T JsonValueWithAlternate(
    const nlohmann::json& parent,
    const std::string& key,
    const std::string& alternate_key,
    const T& fallback )
{
    if ( parent.contains( key ) ) return parent.at( key ).get<T>();
    if ( !alternate_key.empty() && parent.contains( alternate_key ) ) return parent.at( alternate_key ).get<T>();
    return fallback;
}

size_t JsonSizeValueWithAlternate(
    const nlohmann::json& parent,
    const std::string& key,
    const std::string& alternate_key,
    const size_t fallback )
{
    return JsonValueWithAlternate<size_t>( parent, key, alternate_key, fallback );
}

void ValidateMapSize(
    const nlohmann::json& map,
    const std::string& path,
    const size_t expected_value_count )
{
    const nlohmann::json& values = JsonRequiredArray( map, "values" );
    if ( map.contains( "length" ) )
    {
        const size_t length = map.at( "length" ).get<size_t>();
        if ( length != values.size() )
        {
            std::stringstream message;
            message << "Invalid default.json: " << path << ".length is " << length
                    << " but " << path << ".values.size() is " << values.size();
            throw std::runtime_error( message.str() );
        }
    }

    if ( values.size() != expected_value_count )
    {
        std::stringstream message;
        message << "Invalid default.json: " << path << ".values.size() is " << values.size()
                << " but expected " << expected_value_count;
        throw std::runtime_error( message.str() );
    }
}

void ValidateReadableTransferFunctionJson( const nlohmann::json& root )
{
    const nlohmann::json& settings = JsonRequiredObject( root, "settings" );
    const nlohmann::json& transfer_functions = JsonRequiredArray( root, "transfer_functions" );
    const nlohmann::json& transfer_settings = JsonRequiredObject( settings, "transfer_function" );

    if ( !JsonContainsAny( transfer_settings, "count", "transfer_function_count" ) )
    {
        throw std::runtime_error( "Invalid default.json: missing settings.transfer_function.count" );
    }
    if ( !JsonContainsAny( transfer_settings, "resolution", "transfer_function_resolution" ) )
    {
        throw std::runtime_error( "Invalid default.json: missing settings.transfer_function.resolution" );
    }

    const size_t tf_count = JsonSizeValueWithAlternate(
        transfer_settings, "count", "transfer_function_count", transfer_functions.size() );
    const size_t resolution = JsonSizeValueWithAlternate(
        transfer_settings, "resolution", "transfer_function_resolution", 0 );

    if ( tf_count != transfer_functions.size() )
    {
        std::stringstream message;
        message << "Invalid default.json: settings.transfer_function.count is " << tf_count
                << " but transfer_functions.size() is " << transfer_functions.size();
        throw std::runtime_error( message.str() );
    }

    for ( size_t i = 0; i < transfer_functions.size(); ++i )
    {
        const nlohmann::json& tf = transfer_functions.at( i );
        const nlohmann::json& color = JsonRequiredObject( tf, "color" );
        const nlohmann::json& opacity = JsonRequiredObject( tf, "opacity" );
        const nlohmann::json& color_map = JsonRequiredObject( color, "map" );
        const nlohmann::json& opacity_map = JsonRequiredObject( opacity, "map" );

        ValidateMapSize(
            color_map,
            JsonPathForTransferFunction( i, "color.map" ),
            resolution * 3 );
        ValidateMapSize(
            opacity_map,
            JsonPathForTransferFunction( i, "opacity.map" ),
            resolution );
    }
}

const nlohmann::json& GlyphRequiredObject( const nlohmann::json& parent, const std::string& key )
{
    if ( !parent.contains( key ) || !parent.at( key ).is_object() )
    {
        throw std::runtime_error( "Invalid glyph parameter json: missing object '" + key + "'" );
    }
    return parent.at( key );
}

const nlohmann::json& GlyphRequiredArray( const nlohmann::json& parent, const std::string& key )
{
    if ( !parent.contains( key ) || !parent.at( key ).is_array() )
    {
        throw std::runtime_error( "Invalid glyph parameter json: missing array '" + key + "'" );
    }
    return parent.at( key );
}

std::vector<std::string> GlyphStringArray( const nlohmann::json& parent, const std::string& key )
{
    std::vector<std::string> values;
    const nlohmann::json& array = GlyphRequiredArray( parent, key );
    values.reserve( array.size() );
    for ( size_t i = 0; i < array.size(); ++i )
    {
        values.push_back( array.at( i ).get<std::string>() );
    }
    return values;
}

std::vector<int32_t> GlyphInt32Array( const nlohmann::json& parent, const std::string& key )
{
    std::vector<int32_t> values;
    const nlohmann::json& array = GlyphRequiredArray( parent, key );
    values.reserve( array.size() );
    for ( size_t i = 0; i < array.size(); ++i )
    {
        values.push_back( array.at( i ).get<int32_t>() );
    }
    return values;
}

GlyphType GlyphTypeFromString( const std::string& name )
{
    if ( name == "Arrow" ) return GlyphType::Arrow;
    if ( name == "Diamond" ) return GlyphType::Diamond;
    if ( name == "Sphere" ) return GlyphType::Sphere;
    throw std::runtime_error( "Invalid glyph parameter json: unknown type '" + name + "'" );
}

GlyphMode GlyphModeFromString( const std::string& name )
{
    if ( name == "UniformDistribution" ) return GlyphMode::UniformDistribution;
    if ( name == "AllPoints" ) return GlyphMode::AllPoints;
    if ( name == "EveryNthPoints" ) return GlyphMode::EveryNthPoints;
    throw std::runtime_error( "Invalid glyph parameter json: unknown distribution_mode '" + name + "'" );
}

DataDefines DataDefinesFromString( const std::string& name, const std::string& key )
{
    if ( name == "Constant" ) return DataDefines::Constant;
    if ( name == "VariableArray" ) return DataDefines::VariableArray;
    throw std::runtime_error( "Invalid glyph parameter json: unknown " + key + " '" + name + "'" );
}


void ApplyGlyphDefault( GlyphProperty& glyph_property )
{
    std::vector<std::string> size_variables = { "q1" };
    std::vector<std::string> color_data_variables = { "q1" };

    glyph_property.m_glyph_flag                 = false;
    glyph_property.m_glyph_type                 = GlyphType::Arrow;
    glyph_property.m_scale_factor               = 1;
    glyph_property.m_direction_variable[0]      = "q1";
    glyph_property.m_direction_variable[1]      = "q2";
    glyph_property.m_direction_variable[2]      = "q3";
    glyph_property.m_size_sampling_method       = DataDefines::Constant;
    glyph_property.m_size_variable              = size_variables;
    glyph_property.m_distribution_mode          = GlyphMode::UniformDistribution;
    glyph_property.m_number_of_sampling_point   = 1000;
    glyph_property.m_seed                       = 1;
    glyph_property.m_stride                     = 3;
    glyph_property.m_color_data_sampling_method = DataDefines::Constant;
    glyph_property.m_color_data_variable        = color_data_variables;
    glyph_property.m_glyph_color_min            = 0;
    glyph_property.m_glyph_color_max            = 1;
    glyph_property.m_glyph_size_min             = 0;
    glyph_property.m_glyph_size_max             = 1;
    glyph_property.m_glyph_color_map_table.clear();
}

void ApplyGlyphParameterJson( const nlohmann::json& root, GlyphProperty& glyph_property )
{
    glyph_property.m_glyph_flag = root.at( "enabled" ).get<bool>();
    if ( !glyph_property.m_glyph_flag )
    {
        glyph_property.m_glyph_type = GlyphType::Arrow;
        glyph_property.m_stride = 1;
        glyph_property.m_seed = 0;
        glyph_property.m_scale_factor = 1.0f;
        glyph_property.m_number_of_sampling_point = 0.0f;
        glyph_property.m_glyph_color_min = 0.0f;
        glyph_property.m_glyph_color_max = 1.0f;
        glyph_property.m_glyph_size_min = 0.0f;
        glyph_property.m_glyph_size_max = 1.0f;
        glyph_property.m_distribution_mode = GlyphMode::AllPoints;
        glyph_property.m_size_sampling_method = DataDefines::Constant;
        glyph_property.m_color_data_sampling_method = DataDefines::Constant;
        glyph_property.m_glyph_color_map_table.clear();
        glyph_property.m_size_variable.clear();
        glyph_property.m_color_data_variable.clear();
        return;
    }

    const nlohmann::json& size = GlyphRequiredObject( root, "size" );
    const nlohmann::json& color_data = GlyphRequiredObject( root, "color_data" );
    const nlohmann::json& color_range = GlyphRequiredObject( color_data, "range" );

    glyph_property.m_glyph_type = GlyphTypeFromString( root.at( "type" ).get<std::string>() );
    glyph_property.m_scale_factor = root.at( "scale_factor" ).get<float>();
    glyph_property.m_distribution_mode = GlyphModeFromString( root.at( "distribution_mode" ).get<std::string>() );
    glyph_property.m_stride = glyph_property.m_distribution_mode == GlyphMode::AllPoints
        ? 1
        : root.at( "stride" ).get<int>();
    glyph_property.m_seed = root.at( "seed" ).get<int>();
    glyph_property.m_number_of_sampling_point = root.at( "number_of_sampling_points" ).get<float>();

    const std::vector<std::string> direction_variables = GlyphStringArray( root, "direction_variables" );
    if ( direction_variables.size() < 3 )
    {
        throw std::runtime_error( "Invalid glyph parameter json: direction_variables must contain at least 3 values" );
    }
    for ( std::size_t i = 0; i < 3; i++ )
    {
        glyph_property.m_direction_variable[i] = direction_variables[i];
    }

    glyph_property.m_size_sampling_method = DataDefinesFromString(
        size.at( "sampling_method" ).get<std::string>(), "size.sampling_method" );
    glyph_property.m_size_variable = GlyphStringArray( size, "variables" );

    glyph_property.m_color_data_sampling_method = DataDefinesFromString(
        color_data.at( "sampling_method" ).get<std::string>(), "color_data.sampling_method" );
    glyph_property.m_color_data_variable = GlyphStringArray( color_data, "variables" );
    glyph_property.m_glyph_color_min = color_range.at( "min" ).get<float>();
    glyph_property.m_glyph_color_max = color_range.at( "max" ).get<float>();
    glyph_property.m_glyph_color_map_table = GlyphInt32Array( root, "color_map" );
    glyph_property.updateColorMap();
}

const nlohmann::json& ParameterRequiredArray(
    const nlohmann::json& parent,
    const std::string& key,
    const std::string& parameter_name )
{
    if ( !parent.contains( key ) || !parent.at( key ).is_array() )
    {
        throw std::runtime_error(
            "Invalid " + parameter_name + " parameter json: missing array '" + key + "'" );
    }
    return parent.at( key );
}

void ApplyPlotOverLineDefault( PlotOverLineProperty& pol_property )
{
    pol_property.m_plot_flag = false;
    pol_property.m_plot_variable = "q1";
    pol_property.m_sampling_size = 256;
    for ( std::size_t i = 0; i < 3; i++ )
    {
        pol_property.m_start_point[i] = 0.0f;
        pol_property.m_end_point[i] = 1.0f;
    }
}

void ApplyPlotOverTimeDefault( PlotOverTimeProperty& pot_property )
{
    pot_property.m_plot_flag = true;
    for ( std::size_t i = 0; i < 3; i++ )
    {
        pot_property.m_target_point[i] = 0.0f;
    }
}

void ReadFloat3(
    const nlohmann::json& parent,
    const std::string& key,
    float values[3],
    const std::string& parameter_name )
{
    const nlohmann::json& array = ParameterRequiredArray( parent, key, parameter_name );
    if ( array.size() < 3 )
    {
        throw std::runtime_error(
            "Invalid " + parameter_name + " parameter json: '" + key + "' must contain at least 3 values" );
    }

    for ( std::size_t i = 0; i < 3; i++ )
    {
        values[i] = array.at( i ).get<float>();
    }
}

void ApplyPlotOverLineParameterJson( const nlohmann::json& root, PlotOverLineProperty& pol_property )
{
    pol_property.m_plot_flag = root.at( "enabled" ).get<bool>();
    pol_property.m_plot_variable = root.at( "variable" ).get<std::string>();
    pol_property.m_sampling_size = root.at( "sampling_size" ).get<int32_t>();
    ReadFloat3( root, "start_point", pol_property.m_start_point, "plot over line" );
    ReadFloat3( root, "end_point", pol_property.m_end_point, "plot over line" );
}

void ApplyPlotOverTimeParameterJson( const nlohmann::json& root, PlotOverTimeProperty& pot_property )
{
    pot_property.m_plot_flag = root.at( "enabled" ).get<bool>();
    ReadFloat3( root, "target_point", pot_property.m_target_point, "plot over time" );
}

std::vector<int> JsonIntTable( const nlohmann::json& params, const std::string& key )
{
    std::vector<int> values;
    const nlohmann::json& value = params.at( key );

    if ( value.is_array() )
    {
        for ( size_t i = 0; i < value.size(); ++i )
        {
            values.push_back( value.at( i ).get<int>() );
        }
    }
    else
    {
        std::string table_string = value.get<std::string>();
        std::replace( table_string.begin(), table_string.end(), ',', ' ' );
        std::stringstream stream( table_string );
        int table_value = 0;
        while ( stream >> table_value )
        {
            values.push_back( table_value );
        }
    }

    return values;
}

std::vector<float> JsonFloatTable( const nlohmann::json& params, const std::string& key )
{
    std::vector<float> values;
    const nlohmann::json& value = params.at( key );

    if ( value.is_array() )
    {
        for ( size_t i = 0; i < value.size(); ++i )
        {
            values.push_back( value.at( i ).get<float>() );
        }
    }
    else
    {
        std::string table_string = value.get<std::string>();
        std::replace( table_string.begin(), table_string.end(), ',', ' ' );
        std::stringstream stream( table_string );
        float table_value = 0.0f;
        while ( stream >> table_value )
        {
            values.push_back( table_value );
        }
    }

    return values;
}

float JsonRangeValue(
    const nlohmann::json& range,
    const std::string& range_name,
    const std::string& bound_name,
    const float fallback = 0.0f )
{
    if ( !range.contains( range_name ) ) return fallback;
    const nlohmann::json& value = range.at( range_name );
    if ( !value.is_object() || !value.contains( bound_name ) ) return fallback;
    return value.at( bound_name ).get<float>();
}

std::string JsonRangeMode( const nlohmann::json& range )
{
    if ( range.contains( "mode" ) ) return range.at( "mode" ).get<std::string>();
    const std::string active_range = range.value( "active_range", std::string( "user" ) );
    if ( active_range == "server" ) return "ServerSide";
    if ( active_range == "user" ) return "UserRange";
    return "";
}

nlohmann::json BuildLegacyParametersFromReadableJson( const nlohmann::json& root )
{
    nlohmann::json params = root.value( "parameters", nlohmann::json::object() );
    const nlohmann::json& settings = root.at( "settings" );
    const nlohmann::json& transfer_functions = root.at( "transfer_functions" );

    const nlohmann::json& sampling = settings.value( "sampling", nlohmann::json::object() );
    const nlohmann::json& image = settings.value( "image", nlohmann::json::object() );
    const nlohmann::json& transfer_settings = settings.value( "transfer_function", nlohmann::json::object() );

    if ( sampling.contains( "method" ) ) params["SAMPLING_METHOD"] = sampling.at( "method" );
    if ( sampling.contains( "particle_limit" ) ) params["PARTICLE_LIMIT"] = sampling.at( "particle_limit" );
    if ( sampling.contains( "particle_data_size_limit" ) ) params["PARTICLE_DATA_SIZE_LIMIT"] = sampling.at( "particle_data_size_limit" );
    if ( sampling.contains( "particle_data_size_limit_unit" ) ) params["PARTICLE_DATA_SIZE_LIMIT_UNIT"] = sampling.at( "particle_data_size_limit_unit" );
    if ( image.contains( "width" ) ) params["RESOLUTION_WIDTH"] = image.at( "width" );
    if ( image.contains( "height" ) ) params["RESOLUTION_HEIGHT"] = image.at( "height" );
    if ( transfer_settings.contains( "resolution" ) ) params["TF_RESOLUTION"] = transfer_settings.at( "resolution" );
    else if ( transfer_settings.contains( "transfer_function_resolution" ) ) params["TF_RESOLUTION"] = transfer_settings.at( "transfer_function_resolution" );
    if ( transfer_settings.contains( "count" ) ) params["TF_NUMBER"] = transfer_settings.at( "count" );
    else if ( transfer_settings.contains( "transfer_function_count" ) ) params["TF_NUMBER"] = transfer_settings.at( "transfer_function_count" );
    else params["TF_NUMBER"] = transfer_functions.size();
    if ( transfer_settings.contains( "color_synthesis" ) ) params["COLOR_SYNTH"] = transfer_settings.at( "color_synthesis" );
    if ( transfer_settings.contains( "opacity_synthesis" ) ) params["OPACITY_SYNTH"] = transfer_settings.at( "opacity_synthesis" );

    for ( size_t i = 0; i < transfer_functions.size(); ++i )
    {
        const nlohmann::json& tf = transfer_functions.at( i );
        std::stringstream ss;
        ss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = ss.str();

        if ( tf.contains( "id" ) ) params[tag_base + "ID"] = tf.at( "id" );
        if ( tf.contains( "label" ) ) params[tag_base + "LABEL"] = tf.at( "label" );

        if ( tf.contains( "color" ) )
        {
            const nlohmann::json& color = tf.at( "color" );
            if ( color.contains( "variable" ) ) params[tag_base + "VAR_C"] = color.at( "variable" );
            if ( color.contains( "range" ) )
            {
                const nlohmann::json& range = color.at( "range" );
                params[tag_base + "RANGE_MODE_C"] = JsonRangeMode( range );
                params[tag_base + "SERVER_MIN_C"] = JsonRangeValue( range, "server", "min" );
                params[tag_base + "SERVER_MAX_C"] = JsonRangeValue( range, "server", "max" );
                params[tag_base + "USER_MIN_C"] = JsonRangeValue( range, "user", "min" );
                params[tag_base + "USER_MAX_C"] = JsonRangeValue( range, "user", "max" );
            }
            if ( color.contains( "map" ) )
            {
                const nlohmann::json& map = color.at( "map" );
                if ( map.contains( "values" ) ) params[tag_base + "TABLE_C"] = map.at( "values" );
            }
        }

        if ( tf.contains( "opacity" ) )
        {
            const nlohmann::json& opacity = tf.at( "opacity" );
            if ( opacity.contains( "variable" ) ) params[tag_base + "VAR_O"] = opacity.at( "variable" );
            if ( opacity.contains( "range" ) )
            {
                const nlohmann::json& range = opacity.at( "range" );
                params[tag_base + "RANGE_MODE_O"] = JsonRangeMode( range );
                params[tag_base + "SERVER_MIN_O"] = JsonRangeValue( range, "server", "min" );
                params[tag_base + "SERVER_MAX_O"] = JsonRangeValue( range, "server", "max" );
                params[tag_base + "USER_MIN_O"] = JsonRangeValue( range, "user", "min" );
                params[tag_base + "USER_MAX_O"] = JsonRangeValue( range, "user", "max" );
            }
            if ( opacity.contains( "map" ) )
            {
                const nlohmann::json& map = opacity.at( "map" );
                if ( map.contains( "values" ) ) params[tag_base + "TABLE_O"] = map.at( "values" );
            }
        }
    }

    if ( !params.contains( "END_PARAMETER_FILE" ) ) params["END_PARAMETER_FILE"] = "SUCCESS";
    return params;
}

nlohmann::json TransferFunctionParameters( const nlohmann::json& root )
{
    if ( root.contains( "settings" ) && root.contains( "transfer_functions" ) )
    {
        ValidateReadableTransferFunctionJson( root );
        return BuildLegacyParametersFromReadableJson( root );
    }

    return root.at( "parameters" );
}
}

// CSのConnect時に指定した.tfファイルを読み込む
void ParameterFileReader::readTransferFunctionFile( const char* fname )
{
    m_name_list_file.setName( "TF_RESOLUTION" );
    m_name_list_file.setName( "TF_NUMBER" );
    m_name_list_file.setName( "TF_SYNTH_C" );
    m_name_list_file.setName( "TF_SYNTH_O" );

    for ( std::size_t i = 0; i < BEFORE_READ_TF_NUMBER; i++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << i + 1 << "_";

        const std::string tag_base = ss.str();
        m_name_list_file.setName( tag_base + "C" );
        m_name_list_file.setName( tag_base + "VAR_C" );
        m_name_list_file.setName( tag_base + "MIN_C" );
        m_name_list_file.setName( tag_base + "MAX_C" );
        m_name_list_file.setName( tag_base + "TABLE_C" );
        m_name_list_file.setName( tag_base + "O" );
        m_name_list_file.setName( tag_base + "VAR_O" );
        m_name_list_file.setName( tag_base + "MIN_O" );
        m_name_list_file.setName( tag_base + "MAX_O" );
        m_name_list_file.setName( tag_base + "TABLE_O" );
    }

    m_name_list_file.setFileName( std::string( fname ) );

    if( !m_name_list_file.read() )
    {
        std::cerr << "ERROR:Failed to read the transfer function file." << std::endl;
        return;
    }

    // delete TF_NUMBER+1 ~ BEFORE_READ_TF_NUMBER
    const int tf_number = m_name_list_file.getValue<int>( "TF_NUMBER" );
    if ( tf_number < BEFORE_READ_TF_NUMBER )
    {
        for ( std::size_t i = tf_number; i < BEFORE_READ_TF_NUMBER; i++ )
        {
            std::stringstream ss;
            ss << "TF_NAME" << i + 1 << "_";

            const std::string tag_base = ss.str();
            m_name_list_file.setName( tag_base + "C" );
            m_name_list_file.setName( tag_base + "VAR_C" );
            m_name_list_file.setName( tag_base + "MIN_C" );
            m_name_list_file.setName( tag_base + "MAX_C" );
            m_name_list_file.setName( tag_base + "TABLE_C" );
            m_name_list_file.setName( tag_base + "O" );
            m_name_list_file.setName( tag_base + "VAR_O" );
            m_name_list_file.setName( tag_base + "MIN_O" );
            m_name_list_file.setName( tag_base + "MAX_O" );
            m_name_list_file.setName( tag_base + "TABLE_O" );
        }
    }
}

// ISの旧形式default.tfファイルを読み込む
void ParameterFileReader::readParticleParameterFile( const char* fname )
{
    m_name_list_file.setName( "SAMPLING_METHOD" );
    m_name_list_file.setName( "PARTICLE_LIMIT" );
    // m_name_list_file.setName( "EXTRA_OPACITY_FACTOR" ); // 一時的にコメントアウト
    m_name_list_file.setName( "PARTICLE_DATA_SIZE_LIMIT" );
    m_name_list_file.setName( "RESOLUTION_WIDTH" );
    m_name_list_file.setName( "RESOLUTION_HEIGHT" );
    m_name_list_file.setName( "TF_RESOLUTION" );
    m_name_list_file.setName( "COLOR_SYNTH" );
    m_name_list_file.setName( "OPACITY_SYNTH" );
    m_name_list_file.setName( "TF_NUMBER" );

    for ( std::size_t n = 0; n < BEFORE_READ_TF_NUMBER; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";

        const std::string tag_base = ss.str();
        m_name_list_file.setName( tag_base + "VAR_C" );
        m_name_list_file.setName( tag_base + "RANGE_MODE_C" );
        m_name_list_file.setName( tag_base + "SERVER_MIN_C" );
        m_name_list_file.setName( tag_base + "SERVER_MAX_C" );
        m_name_list_file.setName( tag_base + "USER_MIN_C" );
        m_name_list_file.setName( tag_base + "USER_MAX_C" );
        m_name_list_file.setName( tag_base + "VAR_O" );
        m_name_list_file.setName( tag_base + "RANGE_MODE_O" );
        m_name_list_file.setName( tag_base + "SERVER_MIN_O" );
        m_name_list_file.setName( tag_base + "SERVER_MAX_O" );
        m_name_list_file.setName( tag_base + "USER_MIN_O" );
        m_name_list_file.setName( tag_base + "USER_MAX_O" );
        m_name_list_file.setName( tag_base + "TABLE_C" );
        m_name_list_file.setName( tag_base + "TABLE_O" );
    }

    m_name_list_file.setName( "END_PARAMETER_FILE" );

    m_name_list_file.setFileName( std::string( fname ) );

    bool is_read_finished = false;

    while( !is_read_finished )
    {
        if( !m_name_list_file.read() )
        {
            this->set_default_parameter();
            break;
        }

        std::string result = m_name_list_file.getValue<std::string>( "END_PARAMETER_FILE" );

        if( result == "SUCCESS" )
        {
            is_read_finished = true;
        }
    }

    // delete TF_NUMBER+1 ~ BEFORE_READ_TF_NUMBER
    int cur_tf_number = m_name_list_file.getValue<int>("TF_NUMBER");
    if (cur_tf_number < BEFORE_READ_TF_NUMBER) {
        for (size_t n = cur_tf_number; n < BEFORE_READ_TF_NUMBER; n++)
        {
            std::stringstream ss;
            ss << "TF_NAME" << n + 1 << "_";

            const std::string tag_base = ss.str();
            m_name_list_file.deleteLine( tag_base + "VAR_C" );
            m_name_list_file.deleteLine( tag_base + "RANGE_MODE_C" );
            m_name_list_file.deleteLine( tag_base + "SERVER_MIN_C" );
            m_name_list_file.deleteLine( tag_base + "SERVER_MAX_C" );
            m_name_list_file.deleteLine( tag_base + "USER_MIN_C" );
            m_name_list_file.deleteLine( tag_base + "USER_MAX_C" );
            m_name_list_file.deleteLine( tag_base + "VAR_O" );
            m_name_list_file.deleteLine( tag_base + "RANGE_MODE_O" );
            m_name_list_file.deleteLine( tag_base + "SERVER_MIN_O" );
            m_name_list_file.deleteLine( tag_base + "SERVER_MAX_O" );
            m_name_list_file.deleteLine( tag_base + "USER_MIN_O" );
            m_name_list_file.deleteLine( tag_base + "USER_MAX_O" );
            m_name_list_file.deleteLine( tag_base + "TABLE_C" );
            m_name_list_file.deleteLine( tag_base + "TABLE_O" );
        }
    }

    return;
}

// ISのdefault.json/default_old.jsonファイルを読み込む
bool ParameterFileReader::readTransferFunctionFromJson( const char* fname, ParticleProperty& particle_property )
{
    try
    {
        std::cout << "------------------------------------Import json ------------------------------------------" << std::endl;
       nlohmann::json tf = TransferFunctionJsonWriter::LoadTfJson( fname );

       const nlohmann::json params = TransferFunctionParameters( tf );

       const std::string size_sampling_method                  = params.value( "SAMPLING_METHOD", std::string( "" ) );
       particle_property.m_particle_limit                      = params.value( "PARTICLE_LIMIT", 0 );
       // particle_property.m_extra_opacity_factor                = m_name_list_file.getValue<float>( "EXTRA_OPACITY_FACTOR" ); // 一時的にコメントアウト
       particle_property.m_extra_opacity_factor                = 1; // 一時的にハードコーティング
       particle_property.m_particle_data_size_limit            = params.value( "PARTICLE_DATA_SIZE_LIMIT", 0.0f );
       const std::string particle_data_size_limit_unit          = params.value( "PARTICLE_DATA_SIZE_LIMIT_UNIT", std::string( "MB" ) );
       particle_property.m_color_transfer_function_synthesis   = params.value( "COLOR_SYNTH", std::string( "" ) );
       particle_property.m_opacity_transfer_function_synthesis = params.value( "OPACITY_SYNTH", std::string( "" ) );
       if ( particle_data_size_limit_unit != "MB" )
       {
           std::cout << "WARN:particle_data_size_limit_unit is '" << particle_data_size_limit_unit
                     << "', but ParticleProperty stores this value as MB." << std::endl;
       }
       if ( size_sampling_method == "Uniform" )
       {
           particle_property.m_sampling_method = 'u';
       }
       else if ( size_sampling_method == "Metropolis" )
       {
           particle_property.m_sampling_method = 'm';
       }
       else if ( size_sampling_method == "Rejection" )
       {
           particle_property.m_sampling_method = 'r';
       }
       else
       {
           std::cout << "ERROR:particle sampling method is not selected." << std::endl;
           return false;
       }

       const size_t width               = params.value( "RESOLUTION_WIDTH", 0 );
       const size_t height              = params.value( "RESOLUTION_HEIGHT", 0 );
       particle_property.m_camera->setWindowSize( width, height );

       const size_t resolution          = params.value( "TF_RESOLUTION", 0 );
       const int tf_number              = params.value( "TF_NUMBER", 0 );
       if ( resolution == 0 || tf_number <= 0 )
       {
           std::cout << "ERROR:Transfer function resolution or count is invalid." << std::endl;
           return false;
       }

       particle_property.m_transfunc_array.clear();
       particle_property.m_transfunc_array.resize( tf_number );
       // particle_property.m_voleqn.clear();
       // particle_property.m_voleqn.resize( tf_number );

       for ( size_t n = 0; n < tf_number; n++ )
       {
           std::stringstream ss;
           ss << "TF_NAME" << n + 1 << "_";

           std::stringstream s_name;
           std::stringstream f_name;
        s_name << "t" << n + 1;
        f_name << "_F" << n + 1 << "_VAR_";

        const std::string tag_base = ss.str();
        particle_property.m_transfunc_array[n].m_resolution = resolution;
        particle_property.m_transfunc_array[n].m_name = s_name.str();

        const std::string color_variable         = params.value( tag_base + "VAR_C", std::string( "" ) );                                                              
        const std::string opacity_varible        = params.value( tag_base + "VAR_O", std::string( "" ) );
        const std::string color_range_mode       = params.value( tag_base + "RANGE_MODE_C", std::string( "" ) );
        const std::string opacity_range_mode     = params.value( tag_base + "RANGE_MODE_O", std::string( "" ) );
        const float server_color_min             = params.value( tag_base + "SERVER_MIN_C", 0.0f );
        const float server_color_max             = params.value( tag_base + "SERVER_MAX_C", 0.0f );
        const float user_color_min               = params.value( tag_base + "USER_MIN_C", 0.0f );
        const float user_color_max               = params.value( tag_base + "USER_MAX_C", 0.0f );
        const float server_opacity_min           = params.value( tag_base + "SERVER_MIN_O", 0.0f );
        const float server_opacity_max           = params.value( tag_base + "SERVER_MAX_O", 0.0f );
        const float user_opacity_min             = params.value( tag_base + "USER_MIN_O", 0.0f );
        const float user_opacity_max             = params.value( tag_base + "USER_MAX_O", 0.0f );
        const std::vector<int> color_values      = JsonIntTable( params, tag_base + "TABLE_C" );
        const std::vector<float> opacity_values  = JsonFloatTable( params, tag_base + "TABLE_O" );
        const int tf_id                          = params.value( tag_base + "ID", static_cast<int>( n + 1 ) );
        const std::string tf_label               = params.value( tag_base + "LABEL", std::string( "" ) );

        if ( color_values.size() != resolution * 3 )
        {
            std::cout << "ERROR:" << tag_base << "TABLE_C size is " << color_values.size()
                      << ", but expected " << resolution * 3 << "." << std::endl;
            return false;
        }
        if ( opacity_values.size() != resolution )
        {
            std::cout << "ERROR:" << tag_base << "TABLE_O size is " << opacity_values.size()
                      << ", but expected " << resolution << "." << std::endl;
            return false;
        }

        std::cout << "  transfer_functions[" << n << "] id=" << tf_id;
        if ( !tf_label.empty() ) std::cout << " label=" << tf_label;
        std::cout << " color.variable=" << color_variable
                  << " opacity.variable=" << opacity_varible
                  << " color.range.mode=" << color_range_mode
                  << " opacity.range.mode=" << opacity_range_mode
                  << " color.map.values=" << color_values.size()
                  << " opacity.map.values=" << opacity_values.size()
                  << std::endl;

        if ( color_range_mode == "ServerSide" )
        {
            particle_property.m_transfunc_array[n].m_server_color_range_mode = NamedTransferFunction::ServerRangeMode::ServerSide;
        }
        else if ( color_range_mode == "UserRange" )
        {
            particle_property.m_transfunc_array[n].m_server_color_range_mode = NamedTransferFunction::ServerRangeMode::UserRange;
        }
        else
        {
            std::cout << "ERROR:Color Range Mode is unknown" << std::endl;
        }

        if ( opacity_range_mode == "ServerSide" )
        {
            particle_property.m_transfunc_array[n].m_server_opacity_range_mode = NamedTransferFunction::ServerRangeMode::ServerSide;
        }
        else if ( opacity_range_mode == "UserRange" )
        {
            particle_property.m_transfunc_array[n].m_server_opacity_range_mode = NamedTransferFunction::ServerRangeMode::UserRange;
        }
        else
        {
            std::cout << "ERROR:Opacity Range Mode is unknown" << std::endl;
        }

        particle_property.m_transfunc_array[n].m_color_variable              = color_variable;
        particle_property.m_transfunc_array[n].m_opacity_variable            = opacity_varible;
        particle_property.m_transfunc_array[n].m_server_color_variable_min   = server_color_min;
        particle_property.m_transfunc_array[n].m_server_color_variable_max   = server_color_max;
        particle_property.m_transfunc_array[n].m_server_opacity_variable_min = server_opacity_min;
        particle_property.m_transfunc_array[n].m_server_opacity_variable_max = server_opacity_max;
        particle_property.m_transfunc_array[n].m_user_color_variable_min     = user_color_min;
        particle_property.m_transfunc_array[n].m_user_color_variable_max     = user_color_max;
        particle_property.m_transfunc_array[n].m_user_opacity_variable_min   = user_opacity_min;
        particle_property.m_transfunc_array[n].m_user_opacity_variable_max   = user_opacity_max;

        vismodule::ColorMap::Table color_table( resolution * 3 );
        vismodule::OpacityMap::Table opacity_table( resolution );

        for ( size_t i = 0; i < resolution; i++ )
        {
            for ( size_t c = 0; c < 3; c++ )
            {
                color_table.at( i * 3 + c ) = color_values.at( i * 3 + c );
            }
        }

        for ( size_t i = 0; i < resolution; i++ )
        {
            opacity_table.at( i ) = opacity_values.at( i );
        }

        vismodule::ColorMap color_map( color_table );
        vismodule::OpacityMap opacity_map( opacity_table );

        particle_property.m_transfunc_array[n].setColorMap( color_map );
        particle_property.m_transfunc_array[n].setOpacityMap( opacity_map );

        if ( particle_property.m_transfunc_array[n].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[n].setColorRange( server_color_min, server_color_max );
        }
        else if ( particle_property.m_transfunc_array[n].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::UserRange )
        {
            particle_property.m_transfunc_array[n].setColorRange( user_color_min, user_color_max );
        }
        else
        {
            std::cout << "ERROR:Color Range Mode is unknown" << std::endl;
        }

        if ( particle_property.m_transfunc_array[n].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[n].setOpacityRange( server_opacity_min, server_opacity_max );
        }
        else if ( particle_property.m_transfunc_array[n].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::UserRange )
        {
            particle_property.m_transfunc_array[n].setOpacityRange( user_opacity_min, user_opacity_max );
        }
        else
        {
            std::cout << "ERROR:Opacity Range Mode is unknown" << std::endl;
        }
       }
        std::string equation;
        EquationToken eq;

        equation = params.value( "COLOR_SYNTH", std::string( "" ) );
        std::replace( equation.begin(), equation.end(), 'C', 'c' );
        eq = particle_property.m_transfunc_synthesizer->convert_token( equation );
        particle_property.m_transfunc_synthesizer->setColorFunction( eq );

        equation = params.value( "OPACITY_SYNTH", std::string( "" ) );
        std::replace( equation.begin(), equation.end(), 'O', 'a' );
        eq = particle_property.m_transfunc_synthesizer->convert_token( equation );
        particle_property.m_transfunc_synthesizer->setOpacityFunction( eq );

        std::vector<EquationToken> var;

        for ( size_t i = 0; i < tf_number; i++ )
        {
            std::stringstream tss;
            tss << "TF_NAME" << i + 1 << "_";
            const std::string tag_base = tss.str();

            equation = params.value( tag_base + "VAR_C", std::string( "" ) );
            std::replace( equation.begin(), equation.end(), 'X', 'x' );
            std::replace( equation.begin(), equation.end(), 'Y', 'y' );
            std::replace( equation.begin(), equation.end(), 'Z', 'z' );
            eq = particle_property.m_transfunc_synthesizer->convert_token( equation );

            var.push_back( eq );
        }

        particle_property.m_transfunc_synthesizer->setColorVariable( var );
        var.clear();

        for ( size_t i = 0; i < tf_number; i++ )
        {
            std::stringstream tss;
            tss << "TF_NAME" << i + 1 << "_";
            const std::string tag_base = tss.str();

            equation = params.value( tag_base + "VAR_O", std::string( "" ) );
            std::cout << "equation =  " << equation  << std::endl;
            std::replace( equation.begin(), equation.end(), 'X', 'x' );
            std::replace( equation.begin(), equation.end(), 'Y', 'y' );
            std::replace( equation.begin(), equation.end(), 'Z', 'z' );
            eq = particle_property.m_transfunc_synthesizer->convert_token( equation );

            var.push_back( eq );
        }
        particle_property.m_transfunc_synthesizer->setOpacityVariable( var );
        var.clear();

        return true;
    }
    catch ( const std::exception& e )
    {
        std::cerr << "ERROR:Failed to read transfer function json: "
                  << fname << std::endl;
        std::cerr << "ERROR:" << e.what() << std::endl;
        return false;
    }
}

void ParameterFileReader::readGlyphParameterFile( const char* fname, GlyphProperty& glyph_property )
{
    try
    {
        std::ifstream file( fname );
        if ( !file )
        {
            throw std::runtime_error( "failed to open file" );
        }

        nlohmann::json root;
        file >> root;
        ApplyGlyphParameterJson( root, glyph_property );
    }
    catch ( const std::exception& e )
    {
        std::cerr << "ERROR:Failed to read glyph parameter json: "
                  << fname << std::endl;
        std::cerr << "ERROR:" << e.what() << std::endl;
        ApplyGlyphDefault( glyph_property );
    }
}

void ParameterFileReader::readPlotOverLineParameterFile( const char* fname, PlotOverLineProperty& pol_property )
{
    try
    {
        std::ifstream file( fname );
        if ( !file )
        {
            throw std::runtime_error( "failed to open file" );
        }

        nlohmann::json root;
        file >> root;
        ApplyPlotOverLineParameterJson( root, pol_property );
    }
    catch ( const std::exception& e )
    {
        std::cerr << "ERROR:Failed to read plot over line parameter json: "
                  << fname << std::endl;
        std::cerr << "ERROR:" << e.what() << std::endl;
        ApplyPlotOverLineDefault( pol_property );
    }
}

void ParameterFileReader::readPlotOverTimeParameterFile( const char* fname, PlotOverTimeProperty& pot_property )
{
    try
    {
        std::ifstream file( fname );
        if ( !file )
        {
            throw std::runtime_error( "failed to open file" );
        }

        nlohmann::json root;
        file >> root;
        ApplyPlotOverTimeParameterJson( root, pot_property );
    }
    catch ( const std::exception& e )
    {
        std::cerr << "ERROR:Failed to read plot over time parameter json: "
                  << fname << std::endl;
        std::cerr << "ERROR:" << e.what() << std::endl;
        ApplyPlotOverTimeDefault( pot_property );
    }
}

// CSのConnect時に指定した.tfファイルを読み込んだ値を設定する
void ParameterFileReader::setTransferFunctionParameter( ParticleProperty& particle_property )
{
    const int tf_resolution = m_name_list_file.getValue<int>( "TF_RESOLUTION" );
    const int tf_number     = m_name_list_file.getValue<int>( "TF_NUMBER" );
    particle_property.m_color_transfer_function_synthesis = m_name_list_file.getValue<std::string>( "TF_SYNTH_C" );
    particle_property.m_opacity_transfer_function_synthesis = m_name_list_file.getValue<std::string>( "TF_SYNTH_O" );

    particle_property.m_transfunc_array.clear();
    particle_property.m_transfunc_array.resize( tf_number );

    for ( std::size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << i + 1 << "_";

        std::stringstream s_name;
        s_name << "t" << i + 1;

        const std::string tag_base = ss.str();
        particle_property.m_transfunc_array[i].m_resolution = tf_resolution;
        particle_property.m_transfunc_array[i].m_name = s_name.str();

        const std::string color_variable  = m_name_list_file.getValue<std::string>( tag_base + "VAR_C" );
        const std::string opacity_varible = m_name_list_file.getValue<std::string>( tag_base + "VAR_O" );
        const float color_min             = m_name_list_file.getValue<float>( tag_base + "MIN_C" );
        const float color_max             = m_name_list_file.getValue<float>( tag_base + "MAX_C" );
        const float opacity_min           = m_name_list_file.getValue<float>( tag_base + "MIN_O" );
        const float opacity_max           = m_name_list_file.getValue<float>( tag_base + "MAX_O" );
        std::string s_color               = m_name_list_file.getValue<std::string>( tag_base + "TABLE_C" );
        std::string s_opacity             = m_name_list_file.getValue<std::string>( tag_base + "TABLE_O" );

        particle_property.m_transfunc_array[i].m_server_color_range_mode   = NamedTransferFunction::ServerRangeMode::UserRange;
        particle_property.m_transfunc_array[i].m_server_opacity_range_mode = NamedTransferFunction::ServerRangeMode::UserRange;
        particle_property.m_transfunc_array[i].m_color_variable            = color_variable;
        particle_property.m_transfunc_array[i].m_opacity_variable          = opacity_varible;
        particle_property.m_transfunc_array[i].m_user_color_variable_min   = color_min;
        particle_property.m_transfunc_array[i].m_user_color_variable_max   = color_max;
        particle_property.m_transfunc_array[i].m_user_opacity_variable_min = opacity_min;
        particle_property.m_transfunc_array[i].m_user_opacity_variable_max = opacity_max;

        std::replace( s_color.begin(), s_color.end(), ',', ' ' );
        std::replace( s_opacity.begin(), s_opacity.end(), ',', ' ' );

        std::stringstream ss_color( s_color );
        std::stringstream ss_opacity( s_opacity );

        vismodule::ColorMap::Table color_table( tf_resolution * 3 );
        vismodule::OpacityMap::Table opacity_table( tf_resolution );

        for ( std::size_t j = 0; j < tf_resolution; j++ )
        {
            for ( std::size_t c = 0; c < 3; c++ )
            {
                int color_e;
                ss_color >> color_e;
                color_table.at( j * 3 + c ) = color_e;
            }
        }

        for ( std::size_t j = 0; j < tf_resolution; j++ )
        {
            float opacity;
            ss_opacity >> opacity;
            opacity_table.at( j ) = opacity;
        }

        vismodule::ColorMap color_map( color_table );
        vismodule::OpacityMap opacity_map( opacity_table );

        particle_property.m_transfunc_array[i].setColorMap( color_map );
        particle_property.m_transfunc_array[i].setOpacityMap( opacity_map );
        particle_property.m_transfunc_array[i].setColorRange( color_min, color_max );
        particle_property.m_transfunc_array[i].setOpacityRange( opacity_min, opacity_max );        
    }

    std::string equation;
    EquationToken eq;

    equation = m_name_list_file.getValue<std::string>( "TF_SYNTH_C" );
    std::replace( equation.begin(), equation.end(), 'C', 'c' );
    eq = particle_property.m_transfunc_synthesizer->convert_token( equation );
    particle_property.m_transfunc_synthesizer->setColorFunction( eq );

    equation = m_name_list_file.getValue<std::string>( "TF_SYNTH_O" );
    std::replace( equation.begin(), equation.end(), 'O', 'a' );
    eq = particle_property.m_transfunc_synthesizer->convert_token( equation );
    particle_property.m_transfunc_synthesizer->setOpacityFunction( eq );

    std::vector<EquationToken> var;

    for ( std::size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        equation = m_name_list_file.getValue<std::string>( tag_base + "VAR_C" );
        std::replace( equation.begin(), equation.end(), 'X', 'x' );
        std::replace( equation.begin(), equation.end(), 'Y', 'y' );
        std::replace( equation.begin(), equation.end(), 'Z', 'z' );
        eq = particle_property.m_transfunc_synthesizer->convert_token( equation );

        var.push_back( eq );
    }

    particle_property.m_transfunc_synthesizer->setColorVariable( var );
    var.clear();

    for ( std::size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        equation = m_name_list_file.getValue<std::string>( tag_base + "VAR_O" );
        std::replace( equation.begin(), equation.end(), 'X', 'x' );
        std::replace( equation.begin(), equation.end(), 'Y', 'y' );
        std::replace( equation.begin(), equation.end(), 'Z', 'z' );
        eq = particle_property.m_transfunc_synthesizer->convert_token( equation );

        var.push_back( eq );
    }
    particle_property.m_transfunc_synthesizer->setOpacityVariable( var );
    var.clear();

    return;
}

// ISの旧形式default.tfファイルを読み込んだ値を設定する
void ParameterFileReader::setParticleParameter( ParticleProperty& particle_property )
{
    const std::string size_sampling_method                  = m_name_list_file.getValue<std::string>("SAMPLING_METHOD");
    particle_property.m_particle_limit                      = m_name_list_file.getValue<int32_t>( "PARTICLE_LIMIT" );
    // particle_property.m_extra_opacity_factor                = m_name_list_file.getValue<float>( "EXTRA_OPACITY_FACTOR" ); // 一時的にコメントアウト
    particle_property.m_extra_opacity_factor                = 1; // 一時的にハードコーティング
    particle_property.m_particle_data_size_limit            = m_name_list_file.getValue<float>( "PARTICLE_DATA_SIZE_LIMIT" );
    particle_property.m_color_transfer_function_synthesis   = m_name_list_file.getValue<std::string>( "COLOR_SYNTH" );
    particle_property.m_opacity_transfer_function_synthesis = m_name_list_file.getValue<std::string>( "OPACITY_SYNTH" );
    
    if ( size_sampling_method == "Uniform" )
    {
        particle_property.m_sampling_method = 'u';
    }
    else if ( size_sampling_method == "Metropolis" )
    {
        particle_property.m_sampling_method = 'm';
    }
    else if ( size_sampling_method == "Rejection" )
    {
        particle_property.m_sampling_method = 'r';
    }
    else
    {
        std::cout << "ERROR:particle sampling method is not selected." << std::endl;
        return;   
    }

    const std::size_t width               = m_name_list_file.getValue<size_t>( "RESOLUTION_WIDTH" );
    const std::size_t height              = m_name_list_file.getValue<size_t>( "RESOLUTION_HEIGHT" );
    particle_property.m_camera->setWindowSize( width, height );

    const std::size_t resolution          = m_name_list_file.getValue<int>( "TF_RESOLUTION" );
    const int tf_number              = m_name_list_file.getValue<int>( "TF_NUMBER" );

    particle_property.m_transfunc_array.clear();
    particle_property.m_transfunc_array.resize( tf_number );
    // particle_property.m_voleqn.clear();
    // particle_property.m_voleqn.resize( tf_number );
    
    for ( std::size_t n = 0; n < tf_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";

        std::stringstream s_name;
        std::stringstream f_name;
        s_name << "t" << n + 1;
        f_name << "_F" << n + 1 << "_VAR_";

        const std::string tag_base = ss.str();
        particle_property.m_transfunc_array[n].m_resolution = resolution;
        particle_property.m_transfunc_array[n].m_name = s_name.str();

        const std::string color_variable         = m_name_list_file.getValue<std::string>( tag_base + "VAR_C" );
        const std::string opacity_varible        = m_name_list_file.getValue<std::string>( tag_base + "VAR_O" );
        const std::string color_range_mode       = m_name_list_file.getValue<std::string>( tag_base + "RANGE_MODE_C" );
        const std::string opacity_range_mode     = m_name_list_file.getValue<std::string>( tag_base + "RANGE_MODE_O" );
        const float server_color_min             = m_name_list_file.getValue<float>( tag_base + "SERVER_MIN_C" );
        const float server_color_max             = m_name_list_file.getValue<float>( tag_base + "SERVER_MAX_C" );
        const float user_color_min               = m_name_list_file.getValue<float>( tag_base + "USER_MIN_C" );
        const float user_color_max               = m_name_list_file.getValue<float>( tag_base + "USER_MAX_C" );
        const float server_opacity_min           = m_name_list_file.getValue<float>( tag_base + "SERVER_MIN_O" );
        const float server_opacity_max           = m_name_list_file.getValue<float>( tag_base + "SERVER_MAX_O" );
        const float user_opacity_min             = m_name_list_file.getValue<float>( tag_base + "USER_MIN_O" );
        const float user_opacity_max             = m_name_list_file.getValue<float>( tag_base + "USER_MAX_O" );
        std::string s_color                      = m_name_list_file.getValue<std::string>( tag_base + "TABLE_C" );
        std::string s_opacity                    = m_name_list_file.getValue<std::string>( tag_base + "TABLE_O" );

        if ( color_range_mode == "ServerSide" )
        {
            particle_property.m_transfunc_array[n].m_server_color_range_mode = NamedTransferFunction::ServerRangeMode::ServerSide;
        }
        else if ( color_range_mode == "UserRange" )
        {
            particle_property.m_transfunc_array[n].m_server_color_range_mode = NamedTransferFunction::ServerRangeMode::UserRange;
        }
        else
        {
            std::cout << "ERROR:Color Range Mode is unknown" << std::endl;
        }

        if ( opacity_range_mode == "ServerSide" )
        {
            particle_property.m_transfunc_array[n].m_server_opacity_range_mode = NamedTransferFunction::ServerRangeMode::ServerSide;
        }
        else if ( opacity_range_mode == "UserRange" )
        {
            particle_property.m_transfunc_array[n].m_server_opacity_range_mode = NamedTransferFunction::ServerRangeMode::UserRange;
        }
        else
        {
            std::cout << "ERROR:Opacity Range Mode is unknown" << std::endl;
        }

        particle_property.m_transfunc_array[n].m_color_variable              = color_variable;
        particle_property.m_transfunc_array[n].m_opacity_variable            = opacity_varible;
        particle_property.m_transfunc_array[n].m_server_color_variable_min   = server_color_min;
        particle_property.m_transfunc_array[n].m_server_color_variable_max   = server_color_max;
        particle_property.m_transfunc_array[n].m_server_opacity_variable_min = server_opacity_min;
        particle_property.m_transfunc_array[n].m_server_opacity_variable_max = server_opacity_max;
        particle_property.m_transfunc_array[n].m_user_color_variable_min     = user_color_min;
        particle_property.m_transfunc_array[n].m_user_color_variable_max     = user_color_max;
        particle_property.m_transfunc_array[n].m_user_opacity_variable_min   = user_opacity_min;
        particle_property.m_transfunc_array[n].m_user_opacity_variable_max   = user_opacity_max;

        std::replace( s_color.begin(), s_color.end(), ',', ' ' );
        std::replace( s_opacity.begin(), s_opacity.end(), ',', ' ' );

        std::stringstream ss_color( s_color );
        std::stringstream ss_opacity( s_opacity );

        vismodule::ColorMap::Table color_table( resolution * 3 );
        vismodule::OpacityMap::Table opacity_table( resolution );

        for ( std::size_t i = 0; i < resolution; i++ )
        {
            for ( std::size_t c = 0; c < 3; c++ )
            {
                int color_e;
                ss_color >> color_e;
                color_table.at( i * 3 + c ) = color_e;
            }
        }

        for ( std::size_t i = 0; i < resolution; i++ )
        {
            float opacity;
            ss_opacity >> opacity;
            opacity_table.at( i ) = opacity;
        }

        vismodule::ColorMap color_map( color_table );
        vismodule::OpacityMap opacity_map( opacity_table );

        particle_property.m_transfunc_array[n].setColorMap( color_map );
        particle_property.m_transfunc_array[n].setOpacityMap( opacity_map );

        if ( particle_property.m_transfunc_array[n].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[n].setColorRange( server_color_min, server_color_max );
        }
        else if ( particle_property.m_transfunc_array[n].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::UserRange )
        {
            particle_property.m_transfunc_array[n].setColorRange( user_color_min, user_color_max );
        }
        else
        {
            std::cout << "ERROR:Color Range Mode is unknown" << std::endl;
        }

        if ( particle_property.m_transfunc_array[n].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[n].setOpacityRange( server_opacity_min, server_opacity_max );
        }
        else if ( particle_property.m_transfunc_array[n].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::UserRange )
        {
            particle_property.m_transfunc_array[n].setOpacityRange( user_opacity_min, user_opacity_max );
        }
        else
        {
            std::cout << "ERROR:Opacity Range Mode is unknown" << std::endl;
        }

        // particle_property.m_transfunc_array[n].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;

        // particle_property.m_voleqn[n].m_name     = f_name.str() + "C";
        // particle_property.m_voleqn[n].m_equation = color_variable;
    }

    std::string equation;
    EquationToken eq;

    equation = m_name_list_file.getValue<std::string>( "COLOR_SYNTH" );
    std::replace( equation.begin(), equation.end(), 'C', 'c' );
    eq = particle_property.m_transfunc_synthesizer->convert_token( equation );
    particle_property.m_transfunc_synthesizer->setColorFunction( eq );

    equation = m_name_list_file.getValue<std::string>( "OPACITY_SYNTH" );
    std::replace( equation.begin(), equation.end(), 'O', 'a' );
    eq = particle_property.m_transfunc_synthesizer->convert_token( equation );
    particle_property.m_transfunc_synthesizer->setOpacityFunction( eq );

    std::vector<EquationToken> var;

    for ( std::size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        equation = m_name_list_file.getValue<std::string>( tag_base + "VAR_C" );
        std::replace( equation.begin(), equation.end(), 'X', 'x' );
        std::replace( equation.begin(), equation.end(), 'Y', 'y' );
        std::replace( equation.begin(), equation.end(), 'Z', 'z' );
        eq = particle_property.m_transfunc_synthesizer->convert_token( equation );

        var.push_back( eq );
    }

    particle_property.m_transfunc_synthesizer->setColorVariable( var );
    var.clear();

    for ( std::size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        equation = m_name_list_file.getValue<std::string>( tag_base + "VAR_O" );
        std::replace( equation.begin(), equation.end(), 'X', 'x' );
        std::replace( equation.begin(), equation.end(), 'Y', 'y' );
        std::replace( equation.begin(), equation.end(), 'Z', 'z' );
        eq = particle_property.m_transfunc_synthesizer->convert_token( equation );

        var.push_back( eq );
    }
    particle_property.m_transfunc_synthesizer->setOpacityVariable( var );
    var.clear();

    return;
}

std::vector<int> ParameterFileReader::getTableInt( std::string table_string )
{
    std::vector<int> table;

    while( 1 )
    {
        int pos = table_string.find_first_of(",");
        if( pos == std::string::npos ) break;
        std::string num = table_string.substr( 0, pos );
        table.push_back( std::atoi( num.c_str() ) );
        table_string = table_string.substr( pos+1 );
    }

    return table;
}

std::vector<float> ParameterFileReader::getTableFloat( std::string table_string )
{
    std::vector<float> table;

    while( 1 )
    {
        int pos = table_string.find_first_of(",");
        if( pos == std::string::npos ) break;
        std::string num = table_string.substr( 0, pos );
        table.push_back( std::atof( num.c_str() ) );
        table_string = table_string.substr( pos+1 );
    }

    return table;
}

std::vector<std::string> ParameterFileReader::getTableString( std::string table_string )
{
    std::vector<std::string> table;

    while( 1 )
    {
        int pos = table_string.find_first_of(",");
        if( pos == std::string::npos ) break;
        std::string num = table_string.substr( 0, pos );
        table.push_back( num.c_str() );
        table_string = table_string.substr( pos+1 );
    }

    return table;
}


void ParameterFileReader::set_default_parameter()
{
    m_name_list_file.setLine( "PARTICLE_LIMIT"          , static_cast<int>( 10000000 ) );
    // m_name_list_file.setLine( "EXTRA_OPACITY_FACTOR"        , static_cast<float>( 1.0 ) ); // 一時的にコメントアウト
    m_name_list_file.setLine( "PARTICLE_DATA_SIZE_LIMIT", static_cast<float>( 20.0 ) );
    m_name_list_file.setLine( "RESOLUTION_WIDTH"        , static_cast<int>( 620 ) );
    m_name_list_file.setLine( "RESOLUTION_HEIGHT"       , static_cast<int>( 620 ) );
    m_name_list_file.setLine( "TF_RESOLUTION"           , static_cast<int>( 256 ) );
    //m_name_list_file.setLine( "TF_SYNTH"                , std::string( "t1" ) );
    m_name_list_file.setLine( "COLOR_SYNTH"             , std::string( "C1" ) );
    m_name_list_file.setLine( "OPACITY_SYNTH"           , std::string( "O1" ) );
    m_name_list_file.setLine( "TF_NUMBER"               , static_cast<int>( DEFAULT_TF_NUMBER ) );
 
    vismodule::TransferFunction tf;
    vismodule::ColorMap::Table   color_table   = tf.colorMap().table();
    vismodule::OpacityMap::Table opacity_table = tf.opacityMap().table();
    
    std::stringstream tag_c, table_c;
    
    for ( std::size_t i = 0; i < color_table.size(); i++ )
    {
        table_c << static_cast<int>( color_table.at( i ) ) << ",";
    }
    
    std::stringstream tag_o, table_o;
    
    for ( std::size_t i = 0; i < opacity_table.size(); i++ )
    {
        table_o << opacity_table.at( i ) << ",";
    }
    
    int tf_size = DEFAULT_TF_NUMBER;
    for ( std::size_t n = 0; n < tf_size ; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
    
        const std::string tag_base = ss.str();
    
        m_name_list_file.setLine( tag_base + "VAR_C", "q1" );
        m_name_list_file.setLine( tag_base + "MIN_C", static_cast<float>( 0.0 ) );
        m_name_list_file.setLine( tag_base + "MAX_C", static_cast<float>( 1.0 ) );
        m_name_list_file.setLine( tag_base + "TABLE_C", table_c.str() );
        m_name_list_file.setLine( tag_base + "VAR_O", "q1" );
        m_name_list_file.setLine( tag_base + "MIN_O", static_cast<float>( 0.0 ) );
        m_name_list_file.setLine( tag_base + "MAX_O", static_cast<float>( 1.0 ) );
        m_name_list_file.setLine( tag_base + "TABLE_O", table_o.str() );
    }

    m_name_list_file.setLine( "END_PARAMETER_FILE", "SUCCESS" );
}

void ParameterFileReader::setNameListFile( const NameListFile& nameListFile )
{
    m_name_list_file = nameListFile;
}

const NameListFile& ParameterFileReader::getNameListFile() const
{
    return m_name_list_file;
}
