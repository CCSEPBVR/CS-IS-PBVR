#include "ParticleTransferProtocol.h"
#include "serializer.h"

#include <kvs/Camera>
#include <kvs/TransferFunction>

#include <cstring>

// ���g���G���f�B�A�����ł� JKVS
// �r�b�O�G���f�B�A�����ł� SVKJ
const int32_t jpv::ParticleTransferUtils::m_magic_number = 0x53564b4a; // 1398164298;

bool jpv::ParticleTransferUtils::isLittleEndian( void )
{
    char buf[4];
    std::memcpy( buf, reinterpret_cast<const char*>( &jpv::ParticleTransferUtils::m_magic_number ), sizeof( int32_t ) );
    if ( buf[0] == 'J' )
    {
        return true;
    }
    else
    {
        return false;
    }
}

jpv::ParticleTransferClientMessage::ParticleTransferClientMessage( void )
    : m_camera( NULL ), m_transfer_function( NULL )
{
    std::fill( m_header, m_header + sizeof( m_header ), '\0' );
}

int32_t jpv::ParticleTransferClientMessage::byteSize( void ) const
{
    int32_t s = 0;
    s += sizeof( m_header );
    s += sizeof( m_message_size );
    s += sizeof( m_initialize_parameter );
    if ( m_initialize_parameter == -3 )
    {
        s += sizeof( int64_t );
        s += sizeof( char ) * ( m_input_directory.size() + 1 );
    }
    if ( m_initialize_parameter == 1 )
    {
        s += sizeof( m_sampling_method );
        s += sizeof( m_subpixel_level );
        s += sizeof( m_repeat_level );
        s += sizeof( m_shuffle_method );
        s += sizeof( m_node_type );
        s += sizeof( m_rendering_id );
        s += sizeof( m_sampling_step );
        s += sizeof( m_enable_crop_region );
        s += sizeof( m_crop_region[0] ) * 6;
        s += sizeof( m_particle_limit );
        s += sizeof( m_particle_density );
        s += sizeof( particle_data_size_limit );
        s += sizeof( int64_t );
        s += sizeof( char ) * ( m_input_directory.size() + 1 );
        s += jpv::Serializer::byteSize<kvs::Camera>( *m_camera );
        s += sizeof( float ) * 3;  // add by shimomura 20240116
        s += sizeof( bool ) ;
        s += sizeof( float ) * 16;
        s += sizeof( float ) * 16;
    }
    if ( m_initialize_parameter == 1 || m_initialize_parameter == -3 )
    {
        s += jpv::Serializer::byteSize( m_transfer_function.size() );
        for ( size_t i = 0; i < m_transfer_function.size(); i++ )
        {
            s += jpv::Serializer::byteSize( m_transfer_function[i].m_name );
            s += jpv::Serializer::byteSize( m_transfer_function[i].m_color_variable );
            s += jpv::Serializer::byteSize( m_transfer_function[i].m_color_variable_min );
            s += jpv::Serializer::byteSize( m_transfer_function[i].m_color_variable_max );
            s += jpv::Serializer::byteSize( m_transfer_function[i].m_opacity_variable );
            s += jpv::Serializer::byteSize( m_transfer_function[i].m_opacity_variable_min );
            s += jpv::Serializer::byteSize( m_transfer_function[i].m_opacity_variable_max );
            s += jpv::Serializer::byteSize( m_transfer_function[i].m_selection );
            if ( m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
            {
                s += jpv::Serializer::byteSize( m_transfer_function[i].m_resolution );
                s += jpv::Serializer::byteSize( m_transfer_function[i].m_equation_red );
                s += jpv::Serializer::byteSize( m_transfer_function[i].m_equation_green );
                s += jpv::Serializer::byteSize( m_transfer_function[i].m_equation_blue );
                s += jpv::Serializer::byteSize( m_transfer_function[i].m_equation_opacity );
            }
            else if ( m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectTransferFunction )
            {
                s += jpv::Serializer::byteSize<kvs::TransferFunction>( m_transfer_function[i] );
            }
        }
        s += jpv::Serializer::byteSize( m_volume_equation.size() );
        for ( size_t i = 0; i < m_volume_equation.size(); i++ )
        {
            s += jpv::Serializer::byteSize( m_volume_equation[i].m_name );
            s += jpv::Serializer::byteSize( m_volume_equation[i].m_equation );
        }
//        s += jpv::Serializer::byteSize( m_transfer_function_synthesis );
        s += jpv::Serializer::byteSize( color_tf_synthesis );
        s += jpv::Serializer::byteSize( opacity_tf_synthesis );

        //2019 kawamura
        s +=   sizeof(int)*128; //opacity_func
        s +=   sizeof(int)*128; //opacity_func
        s +=   sizeof(float)*128; //opacity_func

        s +=   sizeof(int)*128; //color_func
        s +=   sizeof(int)*128; //color_func
        s +=   sizeof(float)*128; //color_func


        //2019 kawamura
        s += jpv::Serializer::byteSize( opacity_var.size() );
        for( size_t i = 0; i < opacity_var.size(); i++)
        {
            s += sizeof(int)*128;//exp_token
            s += sizeof(int)*128;//var_name
            s += sizeof(float)*128;//value_array
        }

        //2019 kawamura
        s += jpv::Serializer::byteSize( color_var.size() );
        for( size_t i = 0; i < color_var.size(); i++ )
        {
            s += sizeof(int)*128;//exp_token
            s += sizeof(int)*128;//var_name
            s += sizeof(float)*128;//value_array
        }
    }
    if ( m_initialize_parameter >= 0 )
    {
        s += sizeof( m_time_parameter );
        if ( m_time_parameter == 0 )
        {
            s += sizeof( m_memory_size );
        }
        else if ( m_time_parameter == 1 )
        {
            s += sizeof( m_begin_time );
            s += sizeof( m_end_time );
            s += sizeof( m_memory_size );
        }
        else if ( m_time_parameter == 2 )
        {
            s += sizeof( m_step );
        }
        s += sizeof( m_trans_Parameter );
        if ( m_trans_Parameter == 1 )
        {
            s += sizeof( m_level_index );
        }
    }
    return s;
}

size_t jpv::ParticleTransferClientMessage::pack( char* buf ) const
{
    size_t index = 0;
    index += jpv::Serializer::writeArray( buf + index, m_header );
    index += jpv::Serializer::write( buf + index, m_message_size );
    index += jpv::Serializer::write( buf + index, m_initialize_parameter );
    if ( m_initialize_parameter == -3 )
    {
        index += jpv::Serializer::write( buf + index, m_input_directory.size() );
        index += jpv::Serializer::writeArray( buf + index, m_input_directory.c_str(), m_input_directory.size() + 1 );
    }
    if ( m_initialize_parameter == 1 )
    {
        index += jpv::Serializer::write( buf + index, m_sampling_method );
        index += jpv::Serializer::write( buf + index, m_subpixel_level );
        index += jpv::Serializer::write( buf + index, m_repeat_level );
        index += jpv::Serializer::write( buf + index, m_node_type );
        index += jpv::Serializer::write( buf + index, m_rendering_id );
        index += jpv::Serializer::write( buf + index, m_sampling_step );
        index += jpv::Serializer::write( buf + index, m_shuffle_method );
        index += jpv::Serializer::write( buf + index, m_enable_crop_region );
        for ( int i = 0; i < 6; i++ )
        {
            index += jpv::Serializer::write( buf + index, m_crop_region[i] );
        }
        index += jpv::Serializer::write( buf + index, m_particle_limit );
        index += jpv::Serializer::write( buf + index, m_particle_density );
        index += jpv::Serializer::write( buf + index, particle_data_size_limit );
        index += jpv::Serializer::write( buf + index, m_input_directory.size() );
        index += jpv::Serializer::writeArray( buf + index, m_input_directory.c_str(), m_input_directory.size() + 1 );

        index += jpv::Serializer::pack( buf + index, *m_camera );
        index += jpv::Serializer::write( buf + index, m_arrow_component[0] ); //add by shimomura 20240116
        index += jpv::Serializer::write( buf + index, m_arrow_component[1] );
        index += jpv::Serializer::write( buf + index, m_arrow_component[2] );
        // index += jpv::Serializer::write( buf + index, m_arrow_component[3] );
        // index += jpv::Serializer::write( buf + index, m_arrow_component[4] );
        // index += jpv::Serializer::write( buf + index, m_arrow_component[5] );
        index += jpv::Serializer::write( buf + index, m_arrow_flag );
        for (int i = 0;i<16 ; i++)
        {
            index += jpv::Serializer::write( buf + index, m_glyph_xform[i] );
        }
        for (int i = 0;i<16 ; i++)
        {
            index += jpv::Serializer::write( buf + index, m_obj_xform[i] );
        }
    }
    if ( m_initialize_parameter == 1 || m_initialize_parameter == -3 )
    {
        index += jpv::Serializer::write( buf + index, m_transfer_function.size() );
        for ( size_t i = 0; i < m_transfer_function.size(); i++ )
        {
            index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_name );
            index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_color_variable );
            index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_color_variable_min );
            index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_color_variable_max );
            index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_opacity_variable );
            index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_opacity_variable_min );
            index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_opacity_variable_max );
            index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_selection );
            if ( m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
            {
                index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_resolution );
                index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_equation_red );
                index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_equation_green );
                index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_equation_blue );
                index += jpv::Serializer::write( buf + index, m_transfer_function[i].m_equation_opacity );
            }
            else if ( m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectTransferFunction )
            {
                index += jpv::Serializer::pack<kvs::TransferFunction>( buf + index, m_transfer_function[i] );
            }
        }
        index += jpv::Serializer::write( buf + index, m_volume_equation.size() );
        for ( size_t i = 0; i < m_volume_equation.size(); i++ )
        {
            index += jpv::Serializer::write( buf + index, m_volume_equation[i].m_name );
            index += jpv::Serializer::write( buf + index, m_volume_equation[i].m_equation );
        }
