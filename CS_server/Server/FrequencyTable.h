/****************************************************************************/
/**
 *  @file FrequencyTable.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FrequencyTable.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__FREQUENCY_TABLE_H_INCLUDE
#define PBVR__FREQUENCY_TABLE_H_INCLUDE

#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Math>
#include <vismodule/Message>
//#include "VolumeObjectBase.h"
//#include "ImageObject.h"
#include "ClassName.h"
#include <list>

const static size_t DEFAULT_NBINS = 256;

namespace pbvr
{

/*==========================================================================*/
/**
 *  Frequency distribution table class.
 */
/*==========================================================================*/
class FrequencyTable
{
    visModuleClassName( pbvr::FrequencyTable );

protected:

    vismodule::Real64             m_min_range;     ///< min. range value
    vismodule::Real64             m_max_range;     ///< max. range value
    size_t                  m_max_count;     ///< min. count value
    vismodule::Real64             m_mean;          ///< mean value
    vismodule::Real64             m_variance;      ///< variance value
    vismodule::Real64             m_standard_deviation; ///< standard deviation
    vismodule::UInt64             m_nbins;         ///< number of bins
    vismodule::ValueArray<size_t> m_bin;           ///< bin array
    std::list<vismodule::Real64>  m_ignore_values; ///< ignore value list

public:

    FrequencyTable();

    virtual ~FrequencyTable();

public:

    const vismodule::Real64 minRange() const;

    const vismodule::Real64 maxRange() const;

    const size_t maxCount() const;

    const vismodule::Real64 mean() const;

    const vismodule::Real64 variance() const;

    const vismodule::Real64 standardDeviation() const;

    const vismodule::UInt64 nbins() const;

    const vismodule::ValueArray<size_t>& bin() const;

public:

    void setIgnoreValue( const vismodule::Real64 value );

    void setRange( const vismodule::Real64 min_range, const vismodule::Real64 max_range );

    void setNBins( const vismodule::UInt64 nbins );

    void allocate();

public:

    //void create( const pbvr::VolumeObjectBase* volume );

    //void create( const vismodule::ImageObject* image, const size_t channel = 0 );
    //
    template <typename T>
    inline void create( const vismodule::ValueArray<T>& vec );

public:

    const vismodule::UInt64 operator [] ( const size_t index ) const;

    const vismodule::UInt64 at( const size_t index ) const;

private:

    //void calculate_range( const pbvr::VolumeObjectBase* volume );

    //void calculate_range( const vismodule::ImageObject* image );

    //void count_bin( const pbvr::VolumeObjectBase* volume );

    //void count_bin( const vismodule::ImageObject* image, const size_t channel );

    template <typename T>
    inline void count_bin( const vismodule::ValueArray<T>& vec );

    //template <typename T>
    //void binning( const pbvr::VolumeObjectBase* volume );

    //template <typename T>
    //void binning( const vismodule::ImageObject* image, const size_t channel );
    //
    template <typename T>
    inline void binning( const  vismodule::ValueArray<T>& vec );

    bool is_ignore_value( const vismodule::Real64 value );
};

/*==========================================================================*/
/**
 *  Create a bin array.
 *  @param volume [in] pointer to the volume object
 */
/*==========================================================================*/

template <typename T>
inline void FrequencyTable::binning( const vismodule::ValueArray<T>& vec )
{
    const size_t veclen = 1;
    typename vismodule::ValueArray<T>::const_iterator value = vec.begin();
    typename vismodule::ValueArray<T>::const_iterator end   = vec.end();
    const vismodule::Real64 width = ( m_max_range - m_min_range ) / vismodule::Real64( m_nbins - 1 );
//  const vismodule::Real64 width = ( m_max_range - m_min_range + 1 ) / vismodule::Real64( m_nbins );

    size_t total_count = 0;

    m_max_count = 0;
    if ( veclen == 1 )
    {
        while ( value < end )
        {
            if ( !this->is_ignore_value( *value ) && m_min_range <= *value && *value <= m_max_range )
            {
//                const size_t index = static_cast<size_t>( ( *value - m_min_range ) / width + 0.5f );
                const size_t index = static_cast<size_t>( ( *value - m_min_range ) / width );
                m_bin[index] = m_bin[index] + 1;
                m_max_count = vismodule::Math::Max( m_max_count, m_bin[index] );

                total_count++;
            }
            ++value;
        }
    }
    else
    {
        while ( value < end )
        {
            vismodule::Real64 magnitude = 0.0;
            for ( size_t i = 0; i < veclen; ++i )
            {
                magnitude += static_cast<vismodule::Real64>( ( *value ) * ( *value ) );
                ++value;
            }
            magnitude = vismodule::Math::SquareRoot( magnitude );

            if ( !this->is_ignore_value( magnitude ) )
            {
                const size_t index = static_cast<size_t>( ( magnitude - m_min_range ) / width + 0.5f );
//              const size_t index = static_cast<size_t>( ( magnitude - m_min_range ) / width );
                m_bin[index] = m_bin[index] + 1;
                m_max_count = vismodule::Math::Max( m_max_count, m_bin[index] );

                total_count++;
            }
        }
    }

    m_mean = static_cast<vismodule::Real64>( total_count ) / m_nbins;

    vismodule::Real64 sum = 0;
    for ( size_t i = 0; i < m_nbins; i++ ) sum += vismodule::Math::Square( m_bin[i] - m_mean );
    m_variance = sum / m_nbins;

    m_standard_deviation = std::sqrt( m_variance );
}

