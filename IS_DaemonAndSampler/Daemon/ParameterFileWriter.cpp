#include "ParameterFileWriter.h"

void ParameterFileWriter::inputMessage( const jpv::ParticleTransferClientMessage& client_message )
{
    this->inputParameterMessage( client_message );
    this->inputTransferFunctionMessage( client_message );
}

void ParameterFileWriter::inputParameterMessage( const jpv::ParticleTransferClientMessage& client_message )
{
//  m_name_list_file.setLine( "SUB_PIXEL_LEVEL", client_message.subPixelLevel  );
    m_name_list_file.setLine( "PARTICLE_LIMIT" , client_message.m_particle_limit );
    m_name_list_file.setLine( "PARTICLE_DENSITY" ,client_message.m_particle_density );
    m_name_list_file.setLine( "PARTICLE_DATA_SIZE_LIMIT" ,client_message.m_particle_data_size_limit );
//  if ( client_message.samplingMethod == 'u' )
//  {
//      m_name_list_file.setLine( "SAMPLING_TYPE", kvs::visclient::PBVRParam::UniformSampling );
//  }
//  else if ( client_message.samplingMethod == 'r' )
//  {
//      m_name_list_file.setLine( "SAMPLING_TYPE", kvs::visclient::PBVRParam::RejectionSampling );
//  }
//  else if ( client_message.samplingMethod == 'm' )
//  {
//      m_name_list_file.setLine( "SAMPLING_TYPE", kvs::visclient::PBVRParam::MetropolisSampling );
//  }
//  m_name_list_file.setLine( "EYE_POINT_X"      , client_message.m_camera->lookAt().x() );
//  m_name_list_file.setLine( "EYE_POINT_Y"      , client_message.m_camera->lookAt().y() );
//  m_name_list_file.setLine( "EYE_POINT_Z"      , client_message.m_camera->lookAt().z() );
//  m_name_list_file.setLine( "CENTER_POINT_X"   , client_message.m_camera->position().x() );
//  m_name_list_file.setLine( "CENTER_POINT_Y"   , client_message.m_camera->position().y() );
//  m_name_list_file.setLine( "CENTER_POINT_Z"   , client_message.m_camera->position().z() );
//  m_name_list_file.setLine( "UP_VECTOR_X"      , client_message.m_camera->upVector().x() );
//  m_name_list_file.setLine( "UP_VECTOR_Y"      , client_message.m_camera->upVector().y() );
//  m_name_list_file.setLine( "UP_VECTOR_Z"      , client_message.m_camera->upVector().z() );
    m_name_list_file.setLine( "RESOLUTION_WIDTH" , client_message.m_camera->windowWidth() );
    m_name_list_file.setLine( "RESOLUTION_HEIGHT", client_message.m_camera->windowHeight() );
//  m_name_list_file.setLine( "PFI_PATH_SERVER"  , client_message.inputDir );

//  m_name_list_file.setLine( "CROP_TYPE"        , client_message.enable_crop_region ) ;
//  m_name_list_file.setLine( "CROP_CXMIN"       , client_message.crop_region[0] );
//  m_name_list_file.setLine( "CROP_CYMIN"       , client_message.crop_region[1] );
//  m_name_list_file.setLine( "CROP_CZMIN"       , client_message.crop_region[2] );
//  m_name_list_file.setLine( "CROP_CXMAX"       , client_message.crop_region[3] );
//  m_name_list_file.setLine( "CROP_CYMAX"       , client_message.crop_region[4] );
//  m_name_list_file.setLine( "CROP_CZMAX"       , client_message.crop_region[5] );
//
//  m_name_list_file.setLine( "CROP_SCENTERX"    , client_message.crop_region[0] );
//  m_name_list_file.setLine( "CROP_SCENTERY"    , client_message.crop_region[1] );
//  m_name_list_file.setLine( "CROP_SCENTERZ"    , client_message.crop_region[2] );
//  m_name_list_file.setLine( "CROP_SRADIUS"     , client_message.crop_region[3] );
//
//  m_name_list_file.setLine( "CROP_PCENTERX"    , client_message.crop_region[0] );
//  m_name_list_file.setLine( "CROP_PCENTERY"    , client_message.crop_region[1] );
//  m_name_list_file.setLine( "CROP_PCENTERZ"    , client_message.crop_region[2] );
//  m_name_list_file.setLine( "CROP_PRADIUS"     , client_message.crop_region[3] );
//  m_name_list_file.setLine( "CROP_PHEIGHT"     , client_message.crop_region[4] );

//  m_name_list_file.setLine( "ROTATION1X"       , client_message.m_camera->rotation()[0][0] );
//  m_name_list_file.setLine( "ROTATION1Y"       , client_message.m_camera->rotation()[0][1] );
//  m_name_list_file.setLine( "ROTATION1Z"       , client_message.m_camera->rotation()[0][2] );
//  m_name_list_file.setLine( "ROTATION2X"       , client_message.m_camera->rotation()[1][0] );
//  m_name_list_file.setLine( "ROTATION2Y"       , client_message.m_camera->rotation()[1][1] );
//  m_name_list_file.setLine( "ROTATION2Z"       , client_message.m_camera->rotation()[1][2] );
//  m_name_list_file.setLine( "ROTATION3X"       , client_message.m_camera->rotation()[2][0] );
//  m_name_list_file.setLine( "ROTATION3Y"       , client_message.m_camera->rotation()[2][1] );
//  m_name_list_file.setLine( "ROTATION3Z"       , client_message.m_camera->rotation()[2][2] );

//  m_name_list_file.setLine( "TRANSLATIONX"     , client_message.m_camera->translation()[0] );
//  m_name_list_file.setLine( "TRANSLATIONY"     , client_message.m_camera->translation()[1] );
//  m_name_list_file.setLine( "TRANSLATIONZ"     , client_message.m_camera->translation()[2] );
//  m_name_list_file.setLine( "SCALINGX"         , client_message.m_camera->scaling()[0] );
//  m_name_list_file.setLine( "SCALINGY"         , client_message.m_camera->scaling()[1] );
//  m_name_list_file.setLine( "SCALINGZ"         , client_message.m_camera->scaling()[2] );
}

