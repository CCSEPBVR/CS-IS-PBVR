/*****************************************************************************/
/**
 *  @file   GlyphBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GlyphBase.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "GlyphBase.h"
#include <kvs/Quaternion>
#include <kvs/Matrix>
#include <kvs/Xform>
#include <kvs/IgnoreUnusedVariable>


namespace
{
const kvs::Vector3f DefaultDirection = kvs::Vector3f( 0.0, 1.0, 0.0 );
const kvs::Real32 DefaultSize = 1.0f;
const kvs::RGBColor DefaultColor = kvs::RGBColor( 255, 255, 255 );
const kvs::UInt8 DefaultOpacity = 255;
};


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new GlyphBase class.
 */
/*===========================================================================*/
GlyphBase::GlyphBase( void ):
    m_size_mode( GlyphBase::SizeByDefault ),
    m_direction_mode( GlyphBase::DirectionByDefault ),
    m_color_mode( GlyphBase::ColorByMagnitude ),
    m_opacity_mode( GlyphBase::OpacityByDefault )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the GlyphBase class.
 */
/*===========================================================================*/
GlyphBase::~GlyphBase( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Sets a size mode.
 *  @param  mode [in] size mode
 */
/*===========================================================================*/
void GlyphBase::setSizeMode( const GlyphBase::SizeMode mode )
{
    m_size_mode = mode;
}

/*===========================================================================*/
/**
 *  @brief  Sets a direction mode.
 *  @param  mode [in] direction mode
 */
/*===========================================================================*/
void GlyphBase::setDirectionMode( const GlyphBase::DirectionMode mode )
{
    m_direction_mode = mode;
}

/*===========================================================================*/
/**
 *  @brief  Sets a color mode.
 *  @param  mode [in] color mode
 */
/*===========================================================================*/
void GlyphBase::setColorMode( const GlyphBase::ColorMode mode )
{
    m_color_mode = mode;
}

/*===========================================================================*/
/**
 *  @brief  Sets a opacity mode.
 *  @param  mode [in] opacity mode
 */
/*===========================================================================*/
void GlyphBase::setOpacityMode( const GlyphBase::OpacityMode mode )
{
    m_opacity_mode = mode;
}

/*===========================================================================*/
/**
 *  @brief  Transform the glyph object.
 *  @param  position [in] object position
 *  @param  size [in] object size
 */
/*===========================================================================*/
void GlyphBase::transform( const kvs::Vector3f& position, const kvs::Real32 size )
{
    glTranslatef( position.x(), position.y(), position.z() );
    glScalef( m_scale.x(), m_scale.y(), m_scale.z() );
    glScalef( size, size, size );
}

/*===========================================================================*/
/**
 *  @brief  Transform the glyph object.
 *  @param  position [in] object position
 *  @param  direction [in] object direction
 *  @param  size [in] object size
 */
/*===========================================================================*/
void GlyphBase::transform(
    const kvs::Vector3f& position,
    const kvs::Vector3f& direction,
    const kvs::Real32 size )
{
    const kvs::Vector3f v = direction.normalize();
    const kvs::Vector3f c = ::DefaultDirection.cross( v );
    const float d = ::DefaultDirection.dot( v );
    const float s = static_cast<float>( std::sqrt( ( 1.0 + d ) * 2.0 ) );
    const kvs::Quaternion<float> q( c.x()/s, c.y()/s, c.z()/s, s/2.0f );
    const kvs::Matrix33f rot = q.toMatrix();
    const kvs::Xform xform( position, m_scale * size, rot );

    float array[16];
    xform.get( &array );
    glMultMatrixf( array );
}

/*===========================================================================*/
/**
 *  @brief  Calculates the coordinate value array.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
void GlyphBase::calculate_coords( const kvs::VolumeObjectBase* volume )
{
    const kvs::VolumeObjectBase::VolumeType type = volume->volumeType();
    if ( type == kvs::VolumeObjectBase::Structured )
    {
        this->calculate_coords( kvs::StructuredVolumeObject::DownCast( volume ) );
    }
    else // type == kvs::VolumeObjectBase::Unstructured
    {
        this->setCoords( volume->coords() );
    }
}

/*===========================================================================*/
/**
 *  @brief  Calculates the coordinate value array of the structured volume object.
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
void GlyphBase::calculate_coords( const kvs::StructuredVolumeObject* volume )
{
    const kvs::VolumeObjectBase::GridType type = volume->gridType();
    if ( type == kvs::VolumeObjectBase::Uniform )
    {
       this->calculate_uniform_coords( volume );
    }
    else if ( type == kvs::VolumeObjectBase::Rectilinear )
    {
        this->calculate_rectilinear_coords( volume );
    }
    else
    {
        this->setCoords( volume->coords() );
    }
}

/*===========================================================================*/
/**
 *  @brief  Calculates the coordinate value array of the uniform type volume object.
 *  @param  volume [in] pointer to the structrued volume object
 */
/*===========================================================================*/
void GlyphBase::calculate_uniform_coords( const kvs::StructuredVolumeObject* volume )
{
    kvs::ValueArray<kvs::Real32> coords( 3 * volume->nnodes() );
    kvs::Real32* coord = coords.pointer();

    const kvs::Vector3ui resolution( volume->resolution() );
    const kvs::Vector3f  volume_size( volume->maxExternalCoord() - volume->minExternalCoord() );
    const kvs::Vector3ui ngrids( resolution - kvs::Vector3ui( 1, 1, 1 ) );
    const kvs::Vector3f  grid_size(
        volume_size.x() / static_cast<float>( ngrids.x() ),
        volume_size.y() / static_cast<float>( ngrids.y() ),
        volume_size.z() / static_cast<float>( ngrids.z() ) );

    for ( size_t k = 0; k < resolution.z(); ++k )
    {
        const float z =
            grid_size.z() * static_cast<float>( k );
        for ( size_t j = 0; j < resolution.y(); ++j )
        {
            const float y =
                grid_size.y() * static_cast<float>( j );
            for ( size_t i = 0; i < resolution.x(); ++i )
            {
                const float x =
                    grid_size.x() * static_cast<float>( i );

                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z;
            }
        }
    }

    this->setCoords( coords );
}

/*===========================================================================*/
/**
 *  @brief  Calculates the coordinate value array of the rectilinear type volume object.
 *  @param  volume [in] pointer to the structrued volume object
 */
/*===========================================================================*/
void GlyphBase::calculate_rectilinear_coords( const kvs::StructuredVolumeObject* volume )
{
    kvs::IgnoreUnusedVariable( volume );
    kvsMessageError("Rectilinear volume has not yet support.");
}

/*===========================================================================*/
/**
 *  @brief  Calculates the size value array.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
template <typename T>
void GlyphBase::calculate_sizes( const kvs::VolumeObjectBase* volume )
{
    const T* value = reinterpret_cast<const T*>( volume->values().pointer() );
    const size_t veclen = volume->veclen();
    const size_t nnodes = volume->nnodes();

    if ( !volume->hasMinMaxValues() ) { volume->updateMinMaxValues(); }
    const kvs::Real32 min_value = static_cast<kvs::Real32>(volume->minValue());
    const kvs::Real32 max_value = static_cast<kvs::Real32>(volume->maxValue());
    const kvs::Real32 normalize = 1.0f / ( max_value - min_value );

    kvs::ValueArray<kvs::Real32> sizes( nnodes );
    kvs::Real32* size = sizes.pointer();

    switch( m_size_mode )
    {
    case GlyphBase::SizeByDefault:
        for( size_t i = 0; i < nnodes; i++ ) size[i] = ::DefaultSize;
        break;
    case GlyphBase::SizeByMagnitude:
        if ( veclen == 1 )
        {
            for( size_t i = 0; i < nnodes; i++ )
            {
                size[i] = ::DefaultSize * normalize * ( static_cast<float>(value[i]) - min_value );
            }
        }
        else if ( veclen == 3 )
        {
            for( size_t i = 0, index = 0; i < nnodes; i++, index += 3 )
            {
                const kvs::Vector3f v(
                    static_cast<float>(value[index]),
                    static_cast<float>(value[index+1]),
                    static_cast<float>(value[index+2]));
                size[i] = ::DefaultSize * normalize * ( static_cast<float>(v.length()) - min_value );
            }
        }
        break;
    default:
        break;
    }

    this->setSizes( sizes );
}

template void GlyphBase::calculate_sizes<kvs::Int8>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_sizes<kvs::Int16>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_sizes<kvs::Int32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_sizes<kvs::Int64>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_sizes<kvs::UInt8>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_sizes<kvs::UInt16>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_sizes<kvs::UInt32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_sizes<kvs::UInt64>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_sizes<kvs::Real32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_sizes<kvs::Real64>( const kvs::VolumeObjectBase* volume );

/*===========================================================================*/
/**
 *  @brief  Calculates the direction vector array.
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
template <typename T>
void GlyphBase::calculate_directions( const kvs::VolumeObjectBase* volume )
{
    const T* value = reinterpret_cast<const T*>( volume->values().pointer() );
    const size_t veclen = volume->veclen();
    const size_t nnodes = volume->nnodes();
    if ( veclen == 3 )
    {
        kvs::ValueArray<kvs::Real32> directions( nnodes * veclen );
        kvs::Real32* direction = directions.pointer();

        for ( size_t i = 0; i < directions.size(); i++ )
        {
            direction[i] = static_cast<kvs::Real32>( value[i] );
        }

        this->setDirections( directions );
    }
}

template void GlyphBase::calculate_directions<kvs::Int8>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_directions<kvs::Int16>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_directions<kvs::Int32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_directions<kvs::Int64>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_directions<kvs::UInt8>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_directions<kvs::UInt16>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_directions<kvs::UInt32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_directions<kvs::UInt64>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_directions<kvs::Real32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_directions<kvs::Real64>( const kvs::VolumeObjectBase* volume );

/*===========================================================================*/
/**
 *  @brief  Calculates the color value array.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
template <typename T>
void GlyphBase::calculate_colors( const kvs::VolumeObjectBase* volume )
{
    const T* value = reinterpret_cast<const T*>( volume->values().pointer() );
    const size_t veclen = volume->veclen();
    const size_t nnodes = volume->nnodes();

    if ( !volume->hasMinMaxValues() ) { volume->updateMinMaxValues(); }
    const kvs::Real32 min_value = static_cast<kvs::Real32>(volume->minValue());
    const kvs::Real32 max_value = static_cast<kvs::Real32>(volume->maxValue());
    const kvs::Real32 normalize = 1.0f / ( max_value - min_value );

    kvs::ValueArray<kvs::UInt8> colors( 3 * nnodes );
    kvs::UInt8* color = colors.pointer();

    switch( m_color_mode )
    {
    case GlyphBase::ColorByDefault:
        for ( size_t i = 0; i < nnodes; i++ )
        {
            *( color++ ) = ::DefaultColor.r();
            *( color++ ) = ::DefaultColor.r();
            *( color++ ) = ::DefaultColor.r();
        }
        break;
    case GlyphBase::ColorByMagnitude:
    {
        const kvs::ColorMap color_map( BaseClass::transferFunction().colorMap() );
        if ( veclen == 1 )
        {
            for ( size_t i = 0; i < nnodes; i++ )
            {
                const float d = normalize * ( static_cast<float>(value[i]) - min_value );
                const size_t level = static_cast<size_t>( 255.0f * d );
                const kvs::RGBColor c = color_map[ level ];
                *( color++ ) = c.r();
                *( color++ ) = c.g();
                *( color++ ) = c.b();
            }
        }
        else if ( veclen == 3 )
        {
            for ( size_t i = 0, index = 0; i < nnodes; i++, index += 3 )
            {
                const kvs::Vector3f v(
                    static_cast<float>(value[index]),
                    static_cast<float>(value[index+1]),
                    static_cast<float>(value[index+2]));
                const float d = normalize * ( static_cast<float>(v.length()) - min_value );
                const size_t level = static_cast<size_t>( 255.0f * d );
                const kvs::RGBColor c = color_map[ level ];
                *( color++ ) = c.r();
                *( color++ ) = c.g();
                *( color++ ) = c.b();
            }
        }
        break;
    }
    default:
        break;
    }

    this->setColors( colors );
}

template void GlyphBase::calculate_colors<kvs::Int8>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_colors<kvs::Int16>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_colors<kvs::Int32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_colors<kvs::Int64>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_colors<kvs::UInt8>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_colors<kvs::UInt16>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_colors<kvs::UInt32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_colors<kvs::UInt64>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_colors<kvs::Real32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_colors<kvs::Real64>( const kvs::VolumeObjectBase* volume );

/*===========================================================================*/
/**
 *  @brief  Calculates the opacity value array.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
template <typename T>
void GlyphBase::calculate_opacities( const kvs::VolumeObjectBase* volume )
{
    const T* value = reinterpret_cast<const T*>( volume->values().pointer() );
    const size_t veclen = volume->veclen();
    const size_t nnodes = volume->nnodes();

    if ( !volume->hasMinMaxValues() ) { volume->updateMinMaxValues(); }
    const kvs::Real32 min_value = static_cast<kvs::Real32>(volume->minValue());
    const kvs::Real32 max_value = static_cast<kvs::Real32>(volume->maxValue());
    const kvs::Real32 normalize = 255.0f / ( max_value - min_value );

    kvs::ValueArray<kvs::UInt8> opacities( nnodes );
    kvs::UInt8* opacity = opacities.pointer();

    switch( m_opacity_mode )
    {
    case GlyphBase::OpacityByDefault:
        for( size_t i = 0; i < nnodes; i++ ) opacity[i] = ::DefaultOpacity;
        break;
    case GlyphBase::OpacityByMagnitude:
        if ( veclen == 1 )
        {
            for( size_t i = 0; i < nnodes; i++ )
            {
                opacity[i] = static_cast<kvs::UInt8>( normalize * ( static_cast<float>(value[i]) - min_value ) );
            }
        }
        else if ( veclen == 3 )
        {
            for( size_t i = 0, index = 0; i < nnodes; i++, index += 3 )
            {
                const kvs::Vector3f v(
                    static_cast<float>(value[index]),
                    static_cast<float>(value[index+1]),
                    static_cast<float>(value[index+2]));
                opacity[i] = static_cast<kvs::UInt8>( normalize * ( v.length() - min_value ) );
            }
        }
        break;
    default:
        break;
    }

    this->setOpacities( opacities );
}

template void GlyphBase::calculate_opacities<kvs::Int8>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_opacities<kvs::Int16>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_opacities<kvs::Int32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_opacities<kvs::Int64>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_opacities<kvs::UInt8>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_opacities<kvs::UInt16>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_opacities<kvs::UInt32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_opacities<kvs::UInt64>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_opacities<kvs::Real32>( const kvs::VolumeObjectBase* volume );
template void GlyphBase::calculate_opacities<kvs::Real64>( const kvs::VolumeObjectBase* volume );

} // end of namespace kvs
