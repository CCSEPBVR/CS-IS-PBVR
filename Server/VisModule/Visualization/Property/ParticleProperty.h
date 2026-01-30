#ifndef __PARTICLE_PROPERTY__
#define __PARTICLE_PROPERTY__

#include <string>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/ExtendedTransferFunction>

enum ServerMode
{
    CS = 0,
    IS = 1
};

struct ParticleProperty
{
    // property for visualization
    size_t m_subpixel_level;
    size_t m_repeat_level;
    int m_level_index;
    float m_sampling_step;
    char m_sampling_method;
    double m_latency_threshold;
    int m_job_id_pack_size;
    int m_particle_limit;
    float m_extra_opacity_factor;
    float m_particle_data_size_limit;
    vismodule::Camera* m_camera;
    TransferFunctionSynthesizer* m_transfunc_synthesizer; // pack unpackしない
    std::vector<NamedTransferFunction> m_transfunc_array;
    std::string m_color_transfer_function_synthesis;
    std::string m_opacity_transfer_function_synthesis;
    std::string m_x_synthesis;
    std::string m_y_synthesis;
    std::string m_z_synthesis;

    void UpdateTransferFunctionSynthesizer()
    {
        EquationToken color_equation_token;
        std::string colorFunctionSynthesizerBuf = m_color_transfer_function_synthesis;
        std::replace( colorFunctionSynthesizerBuf.begin(), colorFunctionSynthesizerBuf.end(), 'C', 'c' );
        color_equation_token = m_transfunc_synthesizer->convert_token( colorFunctionSynthesizerBuf );
        m_transfunc_synthesizer->setColorFunction( color_equation_token );

        EquationToken opacity_equation_token;
        std::string opacityFunctionSynthesizerBuf = m_opacity_transfer_function_synthesis;
        std::replace( opacityFunctionSynthesizerBuf.begin(), opacityFunctionSynthesizerBuf.end(), 'O', 'a' );
        opacity_equation_token = m_transfunc_synthesizer->convert_token( opacityFunctionSynthesizerBuf );
        m_transfunc_synthesizer->setOpacityFunction( opacity_equation_token );

        std::vector<EquationToken> var_c;
        std::vector<EquationToken> var_o;

        for( size_t i = 0; i < m_transfunc_array.size(); ++i )
        {
            // Color variable token
            {
                std::string buf = m_transfunc_array[i].m_color_variable;
                std::replace( buf.begin(), buf.end(), 'X', 'x' );
                std::replace( buf.begin(), buf.end(), 'Y', 'y' );
                std::replace( buf.begin(), buf.end(), 'Z', 'z' );
                var_c.push_back( m_transfunc_synthesizer->convert_token( buf ) );
            }

            // Opacity variable token
            {
                std::string buf = m_transfunc_array[i].m_opacity_variable;
                std::replace( buf.begin(), buf.end(), 'X', 'x' );
                std::replace( buf.begin(), buf.end(), 'Y', 'y' );
                std::replace( buf.begin(), buf.end(), 'Z', 'z' );
                var_o.push_back( m_transfunc_synthesizer->convert_token( buf ) );
            }
        }

        m_transfunc_synthesizer->setColorVariable( var_c );
        m_transfunc_synthesizer->setOpacityVariable( var_o );
    }
    
    int32_t byteSize() const
    {
        int32_t size = 0;
        TaskSignal task_signal = TaskSignal::GENERATE_PARTICLE;

        size += sizeof( task_signal );
        size += sizeof( m_subpixel_level );
        size += sizeof( m_repeat_level );
        size += sizeof( m_level_index );
        size += sizeof( m_sampling_step );
        size += sizeof( m_sampling_method );
        size += sizeof( m_latency_threshold );
        size += sizeof( m_job_id_pack_size );
        size += sizeof( m_particle_limit );
        size += sizeof( m_extra_opacity_factor );
        size += sizeof( m_particle_data_size_limit );
        size += vismodule::Serializer::byteSize<vismodule::Camera>( *m_camera );
        size += vismodule::Serializer::byteSize( m_transfunc_array.size() );
        for ( size_t i = 0; i < m_transfunc_array.size(); i++ )
        {
            size += vismodule::Serializer::byteSize( m_transfunc_array[i].m_resolution );
            size += vismodule::Serializer::byteSize( m_transfunc_array[i].m_name );
            size += vismodule::Serializer::byteSize( m_transfunc_array[i].m_server_color_range_mode );
            size += vismodule::Serializer::byteSize( m_transfunc_array[i].m_server_opacity_range_mode );
            size += vismodule::Serializer::byteSize( m_transfunc_array[i].m_color_variable );
            size += vismodule::Serializer::byteSize( m_transfunc_array[i].m_opacity_variable );
            size += vismodule::Serializer::byteSize( m_transfunc_array[i].m_user_color_variable_min );
            size += vismodule::Serializer::byteSize( m_transfunc_array[i].m_user_color_variable_max );
            size += vismodule::Serializer::byteSize( m_transfunc_array[i].m_user_opacity_variable_min );
            size += vismodule::Serializer::byteSize( m_transfunc_array[i].m_user_opacity_variable_max );
            size += vismodule::Serializer::byteSize<vismodule::TransferFunction>( m_transfunc_array[i] );
        }
        size += sizeof( m_color_transfer_function_synthesis );
        size += sizeof( m_opacity_transfer_function_synthesis );
        size += sizeof( size_t );
        size += sizeof( char ) * ( m_x_synthesis.size() + 1 );
        size += sizeof( size_t );
        size += sizeof( char ) * ( m_y_synthesis.size() + 1 );
        size += sizeof( size_t );
        size += sizeof( char ) * ( m_z_synthesis.size() + 1 );

        return size;
    }

