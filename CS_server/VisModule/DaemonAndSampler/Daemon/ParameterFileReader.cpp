#include "ParameterFileReader.h"
#include <vismodule/ExtendedTransferFunctionParameter>
#define DEFAULT_TF_NUMBER 5
#define BEFORE_READ_TF_NUMBER 99

void ParameterFileReader::readParticleParameterFile( const char* fname )
{
    m_name_list_file.setName( "SAMPLING_METHOD" );
    m_name_list_file.setName( "PARTICLE_LIMIT" );
    m_name_list_file.setName( "PARTICLE_DENSITY" );
    m_name_list_file.setName( "PARTICLE_DATA_SIZE_LIMIT" );
    m_name_list_file.setName( "RESOLUTION_WIDTH" );
    m_name_list_file.setName( "RESOLUTION_HEIGHT" );
    m_name_list_file.setName( "TF_RESOLUTION" );
    m_name_list_file.setName( "COLOR_SYNTH" );
    m_name_list_file.setName( "OPACITY_SYNTH" );
    m_name_list_file.setName( "TF_NUMBER" );

    for ( size_t n = 0; n < BEFORE_READ_TF_NUMBER; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";

        const std::string tag_base = ss.str();
        m_name_list_file.setName( tag_base + "VAR_C" );
        m_name_list_file.setName( tag_base + "MIN_C" );
        m_name_list_file.setName( tag_base + "MAX_C" );
        m_name_list_file.setName( tag_base + "VAR_O" );
        m_name_list_file.setName( tag_base + "MIN_O" );
        m_name_list_file.setName( tag_base + "MAX_O" );
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
            m_name_list_file.deleteLine( tag_base + "MIN_C" );
            m_name_list_file.deleteLine( tag_base + "MAX_C" );
            m_name_list_file.deleteLine( tag_base + "VAR_O" );
            m_name_list_file.deleteLine( tag_base + "MIN_O" );
            m_name_list_file.deleteLine( tag_base + "MAX_O" );
            m_name_list_file.deleteLine( tag_base + "TABLE_C" );
            m_name_list_file.deleteLine( tag_base + "TABLE_O" );
        }
    }

    return;
}

void ParameterFileReader::readGlyphParameterFile( const char* fname )
{
    m_name_list_file.setName( "GLYPH_FLAG" );
    m_name_list_file.setName( "STRIDE" );
    m_name_list_file.setName( "SEED" );
    m_name_list_file.setName( "NUMBER_OF_SMAPLING_POINT" );
    m_name_list_file.setName( "GLYPH_COLOR_MAX" );
    m_name_list_file.setName( "GLYPH_COLOR_MIN" );
    m_name_list_file.setName( "SIZE_VARIABLES" );
    m_name_list_file.setName( "COLOR_VARIABLES" );
    m_name_list_file.setName( "DIRECTION_VARIABLES" );
    m_name_list_file.setName( "DISTRIBUTION_MODE" );
    m_name_list_file.setName( "SIZE_SAMPLING_METHOD" );
    m_name_list_file.setName( "COLOR_DATA_SAMPLING_METHOD" );
    m_name_list_file.setName( "GLYPH_COLOR_MAP_TABLE" );
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

    return;
}

void ParameterFileReader::readPlotOverLineParameterFile( const char* fname )
{
    m_name_list_file.setName( "PLOT_FLAG" );
    m_name_list_file.setName( "PLOT_VARIABLE" );
    m_name_list_file.setName( "SAMPLING_SIZE" );
    m_name_list_file.setName( "START_POINT" );
    m_name_list_file.setName( "END_POINT" );
    m_name_list_file.setName( "END_PARAMETER_FILE" );

    m_name_list_file.setFileName( std::string( fname ) );

    bool is_read_finished = false;

    while( !is_read_finished )
    {
        if( !m_name_list_file.read() )
        {
            std::cout << "!m_name_list_file.read()" << std::endl;
            this->set_default_parameter();
            break;
        }

        std::string result = m_name_list_file.getValue<std::string>( "END_PARAMETER_FILE" );

        if( result == "SUCCESS" )
        {
            is_read_finished = true;
        }
    }

    return;
}

