#ifndef PBVR__POINT_OBJECT_GENERATOR_H_INCLUDE
#define PBVR__POINT_OBJECT_GENERATOR_H_INCLUDE

#include <vector>
#include <string>
#include "UnstructuredVolumeObject.h"
#include "FilterInformation.h"

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

    void setCoordSynthTS( const int ts )
    {
        m_coord_synthesizer_strings.m_time_step = ts;
    }

    pbvr::CoordSynthesizerStrings getCoordSynthStrs() const
    {
        return m_coord_synthesizer_strings;
    }

private:
    pbvr::PointObject* sampling( const Argument& param, const kvs::Camera& camera, pbvr::VolumeObjectBase* volume, const size_t subpixel_level, const float sampling_step );

};

}

#endif    // PBVR__POINT_OBJECT_GENERATOR_H_INCLUDE
