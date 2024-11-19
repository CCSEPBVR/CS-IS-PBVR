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
#ifndef VIS_MODULE__FREQUENCY_TABLE_H_INCLUDE
#define VIS_MODULE__FREQUENCY_TABLE_H_INCLUDE

#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Math>
#include <vismodule/Message>
#include <vismodule/VolumeObjectBase>
#include <vismodule/ImageObject>
#include <vismodule/ClassName>
#include <list>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Frequency distribution table class.
 */
/*==========================================================================*/
class FrequencyTable
{
    visModuleClassName( vismodule::FrequencyTable );

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

    FrequencyTable( void );

    virtual ~FrequencyTable( void );

public:

    const vismodule::Real64 minRange( void ) const;

    const vismodule::Real64 maxRange( void ) const;

    const size_t maxCount( void ) const;

    const vismodule::Real64 mean( void ) const;

    const vismodule::Real64 variance( void ) const;

    const vismodule::Real64 standardDeviation( void ) const;

    const vismodule::UInt64 nbins( void ) const;

    const vismodule::ValueArray<size_t>& bin( void ) const;

public:

    void setIgnoreValue( const vismodule::Real64 value );

    void setRange( const vismodule::Real64 min_range, const vismodule::Real64 max_range );

    void setNBins( const vismodule::UInt64 nbins );

public:

    void create( const vismodule::VolumeObjectBase* volume );

    void create( const vismodule::ImageObject* image, const size_t channel = 0 );

public:

    const vismodule::UInt64 operator [] ( const size_t index ) const;

    const vismodule::UInt64 at( const size_t index ) const;

private:

    void calculate_range( const vismodule::VolumeObjectBase* volume );

    void calculate_range( const vismodule::ImageObject* image );

    void count_bin( const vismodule::VolumeObjectBase* volume );

    void count_bin( const vismodule::ImageObject* image, const size_t channel );

    template <typename T>
    void binning( const vismodule::VolumeObjectBase* volume );

    template <typename T>
    void binning( const vismodule::ImageObject* image, const size_t channel );

    bool is_ignore_value( const vismodule::Real64 value );
};

/*==========================================================================*/
/**
 *  Create a bin array.
 *  @param volume [in] pointer to the volume object
 */
/*==========================================================================*/
template <typename T>
inline void FrequencyTable::binning( const vismodule::VolumeObjectBase* volume )
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

/*==========================================================================*/
/**
 *  Create a bin array.
 *  @param image [in] pointer to the image object
 *  @param channel [in] color element channel (0, 1, 2, 3)
 */
/*==========================================================================*/
template <typename T>
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

} // end of namespace vismodule

#endif // VIS_MODULE__FREQUENCY_TABLE_H_INCLUDE
