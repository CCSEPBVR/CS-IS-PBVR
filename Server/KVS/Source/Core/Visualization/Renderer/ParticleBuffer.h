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
#ifndef KVS__PARTICLE_BUFFER_H_INCLUDE
#define KVS__PARTICLE_BUFFER_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Shader>


namespace kvs
{

class PointObject;

/*==========================================================================*/
/**
 *  Particle buffer class.
 */
/*==========================================================================*/
class ParticleBuffer
{
    kvsClassName( kvs::ParticleBuffer );

protected:

    size_t  m_width;                      ///< width
    size_t  m_height;                     ///< height
    size_t  m_size;                       ///< pixel data size [byte]
    size_t  m_num_of_projected_particles; ///< total number of projected points
    size_t  m_num_of_stored_particles;    ///< total number of stored points
    size_t  m_subpixel_level;             ///< subpixel level
    bool    m_enable_shading;             ///< shading flag
    size_t  m_extended_width;             ///< m_width * m_subpixel_level

    kvs::ValueArray<kvs::UInt32> m_index_buffer; ///< index buffer
    kvs::ValueArray<kvs::Real32> m_depth_buffer; ///< depth buffer

    // Reference shader (NOTE: not allocated in thie class).
    const kvs::Shader::shader_type* m_ref_shader;
    const kvs::PointObject*         m_ref_point_object;

public:

    ParticleBuffer( void );

    ParticleBuffer( const size_t width, const size_t height, const size_t subpixel_level );

    virtual ~ParticleBuffer( void );

public:

    const size_t width( void ) const;

    const size_t height( void ) const;

    const kvs::ValueArray<kvs::UInt32>& indexBuffer( void ) const;

    const kvs::UInt32 index( const size_t index ) const;

    const kvs::ValueArray<kvs::Real32>& depthBuffer( void ) const;

    const kvs::Real32 depth( const size_t index ) const;

    const size_t subpixelLevel( void ) const;

    const kvs::Shader::shader_type* shader( void ) const;

    const kvs::PointObject* pointObject( void ) const;

    const size_t numOfProjectedParticles( void ) const;

    const size_t numOfStoredParticles( void ) const;

public:

    void setSubpixelLevel( const size_t subpixel_level );

    void attachShader( const kvs::Shader::shader_type* shader );

    void attachPointObject( const kvs::PointObject* point_object );

public:

    void enableShading( void );

    void disableShading( void );

public:

    void add( const float x, const float y, const kvs::Real32 depth, const kvs::UInt32 index );

    bool create( const size_t width, const size_t height, const size_t subpixel_level );

    void clean( void );

    void clear( void );

public:

    void createImage(
        kvs::ValueArray<kvs::UInt8>*  color,
        kvs::ValueArray<kvs::Real32>* depth );

protected:

    void create_image_with_shading(
        kvs::ValueArray<kvs::UInt8>*  color,
        kvs::ValueArray<kvs::Real32>* depth );

    void create_image_without_shading(
        kvs::ValueArray<kvs::UInt8>*  color,
        kvs::ValueArray<kvs::Real32>* depth );
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
    const kvs::Real32 depth,
    const kvs::UInt32 voxel_index )
{
    // Buffer coordinate value.
    const size_t bx = static_cast<size_t>( x * m_subpixel_level );
    const size_t by = static_cast<size_t>( y * m_subpixel_level );

    const size_t index = m_extended_width * by + bx;
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

} // end of namespace kvs

#endif // KVS__PARTICLE_BUFFER_H_INCLUDE
