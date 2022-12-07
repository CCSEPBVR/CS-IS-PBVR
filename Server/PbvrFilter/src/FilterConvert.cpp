/*
 * FilterConvert.cpp
 *
 *  Created on: 2016/11/21
 *      Author: hira
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "FilterConvert.h"
#include "filter_log.h"
#include "filter_utils.h"
#include "filter_tree.h"
#include "FilterDivision.h"
#include "FilterSubVolume.h"
#include "FilterWrite.h"

namespace pbvr
{
namespace filter
{

const int    FilterConvert::m_elementtypeno[] = { 0,     /*  0: point  */
                                          1,     /*  1: line   */
                                          2,     /*  2: tri    */
                                          3,     /*  3: quad   */
                                          4,     /*  4: tetra  */
                                          5,     /*  5: pyr    */
                                          6,     /*  6: prism  */
                                          7,     /*  7: hex    */
                                          8,     /*  8: line2  */
                                          9,     /*  9: tri2   */
                                         10,     /* 10: quad2  */
                                         11,     /* 11: tetra2 */
                                         12,     /* 12: pyr2   */
                                         13,     /* 13: prism2 */
                                         14};    /* 14: hex2   */

FilterConvert::FilterConvert(PbvrFilterInfo *filter_info, PbvrFilter *parent)
    : m_filter_info(filter_info), m_parent(parent) {

    this->m_io_plot3d = new FilterIoPlot3d(filter_info);
    this->m_io_avs = new FilterIoAvs(filter_info);
    this->m_io_stl = new FilterIoStl(filter_info);
#ifdef VTK_ENABLED
    this->m_io_vtk = new FilterIoVtk(filter_info);
#endif

}

FilterConvert::~FilterConvert() {
    if (this->m_io_plot3d != NULL) delete this->m_io_plot3d;
    if (this->m_io_avs != NULL) delete this->m_io_avs;
    if (this->m_io_stl != NULL) delete this->m_io_stl;
#ifdef VTK_ENABLED
    if (this->m_io_vtk != NULL) delete this->m_io_vtk;
#endif
}

int FilterConvert::realloc_work(Int64 *prov_len, Int64 new_len) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;

    int *p;
    p = (int *) realloc(this->m_filter_info->m_subvolume_nodes, sizeof (int)*new_len);
    if (NULL == p) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_nodes).\n");
        return (RTC_NG);
    }
    this->m_filter_info->m_subvolume_nodes = p;
    p = (int *) realloc(this->m_filter_info->m_subvolume_trans, sizeof (int)*new_len);
    if (NULL == p) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_trans).\n");
        return (RTC_NG);
    }
    this->m_filter_info->m_subvolume_trans = p;
    *prov_len = new_len;
    return (RTC_OK);
}
#ifdef MPI_EFFECT
/*****************************************************************************/
void FilterConvert::bcast_param(int mype) {

    if (this->m_filter_info == NULL) return;
    if (this->m_filter_info->m_param == NULL) return;
    FilterParam *param = this->m_filter_info->m_param;

    int i, j;
    Fdetail *fdc;
    Fdetail *fdv;
    Stepfile *sf;

    MPI_Bcast(&param->in_dir, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->in_prefix, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->in_suffix, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->field_file, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->ucd_inp, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->geom_file, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->out_dir, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->out_prefix, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->out_suffix, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->format, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->tmp_dir, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->start_step, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->end_step, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->n_layer, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->component, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->elem_ave, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->out_num, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->file_type, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->nstep, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->ndim, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->dim1, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->dim2, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->dim3, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->nspace, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->veclen, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->data, 16, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->field, 16, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->coordfile, 512 * 3, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->coordtype, 512 * 3, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->coordskip, 3, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->coordstride, 3, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->valuefile, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->valuetype, 512, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->valueskip, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->struct_grid, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->explicit_n_layer, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->nodesearch, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->headfoot, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->avsucd, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->mult_element_type, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->temp_delete, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->mpi_volume_div, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->mpi_step_div, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->mpi_div, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->min_ext, 3, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->max_ext, 3, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->blocksize, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    if (param->struct_grid) {
        if (mype > 0) {
            sf = (Stepfile*) malloc(sizeof (Stepfile) * param->nstep);
            for (i = 0; i < param->nstep; i++) {
                fdc = (Fdetail*) malloc(sizeof (Fdetail) * param->ndim);
                fdv = (Fdetail*) malloc(sizeof (Fdetail) * param->veclen);
                for (j = 0; j < param->nspace; j++) {
                    memset(fdc[j].fname, 0x00, LINELEN_MAX);
                }
                for (j = 0; j < param->veclen; j++) {
                    memset(fdv[j].fname, 0x00, LINELEN_MAX);
                }
                sf[i].coord = fdc;
                sf[i].value = fdv;
            }
            param->stepf = sf;
        }

        if (param->input_format== AVS_INPUT){
            for (i = 0; i < param->nstep; i++) {
                for (j = 0; j < param->nspace; j++) {
                    MPI_Bcast(&param->stepf[i].coord[j].fname, LINELEN_MAX, MPI_CHAR, 0, MPI_COMM_WORLD);
                    MPI_Bcast(&param->stepf[i].coord[j].ftype, 16, MPI_CHAR, 0, MPI_COMM_WORLD);
                    MPI_Bcast(&param->stepf[i].coord[j].skip, 1, MPI_INT, 0, MPI_COMM_WORLD);
                    MPI_Bcast(&param->stepf[i].coord[j].offset, 1, MPI_INT, 0, MPI_COMM_WORLD);
                    MPI_Bcast(&param->stepf[i].coord[j].stride, 1, MPI_LONG, 0, MPI_COMM_WORLD);
                    MPI_Bcast(&param->stepf[i].coord[j].close, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
                }
                for (j = 0; j < param->veclen; j++) {
                    MPI_Bcast(&param->stepf[i].value[j].fname, LINELEN_MAX, MPI_CHAR, 0, MPI_COMM_WORLD);
                    MPI_Bcast(&param->stepf[i].value[j].ftype, 16, MPI_CHAR, 0, MPI_COMM_WORLD);
                    MPI_Bcast(&param->stepf[i].value[j].skip, 1, MPI_INT, 0, MPI_COMM_WORLD);
                    MPI_Bcast(&param->stepf[i].value[j].offset, 1, MPI_INT, 0, MPI_COMM_WORLD);
                    MPI_Bcast(&param->stepf[i].value[j].stride, 1, MPI_LONG, 0, MPI_COMM_WORLD);
                    MPI_Bcast(&param->stepf[i].value[j].close, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
                }
            }
        }

    }
    // add by @hira at 2016/12/01
    MPI_Bcast(&param->division_type, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&param->n_division, 1, MPI_INT, 0, MPI_COMM_WORLD);

}
#endif

int FilterConvert::separate_files() {
    Int64 i;
    int subvolume_id;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;
    Int64 rank_nodes;
    Int64 rank_elems;
    int step_loop;
    int elem_loop;
    int rstat = RTC_NG;
    Int64 size = 0;

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    FilterIoAvs *io_avs = new FilterIoAvs(info);

    LOGOUT(FILTER_LOG, (char*)"|||| read multi ucd binary data start.\n");
    /* 非構造格子                         */
    /* 節点情報・要素構成情報の読み込み */
    if (info->m_ucdbin) { // read the binary ucd file
        rstat = io_avs->read_ucdbin_geom();
    } else {
        rstat = io_avs->skip_ucd_ascii(1);
        rstat = io_avs->read_ucd_ascii_geom();
        rstat = io_avs->read_ucd_ascii_value(-1);
        fseek(info->m_ucd_ascii_fp, 0, SEEK_SET);
    }
    if (RTC_OK != rstat) {
        LOGOUT(FILTER_ERR, (char*)"read_ucd_ascii_geom error.\n");
        goto garbage;
    }
    LOGOUT(FILTER_LOG, (char*)"|||| read multi ucd binary data end.\n");
    if (param->nodesearch) {
        if (RTC_OK != connect_sort()) {
            LOGOUT(FILTER_ERR, (char*)"connect_sort error.\n");
            goto garbage;
        }
        param->nodesearch = (char) 0;
    }

    rank_elems = 0;
    for (i = 0; i < MAX_ELEMENT_TYPE; i++) {
        rank_elems += info->m_types[i];
    }
    LOGOUT(FILTER_LOG, (char*)"|||  rank_elems = %d\n", rank_elems);

    if (info->m_elems_per_subvolume != NULL) free(info->m_elems_per_subvolume);
    info->m_elems_per_subvolume = (unsigned int *) malloc(sizeof (int)*MAX_ELEMENT_TYPE);
    if (NULL == info->m_elems_per_subvolume) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_elems_per_subvolume).\n");
        goto garbage;
    }
    if (info->m_conns_per_subvolume != NULL) free(info->m_conns_per_subvolume);
    info->m_conns_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*MAX_ELEMENT_TYPE);
    if (NULL == info->m_conns_per_subvolume) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_conns_per_subvolume).\n");
        goto garbage;
    }
    if (info->m_nodes_per_subvolume != NULL) free(info->m_nodes_per_subvolume);
    info->m_nodes_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*MAX_ELEMENT_TYPE);
    if (NULL == info->m_nodes_per_subvolume) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodes_per_subvolume).\n");
        goto garbage;
    }

    info->prov_len = info->m_nnodes;
    info->prov_len = info->m_nnodes / MAX_ELEMENT_TYPE * 2;

    LOGOUT(FILTER_LOG, (char*)"|||  prov_len = %d\n", info->prov_len);

    if (info->m_subvolume_nodes != NULL) free(info->m_subvolume_nodes);
    info->m_subvolume_nodes = (int *) malloc(sizeof (int)*info->prov_len);
    if (NULL == info->m_subvolume_nodes) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_nodes).\n");
        goto garbage;
    }
    if (info->m_subvolume_trans != NULL) free(info->m_subvolume_trans);
    info->m_subvolume_trans = (int *) malloc(sizeof (int)*info->prov_len);
    if (NULL == info->m_subvolume_trans) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_trans).\n");
        goto garbage;
    }
    info->prov_len_e = rank_elems;
    LOGOUT(FILTER_LOG, (char*)"|||  param->mult_element_type   = %d\n", param->mult_element_type);
    LOGOUT(FILTER_LOG, (char*)"|||  prov_len_e = %d\n", info->prov_len_e);

    if (info->m_subvolume_elems != NULL) free(info->m_subvolume_elems);
    info->m_subvolume_elems = (int *) malloc(sizeof (int)*info->prov_len_e);
    if (NULL == info->m_subvolume_elems) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_elems).\n");
        goto garbage;
    }
    if (info->m_nodes_trans != NULL) free(info->m_nodes_trans);
    info->m_nodes_trans = (int *) malloc(sizeof (int)*info->m_nnodes);
    if (NULL == info->m_nodes_trans) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodes_trans).\n");
        goto garbage;
    }
    if (info->m_nodeflag != NULL) free(info->m_nodeflag);
    info->m_nodeflag = (char *) malloc(sizeof (char)*info->m_nnodes);
    if (NULL == info->m_nodeflag) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodeflag).\n");
        goto garbage;
    }

    for (i = 0; i < MAX_ELEMENT_TYPE; i++) {
        //      m_elems_per_subvolume[i] = 0;
        info->m_conns_per_subvolume[i] = 0;
        info->m_nodes_per_subvolume[i] = 0;
    }
    for (i = 0; i < info->m_nnodes; i++) {
        info->m_nodeflag[i] = 0;
    }

    rank_nodes = 0;
    rank_elems = 0;
    LOGOUT(FILTER_LOG, (char*)"|||  count_in_elements start.\n");
    for (elem_loop = 0; elem_loop < MAX_ELEMENT_TYPE; elem_loop++) {
        /* サブボリューム内の要素・節点の判別 */
        if (RTC_OK != count_in_elements(
                            elem_loop,
                            &subvolume_elems,
                            &subvolume_nodes,
                            &subvolume_conns,
                            &info->prov_len,
                            rank_nodes,
                            rank_elems,
                            info)) {
            LOGOUT(FILTER_ERR, (char*)"count_in_subvolume error.\n");
            goto garbage;
        }
        LOGOUT(FILTER_LOG, (char*)"|||   elem_loop : %7d , subvolume_elems : %7d\n", elem_loop, subvolume_elems);
        info->m_elems_per_subvolume[elem_loop] = subvolume_elems;
        info->m_nodes_per_subvolume[elem_loop] = subvolume_nodes;
        info->m_conns_per_subvolume[elem_loop] = subvolume_conns;
        rank_nodes += info->m_nodes_per_subvolume[elem_loop];
        rank_elems += info->m_elems_per_subvolume[elem_loop];
    }
    LOGOUT(FILTER_LOG, (char*)"|||  count_in_elements ended.\n");

    info->m_nsteps = param->end_step - param->start_step + 1;
    if (info->m_value_max != NULL) free(info->m_value_max);
    info->m_value_max = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->m_value_max) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_value_min).\n");
        goto garbage;
    }
    if (info->m_value_min != NULL) free(info->m_value_min);
    info->m_value_min = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->m_value_min) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_value_min).\n");
        goto garbage;
    }
    if (info->mt_nulldata != NULL) free(info->mt_nulldata);
    info->mt_nulldata = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->mt_nulldata) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mt_nulldata).\n");
        goto garbage;
    }
    if (info->mt_nullflag != NULL) free(info->mt_nullflag);
    info->mt_nullflag = (int *) malloc(sizeof (int)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->mt_nullflag) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mt_nullflag).\n");
        goto garbage;
    }
    for (i = 0; i < info->m_nkinds * info->m_nsteps; i++) {
        info->m_value_max[i] = 0.0;
        info->m_value_min[i] = 0.0;
    }
    if (info->m_values != NULL) free(info->m_values);
    info->m_values = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nnodes);
    size = info->getNodeCount();
    // info->m_values = (float *) malloc(sizeof (float)*size);
    if (NULL == info->m_values) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
        goto garbage;
    }
    if (info->m_nelemcomponents > 0) {
        if (info->m_elem_in_node != NULL) free(info->m_elem_in_node);
        if (info->m_elem_in_node_addr != NULL) free(info->m_elem_in_node_addr);
        info->m_elem_in_node = (int *) malloc(sizeof (int)*info->m_nnodes);
        info->m_elem_in_node_addr = (int *) malloc(sizeof (int)*info->m_nnodes);
        memset(info->m_elem_in_node, 0x00, sizeof (float)*info->m_nnodes);
        LOGOUT(FILTER_LOG, (char*)"|||  set_element_in_node start.\n");
        if (RTC_OK != set_element_in_node(info)) {
            LOGOUT(FILTER_ERR, (char*)"set_element_in_node error.\n");
            goto garbage;
        }
        LOGOUT(FILTER_LOG, (char*)"|||  set_element_in_node ended.\n");
    }
    for (step_loop = param->start_step; step_loop <= param->end_step; step_loop++) {
#ifdef MPI_EFFECT
        if (info->mype != (step_loop - param->start_step) % info->numpe) {
            continue;
        }
#endif
        LOGOUT(FILTER_LOG, (char*)"<><> time_step %d start.\n", step_loop);
        if (strlen(param->ucd_inp) != 0) {
            if (info->m_ucdbin == 0) {
                rstat = io_avs->skip_ucd_ascii(step_loop);
                if (info->m_cycle_type == 1) {
                    rstat = io_avs->skip_ucd_ascii_geom();
                } else if (info->m_cycle_type == 2) {
                    if (step_loop == param->start_step) {
                        rstat = io_avs->skip_ucd_ascii_geom();
                    }
                }
                rstat = io_avs->read_ucd_ascii_value(step_loop);
            } else {
                rstat = io_avs->read_ucdbin_data(step_loop);
            }
        } else {
            rstat = io_avs->read_ucdbin_data(step_loop);
        }
        if (RTC_OK != rstat) {
            LOGOUT(FILTER_ERR, (char*)"read_ucdbin_data error.\n");
            goto garbage;
        }
        rank_nodes = 0;
        rank_elems = 0;
        for (elem_loop = 0; elem_loop < MAX_ELEMENT_TYPE; elem_loop++) {
            subvolume_id = info->m_tree_node_no - info->m_nvolumes + elem_loop;
            /* サブボリューム内の要素・節点の判別 */
            subvolume_elems = info->m_elems_per_subvolume[elem_loop];
            subvolume_nodes = info->m_nodes_per_subvolume[elem_loop];
            subvolume_conns = info->m_conns_per_subvolume[elem_loop];
            /* サブボリューム内の要素・節点の判別 */
            if (RTC_OK != set_nodes_in_subvolume(
                                subvolume_nodes,
                                rank_nodes,
                                info)) {
                LOGOUT(FILTER_ERR, (char*)"set_node_in_subvolume error.\n");
                goto garbage;
            }

            if (info->m_types[elem_loop] != 0) {
                if (RTC_OK != io_avs->write_ucdbin_elem(step_loop,
                        elem_loop,
                        subvolume_elems,
                        subvolume_nodes,
                        rank_nodes,
                        rank_elems)) {
                    LOGOUT(FILTER_ERR, (char*)"write_ucdbin error.\n");
                    goto garbage;
                }
            }
            rank_nodes += info->m_nodes_per_subvolume[elem_loop];
            rank_elems += info->m_elems_per_subvolume[elem_loop];
            for (i = 0; i < info->m_nnodes; i++) {
                info->m_nodes_trans[i] = 0;
            }
        }
        LOGOUT(FILTER_LOG, (char*)"<><> time_step %d ended.\n", step_loop);
    }
