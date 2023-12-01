#ifndef PBVR__POINT_OBJECT_GENERATOR_H_INCLUDE
#define PBVR__POINT_OBJECT_GENERATOR_H_INCLUDE

#include <vector>
#include <string>
#include "UnstructuredVolumeObject.h"
#include "FilterInformation.h"
#include "ExtendedTransferFunction.h"
#include "timer.h"

typedef struct
{
    //common
    double importing;

    //master
    double master_initialize;
    double master_set_parameters;
    double master_sampling;
//    double master_importing;
//    double master_writting;
    double master_jobdispatch; 
    double master_mpi_reduce;
    double master_recieving;
    double master_sending;

    //worker
    double worker_initialize;
    double worker_set_parameters;
    double worker_sampling;
//    double worker_writting;
    double worker_mpibcast;
    double worker_mpi_reduce;
    double worker_recieving;
    double worker_sending;
    int nparticles;
} time_parameters;                             

class Argument;
namespace kvs
{
class Camera;
}

namespace pbvr
{
class PointObject;
class VolumeObjectBase;
class UnstructuredVolumeObject;

class PointObjectGenerator
{
private:
    pbvr::PointObject* m_object;
    const FilterInformationFile*   m_fi;

    pbvr::CoordSynthesizerStrings m_coord_synthesizer_strings;
    pbvr::CoordSynthesizerTokens  m_coord_synthesizer_tokens;

public:

    PointObjectGenerator() : m_object( NULL ), m_fi(NULL) {};

    void createFromFile(
        const Argument& param, const kvs::Camera& camera, const size_t subpixel_level, const float sampling_step );

    void createFromFile(
        const Argument& param, const kvs::Camera& camera, const size_t subpixel_level, const float sampling_step, const int st, const int vl );

    pbvr::PointObject* getPointObject()
    {
        return m_object;
    }

    std::string getErrorMessage( const size_t maxMemory ) const;

    void setFinlterInfo( const FilterInformationFile *fi )
    {
        m_fi = fi;
    }

    void setCoordSynthStrs( const pbvr::CoordSynthesizerStrings& css )
    {
        m_coord_synthesizer_strings = css;
    }

    void setCoordSynthTkns( const pbvr::CoordSynthesizerTokens& cst )
    {
        m_coord_synthesizer_tokens.m_x_coord_synthesizer_token = cst.m_x_coord_synthesizer_token;
        m_coord_synthesizer_tokens.m_y_coord_synthesizer_token = cst.m_y_coord_synthesizer_token;
        m_coord_synthesizer_tokens.m_z_coord_synthesizer_token = cst.m_z_coord_synthesizer_token;
    }

    void setCoordSynthTS( const int ts )
    {
        m_coord_synthesizer_strings.m_time_step = ts;
    }

    pbvr::CoordSynthesizerStrings getCoordSynthStrs() const
    {
        return m_coord_synthesizer_strings;
    }

    pbvr::CoordSynthesizerTokens  getCoordSynthTkns() const
    {
        return m_coord_synthesizer_tokens;
    }


private:
    pbvr::PointObject* sampling( const Argument& param, const kvs::Camera& camera, pbvr::VolumeObjectBase* volume, const size_t subpixel_level, const float sampling_step );

};

}

#endif    // PBVR__POINT_OBJECT_GENERATOR_H_INCLUDE
