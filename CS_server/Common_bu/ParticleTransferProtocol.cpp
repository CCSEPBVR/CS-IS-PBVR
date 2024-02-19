#include "ParticleTransferProtocol.h"
#include "Serializer.h"

#include <kvs/Camera>
#include <kvs/TransferFunction>

#include <cstring>
#include <algorithm>

// リトルエンディアン環境では JKVS
// ビッグエンディアン環境では SVKJ
const int32_t jpv::ParticleTransferUtils::m_magic_number = 0x53564b4a; // 1398164298;

bool jpv::ParticleTransferUtils::isLittleEndian()
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

jpv::ParticleTransferClientMessage::ParticleTransferClientMessage():
    m_camera( NULL )
    /* ,m_transfer_function( NULL ) delete by @hira at 2016/12/01 */
{
    std::fill( m_header, m_header + sizeof( m_header ), '\0' );
    this->m_filter_parameter_filename.clear();
    m_transfer_function.clear();		// add by @hira at 2016/12/01
}

int32_t jpv::ParticleTransferClientMessage::byteSize() const
{
    int32_t s = 0;
    s += sizeof( m_header );
    s += sizeof( m_message_size );
    s += sizeof( m_initialize_parameter );
    if ( m_initialize_parameter == -3 )
    {
        s += sizeof( int64_t );
        s += sizeof( char ) * ( m_input_directory.size() + 1 );
        // add:start by @hira at 2016/12/01
        s += sizeof( int64_t );
        s += sizeof( char ) * ( m_filter_parameter_filename.size() + 1 );
        // add:end by @hira at 2016/12/01
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
        s += sizeof( int64_t );
        s += sizeof( char ) * ( m_input_directory.size() + 1 );
        // add:start by @hira at 2016/12/01
        s += sizeof( int64_t );
        s += sizeof( char ) * ( m_filter_parameter_filename.size() + 1 );
        // add:end by @hira at 2016/12/01
        s += sizeof( size_t );
        s += sizeof( char ) * ( m_x_synthesis.size() + 1 );
        s += sizeof( size_t );
        s += sizeof( char ) * ( m_y_synthesis.size() + 1 );
        s += sizeof( size_t );
        s += sizeof( char ) * ( m_z_synthesis.size() + 1 );
        s += jpv::Serializer::byteSize<kvs::Camera>( *m_camera );
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
        s += jpv::Serializer::byteSize( m_transfer_function_synthesis );
        // add by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）
        s += jpv::Serializer::byteSize( m_color_transfer_function_synthesis );
        s += jpv::Serializer::byteSize( m_opacity_transfer_function_synthesis );
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
        // add:start by @hira at 2016/12/01
        index += jpv::Serializer::write( buf + index, m_filter_parameter_filename.size() );
        index += jpv::Serializer::writeArray( buf + index, m_filter_parameter_filename.c_str(), m_filter_parameter_filename.size() + 1 );
        // add:end by @hira at 2016/12/01
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
        index += jpv::Serializer::write( buf + index, m_input_directory.size() );
        index += jpv::Serializer::writeArray( buf + index, m_input_directory.c_str(), m_input_directory.size() + 1 );
        // add:start by @hira at 2016/12/01
        index += jpv::Serializer::write( buf + index, m_filter_parameter_filename.size() );
        index += jpv::Serializer::writeArray( buf + index, m_filter_parameter_filename.c_str(), m_filter_parameter_filename.size() + 1 );
        // add:end by @hira at 2016/12/01
        index += jpv::Serializer::write( buf + index, m_x_synthesis.size() );
        index += jpv::Serializer::writeArray( buf + index, m_x_synthesis.c_str(), m_x_synthesis.size() + 1 );
        index += jpv::Serializer::write( buf + index, m_y_synthesis.size() );
        index += jpv::Serializer::writeArray( buf + index, m_y_synthesis.c_str(), m_y_synthesis.size() + 1 );
        index += jpv::Serializer::write( buf + index, m_z_synthesis.size() );
        index += jpv::Serializer::writeArray( buf + index, m_z_synthesis.c_str(), m_z_synthesis.size() + 1 );

        index += jpv::Serializer::pack( buf + index, *m_camera );
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
        index += jpv::Serializer::write( buf + index, m_transfer_function_synthesis );
        // add by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）
        index += jpv::Serializer::write( buf + index, m_color_transfer_function_synthesis );
        index += jpv::Serializer::write( buf + index, m_opacity_transfer_function_synthesis );
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
    index += jpv::Serializer::read( buf + index, &m_message_size );
    index += jpv::Serializer::read( buf + index, &m_initialize_parameter );
    if ( m_initialize_parameter == -3 )
    {
        index += jpv::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_input_directory = std::string( tmp_char );
        delete[] tmp_char;

        // add:start by @hira at 2016/12/01
        index += jpv::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_filter_parameter_filename = std::string( tmp_char );
        delete[] tmp_char;
        // add:end by @hira at 2016/12/01
    }
    if ( m_initialize_parameter == 1 )
    {
        index += jpv::Serializer::read( buf + index, &m_sampling_method );
        index += jpv::Serializer::read( buf + index, &m_subpixel_level );
        index += jpv::Serializer::read( buf + index, &m_repeat_level );
        index += jpv::Serializer::read( buf + index, &m_node_type );
        index += jpv::Serializer::read( buf + index, &m_rendering_id );
        index += jpv::Serializer::read( buf + index, &m_sampling_step );
        index += jpv::Serializer::read( buf + index, &m_shuffle_method );
        index += jpv::Serializer::read( buf + index, &m_enable_crop_region );
        for ( int i = 0; i < 6; i++ )
        {
            index += jpv::Serializer::read( buf + index, &m_crop_region[i] );
        }
        index += jpv::Serializer::read( buf + index, &m_particle_limit );
        index += jpv::Serializer::read( buf + index, &m_particle_density );
        index += jpv::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_input_directory = std::string( tmp_char );
        delete[] tmp_char;

        // add:start by @hira at 2016/12/01
        index += jpv::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_filter_parameter_filename = std::string( tmp_char );
        delete[] tmp_char;
        // add:end by @hira at 2016/12/01

        // X-Z coodrinate Synthesis
        index += jpv::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_x_synthesis = std::string( tmp_char );
        delete[] tmp_char;
        index += jpv::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_y_synthesis = std::string( tmp_char );
        delete[] tmp_char;
        index += jpv::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_z_synthesis = std::string( tmp_char );
        delete[] tmp_char;
        std::cout << "Receive X Syth = " << m_x_synthesis << std::endl;
        std::cout << "Receive Y Syth = " << m_y_synthesis << std::endl;
        std::cout << "Receive Z Syth = " << m_z_synthesis << std::endl;

        index += jpv::Serializer::unpack( buf + index, m_camera );
    }
    if ( m_initialize_parameter == 1 || m_initialize_parameter == -3 )
    {
        size_t s;
        index += jpv::Serializer::read( buf + index, &s );
        m_transfer_function.clear();
        for ( size_t i = 0; i < s; i++ )
        {
            m_transfer_function.push_back( NamedTransferFunctionParameter() );
            index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_name );
            index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_color_variable );
            index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_color_variable_min );
            index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_color_variable_max );
            index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_opacity_variable );
            index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_opacity_variable_min );
            index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_opacity_variable_max );
            index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_selection );
            if ( m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
            {
                index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_resolution );
                index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_equation_red );
                index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_equation_green );
                index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_equation_blue );
                index += jpv::Serializer::read( buf + index, &m_transfer_function[i].m_equation_opacity );
            }
            else if ( m_transfer_function[i].m_selection == NamedTransferFunctionParameter::SelectTransferFunction )
            {
                index += jpv::Serializer::unpack<kvs::TransferFunction>( buf + index, &( m_transfer_function[i] ) );
            }
        }
        index += jpv::Serializer::read( buf + index, &s );
        m_volume_equation.clear();
        for ( size_t i = 0; i < s; i++ )
        {
            m_volume_equation.push_back( VolumeEquation() );
            index += jpv::Serializer::read( buf + index, &m_volume_equation[i].m_name );
            index += jpv::Serializer::read( buf + index, &m_volume_equation[i].m_equation );
        }
        index += jpv::Serializer::read( buf + index, &m_transfer_function_synthesis );
        // add by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）
        index += jpv::Serializer::read( buf + index, &m_color_transfer_function_synthesis );
        index += jpv::Serializer::read( buf + index, &m_opacity_transfer_function_synthesis );
    }
    if ( m_initialize_parameter >= 0 )
    {
        index += jpv::Serializer::read( buf + index, &m_time_parameter );
        if ( m_time_parameter == 0 )
        {
            index += jpv::Serializer::read( buf + index, &m_memory_size );
        }
        else if ( m_time_parameter == 1 )
        {
            index += jpv::Serializer::read( buf + index, &m_begin_time );
            index += jpv::Serializer::read( buf + index, &m_end_time );
            index += jpv::Serializer::read( buf + index, &m_memory_size );
        }
        else if ( m_time_parameter == 2 )
        {
            index += jpv::Serializer::read( buf + index, &m_step );
        }
        index += jpv::Serializer::read( buf + index, &m_trans_Parameter );
        if ( m_trans_Parameter == 1 )
        {
            index += jpv::Serializer::read( buf + index, &m_level_index );
        }
    }

    return index;
}