#ifdef MPI_EFFECT
    MPI_Barrier(MPI_COMM_WORLD);
#endif
    rstat = RTC_OK;
    garbage:
    subvolume_elems = 0;
    subvolume_nodes = 0;
    subvolume_conns = 0;
    rank_nodes = 0;
    rank_elems = 0;

    initialize_memory();
    initialize_value();

    if (io_avs != NULL) delete io_avs;

    return (rstat);
}


/*****************************************************************************/

int FilterConvert::read_struct_grid(int numpe_f, int mype_f
#ifdef MPI_EFFECT
        , MPI_Comm mpi_comm
#endif
    ) {
    Int64 ii, jj, kk, id;
    int rstat = RTC_NG;

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    if (!strstr(param->field, "uniform")) {
        if (mype_f == 0) {
            LOGOUT(FILTER_LOG, (char*)"|||| read geom data start.\n");
            /* Octree生成のための節点情報読み込み */
            if (param->input_format == VTK_INPUT) {

#ifdef VTK_ENABLED
                rstat = this->m_io_vtk->vtk_read_structured_coordinates();
#endif
            } else if (param->input_format == PLOT3D_INPUT) {
                rstat = this->m_io_plot3d->plot3d_getBlockCoordinates();
            } else if (strlen(param->coordfile[0]) > 0) {
                rstat = this->m_io_avs->read_coord_1(0);
            } else {
                rstat = this->m_io_avs->read_geom();
            }
            LOGOUT(FILTER_LOG, (char*)"|||| read geom data end.\n");
            if (RTC_OK != rstat) {
                LOGOUT(FILTER_ERR, (char*)"filter_binary error.\n");
                goto garbage;
            }
        }
#ifdef MPI_EFFECT
        if (numpe_f > 1) {
            /* 本来の座標値に更新 */
            //          MPI_Bcast(m_coords,      m_nnodes*3,   MPI_FLOAT,0,MPI_COMM_FILTER);
            for (ii = 0; ii < 3; ii++) {
                MPI_Bcast(&info->m_coords[ii * info->m_nnodes], info->m_nnodes, MPI_FLOAT, 0, mpi_comm);
            }
        }
#endif
    } else {
        if (param->min_ext[0] == 0.0 && param->min_ext[1] == 0.0 && param->min_ext[2] == 0.0 &&
                param->max_ext[0] == 0.0 && param->max_ext[1] == 0.0 && param->max_ext[2] == 0.0) {
            param->min_ext[0] = 0.0;
            param->min_ext[1] = 0.0;
            param->min_ext[2] = 0.0;
            param->max_ext[0] = param->dim1 - 1;
            param->max_ext[1] = param->dim2 - 1;
            param->max_ext[2] = param->dim3 - 1;
        }
        if (param->ndim == 3) {
            for (kk = 0; kk < param->dim3; kk++) {
                for (jj = 0; jj < param->dim2; jj++) {
                    for (ii = 0; ii < param->dim1; ii++) {
                        id = ii + (param->dim1) * jj + (param->dim1)*(param->dim2) * kk;
                        info->m_coords[id * 3 + 0] = param->min_ext[0]+(param->max_ext[0] - param->min_ext[0]) / (param->dim1 - 1) * ii;
                        info->m_coords[id * 3 + 1] = param->min_ext[1]+(param->max_ext[1] - param->min_ext[1]) / (param->dim2 - 1) * jj;
                        info->m_coords[id * 3 + 2] = param->min_ext[2]+(param->max_ext[2] - param->min_ext[2]) / (param->dim3 - 1) * kk;
                    }
                }
            }
        } else if (param->ndim == 2) {
            for (jj = 0; jj < param->dim2; jj++) {
                for (ii = 0; ii < param->dim1; ii++) {
                    id = ii + (param->dim1) * jj;
                    info->m_coords[id * 3 + 0] = param->min_ext[0]+(param->max_ext[0] - param->min_ext[0]) / (param->dim1 - 1) * ii;
                    info->m_coords[id * 3 + 1] = param->min_ext[1]+(param->max_ext[1] - param->min_ext[1]) / (param->dim2 - 1) * jj;
                    info->m_coords[id * 3 + 2] = 0;
                }
            }
        } else {
            ;
        }
    }

    rstat = RTC_OK;
garbage:
    return (rstat);
}

/*****************************************************************************/

int FilterConvert::connect_sort(void) {
    unsigned int div, mod, ista, iend;
    int i, j;
	int k;

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    div = info->m_connectsize / info->numpe;
    mod = info->m_connectsize % info->numpe;
    if (mod > 0) div++;
    ista = info->mype * div;
    iend = ista + div;
    if (iend > info->m_connectsize) {
        iend = info->m_connectsize;
    }
    if ((iend - ista) > 0) {
        info->m_connections_o = info->m_connections;
        info->m_connections = (int *) malloc(sizeof (int)*info->m_connectsize);
        if (NULL == info->m_connections) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_connections).\n");
            return RTC_NG;
        }
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,j)
#endif
        for (i = 0; i < info->m_connectsize; i++) {
            info->m_connections[i] = 0;
        }
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,k)
#endif
        for (k = ista; k < iend; k++) {
            for (j = 0; j < info->m_nnodes; j++) {
                if (info->m_connections_o[k] == info->m_ids[j]) {
                    info->m_connections[k] = j + 1;
                    break;
                }
            }
        }
        free(info->m_connections_o);
        info->m_connections_o = NULL;
    }
#ifdef MPI_EFFECT
    MPI_Allreduce(MPI_IN_PLACE, info->m_connections, info->m_connectsize, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
#endif
    return RTC_OK;
}

/*****************************************************************************/

int FilterConvert::initialize_memory(void) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;

    DEBUG(3, "!!!!!! initialize_memory (Free if NULL) !!!!!!!\n");

    // メモリ開放
    release_PbvrFilterInfo(this->m_filter_info);

    return (RTC_OK);
}

int FilterConvert::initialize_value(void) {
    DEBUG(3, "!!!!!!!!!!initialize_value (Set To NULL) !!!!!\n");

    // 変数初期化
    initialize_PbvrFilterInfo(this->m_filter_info);

    return (RTC_OK);
}



/*
 * UCD コントロールファイル読込み
 *
 */
int FilterConvert::read_inp_file( void )
{
    this->m_filter_info->m_cycle_type = 0;
    int          state = RTC_NG;
    FILE        *ifs = NULL;
    char        *ch;
    char         line [LINELEN_MAX];
//    char         alloc_flag;
    int          i;
    int          count;
//    int          do_step;
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;

    count = 0;
//    do_step = 0;
//    alloc_flag = 0;
    char *filename = this->m_filter_info->filename;
    sprintf(filename, "%s/%s",param->in_dir, param->ucd_inp );
    ifs = fopen(filename,"rb");
    if(ifs == NULL) {
        LOGOUT(FILTER_ERR,(char*)"Can't open file : %s\n",filename);
        return state;
    }

    param->nstep = 1;

    this->m_filter_info->m_ucdbin = 0;
    count = 0;
    while(fgets(line,LINELEN_MAX,ifs) != NULL) {
        ch=line;
        if(*ch=='\n'){
            continue;
        }
        if(*ch=='#'){
            continue;
        }
        if(count==0) {
            if(strstr(line,"data_geom")){
                this->m_filter_info->m_cycle_type = 1;
                this->m_filter_info->m_ucdbin = 1;
            }else if(strstr(line,"data")){
                this->m_filter_info->m_cycle_type = 2;
                this->m_filter_info->m_ucdbin = 1;
            }else if(strstr(line,"geom")){
                this->m_filter_info->m_cycle_type = 3;
                this->m_filter_info->m_ucdbin = 1;
            }else{
                param->nstep = atoi(line);
                this->m_filter_info->m_ucdbin = 0;
            }
        }
        count++;
    }
    fseek(ifs,0,SEEK_SET);

    /* エラー */
    if(this->m_filter_info->m_ucdbin){
        LOGOUT(FILTER_ERR,(char*)"Not support ucd-binary by avsinp-file : %s\n",filename);
        return state;
    }

    if(this->m_filter_info->m_ucdbin) {
        this->m_filter_info->m_ucdbinfile = (char**)malloc(sizeof(char*)*count);
        this->m_filter_info->m_ucdbinfile_size = count;
        for(i=0;i<count;i++){
            this->m_filter_info->m_ucdbinfile[i] = (char*)malloc(sizeof(char)*LINELEN_MAX);
            memset(this->m_filter_info->m_ucdbinfile[i],0x00,sizeof(char)*LINELEN_MAX);
        }
        count = 0;
        while(fgets(line,LINELEN_MAX,ifs) != NULL) {
            ch=line;
            if(*ch=='\n'){
                continue;
            }
            if(*ch=='#'){
                continue;
            }
            if(count==0) {
                if(strstr(line,"data_geom")){
                    this->m_filter_info->m_cycle_type = 1;
                }else if(strstr(line,"data")){
                    this->m_filter_info->m_cycle_type = 2;
                }else if(strstr(line,"geom")){
                    this->m_filter_info->m_cycle_type = 3;
                }else{
                    param->nstep = atoi(line);
                    ;
                }
            }else{
                for(i=0;i<LINELEN_MAX;i++){
                    if(line[i]=='\n'){
                        break;
                    }
                    this->m_filter_info->m_ucdbinfile[count-1][i]=line[i];
                }
            }
            count++;
        }
        fclose(ifs);
    }else{
        count = 0;
        while(fgets(line,LINELEN_MAX,ifs) != NULL) {
            ch=line;
            if(*ch=='\n'){
                continue;
            }
            if(*ch=='#'){
                continue;
            }
            if(count==0) {
                param->nstep = atoi(line);
                this->m_filter_info->m_ucdbin = 0;
            }else{
                if(strstr(line,"data_geom")){
                    this->m_filter_info->m_cycle_type = 1;
                }else if(strstr(line,"data")){
                    this->m_filter_info->m_cycle_type = 2;
                }else if(strstr(line,"geom")){
                    this->m_filter_info->m_cycle_type = 3;
                }else{
                    //;
                    /*
                    ここでサイクルタイプの記述がなかった場合旧式フォーマットと判定
                    旧フォーマットであった場合はステップ数を1とする(もし2以上のステップ数が来た場合は別途処理が必要になる。
                    またサイクルタイプはdataで固定される。
                    */
                    if(this->m_filter_info->m_cycle_type == 0){
                        std::cout << "Detected an old type of AVS format." << std::endl;
                        param->nstep = 1;
                        this->m_filter_info->m_cycle_type = 2;
                        this->m_filter_info->m_old_avs_format = 1;
                    }
                }
            }
            count++;
            if(count>1){
                break;
            }
        }
        this->m_filter_info->m_ucd_ascii_fp = ifs;
    }

    state = RTC_OK;
    return(state);
}