template <typename T>
inline void FrequencyTable::count_bin( const vismodule::ValueArray<T>& vec )
{
    this->binning<T>( vec );
}

template <typename T>
inline void FrequencyTable::create( const vismodule::ValueArray<T>& vec )
{
    this->count_bin( vec );
}

/*==========================================================================*/
/**
 *  Create a bin array.
 *  @param volume [in] pointer to the volume object
 */
/*==========================================================================*/
/*
template <typename T>
inline void FrequencyTable::binning( const pbvr::VolumeObjectBase* volume )
{
    const size_t veclen = volume->veclen();
    const T* value = reinterpret_cast<const T*>( volume->values().pointer() );
    const T* const end = value + volume->nnodes() * veclen;
//    const vismodule::Real64 width = ( m_max_range - m_min_range ) / vismodule::Real64( m_nbins - 1 );
    const vismodule::Real64 width = ( m_max_range - m_min_range + 1 ) / vismodule::Real64( m_nbins );

    size_t total_count = 0;

    m_max_count = 0;
    if ( veclen == 1 )
    {
        while ( value < end )
        {
            if ( !this->is_ignore_value( *value ) )
            {
                const size_t index = static_cast<size_t>( ( *value - m_min_range ) / width + 0.5f );
//                const size_t index = static_cast<size_t>( ( *value - m_min_range ) / width );
                m_bin[index] = m_bin[index] + 1;
                m_max_count = vismodule::Math::Max( m_max_count, m_bin[index] );

                total_count++;
            }
            ++value;
        }
    }
    else
    {
        while ( value < end )
        {
            vismodule::Real64 magnitude = 0.0;
            for ( size_t i = 0; i < veclen; ++i )
            {
                magnitude += static_cast<vismodule::Real64>( ( *value ) * ( *value ) );
                ++value;
            }
            magnitude = vismodule::Math::SquareRoot( magnitude );

            if ( !this->is_ignore_value( magnitude ) )
            {
                const size_t index = static_cast<size_t>( ( magnitude - m_min_range ) / width + 0.5f );
//                const size_t index = static_cast<size_t>( ( magnitude - m_min_range ) / width );
                m_bin[index] = m_bin[index] + 1;
                m_max_count = vismodule::Math::Max( m_max_count, m_bin[index] );

                total_count++;
            }
        }
    }

    m_mean = static_cast<vismodule::Real64>( total_count ) / m_nbins;

    vismodule::Real64 sum = 0;
    for ( size_t i = 0; i < m_nbins; i++ ) sum += vismodule::Math::Square( m_bin[i] - m_mean );
    m_variance = sum / m_nbins;

    m_standard_deviation = std::sqrt( m_variance );
}
*/
/*==========================================================================*/
/**
 *  Create a bin array.
 *  @param image [in] pointer to the image object
 *  @param channel [in] color element channel (0, 1, 2, 3)
 */
/*==========================================================================*/
/*template <typename T>
inline void FrequencyTable::binning( const vismodule::ImageObject* image, const size_t channel )
{
    if ( channel >= image->nchannels() )
    {
        visModuleMessageError("Specified channel is invalid.");
        return;
    }

    const T* values = reinterpret_cast<const T*>( image->data().pointer() );
//    const vismodule::Real64 width = ( m_max_range - m_min_range ) / vismodule::Real64( m_nbins - 1 );
    const vismodule::Real64 width = ( m_max_range - m_min_range + 1 ) / vismodule::Real64( m_nbins );
    const size_t stride  = image->nchannels();
    const size_t npixels = image->width() * image->height();

    size_t total_count = 0;

    m_max_count = 0;
    for ( size_t i = 0; i < npixels; i++ )
    {
        const T value = *( values + channel + i * stride );

        if ( !this->is_ignore_value( value ) )
        {
//            const size_t index = static_cast<size_t>( ( value - m_min_range ) / width + 0.5f );
            const size_t index = static_cast<size_t>( ( value - m_min_range ) / width );
            m_bin[index] = m_bin[index] + 1;
            m_max_count = vismodule::Math::Max( m_max_count, m_bin[index] );

            total_count++;
        }
    }

    m_mean = static_cast<vismodule::Real64>( total_count ) / m_nbins;

    vismodule::Real64 sum = 0;
    for ( size_t i = 0; i < m_nbins; i++ ) sum += vismodule::Math::Square( m_bin[i] - m_mean );
    m_variance = sum / m_nbins;

    m_standard_deviation = std::sqrt( m_variance );
}
*/
} // end of namespace pbvr

#endif // VIS_MODULE__FREQUENCY_TABLE_H_INCLUDE
