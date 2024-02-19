/****************************************************************************/
/**
 *  @file FrequencyTable.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FrequencyTable.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "FrequencyTable.h"
#include <kvs/Type>
#include <kvs/Value>


namespace pbvr
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new FrequencyTable class.
 */
/*==========================================================================*/
FrequencyTable::FrequencyTable():
    m_min_range( 0 ),
    m_max_range( 0 ),
    m_max_count( 0 ),
    m_mean( 0 ),
    m_variance( 0 ),
    m_standard_deviation( 0 ),
    m_nbins( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the FrequencyTable class.
 */
/*===========================================================================*/
FrequencyTable::~FrequencyTable()
{
}

/*==========================================================================*/
/**
 *  @brief  Returns the min. range.
 *  @return min. range
 */
/*==========================================================================*/
const kvs::Real64 FrequencyTable::minRange() const
{
    return m_min_range;
}

/*==========================================================================*/
/**
 *  @brief  Returns the max. range.
 *  @return max. range
 */
/*==========================================================================*/
const kvs::Real64 FrequencyTable::maxRange() const
{
    return m_max_range;
}

/*==========================================================================*/
/**
 *  @brief  Returns the max. counts of the bins.
 *  @return max. counts
 */
/*==========================================================================*/
const size_t FrequencyTable::maxCount() const
{
    return m_max_count;
}

const kvs::Real64 FrequencyTable::mean() const
{
    return m_mean;
}

const kvs::Real64 FrequencyTable::variance() const
{
    return m_variance;
}

const kvs::Real64 FrequencyTable::standardDeviation() const
{
    return m_standard_deviation;
}

/*==========================================================================*/
/**
 *  @brief  Returns the number of bins.
 *  @return number of bins
 */
/*==========================================================================*/
const kvs::UInt64 FrequencyTable::nbins() const
{
    return m_nbins;
}

/*==========================================================================*/
/**
 *  @brief  Returns the bin array.
 *  @return bin array
 */
/*==========================================================================*/
const kvs::ValueArray<size_t>& FrequencyTable::bin() const
{
    return m_bin;
}

/*==========================================================================*/
/**
 *  @brief  Sets an ignore value.
 */
/*==========================================================================*/
void FrequencyTable::setIgnoreValue( const kvs::Real64 value )
{
    m_ignore_values.push_back( value );
}

/*===========================================================================*/
/**
 *  @brief  Sets data range.
 *  @param  min_range [in] min value range
 *  @param  max_range [in] max value range
 */
/*===========================================================================*/
void FrequencyTable::setRange( const kvs::Real64 min_range, const kvs::Real64 max_range )
{
    m_min_range = min_range;
    m_max_range = max_range;
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of bins
 *  @param  nbins [in] number of bins
 */
/*===========================================================================*/
void FrequencyTable::setNBins( const kvs::UInt64 nbins )
{
    m_nbins = nbins;
}

/*==========================================================================*/
/**
 *  @brief  Allocation a frequency distribution table
 *  @param  volume [in] pointer to the volume object
 */
/*==========================================================================*/
void FrequencyTable::allocate()
{
    m_bin.allocate( static_cast<size_t>( m_nbins ) );
    m_bin.fill( 0x00 );
}

/*==========================================================================*/
/**
 *  @brief  Creates a frequency distribution table for a structured volume object.
 *  @param  volume [in] pointer to the volume object
 */
/*==========================================================================*/
/*
void FrequencyTable::create( const pbvr::VolumeObjectBase* volume )
{
    // Calculate the min/max range value and the number of bins.
    //this->calculate_range( volume );

    // Allocate and initialize the bin array.
    //m_bin.allocate( static_cast<size_t>( m_nbins ) );
    //m_bin.fill( 0x00 );

    // Count the bin.
    this->count_bin( volume );
}
*/
/*==========================================================================*/
/**
 *  @brief  Creates a frequency distribution table for a image object.
 *  @param  image [in] pointer to the image object
 *  @param  channel [in] color element channel (0, 1, 2, 3)
 */
/*==========================================================================*/
/*
void FrequencyTable::create( const kvs::ImageObject* image, const size_t channel )
{
    // Calculate the min/max range value and the number of bins.
    this->calculate_range( image );

    // Allocate and initialize the bin array.
    m_bin.allocate( static_cast<size_t>( m_nbins ) );
    m_bin.fill( 0x00 );

    // Count the bin.
    this->count_bin( image, channel );
}
*/
/*==========================================================================*/
/**
 *  @brief  Returns counts of the bin which is specified by the index.
 *  @param  index [in] index
 *  @return counts
 */
/*==========================================================================*/
const kvs::UInt64 FrequencyTable::operator [] ( const size_t index ) const
{
    return m_bin[ index ];
}

/*==========================================================================*/
/**
 *  @brief  Returns counts of the bin which is specified by the index.
 *  @param  index [in] index
 *  @return counts
 */
/*==========================================================================*/
const kvs::UInt64 FrequencyTable::at( const size_t index ) const
{
    return m_bin.at( index );
}

/*==========================================================================*/
/**
 *  @brief  Calculates the bin range.
 *  @param  volume [in] pointer to the volume object
 */
/*==========================================================================*/
#if 0
void FrequencyTable::calculate_range( const pbvr::VolumeObjectBase* volume )
{
    // Number of bins.
    if ( m_nbins == 0 )
    {
        // It is neccessary to estimate the number of bins by binning process.
        m_nbins = 256;
    }

    // Data range.
    const std::type_info& type = volume->values().typeInfo()->type();
    if ( type == typeid( kvs::Int8 ) )
    {
        if ( kvs::Math::IsZero( m_min_range ) && kvs::Math::IsZero( m_max_range ) )
        {
            m_min_range = kvs::Value<kvs::Int8>::Min();
            m_max_range = kvs::Value<kvs::Int8>::Max();
        }
    }

    else if ( type == typeid( kvs::UInt8 ) )
    {
        if ( kvs::Math::IsZero( m_min_range ) && kvs::Math::IsZero( m_max_range ) )
        {
            m_min_range = kvs::Value<kvs::UInt8>::Min();
            m_max_range = kvs::Value<kvs::UInt8>::Max();
        }
    }

    else if ( type == typeid( kvs::Int16  ) )
    {
        if ( kvs::Math::IsZero( m_min_range ) && kvs::Math::IsZero( m_max_range ) )
        {
            m_min_range = kvs::Value<kvs::Int16>::Min();
            m_max_range = kvs::Value<kvs::Int16>::Max();
        }
    }

    else if ( type == typeid( kvs::UInt16 ) )
    {
        if ( kvs::Math::IsZero( m_min_range ) && kvs::Math::IsZero( m_max_range ) )
        {
            m_min_range = kvs::Value<kvs::UInt16>::Min();
            m_max_range = kvs::Value<kvs::UInt16>::Max();
        }
    }

    else if ( type == typeid( kvs::Int32  ) )
    {
        if ( kvs::Math::IsZero( m_min_range ) && kvs::Math::IsZero( m_max_range ) )
        {
            m_min_range = kvs::Value<kvs::Int32>::Min();
            m_max_range = kvs::Value<kvs::Int32>::Max();
        }
    }

    else if ( type == typeid( kvs::UInt32 ) )
    {
        if ( kvs::Math::IsZero( m_min_range ) && kvs::Math::IsZero( m_max_range ) )
        {
            m_min_range = kvs::Value<kvs::UInt32>::Min();
            m_max_range = kvs::Value<kvs::UInt32>::Max();
        }
    }

    else if ( type == typeid( kvs::Int64 ) )
    {
        if ( kvs::Math::IsZero( m_min_range ) && kvs::Math::IsZero( m_max_range ) )
        {
            m_min_range = static_cast<kvs::Real64>( kvs::Value<kvs::Int64>::Min() );
            m_max_range = static_cast<kvs::Real64>( kvs::Value<kvs::Int64>::Max() );
        }
    }

    else if ( type == typeid( kvs::UInt64 ) )
    {
        if ( kvs::Math::IsZero( m_min_range ) && kvs::Math::IsZero( m_max_range ) )
        {
            m_min_range = static_cast<kvs::Real64>( kvs::Value<kvs::UInt64>::Min() );
            m_max_range = static_cast<kvs::Real64>( kvs::Value<kvs::UInt64>::Max() );
        }
    }

    else if ( type == typeid( kvs::Real32 ) || type == typeid( kvs::Real64 ) )
    {
        if ( kvs::Math::IsZero( m_min_range ) && kvs::Math::IsZero( m_max_range ) )
        {
            m_min_range = volume->minValue();
            m_max_range = volume->maxValue();
        }
    }
}
#endif
/*==========================================================================*/
/**
 *  Calculate a bin range.
 *  @param image [in] pointer to the image object
 */
/*==========================================================================*/
/*void FrequencyTable::calculate_range( const kvs::ImageObject* image )
{
    switch ( image->type() )
    {
    case kvs::ImageObject::Gray8:
    case kvs::ImageObject::Color24: // RGB
    case kvs::ImageObject::Color32: // RGBA
    {
        m_min_range = kvs::Value<kvs::UInt8>::Min();
        m_max_range = kvs::Value<kvs::UInt8>::Max();
        m_nbins     = kvs::UInt64(1) << 8;
        break;
    }
    case kvs::ImageObject::Gray16:
    {
        m_min_range = kvs::Value<kvs::UInt16>::Min();
        m_max_range = kvs::Value<kvs::UInt16>::Max();
        m_nbins     = kvs::UInt64(1) << 16;
        break;
    }
    default: break;
    }
}
*/
/*==========================================================================*/
/**
 *  @brief  Counts the bin.
 *  @param  volume [in] pointer to the volume object
 */
/*==========================================================================*/
/*
void FrequencyTable::count_bin( const pbvr::VolumeObjectBase* volume )
{
    const std::type_info& type = volume->values().typeInfo()->type();
    if (      type == typeid( kvs::Int8 )   ) this->binning<kvs::Int8>( volume );
    else if ( type == typeid( kvs::UInt8 )  ) this->binning<kvs::UInt8>( volume );
    else if ( type == typeid( kvs::Int16 )  ) this->binning<kvs::Int16>( volume );
    else if ( type == typeid( kvs::UInt16 ) ) this->binning<kvs::UInt16>( volume );
    else if ( type == typeid( kvs::Int32 )  ) this->binning<kvs::Int32>( volume );
    else if ( type == typeid( kvs::UInt32 ) ) this->binning<kvs::UInt32>( volume );
    else if ( type == typeid( kvs::Real32 ) ) this->binning<kvs::Real32>( volume );
    else if ( type == typeid( kvs::Real64 ) ) this->binning<kvs::Real64>( volume );
}
*/
/*==========================================================================*/
/**
 *  @brief  Counts the bin.
 *  @param  image [in] pointer to the image object
 *  @param  channel [in] color element channel (0, 1, 2, 3)
 */
/*==========================================================================*/
/*void FrequencyTable::count_bin( const kvs::ImageObject* image, const size_t channel )
{
    switch ( image->type() )
    {
    case kvs::ImageObject::Gray8:
    case kvs::ImageObject::Color24: // RGB
    case kvs::ImageObject::Color32: // RGBA
        this->binning<kvs::UInt8>( image, channel );
        break;
    case kvs::ImageObject::Gray16:
        this->binning<kvs::UInt16>( image, channel );
        break;
    default: break;
    }
}
*/
/*==========================================================================*/
/**
 *  @brief  Tests which a value is the ignore value or not.
 *  @param  value [in] value
 *  @return true, if the value is the ignore value.
 */
/*==========================================================================*/
bool FrequencyTable::is_ignore_value( const kvs::Real64 value )
{
    std::list<kvs::Real64>::const_iterator ignore_value = m_ignore_values.begin();
    std::list<kvs::Real64>::const_iterator end = m_ignore_values.end();
    while ( ignore_value != end )
    {
        if ( kvs::Math::Equal( value, *ignore_value ) ) return true;

        ++ignore_value;
    }

    return false;
}

} // end of namespace pbvr