void ParameterFileReader::outputMessage( jpv::ParticleTransferServerMessage* server_message )
{
    this->outputParameterMessage( server_message );
    this->outputTransferFunctionMessage( server_message );
}

void ParameterFileReader::outputParameterMessage( jpv::ParticleTransferServerMessage* server_message )
{
    server_message->m_particle_limit         =  m_name_list_file.getValue<int32_t>( "PARTICLE_LIMIT"    );
    server_message->m_particle_density       =  m_name_list_file.getValue<float>(   "PARTICLE_DENSITY"  );
    size_t width                           =  m_name_list_file.getValue<size_t>(  "RESOLUTION_WIDTH"  );
    size_t height                          =  m_name_list_file.getValue<size_t>(  "RESOLUTION_HEIGHT" );
    server_message->m_camera->setWindowSize( width, height );
    server_message->m_particle_data_size_limit =  m_name_list_file.getValue<float>(   "PARTICLE_DATA_SIZE_LIMIT"  );
}

void ParameterFileReader::setParticleParameter( Argument& param )
{
    const std::string size_sampling_method = m_name_list_file.getValue<std::string>("SAMPLING_METHOD");
    param.m_particle_limit                 = m_name_list_file.getValue<int32_t>( "PARTICLE_LIMIT" );
    param.m_particle_density               = m_name_list_file.getValue<float>( "PARTICLE_DENSITY" );
    param.m_particle_data_size_limit       = m_name_list_file.getValue<float>( "PARTICLE_DATA_SIZE_LIMIT" );

    if ( size_sampling_method == "Uniform" )
    {
        param.m_sampling_method = 'u';
    }
    else if ( size_sampling_method == "Metropolis" )
    {
        param.m_sampling_method = 'm';
    }
    else if ( size_sampling_method == "Rejection" )
    {
        param.m_sampling_method = 'r';
    }
    else
    {
        std::cout << "ERROR:particle sampling method is not selected." << std::endl;
        return;   
    }

    const size_t width               = m_name_list_file.getValue<size_t>( "RESOLUTION_WIDTH" );
    const size_t height              = m_name_list_file.getValue<size_t>( "RESOLUTION_HEIGHT" );
    param.m_camera->setWindowSize( width, height );

    const size_t resolution          = m_name_list_file.getValue<int>( "TF_RESOLUTION" );
    const int tf_number              = m_name_list_file.getValue<int>( "TF_NUMBER" );

    param.m_transfunc_array.clear();
    param.m_transfunc_array.resize( tf_number );
    param.m_named_transfunc_array.clear();
    param.m_named_transfunc_array.resize( tf_number );
    param.m_voleqn.clear();
    param.m_voleqn.resize( tf_number );
    
    for ( size_t n = 0; n < tf_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";

        std::stringstream s_name;
        std::stringstream f_name;
        s_name << "t" << n + 1;
        f_name << "_F" << n + 1 << "_VAR_";

        const std::string tag_base = ss.str();
        param.m_named_transfunc_array[n].setResolution( resolution );
        param.m_named_transfunc_array[n].m_name = s_name.str();

        const std::string color_variable  = m_name_list_file.getValue<std::string>( tag_base + "VAR_O" );
        const std::string opacity_varible = m_name_list_file.getValue<std::string>( tag_base + "VAR_O" );
        const float color_min             = m_name_list_file.getValue<float>( tag_base + "MIN_C" );
        const float color_max             = m_name_list_file.getValue<float>( tag_base + "MAX_C" );
        const float opacity_min           = m_name_list_file.getValue<float>( tag_base + "MIN_O" );
        const float opacity_max           = m_name_list_file.getValue<float>( tag_base + "MAX_O" );
        std::string s_color               = m_name_list_file.getValue<std::string>( tag_base + "TABLE_C" );
        std::string s_opacity             = m_name_list_file.getValue<std::string>( tag_base + "TABLE_O" );

        param.m_named_transfunc_array[n].m_color_variable       = color_variable;
        param.m_named_transfunc_array[n].m_opacity_variable     = opacity_varible;
        param.m_named_transfunc_array[n].m_color_variable_min   = color_min;
        param.m_named_transfunc_array[n].m_color_variable_max   = color_max;
        param.m_named_transfunc_array[n].m_opacity_variable_min = opacity_min;
        param.m_named_transfunc_array[n].m_opacity_variable_max = opacity_max;

        std::replace( s_color.begin(), s_color.end(), ',', ' ' );
        std::replace( s_opacity.begin(), s_opacity.end(), ',', ' ' );

        std::stringstream ss_color( s_color );
        std::stringstream ss_opacity( s_opacity );

        vismodule::ColorMap::Table color_table( resolution * 3 );
        vismodule::OpacityMap::Table opacity_table( resolution );

        for ( size_t i = 0; i < resolution; i++ )
        {
            for ( size_t c = 0; c < 3; c++ )
            {
                int color_e;
                ss_color >> color_e;
                color_table.at( i * 3 + c ) = color_e;
            }
        }

        for ( size_t i = 0; i < resolution; i++ )
        {
            float opacity;
            ss_opacity >> opacity;
            opacity_table.at( i ) = opacity;
        }

        vismodule::ColorMap color_map( color_table );
        vismodule::OpacityMap opacity_map( opacity_table );

        param.m_transfunc_array[n].setColorMap( color_map );
        param.m_transfunc_array[n].setOpacityMap( opacity_map );
        param.m_named_transfunc_array[n].setColorMap( color_map );
        param.m_named_transfunc_array[n].setOpacityMap( opacity_map );
        param.m_transfunc_array[n].setColorRange( color_min, color_max );
        param.m_transfunc_array[n].setOpacityRange( opacity_min, opacity_max );
        param.m_named_transfunc_array[n].setColorRange( color_min, color_max );
        param.m_named_transfunc_array[n].setOpacityRange( opacity_min, opacity_max );

        param.m_named_transfunc_array[n].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;

        param.m_voleqn[n].m_name     = f_name.str() + "C";
        param.m_voleqn[n].m_equation = color_variable;
    }

    std::string equation;
    EquationToken eq;

    equation   = m_name_list_file.getValue<std::string>( "COLOR_SYNTH" );
    std::replace( equation.begin(), equation.end(), 'C', 'c' );
    eq = param.m_transfunc_synthesizer->convert_token( equation );

    equation = m_name_list_file.getValue<std::string>( "OPACITY_SYNTH" );
    std::replace( equation.begin(), equation.end(), 'O', 'a' );
    eq = param.m_transfunc_synthesizer->convert_token( equation );

    std::vector<EquationToken> var;

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        equation = m_name_list_file.getValue<std::string>( tag_base + "VAR_C" );
        eq = param.m_transfunc_synthesizer->convert_token( equation );

        var.push_back( eq );
    }

    param.m_transfunc_synthesizer->setColorVariable( var );
    var.clear();

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        equation = m_name_list_file.getValue<std::string>( tag_base + "VAR_O" );
        eq = param.m_transfunc_synthesizer->convert_token( equation );

        var.push_back( eq );
    }
    param.m_transfunc_synthesizer->setOpacityVariable( var );
    var.clear();

    return;
}

