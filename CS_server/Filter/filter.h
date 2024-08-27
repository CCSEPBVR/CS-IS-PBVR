#ifndef FILTER_H_
#define FILTER_H_
#include <stdio.h>
/******************************************************************************
 
			    Copyright (C) 2013 by
 
			JAEA All Rights Reserved.
                                       
    $Author:                 $
    $Date:                   $
    $Revision:               $
    $Source:                 $
 
 *****************************************************************************/
#ifdef MPI_EFFECT
#include "mpi.h"
#endif
/*****************************************************************************/
// MSVC needs some help 
#ifdef _MSC_VER
#define inline __inline
#define fseek _fseeki64
#define Int64 __int64
#else
//#define Int64 long int
#define Int64 long long
#endif

/*****************************************************************************/
/* 返却値 */
//#define RTC_NG -1
//#define RTC_OK  1
//#define RTC_OK  0

typedef enum {
    RTC_OK = 0, RTC_NG = 255
} RTC;
/* ファイルパス文字列長MAX */
#define FILELEN_MAX 1024

#include "filter_tree.h"
#include "filter_utils.h"

/*****************************************************************************/

/* 処理対象データ種別 */
#define DTYPE_MUSICGEO  0      /* MUSIC geo/out   */
#define DTYPE_AVSUCDBIN 1      /* AVS UCDバイナリ */

/* 成分名　単位文字列長 */
#define  COMPONENT_LEN 16
/*****************************************************************************/

/* パラメータ群 */
// These seem unused ??????
//enum ElementType
//{
//        Point              = 0, ///< Point.
//        Line,                   ///< Line.
//        Triangle,               ///< Triangle.
//        Quadrangle,             ///< Quadrangle.
//        Tetrahedra,             ///< Tetrahedra.
//        Pyramid,                ///< Pyramid.
//        Prism,                  ///< Prism.
//        Hexahedra,              ///< Hexahedra.
//        Line2,                  ///< Line.
//        Triangle2,              ///< Triangle2.
//        Quadrangle2,            ///< Quadrangle2.
//        Tetrahedra2,            ///< Quadratic tetrahedra.
//        Pyramid2,               ///< Pyramid.
//        Prism2,                 ///< Prism.
//        Hexahedra2,             ///< Quadratic hexahedra.
//        ElementTypeUnknown      ///< Unknown element type.
//};

//    static const size_t quaddiv[6] = {0,1,2,
//                                      0,2,3};  // quad分割用インデックス
//
//    static const size_t tetdiv[4] = {0,1,2,3};  // tetraインデックス
//
//    static const size_t pyrdiv[8] = {0,1,2,3,
//                                     0,3,4,1};  // pyramid分割用インデックス
//
//    static const size_t pridiv[12] = {0,3,4,5,
//                                      4,1,0,2,
//                                      2,4,5,0}; // prism分割用インデックス
//
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

const int* m_elementsize;


/*****************************************************************************/

void timer_init(void);
void timer_fin(void);
void timer_sta(int id);
void timer_end(int id);
void timer_bar(int id);
void timer_init_(void);
void timer_fin_(void);
void timer_sta_(int *id);
void timer_end_(int *id);
void timer_bar_(int *id);
/*****************************************************************************/

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
#ifdef MPI_EFFECT
MPI_Comm MPI_COMM_FILTER;
MPI_Comm MPI_COMM_STEP;
MPI_Comm MPI_COMM_VOLM;
MPI_Comm m_mpi_comm;
#endif
int *m_elem_in_node;
int *m_elem_in_node_addr;
int *m_elem_in_node_array;

int *m_vol_sta;
int *m_vol_end;
int *m_vol_num;

FilterParam Fparam;
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
/*****************************************************************************/
#endif /* FILTER_H_ */
