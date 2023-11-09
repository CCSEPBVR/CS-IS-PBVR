/****************************************************************************/
/**
 *  @file Background.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Background.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__BACKGROUND_H_INCLUDE
#define KVS__BACKGROUND_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/RGBAColor>


namespace kvs
{

/*==========================================================================*/
/**
 *  Background class.
 */
/*==========================================================================*/
class Background
{
    kvsClassName( kvs::Background );

public:

    enum Type
    {
        MonoColor = 0,    ///< mono color background
        TwoSideColor,     ///< gradation color background using two colors
        FourCornersColor, ///< gradation color background using four colors
//        Image             ///< image background
    };

protected:

//    MipmapTexture2D m_mipmap;   ///< mipmap texture
    Type           m_type;     ///< background type
    kvs::RGBAColor m_color[4]; ///< color on the corners

public:

    Background( void );

    Background( const kvs::RGBAColor& color );

    Background( const kvs::RGBAColor& color1, const kvs::RGBAColor& color2 );

    Background( const kvs::RGBAColor& color0, const kvs::RGBAColor& color1,
                const kvs::RGBAColor& color2, const kvs::RGBAColor& color3 );

    virtual ~Background( void );

public:

    Background& operator = ( const Background& bg );

public:

    void setColor( const kvs::RGBAColor& color );

    void setColor( const kvs::RGBAColor& color0, const kvs::RGBAColor& color1 );

    void setColor( const kvs::RGBAColor& color0, const kvs::RGBAColor& color1,
                   const kvs::RGBAColor& color2, const kvs::RGBAColor& color3 );

    const kvs::RGBColor& color( size_t index = 0 ) const;

//    void setImage( const ImageObject& image );

public:

    void apply( void );

private:

    void apply_mono_color( void );

    void apply_gradation_color( void );

//    void apply_image( void );
};

} // end of namespace kvs

#endif // KVS__BACKGROUND_H_INCLUDE
