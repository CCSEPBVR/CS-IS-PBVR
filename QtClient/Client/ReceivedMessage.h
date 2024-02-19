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
//#ifdef COMM_MODE_IS
    float maxObjectCoord[3];
    float minObjectCoord[3];
    bool isUnderAnimation;
	int32_t minServerTimeStep;
    int32_t maxServerTimeStep;
    int32_t serverTimeStep=-1;
//NEW
    int32_t numParticle;
    int32_t numVolDiv;
    int32_t staStep;
    int32_t endStep;
    int32_t numStep;
    float minValue;
    float maxValue;
    int32_t numNodes;
    int32_t numElements;
    int32_t elemType;
    int32_t fileType;
    int32_t numIngredients;
    int32_t flag_send_bins;
    int32_t tf_count;
//#endif
    std::vector<kvs::visclient::FrequencyTable> m_color_bins, m_opacity_bins;
    int32_t m_subpixel_level;

    //add by shimomura 20240118
    float arrow_component[3];
    bool arrow_flag;
    float m_glyph_xform[16];
    float m_obj_xform[16];

public:
    //Osaki QTISPBVR update(2020/11/16) OpenGL版のISでは使用しているためifdefを無効にしました。
    const kvs::visclient::FrequencyTable* findColorFrequencyTable(const std::string &name)
    {
//#ifdef COMM_MODE_CS
        std::vector<kvs::visclient::FrequencyTable>::const_iterator itr;
        for (itr=m_color_bins.begin(); itr!=m_color_bins.end(); itr++) {
            const kvs::visclient::FrequencyTable *table = &(*itr);
            if (table->m_function_name == name) {
                return table;
            }
        }
//#endif;
        return NULL;
    }
    //Osaki QTISPBVR update(2020/11/16) OpenGL版のISでは使用しているためifdefを無効にしました。
    const kvs::visclient::FrequencyTable* findOpacityFrequencyTable(const std::string &name)
    {
//#ifdef COMM_MODE_CS
        std::vector<kvs::visclient::FrequencyTable>::const_iterator itr;
        for (itr=m_opacity_bins.begin(); itr!=m_opacity_bins.end(); itr++) {
            const kvs::visclient::FrequencyTable *table = &(*itr);
            if (table->m_function_name == name) {
                return table;
            }
        }
//#endif
        return NULL;
    }
};

}
}

#endif