int FilterConvert::filter_convert() {

    int rcode = RTC_NG;
    int step_loop;
    int volm_loop;
    int fnum_loop;
    int rstat;
    int count;
    int i, j, k, itmp;
    Int64 ii;
#ifdef MPI_EFFECT
    //  int         *mpiwork;
    int color;
    int key;
#endif
    int subvolume_id;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;
    Int64 rank_nodes;
    Int64 rank_elems;
    Int64 total_nodes;
    Int64 total_elems;
    unsigned int *all_subvolume_nodes = NULL;
    unsigned int *all_subvolume_trans = NULL;
    unsigned int *all_subvolume_elems = NULL;
    int ista;
    int id;
    int element_type_num;
    char first_step;
    char m_out_prefix[LINELEN_MAX];
    char m_out_dir[LINELEN_MAX];
    char m_in_prefix[LINELEN_MAX];
    char m_in_dir[LINELEN_MAX];
    unsigned int *mpiwork = NULL;
    Int64 size = 0;
    bool avs_unstruct_multi_type = false;

#ifdef MPI_EFFECT
    MPI_Comm MPI_COMM_FILTER = MPI_COMM_NULL;
    MPI_Comm MPI_COMM_STEP = MPI_COMM_NULL;
    MPI_Comm MPI_COMM_VOLM = MPI_COMM_NULL;
    MPI_Comm m_mpi_comm = MPI_COMM_NULL;
#endif

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    FilterWrite *writer = new FilterWrite(info);

    // Set all values 0/NULL
    initialize_value();

    subvolume_elems = 0;
    subvolume_nodes = 0;
    subvolume_conns = 0;
    all_subvolume_nodes = NULL;
    all_subvolume_trans = NULL;
    all_subvolume_elems = NULL;

    for (i = 0; i < MAX_ELEMENT_TYPE; i++) {
        info->m_types[i] = 0;
    }

    // Set default values, related to OPENMP/MPI
    first_step = 1;

    /* 引数チェック */
    //    if(2 != argc){
    //        fprintf(stderr, "usage:%s [paramfile].\n", argv[0]);
    //        return(-1);
    //    }

    info->numpe = 1;
    info->mype = 0;
    info->numpe_f = 1;
    info->mype_f = 0;
    info->numpe_s = 1;
    info->mype_s = 0;
    info->numpe_v = 1;
    info->mype_v = 0;
    info->mpidiv_v = 1;
    info->mpidiv_s = 1;
    info->m_ucdbin = 1;

#ifdef MPI_EFFECT
    MPI_Comm_size(MPI_COMM_WORLD, &(info->numpe));
    MPI_Comm_rank(MPI_COMM_WORLD, &(info->mype));
    m_mpi_comm = MPI_COMM_WORLD;
#endif

    FilterLog::getInstance().myrank = info->mype;

    LOGOUT(FILTER_LOG, (char*)"\n\n");
    LOGOUT(FILTER_LOG, (char*)"#### numpe   : %3d, mype   : %3d\n", info->numpe, info->mype);

#ifdef _OPENMP
#pragma omp parallel default(shared)
    {
        LOGOUT(FILTER_LOG, (char*)"#### Thread parallel : %d / %d\n", omp_get_thread_num(), omp_get_num_threads());
    }
#endif

#ifdef MPI_EFFECT
    if (info->mype == 0) {
#endif
        /* パラメータファイルの読み込み */
        //        if(RTC_OK!=read_param_file(argv[1])){
        //            LOGOUT(FILTER_ERR, (char*)"read_filter_param error.\n");
        //            goto garbage;
        //        }
#ifdef MPI_EFFECT
    } // if (info->mype==0)

#endif
    FilterDivision *division = factoryFilterDivision(param->division_type, info);
    writer->setFilterDivision(division);

    if (strlen(param->ucd_inp) != 0) {
        if (RTC_OK != read_inp_file()) {
            LOGOUT(FILTER_ERR, (char*)"read_ucd_inp_file error.\n");
            goto garbage;
        }
    }
    info->m_multi_element = param->mult_element_type;


    LOGOUT(FILTER_LOG, (char*)"filter_convert::struct_grid      : %d\n", param->struct_grid);
    LOGOUT(FILTER_LOG, (char*)"filter_convert::input_format      : %d\n", param->input_format);

    if (!param->struct_grid && param->input_format == VTK_INPUT) {
#ifdef VTK_ENABLED
        this->m_io_vtk->vtk_count_cellTypes();
        if (param->mult_element_type) {
            sprintf(m_out_prefix, "%s", param->out_prefix);
            sprintf(m_out_dir, "%s", param->out_dir);
            sprintf(m_in_prefix, "%s", param->out_prefix);
            sprintf(m_in_dir, "%s", param->in_dir);
        } else {
            sprintf(m_out_prefix, "%s", "");
        }
#endif
    } else if (!param->struct_grid && param->mult_element_type) {

#ifdef MULTI_SEPARATE_FILES
        avs_unstruct_multi_type = false;
        /* 混合要素のファイル分割 */
        this->separate_files();
        sprintf(m_out_prefix, "%s", param->out_prefix);
        sprintf(m_out_dir, "%s", param->out_dir);
        sprintf(m_in_prefix, "%s", param->out_prefix);
        sprintf(m_in_dir, "%s", param->in_dir);
#else
        avs_unstruct_multi_type = true;
        // 要素タイプを取得する
        this->m_io_avs->read_elementtypes();
#endif
    }

    /********************/
    element_type_num = 0;
    info->element_type_num = element_type_num;

    // Label for goto loop construct.
    // goto condition is  (!param->struct_grid && info->m_multi_element)
unstruct_element_proc:
    if (!param->struct_grid && info->m_multi_element) {
        DEBUG(1, "############ unstructured multi element::::: \n");
        while (info->m_types[element_type_num] == 0) {
            printf(" m_types[%d]:%d\n", element_type_num, info->m_types[element_type_num]);
            element_type_num++;
            if (element_type_num >= MAX_ELEMENT_TYPE) {
                DEBUG(3, "############ u unstruct_element_stop;:::: \n");
                goto unstruct_element_stop;
            }
        }
        info->element_type_num = element_type_num;

        if (param->input_format != VTK_INPUT) {
#ifdef MULTI_SEPARATE_FILES
            sprintf(param->in_dir, "%s", param->out_dir);
            sprintf(param->in_prefix, "%02d-%s", m_elementtypeno[element_type_num], m_out_prefix);
            sprintf(param->out_prefix, "%02d-%s", m_elementtypeno[element_type_num], m_out_prefix);
#endif
        }
#ifdef VTK_ENABLED
        else {
            // modify by @hira at 2016/12/01
            // if (param->mult_element_type) {
            //    sprintf(param->out_prefix, "%02d-%s", info->m_element_type, m_out_prefix);
            //}
            sprintf(param->out_prefix, "%02d-%s", m_elementtypeno[element_type_num], m_out_prefix);
            info->m_element_type = m_elementtypeno[element_type_num];
        }
#endif
        DEBUG(1, "##############>%s\n", param->out_prefix);
    }
    else if (param->input_format == VTK_INPUT) {
        element_type_num= info->m_element_type;
    }
    /********************/
#ifdef MPI_EFFECT
    if (info->mype == 0) {
#endif
        if (param->struct_grid) {

            if (param->input_format == VTK_INPUT) {
#ifdef VTK_ENABLED
                if (RTC_OK != this->m_io_vtk->vtk_setStructGridParameters(param->start_step)) {
                    LOGOUT(FILTER_ERR, (char*)"vtk_read_dims error.\n");
                    goto garbage;
                }
#else
                LOGOUT(FILTER_ERR, (char*)"VTK not enabled in this build.\n");
#endif

            } else if (param->input_format == PLOT3D_INPUT) {
                if (RTC_OK != this->m_io_plot3d->plot3d_setBlockParameters()) {
                    LOGOUT(FILTER_ERR, (char*)"read_filter_fld error.\n");
                    goto garbage;
                };
            }/* 構造格子                           */
            else {
                if (RTC_OK != this->m_io_avs->read_fld_file()) {
                    LOGOUT(FILTER_ERR, (char*)"read_filter_fld error.\n");
                    goto garbage;
                }
            }
        } else {
            if (param->input_format == STL_ASCII || param->input_format == STL_BIN) {
                if (param->input_format == STL_ASCII) {
                    LOGOUT(FILTER_LOG, (char*)"INPUT IS STL ASCII.\n");
                    if (this->m_io_stl->stl_ascii_read_geom() != RTC_OK) {
                        LOGOUT(FILTER_ERR, (char*)"stl_ascii_read_geom error in filter_convert.c .\n");
                        goto garbage;
                    }
                } else if (param->input_format == STL_BIN) {
                    LOGOUT(FILTER_LOG, (char*)"INPUT IS STL BINARY.\n");
                    if (this->m_io_stl->stl_bin_read_geom() != RTC_OK) {
                        LOGOUT(FILTER_ERR, (char*)"stl_bin_read_geom error in filter_convert.c .\n");
                        goto garbage;
                    }
                }

                // contine with setup_stl_geom which is used for the ASCII and binary version
                LOGOUT(FILTER_LOG, (char*)"INPUT IS STL.\n");
                rstat = this->m_io_stl->setup_stl_geom();

            } else if (param->input_format == VTK_INPUT) {
#ifdef VTK_ENABLED
                LOGOUT(FILTER_LOG, (char*)"||   read vtk unstr data start.\n");
                if (RTC_OK != this->m_io_vtk->vtk_readUnstructuredCoordinates(element_type_num)) {
                    LOGOUT(FILTER_ERR, (char*)"vtk_read_structured_points error.[element_type_num=%d]\n", element_type_num);
                    goto garbage;
                }
                LOGOUT(FILTER_LOG, (char*)"||   read vtk unstr data end.\n");
#else
                LOGOUT(FILTER_ERR, (char*)"VTK Not enabled in this build.\n");
#endif

            }
#ifndef MULTI_SEPARATE_FILES
            else if (avs_unstruct_multi_type) {
                // 要素タイプを指定して読み込みを行う  add by @hira at 2017/04/15
                if (RTC_OK != this->m_io_avs->read_unstructer_multi_geom(element_type_num)) {
                    LOGOUT(FILTER_ERR, (char*)"read unstructure multi error.[element_type_num=%d]\n", element_type_num);
                    goto garbage;
                }
            }
#endif
            else {
                /* 非構造格子                         */
                /* Octree生成のための節点情報・要素構成情報の読み込み */
                LOGOUT(FILTER_LOG, (char*)"|||| param->ucd_inp : %s, info->m_multi_element : %d\n", param->ucd_inp, info->m_multi_element);
                if (strlen(param->ucd_inp) != 0 && info->m_multi_element == 0) {
                    if (info->m_ucdbin == 0) {
                        LOGOUT(FILTER_LOG, (char*)"||   read ucd ascii data start.\n");
                        rstat = this->m_io_avs->skip_ucd_ascii(1);
                        rstat = this->m_io_avs->read_ucd_ascii_geom();
                        rstat = this->m_io_avs->read_ucd_ascii_value(-1);
                        fseek(info->m_ucd_ascii_fp, 0, SEEK_SET);
                        LOGOUT(FILTER_LOG, (char*)"||   read ucd ascii data  end.\n");
                    } else {
                        LOGOUT(FILTER_LOG, (char*)"||   read ucd binary data start.\n");
                        rstat = this->m_io_avs->read_ucdbin_geom();
                        LOGOUT(FILTER_LOG, (char*)"||   read ucd binary data end.\n");
                    }
                } else {
                    LOGOUT(FILTER_LOG, (char*)"||   read ucd binary data start.\n");
                    rstat = this->m_io_avs->read_ucdbin_geom();
                    LOGOUT(FILTER_LOG, (char*)"||   read ucd binary data end.\n");
                }
                if (RTC_OK != rstat) {
                    LOGOUT(FILTER_ERR, (char*)"read_ucdbin_geom error.\n");
                    goto garbage;
                }
            }
        }
#ifdef MPI_EFFECT
    } // if (info->mype==0)
#endif

#ifdef MPI_EFFECT
    if (info->numpe > 1) {
        if (first_step) {
            m_mpi_comm = MPI_COMM_WORLD;
        } else {
            m_mpi_comm = MPI_COMM_FILTER;
        }
        bcast_param(info->mype);

        MPI_Bcast(&info->m_title, 70, MPI_CHAR, 0, m_mpi_comm);
        MPI_Bcast(&info->m_stepn, 1, MPI_INT, 0, m_mpi_comm);
        MPI_Bcast(&info->m_stept, 1, MPI_FLOAT, 0, m_mpi_comm);
        MPI_Bcast(&info->m_nnodes, 1, MPI_LONG, 0, m_mpi_comm);
        MPI_Bcast(&info->m_nelements, 1, MPI_LONG, 0, m_mpi_comm);
        MPI_Bcast(&info->m_elemcoms, 1, MPI_INT, 0, m_mpi_comm);
        MPI_Bcast(&info->m_connectsize, 1, MPI_LONG, 0, m_mpi_comm);
        MPI_Bcast(&info->m_nnodecomponents, 1, MPI_INT, 0, m_mpi_comm);
        MPI_Bcast(&info->m_nelemcomponents, 1, MPI_INT, 0, m_mpi_comm);
        MPI_Bcast(&info->m_ncomponents, 1, MPI_INT, 0, m_mpi_comm);
        MPI_Bcast(&info->m_nnodekinds, 1, MPI_INT, 0, m_mpi_comm);
        MPI_Bcast(&info->m_nelemkinds, 1, MPI_INT, 0, m_mpi_comm);
        MPI_Bcast(&info->m_nkinds, 1, MPI_INT, 0, m_mpi_comm);
        MPI_Bcast(&info->m_desctype, 1, MPI_INT, 0, m_mpi_comm);
        MPI_Bcast(&info->m_datatype, 1, MPI_INT, 0, m_mpi_comm);
        MPI_Bcast(&info->m_element_type, 1, MPI_CHAR, 0, m_mpi_comm);
        MPI_Bcast(&info->m_filesize, 1, MPI_LONG, 0, m_mpi_comm);
        MPI_Bcast(&info->m_headfoot, 1, MPI_CHAR, 0, m_mpi_comm);

        if (!param->struct_grid) {
            if (info->mype != 0) {
                if (info->m_coords != NULL) free(info->m_coords);
                info->m_coords = (float *) malloc(sizeof (float)*info->m_nnodes * 3);
                if (NULL == info->m_coords) {
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_coords).\n");
                    goto garbage;
                }
                if (info->m_ids != NULL) free(info->m_ids);
                info->m_ids = (int *) malloc(sizeof (int)*info->m_nnodes);
                if (NULL == info->m_ids) {
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_ids).\n");
                    goto garbage;
                }
                if (info->m_connections != NULL) free(info->m_connections);
                info->m_connections = (int *) malloc(sizeof (int)*info->m_connectsize);
                if (NULL == info->m_connections) {
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_connections).\n");
                    goto garbage;
                }
                if (info->m_veclens != NULL) free(info->m_veclens);
                info->m_veclens = (int *) malloc(sizeof (int)*info->m_ncomponents);
                if (NULL == info->m_veclens) {
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_veclens).\n");
                    goto garbage;
                }
                if (info->m_elemtypes != NULL) free(info->m_elemtypes);
                info->m_elemtypes = (char *) malloc(sizeof (char)*info->m_nelements);
                if (NULL == info->m_elemtypes) {
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_elemtypes).\n");
                    goto garbage;
                }
                if (info->m_elementids != NULL) free(info->m_elementids);
                info->m_elementids = (int *) malloc(sizeof (int)*info->m_nelements);
                if (NULL == info->m_elementids) {
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_elementid).\n");
                    goto garbage;
                }
                if (info->m_materials != NULL) free(info->m_materials);
                info->m_materials = (int *) malloc(sizeof (int)*info->m_nelements);
                if (NULL == info->m_materials) {
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_materials).\n");
                    goto garbage;
                }
                if (info->pm_names != NULL) free(info->pm_names);
                info->pm_names = (char**) malloc(sizeof (char*)*info->m_ncomponents);
                for (i = 0; i < info->m_ncomponents; i++) {
                    info->pm_names[i] = (char*) malloc(sizeof (char)*COMPONENT_LEN);
                }
                if (info->pm_units != NULL) free(info->pm_units);
                info->pm_units = (char**) malloc(sizeof (char*)*info->m_ncomponents);
                for (i = 0; i < info->m_ncomponents; i++) {
                    info->pm_units[i] = (char*) malloc(sizeof (char)*COMPONENT_LEN);
                }
                if (info->m_nullflags != NULL) free(info->m_nullflags);
                info->m_nullflags = (int *) malloc(sizeof (int)*info->m_nkinds);
                if (NULL == info->m_nullflags) {
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nullflags).\n");
                    goto garbage;
                }
                if (info->m_nulldatas != NULL) free(info->m_nulldatas);
                info->m_nulldatas = (float*) malloc(sizeof (float)*info->m_nkinds);
                if (NULL == info->m_nulldatas) {
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nulldatas).\n");
                    goto garbage;
                }
            }
            MPI_Bcast(info->m_elemtypes, info->m_nelements, MPI_CHAR, 0, m_mpi_comm);
            MPI_Bcast(info->m_elementids, info->m_nelements, MPI_INT, 0, m_mpi_comm);
            MPI_Bcast(info->m_materials, info->m_nelements, MPI_INT, 0, m_mpi_comm);
            MPI_Bcast(info->m_ids, info->m_nnodes, MPI_INT, 0, m_mpi_comm);
            MPI_Bcast(info->m_coords, info->m_nnodes * 3, MPI_FLOAT, 0, m_mpi_comm);
            MPI_Bcast(&info->node_min.x, 1, MPI_FLOAT, 0, m_mpi_comm);
            MPI_Bcast(&info->node_min.y, 1, MPI_FLOAT, 0, m_mpi_comm);
            MPI_Bcast(&info->node_min.z, 1, MPI_FLOAT, 0, m_mpi_comm);
            MPI_Bcast(&info->node_max.x, 1, MPI_FLOAT, 0, m_mpi_comm);
            MPI_Bcast(&info->node_max.y, 1, MPI_FLOAT, 0, m_mpi_comm);
            MPI_Bcast(&info->node_max.z, 1, MPI_FLOAT, 0, m_mpi_comm);
            MPI_Bcast(info->m_connections, info->m_connectsize, MPI_INT, 0, m_mpi_comm);
            MPI_Bcast(info->m_veclens, info->m_ncomponents, MPI_INT, 0, m_mpi_comm);

            MPI_Bcast(info->m_nullflags, info->m_nkinds, MPI_INT, 0, m_mpi_comm);
            MPI_Bcast(info->m_nulldatas, info->m_nkinds, MPI_FLOAT, 0, m_mpi_comm);
            for (i = 0; i < info->m_ncomponents; i++) {
                MPI_Bcast(&info->pm_names[i][0], COMPONENT_LEN, MPI_CHAR, 0, m_mpi_comm);
                MPI_Bcast(&info->pm_units[i][0], COMPONENT_LEN, MPI_CHAR, 0, m_mpi_comm);
            }
        }
    } /* if(info->numpe>1) */