//        index += jpv::Serializer::write( buf + index, m_transfer_function_synthesis );
        index += jpv::Serializer::write( buf + index, color_tf_synthesis );
        index += jpv::Serializer::write( buf + index, opacity_tf_synthesis );

        //2019 kawamura
        index +=  jpv::Serializer::writeArray( buf + index, opacity_func.exp_token );
        index +=  jpv::Serializer::writeArray( buf + index, opacity_func.var_name );
        index +=  jpv::Serializer::writeArray( buf + index, opacity_func.value_array );

        index +=  jpv::Serializer::writeArray( buf + index, color_func.exp_token );
        index +=  jpv::Serializer::writeArray( buf + index, color_func.var_name );
        index +=  jpv::Serializer::writeArray( buf + index, color_func.value_array );


        //2019 kawamura
        index +=  jpv::Serializer::write( buf + index, opacity_var.size() );
        for( size_t i = 0; i < opacity_var.size(); i++ )
        {
            index +=  jpv::Serializer::writeArray( buf + index, opacity_var[i].exp_token );
            index +=  jpv::Serializer::writeArray( buf + index, opacity_var[i].var_name );
            index +=  jpv::Serializer::writeArray( buf + index, opacity_var[i].value_array );
        }

        //2019 kawamura
        index +=  jpv::Serializer::write( buf + index, color_var.size() );
        for( size_t i = 0; i < color_var.size(); i++ )
        {
            index +=  jpv::Serializer::writeArray( buf + index, color_var[i].exp_token );
            index +=  jpv::Serializer::writeArray( buf + index, color_var[i].var_name );
            index +=  jpv::Serializer::writeArray( buf + index, color_var[i].value_array );
        }
    }
    if ( m_initialize_parameter >= 0 )
    {
        index += jpv::Serializer::write( buf + index, m_time_parameter );
        if ( m_time_parameter == 0 )
        {
            index += jpv::Serializer::write( buf + index, m_memory_size );
        }
        else if ( m_time_parameter == 1 )
        {
            index += jpv::Serializer::write( buf + index, m_begin_time );
            index += jpv::Serializer::write( buf + index, m_end_time );
            index += jpv::Serializer::write( buf + index, m_memory_size );
        }
        else if ( m_time_parameter == 2 )
        {
            index += jpv::Serializer::write( buf + index, m_step );
        }
        index += jpv::Serializer::write( buf + index, m_trans_Parameter );
        if ( m_trans_Parameter == 1 )
        {
            index += jpv::Serializer::write( buf + index, m_level_index );
        }
    }
    return index;
}

