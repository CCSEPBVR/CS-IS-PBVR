#include "ParameterFileWriter.h"

void ParameterFileWriter::getParticleParameter( const ParticleProperty& particle_property )
{
    std::string client_sampling_method;
    if ( particle_property.m_sampling_method == 'u' )
    {
        m_name_list_file.setLine( "SAMPLING_METHOD" , "Uniform" );
    }
    else if ( particle_property.m_sampling_method == 'm' )
    {
        m_name_list_file.setLine( "SAMPLING_METHOD" , "Metropolis" );
    }
    else if ( particle_property.m_sampling_method == 'r' )
    {
        m_name_list_file.setLine( "SAMPLING_METHOD" , "Rejection" );
    }
    else
    {
        std::cout << "ERROR:client sampling method is unknown." << std::endl;
        m_name_list_file.setLine( "SAMPLING_METHOD" , "UNKNOWN" );
    }

    m_name_list_file.setLine( "PARTICLE_LIMIT" , particle_property.m_particle_limit );
    m_name_list_file.setLine( "EXTRA_OPACITY_FACTOR" ,particle_property.m_extra_opacity_factor );
    m_name_list_file.setLine( "PARTICLE_DATA_SIZE_LIMIT" ,particle_property.m_particle_data_size_limit );
    m_name_list_file.setLine( "RESOLUTION_WIDTH" , particle_property.m_camera->windowWidth() );
    m_name_list_file.setLine( "RESOLUTION_HEIGHT", particle_property.m_camera->windowHeight() );
    m_name_list_file.setLine( "TF_RESOLUTION", particle_property.m_transfunc_array[0].m_resolution );
    m_name_list_file.setLine( "TF_NUMBER"    , (int)particle_property.m_transfunc_array.size() );

    const size_t tf_number = particle_property.m_transfunc_array.size();

    for ( size_t n = 0; n < tf_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";

        const std::string tag_base = ss.str();
        m_name_list_file.setLine( tag_base + "MIN_C", particle_property.m_transfunc_array[n].m_user_color_variable_min );
        m_name_list_file.setLine( tag_base + "MAX_C", particle_property.m_transfunc_array[n].m_user_color_variable_max );
        m_name_list_file.setLine( tag_base + "MIN_O", particle_property.m_transfunc_array[n].m_user_opacity_variable_min );
        m_name_list_file.setLine( tag_base + "MAX_O", particle_property.m_transfunc_array[n].m_user_opacity_variable_max );

        vismodule::ColorMap::Table   color_table   = particle_property.m_transfunc_array[n].colorMap().table();
        vismodule::OpacityMap::Table opacity_table = particle_property.m_transfunc_array[n].opacityMap().table();

        std::stringstream tag_c, table_c;

        tag_c << tag_base << "TABLE_C";

        for ( size_t i = 0; i < color_table.size(); i++ )
        {
            table_c << static_cast<int>( color_table.at( i ) ) << ",";
        }

        m_name_list_file.setLine( tag_c.str(), table_c.str() );

        std::stringstream tag_o, table_o;

        tag_o << tag_base << "TABLE_O";

        for ( size_t i = 0; i < opacity_table.size(); i++ )
        {
            table_o << opacity_table.at( i ) << ",";
        }

        m_name_list_file.setLine( tag_o.str(), table_o.str() );
    }

    m_name_list_file.setLine( "COLOR_SYNTH"  , particle_property.m_color_transfer_function_synthesis );
    m_name_list_file.setLine( "OPACITY_SYNTH", particle_property.m_opacity_transfer_function_synthesis );

    for ( size_t n = 0; n < tf_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
        const std::string tag_base = ss.str();
        
        m_name_list_file.setLine( tag_base + "VAR_C", particle_property.m_transfunc_array[n].m_color_variable );
        m_name_list_file.setLine( tag_base + "VAR_O", particle_property.m_transfunc_array[n].m_opacity_variable );
    }

    m_name_list_file.setLine( "END_PARAMETER_FILE", "SUCCESS" );
}

