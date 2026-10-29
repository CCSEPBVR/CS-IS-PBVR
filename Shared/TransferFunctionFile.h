#ifndef PBVR_TRANSFER_FUNCTION_FILE_H
#define PBVR_TRANSFER_FUNCTION_FILE_H

#include "json.hpp"
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// File settings are complete snapshots. Range selection and runtime data are
// deliberately absent; both CS readers always select UserRange.
namespace TransferFunctionFile
{
constexpr int Resolution = 256;
constexpr int MaximumCount = 16;
constexpr int SchemaVersion = 1;
constexpr const char* Format = "PBVR transfer-function parameters";

struct Channel
{
    std::string variable;
    double minimum = 0;
    double maximum = 0;
    std::vector<double> values;
};

struct Item
{
    Channel color;
    Channel opacity;
};

struct Document
{
    std::string colorSynthesis;
    std::string opacitySynthesis;
    std::vector<Item> items;
};

inline bool IsJsonPath( const std::string& path )
{
    if ( path.size() < 5 ) return false;
    std::string extension = path.substr( path.size() - 5 );
    for ( char& c : extension )
        if ( c >= 'A' && c <= 'Z' ) c += 'a' - 'A';
    return extension == ".json";
}

inline void Require( bool condition, const std::string& path, const std::string& reason )
{
    if ( !condition ) throw std::runtime_error( "Invalid transfer function JSON: " + path + ": " + reason );
}

inline double Number( const nlohmann::json& value, const std::string& path )
{
    Require( value.is_number(), path, "expected a number" );
    const double number = value.get<double>();
    Require( std::isfinite( number ) && std::abs( number ) <= std::numeric_limits<float>::max(),
             path, "expected a finite number representable by the server" );
    return number;
}

inline std::string Expression( const nlohmann::json& value, const std::string& path )
{
    Require( value.is_string(), path, "expected an expression string" );
    const auto expression = value.get<std::string>();
    Require( expression.find_first_not_of( " \t\r\n" ) != std::string::npos &&
             expression.find( '\0' ) == std::string::npos, path, "expression is empty or contains a null character" );
    return expression;
}

inline Channel ReadChannel( const nlohmann::json& channel, bool color, const std::string& path )
{
    Require( channel.is_object(), path, "expected an object" );
    Channel result;
    result.variable = Expression( channel.at( "variable" ), path + ".variable" );
    const auto& range = channel.at( "range" ).at( "user" );
    result.minimum = Number( range.at( "min" ), path + ".range.user.min" );
    result.maximum = Number( range.at( "max" ), path + ".range.user.max" );
    Require( result.minimum <= result.maximum, path + ".range.user", "min must be <= max" );
    const auto& map = channel.at( "map" );
    const auto& values = map.at( "values" );
    const int length = color ? Resolution * 3 : Resolution;
    Require( map.at( "length" ).is_number_integer() && map.at( "length" ) == length,
             path + ".map.length", "incorrect element count" );
    Require( values.is_array() && values.size() == static_cast<std::size_t>( length ),
             path + ".map.values", "incorrect element count" );
    const std::string encoding = color ? "flat RGB uint8 triplets" : "float values from 0 to 1";
    Require( map.at( "encoding" ) == encoding, path + ".map.encoding", "unsupported encoding" );
    for ( std::size_t i = 0; i < values.size(); ++i )
    {
        const std::string valuePath = path + ".map.values[" + std::to_string( i ) + "]";
        const double number = Number( values.at( i ), valuePath );
        Require( number >= 0 && number <= ( color ? 255 : 1 ), valuePath, "value is outside the allowed range" );
        Require( !color || values.at( i ).is_number_integer(), valuePath, "RGB values must be integers" );
        result.values.push_back( number );
    }
    return result;
}

inline Document FromJson( const nlohmann::json& root )
{
    Require( root.is_object(), "root", "expected an object" );
    Require( root.at( "format" ) == Format, "format", "unsupported format" );
    Require( root.at( "schema_version" ).is_number_integer() && root.at( "schema_version" ) == SchemaVersion,
             "schema_version", "unsupported version" );
    const auto& settings = root.at( "settings" ).at( "transfer_function" );
    Require( settings.at( "transfer_function_resolution" ).is_number_integer() &&
             settings.at( "transfer_function_resolution" ) == Resolution,
             "settings.transfer_function.transfer_function_resolution", "only resolution 256 is supported" );
    const auto& items = root.at( "transfer_functions" );
    Require( items.is_array() && !items.empty() && items.size() <= MaximumCount,
             "transfer_functions", "expected 1 to 16 transfer functions" );
    Require( settings.at( "transfer_function_count" ).is_number_integer() &&
             settings.at( "transfer_function_count" ) == items.size(),
             "settings.transfer_function.transfer_function_count", "does not match the array size" );
    Document result;
    result.colorSynthesis = Expression( settings.at( "color_synthesis" ), "settings.transfer_function.color_synthesis" );
    result.opacitySynthesis = Expression( settings.at( "opacity_synthesis" ), "settings.transfer_function.opacity_synthesis" );
    for ( std::size_t i = 0; i < items.size(); ++i )
    {
        const auto& item = items.at( i );
        const std::string path = "transfer_functions[" + std::to_string( i ) + "]";
        Require( item.at( "id" ).is_number_integer() && item.at( "id" ) == i + 1,
                 path + ".id", "IDs must be consecutive and start at 1" );
        result.items.push_back( { ReadChannel( item.at( "color" ), true, path + ".color" ),
                                  ReadChannel( item.at( "opacity" ), false, path + ".opacity" ) } );
    }
    return result;
}

inline nlohmann::json ChannelToJson( const Channel& channel, bool color )
{
    auto values = nlohmann::json::array();
    for ( const double value : channel.values )
    {
        if ( color )
        {
            Require( std::isfinite( value ) && value >= 0 && value <= 255 && std::floor( value ) == value,
                     "color.map.values", "RGB values must be integers from 0 to 255" );
            values.push_back( static_cast<int>( value ) );
        }
        else values.push_back( value );
    }
    return {
        { "variable", channel.variable },
        { "range", { { "user", { { "min", channel.minimum }, { "max", channel.maximum } } } } },
        { "map", {
            { "role", color ? "color map" : "opacity map" },
            { "encoding", color ? "flat RGB uint8 triplets" : "float values from 0 to 1" },
            { "length", values.size() },
            { "values", values }
        } }
    };
}

inline nlohmann::json ToJson( const Document& document )
{
    auto items = nlohmann::json::array();
    for ( std::size_t i = 0; i < document.items.size(); ++i )
    {
        items.push_back( {
            { "id", i + 1 }, { "label", "TF" + std::to_string( i + 1 ) },
            { "color", ChannelToJson( document.items[i].color, true ) },
            { "opacity", ChannelToJson( document.items[i].opacity, false ) }
        } );
    }
    nlohmann::json root = {
        { "format", Format }, { "schema_version", SchemaVersion },
        { "settings", { { "transfer_function", {
            { "transfer_function_count", items.size() },
            { "transfer_function_resolution", Resolution },
            { "color_synthesis", document.colorSynthesis },
            { "opacity_synthesis", document.opacitySynthesis }
        } } } },
        { "transfer_functions", items }
    };
    FromJson( root ); // Reject incomplete or invalid snapshots before writing.
    return root;
}

inline std::string Serialize( const Document& document )
{
    std::string contents = ToJson( document ).dump( 2 );
    // All map values have been validated as numeric arrays. Keep each values
    // array on one line while retaining indentation for the document structure.
    const std::string marker = "\"values\": [";
    std::size_t position = 0;
    while ( ( position = contents.find( marker, position ) ) != std::string::npos )
    {
        const std::size_t begin = position + marker.size();
        const std::size_t end = contents.find( ']', begin );
        std::string values;
        for ( std::size_t i = begin; i < end; ++i )
        {
            const char c = contents[i];
            if ( c != ' ' && c != '\n' && c != '\r' && c != '\t' ) values += c;
        }
        contents.replace( begin, end - begin, values );
        position = begin + values.size() + 1;
    }
    return contents + "\n";
}
}

#endif
