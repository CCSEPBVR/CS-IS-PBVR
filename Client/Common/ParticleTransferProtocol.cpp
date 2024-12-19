#include "ParticleTransferProtocol.h"
#include "serializer.h"

#include <kvs/Camera>
#include <kvs/TransferFunction>

#include <cstring>
#include <typeinfo>

// リトルエンディアン環境では JKVS
// ビッグエンディアン環境では SVKJ
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
    this->m_filter_parameter_filename.clear();
    m_transfer_function.clear();		// add by @hira at 2016/12/01
}

int32_t jpv::ParticleTransferClientMessage::byteSize( void ) const
{
    int32_t s = 0;
    s += sizeof( m_header );
    s += sizeof( m_message_size );
    s += sizeof( m_initialize_parameter );
    if ( m_initialize_parameter == InitializeParameter::initial_step )
    {
        s += sizeof( bool );
        s += sizeof( int64_t );
        s += sizeof( char ) * ( m_input_directory.size() + 1 );
        //        // add:start by @hira at 2016/12/01
        //        s += sizeof( int64_t );
        //        s += sizeof( char ) * ( m_filter_parameter_filename.size() + 1 );
        //        // add:end by @hira at 2016/12/01
    }
    if ( m_initialize_parameter == InitializeParameter::generate_particle || m_initialize_parameter == InitializeParameter::initial_step
         || m_initialize_parameter == InitializeParameter::export_TFfile  )
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
        s += sizeof( m_particle_data_size_limit ); //add by shimomura 0308
        s += sizeof( int64_t );
        s += sizeof( char ) * ( m_input_directory.size() + 1 );
        //        // add:start by @hira at 2016/12/01
        //        s += sizeof( int64_t );
        //        s += sizeof( char ) * ( m_filter_parameter_filename.size() + 1 );
        //        // add:end by @hira at 2016/12/01
        s += sizeof( size_t );
        s += sizeof( char ) * ( m_x_synthesis.size() + 1 );
        s += sizeof( size_t );
        s += sizeof( char ) * ( m_y_synthesis.size() + 1 );
        s += sizeof( size_t );
        s += sizeof( char ) * ( m_z_synthesis.size() + 1 );
        s += jpv::Serializer::byteSize<kvs::Camera>( *m_camera );
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
                //デフォルトでこちらの分岐に入る
                //Serializer内部で伝達関数のテーブルサイズと最大最小値のサイズを計算
                s += jpv::Serializer::byteSize<kvs::TransferFunction>( m_transfer_function[i] );
            }
        }
        s += jpv::Serializer::byteSize( m_volume_equation.size() );
        for ( size_t i = 0; i < m_volume_equation.size(); i++ )
        {
            s += jpv::Serializer::byteSize( m_volume_equation[i].m_name );
            s += jpv::Serializer::byteSize( m_volume_equation[i].m_equation );
        }
        //s += jpv::Serializer::byteSize( m_transfer_function_synthesis );
        // add by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）
        s += jpv::Serializer::byteSize( m_color_transfer_function_synthesis );
        s += jpv::Serializer::byteSize( m_opacity_transfer_function_synthesis );

        s += sizeof( bool );
        for( int i = 0; i < 3; i++ )
        {
            s += jpv::Serializer::byteSize( m_direction_variable[i] );
        }

        s += jpv::Serializer::byteSize( m_size_sampling_method );
        s += jpv::Serializer::byteSize( m_size_variables.size() );
        for( int i = 0; i < m_size_variables.size(); i++ )
        {
            s += jpv::Serializer::byteSize( m_size_variables[i] );
        }

        s += jpv::Serializer::byteSize( m_distribution_mode );
        s += jpv::Serializer::byteSize( m_number_of_sampling_point );
        s += jpv::Serializer::byteSize( m_seed );
        s += jpv::Serializer::byteSize( m_stride );

        s += jpv::Serializer::byteSize( m_glyph_color_map_table.size() );
        for( int i = 0; i <  m_glyph_color_map_table.size(); i++ )
        {
            s += jpv::Serializer::byteSize(  m_glyph_color_map_table[i] );
        }

        s += jpv::Serializer::byteSize( m_color_data_sampling_method );
        s += jpv::Serializer::byteSize( m_color_data_variables.size() );
        for( int i = 0; i < m_color_data_variables.size(); i++ )
        {
            s += jpv::Serializer::byteSize( m_color_data_variables[i] );
        }
    }
    if ( m_initialize_parameter == InitializeParameter::generate_particle )
    {
        s += sizeof( m_time_parameter );
        if ( m_time_parameter == 0 )
        {
            s += sizeof( m_memory_size );
        }
        else if ( m_time_parameter == 1 )
        {
            s += sizeof( m_begin_time );
            s += sizeof( m_last_time );
            s += sizeof( m_memory_size );
        }
        else if ( m_time_parameter == 2 )
        {
            s += sizeof( m_step );
        }
        s += sizeof( m_trans_parameter );
        if ( m_trans_parameter == 1 )
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
    // index += jpv::Serializer::write( buf + index, m_initialize_parameter );
    if ( m_initialize_parameter == InitializeParameter::initial_step )
    {
        index += jpv::Serializer::write( buf + index, m_import_flag );
        index += jpv::Serializer::write( buf + index, m_input_directory.size() );
        index += jpv::Serializer::writeArray( buf + index, m_input_directory.c_str(), m_input_directory.size() + 1 );
        //        // add:start by @hira at 2016/12/01
        //        index += jpv::Serializer::write( buf + index, m_filter_parameter_filename.size() );
        //        index += jpv::Serializer::writeArray( buf + index, m_filter_parameter_filename.c_str(), m_filter_parameter_filename.size() + 1 );
        //        // add:end by @hira at 2016/12/01
    }
    if ( m_initialize_parameter == InitializeParameter::generate_particle || m_initialize_parameter == InitializeParameter::initial_step
         || m_initialize_parameter == InitializeParameter::export_TFfile         )
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
        index += jpv::Serializer::write( buf + index, m_particle_data_size_limit );
        index += jpv::Serializer::write( buf + index, m_input_directory.size() );
        index += jpv::Serializer::writeArray( buf + index, m_input_directory.c_str(), m_input_directory.size() + 1 );
        //        // add:start by @hira at 2016/12/01
        //        index += jpv::Serializer::write( buf + index, m_filter_parameter_filename.size() );
        //        index += jpv::Serializer::writeArray( buf + index, m_filter_parameter_filename.c_str(), m_filter_parameter_filename.size() + 1 );
        //        // add:end by @hira at 2016/12/01
        index += jpv::Serializer::write( buf + index, m_x_synthesis.size() );
        index += jpv::Serializer::writeArray( buf + index, m_x_synthesis.c_str(), m_x_synthesis.size() + 1 );
        index += jpv::Serializer::write( buf + index, m_y_synthesis.size() );
        index += jpv::Serializer::writeArray( buf + index, m_y_synthesis.c_str(), m_y_synthesis.size() + 1 );
        index += jpv::Serializer::write( buf + index, m_z_synthesis.size() );
        index += jpv::Serializer::writeArray( buf + index, m_z_synthesis.c_str(), m_z_synthesis.size() + 1 );

        index += jpv::Serializer::pack( buf + index, *m_camera );
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
                //デフォルトでこちらの分岐に入る
                //Serializer内部で伝達関数のテーブルサイズと最大最小値をパック
                index += jpv::Serializer::pack<kvs::TransferFunction>( buf + index, m_transfer_function[i] );
            }
        }
        index += jpv::Serializer::write( buf + index, m_volume_equation.size() );
        for ( size_t i = 0; i < m_volume_equation.size(); i++ )
        {
            index += jpv::Serializer::write( buf + index, m_volume_equation[i].m_name );
            index += jpv::Serializer::write( buf + index, m_volume_equation[i].m_equation );
        }
        //index += jpv::Serializer::write( buf + index, m_transfer_function_synthesis );
        // add by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）
        index += jpv::Serializer::write( buf + index, m_color_transfer_function_synthesis );
        index += jpv::Serializer::write( buf + index, m_opacity_transfer_function_synthesis );

        // glyph
        index += jpv::Serializer::write(buf + index, m_glyph_flag);
        for( int i = 0; i < 3; i++ )
        {
            index += jpv::Serializer::write( buf + index, m_direction_variable[i] );
        }

        index += jpv::Serializer::write( buf + index, m_size_sampling_method );
        index += jpv::Serializer::write( buf + index, m_size_variables.size() );
        for( int i = 0; i < m_size_variables.size(); i++ )
        {
            index += jpv::Serializer::write( buf + index, m_size_variables[i] );
        }


        index += jpv::Serializer::write( buf + index, m_distribution_mode );
        index += jpv::Serializer::write( buf + index, m_number_of_sampling_point );
        index += jpv::Serializer::write( buf + index, m_seed );
        index += jpv::Serializer::write( buf + index, m_stride );

        index += jpv::Serializer::write( buf + index, m_glyph_color_map_table.size() );
        for( int i = 0; i < m_glyph_color_map_table.size(); i++ )
        {
            index += jpv::Serializer::write( buf + index, m_glyph_color_map_table[i] );
        }

        index += jpv::Serializer::write( buf + index, m_color_data_sampling_method );
        index += jpv::Serializer::write( buf + index, m_color_data_variables.size() );
        for( int i = 0; i < m_color_data_variables.size(); i++ )
        {
            index += jpv::Serializer::write( buf + index, m_color_data_variables[i] );
        }
    }
    if ( m_initialize_parameter == InitializeParameter::generate_particle )
    {
        index += jpv::Serializer::write( buf + index, m_time_parameter );
        if ( m_time_parameter == 0 )
        {
            index += jpv::Serializer::write( buf + index, m_memory_size );
        }
        else if ( m_time_parameter == 1 )
        {
            index += jpv::Serializer::write( buf + index, m_begin_time );
            index += jpv::Serializer::write( buf + index, m_last_time );
            index += jpv::Serializer::write( buf + index, m_memory_size );
        }
        else if ( m_time_parameter == 2 )
        {
            index += jpv::Serializer::write( buf + index, m_step );
        }
        index += jpv::Serializer::write( buf + index, m_trans_parameter );
        if ( m_trans_parameter == 1 )
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
    if ( m_initialize_parameter == InitializeParameter::initial_step )
    {
        index += jpv::Serializer::read( buf + index, &m_import_flag );
        index += jpv::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_input_directory = std::string( tmp_char );
        delete[] tmp_char;

        //        // add:start by @hira at 2016/12/01
        //        index += jpv::Serializer::read( buf + index, &tmp_char_size );
        //        tmp_char = new char[tmp_char_size + 1];
        //        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        //        m_filter_parameter_filename = std::string( tmp_char );
        //        delete[] tmp_char;
        //        // add:end by @hira at 2016/12/01
    }
    if ( m_initialize_parameter == InitializeParameter::generate_particle || m_initialize_parameter == InitializeParameter::initial_step
         || m_initialize_parameter == InitializeParameter::export_TFfile  )
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
        index += jpv::Serializer::read( buf + index, &m_particle_data_size_limit );
        index += jpv::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_input_directory = std::string( tmp_char );
        delete[] tmp_char;

        //        // add:start by @hira at 2016/12/01
        //        index += jpv::Serializer::read( buf + index, &tmp_char_size );
        //        tmp_char = new char[tmp_char_size + 1];
        //        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        //        m_filter_parameter_filename = std::string( tmp_char );
        //        delete[] tmp_char;
        //        // add:end by @hira at 2016/12/01

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
                //デフォルトでこちらの分岐に入る
                //Serializer内部で伝達関数のテーブルサイズと最大最小値をアンパック
                index += jpv::Serializer::unpack<kvs::TransferFunction>( buf + index, &m_transfer_function[i] );
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
        //index += jpv::Serializer::read( buf + index, &m_transfer_function_synthesis );
        // add by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）
        index += jpv::Serializer::read( buf + index, &m_color_transfer_function_synthesis );
        index += jpv::Serializer::read( buf + index, &m_opacity_transfer_function_synthesis );

        for( int i = 0; i < 3; i++ )
        {
            index += jpv::Serializer::read( buf + index, &m_direction_variable[i] );
        }

        index += jpv::Serializer::read( buf + index, &m_size_sampling_method );
        index += jpv::Serializer::read( buf + index, &s );
        m_size_variables.clear();
        for ( size_t i = 0; i < s; i++ )
        {
            int32_t value = 0;
            index += jpv::Serializer::read(buf + index, &value);
            m_size_variables.push_back(value);
        }

        index += jpv::Serializer::read( buf + index, &m_glyph_flag );
        index += jpv::Serializer::read( buf + index, &m_distribution_mode );
        index += jpv::Serializer::read( buf + index, &m_number_of_sampling_point );
        index += jpv::Serializer::read( buf + index, &m_seed );
        index += jpv::Serializer::read( buf + index, &m_stride );

        index += jpv::Serializer::read( buf + index, &s );
        m_glyph_color_map_table.clear();
        for ( size_t i = 0; i < s; i++ )
        {
            int32_t value = 0;
            index += jpv::Serializer::read(buf + index, &value);
            m_glyph_color_map_table.push_back(value);
        }

        index += jpv::Serializer::read( buf + index, &m_color_data_sampling_method );
        index += jpv::Serializer::read( buf + index, &s );
        m_color_data_variables.clear();
        for ( size_t i = 0; i < s; i++ )
        {
            int32_t value = 0;
            index += jpv::Serializer::read(buf + index, &value);
            m_color_data_variables.push_back(value);
        }
    }
    if ( m_initialize_parameter == InitializeParameter::generate_particle )
    {
        index += jpv::Serializer::read( buf + index, &m_time_parameter );
        if ( m_time_parameter == 0 )
        {
            index += jpv::Serializer::read( buf + index, &m_memory_size );
        }
        else if ( m_time_parameter == 1 )
        {
            index += jpv::Serializer::read( buf + index, &m_begin_time );
            index += jpv::Serializer::read( buf + index, &m_last_time );
            index += jpv::Serializer::read( buf + index, &m_memory_size );
        }
        else if ( m_time_parameter == 2 )
        {
            index += jpv::Serializer::read( buf + index, &m_step );
        }
        index += jpv::Serializer::read( buf + index, &m_trans_parameter );
        if ( m_trans_parameter == 1 )
        {
            index += jpv::Serializer::read( buf + index, &m_level_index );
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
    std::cout<< "importFlag="<< m_import_flag <<std::endl;
    std::cout<<"initParam="<<static_cast<int>(m_initialize_parameter)<<std::endl;
    std::cout<<"timeParam="<<m_time_parameter<<std::endl;
    std::cout<<"transParam="<<m_trans_parameter<<std::endl;

    std::cout<<"transfunc.size="<<m_transfer_function.size()<<std::endl;
    std::cout<<"transfunc.Name,ColorVar,OpacityVar,ColorVarMin,ColorVarMax, OpacityVarMin, OpacityVarMax"<<std::endl;
    for(int i=0; i<m_transfer_function.size(); i++)
    {
        std::cout << m_transfer_function[i].m_name << "," << m_transfer_function[i].m_color_variable << "," << m_transfer_function[i].m_opacity_variable << "," << m_transfer_function[i].m_color_variable_min << "," << m_transfer_function[i].m_color_variable_max << ", " << m_transfer_function[i].m_opacity_variable_min << "," << m_transfer_function[i].m_opacity_variable_max << std::endl;
    }

    std::cout<<"voleqn.size="<<m_volume_equation.size()<<std::endl;
    std::cout<<"voleqn.Name,Equation"<<std::endl;
    for(int i=0; i<m_volume_equation.size(); i++)
    {
        std::cout<<m_volume_equation[i].m_name<<","<<m_volume_equation[i].m_equation<<std::endl;
    }

    //std::cout<<"transferFunctionSynthesis="<<transferFunctionSynthesis<<std::endl;
    std::cout << "color_tf_synthesis=" << m_color_transfer_function_synthesis << std::endl;
    std::cout << "opacity_tf_synthesis=" << m_opacity_transfer_function_synthesis << std::endl;

    std::cout << "m_x_synthesis = " << m_x_synthesis << std::endl;
    std::cout << "m_y_synthesis = " << m_y_synthesis << std::endl;
    std::cout << "m_z_synthesis = " << m_z_synthesis << std::endl;

    std::cout << __LINE__ << std::endl;
    std::cout << "m_direction_variable" << std::endl;
    std::cout << m_direction_variable[0];
    std::cout << ",";
    std::cout << m_direction_variable[1];
    std::cout << ",";
    std::cout << m_direction_variable[2];
    std::cout << std::endl;

    switch( m_size_sampling_method )
    {
    case DataDefines::Constant:
        std::cout << "Constant" << std::endl;
        break;
    case DataDefines::SingleVariable:
        std::cout << "SingleVariable" << std::endl;
        break;
    case DataDefines::VariableArray:
        std::cout << "VariableArray" << std::endl;
        break;
    }
    for( int i = 0; i < m_size_variables.size(); i++ )
    {
        std::cout << "[" << i << "] : " << m_size_variables[i] << std::endl;
    }

    switch( m_distribution_mode )
    {
    case GlyphMode::UniformDistribution:
        std::cout << "UniformDistribution" << std::endl;
        std::cout << "m_number_of_sampling_point   : " << m_number_of_sampling_point << std::endl;
        std::cout << "m_seed                       : " << m_seed << std::endl;

        break;
    case GlyphMode::AllPoints:
        std::cout << "AllPoints" << std::endl;
        break;
    case GlyphMode::EveryNthPoints:
        std::cout << "EveryNthPoints" << std::endl;
        std::cout << "m_stride : " << m_stride << std::endl;
        break;
    }

    // for( int i = 0; i < m_glyph_color_map_table.size(); i++ )
    // {
    //     std::cout << m_glyph_color_map_table[i];
    //     std::cout << ",";
    // }
    // std::cout << std::endl;

    switch( m_color_data_sampling_method )
    {
    case DataDefines::Constant:
        std::cout << "Constant" << std::endl;
        break;
    case DataDefines::SingleVariable:
        std::cout << "SingleVariable" << std::endl;
        break;
    case DataDefines::VariableArray:
        std::cout << "VariableArray" << std::endl;
        break;
    }
    for( int i = 0; i < m_color_data_variables.size(); i ++ )
    {
        std::cout << "[" << i << "] : " << m_color_data_variables[i] << std::endl;
    }

#if 0
    // show token
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
#endif
    std::cout<<std::endl;
}

jpv::ParticleTransferServerMessage::ParticleTransferServerMessage( void ):
    m_camera( NULL )
{
    std::fill( m_header, m_header + sizeof( m_header ), '\0' );
}

int32_t jpv::ParticleTransferServerMessage::byteSize( void ) const
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
    s += sizeof( m_number_glyph );
    s += sizeof( m_number_volume_divide );
    s += sizeof( m_start_step );
    s += sizeof( m_last_step );
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
    s += m_server_side_variable_range.byteSize();
    s += sizeof( m_flag_send_bins );
    s += sizeof( m_particle_limit );
    s += sizeof( m_particle_density );
    s += sizeof( m_particle_data_size_limit );
    s += jpv::Serializer::byteSize<kvs::Camera>( *m_camera );
    // if ( m_flag_send_bins == 1 || m_flag_send_bins == 2 )
        if ( m_flag_send_bins == 1 )
    {
        s += sizeof( m_transfer_function_count );
        for ( int i = 0; i < m_transfer_function_count; i++ )
        {
            s += sizeof( m_color_nbins[i] );
            s += sizeof( m_color_bins[i] ) * m_color_nbins[i];
            s += sizeof( m_opacity_nbins[i] );
            s += sizeof( m_opacity_bins[i] ) * m_opacity_nbins[i];
        }

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
        //s += jpv::Serializer::byteSize( transferFunctionSynthesis );
        s += jpv::Serializer::byteSize( m_color_transfer_function_synthesis );
        s += jpv::Serializer::byteSize( m_opacity_transfer_function_synthesis );
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
        return 0;
    }
    // MODIFIED START FEAST 2015.12.23
    index += jpv::Serializer::write( buf + index, m_time_step );
    index += jpv::Serializer::write( buf + index, m_subpixel_level );
    index += jpv::Serializer::write( buf + index, m_repeat_level );
    index += jpv::Serializer::write( buf + index, m_level_index );
    index += jpv::Serializer::write( buf + index, m_number_particle );
    index += jpv::Serializer::write( buf + index, m_number_glyph );
    index += jpv::Serializer::write( buf + index, m_number_volume_divide );
    index += jpv::Serializer::write( buf + index, m_start_step );
    index += jpv::Serializer::write( buf + index, m_last_step );
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
    index += m_server_side_variable_range.pack( buf + index );
    index += jpv::Serializer::write( buf + index, m_flag_send_bins );
    index += jpv::Serializer::write( buf + index, m_particle_limit );
    index += jpv::Serializer::write( buf + index, m_particle_density );
    index += jpv::Serializer::write( buf + index, m_particle_data_size_limit );
    index += jpv::Serializer::pack( buf + index, *m_camera );
    // if ( m_flag_send_bins == 1 || m_flag_send_bins == 2)
        if ( m_flag_send_bins == 1 )
    {
        index += jpv::Serializer::write( buf + index, m_transfer_function_count );
        for ( int i = 0; i < m_transfer_function_count; i++ )
        {
            index += jpv::Serializer::write( buf + index, m_color_nbins[i] );
            index += jpv::Serializer::writeArray<kvs::UInt64>( buf + index, m_color_bins[i], m_color_nbins[i] );
            index += jpv::Serializer::write( buf + index, m_opacity_nbins[i] );
            index += jpv::Serializer::writeArray<kvs::UInt64>( buf + index, m_opacity_bins[i], m_opacity_nbins[i] );
        }

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
        //index += jpv::Serializer::write( buf + index, transferFunctionSynthesis );
        index += jpv::Serializer::write( buf + index, m_color_transfer_function_synthesis );
        index += jpv::Serializer::write( buf + index, m_opacity_transfer_function_synthesis );
    }
//    else// if ( flag_send_bins == 2 )

    if ( m_flag_send_bins == 0  )
    {
        index += jpv::Serializer::writeArray<float>( buf + index, m_positions.get(), 3 * m_number_particle );
        index += jpv::Serializer::writeArray<float>( buf + index, m_normals.get(), 3 * m_number_particle );
        index += jpv::Serializer::writeArray<unsigned char>( buf + index, m_colors.get(), 3 * m_number_particle );
    }
    else if ( m_flag_send_bins == 2 )
    {
        index += jpv::Serializer::writeArray<float>( buf + index, m_glyph_coords.get(),    3 * m_number_glyph );
        index += jpv::Serializer::writeArray<float>( buf + index, m_glyph_vectors.get(),      3 * m_number_glyph );
        index += jpv::Serializer::writeArray<float>( buf + index, m_glyph_sizes.get(),          m_number_glyph );
        index += jpv::Serializer::writeArray<unsigned char>( buf + index, m_glyph_colors.get(), 3 * m_number_glyph );
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
    index += jpv::Serializer::read( buf + index, &m_number_glyph );
    index += jpv::Serializer::read( buf + index, &m_number_volume_divide );
    index += jpv::Serializer::read( buf + index, &m_start_step );
    index += jpv::Serializer::read( buf + index, &m_last_step );
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
    index += m_server_side_variable_range.unpack( buf + index );
    index += jpv::Serializer::read( buf + index, &m_flag_send_bins );
    index += jpv::Serializer::read( buf + index, &m_particle_limit );
    index += jpv::Serializer::read( buf + index, &m_particle_density );
    index += jpv::Serializer::read( buf + index, &m_particle_data_size_limit );
    index += jpv::Serializer::unpack( buf + index, m_camera );
    // if ( m_flag_send_bins == 1 || m_flag_send_bins == 2)
    if ( m_flag_send_bins == 1 )
    {
        index += jpv::Serializer::read( buf + index, &m_transfer_function_count );
        m_color_nbins = new kvs::UInt64[ m_transfer_function_count ];
        m_opacity_nbins = new kvs::UInt64[ m_transfer_function_count ];

        m_color_bins.resize( m_transfer_function_count );
        m_opacity_bins.resize( m_transfer_function_count );
        //        m_color_bin_names.resize( m_transfer_function_count );
        //        m_opacity_bin_names.resize( m_transfer_function_count );
        for ( int i = 0; i < m_transfer_function_count; i++ )
        {
            index += jpv::Serializer::read( buf + index, &m_color_nbins[i] );
            m_color_bins[i] =  new kvs::UInt64[ m_color_nbins[i] ];
            index += jpv::Serializer::readArray<kvs::UInt64>( buf + index, m_color_bins[i], m_color_nbins[i] );
            index += jpv::Serializer::read( buf + index, &m_opacity_nbins[i] );
            m_opacity_bins[i] =  new kvs::UInt64[ m_opacity_nbins[i] ];
            index += jpv::Serializer::readArray<kvs::UInt64>( buf + index, m_opacity_bins[i], m_opacity_nbins[i] );

            // add by @hira at 2016/12/01
            //            index += jpv::Serializer::read( buf + index, &m_color_bin_names[i] );
            //            index += jpv::Serializer::read( buf + index, &m_opacity_bin_names[i] );

        }
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
                index += jpv::Serializer::unpack<kvs::TransferFunction>( buf + index, &m_transfer_function[i] );
            }
        }
        index += jpv::Serializer::read( buf + index, &s );
        for ( size_t i = 0; i < s; i++ )
        {
            m_volume_equation.push_back( VolumeEquation() );
            index += jpv::Serializer::read( buf + index, &m_volume_equation[i].m_name );
            index += jpv::Serializer::read( buf + index, &m_volume_equation[i].m_equation );
        }
        //        index += jpv::Serializer::read( buf + index, transferFunctionSynthesis );
        index += jpv::Serializer::read( buf + index, &m_color_transfer_function_synthesis );
        index += jpv::Serializer::read( buf + index, &m_opacity_transfer_function_synthesis );

    }

    return index;
}