void ParameterFileWriter::getGlyphParameter( const GlyphProperty& glyph_property )
{
    std::string glyph_flag ="FALSE";
    if ( glyph_property.m_glyph_flag ) glyph_flag = "TRUE"; 

    m_name_list_file.setLine( "GLYPH_FLAG", glyph_flag );
    m_name_list_file.setLine( "SCALE_FACTOR", glyph_property.m_scale_factor );
    m_name_list_file.setLine( "STRIDE", glyph_property.m_stride );
    m_name_list_file.setLine( "SEED", glyph_property.m_seed );
    m_name_list_file.setLine( "NUMBER_OF_SMAPLING_POINT", glyph_property.m_number_of_sampling_point );
    m_name_list_file.setLine( "GLYPH_COLOR_MAX", 1 );
    m_name_list_file.setLine( "GLYPH_COLOR_MIN", 0 );

    // グリフタイプを文字列に変換
    std::string glyph_type;
    if ( glyph_property.m_glyph_type == GlyphType::Arrow ) glyph_type = "Arrow";
    else if ( glyph_property.m_glyph_type == GlyphType::Diamond ) glyph_type = "Diamond";
    else if ( glyph_property.m_glyph_type == GlyphType::Sphere ) glyph_type = "Sphere";
    else
    {
        glyph_type = "Invalid";
        std::cout << "ERROR:glyph type is invalid." << std::endl;
    }
    m_name_list_file.setLine( "GLYPH_TYPE", glyph_type );

    // 各成分を文字列に変換
    std::stringstream  size_variable, color_data_variable;
    for ( size_t i = 0; i < glyph_property.m_size_variable.size(); i++ )
    {
        size_variable  << glyph_property.m_size_variable.at( i ) << ",";
    }

    for ( size_t i = 0; i < glyph_property.m_color_data_variable.size(); i++ )
    {
        color_data_variable  << glyph_property.m_color_data_variable.at( i ) << ",";
    }

    std::stringstream  direction_variable;
    for ( size_t i = 0; i < 3; i++ )
    {
        direction_variable  << glyph_property.m_direction_variable[ i ] << ",";
    }

    m_name_list_file.setLine( "SIZE_VARIABLES", size_variable.str() );
    m_name_list_file.setLine( "COLOR_VARIABLES", color_data_variable.str() );
    m_name_list_file.setLine( "DIRECTION_VARIABLES", direction_variable.str() );

    std::string distribution_mode;
    if      ( glyph_property.m_distribution_mode          == GlyphMode::AllPoints )           distribution_mode = "AllPoints"; 
    else if ( glyph_property.m_distribution_mode          == GlyphMode::EveryNthPoints )      distribution_mode = "EveryNthPoints"; 
    else if ( glyph_property.m_distribution_mode          == GlyphMode::UniformDistribution ) distribution_mode = "UniformDistribution"; 

    std::string size_sampling_method;
    if      ( glyph_property.m_size_sampling_method       == DataDefines::Constant )       size_sampling_method = "Constant"; 
    else if ( glyph_property.m_size_sampling_method       == DataDefines::VariableArray )  size_sampling_method = "VariableArray"; 

    std::string color_sampling_method;
    if      ( glyph_property.m_color_data_sampling_method == DataDefines::Constant )       color_sampling_method = "Constant"; 
    else if ( glyph_property.m_color_data_sampling_method == DataDefines::VariableArray )  color_sampling_method = "VariableArray"; 

    m_name_list_file.setLine( "DISTRIBUTION_MODE", distribution_mode );
    m_name_list_file.setLine( "SIZE_SAMPLING_METHOD", size_sampling_method );
    m_name_list_file.setLine( "COLOR_DATA_SAMPLING_METHOD", color_sampling_method );

    std::stringstream table;

    for ( size_t i = 0; i < glyph_property.m_glyph_color_map_table.size(); i++ )
    {
        table << glyph_property.m_glyph_color_map_table.at( i ) << ",";
    }
    m_name_list_file.setLine( "GLYPH_COLOR_MAP_TABLE", table.str() );

    m_name_list_file.setLine( "END_PARAMETER_FILE", "SUCCESS" );
}

void ParameterFileWriter::getPlotOverLineParameter( const PlotOverLineProperty& pol_property )
{
    std::string plot_flag = "FALSE";
    if ( pol_property.m_plot_flag ) plot_flag ="TRUE"; 

    m_name_list_file.setLine( "PLOT_FLAG", plot_flag );
    m_name_list_file.setLine( "PLOT_VARIABLE", pol_property.m_plot_variable );
    m_name_list_file.setLine( "SAMPLING_SIZE", pol_property.m_sampling_size );

    // 各成分を文字列に変換
    std::stringstream  start_point, end_point;
    for ( size_t i = 0; i < 3; i++ )
    {
        start_point  << pol_property.m_start_point[i] << ",";
        end_point    << pol_property.m_end_point[i]   << ",";
    }

    m_name_list_file.setLine( "START_POINT", start_point.str() );
    m_name_list_file.setLine( "END_POINT", end_point.str() );

    m_name_list_file.setLine( "END_PARAMETER_FILE", "SUCCESS" );
}

void ParameterFileWriter::writeParameterFile( const char* fname )
{
    m_name_list_file.setFileName( std::string( fname ) );
    m_name_list_file.write();
}

const NameListFile& ParameterFileWriter::getNameListFile() const
{
    return m_name_list_file;
}