#endif

    /* サブボリューム数決定 */
    info->m_nvolumes = division->get_nvolumes();
    info->m_nsteps = param->end_step - param->start_step + 1;
    info->pre_step = param->start_step - 1;
    info->pre_step = -1;
    LOGOUT(FILTER_LOG, (char*)"#### m_nsteps         = %d, m_nvolumes  = %d\n", info->m_nsteps, info->m_nvolumes);
    LOGOUT(FILTER_LOG, (char*)"#### m_ncomponents    = %d, m_nkinds    = %d\n", info->m_ncomponents, info->m_nkinds);
    LOGOUT(FILTER_LOG, (char*)"#### m_nnodecomponents= %d, m_nnodekinds= %d\n", info->m_nnodecomponents, info->m_nnodekinds);
    LOGOUT(FILTER_LOG, (char*)"#### m_nelemcomponents= %d, m_nelemkinds= %d\n", info->m_nelemcomponents, info->m_nelemkinds);

    if (first_step) {
        if (!param->struct_grid && param->nodesearch) {
            if (RTC_OK != connect_sort()) {
                LOGOUT(FILTER_ERR, (char*)"connect_osrt error.\n");
                goto garbage;
            }
            param->nodesearch = (char) 0;
        }
#ifdef MPI_EFFECT
#if 0        // delete by @hira at 2016/12/01
        if (param->mpi_volume_div > 0 && param->mpi_step_div > 0) {
            if (param->mpi_volume_div * param->mpi_step_div != info->numpe) {
                LOGOUT(FILTER_ERR, (char*)"Disagreement. product of mpi_volume_div & mpi_step_div.\n");
                goto garbage;
            }
        }
        if ((param->mpi_volume_div * param->mpi_step_div) > info->numpe) {
            param->mpi_volume_div = 0;
            param->mpi_step_div = 0;
        }
        if (param->mpi_volume_div > 0 && param->mpi_step_div > 0) {
            color = 0;
            if ((param->mpi_volume_div * param->mpi_step_div - 1) < info->mype) {
                color = MPI_UNDEFINED;
            }
            key = info->mype;
            MPI_Comm_split(MPI_COMM_WORLD, color, key, &MPI_COMM_FILTER);
            if (color == MPI_UNDEFINED) {
                goto release;
            }
            MPI_Comm_size(MPI_COMM_FILTER, &info->numpe_f);
            MPI_Comm_rank(MPI_COMM_FILTER, &info->mype_f);

            color = info->mype_f / param->mpi_volume_div;
            key = info->mype_f;
            MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_VOLM);
            MPI_Comm_size(MPI_COMM_VOLM, &info->numpe_v);
            MPI_Comm_rank(MPI_COMM_VOLM, &info->mype_v);

            color = info->mype % param->mpi_volume_div;
            key = info->mype_f;
            MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_STEP);
            MPI_Comm_size(MPI_COMM_STEP, &info->numpe_s);
            MPI_Comm_rank(MPI_COMM_STEP, &info->mype_s);
        } else {

            if (param->mpi_div != 1 && param->mpi_div != 2) {
                if (param->file_type == 1) {
                    param->mpi_div = 1;
                } else {
                    if ((info->numpe % info->m_nvolumes)<(info->numpe % info->m_nsteps)) {
                        param->mpi_div = 1;
                    } else {
                        param->mpi_div = 2;
                    }
                }
            }
            if (param->mpi_div == 1) {
                if (info->m_nvolumes < info->numpe) {
                    color = 0;
                    if (((info->numpe / info->m_nvolumes) * info->m_nvolumes - 1) < info->mype) {
                        color = MPI_UNDEFINED;
                    }
                    key = info->mype;
                    MPI_Comm_split(MPI_COMM_WORLD, color, key, &MPI_COMM_FILTER);
                    if (color == MPI_UNDEFINED) {
                        goto release;
                    }
                    MPI_Comm_size(MPI_COMM_FILTER, &info->numpe_f);
                    MPI_Comm_rank(MPI_COMM_FILTER, &info->mype_f);
                } else {
                    color = 0;
                    key = info->mype;
                    MPI_Comm_split(MPI_COMM_WORLD, color, key, &MPI_COMM_FILTER);
                    MPI_Comm_size(MPI_COMM_FILTER, &info->numpe_f);
                    MPI_Comm_rank(MPI_COMM_FILTER, &info->mype_f);
                }
            } else {
                if (info->m_nsteps < info->numpe) {
                    color = 0;
                    if (((info->numpe / info->m_nsteps) * info->m_nsteps - 1) < info->mype) {
                        color = MPI_UNDEFINED;
                    }
                    key = info->mype;
                    MPI_Comm_split(MPI_COMM_WORLD, color, key, &MPI_COMM_FILTER);
                    if (color == MPI_UNDEFINED) {
                        goto release;
                    }
                    MPI_Comm_size(MPI_COMM_FILTER, &info->numpe_f);
                    MPI_Comm_rank(MPI_COMM_FILTER, &info->mype_f);
                } else {
                    color = 0;
                    key = info->mype;
                    MPI_Comm_split(MPI_COMM_WORLD, color, key, &MPI_COMM_FILTER);
                    MPI_Comm_size(MPI_COMM_FILTER, &info->numpe_f);
                    MPI_Comm_rank(MPI_COMM_FILTER, &info->mype_f);
                }
            }

            if (param->mpi_div == 1) {
                color = info->mype_f / info->m_nvolumes;
                key = info->mype_f;
                MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_VOLM);
                MPI_Comm_size(MPI_COMM_VOLM, &info->numpe_v);
                MPI_Comm_rank(MPI_COMM_VOLM, &info->mype_v);

                color = info->mype % info->m_nvolumes;
                key = info->mype_f;
                MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_STEP);
                MPI_Comm_size(MPI_COMM_STEP, &info->numpe_s);
                MPI_Comm_rank(MPI_COMM_STEP, &info->mype_s);
            } else {
                color = info->mype_f / info->m_nsteps;
                key = info->mype_f;
                MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_STEP);
                MPI_Comm_size(MPI_COMM_STEP, &info->numpe_s);
                MPI_Comm_rank(MPI_COMM_STEP, &info->mype_s);

                color = info->mype % info->m_nsteps;
                key = info->mype_f;
                MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_VOLM);
                MPI_Comm_size(MPI_COMM_VOLM, &info->numpe_v);
                MPI_Comm_rank(MPI_COMM_VOLM, &info->mype_v);
            }
        }
#endif          // delete by @hira at 2016/12/01
        // modify:start by @hira at 2016/12/01
        color = 0;
        key = info->mype;
        MPI_Comm_split(MPI_COMM_WORLD, color, key, &MPI_COMM_FILTER);
        MPI_Comm_size(MPI_COMM_FILTER, &info->numpe_f);
        MPI_Comm_rank(MPI_COMM_FILTER, &info->mype_f);
        LOGOUT(FILTER_LOG, (char*)"[rank=%d/%d] MPI_COMM_FILTER : color=%d, mype_f=%d/%d\n",
                info->mype, info->numpe, color, info->mype_f, info->numpe_f);

        //color = info->mype_f / info->m_nvolumes;
        color = 0;
        if (info->numpe > 1) {
            if (division->getDivisionType() == MASTER_DIVISION) {
                // master-slave方式では、0ランクはサブボリューム対象外
                color = (info->mype_f == 0) ? 0 : 1;
            }
        }
        key = info->mype_f;
        MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_VOLM);
        MPI_Comm_size(MPI_COMM_VOLM, &info->numpe_v);
        MPI_Comm_rank(MPI_COMM_VOLM, &info->mype_v);
        if (info->numpe > 1) {
            if (division->getDivisionType() == MASTER_DIVISION) {
                if (info->mype_f == 0) {
                    // master-slave方式では、0ランクはサブボリューム対象外
                    info->numpe_v = 0;
                    MPI_COMM_VOLM = MPI_COMM_NULL;
                }
            }
        }
        LOGOUT(FILTER_LOG, (char*)"[rank=%d/%d] MPI_COMM_VOLM : color=%d, mype_v=%d/%d\n",
                info->mype, info->numpe, color, info->mype_v, info->numpe_v);

        // color = info->mype % info->m_nvolumes;
        color = 0;
        key = info->mype_f;
        MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_STEP);
        MPI_Comm_size(MPI_COMM_STEP, &info->numpe_s);
        MPI_Comm_rank(MPI_COMM_STEP, &info->mype_s);
        LOGOUT(FILTER_LOG, (char*)"[rank=%d/%d] MPI_COMM_STEP : color=%d, mype_s=%d/%d\n",
                info->mype, info->numpe, color, info->mype_s, info->numpe_s);
        // modify:end by @hira at 2016/12/01

        MPI_Allreduce(&info->numpe_s, &info->mpidiv_s, 1, MPI_INT, MPI_MAX, MPI_COMM_FILTER);
        MPI_Allreduce(&info->numpe_v, &info->mpidiv_v, 1, MPI_INT, MPI_MAX, MPI_COMM_FILTER);
        LOGOUT(FILTER_LOG, (char*)"#### numpe_f  : %5d, mype_f   : %5d\n", info->numpe_f, info->mype_f);
        LOGOUT(FILTER_LOG, (char*)"#### numpe_s  : %5d, mype_s   : %5d\n", info->numpe_s, info->mype_s);
        LOGOUT(FILTER_LOG, (char*)"#### numpe_v  : %5d, mype_v   : %5d\n", info->numpe_v, info->mype_v);
        LOGOUT(FILTER_LOG, (char*)"#### mpidiv_s : %5d, mpidiv_v : %5d\n", info->mpidiv_s, info->mpidiv_v);
