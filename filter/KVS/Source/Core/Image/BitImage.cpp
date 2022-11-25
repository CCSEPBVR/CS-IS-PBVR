/****************************************************************************/
/**
 *  @file BitImage.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: BitImage.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "BitImage.h"
#include "ColorImage.h"
#include "GrayImage.h"
#include <kvs/Matrix44>
#include <kvs/Math>
#include <kvs/Binary>
#include <kvs/File>
#include <kvs/KVSMLObjectImage>
#include <kvs/Bmp>
#include <kvs/Ppm>
#include <kvs/Pgm>
#include <kvs/Pbm>
#include <kvs/Tiff>
#ifndef NO_CLIENT
#include <kvs/Dicom>
#endif


namespace
{

const kvs::UInt8 SetBitMask[9] =
{
    kvsBinary8( 1000, 0000 ),
    kvsBinary8( 0100, 0000 ),
    kvsBinary8( 0010, 0000 ),
    kvsBinary8( 0001, 0000 ),
    kvsBinary8( 0000, 1000 ),
    kvsBinary8( 0000, 0100 ),
    kvsBinary8( 0000, 0010 ),
    kvsBinary8( 0000, 0001 ),
    kvsBinary8( 0000, 0000 )
};

const kvs::UInt8 ResetBitMask[9] =
{
    kvsBinary8( 0111, 1111 ),
    kvsBinary8( 1011, 1111 ),
    kvsBinary8( 1101, 1111 ),
    kvsBinary8( 1110, 1111 ),
    kvsBinary8( 1111, 0111 ),
    kvsBinary8( 1111, 1011 ),
    kvsBinary8( 1111, 1101 ),
    kvsBinary8( 1111, 1110 ),
    kvsBinary8( 1111, 1111 )
};

/*===========================================================================*/
/**
 *  @brief  Binarization by thresholding.
 */
