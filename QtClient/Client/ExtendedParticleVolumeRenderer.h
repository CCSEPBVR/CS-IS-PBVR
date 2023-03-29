#ifndef PBVR__KVS__VISCLIENT__EXTENDED_PARTICLE_VOLUME_RENDERER_H_INCLUDE
#define PBVR__KVS__VISCLIENT__EXTENDED_PARTICLE_VOLUME_RENDERER_H_INCLUDE

#ifdef CPUMODE
#include <kvs/ParticleVolumeRenderer>
#define RENDERER_BASE   kvs::ParticleVolumeRenderer
#define BASE_INIT_ARGS  &point, subpixel_level
#else
#include <kvs/ParticleBasedRenderer>
#define RENDERER_BASE   kvs::glsl::ParticleBasedRenderer
#define BASE_INIT_ARGS  //&(const_cast<kvs::PointObject&>(point)), subpixel_level, repeat_level
#endif

namespace kvs
{
namespace visclient
{


class ExtendedParticleVolumeRenderer : public RENDERER_BASE
{
public:
    ExtendedParticleVolumeRenderer( const kvs::PointObject& point, const size_t subpixel_level = 1, const size_t repeat_level = 1 ): RENDERER_BASE( BASE_INIT_ARGS )
    {

    }

    void recreateImageBuffer()
    {
#ifdef CPUMODE
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.05.28
        this->deleteParticleBuffer();
        this->createParticleBuffer(
                    BaseClass::windowWidth(),
                    BaseClass::windowHeight(),
                    m_subpixel_level );
#endif
    }

};

}
}

#endif    // PBVR__KVS__VISCLIENT__EXTENDED_PARTICLE_VOLUME_RENDERER_H_INCLUDE
