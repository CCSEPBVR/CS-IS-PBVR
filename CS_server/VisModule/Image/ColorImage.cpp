/****************************************************************************/
/**
 *  @file ColorImage.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ColorImage.cpp 842 2011-06-10 07:48:28Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#include "ColorImage.h"
#include "GrayImage.h"
#include "BitImage.h"
#include <kvs/IgnoreUnusedVariable>
#include <kvs/KVSMLObjectImage>
#include <kvs/RGBColor>
#include <kvs/File>
#include <kvs/Bmp>
#include <kvs/Ppm>
#include <kvs/Pgm>
#include <kvs/Pbm>
#include <kvs/Tiff>
#ifndef NO_CLIENT
#include <kvs/Dicom>
#endif


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructs a new color image.
 */
/*==========================================================================*/
ColorImage::ColorImage( void )
{
}

/*==========================================================================*/
/**
 *  Constructs a new color image.
 *  @param width [in] image width
 *  @param height [in] image height
 */
/*==========================================================================*/
ColorImage::ColorImage( const size_t width, const size_t height ):
    kvs::ImageBase( width, height, kvs::ImageBase::Color )
{
}

/*==========================================================================*/
/**
 *  Constructs a new color image from the given pixel data.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param data [in] pointer to pixel data
 */
/*==========================================================================*/
ColorImage::ColorImage(
    const size_t width,
    const size_t height,
    const kvs::UInt8* data ):
    kvs::ImageBase( width, height, kvs::ImageBase::Color, data )
{
}

/*==========================================================================*/
/**
 *  Constructs a new color image from the given pixel data.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param data [in] pixel data array
 */
/*==========================================================================*/
ColorImage::ColorImage(
    const size_t width,
    const size_t height,
    const kvs::ValueArray<kvs::UInt8>& data ):
    kvs::ImageBase( width, height, kvs::ImageBase::Color, data )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a copy of other color image.
 *  @param  image [in] color image
 */
