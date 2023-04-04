/****************************************************************************/
/**
 *  @file RayCastingRenderer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RayCastingRenderer.cpp 605 2010-08-21 05:08:32Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "RayCastingRenderer.h"
#include <kvs/Math>
#include <kvs/Type>
#include <kvs/Message>
#include <kvs/StructuredVolumeObject>
#include <kvs/TrilinearInterpolator>
#include <kvs/VolumeRayIntersector>


namespace kvs
{

RayCastingRenderer::RayCastingRenderer( void )
{
    BaseClass::setShader( kvs::Shader::Lambert() );
    this->initialize();
}

RayCastingRenderer::RayCastingRenderer( const kvs::TransferFunction& tfunc )
{
    BaseClass::setTransferFunction( tfunc );
    BaseClass::setShader( kvs::Shader::Lambert() );
    this->initialize();
}

template <typename ShadingType>
RayCastingRenderer::RayCastingRenderer( const ShadingType shader )
{
    BaseClass::setShader( shader );
    this->initialize();
}

RayCastingRenderer::~RayCastingRenderer( void )
{
}

void RayCastingRenderer::initialize( void )
{
    m_step   = 0.5f;
    m_opaque = 0.97f;
    m_width  = 0;
    m_height = 0;
    m_ray_width = 1;
    m_enable_lod = false;
    memset( m_modelview_matrix, 0, sizeof( m_modelview_matrix ) );
}

void RayCastingRenderer::exec(
    kvs::ObjectBase* object,
    kvs::Camera*     camera,
    kvs::Light*      light )
{
    const kvs::StructuredVolumeObject* volume = reinterpret_cast<kvs::StructuredVolumeObject*>(object);

    BaseClass::m_timer.start();

    this->create_image( volume, camera, light );
    this->draw_image();

    BaseClass::m_timer.stop();
}

/*==========================================================================*/
/**
 *  Create rendering image.
 *
 *  @param volume  [in] volume object
 *  @param camera  [in] camera
 *  @param light   [in] light
 */
/*==========================================================================*/
void RayCastingRenderer::create_image(
    const kvs::StructuredVolumeObject* volume,
    const kvs::Camera*                 camera,
    const kvs::Light*                  light )
{
    if( BaseClass::m_width  != camera->windowWidth() ||
        BaseClass::m_height != camera->windowHeight() )
    {
        BaseClass::m_width  = camera->windowWidth();
        BaseClass::m_height = camera->windowHeight();

        const size_t npixels = BaseClass::m_width * BaseClass::m_height;
        BaseClass::m_color_data.allocate( npixels * 4 );
        BaseClass::m_depth_data.allocate( npixels );

        glGetFloatv( GL_MODELVIEW_MATRIX, m_modelview_matrix );
    }

    BaseClass::m_color_data.fill( 0x00 );
    BaseClass::m_depth_data.fill( 0x00 );

    if ( !volume->hasMinMaxValues() ) volume->updateMinMaxValues();
    const float min_value = static_cast<float>( volume->minValue() );
    const float max_value = static_cast<float>( volume->maxValue() );
    const std::type_info& type = volume->values().typeInfo()->type();
    if(      type == typeid(kvs::UInt8)  )
    {
        if ( !m_tfunc.hasRange() ) BaseClass::m_tfunc.setRange( 0, 255 );
        this->rasterize<kvs::UInt8>( volume, camera, light );
    }
    else if( type == typeid(kvs::UInt16) )
    {
        if ( !m_tfunc.hasRange() ) BaseClass::m_tfunc.setRange( min_value, max_value );
        this->rasterize<kvs::UInt16>( volume, camera, light );
    }
    else if( type == typeid(kvs::Int16) )
    {
        if ( !m_tfunc.hasRange() ) BaseClass::m_tfunc.setRange( min_value, max_value );
        this->rasterize<kvs::Int16>( volume, camera, light );
    }
    else if( type == typeid(kvs::Real32) )
    {
        if ( !m_tfunc.hasRange() ) BaseClass::m_tfunc.setRange( min_value, max_value );
        this->rasterize<kvs::Real32>( volume, camera, light );
    }
    else if( type == typeid(kvs::Real64) )
    {
        if ( !m_tfunc.hasRange() ) BaseClass::m_tfunc.setRange( min_value, max_value );
        this->rasterize<kvs::Real64>( volume, camera, light );
    }
    else
    {
        kvsMessageError( "Not supported data type '%s'.",
                         volume->values().typeInfo()->typeName() );
    }
}

/*==========================================================================*/
/**
 *  Rasterization.
 *
 *  @param volume  [in] volume object
 *  @param camera  [in] camera
 *  @param light   [in] light
 */
