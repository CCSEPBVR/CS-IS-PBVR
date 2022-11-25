#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "filter_tree.h"
#include "filter_log.h"

namespace pbvr
{
namespace filter
{

/*****************************************************************************/
/*
 * 木構造を出力する
 *
 */
int
write_tree_struct( FilterParam* param        ,
                   Int64         tree_node_no ,
                   HierCell*    tree         ,
                   Int64         nnode_data   )
{
  int state = RTC_NG;

  FILE *fp = NULL;
  Int64  i;

  /* オープン */
  fp = fopen("./tree_struct.damp", "w");
  if (NULL == fp)
  {
    LOGOUT(FILTER_ERR, (char*)"can not open tree_struct.damp .\n");
    goto garbage;
  }

  /* 書き込み */
  fprintf(fp, "%lld\n", param->n_layer);
  fprintf(fp, "%lld\n", param->start_step);
  fprintf(fp, "%lld\n", param->end_step);
  fprintf(fp, "%lld\n", nnode_data);

  for (i=0; i<tree_node_no; i++)
  {
    fprintf(fp, "%d %e %e %e %e %e %e %d\n",
      tree[i].parent,
      tree[i].min.x, tree[i].min.y, tree[i].min.z,
      tree[i].max.x, tree[i].max.y, tree[i].max.z,
      tree[i].null_flag);
  }

  state = RTC_OK;
garbage:
  if (NULL != fp) fclose(fp);
  return(state);
}
/*****************************************************************************/
/*
 * 木構造のデータを出力する
 *
 */
int
write_tree_data( FilterParam* param        ,
                 Int64         tree_node_no ,
                 Int64         nnode_data   ,
                 NodeDataVal* tree_data    )
{
  int state = RTC_NG;

  FILE *fp = NULL;
  Int64 i, j;

  /* オープン(追記) */
  fp = fopen("./tree_struct.damp", "a");
  if (NULL == fp)
  {
    LOGOUT(FILTER_ERR, (char*)"can not open tree_struct.damp .\n");
    goto garbage;
  }

  /* ラベルの書き込み */
  for (i=0; i<nnode_data; i++)
  {
    fprintf(fp, "%s\n", tree_data[2*i].label);
  }

  /* データの書き込み */
  for (j=0; j<tree_node_no; j++)
  {
    for (i=0; i<nnode_data; i++)
    {
      fprintf(fp, "%e ", tree_data[2*i].val[j]);
      fprintf(fp, "%e ", tree_data[2*i+1].val[j]);
    }
    fprintf(fp, "\n");
  }

  state = RTC_OK;
garbage:
  if (NULL != fp)
  {
    fclose(fp);
  }
  return(state);
}


} // end of namespace filter
} // end of namespace pbvr

/*****************************************************************************/