/*===========================================================================*/
ColorImage::ColorImage( const kvs::ColorImage& image )
{
    BaseClass::copy( image );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a copy of other gray image.
 *  @param  image [in] gray image
 */
/*===========================================================================*/
ColorImage::ColorImage( const kvs::GrayImage& image )
{
    this->read_image( image );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a copy of other bit image.
 *  @param  image [in] bit image
 */
/*===========================================================================*/
ColorImage::ColorImage( const kvs::BitImage& image )
{
    this->read_image( image );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new ColorImage class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
ColorImage::ColorImage( const std::string& filename )
{
    this->read( filename );
}

/*==========================================================================*/
/**
 *  Destroys the color image.
 */
/*==========================================================================*/
ColorImage::~ColorImage( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Copys other color image.
 *  @param  image [in] color image
 */
/*===========================================================================*/
kvs::ColorImage& ColorImage::operator = ( const kvs::ColorImage& image )
{
    BaseClass::copy( image );
    return( *this );
}

/*==========================================================================*/
/**
 *  Returns the red component.
 *  @param index [in] pixel index
 *  @return red component
 */
/*==========================================================================*/
const kvs::UInt8 ColorImage::r( const size_t index ) const
{
    return( m_data[ 3 * index ] );
}

/*==========================================================================*/
/**
 *  Returns the red component.
 *  @param i [in] pixel index along the horizontal axis
 *  @param j [in] pixel index along the vertical axis
 *  @return red component
 */
/*==========================================================================*/
const kvs::UInt8 ColorImage::r( const size_t i, const size_t j ) const
{
    return( m_data[ 3 * ( m_width * j + i ) ] );
}

/*==========================================================================*/
/**
 *  Returns the green component.
 *  @param index [in] pixel index
 *  @return green component
 */
/*==========================================================================*/
const kvs::UInt8 ColorImage::g( const size_t index ) const
{
    return( m_data[ 3 * index + 1 ] );
}

/*==========================================================================*/
/**
 *  Returns the green component.
 *  @param i [in] pixel index along the horizontal axis
 *  @param j [in] pixel index along the vertical axis
 *  @return green component
 */
/*==========================================================================*/
const kvs::UInt8 ColorImage::g( const size_t i, const size_t j ) const
{
    return( m_data[ 3 * ( m_width * j + i ) + 1 ] );
}

/*==========================================================================*/
/**
 *  Returns the blue component.
 *  @param index [in] pixel index
 *  @return blue component
 */
/*==========================================================================*/
const kvs::UInt8 ColorImage::b( const size_t index ) const
{
    return( m_data[ 3 * index + 2 ] );
}

/*==========================================================================*/
/**
 *  Returns the blue component.
 *  @param i [in] pixel index along the horizontal axis
 *  @param j [in] pixel index along the vertical axis
 *  @return blue component
 */
/*==========================================================================*/
const kvs::UInt8 ColorImage::b( const size_t i, const size_t j ) const
{
    return( m_data[ 3 * ( m_width * j + i ) + 2 ] );
}

/*==========================================================================*/
/**
 *  Returns the pixel color.
 *  @param index [in] pixel index
 *  @return pixel color
 */
/*==========================================================================*/
const kvs::RGBColor ColorImage::pixel( const size_t index ) const
{
    const size_t index3 = index * 3;
    return( kvs::RGBColor( m_data[index3], m_data[index3+1], m_data[index3+2] ) );
}

/*==========================================================================*/
/**
 *  Returns the pixel color.
 *  @param i [in] pixel index along the horizontal axis
 *  @param j [in] pixel index along the vertical axis
 *  @return pixel color
 */
/*==========================================================================*/
const kvs::RGBColor ColorImage::pixel( const size_t i, const size_t j ) const
{
    const size_t index3 = ( m_width * j + i ) * 3;
    return( kvs::RGBColor( m_data[index3], m_data[index3+1], m_data[index3+2] ) );
}

/*==========================================================================*/
/**
 *  Set the pixel color.
 *  @param index [in] pixel index
 *  @param pixel [in] pixel color
 */
/*==========================================================================*/
void ColorImage::set( const size_t index, const kvs::RGBColor& pixel )
{
    const size_t index3 = index * 3;
    m_data[ index3 + 0 ] = pixel.r();
    m_data[ index3 + 1 ] = pixel.g();
    m_data[ index3 + 2 ] = pixel.b();
}

/*==========================================================================*/
/**
 *  Set the pixel color.
 *  @param i [in] pixel index along the horizontal axis
 *  @param j [in] pixel index along the vertical axis
 *  @param pixel [in] pixel color
 */
/*==========================================================================*/
void ColorImage::set( const size_t i, const size_t j, const kvs::RGBColor& pixel )
{
    const size_t index3 = ( m_width * j + i ) * 3;
    m_data[ index3 + 0 ] = pixel.r();
    m_data[ index3 + 1 ] = pixel.g();
    m_data[ index3 + 2 ] = pixel.b();
}

/*===========================================================================*/
/**
 *  @brief  Scales the image data.
 *  @param  ratio [in] scaling ratio
 */
/*===========================================================================*/
void ColorImage::scale( const double ratio )
{
    const size_t width = static_cast<size_t>( this->width() * ratio );
    const size_t height = static_cast<size_t>( this->height() * ratio );
    BaseClass::resize<ColorImage,ColorImage::Bilinear>( width, height, this );
}

/*===========================================================================*/
/**
 *  @brief  Scales the image data.
 *  @param  ratio [in] scaling ratio
 *  @param  method [in] Interpolation method
 */
/*===========================================================================*/
template <typename InterpolationMethod>
void ColorImage::scale( const double ratio, InterpolationMethod method )
{
    kvs::IgnoreUnusedVariable( method );

    const size_t width = static_cast<size_t>( this->width() * ratio );
    const size_t height = static_cast<size_t>( this->height() * ratio );
    BaseClass::resize<ColorImage,InterpolationMethod>( width, height, this );
}

// Specialization.
template
void ColorImage::scale( const double ratio, ColorImage::NearestNeighbor method );

template
void ColorImage::scale( const double ratio, ColorImage::Bilinear method );

/*===========================================================================*/
/**
 *  @brief  Resizes the image data.
 *  @param  width  [in] resized width
 *  @param  height [in] resized height
 */
/*===========================================================================*/
void ColorImage::resize( const size_t width, const size_t height )
{
    BaseClass::resize<ColorImage,ColorImage::Bilinear>( width, height, this );
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
void ColorImage::resize( const size_t width, const size_t height, InterpolationMethod method )
{
    kvs::IgnoreUnusedVariable( method );

    BaseClass::resize<ColorImage,InterpolationMethod>( width, height, this );
}

// Specialization.
template
void ColorImage::resize( const size_t width, const size_t height, ColorImage::NearestNeighbor method );

template
void ColorImage::resize( const size_t width, const size_t height, ColorImage::Bilinear method );

/*==========================================================================*/
/**
 *  Read a image file.
 *  @param filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*==========================================================================*/
const bool ColorImage::read( const std::string& filename )
{
    // KVSML image.
    if ( kvs::KVSMLObjectImage::CheckFileExtension( filename ) )
    {
        const kvs::KVSMLObjectImage kvsml( filename );
        if ( kvsml.pixelType() == "color" )
        {
            const BaseClass::ImageType type = BaseClass::Color;
            return( BaseClass::create( kvsml.width(), kvsml.height(), type, kvsml.data() ) );
        }
        if ( kvsml.pixelType() == "gray" )
        {
            kvs::GrayImage image( kvsml.width(), kvsml.height(), kvsml.data() );
            return( this->read_image( image ) );
        }
    }

    // Bitmap image.
    if ( kvs::Bmp::CheckFileExtension( filename ) )
    {
        const kvs::Bmp bmp( filename );
        const BaseClass::ImageType type = BaseClass::Color;
        return( BaseClass::create( bmp.width(), bmp.height(), type, bmp.data() ) );
    }

    // PPM image.
    if ( kvs::Ppm::CheckFileExtension( filename ) )
    {
        const kvs::Ppm ppm( filename );
        const BaseClass::ImageType type = BaseClass::Color;
        return( BaseClass::create( ppm.width(), ppm.height(), type, ppm.data() ) );
    }

    // PGM image.
    if ( kvs::Pgm::CheckFileExtension( filename ) )
    {
        kvs::GrayImage image; image.read( filename );
        return( this->read_image( image ) );
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
            const BaseClass::ImageType type = BaseClass::Color;
            return( BaseClass::create( tiff.width(), tiff.height(), type, data ) );
        }
        if ( tiff.colorMode() == kvs::Tiff::Gray8 )
        {
            const kvs::UInt8* data = static_cast<kvs::UInt8*>(tiff.rawData().pointer());
            kvs::GrayImage image( tiff.width(), tiff.height(), data );
            return( this->read_image( image ) );
        }
        if ( tiff.colorMode() == kvs::Tiff::Gray16 )
        {
            kvsMessageError( "TIFF image (16bits gray-scale) is not supported." );
            return( false );
        }
    }

#ifndef NO_CLIENT
    // DICOM image.
    if ( kvs::Dicom::CheckFileExtension( filename ) )
    {
        const kvs::Dicom dcm( filename );
        kvs::GrayImage image( dcm.column(), dcm.row(), dcm.pixelData() );
        return( this->read_image( image ) );
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
const bool ColorImage::write( const std::string& filename )
{
    // KVSML image.
    if ( kvs::KVSMLObjectImage::CheckFileExtension( filename ) )
    {
        kvs::KVSMLObjectImage kvsml;
        kvsml.setWidth( m_width );
        kvsml.setHeight( m_height );
        kvsml.setPixelType( "color" );
        kvsml.setWritingDataType( kvs::KVSMLObjectImage::Ascii );
        kvsml.setData( m_data );
        return( kvsml.write( filename ) );
    }

    // Bitmap image.
    if ( kvs::Bmp::CheckFileExtension( filename ) )
    {
        kvs::Bmp bmp( m_width, m_height, m_data );
        return( bmp.write( filename ) );
    }

    // PPM image.
    if ( kvs::Ppm::CheckFileExtension( filename ) )
    {
        kvs::Ppm ppm( m_width, m_height, m_data );
        return( ppm.write( filename ) );
    }

    // PGM image.
    if ( kvs::Pgm::CheckFileExtension( filename ) )
    {
        kvs::GrayImage image( *this );
        return( image.write( filename ) );
    }

    // PBM image.
    if ( kvs::Pbm::CheckFileExtension( filename ) )
    {
        kvs::BitImage image( kvs::GrayImage( *this ) );
        return( image.write( filename ) );
    }

    kvsMessageError( "Write-method for %s is not implemented.",
                     filename.c_str() );

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Reads the gray image.
 *  @param  image [in] gray image
 *  @return true, if the reading process is done successfully.
 */
/*===========================================================================*/
const bool ColorImage::read_image( const kvs::GrayImage& image )
{
    if ( !BaseClass::create( image.width(), image.height(), kvs::ImageBase::Color ) )
    {
        return( false );
    }

    const kvs::UInt8* data = image.data().pointer();
    for ( size_t index = 0, index3 = 0; index < m_npixels; index++, index3 += 3 )
    {
        const kvs::UInt8 pixel = data[ index ];
        m_data[ index3 + 0 ] = pixel;
        m_data[ index3 + 1 ] = pixel;
        m_data[ index3 + 2 ] = pixel;
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Reads the bit image.
 *  @param  image [in] bit image
 *  @return true, if the reading process is done successfully.
 */
/*===========================================================================*/
const bool ColorImage::read_image( const kvs::BitImage& image )
{
    if ( !BaseClass::create( image.width(), image.height(), kvs::ImageBase::Color ) )
    {
        return( false );
    }

    const size_t width = image.width();
    const size_t height = image.height();
    size_t index3 = 0;
    for ( size_t j = 0; j < height; j++ )
    {
        for ( size_t i = 0; i < width; i++, index3 += 3 )
        {
            const kvs::UInt8 pixel = image.pixel( i, j ) ? 255 : 0;
            m_data[ index3 + 0 ] = pixel;
            m_data[ index3 + 1 ] = pixel;
            m_data[ index3 + 2 ] = pixel;
        }
    }

    return( true );
}

} // end of namespace kvs
