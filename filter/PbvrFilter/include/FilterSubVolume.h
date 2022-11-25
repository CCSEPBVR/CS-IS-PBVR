/*
 * FilterSubVolume.h
 *
 *  Created on: 2016/12/24
 *      Author: hira
 */

#ifndef FILTERSUBVOLUME_H_
#define FILTERSUBVOLUME_H_

#include "PbvrFilter.h"

namespace pbvr {
namespace filter {

class FilterSubVolume {
public:
    FilterSubVolume();
    FilterSubVolume(PbvrFilterInfo *filter_info);
    virtual ~FilterSubVolume();
    int count_in_subvolume(int subvolume_no,
                Int64* subelemnum,
                Int64* subnodenum,
                Int64* subconnnum);

    int get_coords_minmax(int volume_id, Int64 rank_nodes, float min[3], float max[3]);

protected:
    PbvrFilterInfo *m_filter_info;

private:
    int realloc_work(Int64 new_len);

    Int64 rank_nodes;
    Int64 rank_elems;
};

} /* namespace filter */
} /* namespace pbvr */

#endif /* FILTERSUBVOLUME_H_ */
