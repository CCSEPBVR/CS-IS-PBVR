#ifndef FILTER_UTILS_H
#define	FILTER_UTILS_H
//#include "filter_io_avs/filter_io_avs.h"

#define LINELEN_MAX 512

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

enum InputTypes
{
    AVS_INPUT = 0,
    STL_INPUT = 1,
    STL_ASCII = 2,
    STL_BIN   = 3,
    PLOT3D_INPUT = 4,
    VTK_INPUT = 5
};

//typedef struct Stepfile Stepfile;

typedef struct FilterParam
{
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

} FilterParam;
int ConvertEndianI(int);
float ConvertEndianF(float);
char check_endian(void);
int set_element_in_node(void);

int create_hexahedron(void);
int create_tetrahedron(void);


int count_in_elements(int,
                      Int64 *,
                      Int64 *,
                      Int64 *,
                      Int64 *,
                      Int64,
                      Int64);
int read_param_file(char*);

int set_etc(void);
int distribution_structure(void);
int distribution_structure_2d(void);
int distribution_unstructure(void);
int count_in_subvolume(int,
                       Int64 *,
                       Int64 *,
                       Int64 *,
                       Int64 *,
                       Int64,
                       Int64);
int count_in_subvolume_b(int,
                         Int64 *,
                         Int64 *,
                         Int64 *);
int set_nodes_in_subvolume(Int64,
                           Int64);
int search_node_id(int,
                   int,
                   int**);

#endif	/* FILTER_UTILS_H */

