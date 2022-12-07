#ifndef PBVR__KVS__FREQUENCY_TABLE_H_INCLUDE
#define PBVR__KVS__FREQUENCY_TABLE_H_INCLUDE

#include <kvs/FrequencyTable>
#include <iostream>
#include <algorithm>

namespace kvs
{
namespace visclient
{

class FrequencyTable : public kvs::FrequencyTable
{
public:
    FrequencyTable() : kvs::FrequencyTable() { }

    FrequencyTable( const kvs::FrequencyTable& table )
    {
        m_min_range = table.minRange();
        m_max_range = table.maxRange();
        m_max_count = table.maxCount();
        m_nbins = table.nbins();
        m_bin = table.bin();
    }

    FrequencyTable( const float min, const float max,
                    const int nbins, const size_t* bins )
    {
        initialize(min, max, nbins, bins);
        /* modify by @hira at 2016/12/01
        m_min_range = min;
        m_max_range = max;
        m_nbins = nbins;
        m_bin.deepCopy( bins, nbins );

        m_max_count = 0;
        for ( int n = 0; n < nbins; n++ )
        {
            m_max_count = std::max( m_max_count, bins[n] );
        }

        if ( m_max_count == 0 )
        {
            m_bin.at( 0 ) = 1;
            m_max_count = 1;
        }
        */
    }

    FrequencyTable( const float min, const float max,
                    const int nbins, const size_t* bins,
                    const std::string &function_name)
         // : FrequencyTable(min, max, nbins, bins)   modify by @hira at 2016/12/01
    {
        initialize(min, max, nbins, bins);
        this->m_function_name = function_name;
    }

    void initialize(const float min, const float max,
                    const int nbins, const size_t* bins )
    {
        m_min_range = min;
        m_max_range = max;
        m_nbins = nbins;
        m_bin.deepCopy( bins, nbins );

        m_max_count = 0;
        for ( int n = 0; n < nbins; n++ )
        {
            m_max_count = std::max( m_max_count, bins[n] );
        }

        if ( m_max_count == 0 )
        {
            m_bin.at( 0 ) = 1;
            m_max_count = 1;
        }
    }

    void dump()
    {
        std::cout << m_min_range << ' ' << m_max_range << ' ' << m_nbins << ' ' << m_max_count << std::endl;
        for ( kvs::UInt64 n = 0; n < m_nbins; n++ )
        {
            std::cout << m_bin[n] << ' ';
        }
        std::cout << std::endl;
    }

public:
    std::string m_function_name;

};

}
}

#endif    // PBVR__KVS__FREQUENCY_TABLE_H_INCLUDE

