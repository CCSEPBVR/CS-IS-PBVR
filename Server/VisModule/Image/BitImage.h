/****************************************************************************/
/**
 *  @file BitImage.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: BitImage.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__BIT_IMAGE_H_INCLUDE
#define VIS_MODULE__BIT_IMAGE_H_INCLUDE

#include <limits>
#include "ImageBase.h"
#include "GrayImage.h"
#include <vismodule/ClassName>


namespace vismodule
{

class RGBColor;
class ColorImage;

/*==========================================================================*/
/**
 *  Bit image class.
 */
/*==========================================================================*/
class BitImage : public vismodule::ImageBase
{
    visModuleClassName( vismodule::BitImage );

public:

    typedef vismodule::ImageBase BaseClass;
    typedef bool           PixelType;

public:

    // Binarization method.

    struct PTile
    {
        void operator () ( const vismodule::GrayImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

    struct Distinction
    {
        void operator () ( const vismodule::GrayImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

    struct Byer
    {
        void operator () ( const vismodule::GrayImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

    struct Halftone
    {
        void operator () ( const vismodule::GrayImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

    struct EmphasizedHalftone
    {
        void operator () ( const vismodule::GrayImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

    struct Screw
    {
        void operator () ( const vismodule::GrayImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

    struct DeformedScrew
    {
        void operator () ( const vismodule::GrayImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

    struct DotConcentrate
    {
        void operator () ( const vismodule::GrayImage& image, vismodule::ValueArray<vismodule::UInt8>& data );
    };

public:

    BitImage( void );

    BitImage( const std::size_t width, const std::size_t height, const bool bit = true );

    BitImage( const std::size_t width, const std::size_t height, const vismodule::UInt8* data );

    BitImage( const std::size_t width, const std::size_t height, const vismodule::ValueArray<vismodule::UInt8>& data );

    BitImage( const vismodule::BitImage& image );

    explicit BitImage( const vismodule::GrayImage& image );

    template <typename BinarizationMethod>
    BitImage( const vismodule::GrayImage& image, BinarizationMethod method );

    explicit BitImage( const std::string& filename );

    virtual ~BitImage( void );

public:

    vismodule::BitImage& operator = ( const vismodule::BitImage& image );

public:

    const bool pixel( const std::size_t index ) const;

    const bool pixel( const std::size_t i, const std::size_t j ) const;

public:

    void set( const std::size_t index, const bool pixel );

    void set( const std::size_t i, const std::size_t j, const bool pixel );

    void flip( const std::size_t index );

    void flip( const std::size_t i, const std::size_t j );

public:

    void set( const bool bit );

    void flip( void );

    const std::size_t count( void ) const;

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

protected:

    void set_bit( const std::size_t i, const std::size_t j );

    void reset_bit( const std::size_t i, const std::size_t j );
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new bit image from the color image.
 *  @param  image [in] gray image
 *  @param  method [in] binarization method
 */
/*===========================================================================*/
template <typename BinarizationMethod>
inline BitImage::BitImage( const vismodule::GrayImage& image, BinarizationMethod method ):
    vismodule::ImageBase( image.width(), image.height(), vismodule::ImageBase::Bit )
{
    method( image, m_data );
}

} // end of namespace vismodule

#endif // VIS_MODULE__BIT_IMAGE_H_INCLUDE
