#ifndef PBVR__KVS__VISCLIENT__PARTICLE_MERGE_PARAM_H_INCLUDE
#define PBVR__KVS__VISCLIENT__PARTICLE_MERGE_PARAM_H_INCLUDE

#include <string>
#include <vector>

namespace kvs
{
namespace visclient
{

class ParticleMergeParameter
{

protected:
    class ParticleData
    {
    public:
        bool m_enable;
        bool m_keep_initial_step;
        bool m_keep_final_step;
        std::string m_file_path;
        double m_polygon_opacity = 0.5;
        kvs::RGBColor m_polygon_color;
        bool is_polygon_display = true;
        float m_x_min, m_y_min, m_z_min;
        float m_x_max, m_y_max, m_z_max;

    public:
        ParticleData():
            m_enable( false ),
            m_keep_initial_step( false ),
            m_keep_final_step( false )
        {
        }

        virtual ~ParticleData()
        {
        }
    };

public:
    bool m_do_export;
    std::string m_export_file_path;
    //Notice 0~5は粒子,6~10はポリゴンデータです。
    std::vector<ParticleData> m_particles; // 0:server 1-10:file
    int m_circuit_time;

public:
    ParticleMergeParameter():
        m_particles( 11 ),
        m_do_export( false )
    {
        m_particles[0].m_file_path = "server";
        m_particles[0].m_enable = true;
    }

    virtual ~ParticleMergeParameter()
    {
    }

};

} // namespace visclient
} // namespace kvs

#endif // PBVR__KVS__VISCLIENT__PARTICLE_MERGE_PARAM_H_INCLUDE