size_t jpv::ParticleTransferClientMessage::unpack( const char* buf )
{
    size_t index = 0;
    int64_t tmp_char_size;
    char*  tmp_char = NULL;
    int64_t tmp_size[6];
    char*  tmp_char_array[6];

    index += jpv::Serializer::readArray( buf + index, m_header );
    if ( strncmp( "JPTP /1.0", m_header, 9 ) != 0 )
    {
        return 0;
    }
    index += jpv::Serializer::read( buf + index, m_message_size );
    index += jpv::Serializer::read( buf + index, m_initialize_parameter );
    if ( m_initialize_parameter == -3 )
    {
        index += jpv::Serializer::read( buf + index, tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_input_directory = std::string( tmp_char );
        delete[] tmp_char;
    }
    if ( m_initialize_parameter == 1 )
    {
        index += jpv::Serializer::read( buf + index, m_sampling_method );
        index += jpv::Serializer::read( buf + index, m_subpixel_level );
        index += jpv::Serializer::read( buf + index, m_repeat_level );
        index += jpv::Serializer::read( buf + index, m_node_type );
        index += jpv::Serializer::read( buf + index, m_rendering_id );
        index += jpv::Serializer::read( buf + index, m_sampling_step );
        index += jpv::Serializer::read( buf + index, m_shuffle_method );
        index += jpv::Serializer::read( buf + index, m_enable_crop_region );
        for ( int i = 0; i < 6; i++ )
        {
            index += jpv::Serializer::read( buf + index, m_crop_region[i] );
        }
        index += jpv::Serializer::read( buf + index, m_particle_limit );
        index += jpv::Serializer::read( buf + index, m_particle_density );
        index += jpv::Serializer::read( buf + index, particle_data_size_limit );
        index += jpv::Serializer::read( buf + index, tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_input_directory = std::string( tmp_char );
        delete[] tmp_char;

        index += jpv::Serializer::unpack( buf + index, *m_camera );
        index += jpv::Serializer::read( buf + index, m_arrow_component[0] ); //add by shimomura 20240116
        index += jpv::Serializer::read( buf + index, m_arrow_component[1] );
        index += jpv::Serializer::read( buf + index, m_arrow_component[2] );
        // index += jpv::Serializer::read( buf + index, m_arrow_component[3] );
        // index += jpv::Serializer::read( buf + index, m_arrow_component[4] );
        // index += jpv::Serializer::read( buf + index, m_arrow_component[5] );
        index += jpv::Serializer::read( buf + index, m_arrow_flag);
        for (int i = 0;i<16 ; i++)
        {
            index += jpv::Serializer::read( buf + index, m_glyph_xform[i] );
        }
        for (int i = 0;i<16 ; i++)
        {
            index += jpv::Serializer::read( buf + index, m_obj_xform[i] );
        }
    }
    if ( m_initialize_parameter == 1 || m_initialize_parameter == -3 )
    {
        size_t s;
        index += jpv::Serializer::read( buf + index, s );
        m_transfer_function.clear();
        for ( size_t i = 0; i < s; i++ )
        {
            m_transfer_function.push_back( NamedTransferFunctionParameter() );
            index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_name );
            index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_color_variable );
            index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_color_variable_min );
            index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_color_variable_max );
            index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_opacity_variable );
            index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_opacity_variable_min );
            index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_opacity_variable_max );
            index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_selection );
            if ( m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
            {
                index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_resolution );
                index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_equation_red );
                index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_equation_green );
                index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_equation_blue );
                index += jpv::Serializer::read( buf + index, m_transfer_function[i].m_equation_opacity );
            }
            else if ( m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectTransferFunction )
            {
                index += jpv::Serializer::unpack<kvs::TransferFunction>( buf + index, m_transfer_function[i] );
            }
        }
        index += jpv::Serializer::read( buf + index, s );
        m_volume_equation.clear();
        for ( size_t i = 0; i < s; i++ )
        {
            m_volume_equation.push_back( VolumeEquation() );
            index += jpv::Serializer::read( buf + index, m_volume_equation[i].m_name );
            index += jpv::Serializer::read( buf + index, m_volume_equation[i].m_equation );
        }
