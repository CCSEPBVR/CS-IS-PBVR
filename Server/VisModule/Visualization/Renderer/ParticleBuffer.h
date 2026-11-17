/****************************************************************************/
/**
 *  @file ParticleBuffer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ParticleBuffer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__PARTICLE_BUFFER_H_INCLUDE
#define VIS_MODULE__PARTICLE_BUFFER_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Shader>


namespace vismodule
{

class PointObject;

/*==========================================================================*/
/**
 *  Particle buffer class.
 */
/*==========================================================================*/
class ParticleBuffer
{
    visModuleClassName( vismodule::ParticleBuffer );

protected:

    std::size_t  m_width;                      ///< width
    std::size_t  m_height;                     ///< height
    std::size_t  m_size;                       ///< pixel data size [byte]
    std::size_t  m_num_of_projected_particles; ///< total number of projected points
    std::size_t  m_num_of_stored_particles;    ///< total number of stored points
    std::size_t  m_subpixel_level;             ///< subpixel level
    bool    m_enable_shading;             ///< shading flag
    std::size_t  m_extended_width;             ///< m_width * m_subpixel_level

    vismodule::ValueArray<vismodule::UInt32> m_index_buffer; ///< index buffer
    vismodule::ValueArray<vismodule::Real32> m_depth_buffer; ///< depth buffer

    // Reference shader (NOTE: not allocated in thie class).
    const vismodule::Shader::shader_type* m_ref_shader;
    const vismodule::PointObject*         m_ref_point_object;

public:

    ParticleBuffer( void );

    ParticleBuffer( const std::size_t width, const std::size_t height, const std::size_t subpixel_level );

    virtual ~ParticleBuffer( void );

public:

    const std::size_t width( void ) const;

    const std::size_t height( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& indexBuffer( void ) const;

    const vismodule::UInt32 index( const std::size_t index ) const;

    const vismodule::ValueArray<vismodule::Real32>& depthBuffer( void ) const;

    const vismodule::Real32 depth( const std::size_t index ) const;

    const std::size_t subpixelLevel( void ) const;

    const vismodule::Shader::shader_type* shader( void ) const;

    const vismodule::PointObject* pointObject( void ) const;

    const std::size_t numOfProjectedParticles( void ) const;

    const std::size_t numOfStoredParticles( void ) const;

public:

    void setSubpixelLevel( const std::size_t subpixel_level );

    void attachShader( const vismodule::Shader::shader_type* shader );

    void attachPointObject( const vismodule::PointObject* point_object );

public:

    void enableShading( void );

    void disableShading( void );

public:

    void add( const float x, const float y, const vismodule::Real32 depth, const vismodule::UInt32 index );

    bool create( const std::size_t width, const std::size_t height, const std::size_t subpixel_level );

    void clean( void );

    void clear( void );

public:

    void createImage(
        vismodule::ValueArray<vismodule::UInt8>*  color,
        vismodule::ValueArray<vismodule::Real32>* depth );

protected:

    void create_image_with_shading(
        vismodule::ValueArray<vismodule::UInt8>*  color,
        vismodule::ValueArray<vismodule::Real32>* depth );

    void create_image_without_shading(
        vismodule::ValueArray<vismodule::UInt8>*  color,
        vismodule::ValueArray<vismodule::Real32>* depth );
};

/*==========================================================================*/
/**
 *  Add a point to the buffer.
 *  @param x [in] x coordinate value in the buffer
 *  @param y [in] y coordinate value in the buffer
 *  @param depth [in] depth value
 *  @param voxel_index [in] voxel index
 */
/*==========================================================================*/
inline void ParticleBuffer::add(
    const float x,
    const float y,
    const vismodule::Real32 depth,
    const vismodule::UInt32 voxel_index )
{
    // Buffer coordinate value.
    const std::size_t bx = static_cast<size_t>( x * m_subpixel_level );
    const std::size_t by = static_cast<size_t>( y * m_subpixel_level );

    const std::size_t index = m_extended_width * by + bx;
    m_num_of_projected_particles++;

    if( m_depth_buffer[index] > 0.0f )
    {
        // Detect collision.
        if( m_depth_buffer[index] > depth )
        {
            m_depth_buffer[index] = depth;
            m_index_buffer[index] = voxel_index;
        }
    }
    else
    {
        m_depth_buffer[index] = depth;
        m_index_buffer[index] = voxel_index;
    }
}

} // end of namespace vismodule

#endif // VIS_MODULE__PARTICLE_BUFFER_H_INCLUDE
