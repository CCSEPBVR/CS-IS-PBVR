#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <PbvrFilter.h>

#include "filter_log.h"
#include "filter_utils.h"

#if !defined(WIN32) && !defined(__APPLE__)
// add by @hira at 2016/12/01
#include <unistd.h>
#include <sys/resource.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include <malloc.h>
#endif

namespace pbvr
{
namespace filter
{

/*****************************************************************************/
int realloc_work(
        Int64 new_len,
        PbvrFilterInfo *filter_info)
{
    int *p;
    if (filter_info == NULL) return RTC_NG;

    p = (int *) realloc(filter_info->m_subvolume_nodes, sizeof (int)*new_len);
    if (NULL == p) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_nodes).\n");
        return (RTC_NG);
    }
    filter_info->m_subvolume_nodes = p;
    p = (int *) realloc(filter_info->m_subvolume_trans, sizeof (int)*new_len);
    if (NULL == p) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_trans).\n");
        return (RTC_NG);
    }
    filter_info->m_subvolume_trans = p;
    return (RTC_OK);
}

/*
 * パラメータの読み込み
 *
 */
int read_param_file(const char* filename, FilterParam* param) {
    DEBUG(3,"### Reading ParameterFile: %s \n", filename);
    int state = RTC_NG;
    FILE *fp;
    int n;
    char line[LINELEN_MAX];
    char work[LINELEN_MAX];
    char *ch;
    char flag0 = 0;
    char flag1 = 0;

    memset(param->in_dir, 0x00, LINELEN_MAX);
    memset(param->in_prefix, 0x00, LINELEN_MAX);
    memset(param->in_suffix, 0x00, LINELEN_MAX);
    memset(param->field_file, 0x00, LINELEN_MAX);
    memset(param->geom_file, 0x00, LINELEN_MAX);
    memset(param->out_dir, 0x00, LINELEN_MAX);
    memset(param->out_prefix, 0x00, LINELEN_MAX);
    memset(param->out_suffix, 0x00, LINELEN_MAX);
    memset(param->format, 0x00, LINELEN_MAX);
    memset(param->ucd_inp, 0x00, LINELEN_MAX);
    memset(param->tmp_dir, 0x00, LINELEN_MAX);
    memset(param->m_paramfile, 0x00, LINELEN_MAX);

    sprintf(param->in_dir, "%s", ".");
    sprintf(param->in_prefix, "%s", "\0");
    sprintf(param->in_suffix, "%s", "\0");
    sprintf(param->field_file, "%s", "\0");
    sprintf(param->geom_file, "%s", "\0");
    sprintf(param->out_dir, "%s", ".");
    sprintf(param->out_prefix, "%s", "\0");
    sprintf(param->out_suffix, "%s", "\0");
    sprintf(param->format, "%s", "%05d");
    sprintf(param->ucd_inp, "%s", "\0");
    sprintf(param->tmp_dir, "%s", "..");

    param->start_step = 0;
    param->end_step = 0;
    param->n_layer = 0;
    param->elem_ave = 0;
    param->out_num = 0;
    param->file_type = 0;
    param->component = -1;
    param->nodesearch = (char) 0;
    param->headfoot = (char) 0;
    param->avsucd = (char) 0;
    param->struct_grid = (char) 0;
    param->explicit_n_layer = (char) 0;
    param->mult_element_type = (char) 0;
    param->temp_delete = (char) 1;
    param->mpi_volume_div = -1;
    param->mpi_step_div = -1;
    param->mpi_div = (char) 2;
    param->blocksize = 1024000;
    if (param->division_type == UNKNOWN_DIVISION) {
        param->division_type = MASTER_DIVISION;
        param->n_division = 0;
    }

    // Added by V.I.C 2015.10 -->
    param->input_format = AVS_INPUT;
    memset(param->plot3d_config, 0x00, LINELEN_MAX);

    // <--
    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("can not open %s .\n", filename);
        return state;
    }

    while (fgets(line, LINELEN_MAX, fp) != NULL) {
        ch = line;
        if (*ch == '\n') {
            continue;
        }
        if (*ch == '#') {
            continue;
        }
        n = 0;
        while (n < LINELEN_MAX) {
            work[n] = '\0';
            n++;
        }
        ch = strchr(line, '=') + 1;
        n = 0;
        while (*ch != '\n' && *ch != '\r') {
            if (*ch != ' ') {
                work[n] = *ch;
                n++;
            }
            ch++;
        }
        //////////////////// VIC Changes  2015.10 //////////////////////////////
        // We are going to have to reconsider this, when adding  more formats
        DEBUG(3,"config_reader entering modified area\n");
        DEBUG(3,"line:%s", line);
        DEBUG(3,"work:%s\n", work);
        if (strstr(line, "stl_ascii_file")) {
            DEBUG(3,"ASCII FILE\n");
            sprintf(param->geom_file, "%s", work);
            param->input_format = STL_ASCII;
        } else if (strstr(line, "stl_bin_file")) {
            DEBUG(3,"BIN FILE\n");
            sprintf(param->geom_file, "%s", work);
            param->input_format = STL_BIN;
        } else if (strstr(line, "plot3d_config_file")) {
            DEBUG(3,"PLOT3D CONFIG  FILE:%s:\n", work);
            sprintf(param->plot3d_config, "%s", work);
            param->input_format = PLOT3D_INPUT;
        } else if (strstr(line, "vtk_file")) {
            DEBUG(3,"VTK FILE\n");
            sprintf(param->field_file, "%s", work);
            param->input_format = VTK_INPUT;
        } else if (strstr(line, "vtk_in_prefix")) {
            DEBUG(3,"VTK FILE PREFIX\n");
            sprintf(param->in_prefix, "%s", work);
            param->input_format = VTK_INPUT;
        } else if (strstr(line, "vtk_in_suffix")) {
            DEBUG(3,"VTK FILE SUFFIX\n");
            sprintf(param->in_suffix, "%s", work);
            param->input_format = VTK_INPUT;
        }/////////////////////////   to here ////////////////////////////////////
        else if (strstr(line, "in_prefix")) {
            sprintf(param->in_prefix, "%s", work);
        } else if (strstr(line, "in_suffix")) {
            sprintf(param->in_suffix, "%s", work);
        } else if (strstr(line, "out_prefix")) {
            n = strlen(work);
            if (work[n - 1] == '.') work[n - 1] = '\0';
            sprintf(param->out_prefix, "%s", work);
        } else if (strstr(line, "out_suffix")) {
            if (work[0] == '.') strcpy(work, &work[1]);
            sprintf(param->out_suffix, "%s", work);
        } else if (strstr(line, "format")) {
            //      if (NULL == strchr(line,'%') || NULL == strchr(line,'d')){
            //         filter_log(FILTER_ERR,"input parameter (format) error.\n");
            //         return(RTC_NG);
            //      }
            sprintf(param->format, "%s", work);
        } else if (strstr(line, "in_dir")) {
            if (n > 0) {
                sprintf(param->in_dir, "%s", work);
            }
        } else if (strstr(line, "out_dir")) {
            if (n > 0) {
                sprintf(param->out_dir, "%s", work);
            }
        } else if (strstr(line, "field_file")) {
            sprintf(param->field_file, "%s", work);
            flag0 = 1;
        } else if (strstr(line, "geom_file")) {
            sprintf(param->geom_file, "%s", work);
            flag1 = 1;
        } else if (strstr(line, "ucd_inp")) {
            sprintf(param->ucd_inp, "%s", work);
            param->m_ucdbin = 0;
        } else if (strstr(line, "start_step")) {
            ch = strchr(line, '=') + 1;
            param->start_step = atoi(ch);
        } else if (strstr(line, "end_step")) {
            ch = strchr(line, '=') + 1;
            param->end_step = atoi(ch);
        } else if (strstr(line, "n_layer") || strstr(line, "layer_number")) {
            ch = strchr(line, '=') + 1;
            param->n_layer = atoi(ch);
            param->explicit_n_layer = 1;
        } else if (strstr(line, "element_average") || strstr(line, "ave_element")) {
            ch = strchr(line, '=') + 1;
            param->elem_ave = atoi(ch);
        } else if (strstr(line, "file_number") || strstr(line, "output_number")) {
            ch = strchr(line, '=') + 1;
            param->out_num = atoi(ch);
        } else if (strstr(line, "file_type") || strstr(line, "output_type")) {
            // delete by @hira at 2017/04/20
            // ch = strchr(line, '=') + 1;
            // param->file_type = atoi(ch);
        } else if (strstr(line, "component")) {
            ch = strchr(line, '=') + 1;
            param->component = atoi(ch);
        } else if (strstr(line, "nodesearch")) {
            ch = strchr(line, '=') + 1;
            param->nodesearch = atoi(ch);
        } else if (strstr(line, "headfoot")) {
            ch = strchr(line, '=') + 1;
            param->headfoot = atoi(ch);
        } else if (strstr(line, "avsucd")) {
            ch = strchr(line, '=') + 1;
            param->avsucd = atoi(ch);
        } else if (strstr(line, "mpi_volume_div")) {
#ifdef MPI_EFFECT
            ch = strchr(line, '=') + 1;
            param->mpi_volume_div = atoi(ch);
#else
            LOGOUT(FILTER_LOG, (char*)"paramter (mpi_volume_div) is ignored.\n");
#endif
        } else if (strstr(line, "mpi_step_div")) {
#ifdef MPI_EFFECT
            ch = strchr(line, '=') + 1;
            param->mpi_step_div = atoi(ch);
#else
            LOGOUT(FILTER_LOG, (char*)"paramter (mpi_step_div) is ignored.\n");
#endif
        } else if (strstr(line, "mpi_div")) {
#ifdef MPI_EFFECT
            ch = strchr(line, '=') + 1;
            param->mpi_div = atoi(ch);
#else
            LOGOUT(FILTER_LOG, (char*)"paramter (mpi_div) is ignored.\n");
#endif
        } else if (strstr(line, "multi_element_type")) {
            ch = strchr(line, '=') + 1;
            param->mult_element_type = atoi(ch);
        } else if (strstr(line, "temp_delete")) {
            ch = strchr(line, '=') + 1;
            param->temp_delete = atoi(ch);
        } else if (strstr(line, "blocksize")) {
            ch = strchr(line, '=') + 1;
            param->blocksize = atoi(ch);
        } else if (param->division_type == UNKNOWN_DIVISION
                    || param->division_type == PARAM_DIVISION) {
            if (strstr(line, "division_type")) {
                ch = strchr(line, '=') + 1;
                param->division_type = (DIVISION_TYPE)atoi(ch);
            } else if (strstr(line, "n_division")) {
                ch = strchr(line, '=') + 1;
                param->n_division = atoi(ch);
            }
        } else {
            ;
        }
    }

    fclose(fp);

    strcpy(param->m_paramfile, filename);

