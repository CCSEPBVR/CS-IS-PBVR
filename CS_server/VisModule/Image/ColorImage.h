/****************************************************************************/
/**
 *  @file ColorImage.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ColorImage.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__COLOR_IMAGE_H_INCLUDE
#define KVS__COLOR_IMAGE_H_INCLUDE

#include <limits>
#include "ImageBase.h"
#include <kvs/ClassName>


namespace kvs
{

class RGBColor;
class GrayImage;
class BitImage;

/*==========================================================================*/
/**
 *  Color image class.
 */
/*==========================================================================*/
class ColorImage : public kvs::ImageBase
{
    kvsClassName( kvs::ColorImage );

public:

    typedef kvs::ImageBase BaseClass;
    typedef kvs::RGBColor  PixelType;

    typedef BaseClass::NearestNeighborInterpolatorColor NearestNeighbor;
    typedef BaseClass::BilinearInterpolatorColor        Bilinear;

public:

    ColorImage( void );

    ColorImage( const size_t width, const size_t height );

    ColorImage( const size_t width, const size_t height, const kvs::UInt8* data );

    ColorImage( const size_t width, const size_t height, const kvs::ValueArray<kvs::UInt8>& data );

    ColorImage( const kvs::ColorImage& image );

    explicit ColorImage( const kvs::GrayImage& image );

    explicit ColorImage( const kvs::BitImage& image );

    explicit ColorImage( const std::string& filename );

    virtual ~ColorImage( void );

public:

    kvs::ColorImage& operator = ( const kvs::ColorImage& image );

public:

    const kvs::UInt8 r( const size_t index ) const;

    const kvs::UInt8 r( const size_t i, const size_t j ) const;

    const kvs::UInt8 g( const size_t index ) const;

    const kvs::UInt8 g( const size_t i, const size_t j ) const;

    const kvs::UInt8 b( const size_t index ) const;

    const kvs::UInt8 b( const size_t i, const size_t j ) const;

    const kvs::RGBColor pixel( const size_t index ) const;

    const kvs::RGBColor pixel( const size_t i, const size_t j ) const;

public:

    void set( const size_t index, const kvs::RGBColor& pixel );

    void set( const size_t i, const size_t j, const kvs::RGBColor& pixel );

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

    const bool read_image( const kvs::GrayImage& image );

    const bool read_image( const kvs::BitImage& image );
};

} // end of namespace kvs

#endif // KVS__COLOR_IMAGE_H_INCLUDE