void ParameterFileReader::setGlyphParameter( Argument& param )
{
    const std::string g_flag = m_name_list_file.getValue<std::string>( "GLYPH_FLAG" );
    if ( strcmp( g_flag.c_str(), "TRUE" ) == 0 )
    {
        param.m_glyph_flag = true;
    }
    else
    {
        param.m_glyph_flag = false;
        return;
    }

    const std::string size_sampling_method = m_name_list_file.getValue<std::string>("SIZE_SAMPLING_METHOD");
    if ( size_sampling_method == "Constant" )
    {
        param.m_size_sampling_method = jpv::DataDefines::Constant;
    }
    else if ( size_sampling_method == "SingleVariable" )
    {
        param.m_size_sampling_method = jpv::DataDefines::SingleVariable;
    }
    else if ( size_sampling_method == "VariableArray" )
    {
        param.m_size_sampling_method = jpv::DataDefines::VariableArray;
    }
    else
    {
        std::cout << "ERROR:size sampling method is not selected, so skip generate glyph." << std::endl;
        param.m_glyph_flag = false;
        return;        
    }

    const std::string distribution_mode = m_name_list_file.getValue<std::string>("DISTRIBUTION_MODE");
    if ( distribution_mode == "AllPoints" )
    {
        param.m_distribution_mode = jpv::GlyphMode::AllPoints;
    }
    else if ( distribution_mode == "EveryNthPoints" )
    {
        param.m_distribution_mode = jpv::GlyphMode::EveryNthPoints;
    }
    else if ( distribution_mode == "UniformDistribution" )
    {
        param.m_distribution_mode = jpv::GlyphMode::UniformDistribution; 
    }
    else
    {
        std::cout << "ERROR:distribution mode is not selected, so skip generate glyph." << std::endl;
        param.m_glyph_flag = false;
        return;   
    }

    if ( distribution_mode == "AllPoints" )
    {
        param.m_stride = 1;
    }
    else
    {
        param.m_stride = m_name_list_file.getValue<int>("STRIDE");
    }

    param.m_seed                                  = m_name_list_file.getValue<int>("SEED");
    param.m_number_of_sampling_point              = m_name_list_file.getValue<int>("NUMBER_OF_SMAPLING_POINT");

    const std::string color_sampling_method = m_name_list_file.getValue<std::string>("COLOR_DATA_SAMPLING_METHOD");
    if (color_sampling_method == "Constant" )
    {
        param.m_color_data_sampling_method = jpv::DataDefines::Constant;
    }
    else if (color_sampling_method == "SingleVariable" )
    {
        param.m_color_data_sampling_method = jpv::DataDefines::SingleVariable;
    }
    else if (color_sampling_method == "VariableArray" )
    {
        param.m_color_data_sampling_method = jpv::DataDefines::VariableArray;
    }
    else
    {
        std::cout << "ERROR:color data sampling method is not selected, so skip generate glyph." << std::endl;
        param.m_glyph_flag = false;
        return;        
    }

    const std::string size_variables_string       = m_name_list_file.getValue<std::string>( "SIZE_VARIABLES" );
    const std::vector<std::string> size_variables_string_table       = getTableString( size_variables_string );

    param.m_size_variable.resize( size_variables_string_table.size() );
    for ( size_t i = 0; i < size_variables_string_table.size(); i++ )
    {
        param.m_size_variable[i] = size_variables_string_table[i];
    }

    const std::string color_data_variables_string = m_name_list_file.getValue<std::string>( "COLOR_VARIABLES" );    
    const std::vector<std::string> color_data_variables_string_table = getTableString( color_data_variables_string );
    
    param.m_color_data_variable.resize( color_data_variables_string_table.size() );
    for ( size_t i = 0; i < color_data_variables_string_table.size(); i++ )
    {
        param.m_color_data_variable[i] = color_data_variables_string_table[i];
    }
    
    const std::string direction_variables_string  = m_name_list_file.getValue<std::string>( "DIRECTION_VARIABLES" );
    const std::vector<std::string> direction_variables_string_table  = getTableString( direction_variables_string );
    
    if ( direction_variables_string_table.size() < 3 )
    {
        std::cout << "INFO:direction variables number is less 3, so skip generate glyph." << std::endl;
        param.m_glyph_flag = false;
        return;
    }

    for ( size_t i = 0; i < 3; i++ )
    {
        param.m_direction_variable[i] = direction_variables_string_table[i];
    }

    const float glyph_color_min = m_name_list_file.getValue<float>("GLYPH_COLOR_MIN");
    const float glyph_color_max = m_name_list_file.getValue<float>("GLYPH_COLOR_MAX");
    const std::string color_map_string   = m_name_list_file.getValue<std::string>( "GLYPH_COLOR_MAP_TABLE" );
    const std::vector<int> color_map_int_table = getTableInt( color_map_string );
    vismodule::ValueArray<vismodule::UInt8> color_map_uint_table( color_map_int_table.size() );
    for ( size_t i = 0; i < color_map_int_table.size(); i++ )
    {
        color_map_uint_table[i] = (vismodule::UInt8)color_map_int_table[i];
    }
    vismodule::ColorMap color_map( color_map_uint_table, glyph_color_min, glyph_color_max );
    param.m_color_map = color_map;

#if 1 // debug
    std::cout << "param.m_direction_variable[0]      = " << param.m_direction_variable[0]    << std::endl; 
    std::cout << "param.m_direction_variable[1]      = " << param.m_direction_variable[1]    << std::endl; 
    std::cout << "param.m_direction_variable[2]      = " << param.m_direction_variable[2]    << std::endl; 
    std::cout << "param.m_size_sampling_method       = " << size_sampling_method             << std::endl; 
    std::cout << "param.m_distribution_mode          = " << distribution_mode                << std::endl; 
    std::cout << "param.m_stride                     = " << param.m_stride                   << std::endl; 
    std::cout << "param.m_seed                       = " << param.m_seed                     << std::endl; 
    std::cout << "param.m_number_of_sampling_point   = " << param.m_number_of_sampling_point << std::endl; 
    std::cout << "param.m_color_data_sampling_method = " << color_sampling_method            << std::endl; 

    // for( size_t i = 0; i < param.m_size_variable.size(); i++ )
    for ( size_t i = 0; i < 1; i++ )
    {
        std::cout << "param.m_size_variable[" << i << "]          = " << param.m_size_variable[i] << std::endl; 
    }

    // // for( size_t i = 0; i < param.m_color_data_variable.size(); i++ )
    for ( size_t i = 0; i < 1; i++ )
    {
        std::cout << "param.m_color_data_variable[" << i << "]     = " << param.m_color_data_variable[i] <<  std::endl; 
    }

    for ( size_t i = 0; i < 3; i++ )
    {
        std::cout << "param.m_direction_variable[" << i << "]     = " << param.m_direction_variable[i] <<  std::endl; 
    }
#endif
}

