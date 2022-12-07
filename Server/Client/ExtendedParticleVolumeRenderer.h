#ifndef PBVR__KVS__VISCLIENT__EXTENDED_PARTICLE_VOLUME_RENDERER_H_INCLUDE
#define PBVR__KVS__VISCLIENT__EXTENDED_PARTICLE_VOLUME_RENDERER_H_INCLUDE

#ifdef CPUMODE
#include <kvs/ParticleVolumeRenderer>
#else
#include "kvs/glew/ParticleVolumeRenderer"
#endif

namespace kvs
{
namespace visclient
{

#ifdef CPUMODE
class ExtendedParticleVolumeRenderer : public kvs::ParticleVolumeRenderer
#else
class ExtendedParticleVolumeRenderer : public kvs::glew::ParticleVolumeRenderer
#endif
{
public:

    ExtendedParticleVolumeRenderer( const kvs::PointObject& point, const size_t subpixel_level = 1, const size_t repeat_level = 1 ):
#ifdef CPUMODE
        kvs::ParticleVolumeRenderer( &point, subpixel_level )
#else
        kvs::glew::ParticleVolumeRenderer( &point, subpixel_level, repeat_level )
#endif
    {}

    ~ExtendedParticleVolumeRenderer() {};


    void changePointObject( const kvs::PointObject& point )
    {
        this->attachPointObject( &point );
#ifndef CPUMODE
        this->align_particles();
        this->create_vertexbuffer();
#endif
    }

#ifdef CPUMODE
    void recreateImageBuffer()
    {
        this->delete_particle_buffer();
        this->create_particle_buffer(
            BaseClass::m_width,
            BaseClass::m_height,
            m_subpixel_level );
    }
#endif

};

}
}

#endif    // PBVR__KVS__VISCLIENT__EXTENDED_PARTICLE_VOLUME_RENDERER_H_INCLUDE
