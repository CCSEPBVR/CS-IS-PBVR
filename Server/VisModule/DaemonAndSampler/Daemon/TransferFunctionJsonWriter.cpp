#include "TransferFunctionJsonWriter.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{

std::string Trim( const std::string& value )
{
    const std::string whitespace = " \t\r\n";
    const std::string::size_type first = value.find_first_not_of( whitespace );
    if ( first == std::string::npos ) return "";

    const std::string::size_type last = value.find_last_not_of( whitespace );
    return value.substr( first, last - first + 1 );
}

bool IsIntegerText( const std::string& value )
{
    if ( value.empty() ) return false;

    std::size_t index = 0;
    if ( value[0] == '-' || value[0] == '+' ) index = 1;
    if ( index == value.size() ) return false;

    for ( ; index < value.size(); ++index )
    {
        if ( value[index] < '0' || value[index] > '9' ) return false;
    }
    return true;
}

bool IsFloatingText( const std::string& value )
{
    if ( value.empty() ) return false;

    char* end = 0;
    std::strtod( value.c_str(), &end );
    return end != value.c_str() && end != 0 && *end == '\0';
}

nlohmann::json ParseScalar( const std::string& text )
{
    const std::string value = Trim( text );

    if ( IsIntegerText( value ) )
    {
        return std::strtoll( value.c_str(), 0, 10 );
    }

    if ( IsFloatingText( value ) )
    {
        return std::strtod( value.c_str(), 0 );
    }

    return value;
}

nlohmann::json ParseValue( const std::string& text )
{
    if ( text.find( ',' ) == std::string::npos )
    {
        return ParseScalar( text );
    }

    nlohmann::json values = nlohmann::json::array();
    std::stringstream stream( text );
    std::string token;

    while ( std::getline( stream, token, ',' ) )
    {
        token = Trim( token );
        if ( token.empty() ) continue;
        values.push_back( ParseScalar( token ) );
    }

    return values;
}

std::string SamplingMethodName( const char sampling_method )
{
    switch ( sampling_method )
    {
    case 'u': return "Uniform";
    case 'm': return "Metropolis";
    case 'r': return "Rejection";
    default:  return "UNKNOWN";
    }
}

std::string RangeModeName( const NamedTransferFunction::ServerRangeMode range_mode )
{
    switch ( range_mode )
    {
    case NamedTransferFunction::ServerRangeMode::UserRange:
        return "UserRange";
    case NamedTransferFunction::ServerRangeMode::ServerSide:
        return "ServerSide";
    default:
        return "Unknown";
    }
}

std::string RangeModeName( const EnsembleTransferFunction::ServerRangeMode range_mode )
{
    switch ( range_mode )
    {
    case EnsembleTransferFunction::ServerRangeMode::UserRange:
        return "UserRange";
    case EnsembleTransferFunction::ServerRangeMode::ServerSide:
        return "ServerSide";
    default:
        return "Unknown";
    }
}

void AddParameter( nlohmann::json& root, const std::string& key, const nlohmann::json& value )
{
    root["order"].push_back( key );
    root["parameters"][key] = value;
}

std::string Indent( const int depth )
{
    return std::string( depth * 4, ' ' );
}

bool ShouldWriteInlineArray( const std::string& key )
{
    if ( key == "values" ) return true;
    if ( key.size() > 8 && key.find( "TF_NAME" ) == 0 )
    {
        return key.find( "_TABLE_C" ) != std::string::npos ||
               key.find( "_TABLE_O" ) != std::string::npos;
    }
    return false;
}

std::string DumpInlineArray( const nlohmann::json& value )
{
    std::stringstream stream;
    stream << "[";
    for ( size_t i = 0; i < value.size(); ++i )
    {
        if ( i > 0 ) stream << ", ";
        stream << value.at( i ).dump();
    }
    stream << "]";
    return stream.str();
}

