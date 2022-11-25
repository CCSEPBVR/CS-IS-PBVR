/******************************************************************************

                Copyright (C) 2013 by

            JAEA All Rights Reserved.

    $Author:                 $
    $Date:                   $
    $Revision:               $
    $Source:                 $

 *****************************************************************************/

#ifndef FILTER_H_
#define FILTER_H_

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <functional>
#ifdef MPI_EFFECT
#include <mpi.h>
#endif

#ifdef _OPENMP
#include "omp.h"
#endif

#include "FilterTypedef.h"

namespace pbvr
{
namespace filter
{

/*****************************************************************************/
/* 返却値 */
typedef enum {
    RTC_OK = 0, RTC_NG = 255
} RTC;
/* ファイルパス文字列長MAX */
#define FILELEN_MAX 1024

#define LINELEN_MAX 512

/*****************************************************************************/

/* 処理対象データ種別 */
#define DTYPE_MUSICGEO  0      /* MUSIC geo/out   */
#define DTYPE_AVSUCDBIN 1      /* AVS UCDバイナリ */

/* 成分名　単位文字列長 */
#define  COMPONENT_LEN 16

//#define NLAYER_MAX      16     /* octree階層数MAX？        */
#define NLAYER_MAX       8     /* octree階層数MAX？        */
#define NDATA_LABEL_LEN 16     /* データラベル最大長       */
#define MAX_ELEMENT_TYPE 16

/*****************************************************************************/

/* パラメータ群 */
static const size_t hexdiv[5][4] = {
    {1, 3, 0, 4},
    {4, 6, 5, 1},
    {4, 7, 6, 3},
    {1, 2, 3, 6},
    {1, 3, 4, 6}
}; // hex分割用インデックス


static const int ucd_elementsize[] = {
    1, /*  0: point  */
    2, /*  1: line   */
    3, /*  2: tri    */
    4, /*  3: quad   */
    4, /*  4: tetra  */
    5, /*  5: pyr    */
    6, /*  6: prism  */
    8, /*  7: hex    */
    3, /*  8: line2  */
    6, /*  9: tri2   */
    8, /* 10: quad2  */
    10, /* 11: tetra2 */
    13, /* 12: pyr2   */
    15, /* 13: prism2 */
    20 /* 14: hex2   */
};

static const int vtk_elementsize[] = {
    0, /* 0: NON_EXISTENT */
    1, /*  1: VTK_VERTEX */
    0, /*  2: VTK_POLY_VERTEX, size unknown at compile time */
    2, /*  3: VTK_LINE    */
    0, /*  4: VTK_POLY_LINE, size unknown at compile time */
    3, /*  5: VTK_TRIANGLE    */
    0, /*  6: VTK_TRIANGLE_STRIP  ,size unknown at compile time*/
    0, /*  7: VTK_POLYGON   ,size unknown at compile time */
    4, /*  8: VTK_PIXEL  */
    4, /*  9: VTK_QUAD   */
    4, /* 10: VTK_TETRA  */
    8, /* 11: VTK_VOXEL */
    8, /* 12: VTK_HEXAHEDRON   */
    6, /* 13: VTK_WEDGE */
    5, /* 14: VTK_PYRAMID   */
    3, /* 11: VTK_Q_EDGE */
    6, /* 12: VTK_Q_TRIANGLE   */
    8, /* 13: VTK_Q_QUAD */
    10, /* 14: VTK_Q_TETRA   */
    20, /* 14: VTK_Q_HEXAHEDRON   */
};

enum InputTypes
{
    AVS_INPUT = 0,
    STL_INPUT = 1,
    STL_ASCII = 2,
    STL_BIN   = 3,
    PLOT3D_INPUT = 4,
    VTK_INPUT = 5
};

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
} HierCell;

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

typedef struct Fdetail
{
  char  fname[LINELEN_MAX];
  char  ftype[16];
  int   skip;
  int   offset;
  Int64  stride;
  char  close;
} Fdetail;

typedef struct Stepfile
{
  Fdetail *coord;
  Fdetail *value;
} Stepfile;

enum DIVISION_TYPE
{
    UNKNOWN_DIVISION = -1,		///< 不明
    MPI_DIVISION = 0,			///< 分割数による分割
    MASTER_DIVISION = 1,		///< ランク０をマスターとして、分割数による分割（デフォルト）
    PARAM_DIVISION = 2,			///< フィルタパラメータファイル記述（division_type,n_division）に従う：デバッグ用
    OCTREE_DIVISION  = 8		///< ８分木分割：未使用
};

typedef enum {
    PBVR_VTK_UNKNOWN = -1,
    PBVR_VTK_STRUCTURED_POINTS = 0,
    PBVR_VTK_RECTILINEAR_GRID,
    PBVR_VTK_STRUCTURED_GRID,
    PBVR_VTK_POLYDATA,
    PBVR_VTK_UNSTRUCTURED_GRID,
} VtkDatasetTypes;

class FilterParam {
public:
    FilterParam();
    FilterParam(const FilterParam &src);
    virtual ~FilterParam();