    size_t pack( char* buf ) const
    {
        size_t index = 0;
        TaskSignal task_signal = TaskSignal::GENERATE_PARTICLE;

        index += vismodule::Serializer::write( buf + index, task_signal );
        index += vismodule::Serializer::write( buf + index, m_subpixel_level );
        index += vismodule::Serializer::write( buf + index, m_repeat_level );
        index += vismodule::Serializer::write( buf + index, m_level_index );
        index += vismodule::Serializer::write( buf + index, m_sampling_step );
        index += vismodule::Serializer::write( buf + index, m_sampling_method );
        index += vismodule::Serializer::write( buf + index, m_latency_threshold );
        index += vismodule::Serializer::write( buf + index, m_job_id_pack_size );
        index += vismodule::Serializer::write( buf + index, m_particle_limit );
        index += vismodule::Serializer::write( buf + index, m_extra_opacity_factor );
        index += vismodule::Serializer::write( buf + index, m_particle_data_size_limit );
        index += vismodule::Serializer::pack( buf + index, *m_camera );

        index += vismodule::Serializer::write( buf + index, m_transfunc_array.size() );
        for ( size_t i = 0; i < m_transfunc_array.size(); i++ )
        {
            index += vismodule::Serializer::write( buf + index, m_transfunc_array[i].m_resolution );
            index += vismodule::Serializer::write( buf + index, m_transfunc_array[i].m_name );
            index += vismodule::Serializer::write( buf + index, m_transfunc_array[i].m_server_color_range_mode );
            index += vismodule::Serializer::write( buf + index, m_transfunc_array[i].m_server_opacity_range_mode );
            index += vismodule::Serializer::write( buf + index, m_transfunc_array[i].m_color_variable );
            index += vismodule::Serializer::write( buf + index, m_transfunc_array[i].m_opacity_variable );
            index += vismodule::Serializer::write( buf + index, m_transfunc_array[i].m_user_color_variable_min );
            index += vismodule::Serializer::write( buf + index, m_transfunc_array[i].m_user_color_variable_max );
            index += vismodule::Serializer::write( buf + index, m_transfunc_array[i].m_user_opacity_variable_min );
            index += vismodule::Serializer::write( buf + index, m_transfunc_array[i].m_user_opacity_variable_max );
            index += jpv::Serializer::pack<vismodule::TransferFunction>( buf + index, m_transfunc_array[i] );
        }

        index += vismodule::Serializer::write( buf + index, m_color_transfer_function_synthesis );
        index += vismodule::Serializer::write( buf + index, m_opacity_transfer_function_synthesis );
        index += vismodule::Serializer::write( buf + index, m_x_synthesis.size() );
        index += vismodule::Serializer::writeArray( buf + index, m_x_synthesis.c_str(), m_x_synthesis.size() + 1 );
        index += vismodule::Serializer::write( buf + index, m_y_synthesis.size() );
        index += vismodule::Serializer::writeArray( buf + index, m_y_synthesis.c_str(), m_y_synthesis.size() + 1 );
        index += vismodule::Serializer::write( buf + index, m_z_synthesis.size() );
        index += vismodule::Serializer::writeArray( buf + index, m_z_synthesis.c_str(), m_z_synthesis.size() + 1 );

        return index;
    }