std::string DumpReadableJson( const nlohmann::json& value, const int depth = 0, const std::string& key = "" )
{
    if ( value.is_array() )
    {
        if ( ShouldWriteInlineArray( key ) ) return DumpInlineArray( value );
        if ( value.empty() ) return "[]";

        std::stringstream stream;
        stream << "[\n";
        for ( size_t i = 0; i < value.size(); ++i )
        {
            if ( i > 0 ) stream << ",\n";
            stream << Indent( depth + 1 ) << DumpReadableJson( value.at( i ), depth + 1 );
        }
        stream << "\n" << Indent( depth ) << "]";
        return stream.str();
    }

    if ( value.is_object() )
    {
        if ( value.empty() ) return "{}";

        std::stringstream stream;
        stream << "{\n";
        bool first = true;
        for ( nlohmann::json::const_iterator it = value.begin(); it != value.end(); ++it )
        {
            if ( !first ) stream << ",\n";
            first = false;
            stream << Indent( depth + 1 )
                   << nlohmann::json( it.key() ).dump()
                   << ": "
                   << DumpReadableJson( it.value(), depth + 1, it.key() );
        }
        stream << "\n" << Indent( depth ) << "}";
        return stream.str();
    }

    return value.dump();
}

nlohmann::json ColorTableToJson( const NamedTransferFunction& transfer_function )
{
    nlohmann::json color_map = nlohmann::json::array();
    const vismodule::ColorMap::Table color_table = transfer_function.colorMap().table();
    const unsigned char* table = color_table.pointer();

    for ( size_t i = 0; i < color_table.size(); ++i )
    {
        color_map.push_back( static_cast<int>( table[i] ) );
    }

    return color_map;
}

nlohmann::json ColorTableToJson( const EnsembleTransferFunction& transfer_function )
{
    nlohmann::json color_map = nlohmann::json::array();
    const vismodule::ColorMap::Table color_table = transfer_function.colorMap().table();
    const unsigned char* table = color_table.pointer();

    for ( size_t i = 0; i < color_table.size(); ++i )
    {
        color_map.push_back( static_cast<int>( table[i] ) );
    }

    return color_map;
}

nlohmann::json OpacityTableToJson( const NamedTransferFunction& transfer_function )
{
    nlohmann::json opacity_map = nlohmann::json::array();
    const vismodule::OpacityMap::Table opacity_table = transfer_function.opacityMap().table();

    for ( size_t i = 0; i < opacity_table.size(); ++i )
    {
        opacity_map.push_back( opacity_table.at( i ) );
    }

    return opacity_map;
}

nlohmann::json OpacityTableToJson( const EnsembleTransferFunction& transfer_function )
{
    nlohmann::json opacity_map = nlohmann::json::array();
    const vismodule::OpacityMap::Table opacity_table = transfer_function.opacityMap().table();

    for ( size_t i = 0; i < opacity_table.size(); ++i )
    {
        opacity_map.push_back( opacity_table.at( i ) );
    }

    return opacity_map;
}

nlohmann::json TableDescription(
    const std::string& role,
    const std::string& encoding,
    const size_t length,
    const nlohmann::json& values )
{
    nlohmann::json table;
    table["role"] = role;
    table["encoding"] = encoding;
    table["length"] = length;
    table["values"] = values;
    return table;
}

nlohmann::json RangeDescription(
    const std::string& mode,
    const float user_min,
    const float user_max )
{
    nlohmann::json range;
    range["active_range"] = mode == "ServerSide" ? "server" : "user";
    range["user"] = { { "min", user_min }, { "max", user_max } };
    return range;
}

