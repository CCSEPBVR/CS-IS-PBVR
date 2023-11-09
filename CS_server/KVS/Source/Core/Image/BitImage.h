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
#ifndef KVS__BIT_IMAGE_H_INCLUDE
#define KVS__BIT_IMAGE_H_INCLUDE

#include <limits>
#include "ImageBase.h"
#include "GrayImage.h"
#include <kvs/ClassName>


namespace kvs
{

class RGBColor;
class ColorImage;

/*==========================================================================*/
/**
 *  Bit image class.
 */
/*==========================================================================*/
class BitImage : public kvs::ImageBase
{
    kvsClassName( kvs::BitImage );

public:

    typedef kvs::ImageBase BaseClass;
    typedef bool           PixelType;

public:

    // Binarization method.

    struct PTile
    {
        void operator () ( const kvs::GrayImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

    struct Distinction
    {
        void operator () ( const kvs::GrayImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

    struct Byer
    {
        void operator () ( const kvs::GrayImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

    struct Halftone
    {
        void operator () ( const kvs::GrayImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

    struct EmphasizedHalftone
    {
        void operator () ( const kvs::GrayImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

    struct Screw
    {
        void operator () ( const kvs::GrayImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

    struct DeformedScrew
    {
        void operator () ( const kvs::GrayImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

    struct DotConcentrate
    {
        void operator () ( const kvs::GrayImage& image, kvs::ValueArray<kvs::UInt8>& data );
    };

public:

    BitImage( void );

    BitImage( const size_t width, const size_t height, const bool bit = true );

    BitImage( const size_t width, const size_t height, const kvs::UInt8* data );

    BitImage( const size_t width, const size_t height, const kvs::ValueArray<kvs::UInt8>& data );

    BitImage( const kvs::BitImage& image );

    explicit BitImage( const kvs::GrayImage& image );

    template <typename BinarizationMethod>
    BitImage( const kvs::GrayImage& image, BinarizationMethod method );

    explicit BitImage( const std::string& filename );

    virtual ~BitImage( void );

public:

    kvs::BitImage& operator = ( const kvs::BitImage& image );

public:

    const bool pixel( const size_t index ) const;

    const bool pixel( const size_t i, const size_t j ) const;

public:

    void set( const size_t index, const bool pixel );

    void set( const size_t i, const size_t j, const bool pixel );

    void flip( const size_t index );

    void flip( const size_t i, const size_t j );

public:

    void set( const bool bit );

    void flip( void );

    const size_t count( void ) const;

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

protected:

    void set_bit( const size_t i, const size_t j );

    void reset_bit( const size_t i, const size_t j );
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new bit image from the color image.
 *  @param  image [in] gray image
 *  @param  method [in] binarization method
 */
/*===========================================================================*/
template <typename BinarizationMethod>
inline BitImage::BitImage( const kvs::GrayImage& image, BinarizationMethod method ):
    kvs::ImageBase( image.width(), image.height(), kvs::ImageBase::Bit )
{
    method( image, m_data );
}

} // end of namespace kvs

#endif // KVS__BIT_IMAGE_H_INCLUDE