#ifdef KEI
    sprintf(param->tmp_dir, "..");
#else
    strcpy(param->tmp_dir, param->out_dir);
#endif
    LOGOUT(FILTER_LOG, (char*)"NEW:param->input_format     : %i\n", param->input_format);
    LOGOUT(FILTER_LOG, (char*)"param->in_dir     : %s\n", param->in_dir);
    LOGOUT(FILTER_LOG, (char*)"param->in_prefix  : %s\n", param->in_prefix);
    LOGOUT(FILTER_LOG, (char*)"param->in_suffix  : %s\n", param->in_suffix);
    LOGOUT(FILTER_LOG, (char*)"param->out_dir    : %s\n", param->out_dir);
    LOGOUT(FILTER_LOG, (char*)"param->out_prefix : %s\n", param->out_prefix);
    LOGOUT(FILTER_LOG, (char*)"param->out_suffix : %s\n", param->out_suffix);
    LOGOUT(FILTER_LOG, (char*)"param->tmp_dir    : %s\n", param->tmp_dir);
    LOGOUT(FILTER_LOG, (char*)"param->format     : %s\n", param->format);
    LOGOUT(FILTER_LOG, (char*)"param->start_step : %d\n", param->start_step);
    LOGOUT(FILTER_LOG, (char*)"param->end_step   : %d\n", param->end_step);
    LOGOUT(FILTER_LOG, (char*)"param->n_layer    : %d\n", param->n_layer);
    LOGOUT(FILTER_LOG, (char*)"param->elem_ave   : %d\n", param->elem_ave);
    LOGOUT(FILTER_LOG, (char*)"param->file_type  : %d\n", param->file_type);
    LOGOUT(FILTER_LOG, (char*)"param->out_num    : %d\n", param->out_num);
    LOGOUT(FILTER_LOG, (char*)"param->field_file : %s\n", param->field_file);
    LOGOUT(FILTER_LOG, (char*)"param->ucd_inp    : %s\n", param->ucd_inp);
    LOGOUT(FILTER_LOG, (char*)"param->geom_file  : %s\n", param->geom_file);
    LOGOUT(FILTER_LOG, (char*)"param->avsucd     : %d\n", param->avsucd);
    LOGOUT(FILTER_LOG, (char*)"param->mpi_div          : %d\n", param->mpi_div);
    LOGOUT(FILTER_LOG, (char*)"param->mpi_volume_div   : %d\n", param->mpi_volume_div);
    LOGOUT(FILTER_LOG, (char*)"param->mpi_step_div     : %d\n", param->mpi_step_div);
    LOGOUT(FILTER_LOG, (char*)"param->mult_element_type: %d\n", param->mult_element_type);
    LOGOUT(FILTER_LOG, (char*)"param->temp_delete      : %d\n", param->temp_delete);
    LOGOUT(FILTER_LOG, (char*)"param->blocksize        : %d\n", param->blocksize);
    LOGOUT(FILTER_LOG, (char*)"param->division_type          : %d\n", param->division_type);
    LOGOUT(FILTER_LOG, (char*)"param->n_division   : %d\n", param->n_division);
    //LOGOUT(FILTER_LOG,"param->component  : %d\n",param->component);
    //LOGOUT(FILTER_LOG,"param->nodesearch : %d\n",param->nodesearch);
    //LOGOUT(FILTER_LOG,"param->headfoot   : %d\n",param->headfoot  );

    if (param->n_layer > NLAYER_MAX) {
        param->n_layer = NLAYER_MAX;
        LOGOUT(FILTER_LOG, (char*)"The value of specified n_layer exceeded the limit value.\n");
    }
    param->struct_grid = flag0 | flag1;
    LOGOUT(FILTER_LOG, (char*)"param->struct_grid      : %d\n", param->struct_grid);

    state = RTC_OK;

    param->nstep = param->end_step - param->start_step + 1;
    if (param->end_step < param->start_step) {
        LOGOUT(FILTER_ERR, (char*)"The laege and small relation is wrong.\n");
        state = RTC_NG;
    }
    return (state);
}

/*****************************************************************************/

/*
 * 構造格子の場合に情報をセットする
 *
 */
