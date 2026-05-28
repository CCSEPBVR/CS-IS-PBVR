#include "ParameterFileWriter.h"

#include <cstdio>
#include <fstream>
#include <ostream>
#include <stdexcept>

namespace
{

std::string GlyphTypeName( const GlyphType glyph_type )
{
    if ( glyph_type == GlyphType::Arrow ) return "Arrow";
    if ( glyph_type == GlyphType::Diamond ) return "Diamond";
    if ( glyph_type == GlyphType::Sphere ) return "Sphere";

    std::cout << "ERROR:glyph type is invalid." << std::endl;
    return "Invalid";
}

std::string GlyphModeName( const GlyphMode glyph_mode )
{
    if ( glyph_mode == GlyphMode::AllPoints ) return "AllPoints";
    if ( glyph_mode == GlyphMode::EveryNthPoints ) return "EveryNthPoints";
    if ( glyph_mode == GlyphMode::UniformDistribution ) return "UniformDistribution";
    return "Invalid";
}

std::string DataDefinesName( const DataDefines data_defines )
{
    if ( data_defines == DataDefines::Constant ) return "Constant";
    if ( data_defines == DataDefines::VariableArray ) return "VariableArray";
    return "Invalid";
}

std::string InlineJsonArray( const nlohmann::json& array )
{
    std::string text = "[";
    for ( std::size_t i = 0; i < array.size(); i++ )
    {
        if ( i > 0 ) text += ", ";
        text += array.at( i ).dump();
    }
    text += "]";
    return text;
}

void WriteGlyphParameterJson( std::ostream& os, const nlohmann::json& root )
{
    const nlohmann::json& size = root.at( "size" );
    const nlohmann::json& color_data = root.at( "color_data" );
    const nlohmann::json& color_range = color_data.at( "range" );

    os << "{\n";
    os << "    \"enabled\": " << root.at( "enabled" ).dump() << ",\n";
    os << "    \"type\": " << root.at( "type" ).dump() << ",\n";
    os << "    \"scale_factor\": " << root.at( "scale_factor" ).dump() << ",\n";
    os << "    \"stride\": " << root.at( "stride" ).dump() << ",\n";
    os << "    \"seed\": " << root.at( "seed" ).dump() << ",\n";
    os << "    \"number_of_sampling_points\": " << root.at( "number_of_sampling_points" ).dump() << ",\n";
    os << "    \"distribution_mode\": " << root.at( "distribution_mode" ).dump() << ",\n";
    os << "    \"direction_variables\": " << InlineJsonArray( root.at( "direction_variables" ) ) << ",\n";
    os << "    \"size\": {\n";
    os << "        \"sampling_method\": " << size.at( "sampling_method" ).dump() << ",\n";
    os << "        \"variables\": " << InlineJsonArray( size.at( "variables" ) ) << "\n";
    os << "    },\n";
    os << "    \"color_data\": {\n";
    os << "        \"sampling_method\": " << color_data.at( "sampling_method" ).dump() << ",\n";
    os << "        \"variables\": " << InlineJsonArray( color_data.at( "variables" ) ) << ",\n";
    os << "        \"range\": {\n";
    os << "            \"min\": " << color_range.at( "min" ).dump() << ",\n";
    os << "            \"max\": " << color_range.at( "max" ).dump() << "\n";
    os << "        }\n";
    os << "    },\n";
    os << "    \"color_map\": " << InlineJsonArray( root.at( "color_map" ) ) << "\n";
    os << "}\n";
}

void WritePlotOverLineParameterJson( std::ostream& os, const nlohmann::json& root )
{
    os << "{\n";
    os << "    \"enabled\": " << root.at( "enabled" ).dump() << ",\n";
    os << "    \"variable\": " << root.at( "variable" ).dump() << ",\n";
    os << "    \"sampling_size\": " << root.at( "sampling_size" ).dump() << ",\n";
    os << "    \"start_point\": " << InlineJsonArray( root.at( "start_point" ) ) << ",\n";
    os << "    \"end_point\": " << InlineJsonArray( root.at( "end_point" ) ) << "\n";
    os << "}\n";
}

void WritePlotOverTimeParameterJson( std::ostream& os, const nlohmann::json& root )
{
    os << "{\n";
    os << "    \"enabled\": " << root.at( "enabled" ).dump() << ",\n";
    os << "    \"target_point\": " << InlineJsonArray( root.at( "target_point" ) ) << "\n";
    os << "}\n";
}

} // namespace