void ParameterFileWriter::inputTransferFunctionMessage( const jpv::ParticleTransferClientMessage& client_message )
{
    m_name_list_file.setLine( "TF_RESOLUTION", client_message.m_transfer_function[0].m_resolution);
    //m_name_list_file.setLine( "TF_SYNTH"     , client_message.transferFunctionSynthesis);
    m_name_list_file.setLine( "COLOR_SYNTH"  , client_message.m_color_transfer_function_synthesis);
    m_name_list_file.setLine( "OPACITY_SYNTH", client_message.m_opacity_transfer_function_synthesis);
    m_name_list_file.setLine( "TF_NUMBER"    , (int)(client_message.m_transfer_function.size()) );

#if 0
    //2019 kawamura
    std::stringstream tag, table;
    tag << "OPA_FUNC_EXP_TOKEN";
    for( int i = 0; i < 128; i++ )
    {
        table << client_message.opacity_func.exp_token[i] << ",";
    }
    m_name_list_file.setLine( tag.str(), table.str() );
    tag.str("");  tag.clear(std::stringstream::goodbit);
    table.str("");  table.clear(std::stringstream::goodbit);

    //2019 kawamura
    tag << "OPA_FUNC_VAR_NAME";
    for( int i = 0; i < 128; i++ )
    {
        table << client_message.opacity_func.var_name[i] << ",";
    }
    m_name_list_file.setLine( tag.str(), table.str() );
    tag.str("");  tag.clear(std::stringstream::goodbit);
    table.str("");  table.clear(std::stringstream::goodbit);

    //2019 kawamura
    tag << "OPA_FUNC_VAL_ARRAY";
    for( int i = 0; i < 128; i++ )
    {
        table << client_message.opacity_func.value_array[i] << ",";
    }
    m_name_list_file.setLine( tag.str(), table.str() );
    tag.str("");  tag.clear(std::stringstream::goodbit);
    table.str("");  table.clear(std::stringstream::goodbit);

    //2019 kawamura
    tag << "COL_FUNC_EXP_TOKEN";
    for( int i = 0; i < 128; i++ )
    {
        table << client_message.color_func.exp_token[i] << ",";
    }
    m_name_list_file.setLine( tag.str(), table.str() );
    tag.str("");  tag.clear(std::stringstream::goodbit);
    table.str("");  table.clear(std::stringstream::goodbit);

    //2019 kawamura
    tag << "COL_FUNC_VAR_NAME";
    for( int i = 0; i < 128; i++ )
    {
        table << client_message.color_func.var_name[i] << ",";
    }
    m_name_list_file.setLine( tag.str(), table.str() );
    tag.str("");  tag.clear(std::stringstream::goodbit);
    table.str("");  table.clear(std::stringstream::goodbit);

    //2019 kawamura
    tag << "COL_FUNC_VAL_ARRAY";
    for( int i = 0; i < 128; i++ )
    {
        table << client_message.color_func.value_array[i] << ",";
    }
    m_name_list_file.setLine( tag.str(), table.str() );
    tag.str("");  tag.clear(std::stringstream::goodbit);
    table.str("");  table.clear(std::stringstream::goodbit);
#endif

    for ( size_t n = 0; n < client_message.m_transfer_function.size(); n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";

        const std::string tag_base = ss.str();
        m_name_list_file.setLine( tag_base + "VAR_C", client_message.m_volume_equation[2*n].m_equation );

#if 0
    std::cout << __FUNCTION__  << ": " << __LINE__ << std::endl;
        //2019 kawamura
        tag << tag_base << "C_EXP_TOKEN";
        for( int i = 0; i < 128; i++ )
        {
            table << client_message.color_var[n].exp_token[i] << ",";
        }
        m_name_list_file.setLine( tag.str(), table.str() );
        tag.str("");  tag.clear(std::stringstream::goodbit);
        table.str("");  table.clear(std::stringstream::goodbit);
    std::cout << __FUNCTION__  << ": " << __LINE__ << std::endl;

        //2019 kawamura
        tag << tag_base<< "C_VAR_NAME";
        for( int i = 0; i < 128; i++ )
        {
            table << client_message.color_var[n].var_name[i] << ",";
        }
        m_name_list_file.setLine( tag.str(), table.str() );
        tag.str("");  tag.clear(std::stringstream::goodbit);
        table.str("");  table.clear(std::stringstream::goodbit);

    std::cout << __FUNCTION__  << ": " << __LINE__ << std::endl;
        //2019 kawamura
        tag << tag_base<< "C_VAL_ARRAY";
        for( int i = 0; i < 128; i++ )
        {
            table << client_message.color_var[n].value_array[i] << ",";
        }
        m_name_list_file.setLine( tag.str(), table.str() );
        tag.str("");  tag.clear(std::stringstream::goodbit);
        table.str("");  table.clear(std::stringstream::goodbit);
#endif

        m_name_list_file.setLine( tag_base + "MIN_C", client_message.m_transfer_function[n].m_color_variable_min   );
        m_name_list_file.setLine( tag_base + "MAX_C", client_message.m_transfer_function[n].m_color_variable_max   );
        m_name_list_file.setLine( tag_base + "VAR_O", client_message.m_volume_equation[2*n + 1].m_equation   );


#if 0
        //2019 kawamura
        tag << tag_base << "O_EXP_TOKEN";
        for( int i = 0; i < 128; i++ )
        {
            table << client_message.opacity_var[n].exp_token[i] << ",";
        }
        m_name_list_file.setLine( tag.str(), table.str() );
        tag.str("");  tag.clear(std::stringstream::goodbit);
        table.str("");  table.clear(std::stringstream::goodbit);

        //2019 kawamura
        tag << tag_base<< "O_VAR_NAME";
        for( int i = 0; i < 128; i++ )
        {
            table << client_message.opacity_var[n].var_name[i] << ",";
        }
        m_name_list_file.setLine( tag.str(), table.str() );
        tag.str("");  tag.clear(std::stringstream::goodbit);
        table.str("");  table.clear(std::stringstream::goodbit);

        //2019 kawamura
        tag << tag_base<< "O_VAL_ARRAY";
        for( int i = 0; i < 128; i++ )
        {
            table << client_message.opacity_var[n].value_array[i] << ",";
        }
        m_name_list_file.setLine( tag.str(), table.str() );
        tag.str("");  tag.clear(std::stringstream::goodbit);
        table.str("");  table.clear(std::stringstream::goodbit);
#endif

        m_name_list_file.setLine( tag_base + "MIN_O", client_message.m_transfer_function[n].m_opacity_variable_min );
        m_name_list_file.setLine( tag_base + "MAX_O", client_message.m_transfer_function[n].m_opacity_variable_max );

        kvs::ColorMap::Table   color_table   = client_message.m_transfer_function[n].colorMap().table();
        kvs::OpacityMap::Table opacity_table = client_message.m_transfer_function[n].opacityMap().table();

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
}

void ParameterFileWriter::inputGlyphParameterMessage( const jpv::ParticleTransferClientMessage& client_message )
{
//    bool glyph_flag =true;
    
    std::string glyph_flag ="FALSE" ; 
    if (client_message.m_glyph_flag) glyph_flag ="TRUE" ; 

    //m_name_list_file.setLine( "PARTICLE_LIMIT" , client_message.m_particle_limit );
    m_name_list_file.setLine( "GLYPH_FLAG"             , glyph_flag );
    m_name_list_file.setLine( "STRIDE"                 ,client_message.m_stride );
    m_name_list_file.setLine( "SEED"                   ,client_message.m_seed );
    m_name_list_file.setLine( "NUMBER_OF_SMAPLING_POINT"   ,client_message.m_number_of_sampling_point );
    m_name_list_file.setLine( "GLYPH_COLOR_MAX"            ,1 );
    m_name_list_file.setLine( "GLYPH_COLOR_MIN"            ,0 );

    // 各成分を文字列に変換
    std::stringstream  size_variable, color_data_variable;
    for ( size_t i = 0; i < client_message.m_size_variable.size(); i++ )
    {
        size_variable  << client_message.m_size_variable.at( i ) << ",";
    }

    for ( size_t i = 0; i < client_message.m_color_data_variable.size(); i++ )
    {
        color_data_variable  << client_message.m_color_data_variable.at( i ) << ",";
    }

    std::stringstream  direction_variable;
    for ( size_t i = 0; i < 3; i++ )
    {
        direction_variable  << client_message.m_direction_variable[ i ] << ",";
    }


    m_name_list_file.setLine( "SIZE_VARIABLES"         ,size_variable.str() );
    m_name_list_file.setLine( "COLOR_VARIABLES"        ,color_data_variable.str() );
    m_name_list_file.setLine( "DIRECTION_VARIABLES"    ,direction_variable.str());

    std::string distribution_mode;
    if      (client_message.m_distribution_mode == jpv::GlyphMode::AllPoints ) distribution_mode = "AllPoints"; 
    else if (client_message.m_distribution_mode == jpv::GlyphMode::EveryNthPoints ) distribution_mode = "EveryNthPoints"; 
    else if (client_message.m_distribution_mode == jpv::GlyphMode::UniformDistribution ) distribution_mode  = "UniformDistribution"; 

    std::string size_sampling_method;
    if(client_message. m_size_sampling_method == jpv::DataDefines::Constant ) size_sampling_method = "Constant"; 
    else if(client_message. m_size_sampling_method == jpv::DataDefines::SingleVariable ) size_sampling_method = "SingleVariable"; 
    else if(client_message. m_size_sampling_method == jpv::DataDefines::VariableArray ) size_sampling_method  = "VariableArray"; 

    std::string color_sampling_method;
    if(client_message. m_color_data_sampling_method == jpv::DataDefines::Constant ) color_sampling_method = "Constant"; 
    else if(client_message. m_color_data_sampling_method == jpv::DataDefines::SingleVariable ) color_sampling_method = "SingleVariable"; 
    else if(client_message. m_color_data_sampling_method == jpv::DataDefines::VariableArray ) color_sampling_method  = "VariableArray"; 

    m_name_list_file.setLine( "DISTRIBUTION_MODE"      ,distribution_mode );
    m_name_list_file.setLine( "SIZE_SAMPLING_METHOD"   ,size_sampling_method );
    m_name_list_file.setLine( "COLOR_DATA_SAMPLING_METHOD" ,color_sampling_method );

    std::stringstream table;

//    kvs::ColorMap::Table   color_table   = client_message.m_glyph_color_map.table();
//    std::vector<int> color_table  = client_message.m_glyph_color_map;   
//    std::cout << "color_table.size() = " << color_table.size() <<std::endl;
    for ( size_t i = 0; i < client_message.m_glyph_color_map_table.size(); i++ )
    {
        table << client_message.m_glyph_color_map_table.at( i ) << ",";
    }
    m_name_list_file.setLine( "GLYPH_COLOR_MAP_TABLE"      ,table.str() );
}

void ParameterFileWriter::inputPlotOverLineParameterMessage( const jpv::ParticleTransferClientMessage& client_message )
{
    
    std::string plot_flag ="FALSE" ; 
    //if (client_message.m_plot_flag) plot_flag ="TRUE" ; 

    int sam = 256;
    m_name_list_file.setLine( "PLOT_FLAG"             ,plot_flag );
//    m_name_list_file.setLine( "SAMPLING_SIZE"         ,client_message.m_sampling_size );
    m_name_list_file.setLine( "SAMPLING_SIZE"         ,sam );

    // 各成分を文字列に変換
    std::stringstream  start_point, end_point;
    for ( size_t i = 0; i < 3; i++ )
    {
        //start_point  << client_message.m_start_point[i] << ",";
        //end_point    << client_message.m_end_point[i] << ",";
        start_point  << 1.2 << ",";
        end_point    << 1.2 << ",";
    }

    m_name_list_file.setLine( "START_POINT"   ,start_point.str() );
    m_name_list_file.setLine( "END_POINT"     ,end_point.str() );

}

void ParameterFileWriter::writeParameterFile( const char* fname )
{
    m_name_list_file.setFileName( std::string( fname ) );
    m_name_list_file.write();

    std::ofstream ofs;
    ofs.open( fname , std::ios::out | std::ios::app  );

    ofs << "END_PARAMETER_FILE=SUCCESS" << std::endl;

    ofs.close();
}


const NameListFile& ParameterFileWriter::getNameListFile() const
{
    return m_name_list_file ;

}