size_t jpv::ParticleTransferServerMessage::unpack_particles( const char* buf )
{
    size_t index = m_message_size;
    // m_positions = new float[3 * m_number_particle];
    // m_normals = new float[3 * m_number_particle];
    // m_colors = new unsigned char[3 * m_number_particle];
    m_positions = std::make_unique<float[]>(3 * m_number_particle);
    m_normals = std::make_unique<float[]>(3 * m_number_particle);
    m_colors = std::make_unique<unsigned char[]>(3 * m_number_particle);
    index += jpv::Serializer::readArray<float>( buf + index, m_positions.get(), 3 * m_number_particle );
    index += jpv::Serializer::readArray<float>( buf + index, m_normals.get(), 3 * m_number_particle );
    index += jpv::Serializer::readArray<unsigned char>( buf + index, m_colors.get(), 3 * m_number_particle );
    return index;
}

size_t jpv::ParticleTransferServerMessage::unpack_glyphs( const char* buf )
{
    size_t index = m_message_size;
    // m_positions = new float[3 * m_number_particle];
    // m_normals = new float[3 * m_number_particle];
    // m_colors = new unsigned char[3 * m_number_particle];
    m_glyph_coords = std::make_unique<float[]>(3 * m_number_glyph);
    m_glyph_vectors = std::make_unique<float[]>(3 * m_number_glyph);
    m_glyph_sizes = std::make_unique<float[]>(  m_number_glyph);
    m_glyph_colors = std::make_unique<unsigned char[]>(3 * m_number_glyph);
    index += jpv::Serializer::readArray<float>( buf + index, m_glyph_coords.get(), 3 * m_number_glyph );
    index += jpv::Serializer::readArray<float>( buf + index, m_glyph_vectors.get(), 3 * m_number_glyph );
    index += jpv::Serializer::readArray<float>( buf + index, m_glyph_sizes.get(),  m_number_glyph );
    index += jpv::Serializer::readArray<unsigned char>( buf + index, m_glyph_colors.get(), 3 * m_number_glyph );
    return index;
}