ParameterFileWriter::ParameterFileWriter()
{
    const char *envBuf = NULL;

    envBuf = std::getenv( "VIS_PARAM_DIR" );

    if ( envBuf == nullptr )
    {
        m_particle_parameter_path     = "./";
        m_particle_parameter_old_path = "./";
        m_glyph_parameter_path        = "./";
        m_pol_parameter_path          = "./";
        m_pot_parameter_path          = "./";
    }
    else
    {
        m_particle_parameter_path     = envBuf;
        m_particle_parameter_old_path = envBuf;
        m_glyph_parameter_path        = envBuf;
        m_pol_parameter_path          = envBuf;
        m_pot_parameter_path          = envBuf;
        if ( m_particle_parameter_path[m_particle_parameter_path.size() - 1] != '/' )         m_particle_parameter_path     += "/";
        if ( m_particle_parameter_old_path[m_particle_parameter_old_path.size() - 1] != '/' ) m_particle_parameter_old_path += "/";
        if ( m_glyph_parameter_path[m_glyph_parameter_path.size() - 1] != '/' )               m_glyph_parameter_path        += "/";
        if ( m_pol_parameter_path[m_pol_parameter_path.size() - 1] != '/' )                   m_pol_parameter_path          += "/";
        if ( m_pot_parameter_path[m_pot_parameter_path.size() - 1] != '/' )                   m_pot_parameter_path          += "/";
    }

    envBuf = std::getenv( "TF_NAME" );

    if ( envBuf == nullptr )
    {
        m_particle_parameter_path     += "default.json";
        m_particle_parameter_old_path += "default_old.json";
        m_glyph_parameter_path        += "glyph_parameter.json";
        m_pol_parameter_path          += "plot_over_line_parameter.json";
        m_pot_parameter_path          += "plot_over_time_parameter.json";
    }
    else
    {
        m_particle_parameter_path += envBuf;
        m_particle_parameter_path     += ".json";
        m_particle_parameter_old_path += envBuf;
        m_particle_parameter_old_path += "_old.json";
        m_glyph_parameter_path    += "glyph_parameter.json";
        m_pol_parameter_path      += "plot_over_line_parameter.json";
        m_pot_parameter_path      += "plot_over_time_parameter.json";
    }

    // std::cout << "ParticleParameterPath:"     << m_particle_parameter_path << std::endl;
    // std::cout << "GlyphParameterPath:"        << m_glyph_parameter_path    << std::endl;
    // std::cout << "PlotOverLineParameterPath:" << m_pol_parameter_path      << std::endl;
    // std::cout << "PlotOverTimeParameterPath:" << m_pot_parameter_path      << std::endl;
}

std::string ParameterFileWriter::getParticleParameterPath()
{
    return m_particle_parameter_path;
}

std::string ParameterFileWriter::getGlyphParameterPath()
{
    return m_glyph_parameter_path;
}

std::string ParameterFileWriter::getPOLParameterPath()
{
    return m_pol_parameter_path;
}

std::string ParameterFileWriter::getPOTParameterPath()
{
    return m_pot_parameter_path;
}

void ParameterFileWriter::getGlyphParameter( const GlyphProperty& glyph_property )
{
    m_glyph_parameter_json = nlohmann::json::object();

    m_glyph_parameter_json["enabled"] = glyph_property.m_glyph_flag;
    m_glyph_parameter_json["type"] = GlyphTypeName( glyph_property.m_glyph_type );
    m_glyph_parameter_json["scale_factor"] = glyph_property.m_scale_factor;
    m_glyph_parameter_json["stride"] = glyph_property.m_stride;
    m_glyph_parameter_json["seed"] = glyph_property.m_seed;
    m_glyph_parameter_json["number_of_sampling_points"] =
        static_cast<int>( glyph_property.m_number_of_sampling_point );
    m_glyph_parameter_json["distribution_mode"] = GlyphModeName( glyph_property.m_distribution_mode );
    m_glyph_parameter_json["direction_variables"] = nlohmann::json::array(
        { glyph_property.m_direction_variable[0],
          glyph_property.m_direction_variable[1],
          glyph_property.m_direction_variable[2] } );

    m_glyph_parameter_json["size"]["sampling_method"] =
        DataDefinesName( glyph_property.m_size_sampling_method );
    m_glyph_parameter_json["size"]["variables"] = glyph_property.m_size_variable;

    m_glyph_parameter_json["color_data"]["sampling_method"] =
        DataDefinesName( glyph_property.m_color_data_sampling_method );
    m_glyph_parameter_json["color_data"]["variables"] = glyph_property.m_color_data_variable;
    m_glyph_parameter_json["color_data"]["range"]["min"] = glyph_property.m_glyph_color_min;
    m_glyph_parameter_json["color_data"]["range"]["max"] = glyph_property.m_glyph_color_max;
    m_glyph_parameter_json["color_map"] = glyph_property.m_glyph_color_map_table;
}