jpv::ParticleTransferServerMessage::ParticleTransferServerMessage()
{
    std::fill( m_header, m_header + sizeof( m_header ), '\0' );
    m_server_status = 0;
}

int32_t jpv::ParticleTransferServerMessage::byteSize() const
{
    int32_t s = 0;
    s += sizeof( m_header );
    s += sizeof( m_message_size );
    // MODIFIED START FEAST 2015.12.23
    s += sizeof( m_server_status );
    if ( m_server_status == 1 )
    {
        return s;
    }
    // MODIFIED START FEAST 2015.12.23
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
    if ( m_flag_send_bins == 1 )
        {
        s += sizeof( m_transfer_function_count );
        for ( int i = 0; i < m_transfer_function_count; i++ )
        {
            s += sizeof( m_color_nbins[i] );
            // modify by @hira 2016/12/01
            //s += sizeof( m_color_bins[i] ) * m_color_nbins[i];
            s += sizeof( m_color_bins[i][0] ) * m_color_nbins[i];
            s += sizeof( m_opacity_bins_number[i] );
            // modify by @hira 2016/12/01
            //s += sizeof( m_opacity_bins[i] ) * m_opacity_bins_number[i];
            s += sizeof( m_opacity_bins[i][0] ) * m_opacity_bins_number[i];

            // add by @hira at 2016/12/01
            s += jpv::Serializer::byteSize( m_color_bin_names[i] );
            s += jpv::Serializer::byteSize( m_opacity_bin_names[i] );
        }
    }
    return s;
}

