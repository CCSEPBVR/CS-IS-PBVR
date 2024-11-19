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
#ifndef VIS_MODULE__GRAY_IMAGE_H_INCLUDE
#define VIS_MODULE__GRAY_IMAGE_H_INCLUDE

#include <limits>
#include "ImageBase.h"
#include "ColorImage.h"
#include <vismodule/ClassName>


namespace vismodule
{

class RGBColor;
class BitImage;

/*==========================================================================*/
/**
 *  Gray image class.
 */
/*==========================================================================*/
class GrayImage : public vismodule::ImageBase
{
    visModuleClassName( vismodule::GrayImage );

public:

    typedef vismodule::ImageBase BaseClass;
    typedef vismodule::UInt8     PixelType;

    typedef BaseClass::NearestNeighborInterpolatorGray NearestNeighbor;
    typedef BaseClass::BilinearInterpolatorGray        Bilinear;

public:

    // Gray-scaling method.

    struct MeanValue
    {
        void operator () ( const vismodule::ColorImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

    struct MiddleValue
    {
        void operator () ( const vismodule::ColorImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

    struct MedianValue
    {
        void operator () ( const vismodule::ColorImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

    struct NTSCWeightedMeanValue
    {
        void operator () ( const vismodule::ColorImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

    struct HDTVWeightedMeanValue
    {
        void operator () ( const vismodule::ColorImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

public:

    GrayImage( void );

    GrayImage( const size_t width, const size_t height );

    GrayImage( const size_t width, const size_t height, const vismodule::UInt8* data );

    GrayImage( const size_t width, const size_t height, const vismodule::ValueArray<vismodule::UInt8>& data );

    GrayImage( const vismodule::GrayImage& image );

    explicit GrayImage( const vismodule::BitImage& image );

    explicit GrayImage( const vismodule::ColorImage& image );

    template <typename GrayScalingMethod>
    GrayImage( const vismodule::ColorImage& image, GrayScalingMethod method );

    explicit GrayImage( const std::string& filename );

    virtual ~GrayImage( void );

public:

    vismodule::GrayImage& operator = ( const vismodule::GrayImage& image );

public:

    const vismodule::UInt8 pixel( const size_t index ) const;

    const vismodule::UInt8 pixel( const size_t i, const size_t j ) const;

public:

    void set( const size_t index, const vismodule::UInt8 pixel );

    void set( const size_t i, const size_t j, const vismodule::UInt8 pixel );

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

    const bool read_image( const vismodule::ColorImage& image );

    const bool read_image( const vismodule::BitImage& image );
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new gray-scale image from the color image.
 *  @param  image [in] color image
 *  @param  method [in] gray scaling method
 */
/*===========================================================================*/
template <typename GrayScalingMethod>
inline GrayImage::GrayImage( const vismodule::ColorImage& image, GrayScalingMethod method ):
    vismodule::ImageBase( image.width(), image.height(), vismodule::ImageBase::Gray )
{
    method( image, m_data );
}

} // end of namespace vismodule

#endif // VIS_MODULE__GRAY_IMAGE_H_INCLUDE