/*==========================================================================*/
template <typename T>
void RayCastingRenderer::rasterize(
    const kvs::StructuredVolumeObject* volume,
    const kvs::Camera*                 camera,
    const kvs::Light*                  light )
{
    // Set shader initial parameters.
    BaseClass::m_shader->set( camera, light );

    // Aliases.
    kvs::UInt8*  const pixel      = BaseClass::m_color_data.pointer();
    kvs::Real32* const depth_data = BaseClass::m_depth_data.pointer();

    // LOD control.
    size_t ray_width = 1;
    if ( m_enable_lod )
    {
        float modelview_matrix[16];
        glGetFloatv( GL_MODELVIEW_MATRIX, modelview_matrix );
        for ( size_t i = 0; i < 16; i++ )
        {
            if ( m_modelview_matrix[i] != modelview_matrix[i] )
            {
                ray_width = m_ray_width;
                break;
            }
        }
        memcpy( m_modelview_matrix, modelview_matrix, sizeof( modelview_matrix ) );
    }

    // Set the trilinear interpolator.
    kvs::TrilinearInterpolator interpolator( volume );

    // Calculate the ray in the object coordinate system.
    kvs::VolumeRayIntersector ray( volume );

    // Execute ray casting.
    const size_t height = BaseClass::m_height;
    const size_t width  = BaseClass::m_width;

    const float step = m_step;
    const float opaque = m_opaque;

    const kvs::Shader::shader_type* const shader = m_shader;
    const kvs::ColorMap&   cmap = BaseClass::transferFunction().colorMap();
    const kvs::OpacityMap& omap = BaseClass::transferFunction().opacityMap();

    size_t depth_index = 0;
    size_t pixel_index = 0;
    for ( size_t y = 0; y < height; y += ray_width )
    {
        const size_t offset = y * width;
        for ( size_t x = 0; x < width; x += ray_width, depth_index = offset + x, pixel_index = depth_index * 4 )
        {
            ray.setOrigin( x, y );

            // Intersection the ray with the bounding box.
            if ( ray.isIntersected() )
            {
                float r = 0.0f;
                float g = 0.0f;
                float b = 0.0f;
                float a = 0.0;

                depth_data[ depth_index ] = ray.depth();

                do
                {
                    // Interpolation.
                    interpolator.attachPoint( ray.point() );

                    // Classification.
                    const float s = interpolator.template scalar<T>();
                    const float opacity = omap.at(s);
                    if ( !kvs::Math::IsZero( opacity ) )
                    {
                        // Shading.
                        const kvs::Vector3f vertex = ray.point();
                        const kvs::Vector3f normal = interpolator.template gradient<T>();
                        const kvs::RGBColor color = shader->shadedColor( cmap.at(s), vertex, normal );

                        // Front-to-back accumulation.
                        const float current_alpha = ( 1.0f - a ) * opacity;
                        r += current_alpha * color.r();
                        g += current_alpha * color.g();
                        b += current_alpha * color.b();
                        a += current_alpha;
                        if ( a > opaque )
                        {
                            a = 1.0f;
                            break;
                        }
                    }

                    ray.step( step );
                } while ( ray.isInside() );

                // Set pixel value.
                pixel[ pixel_index     ] = static_cast<kvs::UInt8>( kvs::Math::Min( r, 255.0f ) + 0.5f );
                pixel[ pixel_index + 1 ] = static_cast<kvs::UInt8>( kvs::Math::Min( g, 255.0f ) + 0.5f );
                pixel[ pixel_index + 2 ] = static_cast<kvs::UInt8>( kvs::Math::Min( b, 255.0f ) + 0.5f );
                pixel[ pixel_index + 3 ] = static_cast<kvs::UInt8>( kvs::Math::Round( a * 255.0f ) );
            }
            else
            {
                depth_data[ depth_index ] = 1.0;
            }
        }
    }

    // Mosaicing by using ray_width x ray_width mask.
    if ( ray_width > 1 )
    {
        pixel_index = 0;
        depth_index = 0;
        for ( size_t y = 0; y < height; y += ray_width )
        {
            // Shift the y position of the mask by -ray_width/2.
            const size_t Y = kvs::Math::Max( int( y - ray_width / 2 ), 0 );

            const size_t offset = y * width;
            for ( size_t x = 0; x < width; x += ray_width, depth_index = offset + x, pixel_index = depth_index * 4 )
            {
                // Shift the x position of the mask by -ray_width/2.
                const size_t X = kvs::Math::Max( int( x - ray_width / 2 ), 0 );

                const kvs::UInt8  r = pixel[ pixel_index ];
                const kvs::UInt8  g = pixel[ pixel_index + 1 ];
                const kvs::UInt8  b = pixel[ pixel_index + 2 ];
                const kvs::UInt8  a = pixel[ pixel_index + 3 ];
                const kvs::Real32 d = depth_data[ depth_index ];
                for ( size_t j = 0; j < ray_width && Y + j < height; j++ )
                {
                    const size_t J = Y + j;
                    for ( size_t i = 0; i < ray_width && X + i < width; i++ )
                    {
                        const size_t I = X + i;
                        const size_t index = J * width + I;
                        const size_t index4 = index * 4;
                        pixel[ index4 ] = r;
                        pixel[ index4 + 1 ] = g;
                        pixel[ index4 + 2 ] = b;
                        pixel[ index4 + 3 ] = a;
                        depth_data[ index ] = d;
                    }
                }
            }
        }
    }

    glFinish();
}

template
void RayCastingRenderer::rasterize<kvs::UInt8>(
    const kvs::StructuredVolumeObject* volume,
    const kvs::Camera*                 camera,
    const kvs::Light*                  light );

template
void RayCastingRenderer::rasterize<kvs::UInt16>(
    const kvs::StructuredVolumeObject* volume,
    const kvs::Camera*                 camera,
    const kvs::Light*                  light );

template
void RayCastingRenderer::rasterize<kvs::Int16>(
    const kvs::StructuredVolumeObject* volume,
    const kvs::Camera*                 camera,
    const kvs::Light*                  light );

template
void RayCastingRenderer::rasterize<kvs::Real32>(
    const kvs::StructuredVolumeObject* volume,
    const kvs::Camera*                 camera,
    const kvs::Light*                  light );

template
void RayCastingRenderer::rasterize<kvs::Real64>(
    const kvs::StructuredVolumeObject* volume,
    const kvs::Camera*                 camera,
    const kvs::Light*                  light );

} // end of namespace kvs