//add by shimomura 2022/12/23
/**
 * Color Histogram用のデータを設定する。
 * @param histogram_size        Histogram数
 * @param nbins                 Histogramデータ数
 * @param c_bins            Color Histogram用のデータ = histogram_size*nbins
 * @param transfer_function_names   すべての1次伝達関数名
 * @param transfunc_synthesizer_names   Histogram用のデータの1次伝達関数名
 */
void jpv::ParticleTransferServerMessage::setColorHistogramBins(
    int histogram_size,
    int nbins,
    const kvs::UInt64* arg_c_bins)
//const kvs::UInt64* arg_c_bins,
//const std::vector<std::string> &transfer_function_names,
//const std::vector<std::string> &transfunc_synthesizer_names)
{
    for ( int synth_tf = 0; synth_tf < histogram_size; synth_tf++ ) {
        this->m_color_nbins[synth_tf] = nbins;
        for ( int res = 0; res < nbins; res++ ) {
            int n = synth_tf*nbins + res;
            this->m_color_bins[synth_tf][res] = arg_c_bins[n];
        }
    }

    return;
}

/**
 * Opacity Histogram用のデータを設定する。
 * @param histogram_size        Histogram数
 * @param nbins                 Histogramデータ数
 * @param o_bins            Opacity Histogram用のデータ = histogram_size*nbins
 * @param transfer_function_names   すべての1次伝達関数名
 * @param transfunc_synthesizer_names   Histogram用のデータの1次伝達関数名
 */
