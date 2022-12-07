/****************************************************************************/
/**
 *  @file GrayImage.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GrayImage.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__GRAY_IMAGE_H_INCLUDE
#define KVS__GRAY_IMAGE_H_INCLUDE

#include <limits>
#include "ImageBase.h"
#include "ColorImage.h"
#include <kvs/ClassName>


namespace kvs
{

class RGBColor;
class BitImage;

/*==========================================================================*/
/**
 *  Gray image class.
 */
/*==========================================================================*/
class GrayImage : public kvs::ImageBase
{
    kvsClassName( kvs::GrayImage );

public:

    typedef kvs::ImageBase BaseClass;
    typedef kvs::UInt8     PixelType;

    typedef BaseClass::NearestNeighborInterpolatorGray NearestNeighbor;
    typedef BaseClass::BilinearInterpolatorGray        Bilinear;

public:

    // Gray-scaling method.

    struct MeanValue
    {
        void operator () ( const kvs::ColorImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

    struct MiddleValue
    {
        void operator () ( const kvs::ColorImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

    struct MedianValue
    {
        void operator () ( const kvs::ColorImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

    struct NTSCWeightedMeanValue
    {
        void operator () ( const kvs::ColorImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

    struct HDTVWeightedMeanValue
    {
        void operator () ( const kvs::ColorImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

public:

    GrayImage( void );

    GrayImage( const size_t width, const size_t height );

    GrayImage( const size_t width, const size_t height, const kvs::UInt8* data );

    GrayImage( const size_t width, const size_t height, const kvs::ValueArray<kvs::UInt8>& data );

    GrayImage( const kvs::GrayImage& image );

    explicit GrayImage( const kvs::BitImage& image );

    explicit GrayImage( const kvs::ColorImage& image );

    template <typename GrayScalingMethod>
    GrayImage( const kvs::ColorImage& image, GrayScalingMethod method );

    explicit GrayImage( const std::string& filename );

    virtual ~GrayImage( void );

public:

    kvs::GrayImage& operator = ( const kvs::GrayImage& image );

public:

    const kvs::UInt8 pixel( const size_t index ) const;

    const kvs::UInt8 pixel( const size_t i, const size_t j ) const;

public:

    void set( const size_t index, const kvs::UInt8 pixel );

    void set( const size_t i, const size_t j, const kvs::UInt8 pixel );

public:

    void scale( const double ratio );

    template <typename InterpolationMethod>
    void scale( const double ratio, InterpolationMethod method );

    void resize( const size_t width, const size_t height );

    template <typename InterpolationMethod>
    void resize( const size_t width, const size_t height, InterpolationMethod method );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

private:

    const bool read_image( const kvs::ColorImage& image );

    const bool read_image( const kvs::BitImage& image );
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new gray-scale image from the color image.
 *  @param  image [in] color image
 *  @param  method [in] gray scaling method
 */
/*===========================================================================*/
template <typename GrayScalingMethod>
inline GrayImage::GrayImage( const kvs::ColorImage& image, GrayScalingMethod method ):
    kvs::ImageBase( image.width(), image.height(), kvs::ImageBase::Gray )
{
    method( image, m_data );
}

} // end of namespace kvs

#endif // KVS__GRAY_IMAGE_H_INCLUDE
