/*
 * FilterDivisionOctree.h
 *
 *  Created on: 2016/12/24
 *      Author: hira
 */

#ifndef FILTERDIVISIONOCTREE_H_
#define FILTERDIVISIONOCTREE_H_

#include "FilterDivision.h"

namespace pbvr {
namespace filter {

class FilterDivisionOctree: public FilterDivision {
public:
    FilterDivisionOctree();
    FilterDivisionOctree(PbvrFilterInfo *filter_info);
    virtual ~FilterDivisionOctree();
    virtual int distribution_structure();
    virtual int distribution_structure_2d();
    virtual int distribution_unstructure();
    virtual int get_nvolumes();
    virtual int create_tree_struct(NodeInf node_min, NodeInf node_max);

private:
    int create_tree_struct(
                    Int64       n_layer      ,
                    NodeInf    min          ,
                    NodeInf    max          ,
                    Int64*      tree_node_no ,
                    HierCell** tree         );
};

} /* namespace filter */
} /* namespace pbvr */

#endif /* FILTERDIVISIONOCTREE_H_ */
