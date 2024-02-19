#include "ParameterFileWriter.h"

void ParameterFileWriter::inputMessage( const jpv::ParticleTransferClientMessage& client_message )
{
    this->inputParameterMessage( client_message );
    this->inputTransferFunctionMessage( client_message );
}

void ParameterFileWriter::inputParameterMessage( const jpv::ParticleTransferClientMessage& client_message )
{
//  m_name_list_file.setLine( "SUB_PIXEL_LEVEL", client_message.subPixelLevel  );
    m_name_list_file.setLine( "PARTICLE_LIMIT" , client_message.particle_limit );
    m_name_list_file.setLine( "PARTICLE_DENSITY" ,client_message.particle_density );
    m_name_list_file.setLine( "PARTICLE_DATA_SIZE_LIMIT" ,client_message.particle_data_size_limit );
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
//  m_name_list_file.setLine( "EYE_POINT_X"      , client_message.camera->lookAt().x() );
//  m_name_list_file.setLine( "EYE_POINT_Y"      , client_message.camera->lookAt().y() );
//  m_name_list_file.setLine( "EYE_POINT_Z"      , client_message.camera->lookAt().z() );
//  m_name_list_file.setLine( "CENTER_POINT_X"   , client_message.camera->position().x() );
//  m_name_list_file.setLine( "CENTER_POINT_Y"   , client_message.camera->position().y() );
//  m_name_list_file.setLine( "CENTER_POINT_Z"   , client_message.camera->position().z() );
//  m_name_list_file.setLine( "UP_VECTOR_X"      , client_message.camera->upVector().x() );
//  m_name_list_file.setLine( "UP_VECTOR_Y"      , client_message.camera->upVector().y() );
//  m_name_list_file.setLine( "UP_VECTOR_Z"      , client_message.camera->upVector().z() );
    m_name_list_file.setLine( "RESOLUTION_WIDTH" , client_message.camera->windowWidth() );
    m_name_list_file.setLine( "RESOLUTION_HEIGHT", client_message.camera->windowHeight() );
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

//  m_name_list_file.setLine( "ROTATION1X"       , client_message.camera->rotation()[0][0] );
//  m_name_list_file.setLine( "ROTATION1Y"       , client_message.camera->rotation()[0][1] );
//  m_name_list_file.setLine( "ROTATION1Z"       , client_message.camera->rotation()[0][2] );
//  m_name_list_file.setLine( "ROTATION2X"       , client_message.camera->rotation()[1][0] );
//  m_name_list_file.setLine( "ROTATION2Y"       , client_message.camera->rotation()[1][1] );
//  m_name_list_file.setLine( "ROTATION2Z"       , client_message.camera->rotation()[1][2] );
//  m_name_list_file.setLine( "ROTATION3X"       , client_message.camera->rotation()[2][0] );
//  m_name_list_file.setLine( "ROTATION3Y"       , client_message.camera->rotation()[2][1] );
//  m_name_list_file.setLine( "ROTATION3Z"       , client_message.camera->rotation()[2][2] );

//  m_name_list_file.setLine( "TRANSLATIONX"     , client_message.camera->translation()[0] );
//  m_name_list_file.setLine( "TRANSLATIONY"     , client_message.camera->translation()[1] );
//  m_name_list_file.setLine( "TRANSLATIONZ"     , client_message.camera->translation()[2] );
//  m_name_list_file.setLine( "SCALINGX"         , client_message.camera->scaling()[0] );
//  m_name_list_file.setLine( "SCALINGY"         , client_message.camera->scaling()[1] );
//  m_name_list_file.setLine( "SCALINGZ"         , client_message.camera->scaling()[2] );
}

void ParameterFileWriter::inputTransferFunctionMessage( const jpv::ParticleTransferClientMessage& client_message )
{
    m_name_list_file.setLine( "TF_RESOLUTION", client_message.transfunc[0].Resolution);
    //m_name_list_file.setLine( "TF_SYNTH"     , client_message.transferFunctionSynthesis);
    m_name_list_file.setLine( "COLOR_SYNTH"  , client_message.color_tf_synthesis);
    m_name_list_file.setLine( "OPACITY_SYNTH", client_message.opacity_tf_synthesis);
    m_name_list_file.setLine( "TF_NUMBER"    , (int)(client_message.transfunc.size()) );

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

    for ( size_t n = 0; n < client_message.transfunc.size(); n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";

        const std::string tag_base = ss.str();
        m_name_list_file.setLine( tag_base + "VAR_C", client_message.voleqn[2*n].Equation );

        //2019 kawamura
        tag << tag_base << "C_EXP_TOKEN";
        for( int i = 0; i < 128; i++ )
        {
            table << client_message.color_var[n].exp_token[i] << ",";
        }
        m_name_list_file.setLine( tag.str(), table.str() );
        tag.str("");  tag.clear(std::stringstream::goodbit);
        table.str("");  table.clear(std::stringstream::goodbit);

        //2019 kawamura
        tag << tag_base<< "C_VAR_NAME";
        for( int i = 0; i < 128; i++ )
        {
            table << client_message.color_var[n].var_name[i] << ",";
        }
        m_name_list_file.setLine( tag.str(), table.str() );
        tag.str("");  tag.clear(std::stringstream::goodbit);
        table.str("");  table.clear(std::stringstream::goodbit);

        //2019 kawamura
        tag << tag_base<< "C_VAL_ARRAY";
        for( int i = 0; i < 128; i++ )
        {
            table << client_message.color_var[n].value_array[i] << ",";
        }
        m_name_list_file.setLine( tag.str(), table.str() );
        tag.str("");  tag.clear(std::stringstream::goodbit);
        table.str("");  table.clear(std::stringstream::goodbit);


        m_name_list_file.setLine( tag_base + "MIN_C", client_message.transfunc[n].ColorVarMin   );
        m_name_list_file.setLine( tag_base + "MAX_C", client_message.transfunc[n].ColorVarMax   );
        m_name_list_file.setLine( tag_base + "VAR_O", client_message.voleqn[2*n + 1].Equation   );


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


        m_name_list_file.setLine( tag_base + "MIN_O", client_message.transfunc[n].OpacityVarMin );
        m_name_list_file.setLine( tag_base + "MAX_O", client_message.transfunc[n].OpacityVarMax );

        kvs::ColorMap::Table   color_table   = client_message.transfunc[n].colorMap().table();
        kvs::OpacityMap::Table opacity_table = client_message.transfunc[n].opacityMap().table();

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