void jpv::ParticleTransferServerMessage::setOpacityHistogramBins(
    int histogram_size,
    int nbins,
    const kvs::UInt64* arg_o_bins)
//const kvs::UInt64* arg_o_bins,
//const std::vector<std::string> &transfer_function_names,
//const std::vector<std::string> &transfunc_synthesizer_names)
{
    for ( int synth_tf = 0; synth_tf < histogram_size; synth_tf++ ) {
        this->m_opacity_nbins[synth_tf] = nbins;
        for ( int res = 0; res < nbins; res++ ) {
            int n = synth_tf*nbins + res;
            this->m_opacity_bins[synth_tf][res] = arg_o_bins[n];
        }
    }

    //std::stringstream debug11;
    //debug11 << "o_bins[1] = {";
    //for(int i =0; i< nbins; i++) debug11 << o_bins[1][i] << "," ;
    //std::cout << debug11.str() << std::endl;

    return;
}

/**
 * 関数の領域確保、初期化を行う
 * @param transfer_function_count       関数数
 * @param nbins     解像度
 */
void jpv::ParticleTransferServerMessage::initializeTransferFunction(
    const int32_t transfer_function_count,
    const int nbins)
{

    this->m_transfer_function_count = transfer_function_count;

    this->m_color_nbins = new kvs::UInt64[transfer_function_count];
    this->m_opacity_nbins = new kvs::UInt64[transfer_function_count];

    this->m_color_bins.resize( transfer_function_count );
    this->m_opacity_bins.resize( transfer_function_count );
//    this->m_color_bin_names.resize( transfer_function_count );
//    this->m_opacity_bin_names.resize( transfer_function_count );

    for ( int tf = 0; tf < this->m_transfer_function_count; tf++ )
    {
        this->m_color_nbins[tf] = nbins;
        this->m_opacity_nbins[tf] = nbins;
        this->m_color_bins[tf] =  new kvs::UInt64[ this->m_color_nbins[tf] ];
        this->m_opacity_bins[tf] =  new kvs::UInt64[ this->m_opacity_nbins[tf] ];
        for ( kvs::UInt64 res = 0; res < this->m_color_nbins[tf]; res++ )
        {
            this->m_color_bins[tf][res] = 0;
        }
        for ( kvs::UInt64 res = 0; res < this->m_opacity_nbins[tf]; res++ )
        {
            this->m_opacity_bins[tf][res] = 0;
        }
//        this->m_color_bin_names[tf].clear();
//        this->m_opacity_bin_names[tf].clear();
    }

    return;
}

