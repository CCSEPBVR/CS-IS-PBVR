#ifndef __PARTICLE_PROPERTY__
#define __PARTICLE_PROPERTY__

#include<string>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/TransferFunction>

struct ParticleProperty
{
	std::string filepath;
    int m_time_step;
    size_t m_subpixel_level;
    size_t m_repeat_level;
    int m_level_index;
    float m_sampling_step;
    char m_sampling_method;
    double m_latency_threshold;
    int m_job_id_pack_size;
    int m_particle_limit;
    float m_particle_density;
    float m_particle_data_size_limit;
    vismodule::Camera* m_camera;
    TransferFunctionSynthesizer* m_transfunc_synthesizer;
    std::vector<vismodule::TransferFunction> m_transfunc_array;
    std::string m_color_transfer_function_synthesis;
    std::string m_opacity_transfer_function_synthesis;
    std::string m_x_synthesis;
    std::string m_y_synthesis;
    std::string m_z_synthesis;
};

#endif //  __PARTICLE__PROPERTY_