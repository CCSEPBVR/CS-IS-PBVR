/****************************************************************************/
/**
 *  @file   ColorImage.h
 *  @author Naohisa Sakamoto
 */
/****************************************************************************/
#pragma once
#include <limits>
#include "ImageBase.h"


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
public:
    using BaseClass = kvs::ImageBase;
    using PixelType = kvs::RGBColor;

    // Interpolator
    using Interpolator = BaseClass::ColorInterpolator;
    static Interpolator Nearest() { return BaseClass::ColorNearest; }
    static Interpolator Bilinear() { return BaseClass::ColorBilinear; }

public:
    ColorImage() = default;
    ColorImage( const std::size_t width, const std::size_t height );
    ColorImage( const std::size_t width, const std::size_t height, const kvs::ValueArray<kvs::UInt8>& pixels );
    explicit ColorImage( const kvs::GrayImage& image );
    explicit ColorImage( const kvs::BitImage& image );
    explicit ColorImage( const std::string& filename );

    bool create( const std::size_t width, const std::size_t height );
    bool create( const std::size_t width, const std::size_t height, const kvs::ValueArray<kvs::UInt8>& pixels );

    kvs::RGBColor pixel( const std::size_t index ) const;
    kvs::RGBColor pixel( const std::size_t i, const std::size_t j ) const;

    void setPixel( const std::size_t index, const kvs::RGBColor& pixel );
    void setPixel( const std::size_t i, const std::size_t j, const kvs::RGBColor& pixel );
    void scale( const double ratio, Interpolator interpolator = Bilinear() );
    void resize( const std::size_t width, const std::size_t height, Interpolator interpolator = Bilinear() );
    bool read( const std::string& filename );
    bool write( const std::string& filename ) const;

private:
    bool read_image( const kvs::GrayImage& image );
    bool read_image( const kvs::BitImage& image );
};

} // end of namespace kvs