    size_t unpack( const char* buf )
    {
        size_t index = 0;
        int64_t tmp_char_size;
        char* tmp_char = NULL;
        TaskSignal task_signal;

        index += vismodule::Serializer::read( buf + index, &task_signal );
        index += vismodule::Serializer::read( buf + index, &m_subpixel_level );
        index += vismodule::Serializer::read( buf + index, &m_repeat_level );
        index += vismodule::Serializer::read( buf + index, &m_level_index );
        index += vismodule::Serializer::read( buf + index, &m_sampling_step );
        index += vismodule::Serializer::read( buf + index, &m_sampling_method );
        index += vismodule::Serializer::read( buf + index, &m_latency_threshold );
        index += vismodule::Serializer::read( buf + index, &m_job_id_pack_size );
        index += vismodule::Serializer::read( buf + index, &m_particle_limit );
        index += vismodule::Serializer::read( buf + index, &m_extra_opacity_factor );
        index += vismodule::Serializer::read( buf + index, &m_particle_data_size_limit );
        index += vismodule::Serializer::read( buf + index, m_camera );

        size_t size;
        index += vismodule::Serializer::read( buf + index, &size );
        m_transfunc_array.clear();
        for ( size_t i = 0; i < size; i++ )
        {
            m_transfunc_array.push_back( NamedTransferFunction() );
            index += vismodule::Serializer::read( buf + index, &m_transfunc_array[i].m_resolution );
            index += vismodule::Serializer::read( buf + index, &m_transfunc_array[i].m_name );
            index += vismodule::Serializer::read( buf + index, &m_transfunc_array[i].m_server_color_range_mode );
            index += vismodule::Serializer::read( buf + index, &m_transfunc_array[i].m_server_opacity_range_mode );
            index += vismodule::Serializer::read( buf + index, &m_transfunc_array[i].m_color_variable );
            index += vismodule::Serializer::read( buf + index, &m_transfunc_array[i].m_opacity_variable );
            index += vismodule::Serializer::read( buf + index, &m_transfunc_array[i].m_user_color_variable_min );
            index += vismodule::Serializer::read( buf + index, &m_transfunc_array[i].m_user_color_variable_max );
            index += vismodule::Serializer::read( buf + index, &m_transfunc_array[i].m_user_opacity_variable_min );
            index += vismodule::Serializer::read( buf + index, &m_transfunc_array[i].m_user_opacity_variable_max );
            index += jpv::Serializer::pack<vismodule::TransferFunction>( buf + index, &m_transfunc_array[i] );
        }

        index += vismodule::Serializer::read( buf + index, &m_color_transfer_function_synthesis );
        index += vismodule::Serializer::read( buf + index, &m_opacity_transfer_function_synthesis );

        index += vismodule::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += vismodule::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_x_synthesis = std::string( tmp_char );
        delete tmp_char;

        index += vismodule::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += vismodule::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_y_synthesis = std::string( tmp_char );
        delete tmp_char;

        index += vismodule::Serializer::read( buf + index, &tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += vismodule::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_z_synthesis = std::string( tmp_char );
        delete tmp_char;

        return index;
    }

    void show( int rank ) const
    {
        std::string sampling_method_string;

        if ( m_sampling_method == 'u' )
        {
            sampling_method_string = "uniform";
        }
        else if ( m_sampling_method == 'm' )
        {
            sampling_method_string = "metropolis";
        }
        else if ( m_sampling_method == 'r' )
        {
            sampling_method_string = "rejection";
        }
        else
        {
            std::cout << "ERROR: unknown sampling method." << std::endl;
            sampling_method_string = "unknown";
        }

        std::cout << "===================== Particle Property ( rank : " << rank << ") START ====================" << std::endl;
        std::cout << "subpixel_level                      : " << m_subpixel_level                      << std::endl;
        std::cout << "repeat_level                        : " << m_repeat_level                        << std::endl;
        std::cout << "level_index                         : " << m_level_index                         << std::endl;
        std::cout << "sampling_step                       : " << m_sampling_step                       << std::endl;
        std::cout << "sampling_method                     : " << sampling_method_string                << std::endl;
        std::cout << "latency_threshold                   : " << m_latency_threshold                   << std::endl;
        std::cout << "job_id_pack_size                    : " << m_job_id_pack_size                    << std::endl;
        std::cout << "particle_limit                      : " << m_particle_limit                      << std::endl;
        std::cout << "extra_opacity_factor                : " << m_extra_opacity_factor                << std::endl;
        std::cout << "particle_data_size_limit            : " << m_particle_data_size_limit            << std::endl;
        std::cout << "color_transfer_function_synthesis   : " << m_color_transfer_function_synthesis   << std::endl;
        std::cout << "opacity_transfer_function_synthesis : " << m_opacity_transfer_function_synthesis << std::endl;
        std::cout << "x_synthesis                         : " << m_x_synthesis                         << std::endl;
        std::cout << "y_synthesis                         : " << m_y_synthesis                         << std::endl;
        std::cout << "z_synthesis                         : " << m_z_synthesis                         << std::endl;
        std::cout << "===================== Particle Property ( rank : " << rank << ") END ====================" << std::endl;
    }
};

#endif //  __PARTICLE__PROPERTY_