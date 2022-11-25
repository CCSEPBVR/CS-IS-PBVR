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
#ifndef KVS__FREQUENCY_TABLE_H_INCLUDE
#define KVS__FREQUENCY_TABLE_H_INCLUDE

#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Math>
#include <kvs/Message>
#include <kvs/VolumeObjectBase>
#include <kvs/ImageObject>
#include <kvs/ClassName>
#include <list>


namespace kvs
{

/*==========================================================================*/
/**
 *  Frequency distribution table class.
 */
/*==========================================================================*/
class FrequencyTable
{
    kvsClassName( kvs::FrequencyTable );

protected:

    kvs::Real64             m_min_range;     ///< min. range value
    kvs::Real64             m_max_range;     ///< max. range value
    size_t                  m_max_count;     ///< min. count value
    kvs::Real64             m_mean;          ///< mean value
    kvs::Real64             m_variance;      ///< variance value
    kvs::Real64             m_standard_deviation; ///< standard deviation
    kvs::UInt64             m_nbins;         ///< number of bins
    kvs::ValueArray<size_t> m_bin;           ///< bin array
    std::list<kvs::Real64>  m_ignore_values; ///< ignore value list

public:

    FrequencyTable( void );

    virtual ~FrequencyTable( void );

public:

    const kvs::Real64 minRange( void ) const;

    const kvs::Real64 maxRange( void ) const;

    const size_t maxCount( void ) const;

    const kvs::Real64 mean( void ) const;

    const kvs::Real64 variance( void ) const;

    const kvs::Real64 standardDeviation( void ) const;

    const kvs::UInt64 nbins( void ) const;

    const kvs::ValueArray<size_t>& bin( void ) const;

public:

    void setIgnoreValue( const kvs::Real64 value );

    void setRange( const kvs::Real64 min_range, const kvs::Real64 max_range );

    void setNBins( const kvs::UInt64 nbins );

public:

    void create( const kvs::VolumeObjectBase* volume );

    void create( const kvs::ImageObject* image, const size_t channel = 0 );

public:

    const kvs::UInt64 operator [] ( const size_t index ) const;

    const kvs::UInt64 at( const size_t index ) const;

private:

    void calculate_range( const kvs::VolumeObjectBase* volume );

    void calculate_range( const kvs::ImageObject* image );

    void count_bin( const kvs::VolumeObjectBase* volume );

    void count_bin( const kvs::ImageObject* image, const size_t channel );

    template <typename T>
    void binning( const kvs::VolumeObjectBase* volume );

    template <typename T>
    void binning( const kvs::ImageObject* image, const size_t channel );

    bool is_ignore_value( const kvs::Real64 value );
};

/*==========================================================================*/
/**
 *  Create a bin array.
 *  @param volume [in] pointer to the volume object
 */
/*==========================================================================*/
template <typename T>
inline void FrequencyTable::binning( const kvs::VolumeObjectBase* volume )
{
    const size_t veclen = volume->veclen();
    const T* value = reinterpret_cast<const T*>( volume->values().pointer() );
    const T* const end = value + volume->nnodes() * veclen;
//    const kvs::Real64 width = ( m_max_range - m_min_range ) / kvs::Real64( m_nbins - 1 );
    const kvs::Real64 width = ( m_max_range - m_min_range + 1 ) / kvs::Real64( m_nbins );

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
                m_max_count = kvs::Math::Max( m_max_count, m_bin[index] );

                total_count++;
            }
            ++value;
        }
    }
    else
    {
        while ( value < end )
        {
            kvs::Real64 magnitude = 0.0;
            for ( size_t i = 0; i < veclen; ++i )
            {
                magnitude += static_cast<kvs::Real64>( ( *value ) * ( *value ) );
                ++value;
            }
            magnitude = kvs::Math::SquareRoot( magnitude );

            if ( !this->is_ignore_value( magnitude ) )
            {
                const size_t index = static_cast<size_t>( ( magnitude - m_min_range ) / width + 0.5f );
//                const size_t index = static_cast<size_t>( ( magnitude - m_min_range ) / width );
                m_bin[index] = m_bin[index] + 1;
                m_max_count = kvs::Math::Max( m_max_count, m_bin[index] );

                total_count++;
            }
        }
    }

    m_mean = static_cast<kvs::Real64>( total_count ) / m_nbins;

    kvs::Real64 sum = 0;
    for ( size_t i = 0; i < m_nbins; i++ ) sum += kvs::Math::Square( m_bin[i] - m_mean );
    m_variance = sum / m_nbins;

    m_standard_deviation = std::sqrt( m_variance );
}

/*==========================================================================*/
/**
 *  Create a bin array.
 *  @param image [in] pointer to the image object
 *  @param channel [in] color element channel (0, 1, 2, 3)
 */
/*==========================================================================*/
template <typename T>
inline void FrequencyTable::binning( const kvs::ImageObject* image, const size_t channel )
{
    if ( channel >= image->nchannels() )
    {
        kvsMessageError("Specified channel is invalid.");
        return;
    }

    const T* values = reinterpret_cast<const T*>( image->data().pointer() );
//    const kvs::Real64 width = ( m_max_range - m_min_range ) / kvs::Real64( m_nbins - 1 );
    const kvs::Real64 width = ( m_max_range - m_min_range + 1 ) / kvs::Real64( m_nbins );
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
            m_max_count = kvs::Math::Max( m_max_count, m_bin[index] );

            total_count++;
        }
    }

    m_mean = static_cast<kvs::Real64>( total_count ) / m_nbins;

    kvs::Real64 sum = 0;
    for ( size_t i = 0; i < m_nbins; i++ ) sum += kvs::Math::Square( m_bin[i] - m_mean );
    m_variance = sum / m_nbins;

    m_standard_deviation = std::sqrt( m_variance );
}

} // end of namespace kvs

#endif // KVS__FREQUENCY_TABLE_H_INCLUDE
