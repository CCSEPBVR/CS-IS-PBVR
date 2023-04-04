#ifndef __KVS_FREQUENCY_TABLE_H__
#define __KVS_FREQUENCY_TABLE_H__

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

    }
    //Osaki QTISPBVR update(2020/11/16) OpenGL版のISでは使用しているためifdefを無効にしました。
//#ifdef CS_MODE
    FrequencyTable( const float min, const float max,
                    const int nbins, const size_t* bins,
                    const std::string &function_name)
    {
        initialize(min, max, nbins, bins);
        this->m_function_name = function_name;
    }
//#endif

    void initialize(const float min, const float max,
                    const int nbins, const size_t* bins )
    {
        m_min_range = min;
        m_max_range = max;
        m_nbins = nbins;
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.05.28
        //m_bin.deepCopy( bins, nbins );
        m_bin.assign( bins, nbins );

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
    //Osaki QTISPBVR update(2020/11/16) OpenGL版のISでは使用しているためifdefを無効にしました。
//#ifdef CS_MODE
public:
    std::string m_function_name;
//#endif
};

}
}

#endif    // PBVR__KVS__FREQUENCY_TABLE_H_INCLUDE

