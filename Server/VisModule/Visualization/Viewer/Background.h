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
#ifndef VIS_MODULE__BACKGROUND_H_INCLUDE
#define VIS_MODULE__BACKGROUND_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/RGBAColor>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Background class.
 */
/*==========================================================================*/
class Background
{
    visModuleClassName( vismodule::Background );

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
    vismodule::RGBAColor m_color[4]; ///< color on the corners

public:

    Background( void );

    Background( const vismodule::RGBAColor& color );

    Background( const vismodule::RGBAColor& color1, const vismodule::RGBAColor& color2 );

    Background( const vismodule::RGBAColor& color0, const vismodule::RGBAColor& color1,
                const vismodule::RGBAColor& color2, const vismodule::RGBAColor& color3 );

    virtual ~Background( void );

public:

    Background& operator = ( const Background& bg );

public:

    void setColor( const vismodule::RGBAColor& color );

    void setColor( const vismodule::RGBAColor& color0, const vismodule::RGBAColor& color1 );

    void setColor( const vismodule::RGBAColor& color0, const vismodule::RGBAColor& color1,
                   const vismodule::RGBAColor& color2, const vismodule::RGBAColor& color3 );

    const vismodule::RGBColor& color( std::size_t index = 0 ) const;

//    void setImage( const ImageObject& image );

public:

    void apply( void );

private:

    void apply_mono_color( void );

    void apply_gradation_color( void );

//    void apply_image( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__BACKGROUND_H_INCLUDE