int set_etc(PbvrFilterInfo* filter_info) {
    Int64 i, j;
    char names[COMPONENT_LEN+1] = "VARIABLES       ";
    char units[COMPONENT_LEN+1] = "variables       ";
    char etype;
    if (filter_info == NULL) return RTC_NG;
    FilterParam* param = filter_info->m_param;

    etype = (char) 7;
    if (param->avsucd) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
        for (i = 0; i < filter_info->m_nelements; i += 1) {
            filter_info->m_elemtypes[i] = etype;
            filter_info->m_materials[i] = 1;
        }
    }
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,j)
#endif
    for (i = 0; i < filter_info->m_ncomponents; i += 1) {
        filter_info->m_veclens[i] = param->veclen;
        filter_info->m_nullflags[i] = 0;
        filter_info->m_nulldatas[i] = 0.0f;
        for (j = 0; j < COMPONENT_LEN; j++) {
            filter_info->pm_names[i][j] = names[j];
            filter_info->pm_units[i][j] = units[j];
        }
    }
    return (RTC_OK);
}
/*****************************************************************************/

/*
 * 構造格子から要素を構成する
 *
 */
int create_hexahedron(PbvrFilterInfo* filter_info) {
    int state = RTC_NG;
    Int64 i, j;
    unsigned int inum;
    unsigned int jnum;
    unsigned int knum;
    unsigned int nelements;
    char names[COMPONENT_LEN+1] = "VARIABLES       ";
    char units[COMPONENT_LEN+1] = "variables       ";
    Int64 k;
    unsigned int jj;
    unsigned int node;
    unsigned int eids;
    unsigned int conn = 8;
    unsigned int connect[8];
    if (filter_info == NULL) return RTC_NG;
    FilterParam* param = filter_info->m_param;

    inum = param->dim1;
    jnum = param->dim2;
    knum = param->dim3;
    nelements = (inum - 1)*(jnum - 1)*(knum - 1);

    for (k = 0; k < knum - 1; k++) {
        for (j = 0; j < jnum - 1; j++) {
            for (i = 0; i < inum - 1; i++) {
                node = i + (inum) * j + (inum)*(jnum) * k + 1;
                eids = i + (inum - 1) * j + (inum - 1)*(jnum - 1) * k;

                connect[0] = node;
                connect[1] = connect[0] + 1;
                connect[2] = connect[1] + inum;
                connect[3] = connect[0] + inum;
                connect[4] = connect[0] + inum*jnum;
                connect[5] = connect[4] + 1;
                connect[6] = connect[5] + inum;
                connect[7] = connect[4] + inum;

                for (jj = 0; jj < conn; jj++) {
                    filter_info->m_connections[eids * conn + jj] = connect[jj];
                }
                if (param->avsucd) {
                    filter_info->m_elementids[eids] = eids + 1;
                }
            }
        }
    }

    if (param->avsucd) {
        for (i = 0; i < nelements; i += 1) {
            filter_info->m_elemtypes[i] = (char) 7;
            filter_info->m_materials[i] = 1;
        }
    }
    for (i = 0; i < filter_info->m_ncomponents; i += 1) {
        filter_info->m_veclens[i] = param->veclen;
        filter_info->m_nullflags[i] = 0;
        filter_info->m_nulldatas[i] = 0.0f;
        for (j = 0; j < COMPONENT_LEN; j++) {
            filter_info->pm_names[i][j] = names[j];
            filter_info->pm_units[i][j] = units[j];
        }
    }

    state = RTC_OK;
    return (state);
}


/*****************************************************************************/

/*
 * 構造格子から要素を構成する
 *
 */
int create_tetrahedron(PbvrFilterInfo* filter_info) {
    int state = RTC_NG;
    Int64 i, j;
    unsigned int inum;
    unsigned int jnum;
    unsigned int knum;
    unsigned int nelements;
    char names[COMPONENT_LEN+1] = "VARIABLES       ";
    char units[COMPONENT_LEN+1] = "variables       ";
    Int64 k;
    unsigned int ii, jj;
    unsigned int node;
    unsigned int eids;
    unsigned int conn = 4;
    unsigned int connect[8];
    if (filter_info == NULL) return RTC_NG;
    FilterParam* param = filter_info->m_param;

    inum = param->dim1;
    jnum = param->dim2;
    knum = param->dim3;
    nelements = (inum - 1)*(jnum - 1)*(knum - 1)*5;

    for (k = 0; k < knum - 1; k++) {
        for (j = 0; j < jnum - 1; j++) {
            for (i = 0; i < inum - 1; i++) {
                node = i + (inum) * j + (inum)*(jnum) * k + 1;
                eids = i + (inum - 1) * j + (inum - 1)*(jnum - 1) * k;

                connect[0] = node;
                connect[1] = connect[0] + 1;
                connect[2] = connect[1] + inum;
                connect[3] = connect[0] + inum;
                connect[4] = connect[0] + inum*jnum;
                connect[5] = connect[4] + 1;
                connect[6] = connect[5] + inum;
                connect[7] = connect[4] + inum;

                for (jj = 0; jj < 5; jj++) {
                    for (ii = 0; ii < conn; ii++) {
                        filter_info->m_connections[eids * 20 + (jj * conn) + ii] = connect[ hexdiv[jj][ii] ];
                    }
                    if (param->avsucd) {
                        filter_info->m_elementids[eids * 5 + jj] = eids * 5 + jj + 1;
                    }
                }
            }
        }
    }

    if (param->avsucd) {
        for (i = 0; i < nelements; i += 1) {
            filter_info->m_elemtypes[i] = (char) 4;
            filter_info->m_materials[i] = 1;
        }
    }
    for (i = 0; i < filter_info->m_ncomponents; i += 1) {
        filter_info->m_veclens[i] = param->veclen;
        filter_info->m_nullflags[i] = 0;
        filter_info->m_nulldatas[i] = 0.0f;
        for (j = 0; j < COMPONENT_LEN; j++) {
            filter_info->pm_names[i][j] = names[j];
            filter_info->pm_units[i][j] = units[j];
        }
    }

    state = RTC_OK;
    return (state);
}


/*****************************************************************************/

/*
 * サブボリューム内の要素・節点
 *
 */
int count_in_subvolume_b(int subvolume_no,
        Int64* subelemnum,
        Int64* subnodenum,
        Int64* subconnnum,
        PbvrFilterInfo *filter_info)
{
    int stat = RTC_OK;
    Int64 i, j, k;
    int node_id;
    int size;

    if (filter_info == NULL) return RTC_NG;
    FilterParam* param = filter_info->m_param;
    int *m_subvolume_num = filter_info->m_subvolume_num;
    char *m_elemtypes = filter_info->m_elemtypes;
    const int *m_elementsize = filter_info->m_elementsize;
    int *m_connections = filter_info->m_connections;
    char *m_nodeflag = filter_info->m_nodeflag;
    int *m_nodes_trans = filter_info->m_nodes_trans;

    *subelemnum = 0;
    *subnodenum = 0;
    *subconnnum = 0;

    for (i = 0; i < filter_info->m_nnodes; i++) {
        m_nodeflag[i] = 0;
        m_nodes_trans[i] = 0;
    }

    if (param->mult_element_type) {
        Int64 addr;
        addr = 0;
        for (i = 0; i < filter_info->m_nelements; i++) {
            size = m_elementsize[(int) m_elemtypes[i]];
            if (subvolume_no == m_subvolume_num[i]) {
                (*subelemnum)++;
                j = 0;
                while (j < size) {
                    node_id = m_connections[addr + j];
                    k = node_id - 1;
                    m_nodeflag[k] = 1;
                    j++;
                }
                (*subconnnum) += size;
            }
            addr += size;
        }
    } else {
        Int64 elem_num;
        size = filter_info->m_elemcoms;
        elem_num = 0;
#ifdef _OPENMP
    {
#pragma omp parallel for default(shared) private(i,j,node_id,k) reduction(+:elem_num)
#endif
    for (i = 0; i < filter_info->m_nelements; i++) {
        if (subvolume_no == m_subvolume_num[i]) {
            elem_num++;
            j = 0;
            while (j < size) {
                node_id = m_connections[(Int64) size * i + j];
                k = node_id - 1;
                m_nodeflag[k] = 1;
                j++;
            }
        }
    }
#ifdef _OPENMP
    }
#endif
        *subelemnum = elem_num;
        *subconnnum = (*subelemnum) * size;
    }

    for (i = 0; i < filter_info->m_nnodes; i++) {
        if (m_nodeflag[i]) {
            (*subnodenum)++;
            m_nodes_trans[i] = *subnodenum;
        } else {
            m_nodes_trans[i] = (char) - 1;
        }
    }
    return (stat);
}