    char in_dir [LINELEN_MAX]; /* ファイル接頭辞       */
    char in_prefix [LINELEN_MAX]; /* ファイル接頭辞       */
    char in_suffix [LINELEN_MAX]; /* ファイル接尾辞       */
    char out_dir [LINELEN_MAX]; /* ファイル接頭辞       */
    char out_prefix[LINELEN_MAX]; /* ファイル接頭辞       */
    char out_suffix[LINELEN_MAX]; /* ファイル接尾辞       */

    char field_file[LINELEN_MAX]; /* fld ファイル名       */
    char geom_file [LINELEN_MAX]; /* geomファイル名       */
    char format [LINELEN_MAX]; /* 時刻ステップ書式     */
    char ucd_inp [LINELEN_MAX]; /* inp ファイル名       */
    char tmp_dir [LINELEN_MAX]; /* 一次ディレクトリ     */

    Int64 start_step; /* 時刻ステップ開始番号 */
    Int64 end_step; /* 時刻ステップ終了番号 */
    Int64 n_layer; /* 階層数               */
    Int64 elem_ave; /* 階層数               */
    Int64 component; /*                      */
    Int64 out_num; /* 出力ファイル数       */
    char file_type; /* 出力形式　　         */

    float null_val; /* NULL値               */

    Int64 nstep; /* fldファイルから      */
    Int64 ndim; /* fldファイルから      */
    Int64 dim1; /* fldファイルから      */
    Int64 dim2; /* fldファイルから      */
    Int64 dim3; /* fldファイルから      */
    Int64 nspace; /* fldファイルから      */
    Int64 veclen; /* fldファイルから      */
    char data[16]; /* fldファイルから      */
    char field[16]; /* fldファイルから      */
    char coordfile[3][LINELEN_MAX]; /* COORD 1 FILE NAME    */
    char coordtype[3][LINELEN_MAX]; /* COORD 1 FILE NAME    */
    Int64 coordskip[3]; /* fldファイルから      */
    Int64 coordstride[3]; /* fldファイルから      */
    char valuefile[LINELEN_MAX]; /* VALUE 1 FILE NAME    */
    char valuetype[LINELEN_MAX]; /* VALUE 1 FILE NAME    */
    Int64 valueskip; /* fldファイルから      */

    char avsucd; /* AVSUCD形式判定フラグ */

    char struct_grid; /* 構造格子判定         */
    char explicit_n_layer; /* 階層指示ありなし判定 */
    char nodesearch; /* 節点番号検索判定フラグ       */
    char headfoot; /* ヘッダー、フッター判定フラグ */
    char mult_element_type; /*                  */
    char temp_delete; /*                  */
    // 2014.11
    int mpi_volume_div; /* 並列分割軸            */
    int mpi_step_div; /* 並列分割軸            */
    char mpi_div; /* 並列分割軸            */
    // 2014.11
    Int64 blocksize;
    float min_ext[3]; /* fldファイルから      */
    float max_ext[3]; /* fldファイルから      */

    Stepfile *stepf;
    // V.I.C addons 2015.10
    int input_format;
    char plot3d_config [LINELEN_MAX]; /* PLOT3D configuration file      */
    int datasettype;       /* vtk dataset type */
    char m_ucdbin;         /* add by @hira at 2016/12/01 */
    bool m_iswrite;
    char m_paramfile [LINELEN_MAX]; /* パラメータファイル       */
    DIVISION_TYPE division_type;		// 分割方法 : 0=MPIプロセス数分割, 1=n_division(パラメータファイル), 2=n_layer（８分木）
    int n_division;			// 分割数：division_type=1の時有効
};


class PbvrFilterInfo {
public:
    PbvrFilterInfo();
    PbvrFilterInfo(const PbvrFilterInfo& src);
    virtual ~PbvrFilterInfo();
    Int64 getTotalSubVolumeNodes() const;
    Int64 getTotalSubVolumeConns() const;
    Int64 getTotalSubVolumeElems() const;
    Int64 getSubVolumeNodeCount();
    Int64 getSubVolumeConnCount();
    Int64 getSubVolumeElemCount();
    Int64 getNodePosition(int subvolume) const;
    Int64 getScatterCounts(int num_rank, int* counts) const;
    int* getScatterPositions(int num_rank, int* displs) const;
    Int64 getNodeCount();
    int scatterValue();
    void copyInfos(const PbvrFilterInfo &src);

public:
    const int* m_elementsize;
    //enum ElementType m_element_type;        ///< Element type.
    char m_element_type;
    Int64 m_tree_node_no;
    HierCell *m_tree;

    char filename[LINELEN_MAX];
    char buff[LINELEN_MAX];

    int m_nsteps;
    int m_nvolumes;