#endif
        first_step = 0;
    }

    if (param->struct_grid) {
        // Construct array of 3 file pointers
        info->m_geom_fp = (FILE **) malloc(sizeof (FILE*)*3);
        for (i = 0; i < 3; i++) {
            info->m_geom_fp[i] = NULL;
        }
        // Construct array of [m_nkinds] file pointers
        info->m_value_fp = (FILE **) malloc(sizeof (FILE*) * info->m_nkinds);
        for (i = 0; i < info->m_nkinds; i++) {
            info->m_value_fp[i] = NULL;
        }
        // Set element type to 'Quad' or 'Hexahedra', based on dimension
        if (param->ndim == 2) {
            info->m_element_type = 3;
        } else {
            info->m_element_type = 7;
        }

        if (info->m_coords != NULL) free(info->m_coords);
        info->m_coords = (float *) malloc(sizeof (float)*info->m_nnodes * 3);
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
        for (i = 0; i < info->m_nnodes * 3; i++) {
            info->m_coords[i] = 0.0;
        }
        // Allocate int array with  [m_nodes] elements
        if (param->nodesearch || param->avsucd || param->datasettype == PBVR_VTK_POLYDATA) {
            if (info->m_ids != NULL) free(info->m_ids);
            info->m_ids = (int *) malloc(sizeof (int)*info->m_nnodes);
        }

        // Allocate int array with  [m_nelements] elements
        if (info->m_conn_top_node != NULL) free(info->m_conn_top_node);
        info->m_conn_top_node = (int *) malloc(sizeof (int)*info->m_nelements);
        if (NULL == info->m_conn_top_node) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(connections).\n");
            goto garbage;
        }
        /* dummy allocate */
        if (param->avsucd || param->datasettype == PBVR_VTK_POLYDATA) {
            if (info->m_elementids != NULL) free(info->m_elementids);
            info->m_elementids = (int *) malloc(sizeof (int)*info->m_nelements);
            if (info->m_materials != NULL) free(info->m_materials);
            info->m_materials = (int *) malloc(sizeof (int)*info->m_nelements);
            if (info->m_elemtypes != NULL) free(info->m_elemtypes);
            info->m_elemtypes = (char *) malloc(sizeof (char)*info->m_nelements);
        }

        // Allocate string array with [m_ncomponents] elements
        info->pm_names = (char**) malloc(sizeof (char*)*info->m_ncomponents);
        for (i = 0; i < info->m_ncomponents; i++) {
            info->pm_names[i] = (char*) malloc(sizeof (char)*COMPONENT_LEN);
        }
        // Allocate string array with [m_ncomponents] elements
        info->pm_units = (char**) malloc(sizeof (char*)*info->m_ncomponents);
        for (i = 0; i < info->m_ncomponents; i++) {
            info->pm_units[i] = (char*) malloc(sizeof (char)*COMPONENT_LEN);
        }
        // Allocate int array with [m_ncomponents] elements
        info->m_veclens = (int *) malloc(sizeof (int)*info->m_ncomponents);
        info->m_veclens[0] = 1;
        // Allocate int array with [m_nkinds] elements
        info->m_nullflags = (int *) malloc(sizeof (int)*info->m_nkinds);
        // Allocate float array with [m_nkinds] elements
        info->m_nulldatas = (float *) malloc(sizeof (float)*info->m_nkinds);
        if (NULL == info->m_nulldatas) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(nulldatas).\n");
            goto garbage;
        }

        if (RTC_OK != set_etc(info)) {
            LOGOUT(FILTER_ERR, (char*)"set_etc error.\n");
            goto garbage;
        }
        // 3D data and irregular field type
        // Fill info->m_coords array with IJK coordinates
        if (param->ndim == 3 && strstr(param->field, "irregular")) {
            info->node_min.x = info->node_min.y = info->node_min.z = 0.0;
            info->node_max.x = (float) (param->dim1 - 1);
            info->node_max.y = (float) (param->dim2 - 1);
            info->node_max.z = (float) (param->dim3 - 1);
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,j,k,ii)
#endif
            for (k = 0; k < param->dim3; k++) {
                for (j = 0; j < param->dim2; j++) {
                    for (i = 0; i < param->dim1; i++) {
                        ii = i + j * param->dim1 + k * param->dim1 * param->dim2;
                        info->m_coords[ii * 3] = (float) i;
                        info->m_coords[ii * 3 + 1] = (float) j;
                        info->m_coords[ii * 3 + 2] = (float) k;
                    }
                }
            }
        } else {
            if (RTC_OK != read_struct_grid(
                            info->numpe_f,
                            info->mype_f
#ifdef MPI_EFFECT
                            , MPI_COMM_FILTER
#endif
                                )) {
                LOGOUT(FILTER_ERR, (char*)"can not read struct grid.).\n");
                goto garbage;
            }
            info->node_min.x = param->min_ext[0];
            info->node_min.y = param->min_ext[1];
            info->node_min.z = param->min_ext[2];
            info->node_max.x = param->max_ext[0];
            info->node_max.y = param->max_ext[1];
            info->node_max.z = param->max_ext[2];
        }
    } /* param->struct_grid */

    LOGOUT(FILTER_LOG, (char*)"  total number of grids       : %10d\n", info->m_nnodes);
    LOGOUT(FILTER_LOG, (char*)"  total number of elements    : %10d\n", info->m_nelements);
    LOGOUT(FILTER_LOG, (char*)"  total number of sub-volumes : %10d\n", info->m_nvolumes);
    LOGOUT(FILTER_LOG, (char*)"  total number of steps       : %10d\n", info->m_nsteps);
    LOGOUT(FILTER_LOG, (char*)"  min.x                       : %f\n", info->node_min.x);
    LOGOUT(FILTER_LOG, (char*)"  min.y                       : %f\n", info->node_min.y);
    LOGOUT(FILTER_LOG, (char*)"  min.z                       : %f\n", info->node_min.z);
    LOGOUT(FILTER_LOG, (char*)"  max.x                       : %f\n", info->node_max.x);
    LOGOUT(FILTER_LOG, (char*)"  max.y                       : %f\n", info->node_max.y);
    LOGOUT(FILTER_LOG, (char*)"  max.z                       : %f\n", info->node_max.z);

    /////////////////////////////////////////////////////////////////////////////
    /* 木構造の作成  || Creating a tree structure  */
    if (RTC_OK != division->create_tree_struct(
                            info->node_min,
                            info->node_max)) {
        LOGOUT(FILTER_ERR, (char*)"create_tree_struct error.\n");
        goto garbage;
    }
    if (param->file_type == 1) {
        if (param->out_num == 0) param->out_num = info->m_nvolumes;
        if (info->m_vol_sta != NULL) free(info->m_vol_sta);
        if (info->m_vol_end != NULL) free(info->m_vol_end);
        if (info->m_vol_num != NULL) free(info->m_vol_num);
        info->m_vol_sta = (int *) malloc(sizeof (int)*param->out_num);
        info->m_vol_end = (int *) malloc(sizeof (int)*param->out_num);
        info->m_vol_num = (int *) malloc(sizeof (int)*param->out_num);
        if (NULL == info->m_vol_sta || NULL == info->m_vol_end || NULL == info->m_vol_num) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_vol_num).\n");
            goto garbage;
        }
        for (k = 0; k < param->out_num; k++) {
            info->m_vol_num[k] = 0;
        }
        ii = info->m_nvolumes;
        k = 0;
        while (ii > 0) {
            info->m_vol_num[k]++;
            k++;
            if (k == param->out_num) {
                k = 0;
            }
            ii--;
        }
        info->m_vol_sta[0] = 0;
        info->m_vol_end[0] = info->m_vol_num[0] - 1;
        for (k = 1; k < param->out_num; k++) {
            info->m_vol_sta[k] = info->m_vol_end[k - 1] + 1;
            info->m_vol_end[k] = info->m_vol_sta[k] + info->m_vol_num[k] - 1;
        }
        free(info->m_vol_num);
        info->m_vol_num = NULL;
    }

    // add by @hira at 2016/12/01
    if (info->m_ids != NULL) {
        free(info->m_ids);
        info->m_ids = NULL;
    }

    // Allocate Array of int with [m_nodes] elements
    if (info->m_subvolume_num != NULL) free(info->m_subvolume_num);
    info->m_subvolume_num = (int *) malloc(sizeof (int)*info->m_nelements);
    // Allocate Array of unsigned int with [m_nvolumes] elements
    if (info->m_elems_per_subvolume != NULL) free(info->m_elems_per_subvolume);
    info->m_elems_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    if (NULL == info->m_subvolume_num) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_num).\n");
        goto garbage;
    }
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
    // Set arrays to 0. What about m_subvolume_num?
    for (i = 0; i < info->m_nvolumes; i++) {
        info->m_elems_per_subvolume[i] = 0;
    }

    if (param->struct_grid || param->input_format == PLOT3D_INPUT) {
        DEBUG(3, "distribution structure enter\n");
        if (param->ndim == 2) {
            // modify by @hira at 2016/12/01
            // if (RTC_OK != distribution_structure_2d(info)) {
            if (RTC_OK != division->distribution_structure_2d()) {
                LOGOUT(FILTER_ERR, (char*)"distribution_structure error.\n");
                goto garbage;
            }
        } else {
            // modify by @hira at 2016/12/01
//            if (RTC_OK != distribution_structure(info)) {
            if (RTC_OK != division->distribution_structure()) {
                LOGOUT(FILTER_ERR, (char*)"distribution_structure error.\n");
                goto garbage;
            }
        }
    } else {
        // modify by @hira at 2016/12/01
//        if (RTC_OK != distribution_unstructure(info)) {
        if (RTC_OK != division->distribution_unstructure()) {
            LOGOUT(FILTER_ERR, (char*)"distribution_unstructure error.\n");
            goto garbage;
        }
    }
    DEBUG(3, "distribution passed\n");
    // Free m_octree int array again.... ?
    if (info->m_octree != NULL) {
        free(info->m_octree);
        info->m_octree = NULL;
    }

    rank_elems = 0;
#if 0            // modify by @hira at 2016/12/01
    if (info->numpe_s < 2) {
        if (param->file_type != 1) {
            for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
                if (info->mype_v != volm_loop % info->numpe_v) {
                    continue;
                }
#endif
                rank_elems += info->m_elems_per_subvolume[volm_loop];
            }
        } else {
            for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {
#ifdef MPI_EFFECT
                if (info->mype_v != fnum_loop % info->numpe_v) {
                    continue;
                }
#endif
                for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                    rank_elems += info->m_elems_per_subvolume[volm_loop];
                }
            }
        }
    } else {
        for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
            if (info->mype_f != volm_loop % info->numpe_f) {
                continue;
            }
#endif
            rank_elems += info->m_elems_per_subvolume[volm_loop];
        }
    }
#endif
    // modify by @hira at 2016/12/01
    for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
        if (!division->isWriteMpiProcess(volm_loop)) {
            continue;
        }
        rank_elems += info->m_elems_per_subvolume[volm_loop];
    }

    // Reads structured grid data
    if (param->struct_grid && param->ndim == 3) {
        if (RTC_OK != read_struct_grid(
                            info->numpe_f,
                            info->mype_f
#ifdef MPI_EFFECT
                            , MPI_COMM_FILTER
#endif
                            )) {
            LOGOUT(FILTER_ERR, (char*)"can not read struct grid.).\n");
            goto garbage;
        }
    }

    //  m_elems_per_subvolume = (int *)malloc(sizeof(int)*m_nvolumes);
    //Allocate connection and node per sub volume arrays, size of [m_nvolumes]
    if (info->m_conns_per_subvolume != NULL) free(info->m_conns_per_subvolume);
    if (info->m_nodes_per_subvolume != NULL) free(info->m_nodes_per_subvolume);
    info->m_conns_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    info->m_nodes_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    if (NULL == info->m_nodes_per_subvolume || NULL == info->m_conns_per_subvolume) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodes_per_subvolume).\n");
        goto garbage;
    }

    info->prov_len = info->m_nnodes;
    if (info->numpe_v > 2) {
        info->prov_len = info->m_nnodes / info->numpe_v * 2;
    }

    // Alloc two int arrays with [prov_len|m_nnodes] elements
    if (info->m_subvolume_nodes != NULL) free(info->m_subvolume_nodes);
    if (info->m_subvolume_trans != NULL) free(info->m_subvolume_trans);
    info->m_subvolume_nodes = (int *) malloc(sizeof (int)*info->prov_len);
    info->m_subvolume_trans = (int *) malloc(sizeof (int)*info->prov_len);
    if (NULL == info->m_subvolume_nodes || NULL == info->m_subvolume_trans) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_trans).\n");
        goto garbage;
    }

    info->prov_len_e = rank_elems;
    if (!param->mult_element_type || avs_unstruct_multi_type) {
        if (info->m_subvolume_elems != NULL) free(info->m_subvolume_elems);
        info->m_subvolume_elems = (int *) malloc(sizeof (int)*info->prov_len_e);
    }

    if (info->m_nodes_trans != NULL) free(info->m_nodes_trans);
    if (info->m_nodeflag != NULL) free(info->m_nodeflag);
    info->m_nodes_trans = (int *) malloc(sizeof (int)*info->m_nnodes);
    info->m_nodeflag = (char *) malloc(sizeof (char)*info->m_nnodes);
    if (NULL == info->m_nodes_trans || NULL == info->m_nodeflag) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodes_trans & m_nodeflag).\n");
        goto garbage;
    }

    for (i = 0; i < info->m_nvolumes; i++) {
        //      m_elems_per_subvolume[i] = 0;
        info->m_conns_per_subvolume[i] = 0;
        info->m_nodes_per_subvolume[i] = 0;
    }
    for (i = 0; i < info->m_nnodes; i++) {
        info->m_nodeflag[i] = 0;
    }
// #endif

    rank_nodes = 0;
    rank_elems = 0;
    LOGOUT(FILTER_LOG, (char*)"|||| set sub-volume informations start.\n");

    // modify by @hira 2016/12/01
    // if (info->numpe_s < 2) {
    {
        if (param->file_type != 1) {
            for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0		// delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
                if (info->mype_v != volm_loop % info->numpe_v) {
                    continue;
                }
#endif
#endif
                // add by @hira at 2016/12/01
                if (!division->isWriteMpiProcess(volm_loop)) {
                    continue;
                }

                subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
                /* サブボリューム内の要素・節点の判別 */
                if (RTC_OK != count_in_subvolume(subvolume_id,
                                    &subvolume_elems,
                                    &subvolume_nodes,
                                    &subvolume_conns,
                                    &info->prov_len,
                                    rank_nodes,
                                    rank_elems,
                                    avs_unstruct_multi_type ? false : param->mult_element_type,
                                    info)) {
                    LOGOUT(FILTER_ERR, (char*)"count_in_subvolume error.\n");
                    goto garbage;
                }

                info->m_elems_per_subvolume[volm_loop] = subvolume_elems;
                info->m_nodes_per_subvolume[volm_loop] = subvolume_nodes;
                info->m_conns_per_subvolume[volm_loop] = subvolume_conns;
                rank_nodes += info->m_nodes_per_subvolume[volm_loop];
                rank_elems += info->m_elems_per_subvolume[volm_loop];
            }
        } else {
            for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {
#ifdef MPI_EFFECT
                if (info->mype_v != fnum_loop % info->numpe_v) {
                    continue;
                }
#endif
                for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                    subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
                    /* サブボリューム内の要素・節点の判別 */
                    if (RTC_OK != count_in_subvolume(subvolume_id,
                                        &subvolume_elems,
                                        &subvolume_nodes,
                                        &subvolume_conns,
                                        &info->prov_len,
                                        rank_nodes,
                                        rank_elems,
                                        avs_unstruct_multi_type ? false : param->mult_element_type,
                                        info)) {
                        LOGOUT(FILTER_ERR, (char*)"count_in_subvolume error.\n");
                        goto garbage;
                    }
                    info->m_elems_per_subvolume[volm_loop] = subvolume_elems;
                    info->m_nodes_per_subvolume[volm_loop] = subvolume_nodes;
                    info->m_conns_per_subvolume[volm_loop] = subvolume_conns;
                    rank_nodes += info->m_nodes_per_subvolume[volm_loop];
                    rank_elems += info->m_elems_per_subvolume[volm_loop];
                }
            }
        }
    }
#if 0       // delete by @hira at 2017/05/22
    else {
        for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0       // delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
            if (info->mype_f != volm_loop % info->numpe_f) {
                continue;
            }
#endif
#endif

            if (!division->isWriteMpiProcess(volm_loop)) {
                continue;
            }

            subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
            /* サブボリューム内の要素・節点の判別 */
            if (RTC_OK != count_in_subvolume(subvolume_id,
                                &subvolume_elems,
                                &subvolume_nodes,
                                &subvolume_conns,
                                &info->prov_len,
                                rank_nodes,
                                rank_elems,
                                avs_unstruct_multi_type ? false : param->mult_element_type,
                                info)) {
                LOGOUT(FILTER_ERR, (char*)"count_in_subvolume error.\n");
                goto garbage;
            }
            //          LOGOUT(FILTER_LOG, (char*)" subvolume_id : %d\n",subvolume_id);
            //          LOGOUT(FILTER_LOG, (char*)" subvolume_elems : %d, subvolume_nodes : %d, subvolume_conns : %d\n",subvolume_elems,subvolume_nodes,subvolume_conns);
            info->m_elems_per_subvolume[volm_loop] = subvolume_elems;
            info->m_nodes_per_subvolume[volm_loop] = subvolume_nodes;
            info->m_conns_per_subvolume[volm_loop] = subvolume_conns;
            rank_nodes += info->m_nodes_per_subvolume[volm_loop];
            rank_elems += info->m_elems_per_subvolume[volm_loop];
        }
    }
#endif

    LOGOUT(FILTER_LOG, (char*)"     rank_nodes  : %d, rank_elems  : %d\n", rank_nodes, rank_elems);
    LOGOUT(FILTER_LOG, (char*)"|||| set sub-volume informations end.\n");
    if (info->m_nodeflag != NULL) {
        free(info->m_nodeflag);
        info->m_nodeflag = NULL;
    }

    if (info->m_element_array != NULL) {
        for (i = 0; i < info->m_nvolumes; i++) {
            free(info->m_element_array[i]);
        }
        free(info->m_element_array);
        info->m_element_array = NULL;
    }

    if (info->m_subvolume_num != NULL) {
        free(info->m_subvolume_num);
        info->m_subvolume_num = NULL;
    }

    if (0 < rank_nodes && rank_nodes < info->prov_len) {

        if (RTC_OK != realloc_work(&info->prov_len,
                rank_nodes)) {
            LOGOUT(FILTER_ERR, (char*)"realloc_work error.\n");
            goto garbage;
        }
    }
#ifdef MPI_EFFECT
    LOGOUT(FILTER_LOG, (char*)"|||| mpi_allreduce start.\n");