void ParameterFileReader::setPlotOverLineParameter( Argument& param )
{
    std::string p_flag = m_name_list_file.getValue<std::string>("PLOT_FLAG");

    if ( strcmp( p_flag.c_str(), "TRUE" ) == 0 )
    {
        param.m_plot_flag = true;
    }
    else
    {
        param.m_plot_flag = false;
        return;
    }

    param.m_sampling_size = m_name_list_file.getValue<int>("SAMPLING_SIZE");
    param.m_plot_variable = m_name_list_file.getValue<std::string>("PLOT_VARIABLE");

    const std::string start_point_string = m_name_list_file.getValue<std::string>("START_POINT");
    const std::vector<float> start_point_float_table = getTableFloat( start_point_string );

    param.m_start_point[0] = start_point_float_table[0];
    param.m_start_point[1] = start_point_float_table[1];
    param.m_start_point[2] = start_point_float_table[2];

    const std::string end_point_string = m_name_list_file.getValue<std::string>("END_POINT");
    const std::vector<float> end_point_float_table = getTableFloat( end_point_string );
    param.m_end_point[0] = end_point_float_table[0];
    param.m_end_point[1] = end_point_float_table[1];
    param.m_end_point[2] = end_point_float_table[2];

    std::cout << "param.m_sampling_size  = " << param.m_sampling_size << std::endl;
    std::cout << "param.m_plot_variable  = " << param.m_plot_variable << std::endl;
    std::cout << "param.m_start_point[0] = " << param.m_start_point[0] << std::endl;
    std::cout << "param.m_start_point[1] = " << param.m_start_point[1] << std::endl;
    std::cout << "param.m_start_point[2] = " << param.m_start_point[2] << std::endl;
    std::cout << "param.m_end_point[0]   = " << param.m_end_point[0] << std::endl; 
    std::cout << "param.m_end_point[1]   = " << param.m_end_point[1] << std::endl; 
    std::cout << "param.m_end_point[2]   = " << param.m_end_point[2] << std::endl; 
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

void ParameterFileReader::outputTransferFunctionMessage( jpv::ParticleTransferServerMessage* server_message )
{
    const size_t resolution                   = m_name_list_file.getValue<int>(         "TF_RESOLUTION" );
    //server_message->transferFunctionSynthesis  = m_name_list_file.getValue<std::string>( "TF_SYNTH"      );
    server_message->m_color_transfer_function_synthesis = m_name_list_file.getValue<std::string>( "COLOR_SYNTH" );
    server_message->m_opacity_transfer_function_synthesis = m_name_list_file.getValue<std::string>( "OPACITY_SYNTH" );

    const int cur_tf_number = m_name_list_file.getValue<int>("TF_NUMBER");
    server_message->m_transfer_function.resize( cur_tf_number );
    for ( size_t n = 0; n < cur_tf_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";

        std::stringstream s_name;
        s_name << "t" << n + 1;

        const std::string tag_base = ss.str();
        server_message->m_transfer_function[n].setResolution( resolution );
        server_message->m_transfer_function[n].m_name          = s_name.str();
        server_message->m_transfer_function[n].m_color_variable      = m_name_list_file.getValue<std::string>( tag_base + "VAR_C" );
        server_message->m_transfer_function[n].m_color_variable_min   = m_name_list_file.getValue<float>( tag_base + "MIN_C" );
        server_message->m_transfer_function[n].m_color_variable_max   = m_name_list_file.getValue<float>( tag_base + "MAX_C" );
        server_message->m_transfer_function[n].m_opacity_variable    = m_name_list_file.getValue<std::string>( tag_base + "VAR_O" );
        server_message->m_transfer_function[n].m_opacity_variable_min = m_name_list_file.getValue<float>( tag_base + "MIN_O" );
        server_message->m_transfer_function[n].m_opacity_variable_max = m_name_list_file.getValue<float>( tag_base + "MAX_O" );

        std::string s_color =   m_name_list_file.getValue<std::string>( tag_base + "TABLE_C" );
        std::string s_opacity = m_name_list_file.getValue<std::string>( tag_base + "TABLE_O" );

        std::replace( s_color.begin(), s_color.end(), ',', ' ' );
        std::replace( s_opacity.begin(), s_opacity.end(), ',', ' ' );

        std::stringstream ss_color( s_color );
        std::stringstream ss_opacity( s_opacity );

        vismodule::ColorMap::Table color_table( resolution * 3 );
        vismodule::OpacityMap::Table opacity_table( resolution );

        for ( size_t i = 0; i < resolution; i++ )
        {
            for ( size_t c = 0; c < 3; c++ )
            {
                int color_e;
                ss_color >> color_e;
                color_table.at( i * 3 + c ) = color_e;
            }
        }

        for ( size_t i = 0; i < resolution; i++ )
        {
            float opacity;
            ss_opacity >> opacity;
            opacity_table.at( i ) = opacity;
        }

        vismodule::ColorMap color_map( color_table );
        vismodule::OpacityMap opacity_map( opacity_table );
        vismodule::TransferFunction tf( color_map, opacity_map );
        vismodule::TransferFunction& tt = server_message->m_transfer_function[n];
        tt = tf;
    }

}

void ParameterFileReader::set_default_parameter()
{
    m_name_list_file.setLine( "PARTICLE_LIMIT"          , static_cast<int>( 10000000 ) );
    m_name_list_file.setLine( "PARTICLE_DENSITY"        , static_cast<float>( 1.0 ) );
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
    
    for ( size_t i = 0; i < color_table.size(); i++ )
    {
        table_c << static_cast<int>( color_table.at( i ) ) << ",";
    }
    
    std::stringstream tag_o, table_o;
    
    for ( size_t i = 0; i < opacity_table.size(); i++ )
    {
        table_o << opacity_table.at( i ) << ",";
    }
    
    int tf_size = DEFAULT_TF_NUMBER;
    for ( size_t n = 0; n < tf_size ; n++ )
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
