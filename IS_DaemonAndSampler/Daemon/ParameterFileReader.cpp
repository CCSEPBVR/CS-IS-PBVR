#include "ParameterFileReader.h"
#define DEFAULT_TF_NUMBER 5
#define BEFORE_READ_TF_NUMBER 99


void ParameterFileReader::outputMessage( jpv::ParticleTransferServerMessage* server_message )
{
    this->outputParameterMessage( server_message );
    this->outputTransferFunctionMessage( server_message );
}

void ParameterFileReader::outputParameterMessage( jpv::ParticleTransferServerMessage* server_message )
{
    server_message->particle_limit         =  m_name_list_file.getValue<int32_t>( "PARTICLE_LIMIT"    );
    server_message->particle_density       =  m_name_list_file.getValue<float>(   "PARTICLE_DENSITY"  );
    size_t width                           =  m_name_list_file.getValue<size_t>(  "RESOLUTION_WIDTH"  );
    size_t height                          =  m_name_list_file.getValue<size_t>(  "RESOLUTION_HEIGHT" );
    server_message->camera->setWindowSize( width, height );
    server_message->particle_data_size_limit =  m_name_list_file.getValue<float>(   "PARTICLE_DATA_SIZE_LIMIT"  );
}

void ParameterFileReader::outputTransferFunctionMessage( jpv::ParticleTransferServerMessage* server_message )
{
    const size_t resolution                   = m_name_list_file.getValue<int>(         "TF_RESOLUTION" );
    //server_message->transferFunctionSynthesis  = m_name_list_file.getValue<std::string>( "TF_SYNTH"      );
    server_message->color_tf_synthesis = m_name_list_file.getValue<std::string>( "COLOR_SYNTH" );
    server_message->opacity_tf_synthesis = m_name_list_file.getValue<std::string>( "OPACITY_SYNTH" );

    const int cur_tf_number = m_name_list_file.getValue<int>("TF_NUMBER");
    server_message->transfunc.resize( cur_tf_number );
    for ( size_t n = 0; n < cur_tf_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";

        std::stringstream s_name;
        s_name << "t" << n + 1;

        const std::string tag_base = ss.str();
        server_message->transfunc[n].setResolution( resolution );
        server_message->transfunc[n].Name          = s_name.str();
        server_message->transfunc[n].ColorVar      = m_name_list_file.getValue<std::string>( tag_base + "VAR_C" );
        server_message->transfunc[n].ColorVarMin   = m_name_list_file.getValue<float>( tag_base + "MIN_C" );
        server_message->transfunc[n].ColorVarMax   = m_name_list_file.getValue<float>( tag_base + "MAX_C" );
        server_message->transfunc[n].OpacityVar    = m_name_list_file.getValue<std::string>( tag_base + "VAR_O" );
        server_message->transfunc[n].OpacityVarMin = m_name_list_file.getValue<float>( tag_base + "MIN_O" );
        server_message->transfunc[n].OpacityVarMax = m_name_list_file.getValue<float>( tag_base + "MAX_O" );

        std::string s_color =   m_name_list_file.getValue<std::string>( tag_base + "TABLE_C" );
        std::string s_opacity = m_name_list_file.getValue<std::string>( tag_base + "TABLE_O" );

        std::replace( s_color.begin(), s_color.end(), ',', ' ' );
        std::replace( s_opacity.begin(), s_opacity.end(), ',', ' ' );

        std::stringstream ss_color( s_color );
        std::stringstream ss_opacity( s_opacity );

        kvs::ColorMap::Table color_table( resolution * 3 );
        kvs::OpacityMap::Table opacity_table( resolution );

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

        kvs::ColorMap color_map( color_table );
        kvs::OpacityMap opacity_map( opacity_table );
        kvs::TransferFunction tf( color_map, opacity_map );
        kvs::TransferFunction& tt = server_message->transfunc[n];
        tt = tf;
    }

}

void ParameterFileReader::readParameterFile( const char* fname )
{
    m_name_list_file.setName( "PARTICLE_LIMIT"    );
    m_name_list_file.setName( "PARTICLE_DENSITY"  );
    m_name_list_file.setName( "PARTICLE_DATA_SIZE_LIMIT"  );
    m_name_list_file.setName( "RESOLUTION_WIDTH"  );
    m_name_list_file.setName( "RESOLUTION_HEIGHT" );

    m_name_list_file.setName( "TF_RESOLUTION" );
    //m_name_list_file.setName( "TF_SYNTH"      );
    m_name_list_file.setName( "COLOR_SYNTH" );
    m_name_list_file.setName( "OPACITY_SYNTH" );
    m_name_list_file.setName( "TF_NUMBER" );

    m_name_list_file.setName( "OPA_FUNC_EXP_TOKEN");
    m_name_list_file.setName( "OPA_FUNC_VAR_NAME");
    m_name_list_file.setName( "OPA_FUNC_VAL_ARRAY");
    m_name_list_file.setName( "COL_FUNC_EXP_TOKEN");
    m_name_list_file.setName( "COL_FUNC_VAR_NAME");
    m_name_list_file.setName( "COL_FUNC_VAL_ARRAY");


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

        m_name_list_file.setName( tag_base + "O_EXP_TOKEN" );
        m_name_list_file.setName( tag_base + "O_VAR_NAME" );
        m_name_list_file.setName( tag_base + "O_VAL_ARRAY" );
        m_name_list_file.setName( tag_base + "C_EXP_TOKEN" );
        m_name_list_file.setName( tag_base + "C_VAR_NAME" );
        m_name_list_file.setName( tag_base + "C_VAL_ARRAY" );
    }

    m_name_list_file.setFileName( std::string( fname ) );
    if( !m_name_list_file.read() )
    {
        this->set_default_parameter();
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

            m_name_list_file.deleteLine( tag_base + "O_EXP_TOKEN" );
            m_name_list_file.deleteLine( tag_base + "O_VAR_NAME" );
            m_name_list_file.deleteLine( tag_base + "O_VAL_ARRAY" );
            m_name_list_file.deleteLine( tag_base + "C_EXP_TOKEN" );
            m_name_list_file.deleteLine( tag_base + "C_VAR_NAME" );
            m_name_list_file.deleteLine( tag_base + "C_VAL_ARRAY" );
        }
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
 
    kvs::TransferFunction tf;
    kvs::ColorMap::Table   color_table   = tf.colorMap().table();
    kvs::OpacityMap::Table opacity_table = tf.opacityMap().table();
    
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
}

const NameListFile& ParameterFileReader::getNameListFile() const
{
    return m_name_list_file;
}