#ifdef MPI_MEM_SAVE
    LOGOUT(FILTER_LOG, (char*)"|||| mpi_allreduce MPI_MEM_SAVE.\n");

    MPI_Allreduce(MPI_IN_PLACE, info->m_elems_per_subvolume, info->m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
    MPI_Allreduce(MPI_IN_PLACE, info->m_nodes_per_subvolume, info->m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
    MPI_Allreduce(MPI_IN_PLACE, info->m_conns_per_subvolume, info->m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
#else
    LOGOUT(FILTER_LOG, (char*)"|||| mpi_allreduce none MPI_MEM_SAVE.\n");

    mpiwork = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    memset(mpiwork, 0x00, sizeof (float)*info->m_nvolumes);
    if (NULL == mpiwork) {
        LOGOUT(FILTER_ERR, (char*)"mpiwork allocate error.\n");
        goto garbage;
    }
    //  MPI_Allreduce(m_elems_per_subvolume,mpiwork,m_nvolumes,MPI_FLOAT,MPI_MAX,MPI_COMM_FILTER);
    MPI_Allreduce(info->m_elems_per_subvolume, mpiwork, info->m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
    LOGOUT(FILTER_LOG, (char*)"|||| mpi_allreduce 1.\n");
    for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
        info->m_elems_per_subvolume[volm_loop] = mpiwork[volm_loop];
    }
    //  MPI_Allreduce(m_nodes_per_subvolume,mpiwork,m_nvolumes,MPI_FLOAT,MPI_MAX,MPI_COMM_FILTER);
    MPI_Allreduce(info->m_nodes_per_subvolume, mpiwork, info->m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
    LOGOUT(FILTER_LOG, (char*)"|||| mpi_allreduce 2.\n");
    for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
        info->m_nodes_per_subvolume[volm_loop] = mpiwork[volm_loop];
    }
    //  MPI_Allreduce(m_conns_per_subvolume,mpiwork,m_nvolumes,MPI_FLOAT,MPI_MAX,MPI_COMM_FILTER);
    LOGOUT(FILTER_LOG, (char*)"|||| mpi_allreduce 3.\n");
    MPI_Allreduce(info->m_conns_per_subvolume, mpiwork, info->m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
    for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
        info->m_conns_per_subvolume[volm_loop] = mpiwork[volm_loop];
    }
    free(mpiwork);
    mpiwork = NULL;
#endif
    LOGOUT(FILTER_LOG, (char*)"|||| mpi_allreduce end.\n");
#endif

    // delete by @hira 2016/12/01
#if 0
    if (param->file_type == 2 || info->numpe_s > 1) {

        if (info->m_conns_accumulate != NULL) free(info->m_conns_accumulate);
        if (info->m_nodes_accumulate != NULL) free(info->m_nodes_accumulate);
        info->m_conns_accumulate = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
        info->m_nodes_accumulate = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
        if (NULL == info->m_conns_accumulate || NULL == info->m_nodes_accumulate) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodes_accumulate).\n");
            goto garbage;
        }
        info->m_nodes_accumulate[0] = 0;
        info->m_conns_accumulate[0] = 0;
        for (volm_loop = 1; volm_loop < info->m_nvolumes; volm_loop++) {
            info->m_nodes_accumulate[volm_loop] = info->m_nodes_accumulate[volm_loop - 1] + info->m_nodes_per_subvolume[volm_loop - 1];
            info->m_conns_accumulate[volm_loop] = info->m_conns_accumulate[volm_loop - 1] + info->m_conns_per_subvolume[volm_loop - 1];
        }
        if (info->numpe_s > 1) {
            if (info->m_elems_accumulate != NULL) free(info->m_elems_accumulate);
            info->m_elems_accumulate = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
            if (NULL == info->m_elems_accumulate) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_elems_accumulate).\n");
                goto garbage;
            }
            info->m_elems_accumulate[0] = 0;
            for (volm_loop = 1; volm_loop < info->m_nvolumes; volm_loop++) {
                info->m_elems_accumulate[volm_loop] = info->m_elems_accumulate[volm_loop - 1] + info->m_elems_per_subvolume[volm_loop - 1];
            }
        }
    }
    ////////
    if (info->numpe_s > 1) {

#ifdef MPI_EFFECT
        MPI_Allreduce(&rank_nodes, &total_nodes, 1, MPI_LONG, MPI_SUM, MPI_COMM_FILTER);
        MPI_Allreduce(&rank_elems, &total_elems, 1, MPI_LONG, MPI_SUM, MPI_COMM_FILTER);
#else
        total_nodes = rank_nodes;
        total_elems = rank_elems;
#endif
        LOGOUT(FILTER_LOG, (char*)"     total_nodes : %d, total_elems : %d\n", total_nodes, total_elems);

        all_subvolume_nodes = (unsigned int *) malloc(sizeof (unsigned int)*total_nodes);
        if (NULL == all_subvolume_nodes) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(all_subvolume_nodes).\n");
            goto garbage;
        }
        all_subvolume_trans = (unsigned int *) malloc(sizeof (unsigned int)*total_nodes);
        if (NULL == all_subvolume_trans) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(all_subvolume_trans).\n");
            goto garbage;
        }
        all_subvolume_elems = (unsigned int *) malloc(sizeof (unsigned int)*total_elems);
        if (NULL == all_subvolume_elems) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(all_subvolume_elems).\n");
            goto garbage;
        }

#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
        for (i = 0; i < total_nodes; i++) {
            all_subvolume_nodes[i] = 0;
            all_subvolume_trans[i] = 0;
        }
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
        for (i = 0; i < total_elems; i++) {
            all_subvolume_elems[i] = 0;
        }
        rank_nodes = 0;
        rank_elems = 0;
        for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0		// delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
            if (info->mype_f != volm_loop % info->numpe_f) {
                continue;
            }
#endif
#endif
            // add by @hira at 2016/12/01
            if (!division->isWriteMpiProcess(volm_loop)) {
                continue;
            }

            subvolume_elems = info->m_elems_per_subvolume[volm_loop];
            subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
            subvolume_conns = info->m_conns_per_subvolume[volm_loop];
            ista = info->m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
            for (i = 0; i < subvolume_nodes; i++) {
                all_subvolume_nodes[ista + i] = info->m_subvolume_nodes[rank_nodes + i];
                all_subvolume_trans[ista + i] = info->m_subvolume_trans[rank_nodes + i];
            }
            ista = info->m_elems_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
            for (i = 0; i < subvolume_elems; i++) {
                all_subvolume_elems[ista + i] = info->m_subvolume_elems[rank_elems + i];
            }
            rank_nodes += subvolume_nodes;
            rank_elems += subvolume_elems;
            ;
        }

#ifdef MPI_EFFECT

#ifdef MPI_MEM_SAVE
        MPI_Allreduce(MPI_IN_PLACE, all_subvolume_nodes, total_nodes, MPI_INT, MPI_MAX, MPI_COMM_FILTER);
        MPI_Allreduce(MPI_IN_PLACE, all_subvolume_trans, total_nodes, MPI_INT, MPI_MAX, MPI_COMM_FILTER);
        MPI_Allreduce(MPI_IN_PLACE, all_subvolume_elems, total_elems, MPI_INT, MPI_MAX, MPI_COMM_FILTER);
#else
        mpiwork = (unsigned int *) malloc(sizeof (int)*total_nodes);
        if (NULL == mpiwork) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mpiwork). [%10d]\n", total_nodes);
            goto garbage;
        }
        MPI_Allreduce(all_subvolume_nodes, mpiwork, total_nodes, MPI_INT, MPI_MAX, MPI_COMM_FILTER);
        for (i = 0; i < total_nodes; i++) {
            all_subvolume_nodes[i] = mpiwork[i];
        }
        MPI_Allreduce(all_subvolume_trans, mpiwork, total_nodes, MPI_INT, MPI_MAX, MPI_COMM_FILTER);
        for (i = 0; i < total_nodes; i++) {
            all_subvolume_trans[i] = mpiwork[i];
        }
        free(mpiwork);
        mpiwork = (unsigned int *) malloc(sizeof (int)*total_elems);
        if (NULL == mpiwork) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory( mpiwork ). [%10d]\n", total_elems);
            goto garbage;
        }
        MPI_Allreduce(all_subvolume_elems, mpiwork, total_elems, MPI_INT, MPI_MAX, MPI_COMM_FILTER);
        for (i = 0; i < total_elems; i++) {
            all_subvolume_elems[i] = mpiwork[i];
        }
        free(mpiwork);
        mpiwork = NULL;
#endif
#endif

        free(info->m_subvolume_nodes);
        free(info->m_subvolume_trans);
        free(info->m_subvolume_elems);
        info->m_subvolume_nodes = NULL;
        info->m_subvolume_trans = NULL;
        info->m_subvolume_elems = NULL;

        if (param->file_type != 1) {
            rank_nodes = 0;
            rank_elems = 0;
            for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0		// delete by @hira
#ifdef MPI_EFFECT
                if (info->mype_f != volm_loop % info->numpe_f) {
                    continue;
                }
#endif
#endif
                // add by @hira at 2016/12/01
                if (!division->isWriteMpiProcess(volm_loop)) {
                    continue;
                }

                rank_nodes += info->m_nodes_per_subvolume[volm_loop];
                rank_elems += info->m_elems_per_subvolume[volm_loop];
            }

            if (info->m_subvolume_nodes != NULL) free(info->m_subvolume_nodes);
            info->m_subvolume_nodes = (int *) malloc(sizeof (int)*rank_nodes);
            if (NULL == info->m_subvolume_nodes) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_nodes).\n");
                goto garbage;
            }
            if (info->m_subvolume_trans != NULL) free(info->m_subvolume_trans);
            info->m_subvolume_trans = (int *) malloc(sizeof (int)*rank_nodes);
            if (NULL == info->m_subvolume_trans) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_trans).\n");
                goto garbage;
            }
            if (info->m_subvolume_elems != NULL) free(info->m_subvolume_elems);
            info->m_subvolume_elems = (int *) malloc(sizeof (int)*rank_elems);
            if (NULL == info->m_subvolume_elems) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_elems).\n");
                goto garbage;
            }

            rank_nodes = 0;
            rank_elems = 0;
            for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0			// delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
                if (info->mype_f != volm_loop % info->numpe_f) {
                    continue;
                }
#endif
#endif
                // add by @hira at 2016/12/01
                if (!division->isWriteMpiProcess(volm_loop)) {
                    continue;
                }

                subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
                subvolume_conns = info->m_conns_per_subvolume[volm_loop];
                subvolume_elems = info->m_elems_per_subvolume[volm_loop];
                ista = info->m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                for (i = 0; i < subvolume_nodes; i++) {
                    info->m_subvolume_nodes[rank_nodes + i] = all_subvolume_nodes[ista + i];
                    info->m_subvolume_trans[rank_nodes + i] = all_subvolume_trans[ista + i];
                }
                ista = info->m_elems_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                for (i = 0; i < subvolume_elems; i++) {
                    info->m_subvolume_elems[rank_elems + i] = all_subvolume_elems[ista + i];
                }
                rank_nodes += info->m_nodes_per_subvolume[volm_loop];
                rank_elems += info->m_elems_per_subvolume[volm_loop];
            }
        } else {
            rank_nodes = 0;
            rank_elems = 0;
            for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {

#ifdef MPI_EFFECT
                /////////////   if(mype_v!=fnum_loop%numpe_v){
                if (info->mype_f != fnum_loop % info->numpe_f) {
                    continue;
                }
#endif
                for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                    rank_nodes += info->m_nodes_per_subvolume[volm_loop];
                    rank_elems += info->m_elems_per_subvolume[volm_loop];
                }
            }
            if (info->m_subvolume_nodes != NULL) free(info->m_subvolume_nodes);
            info->m_subvolume_nodes = (int *) malloc(sizeof (int)*rank_nodes);
            if (NULL == info->m_subvolume_nodes) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_nodes) .\n");
                goto garbage;
            }
            if (info->m_subvolume_trans != NULL) free(info->m_subvolume_trans);
            info->m_subvolume_trans = (int *) malloc(sizeof (int)*rank_nodes);
            if (NULL == info->m_subvolume_trans) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_trans) .\n");
                goto garbage;
            }
            if (info->m_subvolume_elems != NULL) free(info->m_subvolume_elems);
            info->m_subvolume_elems = (int *) malloc(sizeof (int)*rank_elems);
            if (NULL == info->m_subvolume_elems) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_elems) .\n");
                goto garbage;
            }

            rank_nodes = 0;
            rank_elems = 0;

            for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {

#ifdef MPI_EFFECT
                ////////////    if(mype_v!=fnum_loop%numpe_v){
                if (info->mype_f != fnum_loop % info->numpe_f) {
                    continue;
                }
#endif
                for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                    subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
                    subvolume_conns = info->m_conns_per_subvolume[volm_loop];
                    subvolume_elems = info->m_elems_per_subvolume[volm_loop];
                    ista = info->m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                    for (i = 0; i < subvolume_nodes; i++) {
                        info->m_subvolume_nodes[rank_nodes + i] = all_subvolume_nodes[ista + i];
                        info->m_subvolume_trans[rank_nodes + i] = all_subvolume_trans[ista + i];
                    }
                    ista = info->m_elems_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                    for (i = 0; i < subvolume_elems; i++) {
                        info->m_subvolume_elems[rank_elems + i] = all_subvolume_elems[ista + i];
                    }
                    rank_nodes += info->m_nodes_per_subvolume[volm_loop];
                    rank_elems += info->m_elems_per_subvolume[volm_loop];
                }
            }
        }
        //      free(all_subvolume_nodes);
        //      free(all_subvolume_trans);
        //      free(all_subvolume_elems);
    }
    ////////
#endif

    info->m_pbvr_connections.clear();
    info->m_pbvr_connect_count.clear();
    info->m_pbvr_subvolumeids.clear();
    info->m_pbvr_plot3d_offsetvalue.clear();

    rank_nodes = 0;
    rank_elems = 0;
    if (param->file_type == 0) {
        LOGOUT(FILTER_LOG, (char*)"|||| output (file_type=0) start.\n");

        // modify @hira at 2016/12/01
        // write [out_dir]/[out_prefix]_[sub]_[all]_coord.dat, write [out_dir]/[out_prefix]_[sub]_[all]_connect.dat
        if (writer->write_nodes_split() != RTC_OK) {
            goto garbage;
        }
        LOGOUT(FILTER_LOG, (char*)"|||| output (file_type=0) end.\n");
    }

    if (param->file_type == 2) {
        LOGOUT(FILTER_LOG, (char*)"|||| output (file_type=2) start.\n");

        // modify @hira at 2016/12/01
        // write [out_dir]/[out_prefix]_coord.dat, [out_dir]/[out_prefix]_connect.dat
        if (writer->write_nodes_step(
#ifdef MPI_EFFECT
                    MPI_COMM_FILTER
#endif
                        ) != RTC_OK) {
            goto garbage;
        }
        free(info->m_conns_accumulate);
        info->m_conns_accumulate = NULL;
        LOGOUT(FILTER_LOG, (char*)"|||| output (file_type=2) end.\n");
    }
    if (param->file_type == 1) {
        LOGOUT(FILTER_LOG, (char*)"|||| output (file_type=1) start.\n");

        // modify @hira at 2016/12/01
        // write [out_dir]/[out_prefix]_coord.dat, [out_dir]/[out_prefix]_connect.dat
        if (writer->write_nodes_subvolume() != RTC_OK) {
            goto garbage;
        }

        LOGOUT(FILTER_LOG, (char*)"|||| output (file_type=1) end.\n");
    }

    /****  delete by @hira at 2017/0315
    if (info->m_nodes_trans != NULL) free(info->m_nodes_trans);
    info->m_nodes_trans = NULL;
    if (info->m_subvolume_trans != NULL) free(info->m_subvolume_trans);
    info->m_subvolume_trans = NULL;
    // 2014.11
    free(info->m_subvolume_elems);
    info->m_subvolume_elems = NULL;
    if (param->avsucd == 1 || param->datasettype == PBVR_VTK_POLYDATA) {
        free(info->m_conn_top_node);
        info->m_conn_top_node = NULL;
    }
    ****/


    // 2014.11
    ///////////////////////////////
    // delete by @hira 2016/12/01
#if 0
    if (info->numpe_s > 1) {
        free(info->m_subvolume_nodes);
        free(info->m_subvolume_trans);
        free(info->m_subvolume_elems);
        info->m_subvolume_nodes = NULL;
        info->m_subvolume_trans = NULL;
        info->m_subvolume_elems = NULL;

        if (param->file_type != 1) {
            rank_nodes = 0;
            rank_elems = 0;
            for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0		// delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
                if (info->mype_v != volm_loop % info->numpe_v) {
                    continue;
                }
#endif
#endif
                // add by @hira at 2016/12/01
                if (!division->isWriteMpiProcess(volm_loop)) {
                    continue;
                }

                rank_nodes += info->m_nodes_per_subvolume[volm_loop];
                rank_elems += info->m_elems_per_subvolume[volm_loop];
            }
            if (info->m_subvolume_nodes != NULL) free(info->m_subvolume_nodes);
            info->m_subvolume_nodes = (int *) malloc(sizeof (int)*rank_nodes);
            if (NULL == info->m_subvolume_nodes) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_nodes). [%d]\n", rank_nodes);
                goto garbage;
            }
            rank_nodes = 0;
            for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0		// delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
                if (info->mype_v != volm_loop % info->numpe_v) {
                    continue;
                }