size_t jpv::ParticleTransferServerMessage::pack( char* buf ) const
{
    size_t index = 0;
    index += jpv::Serializer::writeArray( buf + index, m_header );
    index += jpv::Serializer::write( buf + index, m_message_size );
    // MODIFIED START FEAST 2015.12.23
    index += jpv::Serializer::write( buf + index, m_server_status );
    if ( m_server_status == 1 )
    {
        return index;
    }
    // MODIFIED START FEAST 2015.12.23
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
    if ( m_flag_send_bins == 1 )
        {
        index += jpv::Serializer::write( buf + index, m_transfer_function_count );
        for ( int i = 0; i < m_transfer_function_count; i++ )
        {
            index += jpv::Serializer::write( buf + index, m_color_nbins[i] );
            index += jpv::Serializer::writeArray<kvs::UInt64>( buf + index, m_color_bins[i], m_color_nbins[i] );
            index += jpv::Serializer::write( buf + index, m_opacity_bins_number[i] );
            index += jpv::Serializer::writeArray<kvs::UInt64>( buf + index, m_opacity_bins[i], m_opacity_bins_number[i] );

            // add by @hira at 2016/12/01
            index += jpv::Serializer::write( buf + index, m_color_bin_names[i] );
            index += jpv::Serializer::write( buf + index, m_opacity_bin_names[i] );
        }
    }
    else
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
    index += jpv::Serializer::read( buf + index, &m_message_size );
    // MODIFIED START FEAST 2015.12.23
    index += jpv::Serializer::read( buf + index, &m_server_status );
    if ( m_server_status == 1 )
    {
        return 0;
    }
    // MODIFIED END FEAST 2015.12.23
    index += jpv::Serializer::read( buf + index, &m_time_step );
    index += jpv::Serializer::read( buf + index, &m_subpixel_level );
    index += jpv::Serializer::read( buf + index, &m_repeat_level );
    index += jpv::Serializer::read( buf + index, &m_level_index );
    index += jpv::Serializer::read( buf + index, &m_number_particle );
    index += jpv::Serializer::read( buf + index, &m_number_volume_divide );
    index += jpv::Serializer::read( buf + index, &m_start_step );
    index += jpv::Serializer::read( buf + index, &m_end_step );
    index += jpv::Serializer::read( buf + index, &m_number_step );
    index += jpv::Serializer::read( buf + index, &m_min_object_coord[0] );
    index += jpv::Serializer::read( buf + index, &m_min_object_coord[1] );
    index += jpv::Serializer::read( buf + index, &m_min_object_coord[2] );
    index += jpv::Serializer::read( buf + index, &m_max_object_coord[0] );
    index += jpv::Serializer::read( buf + index, &m_max_object_coord[1] );
    index += jpv::Serializer::read( buf + index, &m_max_object_coord[2] );
    index += jpv::Serializer::read( buf + index, &m_min_value );
    index += jpv::Serializer::read( buf + index, &m_max_value );
    index += jpv::Serializer::read( buf + index, &m_number_nodes );
    index += jpv::Serializer::read( buf + index, &m_number_elements );
    index += jpv::Serializer::read( buf + index, &m_element_type );
    index += jpv::Serializer::read( buf + index, &m_file_type );
    index += jpv::Serializer::read( buf + index, &m_number_ingredients );
    index += m_variable_range.unpack( buf + index );
    index += jpv::Serializer::read( buf + index, &m_flag_send_bins );
    if ( m_flag_send_bins == 1 )
    {
        index += jpv::Serializer::read( buf + index, &m_transfer_function_count );
        m_color_nbins = new kvs::UInt64[ m_transfer_function_count ];
        m_opacity_bins_number = new kvs::UInt64[ m_transfer_function_count ];

        m_color_bins.resize( m_transfer_function_count );
        m_opacity_bins.resize( m_transfer_function_count );
        m_color_bin_names.resize( m_transfer_function_count );
        m_opacity_bin_names.resize( m_transfer_function_count );
        for ( int i = 0; i < m_transfer_function_count; i++ )
        {
            index += jpv::Serializer::read( buf + index, &m_color_nbins[i] );
            m_color_bins[i] =  new kvs::UInt64[ m_color_nbins[i] ];
            index += jpv::Serializer::readArray<kvs::UInt64>( buf + index, m_color_bins[i], m_color_nbins[i] );
            index += jpv::Serializer::read( buf + index, &m_opacity_bins_number[i] );
            m_opacity_bins[i] =  new kvs::UInt64[ m_opacity_bins_number[i] ];
            index += jpv::Serializer::readArray<kvs::UInt64>( buf + index, m_opacity_bins[i], m_opacity_bins_number[i] );

            // add by @hira at 2016/12/01
            index += jpv::Serializer::read( buf + index, &m_color_bin_names[i] );
            index += jpv::Serializer::read( buf + index, &m_opacity_bin_names[i] );
        }
    }

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


/**
 * Color Histogram用のデータを設定する。
 * @param histogram_size		Histogram数
 * @param nbins					Histogramデータ数
 * @param c_bins			Color Histogram用のデータ = histogram_size*nbins
 * @param transfer_function_names	すべての1次伝達関数名
 * @param transfunc_synthesizer_names	Histogram用のデータの1次伝達関数名
 */
void jpv::ParticleTransferServerMessage::setColorHistogramBins(
        int histogram_size,
        int nbins,
        const kvs::UInt64* c_bins,
        const std::vector<std::string> &transfer_function_names,
        const std::vector<std::string> &transfunc_synthesizer_names)
{
    for ( int synth_tf = 0; synth_tf < histogram_size; synth_tf++ ) {
        std::string synth_name = transfunc_synthesizer_names[synth_tf];
        std::vector<std::string>::const_iterator itr
                = std::find(transfer_function_names.begin(),
                            transfer_function_names.end(),
                            synth_name);
        if (itr == transfer_function_names.end()) continue;

        size_t tf = std::distance(transfer_function_names.begin(), itr);
        if (tf >= transfer_function_names.size()) continue;

        this->m_color_nbins[tf] = nbins;
        for ( int res = 0; res < nbins; res++ ) {
            int n = synth_tf*nbins + res;
            this->m_color_bins[tf][res] = c_bins[n];
        }
        this->m_color_bin_names[tf] = synth_name;
    }

    return;
}


/**
 * Opacity Histogram用のデータを設定する。
 * @param histogram_size		Histogram数
 * @param nbins					Histogramデータ数
 * @param o_bins			Opacity Histogram用のデータ = histogram_size*nbins
 * @param transfer_function_names	すべての1次伝達関数名
 * @param transfunc_synthesizer_names	Histogram用のデータの1次伝達関数名
 */
void jpv::ParticleTransferServerMessage::setOpacityHistogramBins(
        int histogram_size,
        int nbins,
        const kvs::UInt64* o_bins,
        const std::vector<std::string> &transfer_function_names,
        const std::vector<std::string> &transfunc_synthesizer_names)
{
    for ( int synth_tf = 0; synth_tf < histogram_size; synth_tf++ ) {
        std::string synth_name = transfunc_synthesizer_names[synth_tf];
        std::vector<std::string>::const_iterator itr
                = std::find(transfer_function_names.begin(),
                            transfer_function_names.end(),
                            synth_name);
        if (itr == transfer_function_names.end()) continue;

        size_t tf = std::distance(transfer_function_names.begin(), itr);
        if (tf >= transfer_function_names.size()) continue;

        this->m_opacity_bins_number[tf] = nbins;
        for ( int res = 0; res < nbins; res++ ) {
            int n = synth_tf*nbins + res;
            this->m_opacity_bins[tf][res] = o_bins[n];
        }
        this->m_opacity_bin_names[tf] = synth_name;
    }

    return;
}

/**
 * 関数の領域確保、初期化を行う
 * @param transfer_function_count		関数数
 * @param nbins		解像度
 */
void jpv::ParticleTransferServerMessage::initializeTransferFunction(
        const int32_t transfer_function_count,
        const int nbins)
{

    this->m_transfer_function_count = transfer_function_count;

    this->m_color_nbins = new kvs::UInt64[transfer_function_count];
    this->m_opacity_bins_number = new kvs::UInt64[transfer_function_count];

    this->m_color_bins.resize( transfer_function_count );
    this->m_opacity_bins.resize( transfer_function_count );
    this->m_color_bin_names.resize( transfer_function_count );
    this->m_opacity_bin_names.resize( transfer_function_count );

    for ( int tf = 0; tf < this->m_transfer_function_count; tf++ )
    {
        this->m_color_nbins[tf] = nbins;
        this->m_opacity_bins_number[tf] = nbins;
        this->m_color_bins[tf] =  new kvs::UInt64[ this->m_color_nbins[tf] ];
        this->m_opacity_bins[tf] =  new kvs::UInt64[ this->m_opacity_bins_number[tf] ];
        for ( kvs::UInt64 res = 0; res < this->m_color_nbins[tf]; res++ )
        {
            this->m_color_bins[tf][res] = 0;
        }
        for ( kvs::UInt64 res = 0; res < this->m_opacity_bins_number[tf]; res++ )
        {
            this->m_opacity_bins[tf][res] = 0;
        }
        this->m_color_bin_names[tf].clear();
        this->m_opacity_bin_names[tf].clear();
    }

    return;
}

