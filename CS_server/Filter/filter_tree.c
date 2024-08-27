#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "filter.h"
#include "filter_log.h"

/*****************************************************************************/
/* 
 * 木構造の生成
 * 
 */
int
create_tree_struct( Int64       n_layer      ,
                    NodeInf    min          ,
                    NodeInf    max          ,
                    Int64*      tree_node_no ,
                    HierCell** tree         )
{
  int state = RTC_NG;
  
  Int64 curr = 0;
  
  int *stack_val = NULL;
  int  parent_id = 0;
  int  child_id = 0;
  
  float xx = 0.0;
  float yy = 0.0;
  float zz = 0.0;
  
  Int64 i, j, k;
  
  /* 木構造、総ノード数を計算 */
  *tree_node_no = 1;
  curr = 1;
  for (i=0; i<n_layer; i++)
  {
    curr = curr * 8;
    *tree_node_no = *tree_node_no + curr;
  }
  
  /* 領域確保 */
  *tree = (HierCell *)malloc(sizeof(HierCell)*(*tree_node_no));
  if (NULL == *tree) {
    filter_log(FILTER_ERR, "can not allocate memory(tree).\n");
    goto garbage;
  }
  
  stack_val = (int *)malloc(sizeof(int)*(*tree_node_no)*8+1);
  if (NULL == stack_val) 
  {
    filter_log(FILTER_ERR, "can not allocate memory(stack_val).\n");
    goto garbage;
  }
  
  /* 木構造の作成 */
  i = 0;
  (*tree)[i].parent = -1;
  (*tree)[i].max.x = max.x;
  (*tree)[i].min.x = min.x;
  (*tree)[i].max.y = max.y;
  (*tree)[i].min.y = min.y;
  (*tree)[i].max.z = max.z;
  (*tree)[i].min.z = min.z;
  j = 1;
  stack_val[0] = j; /* stack_val number */
  stack_val[j] = i; /* stack_val ID     */
  k = 1;
  i++;

  for (; i<*tree_node_no; ) 
  {
    parent_id = stack_val[k];
    (*tree)[parent_id].child[0] = 8;
    
    for (child_id = 0; child_id < 8; child_id++) 
    {
      
      (*tree)[stack_val[k]].child[child_id+1] = i;
      (*tree)[i].parent = stack_val[k];
      
      xx = ((*tree)[parent_id].max.x + (*tree)[parent_id].min.x)/2.0;
      yy = ((*tree)[parent_id].max.y + (*tree)[parent_id].min.y)/2.0;
      zz = ((*tree)[parent_id].max.z + (*tree)[parent_id].min.z)/2.0;
      
      if (child_id < 4) 
      {
        (*tree)[i].min.z = (*tree)[parent_id].min.z;
        (*tree)[i].max.z = zz;
      }
      else
      {
        (*tree)[i].min.z = zz;
        (*tree)[i].max.z = (*tree)[parent_id].max.z;
      }
      
      if (child_id >= 2 && child_id <= 5) 
      {
        (*tree)[i].min.y = yy;
        (*tree)[i].max.y = (*tree)[parent_id].max.y;
      }
      else
      {
        (*tree)[i].min.y = (*tree)[parent_id].min.y;
        (*tree)[i].max.y = yy;
      }
      
      if (child_id == 0 || child_id == 3 || child_id == 4 || child_id == 7) 
      {
        (*tree)[i].min.x = (*tree)[parent_id].min.x;
        (*tree)[i].max.x = xx;
      }
      else
      {
        (*tree)[i].min.x = xx;
        (*tree)[i].max.x = (*tree)[parent_id].max.x;
      }
      
      (*tree)[i].child[0] = 0;
      j++;
      stack_val[0]++;
      stack_val[j] = i;
      i++;
    }
    k++;
  }
  
  state = RTC_OK;
garbage:
  if (NULL != stack_val)
  {
    free(stack_val);
    stack_val = NULL;
  }
  return(state);
}
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
    filter_log(FILTER_ERR, "can not open tree_struct.damp .\n");
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
    filter_log(FILTER_ERR, "can not open tree_struct.damp .\n");
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
/*****************************************************************************/