//        index += jpv::Serializer::read( buf + index, m_transfer_function_synthesis );
        index += jpv::Serializer::read( buf + index, color_tf_synthesis );
        index += jpv::Serializer::read( buf + index, opacity_tf_synthesis );

        //2019 kawamura
        index +=  jpv::Serializer::readArray( buf + index, opacity_func.exp_token, 128 );
        index +=  jpv::Serializer::readArray( buf + index, opacity_func.var_name, 128 );
        index +=  jpv::Serializer::readArray( buf + index, opacity_func.value_array, 128 );

        index +=  jpv::Serializer::readArray( buf + index, color_func.exp_token, 128 );
        index +=  jpv::Serializer::readArray( buf + index, color_func.var_name, 128 );
        index +=  jpv::Serializer::readArray( buf + index, color_func.value_array, 128 );

        //2019 kawamura
        EquationToken tmp_token;
        size_t size;
        index +=  jpv::Serializer::read( buf + index, size );
        opacity_var.clear();
        for( size_t i = 0; i < size; i++ )
        {
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.exp_token, 128 );
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.var_name, 128 );
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.value_array, 128 );
            opacity_var.push_back( tmp_token );
        }

        //2019 kawamura
        index +=  jpv::Serializer::read( buf + index, size );
        color_var.clear();
        for( size_t i = 0; i < size; i++ )
        {
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.exp_token, 128 );
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.var_name, 128 );
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.value_array, 128 );
            color_var.push_back( tmp_token );
        }
    }
    if ( m_initialize_parameter >= 0 )
    {
        index += jpv::Serializer::read( buf + index, m_time_parameter );
        if ( m_time_parameter == 0 )
        {
            index += jpv::Serializer::read( buf + index, m_memory_size );
        }
        else if ( m_time_parameter == 1 )
        {
            index += jpv::Serializer::read( buf + index, m_begin_time );
            index += jpv::Serializer::read( buf + index, m_end_time );
            index += jpv::Serializer::read( buf + index, m_memory_size );
        }
        else if ( m_time_parameter == 2 )
        {
            index += jpv::Serializer::read( buf + index, m_step );
        }
        index += jpv::Serializer::read( buf + index, m_trans_Parameter );
        if ( m_trans_Parameter == 1 )
        {
            index += jpv::Serializer::read( buf + index, m_level_index );
        }
    }

    return index;
}

