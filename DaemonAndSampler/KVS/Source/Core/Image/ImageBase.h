/****************************************************************************/
/**
 *  @file ImageBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImageBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__IMAGE_BASE_H_INCLUDE
#define KVS__IMAGE_BASE_H_INCLUDE

#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector2>
#include <kvs/ClassName>


namespace kvs
{

class ColorImage;
class GrayImage;

/*==========================================================================*/
/**
 *  Image base class.
 */
/*==========================================================================*/
class ImageBase
{
    kvsClassName( kvs::ImageBase );

public:

    // Image type (number of components)
    enum ImageType
    {
        Bit   = 0, ///< bit image
        Gray  = 1, ///< gray image
        Color = 3  ///< color image
    };

public:

    template <typename ImageDataType>
    class NearestNeighborInterpolator;

    template <typename ImageDataType>
    class BilinearInterpolator;

    typedef NearestNeighborInterpolator<kvs::GrayImage>  NearestNeighborInterpolatorGray;
    typedef NearestNeighborInterpolator<kvs::ColorImage> NearestNeighborInterpolatorColor;

    typedef BilinearInterpolator<kvs::GrayImage>  BilinearInterpolatorGray;
    typedef BilinearInterpolator<kvs::ColorImage> BilinearInterpolatorColor;

protected:

    size_t m_width;   ///< image width [pix]
    size_t m_height;  ///< image height [pix]
    size_t m_npixels; ///< number of pixels
    size_t m_padding; ///< padding bit for bit-image
    size_t m_bpp;     ///< bits per pixel [bit]
    size_t m_bpl;     ///< bytes per line [byte]
    size_t m_size;    ///< data size [byte]
    kvs::ValueArray<kvs::UInt8> m_data; ///< pixel data array

public:

    ImageBase( void );

    ImageBase( const size_t width, const size_t height, const ImageType type );

    ImageBase(
        const size_t width,
        const size_t height,
        const ImageType type,
        const kvs::UInt8* data );

    ImageBase(
        const size_t width,
        const size_t height,
        const ImageType type,
        const kvs::ValueArray<kvs::UInt8>& data );

    virtual ~ImageBase( void );

public:

    void clear( void );

    void copy( const kvs::ImageBase& image );

    const bool create( const size_t width, const size_t height, const ImageType type );

    const bool create(
        const size_t width,
        const size_t height,
        const ImageType type,
        const kvs::UInt8* data );

    const bool create(
        const size_t width,
        const size_t height,
        const ImageType type,
        const kvs::ValueArray<kvs::UInt8>& data );

public:

    const size_t width( void ) const;

    const size_t height( void ) const;

    const size_t bytesPerLine( void ) const;

    const size_t bitsPerPixel( void ) const;

    const size_t npixels( void ) const;

    const size_t padding( void ) const;

    const size_t size( void ) const;

    const kvs::ValueArray<kvs::UInt8>& data( void ) const;

    kvs::ValueArray<kvs::UInt8>& data( void );

public:

    void flip( void );

protected:

    template <typename ImageDataType, typename Interpolator>
    void resize( const size_t width, const size_t height, ImageDataType* image );
};

template <typename ImageDataType>
class ImageBase::NearestNeighborInterpolator
{
protected:

    const ImageDataType* m_reference_image;
    kvs::Vector2d m_p;

public:

    void attach( const ImageDataType* image );

    void setU( const double u );

    void setV( const double v );

    const typename ImageDataType::PixelType operator () ( void ) const;
};

template <typename ImageDataType>
class ImageBase::BilinearInterpolator
{
protected:

    const ImageDataType* m_reference_image;
    kvs::Vector2d m_pmin;
    kvs::Vector2d m_pmax;
    kvs::Vector2d m_rate;

public:

    void attach( const ImageDataType* image );

    void setU( const double u );

    void setV( const double v );

    const typename ImageDataType::PixelType operator () ( void ) const;
};

} // end of namespace kvs

#endif // KVS__IMAGE_BASE_H_INCLUDE
