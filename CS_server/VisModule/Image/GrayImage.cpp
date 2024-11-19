/****************************************************************************/
/**
 *  @file GrayImage.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GrayImage.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "GrayImage.h"
#include "ColorImage.h"
#include "BitImage.h"
#include <kvs/IgnoreUnusedVariable>
#include <kvs/Math>
#include <kvs/File>
#include <kvs/KVSMLObjectImage>
#include <kvs/Bmp>
#include <kvs/Ppm>
#include <kvs/Pbm>
#include <kvs/Pgm>
#include <kvs/Tiff>
#ifndef NO_CLIENT
#include <kvs/Dicom>
#endif
#include <algorithm>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Gray-scaling by the mean-value method.
 *  @param  image [in] color image
 *  @param  data [out] pixel data array
 */
/*===========================================================================*/
void GrayImage::MeanValue::operator () (
    const kvs::ColorImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const size_t width = image.width();
    const size_t height = image.height();
    const kvs::UInt8* image_data = image.data().pointer();
    for( size_t j = 0; j < height; j++ )
    {
        const size_t col_line_index = j * image.bytesPerLine();
        const size_t gry_line_index = j * image.width();
        for( size_t i = 0; i < width; i++ )
        {
            const size_t col_pixel_index = col_line_index + 3 * i;
            const size_t gry_pixel_index = gry_line_index + i;

            unsigned int value = 0;
            value += image_data[ col_pixel_index + 0 ];
            value += image_data[ col_pixel_index + 1 ];
            value += image_data[ col_pixel_index + 2 ];

            data[ gry_pixel_index ] = static_cast<kvs::UInt8>(value / 3);
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Gray-scaling by the middle-value method.
 *  @param  image [in] color image
 *  @param  data [out] pixel data array
 */
/*===========================================================================*/
void GrayImage::MiddleValue::operator () (
    const kvs::ColorImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const size_t width = image.width();
    const size_t height = image.height();
    const kvs::UInt8* image_data = image.data().pointer();
    for( size_t j = 0; j < height; j++ )
    {
        const size_t col_line_index = j * image.bytesPerLine();
        const size_t gry_line_index = j * image.width();
        for( size_t i = 0; i < width; i++ )
        {
            const size_t col_pixel_index = col_line_index + 3 * i;
            const size_t gry_pixel_index = gry_line_index + i;

            const unsigned int r = image_data[ col_pixel_index + 0 ];
            const unsigned int g = image_data[ col_pixel_index + 1 ];
            const unsigned int b = image_data[ col_pixel_index + 2 ];
            const unsigned int max = kvs::Math::Max( r, g, b );
            const unsigned int min = kvs::Math::Min( r, g, b );
            const unsigned int value = ( max + min ) / 2;

            data[ gry_pixel_index ] = static_cast<kvs::UInt8>(value);
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Gray-scaling by the median-value method.
 *  @param  image [in] color image
 *  @param  data [out] pixel data array
 */
/*===========================================================================*/
void GrayImage::MedianValue::operator () (
    const kvs::ColorImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const size_t width = image.width();
    const size_t height = image.height();
    const kvs::UInt8* image_data = image.data().pointer();
    for( size_t j = 0; j < height; j++ )
    {
        const size_t col_line_index = j * image.bytesPerLine();
        const size_t gry_line_index = j * image.width();
        for( size_t i = 0; i < width; i ++ )
        {
            const size_t col_pixel_index = col_line_index + 3 * i;
            const size_t gry_pixel_index = gry_line_index + i;

            unsigned int pixel[3];
            pixel[0] = image_data[ col_pixel_index + 0 ];
            pixel[1] = image_data[ col_pixel_index + 1 ];
            pixel[2] = image_data[ col_pixel_index + 2 ];
            std::sort( pixel, pixel + 3 );

            data[ gry_pixel_index ] = static_cast<kvs::UInt8>(pixel[1]);
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Gray-scaling by the NTSC weighted mean-value method.
 *  @param  image [in] color image
 *  @param  data [out] pixel data array
 */
/*===========================================================================*/
void GrayImage::NTSCWeightedMeanValue::operator () (
    const kvs::ColorImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const size_t width = image.width();
    const size_t height = image.height();
    const kvs::UInt8* image_data = image.data().pointer();
    for( size_t j = 0; j < height; j++ )
    {
        const size_t col_line_index = j * image.bytesPerLine();
        const size_t gry_line_index = j * image.width();
        for( size_t i = 0; i < width; i++ )
        {
            const size_t col_pixel_index = col_line_index + 3 * i;
            const size_t gry_pixel_index = gry_line_index + i;

            const unsigned int r = image_data[ col_pixel_index + 0 ];
            const unsigned int g = image_data[ col_pixel_index + 1 ];
            const unsigned int b = image_data[ col_pixel_index + 2 ];

            /* value = ( 0.298912 * R + 0.586611 * G + 0.114478 * B )
             *       = ( 2 * R + 4 * G + B ) / 7
             */
            const unsigned int value = ( 2 * r + 4 * g + b ) / 7;

            data[ gry_pixel_index ] = static_cast<kvs::UInt8>(value);
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Gray-scaling by the HDTV weighted mean-value method.
 *  @param  image [in] color image
 *  @param  data [out] pixel data array
 */
/*===========================================================================*/
void GrayImage::HDTVWeightedMeanValue::operator () (
    const kvs::ColorImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const double gamma_value = 2.2;
    const size_t width = image.width();
    const size_t height = image.height();
    const kvs::UInt8* image_data = image.data().pointer();
    for( size_t j = 0; j < height; j++ )
    {
        const size_t col_line_index = j * image.bytesPerLine();
        const size_t gry_line_index = j * image.width();
        for( size_t i = 0; i < width; i++ )
        {
            const size_t col_pixel_index = col_line_index + 3 * i;
            const size_t gry_pixel_index = gry_line_index + i;

            const unsigned int r = image_data[ col_pixel_index + 0 ];
            const unsigned int g = image_data[ col_pixel_index + 1 ];
            const unsigned int b = image_data[ col_pixel_index + 2 ];

            const double R = static_cast<double>(r) / 255.0;
            const double G = static_cast<double>(g) / 255.0;
            const double B = static_cast<double>(b) / 255.0;

            const double RR = std::pow( R, gamma_value ) * 0.222015;
            const double GG = std::pow( G, gamma_value ) * 0.706655;
            const double BB = std::pow( B, gamma_value ) * 0.071330;

            const double V = std::pow( ( RR + GG + BB ), ( 1.0 / gamma_value ) );
            const unsigned int value = kvs::Math::Round( V * 255.0 );

            data[ gry_pixel_index ] = static_cast<kvs::UInt8>(value);
        }
    }
}

/*==========================================================================*/
/**
 *  Constructs a new gray-scale image.
 */
/*==========================================================================*/
GrayImage::GrayImage( void )
{
}

/*==========================================================================*/
/**
 *  Constructs a new gray-scale image.
 *  @param width [in] image width
 *  @param height [in] image height
 */
/*==========================================================================*/
GrayImage::GrayImage( const size_t width, const size_t height ):
    kvs::ImageBase( width, height, kvs::ImageBase::Gray )
{
}

/*==========================================================================*/
/**
 *  Constructs a new gray-scale image.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param data [in] pointer to pixel data
 */
/*==========================================================================*/
GrayImage::GrayImage(
    const size_t width,
    const size_t height,
    const kvs::UInt8* data ):
    kvs::ImageBase( width, height, kvs::ImageBase::Gray, data )
{
}

/*==========================================================================*/
/**
 *  Constructs a new gray-scale image.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param data [in] pixel data array
 */
/*==========================================================================*/
GrayImage::GrayImage(
    const size_t width,
    const size_t height,
    const kvs::ValueArray<kvs::UInt8>& data ):
    kvs::ImageBase( width, height, kvs::ImageBase::Gray, data )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a copy of other gray image.
 *  @param  image [in] gray image
 */
/*===========================================================================*/
GrayImage::GrayImage( const kvs::GrayImage& image )
{
    BaseClass::copy( image );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a copy of other bit image.
 *  @param  image [in] bit image
 */
/*===========================================================================*/
GrayImage::GrayImage( const kvs::BitImage& image )
{
    this->read_image( image );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new gray-scale image from the color image.
 *  @param  image [in] color image
 */
/*===========================================================================*/
GrayImage::GrayImage( const kvs::ColorImage& image ):
    kvs::ImageBase( image.width(), image.height(), kvs::ImageBase::Gray )
{
    GrayImage::MeanValue method;
    method( image, m_data );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new GrayImage class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
GrayImage::GrayImage( const std::string& filename )
{
    this->read( filename );
}

/*==========================================================================*/
/**
 *  Destroys the gray-scale image.
 */
/*==========================================================================*/
GrayImage::~GrayImage( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Copys other gray image.
 *  @param  image [in] gray image
 */
/*===========================================================================*/
kvs::GrayImage& GrayImage::operator = ( const kvs::GrayImage& image )
{
    BaseClass::copy( image );
    return( *this );
}

/*==========================================================================*/
/**
 *  Returns the pixel value.
 *  @param index [in] pixel index
 *  @return pixel value
 */
/*==========================================================================*/
const kvs::UInt8 GrayImage::pixel( const size_t index ) const
{
    return( m_data[ index ] );
}

/*==========================================================================*/
/**
 *  Returns the pixel value.
 *  @param i [in] pixel index along the horizontal axis
 *  @param j [in] pixel index along the vertical axis
 *  @return pixel value
 */
/*==========================================================================*/
const kvs::UInt8 GrayImage::pixel( const size_t i, const size_t j ) const
{
    return( m_data[ m_width * j + i ] );
}

/*==========================================================================*/
/**
 *  Set the pixel value.
 *  @param index [in] pixel index
 *  @param pixel [in] pixel value
 */
/*==========================================================================*/
void GrayImage::set( const size_t index, const kvs::UInt8 pixel )
{
    m_data[ index ] = pixel;
}

/*==========================================================================*/
/**
 *  Set the pixel value.
 *  @param i [in] pixel index along the horizontal axis
 *  @param j [in] pixel index along the vertical axis
 *  @param pixel [in] pixel value
 */
/*==========================================================================*/
void GrayImage::set( const size_t i, const size_t j, const kvs::UInt8 pixel )
{
    m_data[ m_width * j + i ] = pixel;
}

/*===========================================================================*/
/**
 *  @brief  Scales the image data.
 *  @param  ratio [in] scaling ratio
 */
/*===========================================================================*/
void GrayImage::scale( const double ratio )
{
    const size_t width = static_cast<size_t>( this->width() * ratio );
    const size_t height = static_cast<size_t>( this->height() * ratio );
    BaseClass::resize<GrayImage,GrayImage::Bilinear>( width, height, this );
}

/*===========================================================================*/
/**
 *  @brief  Scales the image data.
 *  @param  ratio [in] scaling ratio
 *  @param  method [in] Interpolation method
 */
/*===========================================================================*/
template <typename InterpolationMethod>
void GrayImage::scale( const double ratio, InterpolationMethod method )
{
    kvs::IgnoreUnusedVariable( method );

    const size_t width = static_cast<size_t>( this->width() * ratio );
    const size_t height = static_cast<size_t>( this->height() * ratio );
    BaseClass::resize<GrayImage,InterpolationMethod>( width, height, this );
}

// Specialization.
template
void GrayImage::scale( const double ratio, GrayImage::NearestNeighbor method );

template
void GrayImage::scale( const double ratio, GrayImage::Bilinear method );

/*===========================================================================*/
/**
 *  @brief  Resizes the image data.
 *  @param  width  [in] resized width
 *  @param  height [in] resized height
 */
/*===========================================================================*/
void GrayImage::resize( const size_t width, const size_t height )
{
    BaseClass::resize<GrayImage,GrayImage::Bilinear>( width, height, this );
}

/*===========================================================================*/
/**
 *  @brief  Resizes the image data.
 *  @param  width [in] resized width
 *  @param  height [in] resized height
 *  @param  method [in] Interpolation method
 */
/*===========================================================================*/
template <typename InterpolationMethod>
void GrayImage::resize( const size_t width, const size_t height, InterpolationMethod method )
{
    kvs::IgnoreUnusedVariable( method );

    BaseClass::resize<GrayImage,InterpolationMethod>( width, height, this );
}

// Specialization.
template
void GrayImage::resize( const size_t width, const size_t height, GrayImage::NearestNeighbor method );

template
void GrayImage::resize( const size_t width, const size_t height, GrayImage::Bilinear method );

/*==========================================================================*/
/**
 *  Read a image file.
 *  @param filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*==========================================================================*/
const bool GrayImage::read( const std::string& filename )
{
    // KVSML image.
    if ( kvs::KVSMLObjectImage::CheckFileExtension( filename ) )
    {
        const kvs::KVSMLObjectImage kvsml( filename );
        if ( kvsml.pixelType() == "color" )
        {
            kvs::ColorImage image( kvsml.width(), kvsml.height(), kvsml.data() );
            return( this->read_image( image ) );
        }
        if ( kvsml.pixelType() == "gray" )
        {
            const BaseClass::ImageType type = BaseClass::Gray;
            return( BaseClass::create( kvsml.width(), kvsml.height(), type, kvsml.data() ) );
        }
    }

    // Bitmap and PPM image.
    if ( kvs::Bmp::CheckFileExtension( filename ) ||
         kvs::Ppm::CheckFileExtension( filename ) )
    {
        kvs::ColorImage image; image.read( filename );
        return( this->read_image( image ) );
    }

    // PGM image.
    if ( kvs::Pgm::CheckFileExtension( filename ) )
    {
        const kvs::Pgm pgm( filename );
        const BaseClass::ImageType type = BaseClass::Gray;
        return( BaseClass::create( pgm.width(), pgm.height(), type, pgm.data() ) );
    }

    // PBM image.
    if ( kvs::Pbm::CheckFileExtension( filename ) )
    {
        kvs::BitImage image; image.read( filename );
        return( this->read_image( image ) );
    }

    // TIFF image.
    if ( kvs::Tiff::CheckFileExtension( filename ) )
    {
        const kvs::Tiff tiff( filename );
        if ( tiff.colorMode() == kvs::Tiff::Color24 )
        {
            const kvs::UInt8* data = static_cast<kvs::UInt8*>(tiff.rawData().pointer());
            kvs::ColorImage image( tiff.width(), tiff.height(), data );
            return( this->read_image( image ) );
        }
        if ( tiff.colorMode() == kvs::Tiff::Gray8 )
        {
            const kvs::UInt8* data = static_cast<kvs::UInt8*>(tiff.rawData().pointer());
            const BaseClass::ImageType type = BaseClass::Gray;
            return( BaseClass::create( tiff.width(), tiff.height(), type, data ) );
        }
        if ( tiff.colorMode() == kvs::Tiff::Gray16 )
        {
            kvsMessageError( "TIFF image (16bits gray-scale) is not supported." );
            return( false );
        }
    }

    // DICOM image.
#ifndef NO_CLIENT
    if ( kvs::Dicom::CheckFileExtension( filename ) )
    {
        const kvs::Dicom dcm( filename );
        const BaseClass::ImageType type = BaseClass::Gray;
        return( BaseClass::create( dcm.width(), dcm.height(), type, dcm.pixelData() ) );
    }
#endif

    kvsMessageError( "Read-method for %s is not implemented.",
                     filename.c_str() );

    return( false );
}

/*==========================================================================*/
/**
 *  Write a image file.
 *  @param filename [in] filename
 *  @return true, if the writing process is done successfully
 */
/*==========================================================================*/
const bool GrayImage::write( const std::string& filename )
{
    // KVSML image.
    if ( kvs::KVSMLObjectImage::CheckFileExtension( filename ) )
    {
        kvs::KVSMLObjectImage kvsml;
        kvsml.setWidth( m_width );
        kvsml.setHeight( m_height );
        kvsml.setPixelType( "gray" );
        kvsml.setWritingDataType( kvs::KVSMLObjectImage::Ascii );
        kvsml.setData( m_data );
        return( kvsml.write( filename ) );
    }

    // Bitmap and PPM image.
    if ( kvs::Bmp::CheckFileExtension( filename ) ||
         kvs::Ppm::CheckFileExtension( filename ) )
    {
        kvs::ColorImage image( *this );
        return( image.write( filename ) );
    }

    // PGM image.
    if ( kvs::Pgm::CheckFileExtension( filename ) )
    {
        kvs::Pgm pgm( m_width, m_height, m_data );
        return( pgm.write( filename ) );
    }

    // PBM image.
    if ( kvs::Pbm::CheckFileExtension( filename ) )
    {
        kvs::BitImage image( *this );
        return( image.write( filename ) );
    }

    kvsMessageError( "Write-method for %s is not implemented.",
                     filename.c_str() );

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Reads the color image.
 *  @param  image [in] color image
 *  @return true, if the reading process is done successfully.
 */
/*===========================================================================*/
const bool GrayImage::read_image( const kvs::ColorImage& image )
{
    if ( !BaseClass::create( image.width(), image.height(), BaseClass::Gray ) )
    {
        return( false );
    }

    GrayImage::MeanValue method;
    method( image, m_data );

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Reads the bit image.
 *  @param  image [in] bit image
 *  @return true, if the reading process is done successfully.
 */
/*===========================================================================*/
const bool GrayImage::read_image( const kvs::BitImage& image )
{
    if ( !BaseClass::create( image.width(), image.height(), kvs::ImageBase::Gray ) )
    {
        return( false );
    }

    const size_t width = image.width();
    const size_t height = image.height();
    size_t index = 0;
    for ( size_t j = 0; j < height; j++ )
    {
        for ( size_t i = 0; i < width; i++, index++ )
        {
            const kvs::UInt8 pixel = image.pixel( i, j ) ? 255 : 0;
            m_data[ index ] = pixel;
        }
    }

    return( true );
}

} // end of namespace kvs
