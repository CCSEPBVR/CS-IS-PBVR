#ifndef PBVR__KVS__VISCLIENT__VISUALIZATION_PARAMETER_H_INCLUDE
#define PBVR__KVS__VISCLIENT__VISUALIZATION_PARAMETER_H_INCLUDE

#include <cstring>
#include <kvs/TransferFunction>
#include "kvs/Camera"
#include "ParamExTransFunc.h"
#include "ParticleMergeParameter.h"


namespace kvs
{
namespace visclient
{

class VisualizationParameter
{
public:
    enum ParticleShuffleType
    {
        RandomBased = 1,
        SingleShuffleBased = 2,
        CPUShuffleBased = 3,
        GPUShffleBased = 4,
        None = 0,
    };

    enum DetailedParticleTransferType
    {
        Summalized = 1,
        Divided = 2,
    };

    enum TransferType
    {
        Abstract = 1,
        Detailed = 2,
    };

    enum SamplingType
    {
        UniformSampling = 1,
        RejectionSampling = 2,
        MetropolisSampling = 3
    };

    enum NodeType
    {
        AllNodes = 1,
        SlaveNodes = 2,
    };

    std::string m_input_directory;
    std::string m_hostname;
    std::string m_particle_directory;
    int m_port;
    int m_time_step;
    int m_abstract_subpixel_level;
    int m_detailed_subpixel_level;
    int m_abstract_repeat_level;
    int m_detailed_repeat_level;
// APPEND START FP)K.YAJIMA 2015.02.27
    int m_resolution_width;
    int m_resolution_height;
// APPEND END FP)K.YAJIMA 2015.02.27
    int m_min_time_step;
    int m_max_time_step;
    int m_min_server_time_step;
    int m_max_server_time_step;
    int m_repeat_level;
    int m_client_server_mode;
    int m_rendering_id;
    int32_t m_particle_limit; // APPEND Fj) 2015.03.04
    float m_particle_density;
//#ifdef COMM_MODE_IS
    float particle_data_size_limit =20.0;
//#endif
    int m_time_step_key_frame;

    kvs::TransferFunction m_transfer_function;
    kvs::Camera* m_camera;
    TransferType m_transfer_type;
    ParticleShuffleType m_shuffle_type;
    DetailedParticleTransferType m_detailed_transfer_type;
    SamplingType m_sampling_type;
    NodeType m_node_type;

    bool m_shading_type_flag;
#ifdef COMM_MODE_IS
//    ParamExTransFunc paramExTransFunc;
    ExtendedTransferFunctionMessage m_parameter_extend_transfer_function;
#else
    ExtendedTransferFunctionMessage m_parameter_extend_transfer_function;
#endif

    int m_no_repsampling;
#ifdef COMM_MODE_CS
    std::string m_x_synthesis;
    std::string m_y_synthesis;
    std::string m_z_synthesis;

    // add by @hira at 2016/12/01
    std::string m_filter_parameter_filename;
    bool m_hasfin;
#endif

    //OSAKI
    std::string m_polygon_model_filename;
#ifdef ENABLE_LIBLAS
    std::string m_las_model_filename;
#endif
    //OSAKI

#ifdef DISP_MODE_VR
    bool m_lefty_mode;
#endif // DISP_MODE_VR

    VisualizationParameter() :
        m_time_step( 0 ),
        m_abstract_subpixel_level( 2 ),
        m_detailed_subpixel_level( 2 ),
        m_abstract_repeat_level( 4 ),
        m_detailed_repeat_level( 2 ), /* change default 16 to 4. 2014.07.21 */
// APPEND START FP)K.YAJIMA 2015.02.27
        m_resolution_width( 2 ),
        m_resolution_height( 2 ),
        m_particle_limit( 999999999 ),
// APPEND END FP)K.YAJIMA 2015.02.27
        m_min_time_step( 0 ),
        m_max_time_step( 1 ),
        m_max_server_time_step( 0 ),
        m_client_server_mode( 1 ),
        m_rendering_id( 0 ),
        m_shading_type_flag( false ),
        m_no_repsampling( 0 ),
#ifdef DISP_MODE_VR
        m_lefty_mode(false),
#endif // DISP_MODE_VR
        //OSAKI
#ifdef ENABLE_LIBLAS
        m_polygon_model_filename(""),
        m_las_model_filename("")
#else
        m_polygon_model_filename("")
#endif
        //OSAKI
    {
        m_repeat_level = m_detailed_repeat_level;
        m_shuffle_type = RandomBased;
        m_detailed_transfer_type = Summalized;
//		m_transfer_type = Abstract;
        m_transfer_type = Detailed; /* change default Abstract to Detailed. 2014.07.21 */
        m_sampling_type = UniformSampling;
        m_node_type = AllNodes;
    }

    ParticleMergeParameter m_particle_merge_param;
};

}
}

#endif