//2019 kawamura
void jpv::ParticleTransferClientMessage::show( void ) const
{
    std::cout<<"SHOW CLIENT MESSAGE"<<std::endl;

    std::cout<<"header="<<m_header<<std::endl;
    std::cout<<"messageSize="<<m_message_size<<std::endl;
    std::cout<<"initParam="<<m_initialize_parameter<<std::endl;
    std::cout<<"timeParam="<<m_time_parameter<<std::endl;
    std::cout<<"transParam="<<m_trans_Parameter<<std::endl;

    std::cout<<"m_arrow_component = (" << m_arrow_component[0] << ", " << m_arrow_component[1] << ", " << m_arrow_component[2] << ", " << /* m_arrow_component[3]
              << ", " << m_arrow_component[4] << ", " << m_arrow_component[5] << */ " )"<<std::endl;


    std::cout<<"transfunc.size="<<m_transfer_function.size()<<std::endl;
    std::cout<<"transfunc.Name,ColorVar,OpacityVar,ColorVarMin,ColorVarMax"<<std::endl;
    for(int i=0; i<m_transfer_function.size(); i++)
    {
        std::cout<<m_transfer_function[i].m_name<<","<<m_transfer_function[i].m_color_variable<<","<<m_transfer_function[i].m_opacity_variable<<","<<m_transfer_function[i].m_color_variable_min<<","<<m_transfer_function[i].m_color_variable_max<<std::endl;
    }

    std::cout<<"voleqn.size="<<m_volume_equation.size()<<std::endl;
    std::cout<<"voleqn.Name,Equation"<<std::endl;
    for(int i=0; i<m_volume_equation.size(); i++)
    {
        std::cout<<m_volume_equation[i].m_name<<","<<m_volume_equation[i].m_equation<<std::endl;
    }

    //std::cout<<"transferFunctionSynthesis="<<transferFunctionSynthesis<<std::endl;
    std::cout << "color_tf_synthesis=" << color_tf_synthesis << std::endl;
    std::cout << "opacity_tf_synthesis=" << opacity_tf_synthesis << std::endl;

    std::cout<<"opacity_func"<<std::endl;
    std::cout<<"exp_token";
    for(int i=0; i<20; i++)
    {
        std::cout<<opacity_func.exp_token[i]<<",";
    }std::cout<<std::endl;
    std::cout<<"var_name";
    for(int i=0; i<20; i++)
    {
        std::cout<<opacity_func.var_name[i]<<",";
    }std::cout<<std::endl;
    std::cout<<"value_array";
    for(int i=0; i<20; i++)
    {
        std::cout<<opacity_func.value_array[i]<<",";
    }std::cout<<std::endl;

    std::cout<<"color_func"<<std::endl;
    std::cout<<"exp_token";
    for(int i=0; i<20; i++)
    {
        std::cout<<color_func.exp_token[i]<<",";
    }std::cout<<std::endl;
    std::cout<<"var_name";
    for(int i=0; i<20; i++)
    {
        std::cout<<color_func.var_name[i]<<",";
    }std::cout<<std::endl;
    std::cout<<"value_array";
    for(int i=0; i<20; i++)
    {
        std::cout<<color_func.value_array[i]<<",";
    }std::cout<<std::endl;

    std::cout<<"opacity_var.size="<<opacity_var.size()<<std::endl;
    for(int j=0; j<opacity_var.size(); j++)
    {
        std::cout<<"exp_token";
        for(int i=0; i<20; i++)
        {
            std::cout<<opacity_var[j].exp_token[i]<<",";
        }std::cout<<std::endl;
        std::cout<<"var_name";
        for(int i=0; i<20; i++)
        {
            std::cout<<opacity_var[j].var_name[i]<<",";
        }std::cout<<std::endl;
        std::cout<<"value_array";
        for(int i=0; i<20; i++)
        {
            std::cout<<opacity_var[j].value_array[i]<<",";
        }std::cout<<std::endl;
    }

    std::cout<<"color_var.size="<<color_var.size()<<std::endl;
    for(int j=0; j<color_var.size(); j++)
    {
        std::cout<<"exp_token";
        for(int i=0; i<20; i++)
        {
            std::cout<<color_var[j].exp_token[i]<<",";
        }std::cout<<std::endl;
        std::cout<<"var_name";
        for(int i=0; i<20; i++)
        {
            std::cout<<color_var[j].var_name[i]<<",";
        }std::cout<<std::endl;
        std::cout<<"value_array";
        for(int i=0; i<20; i++)
        {
            std::cout<<color_var[j].value_array[i]<<",";
        }std::cout<<std::endl;
    }

    std::cout<<std::endl;
}

jpv::ParticleTransferServerMessage::ParticleTransferServerMessage( void ):
    camera( NULL )
{
    std::fill( m_header, m_header + sizeof( m_header ), '\0' );
}

int32_t jpv::ParticleTransferServerMessage::byteSize( void ) const
{
    int32_t s = 0;
    s += sizeof( m_header );
    s += sizeof( m_message_size );
    s += sizeof( m_time_step );
    s += sizeof( m_subpixel_level );
    s += sizeof( m_repeat_level );
    s += sizeof( m_level_index );
    s += sizeof( m_number_particle );
    s += sizeof( m_number_volume_divide );
    s += sizeof( m_start_step );
    s += sizeof( m_end_step );
    s += sizeof( m_number_step );
    s += sizeof( m_min_object_coord[0] ) * 3;
    s += sizeof( m_max_object_coord[0] ) * 3;
    s += sizeof( m_min_value );
    s += sizeof( m_max_value );
    s += sizeof( m_number_nodes );
    s += sizeof( m_number_elements );
    s += sizeof( m_element_type );
    s += sizeof( m_file_type );
    s += sizeof( m_number_ingredients );
    s += m_variable_range.byteSize();
    s += sizeof( m_flag_send_bins );
    s += sizeof( particle_limit );
    s += sizeof( particle_density );
    s += sizeof( particle_data_size_limit );
    s += jpv::Serializer::byteSize<kvs::Camera>( *camera );
    s += sizeof( m_transfer_function_count );
    s += sizeof( float ) * 3;  // add by shimomura 20240116
    s += sizeof( bool );
    s += sizeof( float ) * 16;
    s += sizeof( float ) * 16;

    for ( int i = 0; i < m_transfer_function_count; i++ )
    {
        s += sizeof( m_color_nbins[i] );
        s += sizeof( m_color_bins[i] ) * m_color_nbins[i];
        s += sizeof( m_opacity_bins_number[i] );
        s += sizeof( m_opacity_bins[i] ) * m_opacity_bins_number[i];
    }

    s += jpv::Serializer::byteSize( transfunc.size() );
    for ( size_t i = 0; i < transfunc.size(); i++ )
    {
        s += jpv::Serializer::byteSize( transfunc[i].m_name );
        s += jpv::Serializer::byteSize( transfunc[i].m_color_variable );
        s += jpv::Serializer::byteSize( transfunc[i].m_color_variable_min );
        s += jpv::Serializer::byteSize( transfunc[i].m_color_variable_max );
        s += jpv::Serializer::byteSize( transfunc[i].m_opacity_variable );
        s += jpv::Serializer::byteSize( transfunc[i].m_opacity_variable_min );
        s += jpv::Serializer::byteSize( transfunc[i].m_opacity_variable_max );
        s += jpv::Serializer::byteSize( transfunc[i].m_selection );
        if ( transfunc[i].m_selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            s += jpv::Serializer::byteSize( transfunc[i].m_resolution );
            s += jpv::Serializer::byteSize( transfunc[i].m_equation_red );
            s += jpv::Serializer::byteSize( transfunc[i].m_equation_green );
            s += jpv::Serializer::byteSize( transfunc[i].m_equation_blue );
            s += jpv::Serializer::byteSize( transfunc[i].m_equation_opacity );
        }
        else if ( transfunc[i].m_selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            s += jpv::Serializer::byteSize<kvs::TransferFunction>( transfunc[i] );
        }
    }
    s += jpv::Serializer::byteSize( voleqn.size() );
    for ( size_t i = 0; i < voleqn.size(); i++ )
    {
        s += jpv::Serializer::byteSize( voleqn[i].m_name );
        s += jpv::Serializer::byteSize( voleqn[i].m_equation );
    }
//    s += jpv::Serializer::byteSize( transferFunctionSynthesis );
    s += jpv::Serializer::byteSize( color_tf_synthesis );
    s += jpv::Serializer::byteSize( opacity_tf_synthesis );
    return s;
}

