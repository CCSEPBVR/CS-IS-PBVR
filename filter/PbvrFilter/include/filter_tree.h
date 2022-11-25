#ifndef FILTER_OCTREE_H
#define	FILTER_OCTREE_H

#include "PbvrFilter.h"

namespace pbvr
{
namespace filter
{

int write_tree_struct( FilterParam* ,
                       Int64         ,
                       HierCell*    ,
                       Int64         );
int write_tree_data( FilterParam* ,
                     Int64         ,
                     Int64         ,
                     NodeDataVal* );

} // end of namespace filter
} // end of namespace pbvr

#endif	/* FILTER_OCTREE_H */

