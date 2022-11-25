/****************************************************************************/
/**
 *  @file ImageBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImageBase.cpp 605 2010-08-21 05:08:32Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ImageBase.h"
#include "ColorImage.h"
#include "GrayImage.h"
#include "RGBColor.h"
#include <kvs/Type>


namespace
{

const kvs::UInt8 BilinearInterpolate(
    const kvs::UInt8 pixel1,
    const kvs::UInt8 pixel2,
    const kvs::UInt8 pixel3,
    const kvs::UInt8 pixel4,
    const double     xrate,
    const double     yrate )
{
    const double d = pixel1 * ( 1.0 - xrate ) + pixel3 * xrate;
    const double e = pixel2 * ( 1.0 - xrate ) + pixel4 * xrate;

    int f = kvs::Math::Round( d * ( 1.0 - yrate ) + e * yrate );
    kvs::Math::Clamp( f, 0, 255 );

    return( static_cast<kvs::UInt8>( f ) );
}

const kvs::RGBColor BilinearInterpolate(
    const kvs::RGBColor pixel1,
    const kvs::RGBColor pixel2,
    const kvs::RGBColor pixel3,
    const kvs::RGBColor pixel4,
    const double        xrate,
    const double        yrate )
{
    const double d[3] = {
        pixel1.r() * ( 1.0 - xrate ) + pixel3.r() * xrate,
        pixel1.g() * ( 1.0 - xrate ) + pixel3.g() * xrate,
        pixel1.b() * ( 1.0 - xrate ) + pixel3.b() * xrate
    };

    const double e[3] = {
        pixel2.r() * ( 1.0 - xrate ) + pixel4.r() * xrate,
        pixel2.g() * ( 1.0 - xrate ) + pixel4.g() * xrate,
        pixel2.b() * ( 1.0 - xrate ) + pixel4.b() * xrate
    };

    int f[3] = {
        kvs::Math::Round( d[0] * ( 1.0 - yrate ) + e[0] * yrate ),
        kvs::Math::Round( d[1] * ( 1.0 - yrate ) + e[1] * yrate ),
        kvs::Math::Round( d[2] * ( 1.0 - yrate ) + e[2] * yrate )
    };

    kvs::Math::Clamp( f[0], 0, 255 );
    kvs::Math::Clamp( f[1], 0, 255 );
    kvs::Math::Clamp( f[2], 0, 255 );

    const kvs::UInt8 r = static_cast<kvs::UInt8>( f[0] );
    const kvs::UInt8 g = static_cast<kvs::UInt8>( f[1] );
    const kvs::UInt8 b = static_cast<kvs::UInt8>( f[2] );

    return( kvs::RGBColor( r, g, b ) );
}

/*==========================================================================*/
/**
 *  Convert 'bit' to 'byte'.
 *  @param value [in] bit value
 *  @return converted byte value from bit value
 */
/*==========================================================================*/
inline size_t BitToByte( size_t value )
{
    return( value >> 3 );
}

/*==========================================================================*/
/**
 *  Convert 'byte' to 'bit'.
 *  @param value [in] byte value
 *  @return converted bit value from byte value
 */
/*==========================================================================*/
inline size_t ByteToBit( size_t value )
{
    return( value << 3 );
}

} // end of namespace