size_t jpv::ParticleTransferServerMessage::pack( char* buf ) const
{
    size_t index = 0;
    index += jpv::Serializer::writeArray( buf + index, m_header );
    index += jpv::Serializer::write( buf + index, m_message_size );
    index += jpv::Serializer::write( buf + index, m_time_step );
    index += jpv::Serializer::write( buf + index, m_subpixel_level );
    index += jpv::Serializer::write( buf + index, m_repeat_level );
    index += jpv::Serializer::write( buf + index, m_level_index );
    index += jpv::Serializer::write( buf + index, m_number_particle );
    index += jpv::Serializer::write( buf + index, m_number_volume_divide );
    index += jpv::Serializer::write( buf + index, m_start_step );
    index += jpv::Serializer::write( buf + index, m_end_step );
    index += jpv::Serializer::write( buf + index, m_number_step );
    index += jpv::Serializer::write( buf + index, m_min_object_coord[0] );
    index += jpv::Serializer::write( buf + index, m_min_object_coord[1] );
    index += jpv::Serializer::write( buf + index, m_min_object_coord[2] );
    index += jpv::Serializer::write( buf + index, m_max_object_coord[0] );
    index += jpv::Serializer::write( buf + index, m_max_object_coord[1] );
    index += jpv::Serializer::write( buf + index, m_max_object_coord[2] );
    index += jpv::Serializer::write( buf + index, m_min_value );
    index += jpv::Serializer::write( buf + index, m_max_value );
    index += jpv::Serializer::write( buf + index, m_number_nodes );
    index += jpv::Serializer::write( buf + index, m_number_elements );
    index += jpv::Serializer::write( buf + index, m_element_type );
    index += jpv::Serializer::write( buf + index, m_file_type );
    index += jpv::Serializer::write( buf + index, m_number_ingredients );
    index += m_variable_range.pack( buf + index );
    index += jpv::Serializer::write( buf + index, m_flag_send_bins );
    index += jpv::Serializer::write( buf + index, particle_limit );
    index += jpv::Serializer::write( buf + index, particle_density );
    index += jpv::Serializer::write( buf + index, particle_data_size_limit );
    index += jpv::Serializer::pack( buf + index, *camera );
    index += jpv::Serializer::write( buf + index, m_transfer_function_count );
    index += jpv::Serializer::write( buf + index, m_arrow_component[0] );
    index += jpv::Serializer::write( buf + index, m_arrow_component[1] );
    index += jpv::Serializer::write( buf + index, m_arrow_component[2] );
    // index += jpv::Serializer::write( buf + index, m_arrow_component[3] );
    // index += jpv::Serializer::write( buf + index, m_arrow_component[4] );
    // index += jpv::Serializer::write( buf + index, m_arrow_component[5] );
    index += jpv::Serializer::write( buf + index, m_arrow_flag );
    for (int i = 0;i<16 ; i++)
    {
        index += jpv::Serializer::write( buf + index, m_glyph_xform[i] );
    }
    for (int i = 0;i<16 ; i++)
    {
        index += jpv::Serializer::write( buf + index, m_obj_xform[i] );
    }

    for ( int i = 0; i < m_transfer_function_count; i++ )
    {
        index += jpv::Serializer::write( buf + index, m_color_nbins[i] );
        index += jpv::Serializer::writeArray<kvs::UInt64>( buf + index, m_color_bins[i], m_color_nbins[i] );
        index += jpv::Serializer::write( buf + index, m_opacity_bins_number[i] );
        index += jpv::Serializer::writeArray<kvs::UInt64>( buf + index, m_opacity_bins[i], m_opacity_bins_number[i] );
    }

    index += jpv::Serializer::write( buf + index, transfunc.size() );
    for ( size_t i = 0; i < transfunc.size(); i++ )
    {
        index += jpv::Serializer::write( buf + index, transfunc[i].m_name );
        index += jpv::Serializer::write( buf + index, transfunc[i].m_color_variable );
        index += jpv::Serializer::write( buf + index, transfunc[i].m_color_variable_min );
        index += jpv::Serializer::write( buf + index, transfunc[i].m_color_variable_max );
        index += jpv::Serializer::write( buf + index, transfunc[i].m_opacity_variable );
        index += jpv::Serializer::write( buf + index, transfunc[i].m_opacity_variable_min );
        index += jpv::Serializer::write( buf + index, transfunc[i].m_opacity_variable_max );
        index += jpv::Serializer::write( buf + index, transfunc[i].m_selection );
        if ( transfunc[i].m_selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            index += jpv::Serializer::write( buf + index, transfunc[i].m_resolution );
            index += jpv::Serializer::write( buf + index, transfunc[i].m_equation_red );
            index += jpv::Serializer::write( buf + index, transfunc[i].m_equation_green );
            index += jpv::Serializer::write( buf + index, transfunc[i].m_equation_blue );
            index += jpv::Serializer::write( buf + index, transfunc[i].m_equation_opacity );
        }
        else if ( transfunc[i].m_selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            index += jpv::Serializer::pack<kvs::TransferFunction>( buf + index, transfunc[i] );
        }
    }
    index += jpv::Serializer::write( buf + index, voleqn.size() );
    for ( size_t i = 0; i < voleqn.size(); i++ )
    {
        index += jpv::Serializer::write( buf + index, voleqn[i].m_name );
        index += jpv::Serializer::write( buf + index, voleqn[i].m_equation );
    }
//    index += jpv::Serializer::write( buf + index, transferFunctionSynthesis );
    index += jpv::Serializer::write( buf + index, color_tf_synthesis );
    index += jpv::Serializer::write( buf + index, opacity_tf_synthesis );

    if ( m_flag_send_bins == 1 )
    {
        index += jpv::Serializer::writeArray<float>( buf + index, m_positions, 3 * m_number_particle );
        index += jpv::Serializer::writeArray<float>( buf + index, m_normals, 3 * m_number_particle );
        index += jpv::Serializer::writeArray<unsigned char>( buf + index, m_colors, 3 * m_number_particle );
    }
    return index;
}

