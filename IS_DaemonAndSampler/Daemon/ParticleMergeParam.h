#ifndef __KVS_VISCLIENT_PBVR_PARTICLE_MERGE_PARAM_H__
#define __KVS_VISCLIENT_PBVR_PARTICLE_MERGE_PARAM_H__

#include <string>
#include <vector>

namespace kvs
{
namespace visclient
{

class ParticleMergeParam
{

protected:
    class ParticleData
    {
    public:
        bool enable;
        bool keep_initial_step;
        bool keep_final_step;
        std::string filepath;
        float xmin, ymin, zmin;
        float xmax, ymax, zmax;

    public:
        ParticleData():
            enable( false ),
            keep_initial_step( false ),
            keep_final_step( false )
        {
        }

        virtual ~ParticleData()
        {
        }
    };

public:
    bool doExport;
    std::string export_filepath;
    std::vector<ParticleData> particles; // 0:server 1-10:file
    int crt_time;

public:
    ParticleMergeParam():
        particles( 11 ),
        doExport( false )
    {
        particles[0].filepath = "server";
        particles[0].enable = true;
    }

    virtual ~ParticleMergeParam()
    {
    }

};

} // namespace visclient
} // namespace kvs

#endif // __KVS_VISCLIENT_PBVR_PARTICLE_MERGE_PARAM_H__
