#ifndef FILTER_OCTREE_H
#define	FILTER_OCTREE_H
#include "filter_utils.h"

//#define NLAYER_MAX      16     /* octree階層数MAX？        */
#define NLAYER_MAX       8     /* octree階層数MAX？        */
#define NDATA_LABEL_LEN 16     /* データラベル最大長       */

#define MAX_ELEMENT_TYPE 16

/* 座標 */
typedef struct NodeInf
{
  float x;
  float y;
  float z;
} NodeInf;

/* 階層 */
typedef struct HierCell
{
  NodeInf max;
  NodeInf min;
  unsigned char null_flag;
  int child[9];
  int parent;
  int dim1;
  int dim2;
  int dim3;
}HierCell;

/* 節点情報 */
typedef struct NodeDataInf
{
  NodeInf p;
  int octree[NLAYER_MAX+1];
} NodeDataInf;

/* 節点データ */
typedef struct NodeData
{
  Int64   num_nodes;
  int    veclen;
  char   label[NDATA_LABEL_LEN];
  float *val;
} NodeDataVal;

int create_tree_struct( Int64       ,
                        NodeInf    ,
                        NodeInf    ,
                        Int64*      ,
                        HierCell** );
int write_tree_struct( FilterParam* ,
                       Int64         ,
                       HierCell*    ,
                       Int64         );
int write_tree_data( FilterParam* ,
                     Int64         ,
                     Int64         ,
                     NodeDataVal* );
#endif	/* FILTER_OCTREE_H */