nlohmann::json TransferFunctionArrayToJson( const std::vector<NamedTransferFunction>& transfer_functions )
{
    nlohmann::json array = nlohmann::json::array();
    for ( size_t i = 0; i < transfer_functions.size(); ++i )
    {
        const NamedTransferFunction& source = transfer_functions[i];
        const std::string color_mode = RangeModeName( source.m_server_color_range_mode );
        const std::string opacity_mode = RangeModeName( source.m_server_opacity_range_mode );
        const nlohmann::json color_table = ColorTableToJson( source );
        const nlohmann::json opacity_table = OpacityTableToJson( source );

        nlohmann::json tf;
        tf["id"] = i + 1;
        tf["label"] = "TF" + std::to_string( i + 1 );
        tf["color"]["variable"] = source.m_color_variable;
        tf["color"]["range"] = RangeDescription(
            color_mode,
            source.m_user_color_variable_min,
            source.m_user_color_variable_max );
        tf["color"]["map"] = TableDescription(
            "color map",
            "flat RGB uint8 triplets",
            color_table.size(),
            color_table );

        tf["opacity"]["variable"] = source.m_opacity_variable;
        tf["opacity"]["range"] = RangeDescription(
            opacity_mode,
            source.m_user_opacity_variable_min,
            source.m_user_opacity_variable_max );
        tf["opacity"]["map"] = TableDescription(
            "opacity map",
            "float values from 0 to 1",
            opacity_table.size(),
            opacity_table );

        array.push_back( tf );
    }

    return array;
}

nlohmann::json TransferFunctionArrayToJson( const std::vector<EnsembleTransferFunction>& transfer_functions )
{
    nlohmann::json array = nlohmann::json::array();
    for ( size_t i = 0; i < transfer_functions.size(); ++i )
    {
        const EnsembleTransferFunction& source = transfer_functions[i];
        const std::string mode = RangeModeName( source.m_server_range_mode );
        const nlohmann::json color_table = ColorTableToJson( source );
        const nlohmann::json opacity_table = OpacityTableToJson( source );

        nlohmann::json tf;
        tf["id"] = source.m_id > 0 ? source.m_id : static_cast<int>( i + 1 );
        tf["label"] = source.m_label.empty() ? "TF" + std::to_string( i + 1 ) : source.m_label;
        tf["color"]["variable"] = source.m_variable;
        tf["color"]["range"] = RangeDescription(
            mode,
            source.m_user_variable_min,
            source.m_user_variable_max );
        tf["color"]["map"] = TableDescription(
            "color map",
            "flat RGB uint8 triplets",
            color_table.size(),
            color_table );

        tf["opacity"]["variable"] = source.m_variable;
        tf["opacity"]["range"] = RangeDescription(
            mode,
            source.m_user_variable_min,
            source.m_user_variable_max );
        tf["opacity"]["map"] = TableDescription(
            "opacity map",
            "float values from 0 to 1",
            opacity_table.size(),
            opacity_table );

        array.push_back( tf );
    }

    return array;
}

