/****************************************************************************/
/**
 *  @file RGBAColor.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RGBAColor.h 869 2011-07-25 05:11:49Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__RGBA_COLOR_H_INCLUDE
#define KVS__RGBA_COLOR_H_INCLUDE

#include "RGBColor.h"
#include <kvs/Type>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  RGB-a color class.
 */
/*==========================================================================*/
class RGBAColor : public kvs::RGBColor
{
    kvsClassName_without_virtual( kvs::RGBAColor );

protected:

    float m_opacity; ///< opacity [0-1]

public:

    RGBAColor( float opacity = 1.0f );

    RGBAColor( kvs::UInt8 red, kvs::UInt8 greeb, kvs::UInt8 blue, float opacity = 1.0f );

    RGBAColor( const kvs::UInt8 rgb[3], float opacity = 1.0f );

    RGBAColor( const kvs::RGBColor& rgb, float opacity = 1.0f );

public:

    RGBAColor& operator = ( const RGBAColor& rgba );

    RGBAColor& operator = ( const RGBColor& rgb );

public:

    const float a( void ) const;

    const float alpha( void ) const;

    const float opacity( void ) const;
};

} // end of namespace kvs

#endif // KVS__RGBA_COLOR_H_INCLUDE
