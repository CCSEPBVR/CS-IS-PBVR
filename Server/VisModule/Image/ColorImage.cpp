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
#include <vismodule/IgnoreUnusedVariable>
#include <vismodule/KVSMLObjectImage>
#include <vismodule/RGBColor>
#include <vismodule/File>
#include <vismodule/Bmp>
#include <vismodule/Ppm>
#include <vismodule/Pgm>
#include <vismodule/Pbm>
#include <vismodule/Tiff>


namespace vismodule
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
ColorImage::ColorImage( const std::size_t width, const std::size_t height ):
    vismodule::ImageBase( width, height, vismodule::ImageBase::Color )
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
    const std::size_t width,
    const std::size_t height,
    const vismodule::UInt8* data ):
    vismodule::ImageBase( width, height, vismodule::ImageBase::Color, data )
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
    const std::size_t width,
    const std::size_t height,
    const vismodule::ValueArray<vismodule::UInt8>& data ):
    vismodule::ImageBase( width, height, vismodule::ImageBase::Color, data )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a copy of other color image.
 *  @param  image [in] color image
 */
/*===========================================================================*/
ColorImage::ColorImage( const vismodule::ColorImage& image )
{
    BaseClass::copy( image );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a copy of other gray image.
 *  @param  image [in] gray image
 */
/*===========================================================================*/
ColorImage::ColorImage( const vismodule::GrayImage& image )
{
    this->read_image( image );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a copy of other bit image.
 *  @param  image [in] bit image
 */
/*===========================================================================*/
ColorImage::ColorImage( const vismodule::BitImage& image )
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
vismodule::ColorImage& ColorImage::operator = ( const vismodule::ColorImage& image )
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
const vismodule::UInt8 ColorImage::r( const std::size_t index ) const
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
const vismodule::UInt8 ColorImage::r( const std::size_t i, const std::size_t j ) const
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
const vismodule::UInt8 ColorImage::g( const std::size_t index ) const
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
const vismodule::UInt8 ColorImage::g( const std::size_t i, const std::size_t j ) const
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
const vismodule::UInt8 ColorImage::b( const std::size_t index ) const
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
const vismodule::UInt8 ColorImage::b( const std::size_t i, const std::size_t j ) const
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
const vismodule::RGBColor ColorImage::pixel( const std::size_t index ) const
{
    const std::size_t index3 = index * 3;
    return( vismodule::RGBColor( m_data[index3], m_data[index3+1], m_data[index3+2] ) );
}

/*==========================================================================*/
/**
 *  Returns the pixel color.
 *  @param i [in] pixel index along the horizontal axis
 *  @param j [in] pixel index along the vertical axis
 *  @return pixel color
 */
/*==========================================================================*/
const vismodule::RGBColor ColorImage::pixel( const std::size_t i, const std::size_t j ) const
{
    const std::size_t index3 = ( m_width * j + i ) * 3;
    return( vismodule::RGBColor( m_data[index3], m_data[index3+1], m_data[index3+2] ) );
}

/*==========================================================================*/
/**
 *  Set the pixel color.
 *  @param index [in] pixel index
 *  @param pixel [in] pixel color
 */
/*==========================================================================*/
void ColorImage::set( const std::size_t index, const vismodule::RGBColor& pixel )
{
    const std::size_t index3 = index * 3;
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
void ColorImage::set( const std::size_t i, const std::size_t j, const vismodule::RGBColor& pixel )
{
    const std::size_t index3 = ( m_width * j + i ) * 3;
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
    const std::size_t width = static_cast<size_t>( this->width() * ratio );
    const std::size_t height = static_cast<size_t>( this->height() * ratio );
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
    vismodule::IgnoreUnusedVariable( method );

    const std::size_t width = static_cast<size_t>( this->width() * ratio );
    const std::size_t height = static_cast<size_t>( this->height() * ratio );
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
void ColorImage::resize( const std::size_t width, const std::size_t height )
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
void ColorImage::resize( const std::size_t width, const std::size_t height, InterpolationMethod method )
{
    vismodule::IgnoreUnusedVariable( method );

    BaseClass::resize<ColorImage,InterpolationMethod>( width, height, this );
}

// Specialization.
template
void ColorImage::resize( const std::size_t width, const std::size_t height, ColorImage::NearestNeighbor method );

template
void ColorImage::resize( const std::size_t width, const std::size_t height, ColorImage::Bilinear method );

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
    if ( vismodule::KVSMLObjectImage::CheckFileExtension( filename ) )
    {
        const vismodule::KVSMLObjectImage kvsml( filename );
        if ( kvsml.pixelType() == "color" )
        {
            const BaseClass::ImageType type = BaseClass::Color;
            return( BaseClass::create( kvsml.width(), kvsml.height(), type, kvsml.data() ) );
        }
        if ( kvsml.pixelType() == "gray" )
        {
            vismodule::GrayImage image( kvsml.width(), kvsml.height(), kvsml.data() );
            return( this->read_image( image ) );
        }
    }

    // Bitmap image.
    if ( vismodule::Bmp::CheckFileExtension( filename ) )
    {
        const vismodule::Bmp bmp( filename );
        const BaseClass::ImageType type = BaseClass::Color;
        return( BaseClass::create( bmp.width(), bmp.height(), type, bmp.data() ) );
    }

    // PPM image.
    if ( vismodule::Ppm::CheckFileExtension( filename ) )
    {
        const vismodule::Ppm ppm( filename );
        const BaseClass::ImageType type = BaseClass::Color;
        return( BaseClass::create( ppm.width(), ppm.height(), type, ppm.data() ) );
    }

    // PGM image.
    if ( vismodule::Pgm::CheckFileExtension( filename ) )
    {
        vismodule::GrayImage image; image.read( filename );
        return( this->read_image( image ) );
    }

    // PBM image.
    if ( vismodule::Pbm::CheckFileExtension( filename ) )
    {
        vismodule::BitImage image; image.read( filename );
        return( this->read_image( image ) );
    }

    // TIFF image.
    if ( vismodule::Tiff::CheckFileExtension( filename ) )
    {
        const vismodule::Tiff tiff( filename );
        if ( tiff.colorMode() == vismodule::Tiff::Color24 )
        {
            const vismodule::UInt8* data = static_cast<vismodule::UInt8*>(tiff.rawData().pointer());
            const BaseClass::ImageType type = BaseClass::Color;
            return( BaseClass::create( tiff.width(), tiff.height(), type, data ) );
        }
        if ( tiff.colorMode() == vismodule::Tiff::Gray8 )
        {
            const vismodule::UInt8* data = static_cast<vismodule::UInt8*>(tiff.rawData().pointer());
            vismodule::GrayImage image( tiff.width(), tiff.height(), data );
            return( this->read_image( image ) );
        }
        if ( tiff.colorMode() == vismodule::Tiff::Gray16 )
        {
            visModuleMessageError( "TIFF image (16bits gray-scale) is not supported." );
            return( false );
        }
    }

    visModuleMessageError( "Read-method for %s is not implemented.",
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
    if ( vismodule::KVSMLObjectImage::CheckFileExtension( filename ) )
    {
        vismodule::KVSMLObjectImage kvsml;
        kvsml.setWidth( m_width );
        kvsml.setHeight( m_height );
        kvsml.setPixelType( "color" );
        kvsml.setWritingDataType( vismodule::KVSMLObjectImage::Ascii );
        kvsml.setData( m_data );
        return( kvsml.write( filename ) );
    }

    // Bitmap image.
    if ( vismodule::Bmp::CheckFileExtension( filename ) )
    {
        vismodule::Bmp bmp( m_width, m_height, m_data );
        return( bmp.write( filename ) );
    }

    // PPM image.
    if ( vismodule::Ppm::CheckFileExtension( filename ) )
    {
        vismodule::Ppm ppm( m_width, m_height, m_data );
        return( ppm.write( filename ) );
    }

    // PGM image.
    if ( vismodule::Pgm::CheckFileExtension( filename ) )
    {
        vismodule::GrayImage image( *this );
        return( image.write( filename ) );
    }

    // PBM image.
    if ( vismodule::Pbm::CheckFileExtension( filename ) )
    {
        vismodule::BitImage image( vismodule::GrayImage( *this ) );
        return( image.write( filename ) );
    }

    visModuleMessageError( "Write-method for %s is not implemented.",
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
const bool ColorImage::read_image( const vismodule::GrayImage& image )
{
    if ( !BaseClass::create( image.width(), image.height(), vismodule::ImageBase::Color ) )
    {
        return( false );
    }

    const vismodule::UInt8* data = image.data().pointer();
    for ( std::size_t index = 0, index3 = 0; index < m_npixels; index++, index3 += 3 )
    {
        const vismodule::UInt8 pixel = data[ index ];
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
const bool ColorImage::read_image( const vismodule::BitImage& image )
{
    if ( !BaseClass::create( image.width(), image.height(), vismodule::ImageBase::Color ) )
    {
        return( false );
    }

    const std::size_t width = image.width();
    const std::size_t height = image.height();
    std::size_t index3 = 0;
    for ( std::size_t j = 0; j < height; j++ )
    {
        for ( std::size_t i = 0; i < width; i++, index3 += 3 )
        {
            const vismodule::UInt8 pixel = image.pixel( i, j ) ? 255 : 0;
            m_data[ index3 + 0 ] = pixel;
            m_data[ index3 + 1 ] = pixel;
            m_data[ index3 + 2 ] = pixel;
        }
    }

    return( true );
}

} // end of namespace vismodule
