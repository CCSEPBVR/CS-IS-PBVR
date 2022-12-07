/****************************************************************************/
/**
 *  @file ParticleBuffer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ParticleBuffer.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ParticleBuffer.h"
#include <kvs/Type>
#include <kvs/Math>
#include <kvs/PointObject>


namespace kvs
{

ParticleBuffer::ParticleBuffer( void ):
    m_ref_shader( NULL ),
    m_ref_point_object( NULL )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param width [in] width
 *  @param height [in] height
 *  @param subpixel_level [in] subpixel level
 */
/*==========================================================================*/
ParticleBuffer::ParticleBuffer(
    const size_t width,
    const size_t height,
    const size_t subpixel_level ):
    m_ref_shader( NULL )
{
    this->create( width, height, subpixel_level );
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
ParticleBuffer::~ParticleBuffer( void )
{
    this->clear();
}

/*==========================================================================*/
/**
 *  Return the buffer width.
 */
/*==========================================================================*/
const size_t ParticleBuffer::width( void ) const
{
    return( m_width );
}

/*==========================================================================*/
/**
 *  Return the buffer height.
 */
/*==========================================================================*/
const size_t ParticleBuffer::height( void ) const
{
    return( m_height );
}

/*==========================================================================*/
/**
 *  Return the index buffer.
 */
/*==========================================================================*/
const kvs::ValueArray<kvs::UInt32>& ParticleBuffer::indexBuffer( void ) const
{
    return( m_index_buffer );
}

/*==========================================================================*/
/**
 *  Return the index which is stored in the index buffer.
 *  @param index [in] buffer index
 *  @return stored index in the index buffer
 */
/*==========================================================================*/
const kvs::UInt32 ParticleBuffer::index( const size_t index ) const
{
    return( m_index_buffer[index] );
}

/*==========================================================================*/
/**
 *  Return the depth buffer.
 */
/*==========================================================================*/
const kvs::ValueArray<kvs::Real32>& ParticleBuffer::depthBuffer( void ) const
{
    return( m_depth_buffer );
}

/*==========================================================================*/
/**
 *  Return the depth which is stored in the depth buffer.
 *  @param index [in] buffer index
 *  @return stored depth in the depth buffer
 */
/*==========================================================================*/
const kvs::Real32 ParticleBuffer::depth( const size_t index ) const
{
    return( m_depth_buffer[index] );
}

/*==========================================================================*/
/**
 *  Return the pointer to the attached shader.
 */
/*==========================================================================*/
const kvs::Shader::shader_type* ParticleBuffer::shader( void ) const
{
    return( m_ref_shader );
}

/*==========================================================================*/
/**
 *  Return the pointer to the attached point object.
 */
/*==========================================================================*/
const kvs::PointObject* ParticleBuffer::pointObject( void ) const
{
    return( m_ref_point_object );
}

/*==========================================================================*/
/**
 *  Return the number of projected particles.
 */
/*==========================================================================*/
const size_t ParticleBuffer::numOfProjectedParticles( void ) const
{
    return( m_num_of_projected_particles );
}

/*==========================================================================*/
/**
 *  Return the number of stored particles.
 */
/*==========================================================================*/
const size_t ParticleBuffer::numOfStoredParticles( void ) const
{
    return( m_num_of_stored_particles );
}

/*==========================================================================*/
/**
 *  Set the subpixel level.
 *  @param subpixel_level [in] subpixel level
 */
/*==========================================================================*/
void ParticleBuffer::setSubpixelLevel( const size_t subpixel_level )
{
    m_subpixel_level = subpixel_level;
}

/*==========================================================================*/
/**
 *  Attach the shader.
 *  @param shader [in] pointer to the shader
 */
/*==========================================================================*/
void ParticleBuffer::attachShader( const kvs::Shader::shader_type* shader )
{
    m_ref_shader = shader;
}

/*==========================================================================*/
/**
 *  Attach the point object.
 *  @param point_object [in] pointer to the point object
 */
/*==========================================================================*/
void ParticleBuffer::attachPointObject( const kvs::PointObject* point_object )
{
    m_ref_point_object = NULL;
    m_ref_point_object = point_object;
}

/*==========================================================================*/
/**
 *  Be enable shading.
 */
/*==========================================================================*/
void ParticleBuffer::enableShading( void )
{
    m_enable_shading = true;
}

/*==========================================================================*/
/**
 *  Be disable shading.
 */
/*==========================================================================*/
void ParticleBuffer::disableShading( void )
{
    m_enable_shading = false;
}

/*==========================================================================*/
/**
 *  Create.
 *  @param width [in] width
 *  @param height [in] height
 *  @param subpixel_level [in] subpixel level
 *  @return true/false
 */
/*==========================================================================*/
bool ParticleBuffer::create(
    const size_t width,
    const size_t height,
    const size_t subpixel_level )
{
    const size_t npixels = width * height;

    m_width                   = width;
    m_height                  = height;
    m_subpixel_level          = subpixel_level;
    m_size                    = npixels * 4;
    m_num_of_projected_particles = 0;
    m_num_of_stored_particles    = 0;
    m_enable_shading          = true;
    m_extended_width          = m_width * m_subpixel_level;

    if( m_width == 0 || m_height == 0 )
    {
        kvsMessageError("Cannot create the pixel data for the particle buffer.");
        return( false );
    }

    const size_t subpixeled_npixels = npixels * subpixel_level * subpixel_level;
    m_index_buffer.allocate( subpixeled_npixels );
    m_depth_buffer.allocate( subpixeled_npixels );

    m_index_buffer.fill( 0x00 );
    m_depth_buffer.fill( 0x00 );

    return( true );
}

/*==========================================================================*/
/**
 *  Clean.
 */
/*==========================================================================*/
void ParticleBuffer::clean( void )
{
    m_index_buffer.fill( 0x00 );
    m_depth_buffer.fill( 0x00 );

    m_num_of_projected_particles = 0;
    m_num_of_stored_particles    = 0;
}

/*==========================================================================*/
/**
 *  Clear.
 */
/*==========================================================================*/
void ParticleBuffer::clear( void )
{
    m_index_buffer.deallocate();
    m_depth_buffer.deallocate();
}

/*==========================================================================*/
/**
 *  Create the rendering image.
 *  @param  color [out] pointer to the color array
 *  @param  depth [out] pointer to the depth array
 */
/*==========================================================================*/
void ParticleBuffer::createImage(
    kvs::ValueArray<kvs::UInt8>*  color,
    kvs::ValueArray<kvs::Real32>* depth )
{
    if( m_enable_shading ) this->create_image_with_shading( color, depth );
    else                   this->create_image_without_shading( color, depth );
}

void ParticleBuffer::create_image_with_shading(
    kvs::ValueArray<kvs::UInt8>*  color,
    kvs::ValueArray<kvs::Real32>* depth )
{
    const kvs::Real32* point_coords = m_ref_point_object->coords().pointer();
    const kvs::UInt8* point_color = m_ref_point_object->colors().pointer();
    const kvs::Real32* point_normal = m_ref_point_object->normals().pointer();

    const float inv_ssize = 1.0f / ( m_subpixel_level * m_subpixel_level );
    const float normalize_alpha = 255.0f * inv_ssize;

    size_t pindex   = 0;
    size_t pindex4  = 0;
    size_t by_start = 0;
    const size_t bw = m_extended_width;
    for( size_t py = 0; py < m_height; py++, by_start += m_subpixel_level )
    {
        size_t bx_start = 0;
        for( size_t px = 0; px < m_width; px++, pindex++, pindex4 += 4, bx_start += m_subpixel_level )
        {
            float R = 0.0f;
            float G = 0.0f;
            float B = 0.0f;
            float D = 0.0f;
            size_t npoints = 0;
            for( size_t by = by_start; by < by_start + m_subpixel_level; by++ )
            {
                const size_t bindex_start = bw * by;
                for( size_t bx = bx_start; bx < bx_start + m_subpixel_level; bx++ )
                {
                    const size_t bindex = bindex_start + bx;
                    if( m_depth_buffer[bindex] > 0.0f )
                    {
                        const size_t point_index3 = 3 * m_index_buffer[ bindex ];

                        const kvs::Vector3f vertex( point_coords + point_index3 );
                        const kvs::Vector3f normal( point_normal + point_index3 );
                        kvs::RGBColor color( point_color + point_index3 );
                        color = m_ref_shader->shadedColor( color, vertex, normal );
                        R += color.r();
                        G += color.g();
                        B += color.b();
                        D = kvs::Math::Max( D, m_depth_buffer[ bindex ] );

                        npoints++;
                    }
                }
            }

            R *= inv_ssize;
            G *= inv_ssize;
            B *= inv_ssize;

            (*color)[ pindex4 + 0 ] = static_cast<kvs::UInt8>( kvs::Math::Min( R, 255.0f ) + 0.5f );
            (*color)[ pindex4 + 1 ] = static_cast<kvs::UInt8>( kvs::Math::Min( G, 255.0f ) + 0.5f );
            (*color)[ pindex4 + 2 ] = static_cast<kvs::UInt8>( kvs::Math::Min( B, 255.0f ) + 0.5f );
            (*color)[ pindex4 + 3 ] = static_cast<kvs::UInt8>( npoints * normalize_alpha );
            (*depth)[ pindex ]      = ( npoints == 0 ) ? 1.0f : D;
        }
    }
}

void ParticleBuffer::create_image_without_shading(
    kvs::ValueArray<kvs::UInt8>*  color,
    kvs::ValueArray<kvs::Real32>* depth )
{
    const kvs::UInt8* point_color = m_ref_point_object->colors().pointer();

    const float inv_ssize = 1.0f / ( m_subpixel_level * m_subpixel_level );
    const float normalize_alpha = 255.0f * inv_ssize;

    size_t pindex   = 0;
    size_t pindex4  = 0;
    size_t by_start = 0;
    const size_t bw = m_extended_width;
    for( size_t py = 0; py < m_height; py++, by_start += m_subpixel_level )
    {
        size_t bx_start = 0;
        for( size_t px = 0; px < m_width; px++, pindex++, pindex4 += 4, bx_start += m_subpixel_level )
        {
            float R = 0.0f;
            float G = 0.0f;
            float B = 0.0f;
            float D = 0.0f;
            size_t npoints = 0;
            for( size_t by = by_start; by < by_start + m_subpixel_level; by++ )
            {
                const size_t bindex_start = bw * by;
                for( size_t bx = bx_start; bx < bx_start + m_subpixel_level; bx++ )
                {
                    const size_t bindex = bindex_start + bx;
                    if( m_depth_buffer[bindex] > 0.0f )
                    {
                        const size_t point_index3 = 3 * m_index_buffer[ bindex ];

                        R += point_color[ point_index3 + 0 ];
                        G += point_color[ point_index3 + 1 ];
                        B += point_color[ point_index3 + 2 ];
                        D = kvs::Math::Max( D, m_depth_buffer[ bindex ] );
                        npoints++;
                    }
                }
            }

            R *= inv_ssize;
            G *= inv_ssize;
            B *= inv_ssize;

            (*color)[ pindex4 + 0 ] = static_cast<kvs::UInt8>(R);
            (*color)[ pindex4 + 1 ] = static_cast<kvs::UInt8>(G);
            (*color)[ pindex4 + 2 ] = static_cast<kvs::UInt8>(B);
            (*color)[ pindex4 + 3 ] = static_cast<kvs::UInt8>( npoints * normalize_alpha );
            (*depth)[ pindex ]      = ( npoints == 0 ) ? 1.0f : D;
        }
    }
}

} // end of namesapce kvs
