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
#ifndef VIS_MODULE__COLOR_IMAGE_H_INCLUDE
#define VIS_MODULE__COLOR_IMAGE_H_INCLUDE

#include <limits>
#include "ImageBase.h"
#include <vismodule/ClassName>


namespace vismodule
{

class RGBColor;
class GrayImage;
class BitImage;

/*==========================================================================*/
/**
 *  Color image class.
 */
/*==========================================================================*/
class ColorImage : public vismodule::ImageBase
{
    visModuleClassName( vismodule::ColorImage );

public:

    typedef vismodule::ImageBase BaseClass;
    typedef vismodule::RGBColor  PixelType;

    typedef BaseClass::NearestNeighborInterpolatorColor NearestNeighbor;
    typedef BaseClass::BilinearInterpolatorColor        Bilinear;

public:

    ColorImage( void );

    ColorImage( const size_t width, const size_t height );

    ColorImage( const size_t width, const size_t height, const vismodule::UInt8* data );

    ColorImage( const size_t width, const size_t height, const vismodule::ValueArray<vismodule::UInt8>& data );

    ColorImage( const vismodule::ColorImage& image );

    explicit ColorImage( const vismodule::GrayImage& image );

    explicit ColorImage( const vismodule::BitImage& image );

    explicit ColorImage( const std::string& filename );

    virtual ~ColorImage( void );

public:

    vismodule::ColorImage& operator = ( const vismodule::ColorImage& image );

public:

    const vismodule::UInt8 r( const size_t index ) const;

    const vismodule::UInt8 r( const size_t i, const size_t j ) const;

    const vismodule::UInt8 g( const size_t index ) const;

    const vismodule::UInt8 g( const size_t i, const size_t j ) const;

    const vismodule::UInt8 b( const size_t index ) const;

    const vismodule::UInt8 b( const size_t i, const size_t j ) const;

    const vismodule::RGBColor pixel( const size_t index ) const;

    const vismodule::RGBColor pixel( const size_t i, const size_t j ) const;

public:

    void set( const size_t index, const vismodule::RGBColor& pixel );

    void set( const size_t i, const size_t j, const vismodule::RGBColor& pixel );

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

    const bool read_image( const vismodule::GrayImage& image );

    const bool read_image( const vismodule::BitImage& image );
};

} // end of namespace vismodule

#endif // VIS_MODULE__COLOR_IMAGE_H_INCLUDE
