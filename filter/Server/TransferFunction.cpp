/****************************************************************************/
/**
 *  @file TransferFunction.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunction.cpp 611 2010-09-15 11:12:12Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "TransferFunction.h"

#include <kvs/Assert>
#include <kvs/KVSMLTransferFunction>


namespace
{
const size_t Defaultm_resolution = 256;
}

namespace pbvr
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new TransferFunction.
 *  @param  resolution [in] m_resolution.
 */
/*==========================================================================*/
TransferFunction::TransferFunction( const size_t resolution ):
    m_color_map( resolution ),
    m_opacity_map( resolution )
{
    m_color_map.create();
    m_opacity_map.create();
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new TransferFunction.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
/*
TransferFunction::TransferFunction( const std::string& filename )
{
    this->read( filename );
}
*/
/*==========================================================================*/
/**
 *  @brief  Constructs a new TransferFunction.
 *  @param  color_map [in] Color map.
 */
/*==========================================================================*/
TransferFunction::TransferFunction( const kvs::ColorMap& color_map ):
    m_color_map( color_map ),
    m_opacity_map( color_map.resolution() )
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new TransferFunction.
 *  @param  opacity_map [in] Opacity map.
 */
/*==========================================================================*/
TransferFunction::TransferFunction( const kvs::OpacityMap& opacity_map ):
    m_color_map( opacity_map.resolution() ),
    m_opacity_map( opacity_map )
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new TransferFunction.
 *  @param  color_map [in] Color map.
 *  @param  opacity_map [in] Opacity map.
 */
/*==========================================================================*/
TransferFunction::TransferFunction(
    const kvs::ColorMap&   color_map,
    const kvs::OpacityMap& opacity_map ):
    m_color_map( color_map ),
    m_opacity_map( opacity_map )
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a copy of other.
 *  @param  other [in] Transfer function.
 */
/*==========================================================================*/
TransferFunction::TransferFunction( const TransferFunction& other ):
    m_color_map( other.m_color_map ),
    m_opacity_map( other.m_opacity_map )
{
}

/*==========================================================================*/
/**
 *  @brief  Destroys TransferFunction.
 */
/*==========================================================================*/
TransferFunction::~TransferFunction()
{
}

/*==========================================================================*/
/**
 *  @brief  Set color map.
 *  @param  color_map [in] color map
 */
/*==========================================================================*/
void TransferFunction::setColorMap( const kvs::ColorMap& color_map )
{
    m_color_map = color_map;
}

/*==========================================================================*/
/**
 *  @brief  Set opacity map.
 *  @param  opacity_map [in] opacity map
 */
/*==========================================================================*/
void TransferFunction::setOpacityMap( const kvs::OpacityMap& opacity_map )
{
    m_opacity_map = opacity_map;
}

/*===========================================================================*/
/**
 *  @brief  Sets min and max values.
 *  @param  min_value [in] min. value
 *  @param  max_value [in] max. value
 */
/*===========================================================================*/
void TransferFunction::setRange( const float min_value, const float max_value )
{
    m_color_map.setRange( min_value, max_value );
    m_opacity_map.setRange( min_value, max_value );
}

void TransferFunction::setRange( const pbvr::VolumeObjectBase& volume )
{
    const float min_value = static_cast<float>( volume.minValue() );
    const float max_value = static_cast<float>( volume.maxValue() );
    this->setRange( min_value, max_value );
}

void TransferFunction::adjustRange( const float min_value, const float max_value )
{
    if ( this->hasRange() )
    {
        const float src_min_value = this->minValue();
        const float src_max_value = this->maxValue();

        // Adjust color map.
        {
            kvs::ColorMap::Points::const_iterator point = m_color_map.points().begin();
            kvs::ColorMap::Points::const_iterator last = m_color_map.points().end();
            while ( point != last )
            {
                const float value = point->first;
                const kvs::RGBColor color = point->second;

                const float normalized_value = ( value - src_min_value ) / ( src_max_value - src_min_value );
                const float adjusted_value = normalized_value * ( max_value - min_value ) + min_value;

                m_color_map.removePoint( value );
                m_color_map.addPoint( adjusted_value, color );

                point++;
            }
        }

        // Adjust opacity map.
        {
            kvs::OpacityMap::Points::const_iterator point = m_opacity_map.points().begin();
            kvs::OpacityMap::Points::const_iterator last = m_opacity_map.points().end();
            while ( point != last )
            {
                const float value = point->first;
                const float opacity = point->second;

                const float normalized_value = ( value - src_min_value ) / ( src_max_value - src_min_value );
                const float adjusted_value = normalized_value * ( max_value - min_value ) + min_value;

                m_opacity_map.removePoint( value );
                m_opacity_map.addPoint( adjusted_value, opacity );

                point++;
            }
        }
    }

    this->setRange( min_value, max_value );
}

void TransferFunction::adjustRange( const pbvr::VolumeObjectBase& volume )
{
    const float min_value = static_cast<float>( volume.minValue() );
    const float max_value = static_cast<float>( volume.maxValue() );
    this->adjustRange( min_value, max_value );
}

const bool TransferFunction::hasRange() const
{
    return m_color_map.hasRange() && m_opacity_map.hasRange();
}

const float TransferFunction::minValue() const
{
    KVS_ASSERT( m_color_map.minValue() == m_opacity_map.minValue() );
    return m_color_map.minValue();
}

const float TransferFunction::maxValue() const
{
    KVS_ASSERT( m_color_map.maxValue() == m_opacity_map.maxValue() );
    return m_color_map.maxValue();
}

/*==========================================================================*/
/**
 *  @brief  Returns the color map.
 */
/*==========================================================================*/
const kvs::ColorMap& TransferFunction::colorMap() const
{
    return m_color_map;
}

/*==========================================================================*/
/**
 *  @brief  Returns the opacity map.
 */
/*==========================================================================*/
const kvs::OpacityMap& TransferFunction::opacityMap() const
{
    return m_opacity_map;
}

/*==========================================================================*/
/**
 *  @brief  Returns the resolution.
 */
/*==========================================================================*/
const size_t TransferFunction::resolution() const
{
    KVS_ASSERT( m_opacity_map.resolution() == m_color_map.resolution() );
    return m_opacity_map.resolution();
}

/*==========================================================================*/
/**
 *  @brief  Create the alpha map.
 *  @param  resolution [in] resolution
 */
/*==========================================================================*/
void TransferFunction::create( const size_t resolution )
{
    m_opacity_map.setResolution( resolution );
    m_opacity_map.create();

    m_color_map.setResolution( resolution );
    m_color_map.create();
}

/*==========================================================================*/
/**
 *  @brief  Read a trasfer function data from the given file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*==========================================================================*/
/* 131017 revivaled */
const bool TransferFunction::read( const std::string& filename )
{
    kvs::KVSMLTransferFunction m_transfer_function( filename );
    if ( m_transfer_function.isFailure() ) return false;

    const float min_value = m_transfer_function.minValue();
    const float max_value = m_transfer_function.maxValue();
    const bool has_range = !( kvs::Math::Equal( min_value, max_value ) );
    if ( has_range )
    {
        m_color_map.setRange( min_value, max_value );
        m_opacity_map.setRange( min_value, max_value );
    }

    m_color_map.setResolution( m_transfer_function.resolution() );
    if ( m_transfer_function.colorPointList().size() > 0 )
    {
        kvs::KVSMLTransferFunction::ColorPointList::const_iterator point =
            m_transfer_function.colorPointList().begin();
        kvs::KVSMLTransferFunction::ColorPointList::const_iterator last =
            m_transfer_function.colorPointList().end();
        while ( point != last )
        {
            const float value = point->first;
            const kvs::RGBColor color = point->second;
            m_color_map.addPoint( value, color );
            point++;
        }

        m_color_map.create();
    }
    else
    {
        m_color_map = kvs::ColorMap( m_transfer_function.colors() );
    }

    m_opacity_map.setResolution( m_transfer_function.resolution() );
    if ( m_transfer_function.opacityPointList().size() > 0 )
    {
        kvs::KVSMLTransferFunction::OpacityPointList::const_iterator point =
            m_transfer_function.opacityPointList().begin();
        kvs::KVSMLTransferFunction::OpacityPointList::const_iterator last =
            m_transfer_function.opacityPointList().end();
        while ( point != last )
        {
            const float value = point->first;
            const kvs::Real32 opacity = point->second;
            m_opacity_map.addPoint( value, opacity );
            point++;
        }

        m_opacity_map.create();
    }
    else
    {
        m_opacity_map = kvs::OpacityMap( m_transfer_function.opacities() );
    }

    return true;
}

/*==========================================================================*/
/**
 *  @brief  Writes the trasfer function data.
 *  @param  filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*==========================================================================*/
/*
const bool TransferFunction::write( const std::string& filename )
{
   kvs::KVSMLTransferFunction m_transfer_function;
   m_transfer_function.setm_resolution( this->resolution() );
   m_transfer_function.setWritingDataType( kvs::KVSMLTransferFunction::Ascii );

   if ( m_color_map.points().size() > 0 )
   {
       kvs::ColorMap::Points::const_iterator point = m_color_map.points().begin();
       kvs::ColorMap::Points::const_iterator last = m_color_map.points().end();
       while ( point != last )
       {
           m_transfer_function.addColorPoint( point->first, point->second );
           point++;
       }
   }
   else
   {
       m_transfer_function.setColors( m_color_map.table() );
   }

   if ( m_opacity_map.points().size() > 0 )
   {
       kvs::OpacityMap::Points::const_iterator point = m_opacity_map.points().begin();
       kvs::OpacityMap::Points::const_iterator last = m_opacity_map.points().end();
       while ( point != last )
       {
           m_transfer_function.addOpacityPoint( point->first, point->second );
           point++;
       }
   }
   else
   {
       m_transfer_function.setOpacities( m_opacity_map.table() );
   }

   return m_transfer_function.write( filename );
}
*/
/*==========================================================================*/
/**
 *  @brief  Substitution operator =.
 *  @param  trunsfer_function [in] transfer function
 *  @retval transfer function
 */
/*==========================================================================*/
TransferFunction& TransferFunction::operator =( const TransferFunction& rhs )
{
    m_color_map   = rhs.m_color_map;
    m_opacity_map = rhs.m_opacity_map;

    return *this;
}

} // end of namespace pbvr