size_t jpv::ParticleTransferServerMessage::unpack_message( const char* buf )
{
    size_t index = 0;
    index += jpv::Serializer::readArray( buf + index, m_header );
    if ( strncmp( "JPTP /1.0", m_header, 9 ) != 0 )
    {
        return 0;
    }
    index += jpv::Serializer::read( buf + index, m_message_size );
    index += jpv::Serializer::read( buf + index, m_time_step );
    index += jpv::Serializer::read( buf + index, m_subpixel_level );
    index += jpv::Serializer::read( buf + index, m_repeat_level );
    index += jpv::Serializer::read( buf + index, m_level_index );
    index += jpv::Serializer::read( buf + index, m_number_particle );
    index += jpv::Serializer::read( buf + index, m_number_volume_divide );
    index += jpv::Serializer::read( buf + index, m_start_step );
    index += jpv::Serializer::read( buf + index, m_end_step );
    index += jpv::Serializer::read( buf + index, m_number_step );
    index += jpv::Serializer::read( buf + index, m_min_object_coord[0] );
    index += jpv::Serializer::read( buf + index, m_min_object_coord[1] );
    index += jpv::Serializer::read( buf + index, m_min_object_coord[2] );
    index += jpv::Serializer::read( buf + index, m_max_object_coord[0] );
    index += jpv::Serializer::read( buf + index, m_max_object_coord[1] );
    index += jpv::Serializer::read( buf + index, m_max_object_coord[2] );
    index += jpv::Serializer::read( buf + index, m_min_value );
    index += jpv::Serializer::read( buf + index, m_max_value );
    index += jpv::Serializer::read( buf + index, m_number_nodes );
    index += jpv::Serializer::read( buf + index, m_number_elements );
    index += jpv::Serializer::read( buf + index, m_element_type );
    index += jpv::Serializer::read( buf + index, m_file_type );
    index += jpv::Serializer::read( buf + index, m_number_ingredients );
    index += m_variable_range.unpack( buf + index );
    index += jpv::Serializer::read( buf + index, m_flag_send_bins );
    index += jpv::Serializer::read( buf + index, particle_limit );
    index += jpv::Serializer::read( buf + index, particle_density );
    index += jpv::Serializer::read( buf + index, particle_data_size_limit );
    index += jpv::Serializer::unpack( buf + index, *camera );
    index += jpv::Serializer::read( buf + index, m_transfer_function_count );
    index += jpv::Serializer::read( buf + index, m_arrow_component[0] );
    index += jpv::Serializer::read( buf + index, m_arrow_component[1] );
    index += jpv::Serializer::read( buf + index, m_arrow_component[2] );
    // index += jpv::Serializer::read( buf + index, m_arrow_component[3] );
    // index += jpv::Serializer::read( buf + index, m_arrow_component[4] );
    // index += jpv::Serializer::read( buf + index, m_arrow_component[5] );
    index += jpv::Serializer::read( buf + index, m_arrow_flag );
    for (int i = 0;i<16 ; i++)
    {
        index += jpv::Serializer::read( buf + index, m_glyph_xform[i] );
    }
    for (int i = 0;i<16 ; i++)
    {
        index += jpv::Serializer::read( buf + index, m_obj_xform[i] );
    }
    m_color_nbins = new kvs::UInt64[ m_transfer_function_count ];
    m_opacity_bins_number = new kvs::UInt64[ m_transfer_function_count ];

    m_color_bins.resize( m_transfer_function_count );
    m_opacity_bins.resize( m_transfer_function_count );
    for ( int i = 0; i < m_transfer_function_count; i++ )
    {
        index += jpv::Serializer::read( buf + index, m_color_nbins[i] );
        m_color_bins[i] =  new kvs::UInt64[ m_color_nbins[i] ];
        index += jpv::Serializer::readArray<kvs::UInt64>( buf + index, m_color_bins[i], m_color_nbins[i] );
        index += jpv::Serializer::read( buf + index, m_opacity_bins_number[i] );
        m_opacity_bins[i] =  new kvs::UInt64[ m_opacity_bins_number[i] ];
        index += jpv::Serializer::readArray<kvs::UInt64>( buf + index, m_opacity_bins[i], m_opacity_bins_number[i] );
    }

    size_t s;
    index += jpv::Serializer::read( buf + index, s );
    transfunc.clear();
    for ( size_t i = 0; i < s; i++ )
    {
        transfunc.push_back( NamedTransferFunctionParameter() );
        index += jpv::Serializer::read( buf + index, transfunc[i].m_name );
        index += jpv::Serializer::read( buf + index, transfunc[i].m_color_variable );
        index += jpv::Serializer::read( buf + index, transfunc[i].m_color_variable_min );
        index += jpv::Serializer::read( buf + index, transfunc[i].m_color_variable_max );
        index += jpv::Serializer::read( buf + index, transfunc[i].m_opacity_variable );
        index += jpv::Serializer::read( buf + index, transfunc[i].m_opacity_variable_min );
        index += jpv::Serializer::read( buf + index, transfunc[i].m_opacity_variable_max );
        index += jpv::Serializer::read( buf + index, transfunc[i].m_selection );
        if ( transfunc[i].m_selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            index += jpv::Serializer::read( buf + index, transfunc[i].m_resolution );
            index += jpv::Serializer::read( buf + index, transfunc[i].m_equation_red );
            index += jpv::Serializer::read( buf + index, transfunc[i].m_equation_green );
            index += jpv::Serializer::read( buf + index, transfunc[i].m_equation_blue );
            index += jpv::Serializer::read( buf + index, transfunc[i].m_equation_opacity );
        }
        else if ( transfunc[i].m_selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            index += jpv::Serializer::unpack<kvs::TransferFunction>( buf + index, transfunc[i] );
        }
    }
    index += jpv::Serializer::read( buf + index, s );
    voleqn.clear();
    for ( size_t i = 0; i < s; i++ )
    {
        voleqn.push_back( VolumeEquation() );
        index += jpv::Serializer::read( buf + index, voleqn[i].m_name );
        index += jpv::Serializer::read( buf + index, voleqn[i].m_equation );
    }
//    index += jpv::Serializer::read( buf + index, transferFunctionSynthesis );
    index += jpv::Serializer::read( buf + index, color_tf_synthesis );
    index += jpv::Serializer::read( buf + index, opacity_tf_synthesis );
    return index;
}

