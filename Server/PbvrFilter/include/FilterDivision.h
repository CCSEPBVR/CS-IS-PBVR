/*
 * FilterDivision.h
 *
 *  Created on: 2016/12/24
 *      Author: hira
 */

#ifndef FILTERDIVISION_H_
#define FILTERDIVISION_H_

#include "PbvrFilter.h"

namespace pbvr
{
namespace filter
{

class FilterDivision {
public:
    FilterDivision();
    FilterDivision(PbvrFilterInfo *filter_info);
    virtual ~FilterDivision();
    DIVISION_TYPE getDivisionType() const;
    virtual int distribution_structure();
    virtual int distribution_structure_2d();
    virtual int distribution_unstructure();
    virtual int get_nvolumes();
    virtual int create_tree_struct(NodeInf node_min, NodeInf node_max);
    bool isWriteMpiProcess(int volm_loop, int rank = -1) const;
    bool getWriteMpiProcess(int volm_loop, int &rank, int &count) const;
    int distribution_subvolume(char struct_grid, int ndim);

protected:
    PbvrFilterInfo *m_filter_info;

private:
    unsigned int get_division_id(Int64 eid, Int64 m_nelements, Int64 m_nvolumes);

};

FilterDivision* factoryFilterDivision(DIVISION_TYPE division_type, PbvrFilterInfo *filter_info);

} /* namespace filter */
} /* namespace pbvr */

#endif /* FILTERDIVISION_H_ */