    char m_title[70];
    int m_stepn;
    float m_stept;
    Int64 m_nnodes;
    Int64 m_nelements;
    int m_elemcoms;
    Int64 m_connectsize;
    int m_ncomponents;
    int m_nnodecomponents;
    int m_nelemcomponents;
    int m_nkinds;
    int m_nnodekinds;
    int m_nelemkinds;
    int m_desctype;
    int m_datatype;
    int m_component_id;
    Int64 m_nvalues_per_node;
    Int64 m_ncomponents_per_node;
    int *m_ids;
    float *m_coords;
    int *m_elementids;
    int *m_materials;
    char *m_elemtypes;
    int *m_connections;
    int *m_connections_o;
    int *m_conn_top_node;
    float *m_values;
    char *m_names;
    char *m_units;
    char **pm_names;
    char **pm_units;
    int *m_veclens;
    int *m_elemveclens;
    int *m_nullflags;
    float *m_nulldatas;
    unsigned int *m_elems_per_subvolume;
    unsigned int *m_conns_per_subvolume;
    unsigned int *m_nodes_per_subvolume;
    unsigned int *m_elems_accumulate;
    unsigned int *m_conns_accumulate;
    unsigned int *m_nodes_accumulate;
    int *m_subvolume_num;
    char *m_nodeflag;
    int *m_nodes_trans;
    int *m_octree;
    int *m_subvolume_nodes;
    int *m_subvolume_trans;
    int *m_subvolume_elems;

    float *m_subvolume_coord;
    int *m_subvolume_connect;
    float *m_subvolume_value;
    float *m_value_max;
    float *m_value_min;
    int *mt_nullflag;
    float *mt_nulldata;
    Int64 m_subvolume_coord_size;
    Int64 m_subvolume_connect_size;

    int mype;
    int numpe;
    int mype_f;
    int numpe_f;
    int mype_s;
    int numpe_s;
    int mype_v;
    int numpe_v;
    int mpidiv_s;
    int mpidiv_v;

    int *m_elem_in_node;
    int *m_elem_in_node_addr;
    int *m_elem_in_node_array;
    Int64 m_elem_in_node_array_size;

    int *m_vol_sta;
    int *m_vol_end;
    int *m_vol_num;

    FilterParam *m_param;
    char ENDIAN;

    Int64 prov_len, prov_len_e;

    unsigned int **m_element_array;

    int pre_step;
    unsigned int m_types[MAX_ELEMENT_TYPE];
    Int64 m_filesize;
    char m_headfoot;
    char m_multi_element;

    char m_old_avs_format;
    char m_cycle_type;
    char m_ucdbin;
    char **m_ucdbinfile;
    int m_ucdbinfile_size;

    FILE **m_value_fp;
    FILE **m_geom_fp;
    FILE *m_ucd_ascii_fp;

    int current_block;
    int number_of_blocks;
    Int64 *offset_values;
#ifdef DBG
    FILE *PeFp;
    char LogFile[LINELEN_MAX];
#endif

    NodeInf node_max;      // add by @hira at 2016/12/01 : filter_convert.hのmaxから移動
    NodeInf node_min;      // add by @hira at 2016/12/01 : filter_convert.hのminから移動

    // connect vector   add by @hira  at 2016/12/01
    std::vector<int> m_pbvr_connections;      // ノードの接続情報:subvolumeすべて
    std::vector<int> m_pbvr_connect_count;    // ノードの接続情報のsubvolume毎のm_pbvr_connectionsの挿入数
    std::vector<int> m_pbvr_subvolumeids;     // ランクが担当したサブボリュームID(0〜)
    std::vector<int> m_pbvr_plot3d_offsetvalue;
    Int64 m_values_size;

    // value  vector   add by @hira  at 2016/12/01
    std::vector<float> m_pbvr_values;    // ノードの物理量
    int element_type_num;
    NodeInf m_min_object_coord;
    NodeInf m_max_object_coord;
    unsigned int *m_elems_per_multivolume;
    unsigned int *m_conns_per_multivolume;
    unsigned int *m_nodes_per_multivolume;
    int *m_multivolume_nodes;
    int step_count;
};

class FilterConvert;

class PbvrFilter {
public:
    PbvrFilter();
    virtual ~PbvrFilter();
    int loadPbvrFilter(const char* filename);
    int readStepValue(int step_loop, int n_subvolume, int read_elem_type);
    void setIsWrite(bool iswrite);
    void setIsReleaseInfo(bool isrelease);
    bool isWrite();
    bool isReleaseInfo(bool isrelease);
    PbvrFilterInfo * getPbvrFilterInfo() const;
    void setPbvrFilterInfo(PbvrFilterInfo *info);
    void deletePbvrFilterInfo();
    int count_cellTypes(const char* filename);
    void setFilterDivision(DIVISION_TYPE division_type, int n_division);
    int loadMpiSubvolume(int step_loop, int read_elem_type);
    void releaseMpiSubvolume() const;
    bool isWriteMpiProcess(int n_subvolume) const;
    std::vector<PbvrFilterInfo*>& getPbvrFilterList();
    int setPbvrFilterList(PbvrFilterInfo* info);
    void finalize();

protected:
    FilterConvert* factoryFilterConvert(int input_format, PbvrFilterInfo *filter_info);
    int readValue(int step_loop, int read_elem_type);

private:
    PbvrFilterInfo *m_filter_info;
    std::vector<PbvrFilterInfo*> m_filter_list;
    bool m_is_releaseinfo;
};

} // end of namespace filter
} // end of namespace pbvr


#endif /* FILTER_H_ */