void ParameterFileWriter::getPlotOverLineParameter( const PlotOverLineProperty& pol_property )
{
    m_pol_parameter_json = nlohmann::json::object();
    m_pol_parameter_json["enabled"] = pol_property.m_plot_flag;
    m_pol_parameter_json["variable"] = pol_property.m_plot_variable;
    m_pol_parameter_json["sampling_size"] = pol_property.m_sampling_size;
    m_pol_parameter_json["start_point"] = nlohmann::json::array(
        { pol_property.m_start_point[0],
          pol_property.m_start_point[1],
          pol_property.m_start_point[2] } );
    m_pol_parameter_json["end_point"] = nlohmann::json::array(
        { pol_property.m_end_point[0],
          pol_property.m_end_point[1],
          pol_property.m_end_point[2] } );
}

void ParameterFileWriter::getPlotOverTimeParameter( const PlotOverTimeProperty& pot_property )
{
    m_pot_parameter_json = nlohmann::json::object();
    m_pot_parameter_json["enabled"] = pot_property.m_plot_flag;
    m_pot_parameter_json["target_point"] = nlohmann::json::array(
        { pot_property.m_target_point[0],
          pot_property.m_target_point[1],
          pot_property.m_target_point[2] } );
}

void ParameterFileWriter::writeTF2Json( const ParticleProperty& particle_property )
{
        // JSON ファイルで出力
        std::cout << "------------------------------------Export json ------------------------------------------" << std::endl;
       TransferFunctionJsonWriter::WriteTfJson( particle_property, m_particle_parameter_path );
}

void ParameterFileWriter::writeTF2OldJson( const ParticleProperty& particle_property )
{
        // JSON ファイルで出力
        std::cout << "------------------------------------Export json ------------------------------------------" << std::endl;
       TransferFunctionJsonWriter::WriteTfJson( particle_property, m_particle_parameter_old_path );
}

void ParameterFileWriter::writeGlyphParameterFile()
{
    const std::string tmp_json_file_path = m_glyph_parameter_path + ".tmp";

    std::ofstream file( tmp_json_file_path.c_str() );
    if ( !file )
    {
        throw std::runtime_error( "Cannot open temporary json file for writing: " + tmp_json_file_path );
    }

    WriteGlyphParameterJson( file, m_glyph_parameter_json );
    file.close();
    if ( !file )
    {
        throw std::runtime_error( "Cannot write temporary json file: " + tmp_json_file_path );
    }

    if ( std::rename( tmp_json_file_path.c_str(), m_glyph_parameter_path.c_str() ) != 0 )
    {
        throw std::runtime_error( "Cannot rename temporary json file: " + tmp_json_file_path + " -> " + m_glyph_parameter_path );
    }
}

void ParameterFileWriter::writePlotOverLineParameterFile()
{
    const std::string tmp_json_file_path = m_pol_parameter_path + ".tmp";

    std::ofstream file( tmp_json_file_path.c_str() );
    if ( !file )
    {
        throw std::runtime_error( "Cannot open temporary json file for writing: " + tmp_json_file_path );
    }

    WritePlotOverLineParameterJson( file, m_pol_parameter_json );
    file.close();
    if ( !file )
    {
        throw std::runtime_error( "Cannot write temporary json file: " + tmp_json_file_path );
    }

    if ( std::rename( tmp_json_file_path.c_str(), m_pol_parameter_path.c_str() ) != 0 )
    {
        throw std::runtime_error( "Cannot rename temporary json file: " + tmp_json_file_path + " -> " + m_pol_parameter_path );
    }
}

void ParameterFileWriter::writePlotOverTimeParameterFile()
{
    const std::string tmp_json_file_path = m_pot_parameter_path + ".tmp";

    std::ofstream file( tmp_json_file_path.c_str() );
    if ( !file )
    {
        throw std::runtime_error( "Cannot open temporary json file for writing: " + tmp_json_file_path );
    }

    WritePlotOverTimeParameterJson( file, m_pot_parameter_json );
    file.close();
    if ( !file )
    {
        throw std::runtime_error( "Cannot write temporary json file: " + tmp_json_file_path );
    }

    if ( std::rename( tmp_json_file_path.c_str(), m_pot_parameter_path.c_str() ) != 0 )
    {
        throw std::runtime_error( "Cannot rename temporary json file: " + tmp_json_file_path + " -> " + m_pot_parameter_path );
    }
}

const NameListFile& ParameterFileWriter::getNameListFile() const
{
    return m_name_list_file;
}