#endif
#endif
                // add by @hira at 2016/12/01
                if (!division->isWriteMpiProcess(volm_loop)) {
                    continue;
                }

                subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
                ista = info->m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                for (i = 0; i < subvolume_nodes; i++) {
                    info->m_subvolume_nodes[rank_nodes + i] = all_subvolume_nodes[ista + i];
                }
                rank_nodes += info->m_nodes_per_subvolume[volm_loop];
            }
            free(all_subvolume_nodes);
            all_subvolume_nodes = NULL;

            if (info->m_subvolume_trans != NULL) free(info->m_subvolume_trans);
            info->m_subvolume_trans = (int *) malloc(sizeof (int)*rank_nodes);
            if (NULL == info->m_subvolume_trans) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_trans). [%d]\n", rank_nodes);
                goto garbage;
            }
            rank_nodes = 0;
            for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0		// delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
                if (info->mype_v != volm_loop % info->numpe_v) {
                    continue;
                }
#endif
#endif
                // add by @hira at 2016/12/01
                if (!division->isWriteMpiProcess(volm_loop)) {
                    continue;
                }

                subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
                ista = info->m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                for (i = 0; i < subvolume_nodes; i++) {
                    info->m_subvolume_trans[rank_nodes + i] = all_subvolume_trans[ista + i];
                }
                rank_nodes += info->m_nodes_per_subvolume[volm_loop];
            }
            free(all_subvolume_trans);
            all_subvolume_trans = NULL;

            if (info->m_subvolume_elems != NULL) free(info->m_subvolume_elems);
            info->m_subvolume_elems = (int *) malloc(sizeof (int)*rank_elems);
            if (NULL == info->m_subvolume_elems) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_elems). [%d]\n", rank_elems);
                goto garbage;
            }

            rank_elems = 0;
            for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0		// delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
                if (info->mype_v != volm_loop % info->numpe_v) {
                    continue;
                }
#endif
#endif
                // add by @hira at 2016/12/01
                if (!division->isWriteMpiProcess(volm_loop)) {
                    continue;
                }

                subvolume_elems = info->m_elems_per_subvolume[volm_loop];
                ista = info->m_elems_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                for (i = 0; i < subvolume_elems; i++) {
                    info->m_subvolume_elems[rank_elems + i] = all_subvolume_elems[ista + i];
                }
                rank_elems += info->m_elems_per_subvolume[volm_loop];
            }
            free(all_subvolume_elems);
            all_subvolume_elems = NULL;
        } else {
            rank_nodes = 0;
            rank_elems = 0;
            for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {
#ifdef MPI_EFFECT
                if (info->mype_v != fnum_loop % info->numpe_v) {
                    continue;
                }
#endif
                for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                    rank_nodes += info->m_nodes_per_subvolume[volm_loop];
                    rank_elems += info->m_elems_per_subvolume[volm_loop];
                }
            }
            if (info->m_subvolume_nodes != NULL) free(info->m_subvolume_nodes);
            info->m_subvolume_nodes = (int *) malloc(sizeof (int)*rank_nodes);
            if (NULL == info->m_subvolume_nodes) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_trans).\n");
                goto garbage;
            }

            rank_nodes = 0;
            for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {
#ifdef MPI_EFFECT
                if (info->mype_v != fnum_loop % info->numpe_v) {
                    continue;
                }
#endif
                for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                    subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
                    ista = info->m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                    for (i = 0; i < subvolume_nodes; i++) {
                        info->m_subvolume_nodes[rank_nodes + i] = all_subvolume_nodes[ista + i];
                    }
                    rank_nodes += info->m_nodes_per_subvolume[volm_loop];
                }
            }
            free(all_subvolume_nodes);
            all_subvolume_nodes = NULL;

            if (info->m_subvolume_trans != NULL) free(info->m_subvolume_trans);
            info->m_subvolume_trans = (int *) malloc(sizeof (int)*rank_nodes);
            if (NULL == info->m_subvolume_trans) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_trans).\n");
                goto garbage;
            }
            rank_nodes = 0;
            for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {
#ifdef MPI_EFFECT
                if (info->mype_v != fnum_loop % info->numpe_v) {
                    continue;
                }
#endif
                for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                    subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
                    ista = info->m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                    for (i = 0; i < subvolume_nodes; i++) {
                        info->m_subvolume_trans[rank_nodes + i] = all_subvolume_trans[ista + i];
                    }
                    rank_nodes += info->m_nodes_per_subvolume[volm_loop];
                }
            }
            free(all_subvolume_trans);
            all_subvolume_trans = NULL;

            if (info->m_subvolume_elems != NULL) free(info->m_subvolume_elems);
            info->m_subvolume_elems = (int *) malloc(sizeof (int)*rank_elems);
            if (NULL == info->m_subvolume_elems) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_elems).\n");
                goto garbage;
            }
            rank_elems = 0;
            for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {
#ifdef MPI_EFFECT
                if (info->mype_v != fnum_loop % info->numpe_v) {
                    continue;
                }
#endif
                for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                    subvolume_elems = info->m_elems_per_subvolume[volm_loop];
                    ista = info->m_elems_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                    for (i = 0; i < subvolume_elems; i++) {
                        info->m_subvolume_elems[rank_elems + i] = all_subvolume_elems[ista + i];
                    }
                    rank_elems += info->m_elems_per_subvolume[volm_loop];
                }
            }
            free(all_subvolume_elems);
            all_subvolume_elems = NULL;
        }
    }
#endif

    if (param->struct_grid) {
        float x_min, y_min, z_min;
        float x_max, y_max, z_max;

        if (info->m_value_max != NULL) free(info->m_value_max);
        if (info->m_value_min != NULL) free(info->m_value_min);
        info->m_value_max = (float*) malloc(sizeof (float)*info->m_nvolumes * 3);
        info->m_value_min = (float*) malloc(sizeof (float)*info->m_nvolumes * 3);
        if (NULL == info->m_value_min || NULL == info->m_value_max) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_value_min&max).\n");
            goto garbage;
        }
        memset(info->m_value_max, 0x00, sizeof (float)*info->m_nvolumes * 3);
        memset(info->m_value_min, 0x00, sizeof (float)*info->m_nvolumes * 3);
        for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
            info->m_value_min[volm_loop * 3 ] = FLT_MAX;
            info->m_value_min[volm_loop * 3 + 1] = FLT_MAX;
            info->m_value_min[volm_loop * 3 + 2] = FLT_MAX;
            info->m_value_max[volm_loop * 3 ] = FLT_MIN;
            info->m_value_max[volm_loop * 3 + 1] = FLT_MIN;
            info->m_value_max[volm_loop * 3 + 2] = FLT_MIN;
        }
        if (param->file_type != 1) {
            rank_nodes = 0;
            for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0		// delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
                if (info->numpe_v > 1) {
                    if (info->mype_v != volm_loop % info->numpe_v) {
                        continue;
                    }
                }
#endif
#endif
                // add by @hira at 2016/12/01
                if (!division->isWriteMpiProcess(volm_loop)) {
                    continue;
                }

                subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
                subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
                if (subvolume_nodes == 0) {
                    continue;
                }
                x_min = y_min = z_min = FLT_MAX;
                x_max = y_max = z_max = FLT_MIN;
                for (i = 0; i < subvolume_nodes; i++) {
                    id = info->m_subvolume_nodes[rank_nodes + i];
                    j = id * 3;
                    if (info->m_coords[j ] < x_min) x_min = info->m_coords[j ];
                    if (info->m_coords[j ] > x_max) x_max = info->m_coords[j ];
                    if (info->m_coords[j + 1] < y_min) y_min = info->m_coords[j + 1];
                    if (info->m_coords[j + 1] > y_max) y_max = info->m_coords[j + 1];
                    if (info->m_coords[j + 2] < z_min) z_min = info->m_coords[j + 2];
                    if (info->m_coords[j + 2] > z_max) z_max = info->m_coords[j + 2];
                }
                info->m_value_min[volm_loop * 3 ] = x_min;
                info->m_value_min[volm_loop * 3 + 1] = y_min;
                info->m_value_min[volm_loop * 3 + 2] = z_min;
                info->m_value_max[volm_loop * 3 ] = x_max;
                info->m_value_max[volm_loop * 3 + 1] = y_max;
                info->m_value_max[volm_loop * 3 + 2] = z_max;
                rank_nodes += info->m_nodes_per_subvolume[volm_loop];
            }
        } else {
            rank_nodes = 0;
            for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {
#ifdef MPI_EFFECT
                if (info->mype_v != fnum_loop % info->numpe_v) {
                    continue;
                }
#endif
                for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                    subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
                    subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
                    x_min = y_min = z_min = FLT_MAX;
                    x_max = y_max = z_max = FLT_MIN;
                    for (i = 0; i < subvolume_nodes; i++) {
                        id = info->m_subvolume_nodes[rank_nodes + i];
                        j = id * 3;
                        if (info->m_coords[j ] < x_min) x_min = info->m_coords[j ];
                        if (info->m_coords[j ] > x_max) x_max = info->m_coords[j ];
                        if (info->m_coords[j + 1] < y_min) y_min = info->m_coords[j + 1];
                        if (info->m_coords[j + 1] > y_max) y_max = info->m_coords[j + 1];
                        if (info->m_coords[j + 2] < z_min) z_min = info->m_coords[j + 2];
                        if (info->m_coords[j + 2] > z_max) z_max = info->m_coords[j + 2];
                    }
                    info->m_value_min[volm_loop * 3 ] = x_min;
                    info->m_value_min[volm_loop * 3 + 1] = y_min;
                    info->m_value_min[volm_loop * 3 + 2] = z_min;
                    info->m_value_max[volm_loop * 3 ] = x_max;
                    info->m_value_max[volm_loop * 3 + 1] = y_max;
                    info->m_value_max[volm_loop * 3 + 2] = z_max;
                    rank_nodes += info->m_nodes_per_subvolume[volm_loop];
                }
            }
        }
#ifdef MPI_EFFECT
        MPI_Allreduce(MPI_IN_PLACE, info->m_value_min, info->m_nvolumes * 3, MPI_FLOAT, MPI_MIN, MPI_COMM_FILTER);
        MPI_Allreduce(MPI_IN_PLACE, info->m_value_max, info->m_nvolumes * 3, MPI_FLOAT, MPI_MAX, MPI_COMM_FILTER);
#endif
        for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
            subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
            info->m_tree[subvolume_id].min.x = info->m_value_min[volm_loop * 3 ];
            info->m_tree[subvolume_id].min.y = info->m_value_min[volm_loop * 3 + 1];
            info->m_tree[subvolume_id].min.z = info->m_value_min[volm_loop * 3 + 2];
            info->m_tree[subvolume_id].max.x = info->m_value_max[volm_loop * 3 ];
            info->m_tree[subvolume_id].max.y = info->m_value_max[volm_loop * 3 + 1];
            info->m_tree[subvolume_id].max.z = info->m_value_max[volm_loop * 3 + 2];
        }
        free(info->m_value_min);
        free(info->m_value_max);
        info->m_value_min = NULL;
        info->m_value_max = NULL;
    }
    else {
        FilterSubVolume filter_sub(info);
        float min[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
        float max[3] = {(-1)*FLT_MAX, (-1)*FLT_MAX, (-1)*FLT_MAX};
        rank_nodes = 0;
        for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
            if (!division->isWriteMpiProcess(volm_loop)) {
                continue;
            }
            filter_sub.get_coords_minmax(volm_loop, rank_nodes, min, max);
            subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
            info->m_tree[subvolume_id].min.x = min[0];
            info->m_tree[subvolume_id].min.y = min[1];
            info->m_tree[subvolume_id].min.z = min[2];
            info->m_tree[subvolume_id].max.x = max[0];
            info->m_tree[subvolume_id].max.y = max[1];
            info->m_tree[subvolume_id].max.z = max[2];
            rank_nodes += info->m_nodes_per_subvolume[volm_loop];
        }
    }

    /* データ書き込み */
