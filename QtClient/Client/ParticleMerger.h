#ifndef PBVR__KVS__VISCLIENT__PBVR_PARTICLE_MERGER_H_INCLUDE
#define PBVR__KVS__VISCLIENT__PBVR_PARTICLE_MERGER_H_INCLUDE

#include <string>
#include <vector>
#include <kvs/PointObject>
#include <kvs/PolygonObject>
#include "ParticleMergeParameter.h"

namespace kvs
{
namespace visclient
{

class ParticleMerger
{

protected:

    ParticleMergeParameter m_parameter;
    ParticleMergeParameter m_last_parameter;

    std::vector<size_t> m_initial_step;
    std::vector<size_t> m_final_step;
    std::vector<size_t> m_division;
    std::vector<std::string> m_file_prefix;
    float m_x_min, m_y_min, m_z_min;
    float m_x_max, m_y_max, m_z_max;
    kvs::Real32 m_local_object_subpixel_level; /*//粒子統合時に最後に入力されたローカルファイルのサブピクセルレベルが入る。 */
    kvs::Real32 m_server_side_subpixel_level;

public:

    ParticleMerger();
    virtual ~ParticleMerger();

    void setParam( const ParticleMergeParameter& param, const size_t server_init_step, const size_t server_fin_step );
    bool calculateExternalCoords( float* crd );
    size_t getMergedInitialTimeStep();
    size_t getMergedFinalTimeStep();
    kvs::PointObject* doMerge(
        const kvs::PointObject* server_particle,
        const size_t step ,
            bool is_client_server_mode);
    void setEnable( const int i );

    bool isPolygonEnable(int);
    std::string getPolygonFilePath(int);
    double getPolygon_opacity(int);
    kvs::RGBColor getPolygonColor(int);
    size_t getLocalObjectInitialStep(int);
    size_t getLocalObjectFinalStep(int);
    bool getLocalObjectIsEnableKeepInitial(int);
    bool getLocalObjectIsEnableKeepFinal(int);
};

} // namespace visclient
} // namespace kvs

#endif // PBVR__KVS__VISCLIENT__PBVR_PARTICLE_MERGER_H_INCLUDE