namespace kvs
{

/*==========================================================================*/
/**
 *  Default constructor.
 */
/*==========================================================================*/
ImageBase::ImageBase( void ):
    m_width( 0 ),
    m_height( 0 ),
    m_npixels( 0 ),
    m_padding( 0 ),
    m_bpp( 0 ),
    m_bpl( 0 ),
    m_size( 0 )
{
}

/*==========================================================================*/
/**
 *  Constructor for creation.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param type [in] pixel type
 */
/*==========================================================================*/
ImageBase::ImageBase( const size_t width, const size_t height, const ImageType type )
{
    this->create( width, height, type );
}

/*==========================================================================*/
/**
 *  Constructor for creation.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param type [in] pixel type
 *  @param data [in] pixel data
 */
/*==========================================================================*/
ImageBase::ImageBase(
    const size_t width,
    const size_t height,
    const ImageType type,
    const kvs::UInt8* data )
{
    this->create( width, height, type, data );
}

/*==========================================================================*/
/**
 *  Constructor for creation.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param type [in] pixel type
 *  @param data [in] pixel data
 */
/*==========================================================================*/
ImageBase::ImageBase(
    const size_t width,
    const size_t height,
    const ImageType type,
    const kvs::ValueArray<kvs::UInt8>& data )
{
    this->create( width, height, type, data );
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
ImageBase::~ImageBase( void )
{
    this->clear();
}

/*==========================================================================*/
/**
 *  Clear.
 */
/*==========================================================================*/
void ImageBase::clear( void )
{
    m_width   = 0;
    m_height  = 0;
    m_npixels = 0;
    m_padding = 0;
    m_bpp     = 0;
    m_bpl     = 0;
    m_size    = 0;
    m_data.deallocate();
}

/*===========================================================================*/
/**
 *  @brief  Copy the specified image data.
 *  @param  image [in] image data
 */
/*===========================================================================*/
void ImageBase::copy( const ImageBase& image )
{
    m_width   = image.m_width;
    m_height  = image.m_height;
    m_npixels = image.m_npixels;
    m_padding = image.m_padding;
    m_bpp     = image.m_bpp;
    m_bpl     = image.m_bpl;
    m_size    = image.m_size;
    m_data    = image.m_data;
}

/*==========================================================================*/
/**
 *  Create the image.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param type [in] pixel type
 *  @return true, if the create process is done successfully
 */
/*==========================================================================*/
const bool ImageBase::create( const size_t width, const size_t height, const ImageType type )
{
    m_width   = width;
    m_height  = height;
    m_npixels = width * height;

    if( type == Bit )
    {
        m_padding = ::ByteToBit( ::BitToByte( width + 7 ) ) - width;
        m_bpp     = 1;
        m_bpl     = ::BitToByte( width + 7 );
    }
    else
    {
        const size_t ncomponents = static_cast<size_t>( type );
        m_padding = 0;
        m_bpp     = ::ByteToBit( sizeof(kvs::UInt8) ) * ncomponents;
        m_bpl     = width * ::BitToByte( m_bpp );
    }

    m_size = height * m_bpl;
    if ( !m_data.allocate( m_size ) ) return( false );

    m_data.fill( 0x00 );
    return( true );
}

/*==========================================================================*/
/**
 *  Create the image.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param type [in] pixel type
 *  @param data [in] pixel data
 *  @return true, if the create process is done successfully
 */
/*==========================================================================*/
const bool ImageBase::create(
    const size_t width,
    const size_t height,
    const ImageType type,
    const kvs::UInt8* data )
{
    m_width   = width;
    m_height  = height;
    m_npixels = width * height;

    if( type == Bit )
    {
        m_padding = ::ByteToBit( ::BitToByte( width + 7 ) ) - width;
        m_bpp     = 1;
        m_bpl     = ::BitToByte( width + 7 );
    }
    else
    {
        const size_t ncomponents = static_cast<size_t>( type );
        m_padding = 0;
        m_bpp     = ::ByteToBit( sizeof(kvs::UInt8) ) * ncomponents;
        m_bpl     = width * ::BitToByte( m_bpp );
    }

    m_size = height * m_bpl;
    m_data.deepCopy( data, m_size );

    if ( !m_data.pointer() ) return( false );

    return( true );
}

/*==========================================================================*/
/**
 *  Create the image.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param type [in] pixel type
 *  @param data [in] pixel data
 *  @return true, if the create process is done successfully
 */
/*==========================================================================*/
const bool ImageBase::create(
    const size_t width,
    const size_t height,
    const ImageType type,
    const kvs::ValueArray<kvs::UInt8>& data )
{
    m_width   = width;
    m_height  = height;
    m_npixels = width * height;

    if( type == Bit )
    {
        m_padding = ::ByteToBit( ::BitToByte( width + 7 ) ) - width;
        m_bpp     = 1;
        m_bpl     = ::BitToByte( width + 7 );
    }
    else
    {
        const size_t ncomponents = static_cast<size_t>( type );
        m_padding = 0;
        m_bpp     = ::ByteToBit( sizeof(kvs::UInt8) ) * ncomponents;
        m_bpl     = width * ::BitToByte( m_bpp );
    }

    m_size = height * m_bpl;
    if( m_size != data.size() ) return( false );

    m_data = data;

    return( true );
}

/*==========================================================================*/
/**
 *  Get the image width.
 */
/*==========================================================================*/
const size_t ImageBase::width( void ) const
{
    return( m_width );
}

/*==========================================================================*/
/**
 *  Get the image height.
 */
/*==========================================================================*/
const size_t ImageBase::height( void ) const
{
    return( m_height );
}

/*==========================================================================*/
/**
 *  Get the number of bytes per line.
 */
/*==========================================================================*/
const size_t ImageBase::bytesPerLine( void ) const
{
    return( m_bpl );
}

/*==========================================================================*/
/**
 *  Get the number of bits per pixel.
 */
/*==========================================================================*/
const size_t ImageBase::bitsPerPixel( void ) const
{
    return( m_bpp );
}

/*==========================================================================*/
/**
 *  Get the number of pixels.
 */
/*==========================================================================*/
const size_t ImageBase::npixels( void ) const
{
    return( m_npixels );
}

/*==========================================================================*/
/**
 *  Get the number of padding bits.
 */
/*==========================================================================*/
const size_t ImageBase::padding( void ) const
{
    return( m_padding );
}

/*==========================================================================*/
/**
 *  Get data size [byte].
 */
/*==========================================================================*/
const size_t ImageBase::size( void ) const
{
    return( m_size );
}

/*==========================================================================*/
/**
 *  Get the pointer to the pixel data.
 */
/*==========================================================================*/
const kvs::ValueArray<kvs::UInt8>& ImageBase::data( void ) const
{
    return( m_data );
}

/*==========================================================================*/
/**
 *  Get the pointer to the pixel data.
 */
/*==========================================================================*/
kvs::ValueArray<kvs::UInt8>& ImageBase::data( void )
{
    return( m_data );
}

/*===========================================================================*/
/**
 *  @brief  Flip the image data.
 */
/*===========================================================================*/
void ImageBase::flip( void )
{
    const size_t stride = m_width * ::BitToByte( m_bpp );

    kvs::UInt8* pdata = m_data.pointer();
    const size_t end_line = m_height / 2;
    for ( size_t i = 0; i < end_line; i++ )
    {
        kvs::UInt8* src = pdata + ( i * stride );
        kvs::UInt8* dst = pdata + ( ( m_height - i - 1 ) * stride );
        for ( size_t j = 0; j < stride; j++ )
        {
            kvs::UInt8 tmp = *src;
            *src = *dst;
            *dst = tmp;
            src++;
            dst++;
        }
    }
}

template <typename ImageDataType, typename Interpolator>
void ImageBase::resize( const size_t width, const size_t height, ImageDataType* image )
{
    // Resized image.
    ImageDataType resized_image( width, height );

    // Interpolator.
    Interpolator interpolator;
    interpolator.attach( image );

    const double ratio_width  = m_width / static_cast<double>( width );
    const double ratio_height = m_height / static_cast<double>( height );
    for( size_t j = 0; j < height; j++ )
    {
        const double v = j * ratio_height;
        interpolator.setV( v );

        for( size_t i = 0; i < width; i++ )
        {
            const double u = i * ratio_width;
            interpolator.setU( u );

            typename ImageDataType::PixelType pixel = interpolator();
            resized_image.set( i, j, pixel );
        }
    }

    *image = resized_image;
}

// Specialization.
template
void ImageBase::resize<kvs::GrayImage,ImageBase::NearestNeighborInterpolatorGray>(
    const size_t width,
    const size_t height,
    kvs::GrayImage* image );

template
void ImageBase::resize<kvs::ColorImage,ImageBase::NearestNeighborInterpolatorColor>(
    const size_t  width,
    const size_t  height,
    kvs::ColorImage* image );

template
void ImageBase::resize<kvs::GrayImage,ImageBase::BilinearInterpolatorGray>(
    const size_t width,
    const size_t height,
    kvs::GrayImage* image );

template
void ImageBase::resize<kvs::ColorImage,ImageBase::BilinearInterpolatorColor>(
    const size_t width,
    const size_t height,
    kvs::ColorImage* image );

template <typename ImageDataType>
void ImageBase::NearestNeighborInterpolator<ImageDataType>::attach( const ImageDataType* image )
{
    m_reference_image = image;
}

template <typename ImageDataType>
void ImageBase::NearestNeighborInterpolator<ImageDataType>::setU( const double u )
{
    m_p.x() = kvs::Math::Floor( u );
}

template <typename ImageDataType>
void ImageBase::NearestNeighborInterpolator<ImageDataType>::setV( const double v )
{
    m_p.y() = kvs::Math::Floor( v );
}

template <typename ImageDataType>
const typename ImageDataType::PixelType
ImageBase::NearestNeighborInterpolator<ImageDataType>::operator () ( void ) const
{
    const size_t x = static_cast<size_t>( m_p.x() );
    const size_t y = static_cast<size_t>( m_p.y() );
    return( m_reference_image->pixel( x, y ) );
}

// Specialization.
template class ImageBase::NearestNeighborInterpolator<kvs::GrayImage>;
template class ImageBase::NearestNeighborInterpolator<kvs::ColorImage>;


template <typename ImageDataType>
void ImageBase::BilinearInterpolator<ImageDataType>::attach( const ImageDataType* image )
{
    m_reference_image = image;
}

template <typename ImageDataType>
void ImageBase::BilinearInterpolator<ImageDataType>::setU( const double u )
{
    const double width = static_cast<double>( m_reference_image->width() - 1 );

    m_pmin.x() = kvs::Math::Floor( u );
    m_pmax.x() = m_pmin.x() + ( width > m_pmin.x() ? 1.0 : 0.0 );
    m_rate.x() = u - static_cast<double>( m_pmin.x() );
}

template <typename ImageDataType>
void ImageBase::BilinearInterpolator<ImageDataType>::setV( const double v )
{
    const double height = static_cast<double>( m_reference_image->height() - 1 );

    m_pmin.y() = kvs::Math::Floor( v );
    m_pmax.y() = m_pmin.y() + ( height > m_pmin.y() ? 1.0 : 0.0 );
    m_rate.y() = v - static_cast<double>( m_pmin.y() );
}

template <typename ImageDataType>
const typename ImageDataType::PixelType
ImageBase::BilinearInterpolator<ImageDataType>::operator () ( void ) const
{
    const size_t pmin_x = static_cast<size_t>( m_pmin.x() );
    const size_t pmin_y = static_cast<size_t>( m_pmin.y() );
    const size_t pmax_x = static_cast<size_t>( m_pmax.x() );
    const size_t pmax_y = static_cast<size_t>( m_pmax.y() );

    typename ImageDataType::PixelType p1 = m_reference_image->pixel( pmin_x, pmin_y );
    typename ImageDataType::PixelType p2 = m_reference_image->pixel( pmin_x, pmax_y );
    typename ImageDataType::PixelType p3 = m_reference_image->pixel( pmax_x, pmin_y );
    typename ImageDataType::PixelType p4 = m_reference_image->pixel( pmax_x, pmax_y );

    return( BilinearInterpolate( p1, p2, p3, p4, m_rate.x(), m_rate.y() ) );
}

// Instantiation.
template class ImageBase::BilinearInterpolator<kvs::GrayImage>;
template class ImageBase::BilinearInterpolator<kvs::ColorImage>;

} // end of namespace kvs