/*****************************************************************************/

/*
 * サブボリューム内の要素・節点
 *
 */
int count_in_subvolume(int subvolume_no,
        Int64* subelemnum,
        Int64* subnodenum,
        Int64* subconnnum,
        Int64* prov_len,
        Int64 rank_nodes,
        Int64 rank_elems,
        bool mult_element_type,
        PbvrFilterInfo *filter_info)
{
    int stat = RTC_OK;
    Int64 i;
    Int64 j;
    Int64 k;
    unsigned int node_id;
    unsigned int node_num;
    unsigned int elem_num;
    unsigned int elem_id;
    unsigned int connect[8];
    unsigned int inum;
    unsigned int jnum;
    /*  unsigned int knum; */
    int size;
    int shift;

    if (filter_info == NULL) return RTC_NG;
    FilterParam* param = filter_info->m_param;
    int *m_subvolume_num = filter_info->m_subvolume_num;
    unsigned int *m_elems_per_subvolume = filter_info->m_elems_per_subvolume;
    int *m_conn_top_node = filter_info->m_conn_top_node;
    char *m_elemtypes = filter_info->m_elemtypes;
    const int *m_elementsize = filter_info->m_elementsize;
    int *m_connections = filter_info->m_connections;
    char *m_nodeflag = filter_info->m_nodeflag;
    int *m_subvolume_elems = filter_info->m_subvolume_elems;

    shift = filter_info->m_tree_node_no - filter_info->m_nvolumes;
    *subelemnum = 0;
    *subnodenum = 0;
    *subconnnum = 0;

    if (mult_element_type) {
        DEBUG(4,"######## count in subvolume multi element type %ld \n", filter_info->m_nelements);
        Int64 addr;
        addr = 0;
        for (i = 0; i < filter_info->m_nelements; i++) {
            size = m_elementsize[(int) m_elemtypes[i]];
            if (subvolume_no == m_subvolume_num[i]) {
                (*subelemnum)++;
                j = 0;
                while (j < size) {
                    node_id = m_connections[addr + j];
                    k = node_id - 1;
                    if (!m_nodeflag[k]) {
                        m_nodeflag[k] = 1;
                        (*subnodenum)++;
                    }
                    j++;
                }
                (*subconnnum) += size;
            }
            addr += size;
        }
    } else {
        DEBUG(4,"######## count in subvolume single element type\n");
        if (param->struct_grid) {
            DEBUG(4,"######## count in struct grid\n");
            inum = param->dim1;
            jnum = param->dim2;
            /* knum = param->dim3; */

            elem_num = m_elems_per_subvolume[subvolume_no - shift];
            if (param->ndim == 2) {
                size = 4;
                for (i = 0; i < elem_num; i++) {
                    elem_id = filter_info->m_element_array[subvolume_no - shift][i];
                    m_subvolume_elems[rank_elems + i] = elem_id;
                    node_id = m_conn_top_node[elem_id];

                    connect[0] = node_id;
                    connect[1] = connect[0] + 1;
                    connect[2] = connect[1] + inum;
                    connect[3] = connect[0] + inum;

                    j = 0;
                    while (j < size) {
                        node_id = connect[j];
                        k = node_id - 1;
                        if (!m_nodeflag[k]) {
                            m_nodeflag[k] = 1;
                            (*subnodenum)++;
                        }
                        j++;
                    }
                }
            } else {
                size = 8;
                for (i = 0; i < elem_num; i++) {
                    elem_id = filter_info->m_element_array[subvolume_no - shift][i];
                    m_subvolume_elems[rank_elems + i] = elem_id;
                    node_id = m_conn_top_node[elem_id];

                    connect[0] = node_id;
                    connect[1] = connect[0] + 1;
                    connect[2] = connect[1] + inum;
                    connect[3] = connect[0] + inum;
                    connect[4] = connect[0] + inum*jnum;
                    connect[5] = connect[4] + 1;
                    connect[6] = connect[5] + inum;
                    connect[7] = connect[4] + inum;

                    j = 0;
                    while (j < size) {
                        node_id = connect[j];
                        k = node_id - 1;
                        if (!m_nodeflag[k]) {
                            m_nodeflag[k] = 1;
                            (*subnodenum)++;
                        }
                        j++;
                    }
                }
            }
            *subelemnum = elem_num;
            *subconnnum = elem_num*size;
        } else {
            DEBUG(4,"######## count in subvolume unstruct grid\n");
            elem_num = m_elems_per_subvolume[subvolume_no - shift];
            size = filter_info->m_elemcoms;
            for (i = 0; i < elem_num; i++) {
                elem_id = filter_info->m_element_array[subvolume_no - shift][i];
                m_subvolume_elems[rank_elems + i] = elem_id;
                j = 0;
                while (j < size) {
                    node_id = m_connections[(Int64) size * (Int64) elem_id + j];
                    k = node_id - 1;
                    if (!m_nodeflag[k]) {
                        m_nodeflag[k] = 1;
                        (*subnodenum)++;
                    }
                    j++;
                }
            }
            *subelemnum = elem_num;
            *subconnnum = elem_num*size;
        }
    }

    if ((rank_nodes + *subnodenum)>*prov_len) {
        if (RTC_OK != realloc_work(rank_nodes + (*subnodenum), filter_info)) {
            LOGOUT(FILTER_ERR, (char*)"realloc_work error.\n");
            return (RTC_NG);
        }
        *prov_len = rank_nodes + (*subnodenum);
    }


    node_num = 0;
    for (i = 0; i < filter_info->m_nnodes; i++) {
        if (m_nodeflag[i]) {
            filter_info->m_subvolume_nodes[rank_nodes + node_num] = i;
            filter_info->m_subvolume_trans[rank_nodes + node_num] = node_num + 1;
            node_num++;
            m_nodeflag[i] = 0;
        }
    }

    return (stat);
}

/*****************************************************************************/

/*
 * サブボリューム内の要素・節点
 *
 */
int set_nodes_in_subvolume(Int64 volume_nodes,
        Int64 rank_nodes,
        PbvrFilterInfo *filter_info)
{
    int stat = RTC_OK;
    Int64 i;
    int node_id;

    if (filter_info == NULL) return RTC_NG;
    int *m_subvolume_trans = filter_info->m_subvolume_trans;
    int *m_subvolume_nodes = filter_info->m_subvolume_nodes;
    int *m_nodes_trans = filter_info->m_nodes_trans;

#ifdef _OPENMP
    {
#pragma omp parallel default(shared)
#pragma omp for private(i,node_id)
#endif
    for (i = 0; i < volume_nodes; i++) {
        node_id = m_subvolume_nodes[rank_nodes + i];
        m_nodes_trans[node_id] = m_subvolume_trans[rank_nodes + i];
    }
#ifdef _OPENMP
    }
#endif

    return (stat);
}