/*===========================================================================*/
inline void Thresholding(
    const kvs::UInt8 threshold,
    const kvs::GrayImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const size_t width = image.width();
    const size_t height = image.height();
    const size_t bpl = ( width + 7 ) >> 3;

    for ( size_t j = 0; j < height; j++ )
    {
        for ( size_t i = 0; i < width; i++ )
        {
            const kvs::UInt8 value = image.pixel( i, j );
            if ( value < threshold )
            {
                data[ j * bpl + ( i >> 3 ) ] &= ResetBitMask[ i & 7 ];
            }
            else
            {
                data[ j * bpl + ( i >> 3 ) ] |= SetBitMask[ i & 7 ];
            }
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Binarization by dithering.
 */
/*===========================================================================*/
inline void Dithering(
    const kvs::Matrix44d& mask,
    const kvs::GrayImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const size_t width = image.width();
    const size_t height = image.height();
    const size_t bpl = ( width + 7 ) >> 3;
    const double r = 1.0 / 15.0;
    const kvs::Matrix44d dmask = ( mask - kvs::Matrix44d( 8.0 ) ) * r;

    for ( size_t j = 0; j < width; j++ )
    {
        for ( size_t i = 0; i < height; i++ )
        {
            const double p = image.pixel( i, j ) / 255.0;
            if ( kvs::Math::Floor( p + dmask[i%4][j%4] + 0.5 ) == 1 )
            {
                data[ j * bpl + ( i >> 3 ) ] &= ResetBitMask[ i & 7 ];
            }
            else
            {
                data[ j * bpl + ( i >> 3 ) ] |= SetBitMask[ i & 7 ];
            }
        }
    }
}

/*===========================================================================*/
/**
 *  Returns histogram information of the gray-scale image.
 *  @param  image [in] gray-scale iamge
 *  @return histogram information
 */
/*===========================================================================*/
inline kvs::ValueArray<kvs::UInt32> Histogram( const kvs::GrayImage& image )
{
    const size_t npixels = image.npixels();
    const kvs::UInt8* data = image.data().pointer();

    kvs::ValueArray<kvs::UInt32> count( 256 );
    count.fill( 0x00 );
    for ( size_t index = 0; index < npixels; index++ )
    {
        const size_t value = data[index];
        count[ value ] += 1;
    }

    return( count );
}

} // end of namespace


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Binarize by using P-tile method.
 *  @param  image [in] grey-scale image
 *  @param  data [out] pixel data array (bit array)
 */
/*===========================================================================*/
void BitImage::PTile::operator () (
    const kvs::GrayImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const size_t width = image.width();
    const size_t height= image.height();
    const double ratio = 1.0 / static_cast<double>( width * height );
    const kvs::ValueArray<kvs::UInt32> histogram = ::Histogram( image );

    // Create the cumulative frequency.
    kvs::ValueArray<double> cum( 256 );
    cum[0] = histogram[0];
    for ( size_t i = 1; i < 256; i++ ) cum[i] = cum[i-1] + static_cast<double>(histogram[i]);
    for ( size_t i = 0; i < 256; i++ ) cum[i] = cum[i] * ratio;

    const double p = 0.4;
    double diff = 100.0;
    double temp = 0.0;
    size_t threshold = 0;
    for( size_t i = 0; i < 256; i++ )
    {
        temp = kvs::Math::Abs( p - cum[i] );
        if( temp < diff )
        {
            diff      = temp;
            threshold = i;
        }
    }

    ::Thresholding( static_cast<kvs::UInt8>(threshold), image, data );
}

/*===========================================================================*/
/**
 *  @brief  Binarize by using Distinction method.
 *  @param  image [in] gray image
 *  @param  data [out] pixel data array (bit array)
 */
/*===========================================================================*/
void BitImage::Distinction::operator () (
    const kvs::GrayImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const size_t width = image.width();
    const size_t height= image.height();
    const double ratio = 1.0 / static_cast<double>( width * height );
    const kvs::ValueArray<kvs::UInt32> histogram = ::Histogram( image );

    // Create the probability distribution.
    kvs::ValueArray<double> p( 256 );
    for ( size_t i = 0; i < 256; i++ ) p[i] = static_cast<double>(histogram[i]) * ratio;

    // Calculate the sum of the probability distribution for each class.
    kvs::ValueArray<double> w1( 256 ); w1.fill( 0x00 );
    kvs::ValueArray<double> w2( 256 ); w2.fill( 0x00 );
    for ( size_t i = 0; i < 256; i++ )
    {
        for ( size_t j = 0; j <   i; j++ ) w1[i] += p[j];
        for ( size_t k = i; k < 256; k++ ) w2[i] += p[k];
    }
    // Calculate the mean for each class.
    kvs::ValueArray<double> m1( 256 ); m1.fill( 0x00 );
    kvs::ValueArray<double> m2( 256 ); m2.fill( 0x00 );
    for ( size_t i = 0; i < 256; i++ )
    {
        for ( size_t j = 0; j <   i; j++ ) m1[i] += static_cast<double>(j) * p[j] / w1[i];
        for ( size_t k = i; k < 256; k++ ) m2[i] += static_cast<double>(k) * p[k] / w2[i];
    }

    // Calculate the variance for each class.
    kvs::ValueArray<double> s2_1( 256 ); s2_1.fill( 0x00 );
    kvs::ValueArray<double> s2_2( 256 ); s2_2.fill( 0x00 );
    for ( size_t i = 0; i < 256; i++ )
    {
        const double v = static_cast<double>(i);
        for ( size_t j = 0; j < i; j++ )
        {
            s2_1[i] += p[j] * ( v - m1[i] ) * ( v - m1[i] ) / w1[i];
        }
        for ( size_t k = 0; k < 256; k++ )
        {
            s2_2[i] += p[k] * ( v - m2[i] ) * ( v - m2[i] ) / w2[i];
        }
    }

    // Calculate the variance between the classes and the variance within
    // the classes. And then, calculate the threshold value.
    kvs::ValueArray<double> s2_b( 256 ); s2_b.fill( 0x00 );
    kvs::ValueArray<double> s2_w( 256 ); s2_w.fill( 0x00 );
    double max_ratio = 0.0;
    size_t threshold = 0;
    for ( size_t i = 0; i < 256; i++ )
    {
        // The variance between the classes.
        s2_b[i] = w1[i] * w2[i] * ( m1[i] - m2[i] ) * ( m1[i] - m2[i] );
        // The variance within the classes.
        s2_w[i] = w1[i] * s2_1[i] + w2[i] * s2_2[i];

        const double ratio = s2_b[i] / s2_w[i];
        if ( ratio > max_ratio )
        {
            max_ratio = ratio;
            threshold = i;
        }
    }

    ::Thresholding( static_cast<kvs::UInt8>(threshold), image, data );
}

/*===========================================================================*/
/**
 *  @brief  Binarize by using Byer method.
 *  @param  image [in] gray image
 *  @param  data [out] pixel data array (bit array)
 */
/*===========================================================================*/
void BitImage::Byer::operator () (
    const kvs::GrayImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const kvs::Matrix44d mask(
        0.0,  8.0,  2.0,  10.0,
        12.0, 4.0,  14.0, 6.0,
        3.0,  11.0, 1.0,  9.0,
        15.0, 7.0,  13.0, 5.0 );

    ::Dithering( mask, image, data );
}

/*===========================================================================*/
/**
 *  @brief  Binarize by using Halftone method.
 *  @param  image [in] gray image
 *  @param  data [out] pixel data array (bit array)
 */
/*===========================================================================*/
void BitImage::Halftone::operator () (
    const kvs::GrayImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const kvs::Matrix44d mask(
        10.0, 4.0,  6.0,  8.0,
        12.0, 0.0,  2.0,  14.0,
        7.0,  9.0,  11.0, 5.0,
        3.0,  15.0, 13.0, 1.0 );

    ::Dithering( mask, image, data );
}

/*===========================================================================*/
/**
 *  @brief  Binarize by using Emphasized halftone method.
 *  @param  image [in] gray image
 *  @param  data [out] pixel data array (bit array)
 */
/*===========================================================================*/
void BitImage::EmphasizedHalftone::operator () (
    const kvs::GrayImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const kvs::Matrix44d mask(
        12.0, 4.0, 8.0, 14.0,
        11.0, 0.0, 2.0, 6.0,
        7.0,  3.0, 1.0, 10.0,
        15.0, 9.0, 5.0, 13.0 );

    ::Dithering( mask, image, data );
}

/*===========================================================================*/
/**
 *  @brief  Binarize by using Screw method.
 *  @param  image [in] gray image
 *  @param  data [out] pixel data array (bit array)
 */
/*===========================================================================*/
void BitImage::Screw::operator () (
    const kvs::GrayImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const kvs::Matrix44d mask(
        13.0, 7.0,  6.0,  12.0,
        8.0,  1.0,  0.0,  5.0,
        9.0,  2.0,  3.0,  4.0,
        14.0, 10.0, 11.0, 15.0 );

    ::Dithering( mask, image, data );
}

/*===========================================================================*/
/**
 *  @brief  Binarize by using Deformed screw method.
 *  @param  image [in] gray image
 *  @param  data [out] pixel data array (bit array)
 */
/*===========================================================================*/
void BitImage::DeformedScrew::operator () (
    const kvs::GrayImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const kvs::Matrix44d mask(
        15.0, 4.0, 8.0, 12.0,
        11.0, 0.0, 1.0, 5.0,
        7.0,  3.0, 2.0, 9.0,
        14.0, 9.0, 6.0, 13.0 );

    ::Dithering( mask, image, data );
}

/*===========================================================================*/
/**
 *  @brief  Binarize by using Dot concentrate method.
 *  @param  image [in] gray image
 *  @param  data [out] pixel data array (bit array)
 */
/*===========================================================================*/
void BitImage::DotConcentrate::operator () (
    const kvs::GrayImage& image,
    kvs::ValueArray<kvs::UInt8>& data )
{
    const kvs::Matrix44d mask(
        13.0, 4.0, 8.0, 14.0,
        10.0, 0.0, 1.0, 7.0,
        6.0,  3.0, 2.0, 11.0,
        15.0, 9.0, 5.0, 13.0 );

    ::Dithering( mask, image, data );
}

/*==========================================================================*/
/**
 *  Constructs a new bit image.
 */
/*==========================================================================*/
BitImage::BitImage( void )
{
}

/*==========================================================================*/
/**
 *  Constructs a new bit image.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param bit [in] bit flag
 */
/*==========================================================================*/
BitImage::BitImage( const size_t width, const size_t height, const bool bit ):
    kvs::ImageBase( width, height, kvs::ImageBase::Bit )
{
    this->set( bit );
}

/*==========================================================================*/
/**
 *  Constructs a new bit image.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param data [in] pointer to pixel data
 */
/*==========================================================================*/
BitImage::BitImage(
    const size_t width,
    const size_t height,
    const kvs::UInt8* data ):
    kvs::ImageBase( width, height, kvs::ImageBase::Bit, data )
{
}

/*==========================================================================*/
/**
 *  Constructs a new bit image.
 *  @param width [in] image width
 *  @param height [in] image height
 *  @param data [in] pixel data array
 */
/*==========================================================================*/
BitImage::BitImage(
    const size_t width,
    const size_t height,
    const kvs::ValueArray<kvs::UInt8>& data ):
    kvs::ImageBase( width, height, kvs::ImageBase::Bit, data )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new BitImage class as copy
 *  @param  image [in] bit image
 */
/*===========================================================================*/
BitImage::BitImage( const kvs::BitImage& image )
{
    BaseClass::copy( image );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new bit image from the color image.
 *  @param  image [in] gray image
 */
/*===========================================================================*/
BitImage::BitImage( const kvs::GrayImage& image ):
    kvs::ImageBase( image.width(), image.height(), kvs::ImageBase::Bit )
{
    BitImage::PTile method;
    method( image, m_data );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new BitImage class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
BitImage::BitImage( const std::string& filename )
{
    this->read( filename );
}

/*==========================================================================*/
/**
 *  Destroys the bit image.
 */
/*==========================================================================*/
BitImage::~BitImage( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Copys other bit image.
 *  @param  image [in] bit image
 */
/*===========================================================================*/
kvs::BitImage& BitImage::operator = ( const kvs::BitImage& image )
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
const bool BitImage::pixel( const size_t index ) const
{
    const size_t i = index / m_width;
    const size_t j = index % m_width;
    return( this->pixel( i, j ) );
}

/*==========================================================================*/
/**
 *  Returns the pixel value.
 *  @param i [in] pixel index along the horizontal axis
 *  @param j [in] pixel index along the vertical axis
 *  @return pixel value
 */
/*==========================================================================*/
const bool BitImage::pixel( const size_t i, const size_t j ) const
{
    return( m_data[ j * m_bpl + ( i >> 3 ) ] & ::SetBitMask[ i & 7 ] ? true : false );
}

/*==========================================================================*/
/**
 *  Set the pixel value.
 *  @param index [in] pixel index
 *  @param pixel [in] pixel value
 */
/*==========================================================================*/
void BitImage::set( const size_t index, const bool pixel )
{
    const size_t i = index / m_width;
    const size_t j = index % m_width;
    if ( pixel ) this->set_bit( i, j );
    else this->reset_bit( i, j );
}

/*==========================================================================*/
/**
 *  Set the pixel value.
 *  @param i [in] pixel index along the horizontal axis
 *  @param j [in] pixel index along the vertical axis
 *  @param pixel [in] pixel value
 */
/*==========================================================================*/
void BitImage::set( const size_t i, const size_t j, const bool pixel )
{
    if ( pixel ) this->set_bit( i, j );
    else this->reset_bit( i, j );
}

/*===========================================================================*/
/**
 *  @brief  Flips the pixel value that is specified by the index.
 *  @param  index [in] pixel index
 */
/*===========================================================================*/
void BitImage::flip( const size_t index )
{
    const size_t i = index / m_width;
    const size_t j = index % m_width;
    this->flip( i, j );
}

/*===========================================================================*/
/**
 *  @brief  Flips the pixel value that is specified by the index i and j.
 *  @param  i [in] index along the vertical axis
 *  @param  j [in] index along the horizontal axis
 */
/*===========================================================================*/
void BitImage::flip( const size_t i, const size_t j )
{
    m_data[ j * m_bpl + ( i >> 3 ) ] ^= ::SetBitMask[ i & 7 ];
}

/*===========================================================================*/
/**
 *  @brief  Sets all pixels by the given bit (true/false).
 *  @param  bit [in] pixel bit value
 */
/*===========================================================================*/
void BitImage::set( const bool bit )
{
    const kvs::UInt8 mask = ( bit ) ? ( ::SetBitMask[8] ) : ( ::ResetBitMask[8] );

    for( size_t j = 0; j < m_height; j++ )
    {
        const size_t line_head = j * m_bpl;
        for( size_t i = 0; i < m_bpl; i++ )
        {
            m_data[ line_head + i ] = mask;
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Flips all pixels.
 */
/*===========================================================================*/
void BitImage::flip( void )
{
    for( size_t j = 0; j < m_height; j++ )
    {
        const size_t line_head = j * m_bpl;
        for( size_t i = 0; i < m_bpl; i++ )
        {
            m_data[ line_head + i ] = ~m_data[ line_head + i ];
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Counts the number of active pixels ('true' bits).
 *  @return number of active pixels
 */
/*===========================================================================*/
const size_t BitImage::count( void ) const
{
    size_t counter = 0;

    for( size_t j = 0; j < m_height; j++ )
    {
        const size_t line_head = j * m_bpl;
        for( size_t i = 0; i < m_bpl - 1; i++ )
        {
            const size_t data_head = line_head + i;
            for( size_t b = 0; b < 8; b++ )
            {
                if( ( m_data[data_head] & ::SetBitMask[b] ) == 255 ) counter++;
            }
        }

        // Decrement padding bit.
        const size_t data_head = line_head + ( m_bpl - 1 );
        for( size_t b = 0; b < 8 - m_padding; b++ )
        {
            if( ( m_data[data_head] & ::SetBitMask[b] ) == 255 ) counter++;
        }
    }

    return( counter );
}

/*==========================================================================*/
/**
 *  Read a image file.
 *  @param filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*==========================================================================*/
const bool BitImage::read( const std::string& filename )
{
    // Color or Gray image.
    if ( kvs::KVSMLObjectImage::CheckFileExtension( filename ) ||
         kvs::Bmp::CheckFileExtension( filename ) ||
         kvs::Ppm::CheckFileExtension( filename ) ||
         kvs::Pgm::CheckFileExtension( filename ) ||
#ifndef NO_CLIENT
         kvs::Tiff::CheckFileExtension( filename ) ||
         kvs::Dicom::CheckFileExtension( filename ) )
#else
         kvs::Tiff::CheckFileExtension( filename ) )
#endif
    {
        kvs::GrayImage image; image.read( filename );
        if ( !BaseClass::create( image.width(), image.height(), BaseClass::Bit ) )
        {
            return( false );
        }

        BitImage::PTile method;
        method( image, m_data );

        return( true );
    }

    // Bit image.
    if ( kvs::Pbm::CheckFileExtension( filename ) )
    {
        const kvs::Pbm pbm( filename );
        const kvs::UInt8* data = static_cast<const kvs::UInt8*>(pbm.data().pointer());
        return( BaseClass::create( pbm.width(), pbm.height(), BaseClass::Bit, data ) );
    }

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
const bool BitImage::write( const std::string& filename )
{
    const kvs::File file( filename );
    const std::string extension = file.extension();

    // Color image.
    if ( kvs::KVSMLObjectImage::CheckFileExtension( filename ) ||
         kvs::Bmp::CheckFileExtension( filename ) ||
         kvs::Ppm::CheckFileExtension( filename ) )
    {
        kvs::ColorImage image( *this );
        return( image.write( filename ) );
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
        const size_t nvalues = m_width * m_height;
        const kvs::UInt8* values = m_data.pointer();
        kvs::Pbm pbm( m_width, m_height, kvs::BitArray( values, nvalues) );
        return( pbm.write( filename ) );
    }

    kvsMessageError( "Write-method for %s is not implemented.",
                     filename.c_str() );

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Sets the pixel that is specified by the index i and j.
 *  @param  i [in] index along the vertical axis
 *  @param  j [in] index along the horizontal axis
 */
/*===========================================================================*/
void BitImage::set_bit( const size_t i, const size_t j )
{
    m_data[ j * m_bpl +( i >> 3 ) ] |= ::SetBitMask[ i & 7 ];
}

/*===========================================================================*/
/**
 *  @brief  Resets the pixel that is specified by the index i and j.
 *  @param  i [in] index along the vertical axis
 *  @param  j [in] index along the horizontal axis
 */
/*===========================================================================*/
void BitImage::reset_bit( const size_t i, const size_t j )
{
    m_data[ j * m_bpl + ( i >> 3 ) ] &= ::ResetBitMask[ i & 7 ];
}

} // end of namespace kvs
