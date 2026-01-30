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
    TransferFunctionSynthesizer* m_transfunc_synthesizer;
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
};

#endif //  __PARTICLE__PROPERTY_