nlohmann::json BuildHumanReadableView( const ParticleProperty& particle_property )
{
    nlohmann::json view;
    view["purpose"] = "Particle-based volume rendering parameters for PBVR.";
    view["editing_notes"] = nlohmann::json::array(
    {
        "The settings section contains global settings shared across the visualization.",
        "Each transfer_functions entry defines the mapping between a physical quantity computed by a synthesis expression and its corresponding color and opacity.",
        "The range.active_range field specifies the min/max values used for histogram generation. When set to user, user.min and user.max are used. When set to server, server.min and server.max are used.",
        "The color.map.values field contains a flat uint8 RGB array with three components (R, G, B) per control point.",
        "The opacity.map.values field contains a one-dimensional array of floating-point values in the range [0, 1].",
        "color_synthesis and opacity_synthesis specify the synthesis expressions defined in the Transfer Function Editor.",
        "The mean_transfer_functions, variance_transfer_functions, and coefficient_of_variation_transfer_functions sections contain transfer-function parameters for ensemble statistics."
    } );

    view["settings"]["sampling"]["method"] = SamplingMethodName( particle_property.m_sampling_method );
    view["settings"]["sampling"]["particle_limit"] = particle_property.m_particle_limit;
    view["settings"]["sampling"]["particle_data_size_limit"] = particle_property.m_particle_data_size_limit;
    view["settings"]["sampling"]["particle_data_size_limit_unit"] = "MB";
    view["settings"]["sampling"]["repeat_level"] = particle_property.m_repeat_level;

    if ( particle_property.m_camera != 0 )
    {
        view["settings"]["image"]["width"] = particle_property.m_camera->windowWidth();
        view["settings"]["image"]["height"] = particle_property.m_camera->windowHeight();
    }

    view["settings"]["transfer_function"]["transfer_function_count"] = particle_property.m_transfunc_array.size();
    if ( !particle_property.m_transfunc_array.empty() )
    {
        view["settings"]["transfer_function"]["transfer_function_resolution"] = particle_property.m_transfunc_array[0].m_resolution;
    }
    view["settings"]["transfer_function"]["color_synthesis"] = particle_property.m_color_transfer_function_synthesis;
    view["settings"]["transfer_function"]["opacity_synthesis"] = particle_property.m_opacity_transfer_function_synthesis;

    //view["transfer_functions"] = TransferFunctionArrayToJson( particle_property.m_transfunc_array );
    view["transfer_functions"] = TransferFunctionArrayToJson( particle_property.m_mean_transfer_function_array );
    view["mean_transfer_functions"] = TransferFunctionArrayToJson( particle_property.m_mean_transfer_function_array );
    view["variance_transfer_functions"] = TransferFunctionArrayToJson( particle_property.m_variance_transfer_function_array );
    view["coefficient_of_variation_transfer_functions"] =
        TransferFunctionArrayToJson( particle_property.m_coefficient_of_variation_transfer_function_array );

    return view;
}

} // namespace

namespace TransferFunctionJsonWriter
{

nlohmann::json ToJson( const ParticleProperty& particle_property )
{
    nlohmann::json root;
    root["format"] = "PBVR transfer-function parameters";
    root["schema_version"] = 1;
    const nlohmann::json view = BuildHumanReadableView( particle_property );
    root["purpose"] = view["purpose"];
    root["documentation"]["transfer_function_editor_jp"] = "https://github.com/CCSEPBVR/CS-IS-PBVR/wiki/TransferFunctionEditor_JP";
    root["documentation"]["transfer_function_editor_en"] = "https://github.com/CCSEPBVR/CS-IS-PBVR/wiki/TransferFunctionEditor_EN";
    root["editing_notes"] = view["editing_notes"];
    root["settings"] = view["settings"];
    root["transfer_functions"] = view["transfer_functions"];
    root["mean_transfer_functions"] = view["mean_transfer_functions"];
    root["variance_transfer_functions"] = view["variance_transfer_functions"];
    root["coefficient_of_variation_transfer_functions"] =
        view["coefficient_of_variation_transfer_functions"];

    return root;
}

void WriteTfJson( const ParticleProperty& particle_property, const std::string& json_file_path )
{
    const nlohmann::json root = ToJson( particle_property );
    const std::string tmp_json_file_path = json_file_path + ".tmp";

    std::ofstream output( tmp_json_file_path.c_str() );
    if ( !output )
    {
        throw std::runtime_error( "Cannot open temporary json file for writing: " + tmp_json_file_path );
    }

    output << DumpReadableJson( root ) << std::endl;
    output.close();
    if ( !output )
    {
        throw std::runtime_error( "Cannot write temporary json file: " + tmp_json_file_path );
    }

    if ( std::rename( tmp_json_file_path.c_str(), json_file_path.c_str() ) != 0 )
    {
        throw std::runtime_error( "Cannot rename temporary json file: " + tmp_json_file_path + " -> " + json_file_path );
    }
}

nlohmann::json LoadTfJson( const std::string& json_file_path )
{
    std::ifstream input( json_file_path.c_str() );
    if ( !input )
    {
        throw std::runtime_error( "Cannot open json file: " + json_file_path );
    }

    nlohmann::json root;
    input >> root;
    return root;
}

nlohmann::json LoadParticlePropertyJson( const std::string& json_file_path )
{
    return LoadTfJson( json_file_path );
}

} // namespace default_tf_json
