/*****************************************************************************/
/**
 *  @file   RGBFormulae.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#ifndef VIS_MODULE__RGB_FORMULAE_H_INCLUDE
#define VIS_MODULE__RGB_FORMULAE_H_INCLUDE

#include <vismodule/ColorMap>


namespace vismodule
{

namespace RGBFormulae
{

vismodule::ColorMap Create( const int index0, const int index1, const int index2, const std::size_t resolution );

/*===========================================================================*/
/**
 *  @brief  Returns PM3D colormap.
 *  @param  resolution [in] table resolution
 *  @return colormap
 */
/*===========================================================================*/
inline vismodule::ColorMap PM3D( const std::size_t resolution )
{
    return( Create( 7, 5, 15, resolution ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns GreenRedViolet colormap.
 *  @param  resolution [in] table resolution
 *  @return colormap
 */
/*===========================================================================*/
inline vismodule::ColorMap GreenRedViolet( const std::size_t resolution )
{
    return( Create( 3, 11, 6, resolution ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns Ocean colormap.
 *  @param  resolution [in] table resolution
 *  @return colormap
 */
/*===========================================================================*/
inline vismodule::ColorMap Ocean( const std::size_t resolution )
{
    return( Create( 23, 28, 3, resolution ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns Hot colormap.
 *  @param  resolution [in] table resolution
 *  @return colormap
 */
/*===========================================================================*/
inline vismodule::ColorMap Hot( const std::size_t resolution )
{
    return( Create( 21, 22, 23, resolution ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns ColorPrintable colormap.
 *  @param  resolution [in] table resolution
 *  @return colormap
 */
/*===========================================================================*/
inline vismodule::ColorMap ColorPrintable( const std::size_t resolution )
{
    return( Create( 30, 31, 32, resolution ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns Rainbow colormap.
 *  @param  resolution [in] table resolution
 *  @return colormap
 */
/*===========================================================================*/
inline vismodule::ColorMap Rainbow( const std::size_t resolution )
{
    return( Create( 33, 13, 10, resolution ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns AFMHot colormap.
 *  @param  resolution [in] table resolution
 *  @return colormap
 */
/*===========================================================================*/
inline vismodule::ColorMap AFMHot( const std::size_t resolution )
{
    return( Create( 34, 35, 36, resolution ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns Jet colormap.
 *  @param  resolution [in] table resolution
 *  @return colormap
 */
/*===========================================================================*/
inline vismodule::ColorMap Jet( const std::size_t resolution )
{
    return( Create( 37, 38, 39, resolution ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns Bone colormap.
 *  @param  resolution [in] table resolution
 *  @return colormap
 */
/*===========================================================================*/
inline vismodule::ColorMap Bone( const std::size_t resolution )
{
    return( Create( 40, 41, 42, resolution ) );
}

} // end of namespace RGBFormulae

} // end of namespace vismodule

#endif // VIS_MODULE__RGB_FORMULAE_H_INCLUDE