/*****************************************************************************/

/*
 * ノード番号の検索
 *
 */
int search_node_id(int node_id,
        int m_nnodes,
        int** m_ids) {
#if 0
    size_t left;
    size_t right;
    size_t mid;

    left = 0;
    right = m_nnodes;
    while (left <= right) {
        mid = (left + right) / 2;
        if ((*m_ids)[mid] == node_id) {
            return mid;
        } else if ((*m_ids)[mid] < node_id) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    LOGOUT(FILTER_ERR, "error not found node id %d\n", node_id);
    return -1;
#else
    int i;
    for (i = 0; i < m_nnodes; i++) {
        if (node_id == (*m_ids)[i]) {
            return i;
        }
    }
    return -1;
#endif
}

/*****************************************************************************/

int ConvertEndianI(int val) {
    unsigned char tmp0[4];
    unsigned char tmp1[4];
    int i;
    memcpy(tmp0, &val, 4);
    for (i = 0; i < 4; i++) {
        tmp1[i] = tmp0[3 - i];
    }
    memcpy(&val, tmp1, 4);
    return val;
}

/*****************************************************************************/

float ConvertEndianF(float val) {
    unsigned char tmp0[4];
    unsigned char tmp1[4];
    int i;
    memcpy(tmp0, &val, 4);
    for (i = 0; i < 4; i++) {
        tmp1[i] = tmp0[3 - i];
    }
    memcpy(&val, tmp1, 4);
    return val;
}

/*****************************************************************************/

char check_endian(void) {
    char tmp[4];
    int ival;
    ival = 1;
    memcpy(tmp, &ival, 4);
    if (tmp[3]) {
        return (char) 1;
    }
    return (char) 0;
}

/*****************************************************************************/
/*
 * 節点を含む要素
 *
 */
int set_element_in_node(PbvrFilterInfo *filter_info) {
    int stat = RTC_OK;
    Int64 i, j, k;
    int node_id;
    int conn_size;
    int conn_addr;
    int *work;

    if (filter_info == NULL) return RTC_NG;
    const int *m_elementsize = filter_info->m_elementsize;
    char *m_elemtypes = filter_info->m_elemtypes;
    int *m_connections = filter_info->m_connections;
    int *m_elem_in_node = filter_info->m_elem_in_node;
    int *m_elem_in_node_addr = filter_info->m_elem_in_node_addr;

    conn_addr = 0;
    for (i = 0; i < filter_info->m_nelements; i++) {
        conn_size = m_elementsize[(int) m_elemtypes[i]];
        for (j = 0; j < conn_size; j++) {
            /* node id */
            node_id = m_connections[conn_addr + j];
            k = node_id - 1;
            m_elem_in_node[k]++;
        }
        conn_addr += conn_size;
    }

    m_elem_in_node_addr[0] = 0;
    for (i = 1; i < filter_info->m_nnodes; i++) {
        m_elem_in_node_addr[i] = m_elem_in_node_addr[i - 1] + m_elem_in_node[i - 1];
    }

    work = (int *) malloc(sizeof (int)*filter_info->m_nnodes);
    memset(work, 0x00, sizeof (int)*filter_info->m_nnodes);

    if (filter_info->m_elem_in_node_array != NULL) free(filter_info->m_elem_in_node_array);
    filter_info->m_elem_in_node_array = (int *) malloc(sizeof (int)*conn_addr);
    filter_info->m_elem_in_node_array_size = conn_addr;

    conn_addr = 0;
    for (i = 0; i < filter_info->m_nelements; i++) {
        conn_size = m_elementsize[(int) m_elemtypes[i]];
        for (j = 0; j < conn_size; j++) {
            /* node id */
            node_id = m_connections[conn_addr + j];
            k = node_id - 1;
            filter_info->m_elem_in_node_array[m_elem_in_node_addr[k] + work[k]] = i + 1;
            work[k]++;
        }
        conn_addr += conn_size;
    }
    free(work);
    work = NULL;

    return (stat);
}

/*****************************************************************************/

/*
 * 節点を含む要素毎のフラグセット
 *
 */
int set_nodeflag_per_element(PbvrFilterInfo *filter_info) {
    int stat = RTC_OK;
    Int64 i, j;
    int node_id;
    int conn_size;
    int conn_addr;
    int *work;

    if (filter_info == NULL) return RTC_NG;
    const int *m_elementsize = filter_info->m_elementsize;
    char *m_elemtypes = filter_info->m_elemtypes;
    int *m_connections = filter_info->m_connections;
    int *m_elem_in_node = filter_info->m_elem_in_node;
    int *m_elem_in_node_addr = filter_info->m_elem_in_node_addr;

    conn_addr = 0;
    for (i = 0; i < filter_info->m_nelements; i++) {
        conn_size = m_elementsize[(int) m_elemtypes[i]];
        for (j = 0; j < conn_size; j++) {
            /* node id */
            node_id = m_connections[conn_addr + j];
            m_elem_in_node[node_id - 1]++;
        }
        conn_addr += conn_size;
    }

    m_elem_in_node_addr[0] = 0;
    for (i = 1; i < filter_info->m_nnodes; i++) {
        m_elem_in_node_addr[i] = m_elem_in_node_addr[i - 1] + m_elem_in_node[i - 1];
    }

    work = (int *) malloc(sizeof (int)*filter_info->m_nnodes);
    memset(work, 0x00, sizeof (int)*filter_info->m_nnodes);
    filter_info->m_elem_in_node_array = (int *) malloc(sizeof (int)*conn_addr);
    filter_info->m_elem_in_node_array_size = conn_addr;

    conn_addr = 0;
    for (i = 0; i < filter_info->m_nelements; i++) {
        conn_size = m_elementsize[(int) m_elemtypes[i]];
        for (j = 0; j < conn_size; j++) {
            /* node id */
            node_id = m_connections[conn_addr + j];
            filter_info->m_elem_in_node_array[m_elem_in_node_addr[node_id - 1] + work[node_id - 1]] = i + 1;
            work[node_id - 1]++;
        }
        conn_addr += conn_size;
    }
    free(work);
    work = NULL;

    return (stat);
}

/*****************************************************************************/

/*
 * サブボリューム内の要素・節点
 *
 */
int count_in_elements(int elem_id,
        Int64* subelemnum,
        Int64* subnodenum,
        Int64* subconnnum,
        Int64* prov_len,
        Int64 rank_nodes,
        Int64 rank_elems,
        PbvrFilterInfo *filter_info)
{
    int stat = RTC_OK;
    Int64 i;
    Int64 j;
    Int64 k;
    unsigned int node_id;
    unsigned int node_num;
    char elem_type;
    int size;
    Int64 addr;

    if (filter_info == NULL) return RTC_NG;
    const int *m_elementsize = filter_info->m_elementsize;
    char *m_elemtypes = filter_info->m_elemtypes;
    int *m_connections = filter_info->m_connections;
    char *m_nodeflag = filter_info->m_nodeflag;

    *subelemnum = 0;
    *subnodenum = 0;
    *subconnnum = 0;

    //  elem_type = m_types[elem_id];
    elem_type = elem_id;

    size = 0;
    addr = 0;
    for (i = 0; i < filter_info->m_nelements; i++) {
        size = m_elementsize[(int) m_elemtypes[i]];
        if (m_elemtypes[i] == elem_type) {
            (*subelemnum)++;
            j = 0;
            while (j < size) {
                node_id = m_connections[addr + j];
                k = node_id - 1;
                if (!m_nodeflag[k]) {
                    m_nodeflag[k] = 1;
                    (*subnodenum)++;
                }
                j++;
            }
            (*subconnnum) += size;
        }
        addr += size;
    }

    if ((rank_nodes + *subnodenum)>*prov_len) {
        if (RTC_OK != realloc_work(rank_nodes + (*subnodenum), filter_info)) {
            LOGOUT(FILTER_ERR, (char*)"realloc_work error.\n");
            return (RTC_NG);
        }
        *prov_len = rank_nodes + (*subnodenum);
    }


    node_num = 0;
    for (i = 0; i < filter_info->m_nnodes; i++) {
        if (m_nodeflag[i]) {
            filter_info->m_subvolume_nodes[rank_nodes + node_num] = i;
            filter_info->m_subvolume_trans[rank_nodes + node_num] = node_num + 1;
            node_num++;
            m_nodeflag[i] = 0;
        }
    }

    LOGOUT(FILTER_LOG, (char*)"||<> elem_id      .: %5d\n", elem_id);
    LOGOUT(FILTER_LOG, (char*)"||<> node_num     .: %5d\n", node_num);
    LOGOUT(FILTER_LOG, (char*)"||<> nodes    / all nodes    : %9d / %9d\n", *subnodenum, filter_info->m_nnodes);
    LOGOUT(FILTER_LOG, (char*)"||<> elements / all elements : %9d / %9d\n", *subelemnum, filter_info->m_nelements);
    LOGOUT(FILTER_LOG, (char*)"||<> subconnnum              : %9d      \n", *subconnnum);
    return (stat);
}


int release_PbvrFilterInfo(PbvrFilterInfo *filter_info) {

    if (filter_info == NULL) return RTC_NG;

    DEBUG(3, "!!!!!! initialize_memory (Free if NULL) !!!!!!!\n");
    int i;
    /* メモリ開放 */
    if (NULL != filter_info->m_geom_fp) free(filter_info->m_geom_fp);
    if (NULL != filter_info->m_value_fp) free(filter_info->m_value_fp);
    if (NULL != filter_info->m_octree) free(filter_info->m_octree);
    if (NULL != filter_info->m_tree) free(filter_info->m_tree);
    if (NULL != filter_info->m_ids) free(filter_info->m_ids);
    if (NULL != filter_info->m_coords) free(filter_info->m_coords);
    if (NULL != filter_info->m_connections) free(filter_info->m_connections);
    if (NULL != filter_info->m_connections_o) free(filter_info->m_connections_o);
    if (NULL != filter_info->m_conn_top_node) free(filter_info->m_conn_top_node);
    if (NULL != filter_info->m_materials) free(filter_info->m_materials);
    if (NULL != filter_info->m_elemtypes) free(filter_info->m_elemtypes);
    if (NULL != filter_info->m_values) free(filter_info->m_values);
    if (NULL != filter_info->mt_nullflag) free(filter_info->mt_nullflag);
    if (NULL != filter_info->mt_nulldata) free(filter_info->mt_nulldata);
    if (NULL != filter_info->m_value_max) free(filter_info->m_value_max);
    if (NULL != filter_info->m_value_min) free(filter_info->m_value_min);
    if (NULL != filter_info->m_veclens) free(filter_info->m_veclens);
    if (NULL != filter_info->m_nullflags) free(filter_info->m_nullflags);
    if (NULL != filter_info->m_nulldatas) free(filter_info->m_nulldatas);
    if (NULL != filter_info->m_subvolume_num) free(filter_info->m_subvolume_num);
    if (NULL != filter_info->m_nodeflag) free(filter_info->m_nodeflag);
    if (NULL != filter_info->m_nodes_trans) free(filter_info->m_nodes_trans);
    if (NULL != filter_info->m_elems_per_subvolume) free(filter_info->m_elems_per_subvolume);
    if (NULL != filter_info->m_nodes_per_subvolume) free(filter_info->m_nodes_per_subvolume);
    if (NULL != filter_info->m_conns_per_subvolume) free(filter_info->m_conns_per_subvolume);
    if (NULL != filter_info->m_elems_accumulate) free(filter_info->m_elems_accumulate);
    if (NULL != filter_info->m_nodes_accumulate) free(filter_info->m_nodes_accumulate);
    if (NULL != filter_info->m_conns_accumulate) free(filter_info->m_conns_accumulate);
    if (NULL != filter_info->m_elementids) free(filter_info->m_elementids);
    if (NULL != filter_info->m_subvolume_nodes) free(filter_info->m_subvolume_nodes);
    if (NULL != filter_info->m_elem_in_node_array) free(filter_info->m_elem_in_node_array);
    if (NULL != filter_info->m_elem_in_node_addr) free(filter_info->m_elem_in_node_addr);
    if (NULL != filter_info->m_elem_in_node) free(filter_info->m_elem_in_node);
    if (filter_info->pm_units != NULL) {
        for (i = 0; i < filter_info->m_ncomponents; i++) {
            free(filter_info->pm_units[i]);
        }
        free(filter_info->pm_units);
    }
    if (filter_info->pm_names != NULL) {
        for (i = 0; i < filter_info->m_ncomponents; i++) {
            free(filter_info->pm_names[i]);
        }
        free(filter_info->pm_names);
    }
    if (filter_info->m_subvolume_coord != NULL) free(filter_info->m_subvolume_coord);
    if (filter_info->m_subvolume_connect != NULL) free(filter_info->m_subvolume_connect);
    if (filter_info->m_subvolume_trans != NULL) free(filter_info->m_subvolume_trans);
    if (filter_info->m_subvolume_elems != NULL) free(filter_info->m_subvolume_elems);
    if (filter_info->m_elems_per_multivolume != NULL) free(filter_info->m_elems_per_multivolume);
    if (filter_info->m_conns_per_multivolume != NULL) free(filter_info->m_conns_per_multivolume);
    if (filter_info->m_nodes_per_multivolume != NULL) free(filter_info->m_nodes_per_multivolume);
    if (filter_info->m_multivolume_nodes != NULL) free(filter_info->m_multivolume_nodes);

    initialize_PbvrFilterInfo(filter_info);

    return (RTC_OK);
}


int pre_release_PbvrFilterInfo(PbvrFilterInfo *filter_info) {

    if (filter_info == NULL) return RTC_NG;

    DEBUG(3, "!!!!!! initialize_memory (Free if NULL) !!!!!!!\n");
    int i;
    /* メモリ開放 */
    if (NULL != filter_info->m_geom_fp) {
        free(filter_info->m_geom_fp);
        filter_info->m_geom_fp = NULL;
    }
    // if (NULL != filter_info->m_value_fp) {
    //    free(filter_info->m_value_fp);
    //    filter_info->m_value_fp = NULL;
    // }
    if (NULL != filter_info->m_octree) {
        free(filter_info->m_octree);
        filter_info->m_octree = NULL;
    }
    //if (NULL != filter_info->m_tree) free(filter_info->m_tree);
    if (NULL != filter_info->m_ids) {
        free(filter_info->m_ids);
        filter_info->m_ids = NULL;
    }

    // delete by @hira at 2017/04/01 m_coordsをfreeする代わりにm_subvolume_coordを生かす。
    if (NULL != filter_info->m_coords) {
        free(filter_info->m_coords);
        filter_info->m_coords = NULL;
    }

    if (NULL != filter_info->m_connections) {
        free(filter_info->m_connections);
        filter_info->m_connections = NULL;
    }
    if (NULL != filter_info->m_connections_o) {
        free(filter_info->m_connections_o);
        filter_info->m_connections_o = NULL;
    }
    if (NULL != filter_info->m_conn_top_node) {
        free(filter_info->m_conn_top_node);
        filter_info->m_conn_top_node = NULL;
    }

    // if (NULL != filter_info->m_materials) free(filter_info->m_materials);
    // if (NULL != filter_info->m_elemtypes) free(filter_info->m_elemtypes);
    // if (NULL != filter_info->m_values) free(filter_info->m_values);
    if (NULL != filter_info->mt_nullflag) {
        free(filter_info->mt_nullflag);
        filter_info->mt_nullflag = NULL;
    }
    if (NULL != filter_info->mt_nulldata) {
        free(filter_info->mt_nulldata);
        filter_info->mt_nulldata = NULL;
    }
    // if (NULL != filter_info->m_value_max) free(filter_info->m_value_max);
    // if (NULL != filter_info->m_value_min) free(filter_info->m_value_min);
    // if (NULL != filter_info->m_veclens) free(filter_info->m_veclens);
    // if (NULL != filter_info->m_nullflags) free(filter_info->m_nullflags);
    // if (NULL != filter_info->m_nulldatas) free(filter_info->m_nulldatas);
    if (NULL != filter_info->m_subvolume_num) {
        free(filter_info->m_subvolume_num);
        filter_info->m_subvolume_num = NULL;
    }
    if (NULL != filter_info->m_nodeflag) {
        free(filter_info->m_nodeflag);
        filter_info->m_nodeflag = NULL;
    }

    if (NULL != filter_info->m_nodes_trans) {
        free(filter_info->m_nodes_trans);
        filter_info->m_nodes_trans = NULL;
    }

    // m_elems_per_subvolumeは削除不可 : PbvrFilterInformationFileにポインタコピー
    // if (NULL != filter_info->m_elems_per_subvolume) free(filter_info->m_elems_per_subvolume);
    // m_nodes_per_subvolumeは削除不可 : PbvrFilterInformationFileにポインタコピー
    // if (NULL != filter_info->m_nodes_per_subvolume) free(filter_info->m_nodes_per_subvolume);
    // m_conns_per_subvolumeは削除不可 : PbvrFilterInformationFileにポインタコピー
    // if (NULL != filter_info->m_conns_per_subvolume) free(filter_info->m_conns_per_subvolume);
    if (NULL != filter_info->m_elems_accumulate) {
        free(filter_info->m_elems_accumulate);
        filter_info->m_elems_accumulate = NULL;
    }
    if (NULL != filter_info->m_nodes_accumulate) {
        free(filter_info->m_nodes_accumulate);
        filter_info->m_nodes_accumulate = NULL;
    }
    if (NULL != filter_info->m_conns_accumulate) {
        free(filter_info->m_conns_accumulate);
        filter_info->m_conns_accumulate = NULL;
    }

    // m_subvolume_nodesは削除不可 : PbvrFilterInformationFileにポインタコピー
    // if (NULL != filter_info->m_subvolume_nodes) free(filter_info->m_subvolume_nodes);

    // ランク０のみアロケートしている。read_ucdbin_dataにて使用の為削除不可
    // if (NULL != filter_info->m_elementids) free(filter_info->m_elementids);
    // if (NULL != filter_info->m_elem_in_node_array) free(filter_info->m_elem_in_node_array);
    // if (NULL != filter_info->m_elem_in_node_addr) free(filter_info->m_elem_in_node_addr);
    // if (NULL != filter_info->m_elem_in_node) free(filter_info->m_elem_in_node);
//    if (filter_info->pm_units != NULL) {
//        for (i = 0; i < filter_info->m_ncomponents; i++) {
//            free(filter_info->pm_units[i]);
//            filter_info->pm_units[i] = NULL;
//        }
//        free(filter_info->pm_units);
//        filter_info->pm_units = NULL;
//    }
//    if (filter_info->pm_names != NULL) {
//        for (i = 0; i < filter_info->m_ncomponents; i++) {
//            free(filter_info->pm_names[i]);
//            filter_info->pm_names[i] = NULL;
//        }
//        free(filter_info->pm_names);
//        filter_info->pm_names = NULL;
//    }

    // delete by @hira at 2017/04/01 m_coordsをfreeする代わりにm_subvolume_coordを生かす。
    // if (filter_info->m_subvolume_coord != NULL) {
    //    free(filter_info->m_subvolume_coord);
    //    filter_info->m_subvolume_coord = NULL;
    //}

    if (filter_info->m_subvolume_connect != NULL) {
        free(filter_info->m_subvolume_connect);
        filter_info->m_subvolume_connect = NULL;
    }
    if (filter_info->m_subvolume_trans != NULL) {
        free(filter_info->m_subvolume_trans);
        filter_info->m_subvolume_trans = NULL;
    }

    if (filter_info->m_subvolume_elems != NULL) {
        free(filter_info->m_subvolume_elems);
        filter_info->m_subvolume_elems = NULL;
    }


    return (RTC_OK);
}


int initialize_PbvrFilterInfo(PbvrFilterInfo *filter_info) {
    DEBUG(3, "!!!!!!!!!!initialize_value (Set To NULL) !!!!!\n");

    filter_info->m_tree_node_no = 0;
    filter_info->m_stepn = 0;
    filter_info->m_stept = 0.0f;
    filter_info->m_nnodes = 0;
    filter_info->m_nelements = 0;
    filter_info->m_nnodekinds = 0;
    filter_info->m_nelemkinds = 0;
    filter_info->m_nkinds = 0;
    filter_info->m_nnodecomponents = 0;
    filter_info->m_nelemcomponents = 0;
    filter_info->m_ncomponents = 0;
    filter_info->m_elemcoms = 0;
    filter_info->m_desctype = 0;
    filter_info->m_datatype = 0;
    filter_info->m_component_id = 0;
    filter_info->m_nvalues_per_node = 0;
    filter_info->m_ncomponents_per_node = 0;
    filter_info->m_headfoot = 0;
    filter_info->m_filesize = 0;
    filter_info->m_ucdbin = 1;		// add by @hira at 2017/04/15

    filter_info->m_values_size = 0;
    filter_info->m_values_size = 0;
    filter_info->m_subvolume_coord_size = 0;
    filter_info->m_subvolume_connect_size = 0;
    filter_info->m_elem_in_node_array_size = 0;
    filter_info->m_ucdbinfile_size = 0;
    filter_info->element_type_num = 0;
    filter_info->prov_len = 0;
    filter_info->prov_len_e = 0;
    filter_info->pre_step = 0;
    filter_info->current_block = 0;
    filter_info->number_of_blocks = 0;
    filter_info->m_values_size = 0;
    filter_info->element_type_num = 0;
    filter_info->m_ucdbinfile_size = 0;
    filter_info->m_elem_in_node_array_size = 0;

    /*
    filter_info->subvolume_elems = 0;
    filter_info->subvolume_nodes = 0;
    filter_info->subvolume_conns = 0;
     */
    filter_info->m_geom_fp = NULL;
    filter_info->m_value_fp = NULL;
    filter_info->m_octree = NULL;
    filter_info->m_tree = NULL;
    filter_info->m_ids = NULL;
    filter_info->m_coords = NULL;
    filter_info->m_connections = NULL;
    filter_info->m_connections_o = NULL;
    filter_info->m_conn_top_node = NULL;
    filter_info->m_materials = NULL;
    filter_info->m_elemtypes = NULL;
    filter_info->m_values = NULL;
    filter_info->m_value_max = NULL;
    filter_info->m_value_min = NULL;
    filter_info->m_veclens = NULL;
    filter_info->m_nullflags = NULL;
    filter_info->m_nulldatas = NULL;
    filter_info->mt_nullflag = NULL;
    filter_info->mt_nulldata = NULL;
    filter_info->m_subvolume_num = NULL;
    filter_info->m_nodeflag = NULL;
    filter_info->m_nodes_trans = NULL;
    filter_info->m_elems_per_subvolume = NULL;
    filter_info->m_conns_per_subvolume = NULL;
    filter_info->m_nodes_per_subvolume = NULL;
    filter_info->m_elems_accumulate = NULL;
    filter_info->m_conns_accumulate = NULL;
    filter_info->m_nodes_accumulate = NULL;
    filter_info->m_elementids = NULL;
    filter_info->m_subvolume_nodes = NULL;
    filter_info->m_elem_in_node_array = NULL;
    filter_info->m_elem_in_node_addr = NULL;
    filter_info->m_elem_in_node = NULL;
    filter_info->pm_names = NULL;
    filter_info->pm_units = NULL;
    filter_info->m_subvolume_coord = NULL;
    filter_info->m_subvolume_connect = NULL;
    filter_info->m_subvolume_trans = NULL;
    filter_info->m_subvolume_elems = NULL;
    filter_info->m_ucdbinfile = NULL;
	filter_info->m_ucd_ascii_fp = NULL;		// Added by kkawa (2018/11/29)

    /*
    filter_info->offset_values = NULL;
    filter_info->all_subvolume_nodes = NULL;
    filter_info->all_subvolume_trans = NULL;
    filter_info->all_subvolume_elems = NULL;
     */
    filter_info->m_pbvr_connections.clear();
    filter_info->m_pbvr_connect_count.clear();
    filter_info->m_pbvr_subvolumeids.clear();
    std::vector<int>().swap(filter_info->m_pbvr_connections);
    std::vector<int>().swap(filter_info->m_pbvr_connect_count);
    std::vector<int>().swap(filter_info->m_pbvr_subvolumeids);

    filter_info->m_elems_per_multivolume = NULL;
    filter_info->m_conns_per_multivolume = NULL;
    filter_info->m_nodes_per_multivolume = NULL;;
    filter_info->m_multivolume_nodes = NULL;
    filter_info->step_count = 0;

    return (RTC_OK);
}

char* getFilterCellTypeText(int element_type, char* cell_type) {
    if (element_type == 4) {
        strcpy(cell_type, "tetrahedra");
    } else if (element_type == 11) {
        strcpy(cell_type, "quadratic tetrahedra");
    } else if (element_type == 7) {
        strcpy(cell_type, "hexahedra");
    } else if (element_type == 14) {
        strcpy(cell_type, "quadratic hexahedra");
    } else if (element_type == 5) {
        strcpy(cell_type, "pyramid");
    } else if (element_type == 6) {
        strcpy(cell_type, "prism");
    } else if (element_type == 2) {
        strcpy(cell_type, "triangle");
    } else if (element_type == 3) {
        strcpy(cell_type, "quadratic");
    } else if (element_type == 9) {
        strcpy(cell_type, "triangle2");
    } else if (element_type == 10) {
        strcpy(cell_type, "quadratic2");
    } else {
        strcpy(cell_type, "unknown");
    }

    return cell_type;
}


void print_memory()
{
#ifndef WIN32
#if 0
    {
        const char* cmdline = "free |grep '\\-\\/+ buff' |sed 's/\\s\\{1,\\}/ /g' | cut -d' ' -f4";
        FILE* fp;
        char buff[512];
        char buf[256] = {0x00};
        int c;
        int t;
        size_t c_smemory;

        buf[0] = 0x00;
        fp = popen( cmdline, "r" );
        fgets( buf, 256, fp );
        pclose( fp );


        sscanf( buf, "%d", &c );
        c = c / 1024;


        long pages = sysconf( _SC_PHYS_PAGES );
        long page_size = sysconf( _SC_PAGE_SIZE );

        t = pages * page_size / 1024 / 1024;
        LOGOUT(FILTER_LOG, (char*)"Free MEMORY/Total MEMORY : %ld(k)/%ld(M) \n", c, t);
        c_smemory = c;
        c = t - c;
        int p = c * 100 / t;
        LOGOUT(FILTER_LOG, (char*)"CPU MEMORY %d/%d %d\n", c, t, p );
    }
    {
        struct rusage r;
        if (getrusage(RUSAGE_SELF, &r) != 0) {
            return;
        }
        LOGOUT(FILTER_LOG, (char*)"GETRUSAGE MEMORY ru_maxrss=%d\n", r.ru_maxrss );
    }
#endif
#if !defined(__APPLE__)
    {
        struct sysinfo info;

        unsigned long totalram = 0;
        unsigned long freeram = 0;

        if( sysinfo(&info) != 0 )
        {
            return;
        }

        totalram = (info.totalram * info.mem_unit) / 1024;
        freeram = (info.freeram * info.mem_unit ) / 1024;

        LOGOUT(FILTER_LOG, (char*)"SYSINFO MEMORY total=%ld, free=%ld, use=%ld \n",
                    totalram,
                    freeram,
                    totalram - freeram);
    }
#endif
#if 0
    {
        FILE *fp;
        char command[256];
        char vmsize[256];
        char vmhwm[256];
        char vmrss[256];
        sprintf(command, "grep VmSize /proc/%d/status", getpid());
        if ((fp = popen(command, "r")) == NULL) {
            /*Failure*/
            return;
        }

        while (fgets(vmsize, 256, fp) != NULL) {
        }
        trim(vmsize);

        if (pclose(fp) == -1) {
            /*Failure*/
        }
        sprintf(command, "grep VmHWM /proc/%d/status", getpid());
        if ((fp = popen(command, "r")) == NULL) {
            /*Failure*/
            return;
        }

        while (fgets(vmhwm, 256, fp) != NULL) {
        }
        trim(vmhwm);

        if (pclose(fp) == -1) {
            /*Failure*/
        }

        sprintf(command, "grep VmRSS /proc/%d/status", getpid());
        if ((fp = popen(command, "r")) == NULL) {
            /*Failure*/
            return;
        }

        while (fgets(vmrss, 256, fp) != NULL) {
        }
        trim(vmrss);

        if (pclose(fp) == -1) {
            /*Failure*/
        }

        sprintf(command, "grep /proc/%d/status", getpid());
        LOGOUT(FILTER_LOG, (char*)"%s : vmhwm=%s, vmrss=%s \n",
                    command, vmhwm, vmrss);
    }
#endif
#endif

    return;
};

// 文字列 s を前方トリムして返します．
char *ltrim(char *s)
{
    if (s && *s) {
        size_t len = strlen(s);
        size_t lws = strspn(s, " \n\r\t\v");
        memmove(s, s + lws, len -= lws);
        s[len] = '\0';
    }
    return s;
}

// 文字列 s を後方トリムして返します．
char *rtrim(char *s)
{
    if (s && *s) {
        size_t len = strlen(s);
        while (len && isspace(s[len-1])) --len;
        s[len] = '\0';
    }
    return s;
}
// 文字列 s をトリムして返します．
char *trim(char *s)
{
  return ltrim(rtrim(s));
}


} // end of namespace filter
} // end of namespace pbvr



