/****************************************************************************/
/**
 *  @file ParticleBufferAccumulator.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ParticleBufferAccumulator.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ParticleBufferAccumulator.h"


namespace vismodule
{

/*==========================================================================*/
/**
 *  Construct a new class.
 *  @param width [in] width
 *  @param height [in] height
 *  @param subpixel_level [in] subpixel level
 */
/*==========================================================================*/
ParticleBufferAccumulator::ParticleBufferAccumulator(
    const std::size_t width,
    const std::size_t height,
    const std::size_t subpixel_level ):
    vismodule::ParticleBuffer( width, height, subpixel_level )
{
    this->create( width, height, subpixel_level );
}

/*==========================================================================*/
/**
 *  Destruct this class.
 */
/*==========================================================================*/
ParticleBufferAccumulator::~ParticleBufferAccumulator( void )
{
    this->clear();
}

/*==========================================================================*/
/**
 *  Create a new class.
 *  @param width [in] width
 *  @param height [in] height
 *  @param subpixel_level [in] subpixel level
 *  @return true/false
 */
/*==========================================================================*/
bool ParticleBufferAccumulator::create(
    const std::size_t width,
    const std::size_t height,
    const std::size_t subpixel_level )
{
    const std::size_t npixels = width * height;
    const std::size_t subpixeled_npixels = npixels * subpixel_level * subpixel_level;

    m_id_buffer.allocate( subpixeled_npixels );
    m_id_buffer.fill( 0x00 );

    return( true );
}

/*==========================================================================*/
/**
 *  Clean this class. (not delete)
 */
/*==========================================================================*/
void ParticleBufferAccumulator::clean( void )
{
    SuperClass::clean();
    m_id_buffer.fill( 0x00 );
}

/*==========================================================================*/
/**
 *  Clear this class. (delete this class)
 */
/*==========================================================================*/
void ParticleBufferAccumulator::clear( void )
{
    SuperClass::clear();
    m_id_buffer.deallocate();
}

/*==========================================================================*/
/**
 *  Accumulate the particle buffer.
 *  @param id [in] ID number of the object
 *  @param buffer [in] pointer to the particle buffer
 */
/*==========================================================================*/
void ParticleBufferAccumulator::accumulate(
    const std::size_t               id,
    const vismodule::ParticleBuffer* buffer )
{
    const std::size_t nsubpixels = m_width * m_height * m_subpixel_level * m_subpixel_level;
    for( std::size_t index = 0; index < nsubpixels; index++ )
    {
        const vismodule::Real32 buffer_depth = buffer->depth( index );
        if( buffer_depth > 0.0f )
        {
            const vismodule::UInt32 buffer_index = buffer->index( index );
            this->add( index, id, buffer_depth, buffer_index );
        }
    }
}

/*==========================================================================*/
/**
 *  Add a particle to the buffer for accumulating.
 *  @param index [in] subpixel index
 *  @param id [in] ID number of the object
 *  @param depth [in] buffer depth
 *  @param vindex [in] buffer index
 */
/*==========================================================================*/
void ParticleBufferAccumulator::add(
    const std::size_t      index,
    const std::size_t      id,
    const vismodule::Real32 depth,
    const vismodule::UInt32 vindex )
{
    const vismodule::Real32 buffer_depth = SuperClass::m_depth_buffer[index];
    if( buffer_depth > 0.0f )
    {
        // Detect collision.
        if( buffer_depth > depth )
        {
            SuperClass::m_depth_buffer[index] = depth;
            SuperClass::m_index_buffer[index] = vindex;
            m_id_buffer[index] = static_cast<vismodule::UInt8>( id );
        }
    }
    else
    {
        // Not collision.
        SuperClass::m_depth_buffer[index] = depth;
        SuperClass::m_index_buffer[index] = vindex;
        m_id_buffer[index] = static_cast<vismodule::UInt8>( id );
    }
}

/*==========================================================================*/
/**
 *  Create color and depth buffer.
 *  @param object_list [in] object list
 *  @param renderer_list [in] renderer list
 *  @param color [in] color buffer
 *  @param depth [in] depth buffer
 */
/*==========================================================================*/
void ParticleBufferAccumulator::createImage(
    ParticleBufferAccumulator::ObjectList&   object_list,
    ParticleBufferAccumulator::RendererList& renderer_list,
    vismodule::ValueArray<vismodule::UInt8>*  color,
    vismodule::ValueArray<vismodule::Real32>* depth )
{
    const float inv_ssize = 1.0f / ( m_subpixel_level * m_subpixel_level );
    const float normalize_alpha = 255.0f * inv_ssize;

    std::size_t pindex   = 0;
    std::size_t pindex4  = 0;
    std::size_t by_start = 0;
    const std::size_t bw = m_width * m_subpixel_level;
    for ( std::size_t py = 0; py < m_height; py++, by_start += m_subpixel_level )
    {
        std::size_t bx_start = 0;
        for( std::size_t px = 0; px < m_width; px++, pindex++, pindex4 += 4, bx_start += m_subpixel_level )
        {
            float R = 0.0f;
            float G = 0.0f;
            float B = 0.0f;
            float D = 0.0f;
            std::size_t npoints = 0;
            for( std::size_t by = by_start; by < by_start + m_subpixel_level; by++ )
            {
                const std::size_t bindex_start = bw * by;
                for( std::size_t bx = bx_start; bx < bx_start + m_subpixel_level; bx++ )
                {
                    const std::size_t bindex = bindex_start + bx;
                    if( m_depth_buffer[bindex] > 0.0f )
                    {
                        const std::size_t id = m_id_buffer[ bindex ];
                        const vismodule::PointObject*            object   = object_list[id];
                        const vismodule::ParticleVolumeRenderer* renderer = renderer_list[id];

                        const std::size_t point_index3 = 3 * m_index_buffer[bindex];

                        vismodule::RGBColor color( object->colors().pointer() + point_index3 );
                        if( renderer->isEnabledShading() )
                        {
                            const vismodule::Shader::shader_type* shader = renderer->particleBuffer()->shader();
                            const vismodule::Vector3f vertex( object->coords().pointer() + point_index3 );
                            const vismodule::Vector3f normal( object->normals().pointer() + point_index3 );
                            color = shader->shadedColor( color, vertex, normal );
                        }

                        R += color.r();
                        G += color.g();
                        B += color.b();
                        D = vismodule::Math::Max( D, m_depth_buffer[ bindex ] );

                        npoints++;
                    }
                }
            }

            R *= inv_ssize;
            G *= inv_ssize;
            B *= inv_ssize;

            (*color)[ pindex4 + 0 ] = static_cast<vismodule::UInt8>(R);
            (*color)[ pindex4 + 1 ] = static_cast<vismodule::UInt8>(G);
            (*color)[ pindex4 + 2 ] = static_cast<vismodule::UInt8>(B);
            (*color)[ pindex4 + 3 ] = static_cast<vismodule::UInt8>( npoints * normalize_alpha );
            (*depth)[ pindex ]      = ( npoints == 0 ) ? 1.0f : D;
        }
    }
}

} // end of namespace vismodule