#ifdef MPI_EFFECT
    if (info->mype_f == 0) {
#endif
        LOGOUT(FILTER_LOG, (char*)"|||| output pfi file start.\n");
        if (RTC_OK != writer->write_pfi(info->node_min, info->node_max)) {
            LOGOUT(FILTER_ERR, (char*)"write_pfi error.\n");
            goto garbage;
        }
        // add by @hira at 2016/12/01
        info->m_min_object_coord = info->node_min;
        info->m_max_object_coord = info->node_max;

        LOGOUT(FILTER_LOG, (char*)"|||| output pfi file end.\n");

#ifdef MPI_EFFECT
    }
    {
        float coords[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        if (info->mype_f == 0) {
            coords[0] = info->m_min_object_coord.x;
            coords[1] = info->m_min_object_coord.y;
            coords[2] = info->m_min_object_coord.z;
            coords[3] = info->m_max_object_coord.x;
            coords[4] = info->m_max_object_coord.y;
            coords[5] = info->m_max_object_coord.z;
        }
        MPI_Bcast(&coords, 6, MPI_FLOAT, 0, MPI_COMM_FILTER);
        if (info->mype_f != 0) {
            info->m_min_object_coord.x = coords[0];
            info->m_min_object_coord.y = coords[1];
            info->m_min_object_coord.z = coords[2];
            info->m_max_object_coord.x = coords[3];
            info->m_max_object_coord.y = coords[4];
            info->m_max_object_coord.z = coords[5];
        }
    }
#endif

    ///////////////////////////////
    if (info->m_value_max != NULL) free(info->m_value_max);
    info->m_value_max = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->m_value_max) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_value_min).\n");
        goto garbage;
    }
    if (info->m_value_min != NULL) free(info->m_value_min);
    info->m_value_min = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->m_value_min) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_value_min).\n");
        goto garbage;
    }
    if (info->mt_nulldata != NULL) free(info->mt_nulldata);
    info->mt_nulldata = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->mt_nulldata) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mt_nulldata).\n");
        goto garbage;
    }
    if (info->mt_nullflag != NULL) free(info->mt_nullflag);
    info->mt_nullflag = (int *) malloc(sizeof (int)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->mt_nullflag) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mt_nullflag).\n");
        goto garbage;
    }
    for (i = 0; i < info->m_nkinds * info->m_nsteps; i++) {
        info->m_value_max[i] = 0.0;
        info->m_value_min[i] = 0.0;
        info->mt_nulldata[i] = 0.0;
        info->mt_nullflag[i] = 0;
    }

    // modify @hira at 2017/04/01 : ランク０のみ全領域をアロケートする
    if (info->mype == 0) {
        if (info->m_values != NULL) free(info->m_values);
        // modify @hira at 2017/03/15 : ランク０は通信バッファー用として、サブボリュームの総ノード数とする。
        info->m_values = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nnodes);
        // ランク０はサブボリュームの総ノード数とする。
        // ランク０以外は担当サブボリュームのノード数とする。
        size = info->getNodeCount();
        // info->m_values = (float *) malloc(sizeof (float)*size);

        if (NULL == info->m_values) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
            goto garbage;
        }

        // modify @hira at 2017/04/10 : ランク０のみ全領域をアロケートする
        // 2014.11
        LOGOUT(FILTER_LOG, (char*)"|||| m_nelemcomponents.\n");
        if (info->m_nelemcomponents > 0) {
            if (info->m_elem_in_node != NULL) free(info->m_elem_in_node);
            if (info->m_elem_in_node_addr != NULL) free(info->m_elem_in_node_addr);
            info->m_elem_in_node = (int *) malloc(sizeof (int)*info->m_nnodes);
            info->m_elem_in_node_addr = (int *) malloc(sizeof (int)*info->m_nnodes);
            memset(info->m_elem_in_node, 0x00, sizeof (float)*info->m_nnodes);
            if (RTC_OK != set_element_in_node(info)) {
                LOGOUT(FILTER_ERR, (char*)"set_element_in_node error.\n");
                goto garbage;
            }
        }
    }

    for (step_loop = param->start_step; step_loop <= param->end_step; step_loop++) {
#if 0		// delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
        if (info->mpidiv_s > 1) {
            if (info->mype_s != (step_loop - param->start_step) % info->mpidiv_s) {
                continue;
            }
        }
#endif
#ifdef MPI_EFFECT
        if (info->mpidiv_v > 1) {
            if (info->mype_v > 0) {
                goto skip_read_value;
            }
        }
#endif
#endif
        if (info->mype != 0) {
            goto skip_read_value;
        }

        LOGOUT(FILTER_LOG, (char*)":::: time_step %d start.\n", step_loop);

        /* データ読込み  Read data*/
        LOGOUT(FILTER_LOG, (char*)"|||| read variable data start.\n");

        if (param->input_format == PLOT3D_INPUT) {
            info->m_stepn = step_loop;
            rstat = this->m_io_plot3d->plot3d_getBlockValuesForTstep(step_loop);
        }
#ifdef VTK_ENABLED
        else if (param->input_format == VTK_INPUT) {
            if (param->struct_grid) {
                rstat = this->m_io_vtk->vtk_read_structured_values(step_loop);
            } else {
                DEBUG(1, " reading step variables for element_type_number %d\n\n", element_type_num);
                rstat = this->m_io_vtk->vtk_read_unstructured_values(step_loop, element_type_num);
                //		ASSERT_FAIL("F");
            }
        }
#endif
        else if (param->input_format == STL_ASCII || param->input_format == STL_BIN) {
            LOGOUT(FILTER_LOG, (char*)"ENTER STL BRANCH.\n");
            // When reading STL the there are no values
            // so set summy values
            size = info->getNodeCount();
            for (count = 0; count < info->m_nnodes; count++) {
            // for (count = 0; count < size; count++) {
                info->m_values[count] = FLT_MIN;
            }
        } else if (param->struct_grid) {
            info->m_stepn = step_loop;
            if (strlen(param->in_prefix) == 0) {
                /*rstat=read_variables( step_loop );*/
                rstat = this->m_io_avs->read_variables_3(step_loop - param->start_step);
            } else {
                rstat = this->m_io_avs->read_variables_per_step(step_loop);
            }
            if (RTC_OK != rstat) {
                LOGOUT(FILTER_ERR, (char*)"read_variables_per_step error.\n");
                goto garbage;
            }
        }
#ifndef MULTI_SEPARATE_FILES
        else if (avs_unstruct_multi_type) {
            // 要素タイプを指定して読み込みを行う  add by @hira at 2017/04/15
            if (RTC_OK != this->m_io_avs->read_unstructer_multi_value(element_type_num, step_loop)) {
                LOGOUT(FILTER_ERR, (char*)"read unstructure multi error.[element_type_num=%d]\n", element_type_num);
                goto garbage;
            }
        }
#endif
        else {
            if (strlen(param->ucd_inp) != 0 && info->m_multi_element == 0) {
                if (info->m_ucdbin == 0) {
                    rstat = this->m_io_avs->skip_ucd_ascii(step_loop);
                    if (info->m_cycle_type == 1) {
                        rstat = this->m_io_avs->skip_ucd_ascii_geom();
                    } else if (info->m_cycle_type == 2) {
                        if (step_loop == param->start_step) {
                            rstat = this->m_io_avs->skip_ucd_ascii_geom();
                        }
                    }
                    rstat = this->m_io_avs->read_ucd_ascii_value(step_loop);
                } else {
                    rstat = this->m_io_avs->read_ucdbin_data(step_loop);
                }
            } else {
                rstat = this->m_io_avs->read_ucdbin_data(step_loop);
            }
            if (RTC_OK != rstat) {
                LOGOUT(FILTER_ERR, (char*)"read_ucdbin_data error.\n");
                goto garbage;
            }
        }
        LOGOUT(FILTER_LOG, (char*)"|||| read variable data end.\n");

skip_read_value:
#ifdef MPI_EFFECT
        if (info->mpidiv_v > 1) {
            LOGOUT(FILTER_LOG, (char*)"|||| MPI_Bcast variable data start.\n");
            MPI_Bcast(&info->m_stepn, 1, MPI_INT, 0, MPI_COMM_FILTER);
            MPI_Bcast(&info->m_stept, 1, MPI_FLOAT, 0, MPI_COMM_FILTER);
            MPI_Bcast(info->m_veclens, info->m_ncomponents, MPI_INT, 0, MPI_COMM_FILTER);
            MPI_Bcast(info->m_nullflags, info->m_nkinds, MPI_INT, 0, MPI_COMM_FILTER);
            MPI_Bcast(info->m_nulldatas, info->m_nkinds, MPI_FLOAT, 0, MPI_COMM_FILTER);

            // modify @hira at 2017/04/01 : ランク０から物理量をMPI_Scattervにて配信、アロケートする
            // MPI_Bcast(info->m_values, info->m_nkinds*info->m_nnodes, MPI_FLOAT, 0, MPI_COMM_FILTER);
            info->scatterValue();

            LOGOUT(FILTER_LOG, (char*)"|||| MPI_Bcast variable data end.\n");
        }
#endif
        // modify @hira at 2017/03/15
        // if (info->mype == 0) {
            /* KVSML形式ファイル  |  KVSML format file */
            if (param->file_type == 0) {
                // modify by @hira
                // write [out_dir]/[out_prefix]_[step_loop]_[sub]_[all]_value.dat
                if (writer->write_values_split(step_loop) != RTC_OK) {
                    goto garbage;
                }
            }

            /* ステップ集約ファイル */
            if (param->file_type == 2) {
                //          sprintf(buff,param->format,step_loop );
                if (writer->write_values_step(
                                    step_loop
    #ifdef MPI_EFFECT
                                    ,MPI_COMM_VOLM
    #endif
                                    ) != RTC_OK) {
                    goto garbage;
                }
            }

            /* サブボリューム集約ファイル形式 (param->collect==1) */
            if (param->file_type == 1) {
                if (writer->write_valuetmp1_subvolume(step_loop) != RTC_OK) {
                    goto garbage;
                }

            } /* サブボリューム集約ファイル形式 (param->collect==1) */

            if (!param->struct_grid && info->m_multi_element && param->temp_delete) {
                writer->remove_tmpfile(m_elementtypeno[element_type_num],
                                        m_out_prefix,
                                        step_loop);
            }
        // }

        LOGOUT(FILTER_LOG, (char*)":::: time_step %d end.\n", step_loop);
    }

#ifdef MPI_EFFECT
    // modify by @hira at 2016/12/01
    if (info->numpe_s > 1) {
//    if (info->numpe_s > 1) {
//        if (info->mype_v == 0) {

#ifdef MPI_MEM_SAVE
            MPI_Allreduce(MPI_IN_PLACE, info->m_value_max, info->m_nkinds*info->m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
            MPI_Allreduce(MPI_IN_PLACE, info->m_value_min, info->m_nkinds*info->m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
            MPI_Allreduce(MPI_IN_PLACE, info->mt_nulldata, info->m_nkinds*info->m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
            MPI_Allreduce(MPI_IN_PLACE, info->mt_nullflag, info->m_nkinds*info->m_nsteps, MPI_INT, MPI_SUM, MPI_COMM_STEP);
#else
            float *mpiworkf = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nsteps);
            for (i = 0; i < info->m_nkinds * info->m_nsteps; i++) {
                mpiworkf[i] = 0.0;
            }
            MPI_Allreduce(info->m_value_max, mpiworkf, info->m_nkinds*info->m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
            for (i = 0; i < info->m_nkinds * info->m_nsteps; i++) {
                info->m_value_max[i] = mpiworkf[i];
            }
            for (i = 0; i < info->m_nkinds * info->m_nsteps; i++) {
                mpiworkf[i] = 0.0;
            }
            MPI_Allreduce(info->m_value_min, mpiworkf, info->m_nkinds*info->m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
            for (i = 0; i < info->m_nkinds * info->m_nsteps; i++) {
                info->m_value_min[i] = mpiworkf[i];
            }

            for (i = 0; i < info->m_nkinds * info->m_nsteps; i++) {
                mpiworkf[i] = 0.0;
            }
            MPI_Allreduce(info->mt_nulldata, mpiworkf, info->m_nkinds*info->m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
            for (i = 0; i < info->m_nkinds * info->m_nsteps; i++) {
                info->mt_nulldata[i] = mpiworkf[i];
            }
            for (i = 0; i < info->m_nkinds * info->m_nsteps; i++) {
                mpiworkf[i] = 0.0;
            }
            MPI_Allreduce(info->mt_nullflag, mpiworkf, info->m_nkinds*info->m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
            for (i = 0; i < info->m_nkinds * info->m_nsteps; i++) {
                info->mt_nullflag[i] = mpiworkf[i];
            }
            free(mpiworkf);
            mpiworkf = NULL;
#endif
//        }
    }
#endif

#ifdef MPI_EFFECT
    if (info->mype_f == 0) {
#endif
        if (RTC_OK != writer->write_pfi_value()) {
            LOGOUT(FILTER_ERR, (char*)"write_pfi_value error.\n");
            goto garbage;
        }
#ifdef MPI_EFFECT
    }
#endif

#ifdef MPI_EFFECT
    MPI_Barrier(MPI_COMM_FILTER);
#endif

    LOGOUT(FILTER_LOG, (char*)"param->file_type %d\n", param->file_type);
    LOGOUT(FILTER_LOG, (char*)"mpidiv_v         %d\n", info->mpidiv_v);
    /* サブボリューム集約ファイル形式 (param->collect==1) */
    if (param->file_type == 1 && info->mpidiv_s > 1) {
        if (writer->write_values_subvolume() != RTC_OK) {
            goto garbage;
        }
    }

    /* メモリ開放 */
    // initialize_memory(); // Frees pointers if they are null
    // initialize_value(); // Sets variables to 0|NULL


    subvolume_elems = 0;
    subvolume_nodes = 0;
    subvolume_conns = 0;
    if (all_subvolume_nodes != NULL) free(all_subvolume_nodes);
    if (all_subvolume_trans != NULL) free(all_subvolume_trans);
    if (all_subvolume_elems != NULL) free(all_subvolume_elems);
    all_subvolume_nodes = NULL;
    all_subvolume_trans = NULL;
    all_subvolume_elems = NULL;
    /********************/
    DEBUG(3, ">>>>>>>>>>>> End of convert chain here!!!\n");

    // 利用しない領域の開放
    pre_release_PbvrFilterInfo(info);

    // add by @hira at 2017/03/20 : m_pbvr_valuesからm_valueにコピーする
    {
        // m_valuesをリサイズする
        if (info->m_values != NULL) free(info->m_values);
        info->m_values = NULL;
        size = info->m_pbvr_values.size();
        info->m_values_size = size;
        if (size > 0) {
            info->m_values = (float *) malloc(sizeof (float)*size);
            std::vector<float>::iterator itr;
            size_t value_id = 0;
            for (itr=info->m_pbvr_values.begin(); itr!=info->m_pbvr_values.end(); itr++) {
                info->m_values[value_id++] = (*itr);
            }
        }
        info->m_pbvr_values.clear();
        std::vector<float>().swap(info->m_pbvr_values);
    }

    // add by @hira at 2016/12/01
    if (this->m_parent != NULL) {
        this->m_parent->setPbvrFilterList(info);
    }

    // Loop back to unstruct_element_proc
    if (!param->struct_grid && info->m_multi_element) {
        element_type_num++;
        goto unstruct_element_proc;
    }

    LOGOUT(FILTER_LOG, (char*)"[file:%s] %d : >>>>>>>>>>>> Return of convert chain here!!!\n", __FILE__, __LINE__);

unstruct_element_stop:
    /********************/

#ifdef MPI_EFFECT
release :
    MPI_Barrier(MPI_COMM_WORLD);
#endif
    // initialize_memory();
    // initialize_value();
    rcode = RTC_OK;

garbage:
    // if (info->m_tree != NULL) free(info->m_tree);
    // info->m_tree = NULL;
    if (writer != NULL) delete writer;
    if (division != NULL) delete division;
    return (0);
}

/**
 * Subvolume情報を設定する
 * @return
 */
int FilterConvert::createSubvolumeInfos()
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    FilterDivision *division = factoryFilterDivision(param->division_type, info);

    int volm_loop;
    int subvolume_id;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;
    Int64 rank_nodes = 0;
    Int64 rank_elems = 0;

    if (info->m_subvolume_num != NULL) free(info->m_subvolume_num);
    info->m_subvolume_num = (int *) malloc(sizeof (int)*info->m_nelements);
    if (info->m_elems_per_subvolume != NULL) free(info->m_elems_per_subvolume);
    info->m_elems_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    if (info->m_nodes_per_subvolume != NULL) free(info->m_nodes_per_subvolume);
    info->m_nodes_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    if (info->m_conns_per_subvolume != NULL) free(info->m_conns_per_subvolume);
    info->m_conns_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);

    memset(info->m_elems_per_subvolume, 0x00, sizeof (unsigned int)*info->m_nvolumes);
    memset(info->m_nodes_per_subvolume, 0x00, sizeof (unsigned int)*info->m_nvolumes);
    memset(info->m_conns_per_subvolume, 0x00, sizeof (unsigned int)*info->m_nvolumes);

    for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
        // add by @hira at 2016/12/01
        if (!division->isWriteMpiProcess(volm_loop)) {
            continue;
        }
        char struct_grid = 0;
        if (param->struct_grid || param->input_format == PLOT3D_INPUT) {
            struct_grid = 1;
        }

        if (RTC_OK != division->distribution_subvolume(struct_grid, param->ndim)) {
            LOGOUT(FILTER_ERR, (char*)"distribution_subvolume error.\n");
            return RTC_NG;
        }

        subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
        /* サブボリューム内の要素・節点の判別 */
        if (RTC_OK != count_in_subvolume(subvolume_id,
                            &subvolume_elems,
                            &subvolume_nodes,
                            &subvolume_conns,
                            &info->prov_len,
                            rank_nodes,
                            rank_elems,
                            param->mult_element_type,
                            info)) {
            LOGOUT(FILTER_ERR, (char*)"count_in_subvolume error.\n");
            return RTC_NG;
        }

        info->m_elems_per_subvolume[volm_loop] = subvolume_elems;
        info->m_nodes_per_subvolume[volm_loop] = subvolume_nodes;
        info->m_conns_per_subvolume[volm_loop] = subvolume_conns;
        rank_nodes += info->m_nodes_per_subvolume[volm_loop];
        rank_elems += info->m_elems_per_subvolume[volm_loop];
    }

    return RTC_OK;
}

} // end of namespace filter
} // end of namespace pbvr
