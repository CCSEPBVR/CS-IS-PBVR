/*****************************************************************************/
/**
 *  @file   GlyphObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GlyphObject.cpp 847 2011-06-21 07:21:11Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "GlyphObject.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new GlyphObject class.
 */
/*===========================================================================*/
GlyphObject::GlyphObject( void ):
    m_scale( 1.0f, 1.0f, 1.0f )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the GlyphObject class.
 */
/*===========================================================================*/
GlyphObject::~GlyphObject( void )
{
    m_coords.deallocate();
    m_sizes.deallocate();
    m_directions.deallocate();
    m_colors.deallocate();
    m_opacities.deallocate();
}

const vismodule::ObjectBase::ObjectType GlyphObject::objectType( void ) const
{
    return( vismodule::ObjectBase::Glyph );
}

/*===========================================================================*/
/**
 *  @brief  Returns a coordinate value array.
 *  @return coordinate value array
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& GlyphObject::coords( void ) const
{
    return( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns a size value array.
 *  @return size value array
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& GlyphObject::sizes( void ) const
{
    return( m_sizes );
}

/*===========================================================================*/
/**
 *  @brief  Returns a direction vector array.
 *  @return direction vector array
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& GlyphObject::directions( void ) const
{
    return( m_directions );
}

/*===========================================================================*/
/**
 *  @brief  Returns a color value array.
 *  @return color value array
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::UInt8>& GlyphObject::colors( void ) const
{
    return( m_colors );
}

/*===========================================================================*/
/**
 *  @brief  Returns a opacity value array.
 *  @return opacity value array
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::UInt8>& GlyphObject::opacities( void ) const
{
    return( m_opacities );
}

/*===========================================================================*/
/**
 *  @brief  Returns a scaling vector.
 *  @return scaling vector
 */
/*===========================================================================*/
const vismodule::Vector3f& GlyphObject::scale( void ) const
{
    return( m_scale );
}

/*===========================================================================*/
/**
 *  @brief  Sets a coordinate value array.
 *  @param  coords [in] coordinate value array
 */
/*===========================================================================*/
void GlyphObject::setCoords( const vismodule::ValueArray<vismodule::Real32>& coords )
{
    m_coords = coords;
}

/*===========================================================================*/
/**
 *  @brief  Sets a size value array.
 *  @param  sizes [in] size value array
 */
/*===========================================================================*/
void GlyphObject::setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes )
{
    m_sizes = sizes;
}

/*===========================================================================*/
/**
 *  @brief  Sets a direction vector array.
 *  @param  directions [in] direction vector array
 */
/*===========================================================================*/
void GlyphObject::setDirections( const vismodule::ValueArray<vismodule::Real32>& directions )
{
    m_directions = directions;
}

/*===========================================================================*/
/**
 *  @brief  Sets a color value array.
 *  @param  colors [in] color value array
 */
/*===========================================================================*/
void GlyphObject::setColors( const vismodule::ValueArray<vismodule::UInt8>& colors )
{
    m_colors = colors;
}

/*===========================================================================*/
/**
 *  @brief  Sets a opacity value array.
 *  @param  opacities [in] opacity value array
 */
/*===========================================================================*/
void GlyphObject::setOpacities( const vismodule::ValueArray<vismodule::UInt8>& opacities )
{
    m_opacities = opacities;
}

/*===========================================================================*/
/**
 *  @brief  Sets a scaling factor.
 *  @param  scale [in] scaling factor
 */
/*===========================================================================*/
void GlyphObject::setScale( const vismodule::Real32 scale )
{
    m_scale = vismodule::Vector3f( scale, scale, scale );
}

/*===========================================================================*/
/**
 *  @brief  Sets a scaling vector.
 *  @param  scale [in] scaling vector
 */
/*===========================================================================*/
void GlyphObject::setScale( const vismodule::Vector3f& scale )
{
    m_scale = scale;
}

} // end of namespace vismodule
