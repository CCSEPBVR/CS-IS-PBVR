#ifndef PBVR__KVS__VISCLIENT__PBVR_RESULT_H_INCLUDE
#define PBVR__KVS__VISCLIENT__PBVR_RESULT_H_INCLUDE

#include "VariableRange.h"
#include "FrequencyTable.h"
#include <vector>

namespace kvs
{
namespace visclient
{

class ReceivedMessage
{
public:
    VariableRange m_var_range;
    size_t m_max_merged_time_step;
    size_t m_min_merged_time_step;

    std::vector<kvs::visclient::FrequencyTable> m_color_bins, m_opacity_bins;
    int32_t m_subpixel_level;

public:
    const kvs::visclient::FrequencyTable* findColorFrequencyTable(const std::string &name)
    {
        std::vector<kvs::visclient::FrequencyTable>::const_iterator itr;
        for (itr=m_color_bins.begin(); itr!=m_color_bins.end(); itr++) {
            const kvs::visclient::FrequencyTable *table = &(*itr);
            if (table->m_function_name == name) {
                return table;
            }
        }

        return NULL;
    }

    const kvs::visclient::FrequencyTable* findOpacityFrequencyTable(const std::string &name)
    {
        std::vector<kvs::visclient::FrequencyTable>::const_iterator itr;
        for (itr=m_opacity_bins.begin(); itr!=m_opacity_bins.end(); itr++) {
            const kvs::visclient::FrequencyTable *table = &(*itr);
            if (table->m_function_name == name) {
                return table;
            }
        }

        return NULL;
    }
};

}
}

#endif