size_t jpv::ParticleTransferServerMessage::unpack_particles( const char* buf )
{
    size_t index = m_message_size;
    m_positions = new float[3 * m_number_particle];
    m_normals = new float[3 * m_number_particle];
    m_colors = new unsigned char[3 * m_number_particle];
    index += jpv::Serializer::readArray<float>( buf + index, m_positions, 3 * m_number_particle );
    index += jpv::Serializer::readArray<float>( buf + index, m_normals, 3 * m_number_particle );
    index += jpv::Serializer::readArray<unsigned char>( buf + index, m_colors, 3 * m_number_particle );
    return index;
}

//2019 kawamura
void jpv::ParticleTransferServerMessage::show( void ) const
{
    std::cout<<"SHOW SERVER MESSAGE"<<std::endl;
    std::cout<<"header="<<m_header<<std::endl;
    std::cout<<"messageSize="<<m_message_size<<std::endl;
    std::cout<<"timeStep="<<m_time_step<<std::endl;
    std::cout<<"numParticle="<<m_number_particle<<std::endl;

    std::cout<<"m_arrow_component = (" << m_arrow_component[0] << ", " << m_arrow_component[1] << ", " << m_arrow_component[2] << /*", " << m_arrow_component[3]
             << ", " << m_arrow_component[4] << ", " << m_arrow_component[5] <<*/  " )"<<std::endl;

    std::cout<<"transfunc.size="<<transfunc.size()<<std::endl;
    std::cout<<"transfunc.Name,ColorVar,OpacityVar,ColorVarMin,ColorVarMax"<<std::endl;
    for(int i=0; i<transfunc.size(); i++)
    {
        std::cout<<transfunc[i].m_name<<","<<transfunc[i].m_color_variable<<","<<transfunc[i].m_opacity_variable<<","<<transfunc[i].m_color_variable_min<<","<<transfunc[i].m_color_variable_max<<std::endl;
    }

    std::cout<<"voleqn.size="<<voleqn.size()<<std::endl;
    std::cout<<"voleqn.Name,Equation"<<std::endl;
    for(int i=0; i<voleqn.size(); i++)
    {
        std::cout<<voleqn[i].m_name<<","<<voleqn[i].m_equation<<std::endl;
    }

    //std::cout<<"transferFunctionSynthesis="<<transferFunctionSynthesis<<std::endl;
    std::cout << "color_tf_synthesis=" << color_tf_synthesis << std::endl;
    std::cout << "opacity_tf_synthesis=" << opacity_tf_synthesis << std::endl;

    std::cout<<std::endl;
}