//2019 kawamura
void jpv::ParticleTransferServerMessage::show( void ) const
{
    std::cout<<"SHOW SERVER MESSAGE"<<std::endl;
    std::cout<<"header="<<m_header<<std::endl;
    std::cout<<"messageSize="<<m_message_size<<std::endl;
    std::cout<<"timeStep="<<m_time_step<<std::endl;
    std::cout<<"numParticle="<<m_number_particle<<std::endl;
    std::cout<<"numGlyph="<<m_number_glyph<<std::endl;

    std::cout<<"transfunc.size="<<m_transfer_function.size()<<std::endl;
    std::cout<<"transfunc.Name,ColorVar,OpacityVar,ColorVarMin,ColorVarMax"<<std::endl;
    for(int i=0; i<m_transfer_function.size(); i++)
    {
        std::cout<<m_transfer_function[i].m_name<<","<<m_transfer_function[i].m_color_variable<<","<<m_transfer_function[i].m_opacity_variable<<","<<m_transfer_function[i].m_color_variable_min<<","<<m_transfer_function[i].m_color_variable_max<<std::endl;
    }

    // std::cout << "c_bin = {" << std::endl;
    // for (int n = 0; n < m_transfer_function_count; n++)
    // {
    //     for(int i=0; i<256; i++) std::cout << m_color_bins[n][i] << " " ;
    //     std::cout << std::endl;
    // }
    std::cout<<"voleqn.size="<<m_volume_equation.size()<<std::endl;
    std::cout<<"voleqn.Name,Equation"<<std::endl;
    for(int i=0; i<m_volume_equation.size(); i++)
    {
        std::cout<<m_volume_equation[i].m_name<<","<<m_volume_equation[i].m_equation<<std::endl;
    }

    //std::cout<<"transferFunctionSynthesis="<<transferFunctionSynthesis<<std::endl;
    std::cout << "color_tf_synthesis=" << m_color_transfer_function_synthesis << std::endl;
    std::cout << "opacity_tf_synthesis=" << m_opacity_transfer_function_synthesis << std::endl;

    for ( int i = 0; i < m_number_glyph; ++i )
    {
        std::cout << "servMes.m_glyph_coords[3 * i + 0] = " <<  m_glyph_coords[3 * i + 0] << std::endl;
        std::cout << "servMes.m_glyph_coords[3 * i + 1] = " <<  m_glyph_coords[3 * i + 1] << std::endl;
        std::cout << "servMes.m_glyph_coords[3 * i + 2] = " <<  m_glyph_coords[3 * i + 2] << std::endl;
        std::cout << "servMes.m_glyph_vectors[3 * i + 0] = " <<  m_glyph_vectors[3 * i + 0] << std::endl;
        std::cout << "servMes.m_glyph_vectors[3 * i + 1] = " <<  m_glyph_vectors[3 * i + 1] << std::endl;
        std::cout << "servMes.m_glyph_vectors[3 * i + 2] = " <<  m_glyph_vectors[3 * i + 2] << std::endl;
        std::cout << "servMes.m_glyph_colors[3 * i + 0] = " <<  (int)m_glyph_colors[3 * i + 0] << std::endl;
        std::cout << "servMes.m_glyph_colors[3 * i + 1] = " <<  (int)m_glyph_colors[3 * i + 1] << std::endl;
        std::cout << "servMes.m_glyph_colors[3 * i + 2] = " <<  (int)m_glyph_colors[3 * i + 2] << std::endl;
        std::cout << "servMes.m_glyph_sizes[  i ] = " <<  m_glyph_sizes[ i ] << std::endl;
    }


    std::cout<<std::endl;
}

