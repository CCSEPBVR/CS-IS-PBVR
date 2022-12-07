/*
 * PbvrFilter.cpp
 *
 *  Created on: 2016/12/02
 *      Author: hira
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <iterator>

#include "PbvrFilter.h"
#include "FilterConvert.h"
#include "FilterConvertPlot3d.h"
#ifdef VTK_ENABLED
#include "FilterConvertVtk.h"
#endif
#include "filter_log.h"
#include "filter_utils.h"
#include "FilterWrite.h"
#include "../../Server/timer_simple.h"

namespace pbvr
{
namespace filter
{

/**
 * コンストラクタ
 */
FilterParam::FilterParam() {
    m_iswrite = false;
    memset(data, 0x00, sizeof(char)*16);
    memset(field, 0x00, sizeof(char)*16);
    memset(coordfile, 0x00, sizeof(char)*3*LINELEN_MAX);
    memset(coordtype, 0x00, sizeof(char)*3*LINELEN_MAX);
    memset(valuefile, 0x00, sizeof(char)*LINELEN_MAX);
    memset(valuetype, 0x00, sizeof(char)*LINELEN_MAX);
    division_type = UNKNOWN_DIVISION;
    stepf = NULL;
    nstep = 0;
    ndim = 0;
    dim1 = 0;
    dim2 = 0;
    dim3 = 0;
    nspace = 0;
    veclen = 0;
    this->nodesearch = 0;
    this->avsucd = 0;
    this->datasettype = PBVR_VTK_UNKNOWN;

}

/**
 * コピーコンストラクタ
 * @param src
 */
FilterParam::FilterParam(const FilterParam &src)
{

    strcpy(this->in_dir, src.in_dir); /* ファイル接頭辞       */
    strcpy(this->in_prefix, src.in_prefix); /* ファイル接頭辞       */
    strcpy(this->in_suffix, src.in_suffix); /* ファイル接尾辞       */
    strcpy(this->out_dir, src.out_dir); /* ファイル接頭辞       */
    strcpy(this->out_prefix, src.out_prefix); /* ファイル接頭辞       */
    strcpy(this->out_suffix, src.out_suffix); /* ファイル接尾辞       */
    strcpy(this->field_file, src.field_file); /* fld ファイル名       */
    strcpy(this->geom_file, src.geom_file); /* geomファイル名       */
    strcpy(this->format, src.format); /* 時刻ステップ書式     */
    strcpy(this->ucd_inp, src.ucd_inp); /* inp ファイル名       */
    strcpy(this->tmp_dir, src.tmp_dir); /* 一次ディレクトリ     */
    this->start_step = src.start_step; /* 時刻ステップ開始番号 */
    this->end_step = src.end_step; /* 時刻ステップ終了番号 */
    this->n_layer = src.n_layer; /* 階層数               */
    this->elem_ave = src.elem_ave; /* 階層数               */
    this->component = src.component; /*                      */
    this->out_num = src.out_num; /* 出力ファイル数       */
    this->file_type = src.file_type; /* 出力形式　　         */
    this->null_val = src.null_val; /* NULL値               */
    this->nstep = src.nstep; /* fldファイルから      */
    this->ndim = src.ndim; /* fldファイルから      */
    this->dim1 = src.dim1; /* fldファイルから      */
    this->dim2 = src.dim2; /* fldファイルから      */
    this->dim3 = src.dim3; /* fldファイルから      */
    this->nspace = src.nspace; /* fldファイルから      */
    this->veclen = src.veclen; /* fldファイルから      */
    strcpy(this->data, src.data); /* fldファイルから      */
    strcpy(this->field, src.field); /* fldファイルから      */
    memcpy(this->coordfile, src.coordfile, sizeof(char)*3*LINELEN_MAX); /* COORD 1 FILE NAME    */
    memcpy(this->coordtype, src.coordtype, sizeof(char)*3*LINELEN_MAX); /* COORD 1 FILE NAME    */
    memcpy(this->coordskip, src.coordskip, sizeof(Int64)*3); /* fldファイルから      */
    memcpy(this->coordstride, src.coordstride, sizeof(Int64)*3); /* fldファイルから      */
    strcpy(this->valuefile, src.valuefile); /* VALUE 1 FILE NAME    */
    strcpy(this->valuetype, src.valuetype); /* VALUE 1 FILE NAME    */
    this->valueskip = src.valueskip; /* fldファイルから      */
    this->avsucd = src.avsucd; /* AVSUCD形式判定フラグ */
    this->struct_grid = src.struct_grid; /* 構造格子判定         */
    this->explicit_n_layer = src.explicit_n_layer; /* 階層指示ありなし判定 */
    this->nodesearch = src.nodesearch; /* 節点番号検索判定フラグ       */
    this->headfoot = src.headfoot; /* ヘッダー、フッター判定フラグ */
    this->mult_element_type = src.mult_element_type;
    this->temp_delete = src.temp_delete;
    this->mpi_volume_div = src.mpi_volume_div; /* 並列分割軸            */
    this->mpi_step_div = src.mpi_step_div; /* 並列分割軸            */
    this->mpi_div = src.mpi_div; /* 並列分割軸            */
    this->blocksize = src.blocksize;
    for (int n=0; n<3; n++) this->min_ext[n] = src.min_ext[n]; /* fldファイルから      */
    for (int n=0; n<3; n++) this->max_ext[n] = src.max_ext[n]; /* fldファイルから      */
    this->stepf = NULL;
    if (src.stepf != NULL && src.nstep > 0
        && src.nspace > 0 && src.veclen > 0) {
        this->stepf = (Stepfile*) malloc(sizeof (Stepfile) * src.nstep);
        if (NULL != this->stepf) {
            for (int i = 0; i < src.nstep; i++) {
                this->stepf[i].coord = (Fdetail*) malloc(sizeof (Fdetail) * src.nspace);
                this->stepf[i].value = (Fdetail*) malloc(sizeof (Fdetail) * src.veclen);
                memcpy(this->stepf[i].coord, src.stepf[i].coord, sizeof (Fdetail) * src.nspace);
                memcpy(this->stepf[i].value, src.stepf[i].value, sizeof (Fdetail) * src.veclen);
            }
        }
    }

    this->input_format = src.input_format;
    strcpy(this->plot3d_config, src.plot3d_config); /* PLOT3D configuration file      */
    this->datasettype = src.datasettype;       /* vtk dataset type */
    this->m_ucdbin = src.m_ucdbin;         /* add by @hira at 2016/12/01 */
    this->m_iswrite = src.m_iswrite;
    strcpy(this->m_paramfile, src.m_paramfile); /* パラメータファイル       */
    this->division_type = src.division_type;
    this->n_division = src.n_division;
}


/**
 * デストラクタ
 */
FilterParam::~FilterParam() {

    if (this->stepf != NULL && this->nstep > 0) {
        for (int i = 0; i < this->nstep; i++) {
            free(this->stepf[i].coord);
            free(this->stepf[i].value);
            this->stepf[i].coord = NULL;
            this->stepf[i].value = NULL;
        }
        free(this->stepf);
        this->stepf = NULL;
    }
}

/**
 * コンストラクタ
 */
PbvrFilterInfo::PbvrFilterInfo() {
    this->m_param = new FilterParam();
    this->current_block = 0;
    memset(this->buff, 0x00, sizeof(char)*LINELEN_MAX);
    memset(this->m_title, 0x00, sizeof(char)*70);
    m_vol_sta = NULL;
    m_vol_end = NULL;
    m_vol_num = NULL;
    offset_values = NULL;
	m_ucd_ascii_fp = NULL;
    // add by @hira at 2017/04/17
    initialize_PbvrFilterInfo(this);
}

/**
 * コピーコンストラクタ
 * @param src
 */
PbvrFilterInfo::PbvrFilterInfo(const PbvrFilterInfo &src) {
    initialize_PbvrFilterInfo(this);
    this->copyInfos(src);

    return;
}

/**
 * デストラクタ
 */
PbvrFilterInfo::~PbvrFilterInfo() {
    release_PbvrFilterInfo(this);
    initialize_PbvrFilterInfo(this);
    if (this->m_param != NULL) delete this->m_param;
}


/**
 * コピーコンストラクタ
 * @param src
 */
void PbvrFilterInfo::copyInfos(const PbvrFilterInfo &src) {

    this->m_param = new FilterParam(*src.m_param);

    switch (this->m_param->input_format) {
    case AVS_INPUT:
        this->m_elementsize = ucd_elementsize;
        break;
    case VTK_INPUT:
#ifdef VTK_ENABLED
        this->m_elementsize = vtk_elementsize;
#endif
        break;
    }
    this->m_element_type = src.m_element_type;
    this->m_tree_node_no = src.m_tree_node_no;
    if (src.m_tree != NULL && this->m_tree_node_no > 0) {
        this->m_tree = (HierCell *)malloc(sizeof(HierCell)*(this->m_tree_node_no));
        memcpy(this->m_tree, src.m_tree, sizeof(HierCell)*(this->m_tree_node_no));
    }
    strcpy(this->filename, src.filename);
    strcpy(this->buff, src.buff);
    this->m_nsteps = src.m_nsteps;
    this->m_nvolumes = src.m_nvolumes;

    strcpy(this->m_title, src.m_title);
    this->m_stepn = src.m_stepn;
    this->m_stept = src.m_stept;
    this->m_nnodes = src.m_nnodes;
    this->m_nelements = src.m_nelements;
    this->m_elemcoms = src.m_elemcoms;
    this->m_connectsize = src.m_connectsize;
    this->m_ncomponents = src.m_ncomponents;
    this->m_nnodecomponents = src.m_nnodecomponents;
    this->m_nelemcomponents = src.m_nelemcomponents;
    this->m_nkinds = src.m_nkinds;
    this->m_nnodekinds = src.m_nnodekinds;
    this->m_nelemkinds = src.m_nelemkinds;
    this->m_desctype = src.m_desctype;
    this->m_datatype = src.m_datatype;
    this->m_component_id = src.m_component_id;
    this->m_nvalues_per_node = src.m_nvalues_per_node;
    this->m_ncomponents_per_node = src.m_ncomponents_per_node;
    this->prov_len = src.prov_len;
    this->prov_len_e = src.prov_len_e;
    this->m_subvolume_coord_size = src.m_subvolume_coord_size;
    this->m_subvolume_connect_size = src.m_subvolume_connect_size;
    this->m_ids = NULL;
    this->m_coords = NULL;
    if (src.m_ids != NULL && src.m_nnodes > 0) {
        this->m_ids = (int *) malloc(sizeof (int)*src.m_nnodes);
        memcpy(this->m_ids, src.m_ids, sizeof (int)*src.m_nnodes);
    }
    if (src.m_coords != NULL && src.m_nnodes > 0) {
        this->m_coords = (float *) malloc(sizeof (float)*src.m_nnodes*3);
        memcpy(this->m_coords, src.m_coords, sizeof (float)*src.m_nnodes*3);
    }
    this->m_elementids = NULL;
    this->m_materials = NULL;
    this->m_elemtypes = NULL;
    if (src.m_elementids != NULL && src.m_nelements > 0) {
        this->m_elementids = (int *) malloc(sizeof (int)*src.m_nelements);
        memcpy(this->m_elementids, src.m_elementids, sizeof (int)*src.m_nelements);
    }
    if (src.m_materials != NULL && src.m_nelements > 0) {
        this->m_materials = (int *) malloc(sizeof (int)*src.m_nelements);
        memcpy(this->m_materials, src.m_materials, sizeof (int)*src.m_nelements);
    }
    if (src.m_elemtypes != NULL && src.m_nelements > 0) {
        this->m_elemtypes = (char *)malloc(sizeof(char) * src.m_nelements );
        memcpy(this->m_elemtypes, src.m_elemtypes, sizeof (char)*src.m_nelements);
    }
    this->m_connections = NULL;
    if (src.m_connections != NULL && src.m_connectsize > 0) {
        this->m_connections = (int *)malloc(sizeof(int)*src.m_connectsize );
        memcpy(this->m_connections, src.m_connections, sizeof (int)*src.m_connectsize);
    }
    this->m_connections_o = NULL;

    m_conn_top_node = NULL;
    if (src.m_conn_top_node != NULL && src.m_nelements > 0) {
        this->m_conn_top_node = (int *) malloc(sizeof (int)*src.m_nelements);
        memcpy(this->m_conn_top_node, src.m_conn_top_node, sizeof (int)*src.m_nelements);
    }
    this->m_values = NULL;
    //if (src.m_values != NULL && src.m_nnodes * src.m_nkinds > 0) {
    //	this->m_values = (float *) malloc(sizeof (float)*src.m_nkinds * src.m_nnodes);
    //	memcpy(this->m_values, src.m_values, sizeof (float)*src.m_nkinds * src.m_nnodes);
    // }
    Int64 size = src.m_values_size;
    if (src.m_values != NULL && size > 0) {
        this->m_values = (float *) malloc(sizeof (float)*size);
        memcpy(this->m_values, src.m_values, sizeof (float)*size);
    }
    this->m_names = NULL;
    this->m_units = NULL;
    this->pm_names = NULL;
    this->pm_units = NULL;
    if(src.pm_names != NULL && src.m_ncomponents > 0) {
        this->pm_names = (char**)malloc(sizeof(char*)*src.m_ncomponents);
        for(int i=0;i<src.m_ncomponents;i++){
            this->pm_names[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }
        for(int i=0;i<src.m_ncomponents;i++){
            memcpy(this->pm_names[i], src.pm_names[i], sizeof(char)*COMPONENT_LEN);
        }
    }
    if(src.pm_units != NULL && src.m_ncomponents > 0) {
        this->pm_units = (char**)malloc(sizeof(char*)*src.m_ncomponents);
        for(int i=0;i<src.m_ncomponents;i++){
            this->pm_units[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }
        for(int i=0;i<src.m_ncomponents;i++){
            memcpy(this->pm_units[i], src.pm_units[i], sizeof(char)*COMPONENT_LEN);
        }
    }
    this->m_veclens = NULL;
    if (src.m_veclens != NULL && src.m_ncomponents > 0) {
        this->m_veclens = (int *) malloc(sizeof (int)*src.m_ncomponents);
        memcpy(this->m_veclens, src.m_veclens, sizeof (int)*src.m_ncomponents);
    }
    this->m_elemveclens = NULL;
    m_nullflags = NULL;
    m_nulldatas = NULL;
    if (src.m_nullflags != NULL && src.m_nkinds > 0) {
        this->m_nullflags = (int *) malloc(sizeof (int)*src.m_nkinds);
        memcpy(this->m_nullflags, src.m_nullflags, sizeof (int)*src.m_nkinds);
    }
    if (src.m_nulldatas != NULL && src.m_nkinds > 0) {
        this->m_nulldatas = (float *) malloc(sizeof (float)*src.m_nkinds);
        memcpy(this->m_nulldatas, src.m_nulldatas, sizeof (float)*src.m_nkinds);
    }
    if (src.m_nvolumes > 0 && src.m_elems_per_subvolume != NULL) {
        this->m_elems_per_subvolume = (unsigned int *) malloc(sizeof (int)*src.m_nvolumes);
        memcpy(this->m_elems_per_subvolume, src.m_elems_per_subvolume, sizeof (int)*src.m_nvolumes);
    }
    if (src.m_nvolumes > 0 && src.m_conns_per_subvolume != NULL) {
        this->m_conns_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*src.m_nvolumes);
        memcpy(this->m_conns_per_subvolume, src.m_conns_per_subvolume, sizeof (int)*src.m_nvolumes);
    }
    if (src.m_nvolumes > 0 && src.m_nodes_per_subvolume != NULL) {
        this->m_nodes_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*src.m_nvolumes);
        memcpy(this->m_nodes_per_subvolume, src.m_nodes_per_subvolume, sizeof (unsigned int)*src.m_nvolumes);
    }

    this->m_elems_accumulate = NULL;
    if (src.m_elems_accumulate != NULL && src.m_nvolumes > 0) {
        this->m_elems_accumulate = (unsigned int *) malloc(sizeof (unsigned int)*src.m_nvolumes);
        memcpy(this->m_elems_accumulate, src.m_elems_accumulate, sizeof (unsigned int)*src.m_nvolumes);
    }
    this->m_conns_accumulate = NULL;
    if (src.m_conns_accumulate != NULL && src.m_nvolumes > 0) {
        this->m_conns_accumulate = (unsigned int *) malloc(sizeof (unsigned int)*src.m_nvolumes);
        memcpy(this->m_conns_accumulate, src.m_conns_accumulate, sizeof (unsigned int)*src.m_nvolumes);
    }
    this->m_nodes_accumulate = NULL;
    if (src.m_nodes_accumulate != NULL && src.m_nvolumes > 0) {
        this->m_nodes_accumulate = (unsigned int *) malloc(sizeof (unsigned int)*src.m_nvolumes);
        memcpy(this->m_nodes_accumulate, src.m_nodes_accumulate, sizeof (unsigned int)*src.m_nvolumes);
    }

    this->m_subvolume_num = NULL;
    if (src.m_subvolume_num != NULL && src.m_nelements > 0) {
        this->m_subvolume_num = (int *) malloc(sizeof (int)*src.m_nelements);
        memcpy(this->m_subvolume_num, src.m_subvolume_num, sizeof (int)*src.m_nelements);
    }
    this->m_nodeflag = NULL;
    if (src.m_nodeflag != NULL && src.m_nnodes > 0) {
        this->m_nodeflag = (char *) malloc(sizeof (char)*src.m_nnodes);
        memcpy(this->m_nodeflag, src.m_nodeflag, sizeof (char)*src.m_nnodes);
    }

    this->m_nodes_trans = NULL;
    if (src.m_nodes_trans != NULL && src.m_nnodes > 0) {
        this->m_nodes_trans = (int *) malloc(sizeof (int)*src.m_nnodes);
        memcpy(this->m_nodes_trans, src.m_nodes_trans, sizeof (int)*src.m_nnodes);
    }
    this->m_octree = NULL;
    if (src.m_octree != NULL && src.m_nnodes > 0) {
        this->m_octree = (int *) malloc(sizeof (int)*src.m_nnodes);
        memcpy(this->m_octree, src.m_octree, sizeof (int)*src.m_nnodes);
    }
    this->m_subvolume_nodes = NULL;
    if (src.m_subvolume_nodes != NULL && src.prov_len > 0) {
        this->m_subvolume_nodes = (int *) malloc(sizeof (int)*src.prov_len);
        memcpy(this->m_subvolume_nodes, src.m_subvolume_nodes, sizeof (int)*src.prov_len);
    }
    this->m_subvolume_trans = NULL;
    if (src.m_subvolume_trans != NULL && src.prov_len > 0) {
        this->m_subvolume_trans = (int *) malloc(sizeof (int)*src.prov_len);
        memcpy(this->m_subvolume_trans, src.m_subvolume_trans, sizeof (int)*src.prov_len);
    }
    this->m_subvolume_elems = NULL;
    if (src.m_subvolume_elems != NULL && src.prov_len_e > 0) {
        this->m_subvolume_elems = (int *) malloc(sizeof (int)*src.prov_len_e);
        memcpy(this->m_subvolume_elems, src.m_subvolume_elems, sizeof (int)*src.prov_len_e);
    }
    this->m_subvolume_coord = NULL;
    if (src.m_subvolume_coord != NULL && src.m_subvolume_coord_size > 0) {
        this->m_subvolume_coord = (float *) malloc(sizeof (int)*src.m_subvolume_coord_size);
        memcpy(this->m_subvolume_coord, src.m_subvolume_coord, sizeof (int)*src.m_subvolume_coord_size);
    }
    this->m_subvolume_connect = NULL;
    if (src.m_subvolume_connect != NULL && src.m_subvolume_connect_size > 0) {
        this->m_subvolume_connect = (int *) malloc(sizeof (int)*src.m_subvolume_connect_size);
        memcpy(this->m_subvolume_connect, src.m_subvolume_connect, sizeof (int)*src.m_subvolume_connect_size);
    }
    this->m_subvolume_value = NULL;
    this->m_value_max = NULL;
    this->m_value_min = NULL;
    this->mt_nullflag = NULL;
    this->mt_nulldata = NULL;
    if (src.m_value_max != NULL && src.m_nkinds * src.m_nsteps > 0) {
        this->m_value_max = (float *) malloc(sizeof (float)*src.m_nkinds * src.m_nsteps);
        memcpy(this->m_value_max, src.m_value_max, sizeof(float)*src.m_nkinds * src.m_nsteps);
    }
    if (src.m_value_min != NULL && src.m_nkinds * src.m_nsteps > 0) {
        this->m_value_min = (float *) malloc(sizeof (float)*src.m_nkinds * src.m_nsteps);
        memcpy(this->m_value_min, src.m_value_min, sizeof(float)*src.m_nkinds * src.m_nsteps);
    }
    if (src.mt_nulldata != NULL && src.m_nkinds * src.m_nsteps > 0) {
        this->mt_nulldata = (float *) malloc(sizeof (float)*src.m_nkinds * src.m_nsteps);
        memcpy(this->mt_nulldata, src.mt_nulldata, sizeof(float)*src.m_nkinds * src.m_nsteps);
    }
    if (src.mt_nullflag != NULL && src.m_nkinds * src.m_nsteps > 0) {
        this->mt_nullflag = (int*) malloc(sizeof (int)*src.m_nkinds * src.m_nsteps);
        memcpy(this->mt_nullflag, src.mt_nullflag, sizeof(int)*src.m_nkinds * src.m_nsteps);
    }
    this->mype = src.mype;
    this->numpe = src.numpe;
    this->mype_f = src.mype_f;
    this->numpe_f = src.numpe_f;
    this->mype_s = src.mype_s;
    this->numpe_s = src.numpe_s;
    this->mype_v = src.mype_v;
    this->numpe_v = src.numpe_v;
    this->mpidiv_s = src.mpidiv_s;
    this->mpidiv_v = src.mpidiv_v;

    this->m_elem_in_node = NULL;
    if (src.m_elem_in_node != NULL && src.m_nnodes > 0) {
        this->m_elem_in_node = (int *) malloc(sizeof (int)*src.m_nnodes);
        memcpy(this->m_elem_in_node, src.m_elem_in_node, sizeof (int)*src.m_nnodes);
    }
    this->m_elem_in_node_addr = NULL;
    if (src.m_elem_in_node_addr != NULL && src.m_nnodes > 0) {
        this->m_elem_in_node_addr = (int *) malloc(sizeof (int)*src.m_nnodes);
        memcpy(this->m_elem_in_node_addr, src.m_elem_in_node_addr, sizeof (int)*src.m_nnodes);
    }
    this->m_elem_in_node_array = NULL;
    if (src.m_elem_in_node_array != NULL && src.m_elem_in_node_array_size > 0) {
        this->m_elem_in_node_array = (int *) malloc(sizeof (int)*src.m_elem_in_node_array_size);
        memcpy(this->m_elem_in_node_array, src.m_elem_in_node_array, sizeof (int)*src.m_elem_in_node_array_size);
    }
    this->m_vol_sta = NULL;
    this->m_vol_end = NULL;
    this->m_vol_num = NULL;
    if (src.m_vol_sta != NULL && src.m_param->out_num > 0) {
        this->m_vol_sta = (int *) malloc(sizeof (int)*src.m_param->out_num);
        memcpy(this->m_vol_sta, src.m_vol_sta, sizeof (int)*src.m_param->out_num);
    }
    if (src.m_vol_end != NULL && src.m_param->out_num > 0) {
        this->m_vol_end = (int *) malloc(sizeof (int)*src.m_param->out_num);
        memcpy(this->m_vol_end, src.m_vol_end, sizeof (int)*src.m_param->out_num);
    }
    if (src.m_vol_num != NULL && src.m_param->out_num > 0) {
        this->m_vol_num = (int *) malloc(sizeof (int)*src.m_param->out_num);
        memcpy(this->m_vol_num, src.m_vol_end, sizeof (int)*src.m_param->out_num);
    }
    this->ENDIAN = src.ENDIAN;
    this->prov_len = src.prov_len;
    this->prov_len_e = src.prov_len_e;
    this->m_element_array = NULL;
    this->pre_step = src.pre_step;
    memcpy(this->m_types, src.m_types, sizeof(unsigned int)*MAX_ELEMENT_TYPE);
    this->m_filesize = src.m_filesize;
    this->m_headfoot = src.m_headfoot;
    this->m_multi_element = src.m_multi_element;
    this->m_cycle_type = src.m_cycle_type;
    this->m_ucdbin = src.m_ucdbin;
    this->m_ucdbinfile = NULL;
    if (src.m_ucdbinfile != NULL && src.m_ucdbinfile_size > 0) {
        this->m_ucdbinfile = (char**)malloc(sizeof(char*)*src.m_ucdbinfile_size);
        for(int i=0;i<src.m_ucdbinfile_size;i++){
            this->m_ucdbinfile[i] = (char*)malloc(sizeof(char)*LINELEN_MAX);
            memcpy(this->m_ucdbinfile[i],src.m_ucdbinfile[i], sizeof(char)*LINELEN_MAX);
        }
    }
    this->m_value_fp = NULL;
    this->m_geom_fp = NULL;
    this->m_ucd_ascii_fp = NULL;

    this->current_block = src.current_block;
    this->number_of_blocks = src.number_of_blocks;
    this->offset_values = NULL;
    if (src.offset_values != NULL && src.m_nvolumes > 0) {
        this->offset_values = (Int64*)malloc(src.m_nvolumes * sizeof (Int64));
        memcpy(this->offset_values, src.offset_values, src.m_nvolumes * sizeof (Int64));
    }
    this->node_max.x = src.node_max.x;
    this->node_max.y = src.node_max.y;
    this->node_max.z = src.node_max.z;
    this->node_min.x = src.node_min.x;
    this->node_min.y = src.node_min.y;
    this->node_min.z = src.node_min.z;
    this->m_min_object_coord.x = src.m_min_object_coord.x;
    this->m_min_object_coord.y = src.m_min_object_coord.y;
    this->m_min_object_coord.z = src.m_min_object_coord.z;
    this->m_max_object_coord.x = src.m_max_object_coord.x;
    this->m_max_object_coord.y = src.m_max_object_coord.y;
    this->m_max_object_coord.z = src.m_max_object_coord.z;

    this->m_pbvr_connections.clear();
    if (src.m_pbvr_connections.size() > 0) {
        this->m_pbvr_connections.reserve(src.m_pbvr_connections.size());
        std::copy(src.m_pbvr_connections.begin(), src.m_pbvr_connections.end(), std::back_inserter(this->m_pbvr_connections));
    }

    this->m_pbvr_connect_count.clear();
    if (src.m_pbvr_connect_count.size() > 0) {
        this->m_pbvr_connect_count.reserve(src.m_pbvr_connect_count.size());
        std::copy(src.m_pbvr_connect_count.begin(), src.m_pbvr_connect_count.end(), std::back_inserter(this->m_pbvr_connect_count));
    }

    this->m_pbvr_subvolumeids.clear();
    if (src.m_pbvr_subvolumeids.size() > 0) {
        this->m_pbvr_subvolumeids.reserve(src.m_pbvr_subvolumeids.size());
        std::copy(src.m_pbvr_subvolumeids.begin(), src.m_pbvr_subvolumeids.end(), std::back_inserter(this->m_pbvr_subvolumeids));
    }

    this->m_pbvr_values.clear();
    if (src.m_pbvr_values.size() > 0) {
        this->m_pbvr_values.reserve(src.m_pbvr_values.size());
        std::copy(src.m_pbvr_values.begin(), src.m_pbvr_values.end(), std::back_inserter(this->m_pbvr_values));
    }

    this->m_pbvr_plot3d_offsetvalue.clear();
    if (src.m_pbvr_plot3d_offsetvalue.size() > 0) {
        this->m_pbvr_plot3d_offsetvalue.reserve(src.m_pbvr_plot3d_offsetvalue.size());
        std::copy(src.m_pbvr_plot3d_offsetvalue.begin(), src.m_pbvr_plot3d_offsetvalue.end(), std::back_inserter(this->m_pbvr_plot3d_offsetvalue));
    }

    this->element_type_num = src.element_type_num;

    if (src.m_elems_per_multivolume != NULL) {
        this->m_elems_per_multivolume = (unsigned int *) malloc(sizeof (int)*MAX_ELEMENT_TYPE);
        memcpy(this->m_elems_per_multivolume, src.m_elems_per_multivolume, sizeof (int)*MAX_ELEMENT_TYPE);
    }
    if (src.m_conns_per_multivolume != NULL) {
        this->m_conns_per_multivolume = (unsigned int *) malloc(sizeof (int)*MAX_ELEMENT_TYPE);
        memcpy(this->m_conns_per_multivolume, src.m_conns_per_multivolume, sizeof (int)*MAX_ELEMENT_TYPE);
    }
    if (src.m_nodes_per_multivolume != NULL) {
        this->m_nodes_per_multivolume = (unsigned int *) malloc(sizeof (int)*MAX_ELEMENT_TYPE);
        memcpy(this->m_nodes_per_multivolume, src.m_nodes_per_multivolume, sizeof (int)*MAX_ELEMENT_TYPE);
    }
    if (src.m_multivolume_nodes != NULL) {
        int rank_nodes = 0;
        for (int elem_loop = 0; elem_loop < MAX_ELEMENT_TYPE; elem_loop++) {
            rank_nodes += src.m_nodes_per_multivolume[elem_loop];
        }
        if (rank_nodes > 0) {
            this->m_multivolume_nodes = (int *) malloc(sizeof (int)*rank_nodes);
            memcpy(this->m_multivolume_nodes, src.m_multivolume_nodes, sizeof (int)*rank_nodes);
        }
    }

    return;
}


/**
 * サブボリュームの総ノード数を取得する
 * @return		総ノード数
 */
Int64 PbvrFilterInfo::getTotalSubVolumeNodes() const
{
    if (this->m_nodes_per_subvolume == NULL) return 0;
    if (this->m_nvolumes == 0) return 0;
    Int64 count = 0;
    for (int i=0; i<this->m_nvolumes; i++) {
        count += this->m_nodes_per_subvolume[i];
    }
    return count;
}

/**
 * サブボリュームの総接続数を取得する
 * @return		総接続数
 */
Int64 PbvrFilterInfo::getTotalSubVolumeConns() const
{
    if (this->m_conns_per_subvolume == NULL) return 0;
    if (this->m_nvolumes == 0) return 0;
    Int64 count = 0;
    for (int i=0; i<this->m_nvolumes; i++) {
        count += this->m_conns_per_subvolume[i];
    }
    return count;
}

/**
 * サブボリュームの総要素数を取得する
 * @return		総要素数
 */
Int64 PbvrFilterInfo::getTotalSubVolumeElems() const
{
    if (this->m_elems_per_subvolume == NULL) return 0;
    if (this->m_nvolumes == 0) return 0;
    Int64 count = 0;
    for (int i=0; i<this->m_nvolumes; i++) {
        count += this->m_elems_per_subvolume[i];
    }
    return count;
}

/**
 * 自領域が担当するノード数を取得する
 * @return		自領域が担当するノード数
 */
Int64 PbvrFilterInfo::getSubVolumeNodeCount()
{
    FilterDivision *division = factoryFilterDivision(this->m_param->division_type, this);

    Int64 count = 0;
    for (int i=0; i<this->m_nvolumes; i++) {
        if (division->isWriteMpiProcess(i)) {
            count += this->m_nodes_per_subvolume[i];
        }
    }
    delete division;

    return count;
}


/**
 * 自領域が担当するノード数を取得する
 * @return		自領域が担当するノード数
 */
Int64 PbvrFilterInfo::getNodeCount()
{
    Int64 count = 0;
    if (this->mype == 0) {
        // ランク０はサブボリュームの総ノード数とする。
        count = this->getTotalSubVolumeNodes();
    }
    else {
        // ランク０以外は担当サブボリュームのノード数とする。
        count = this->getSubVolumeNodeCount();
    }
    int nkinds = this->m_nkinds;
    if (nkinds > 0) {
        count = nkinds * count;
    }

    return count;
}

/**
 * 自領域が担当する接続数を取得する
 * @return		自領域が担当する接続数
 */
Int64 PbvrFilterInfo::getSubVolumeConnCount()
{
    FilterDivision *division = factoryFilterDivision(this->m_param->division_type, this);

    Int64 count = 0;
    for (int i=0; i<this->m_nvolumes; i++) {
        if (division->isWriteMpiProcess(i)) {
            count += this->m_conns_per_subvolume[i];
        }
    }
    delete division;

    return count;
}

/**
 * 自領域が担当する要素数を取得する
 * @return		自領域が担当する要素数
 */
Int64 PbvrFilterInfo::getSubVolumeElemCount()
{
    FilterDivision *division = factoryFilterDivision(this->m_param->division_type, this);

    Int64 count = 0;
    for (int i=0; i<this->m_nvolumes; i++) {
        if (division->isWriteMpiProcess(i)) {
            count += this->m_elems_per_subvolume[i];
        }
    }
    delete division;

    return count;
}


/**
 * ノード位置を取得する
 * @return		ノード位置
 */
Int64 PbvrFilterInfo::getNodePosition(int subvolume) const
{
    Int64 count = 0;
    int nkinds = this->m_nkinds;
    if (nkinds <= 0) {
        nkinds = 1;
    }
    for (int i=0; i<this->m_nvolumes; i++) {
        if (subvolume == i) {
            break;
        }
        count += this->m_nodes_per_subvolume[i]*nkinds;
    }

    return count;
}

/**
 * 各ランクが担当するノード数リストを取得する
 * @return		各ランクが担当するノード数リスト
 */
Int64 PbvrFilterInfo::getScatterCounts(int num_rank, int* counts) const
{
    Int64 count = 0;
    for (int i=0; i<num_rank; i++) {
        counts[i] = 0;
    }
    int nkinds = this->m_nkinds;
    if (nkinds <= 0) {
        nkinds = 1;
    }
    for (int i=0; i<this->m_nvolumes; i++) {
        if (i+1>=num_rank) break;
        int vol_count = this->m_nodes_per_subvolume[i]*nkinds;
        counts[i+1] = vol_count;
        count += vol_count;
    }

    return count;
}


/**
 * 各ランクが担当するノード位置リストを取得する
 * @return		各ランクが担当するノード位置リスト
 */
int* PbvrFilterInfo::getScatterPositions(int num_rank, int* displs) const
{
    for (int i=0; i<num_rank; i++) {
        displs[i] = 0;
    }
    int nkinds = this->m_nkinds;
    if (nkinds <= 0) {
        nkinds = 1;
    }
    for (int i=0; i<this->m_nvolumes; i++) {
        if (i+1>=num_rank) break;
        displs[i+1] = this->getNodePosition(i);
    }

    return displs;
}


/**
 * 物理量（m_values）のアロケート及びMPI_Scattervによる配信を行う。
 * @return		RTC_OK/RTC_NG
 */
int PbvrFilterInfo::scatterValue()
{
    int rstat = RTC_OK;
    int rank = 0;
    int mpi_size = 1;
#ifdef MPI_EFFECT
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    {
        LOGOUT(FILTER_LOG, (char*)"|||| MPI_Bcast variable data start.\n");
        int *sendcounts = new int[mpi_size];
        int *displs = new int[mpi_size];
        int *displs_start = new int[mpi_size];
        int *displs_end = new int[mpi_size];
        memset(sendcounts, 0x00, sizeof(int)*mpi_size);
        memset(displs, 0x00, sizeof(int)*mpi_size);
        memset(displs_start, 0x00, sizeof(int)*mpi_size);
        memset(displs_end, 0x00, sizeof(int)*mpi_size);

	int my_displs_start, my_displs_end;

        if (rank == 0) {
            displs[0] = 0;
            //displs_start[0] = 0;
            //displs_end[0] = 0;
            my_displs_start = 0;
            my_displs_end = 0;
        }
        else {
            unsigned int subvolume_nodes = this->m_nodes_per_subvolume[rank-1];
            //displs_start[rank] = this->m_subvolume_nodes[0]*this->m_nkinds;
            //displs_end[rank] = this->m_subvolume_nodes[subvolume_nodes-1]*this->m_nkinds;
            my_displs_start = this->m_subvolume_nodes[0]*this->m_nkinds;
            my_displs_end = this->m_subvolume_nodes[subvolume_nodes-1]*this->m_nkinds;

        }
        //MPI_Allgather( &(displs_start[rank]), 1, MPI_INT, displs_start, 1, MPI_INT, MPI_COMM_WORLD);
        //MPI_Allgather( &(displs_end[rank]), 1, MPI_INT, displs_end, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Allgather( &my_displs_start, 1, MPI_INT, displs_start, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Allgather( &my_displs_end, 1, MPI_INT, displs_end, 1, MPI_INT, MPI_COMM_WORLD);

        for (int n = 0; n < mpi_size; n++) {
            sendcounts[n] = displs_end[n] - displs_start[n] + this->m_nkinds;
            displs[n] = displs_start[n];
        }

        if (rank != 0) {
            if (this->m_values != NULL) free(this->m_values);
            this->m_values = (float*)malloc(sizeof(float)*sendcounts[rank]);
            memset(this->m_values, 0x00, sizeof(float)*sendcounts[rank]);
        }

        if (rank == 0) {
            MPI_Scatterv(this->m_values, sendcounts, displs,
                    MPI_FLOAT, MPI_IN_PLACE, sendcounts[rank],
                    MPI_FLOAT, 0, MPI_COMM_WORLD);
        }
        else {
            MPI_Scatterv(this->m_values, sendcounts, displs,
                    MPI_FLOAT, this->m_values, sendcounts[rank],
                    MPI_FLOAT, 0, MPI_COMM_WORLD);
        }

        if (sendcounts != NULL) delete sendcounts;
        if (displs != NULL) delete displs;
        if (displs_start != NULL) delete displs_start;
        if (displs_end != NULL) delete displs_end;
     }

#endif
    return RTC_OK;
}


PbvrFilter::PbvrFilter() : m_filter_info(NULL), m_is_releaseinfo(true)  {
    this->m_filter_info = new PbvrFilterInfo();
    return;
}

/**
 * デストラクタ
 */
PbvrFilter::~PbvrFilter() {
    if (this->m_is_releaseinfo) {
        if (this->m_filter_info != NULL) delete this->m_filter_info;
    }

    /** delete by @hira : m_filter_listのPbvrFilterInfoは
     * PbvrFilterInformationFile::m_pbvrfilter::m_filter_infoに移動しているので
     * ここではdelete禁止。
     *****
    std::vector<PbvrFilterInfo*>::iterator itr;
    for (itr=this->m_filter_list.begin(); itr!=this->m_filter_list.end(); itr++) {
        PbvrFilterInfo* info = (*itr);
        if (info != NULL) delete info;
        info = NULL;
    }
    *****/

    this->m_filter_list.clear();

}



FilterConvert* PbvrFilter::factoryFilterConvert(int input_format, PbvrFilterInfo *filter_info) {

    FilterConvert* convert = NULL;
    switch (input_format) {
    case AVS_INPUT:
    case STL_ASCII:
    case STL_BIN:
        convert = new FilterConvert(filter_info, this);
        break;
    case VTK_INPUT:
#ifdef VTK_ENABLED
        convert = new FilterConvertVtk(filter_info, this);
#endif
        break;
    case PLOT3D_INPUT:
        convert = new FilterConvertPlot3d(filter_info, this);
        break;
    default:
        convert = new FilterConvert(filter_info, this);
    }

    return convert;
}

void PbvrFilter::setIsWrite(bool iswrite)
{
    if (this->m_filter_info == NULL) return;
    if (this->m_filter_info->m_param == NULL) return;
    this->m_filter_info->m_param->m_iswrite = iswrite;
}


bool PbvrFilter::isWrite()
{
    if (this->m_filter_info == NULL) return false;
    if (this->m_filter_info->m_param == NULL) return false;
    return this->m_filter_info->m_param->m_iswrite;
}

void PbvrFilter::setIsReleaseInfo(bool isrelease)
{
    this->m_is_releaseinfo = isrelease;
}

bool PbvrFilter::isReleaseInfo(bool isrelease)
{
    return this->m_is_releaseinfo;
}

PbvrFilterInfo* PbvrFilter::getPbvrFilterInfo() const {
    return this->m_filter_info;
}

void PbvrFilter::setPbvrFilterInfo(PbvrFilterInfo* info) {
    this->m_filter_info = info;
}

void PbvrFilter::deletePbvrFilterInfo() {
    if (this->m_filter_info != NULL) delete this->m_filter_info;
}


int PbvrFilter::loadPbvrFilter(const char* filename)
{
    if (filename == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;
    int status = RTC_NG;

#if 1	// for debug by @hira
    LOGOUT(FILTER_LOG, (char*)"[file:%s] %d : loadPbvrFilter start.[filename=%s]\n",
            __FILE__, __LINE__, filename);
#endif

    if (RTC_OK != read_param_file(filename, param)) {
        LOGOUT(FILTER_ERR, (char*)"read_filter_param error.\n");
        return -1;
    }
    this->m_filter_info->ENDIAN = check_endian();
    LOGOUT(FILTER_LOG, (char*)"#### %s endian\n", (this->m_filter_info->ENDIAN ? "Big" : "Little"));
    DEBUG(3, "File Input Format:%i \n", param->input_format);

    FilterConvert *convert = this->factoryFilterConvert(param->input_format, this->m_filter_info);
    if (convert == NULL) {
        ASSERT_FAIL("FilterConvert can not create.");
        return RTC_NG;
    }

    switch (param->input_format) {
    case AVS_INPUT:
        this->m_filter_info->m_elementsize = ucd_elementsize;
        status = convert->filter_convert();
        break;
    case STL_ASCII:
        status = convert->filter_convert();
        break;
    case STL_BIN:
        status = convert->filter_convert();
        break;
    case VTK_INPUT:
#ifdef VTK_ENABLED
        this->m_filter_info->m_elementsize = vtk_elementsize;
        status = convert->filter_convert();
#else
        ASSERT_FAIL("VTK Not enabled in this build\n");
#endif
        break;
    case PLOT3D_INPUT:
        status = convert->filter_convert();
        break;
    default:
        LOGOUT(FILTER_LOG, (char*)"WARNING!  Unknown input type: %s ", param->input_format);
        status = convert->filter_convert();
    }

    delete convert;

#if 1	// for debug by @hira
    LOGOUT(FILTER_LOG, (char*)"[file:%s] %d : loadPbvrFilter end.\n", __FILE__, __LINE__);
#endif
    return status;
}

/**
 * 指定ステップ、サブボリューム、要素タイプのモデル物理量の読み込みを行う。
 * MPI非対応：MPI対応する場合は、事前にloadMpiSubvolumeを呼び出すこと。
 * @param step_loop		ステップ番号(フィルタパラメータファイル記述(start_step〜end_step)
 * @param n_subvolume	サブボリュームID（0〜)
 * @param read_elem_type	要素タイプ
 * @return		RTC_OK/RTC_NG
 */
int PbvrFilter::readStepValue(int step_loop, int n_subvolume, int read_elem_type) {
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;

    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    int rstat = RTC_OK;

	// 存在しないステップのとき強制に変更
#if 1	// for debug by @hira
    LOGOUT(FILTER_LOG, (char*)"[file:%s] %d : readStepValue start.[step_loop=%d, n_subvolume=%d, read_elem_type=%d]\n",
            __FILE__, __LINE__, step_loop, n_subvolume, read_elem_type);
#endif

    if (info->m_values == NULL) {
        // ファイルからデータの読込みを行う
        rstat = this->readValue(step_loop, read_elem_type);
        if (rstat != RTC_OK) {
            LOGOUT(FILTER_ERR, (char*)"[Error] read value data[step_loop=%d,n_subvolume=%d,read_elem_type=%d].\n",
                    step_loop, n_subvolume, read_elem_type);
            return RTC_NG;
        }
    }

    FilterWrite *writer = new FilterWrite(info);
    FilterDivision *division = factoryFilterDivision(param->division_type, info);
    writer->setFilterDivision(division);

    int volm_loop;
    int subvolume_id;
    Int64 rank_nodes;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;
    Int64 size;
    int rank = 0;
    int mpi_size = 1;
#ifdef MPI_EFFECT
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

    /* サブボリュームのループ  | KVSML format file */
    rank_nodes = 0;
    for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0	// delete by @hira at 2016/12/01
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
        subvolume_elems = info->m_elems_per_subvolume[volm_loop];
        subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
        subvolume_conns = info->m_conns_per_subvolume[volm_loop];
        // add by @hira at 2017/04/01
        int node_offset = info->m_subvolume_nodes[0];
        if (volm_loop == n_subvolume) {
            /* VARIABLESの出力 */
            if (RTC_OK != writer->write_kvsml_value(step_loop,
                            volm_loop + 1,
                            info->m_nvolumes,
                            subvolume_nodes,
                            rank_nodes,
                            node_offset)) {
                LOGOUT(FILTER_ERR, (char*)"write_kvsml_value error.\n");
                goto garbage;
            }
            break;
        }
        rank_nodes += info->m_nodes_per_subvolume[volm_loop];
    } /* サブボリュームのループ */

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

garbage:

    LOGOUT(FILTER_LOG, (char*)":::: time_step %d end.\n", step_loop);
    if (division != NULL) delete division;
    if (writer != NULL) delete writer;

#if 1	// for debug by @hira
    LOGOUT(FILTER_LOG, (char*)"[file:%s] %d : readStepValue end.\n", __FILE__, __LINE__);
#endif

    return RTC_OK;
}


int PbvrFilter::count_cellTypes(const char* filename)
{
    if (filename == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

#if 1	// for debug by @hira
    LOGOUT(FILTER_LOG, (char*)"[file:%s] %d : count_cellTypes start.[filename=%s]\n",
            __FILE__, __LINE__, filename);
#endif

    if (RTC_OK != read_param_file(filename, param)) {
        LOGOUT(FILTER_ERR, (char*)"read_filter_param error.\n");
        return -1;
    }

    FilterIoPlot3d *io_plot3d = new FilterIoPlot3d(info);
    FilterIoAvs *io_avs = new FilterIoAvs(info);
    FilterIoStl *io_stl = new FilterIoStl(info);
#ifdef VTK_ENABLED
    FilterIoVtk *io_vtk = new FilterIoVtk(info);
#endif
    FilterConvert *convert = new FilterConvert(info, this);
    Int64 count = 0;
    int element_type_num = 0;

    this->m_filter_info->ENDIAN = check_endian();

    switch (param->input_format) {
    case AVS_INPUT:
        this->m_filter_info->m_elementsize = ucd_elementsize;
        break;
    case STL_ASCII:
        break;
    case STL_BIN:
        break;
    case VTK_INPUT:
#ifdef VTK_ENABLED
        this->m_filter_info->m_elementsize = vtk_elementsize;
        io_vtk->vtk_get_datasetInfo();
#else
        fprintf(stderr, "[Error] VTK Not enabled in this build\n");
        ASSERT_FAIL("VTK Not enabled in this build\n");
#endif
        break;
    case PLOT3D_INPUT:
        FilterConvertPlot3d *plot3d = new FilterConvertPlot3d(info, this);
        plot3d->initializePlot3d(io_plot3d);
        delete plot3d;
        break;
    }


    if (!param->struct_grid && param->input_format == VTK_INPUT) {
#ifdef VTK_ENABLED
        io_vtk->vtk_count_cellTypes();
#endif
    } else if (!param->struct_grid && param->mult_element_type) {
        /* 混合要素のファイル分割 */
        // convert->separate_files();
    }

    if (io_plot3d!= NULL) {
        FilterConvertPlot3d *plot3d = new FilterConvertPlot3d(info, this);
        plot3d->finalizePlot3d(io_plot3d);
        delete io_plot3d;
        delete plot3d;
    }
    if (io_avs != NULL) delete io_avs;
    if (io_stl != NULL) delete io_stl;
#ifdef VTK_ENABLED
    if (io_vtk != NULL) delete io_vtk;;
#endif
    if (convert != NULL) delete convert;

    count = 0;
    if (!param->struct_grid && info->m_multi_element) {
        for (int n=0; n<MAX_ELEMENT_TYPE; n++) {
            if (info->m_types[n] != 0) {
                count++;
            }
        }
    }
    else {
        count = 1;
    }

#if 1	// for debug by @hira
    LOGOUT(FILTER_LOG, (char*)"[file:%s] %d : count_cellTypes end.\n", __FILE__, __LINE__);
#endif
    return count;
}

/**
 * 分割方法、分割数を設定する。
 * @param division_type		分割方法
 * 				MPI_DIVISION：分割数による分割
 * 				MASTER_DIVISION：ランク０をマスターとして、分割数による分割（デフォルト）
 * 				PARAM_DIVISION：フィルタパラメータファイル記述（n_division）の分割数を使用する：デバッグ用
 * 				OCTREE_DIVISION：８分木分割：未使用
 * @param n_division		分割数
 */
void PbvrFilter::setFilterDivision(DIVISION_TYPE division_type, int n_division)
{
    if (this->m_filter_info == NULL) return;
    if (this->m_filter_info->m_param == NULL) return;

    this->m_filter_info->m_param->division_type = division_type;
    this->m_filter_info->m_param->n_division = n_division;
}


/**
 * 指定ステップ、サブボリューム、要素タイプのモデル物理量の読み込みを行う。
 * MPI対応：ランク０が読み込みを行い、全ランクに配信を行う。
 * 実際の物理量の取得は、readStepValueにて行うこと
 * @param step_loop		ステップ番号(フィルタパラメータファイル記述(start_step〜end_step)
 * @param read_elem_type	要素タイプ
 * @return		RTC_OK/RTC_NG
 */
int PbvrFilter::loadMpiSubvolume(int step_loop, int read_elem_type)
{
    PBVR_TIMER_STA(502);
    if (this->m_filter_info == NULL)
    {
         PBVR_TIMER_END(502);
         return RTC_NG;
    }
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    FilterWrite *writer = new FilterWrite(info);
    FilterDivision *division = factoryFilterDivision(param->division_type, info);
    writer->setFilterDivision(division);

    int rank = 0;
    int mpi_size = 1;
#ifdef MPI_EFFECT
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

#if 1	// for debug by @hira
    LOGOUT(FILTER_LOG, (char*)"[file:%s] %d : loadMpiSubvolume start.[step_loop=%d, read_elem_type=%d]\n",
                __FILE__, __LINE__, step_loop, read_elem_type);
#endif

    // 物理量のクリア
    this->releaseMpiSubvolume();
	// 存在しないステップの対応
    int rstat = RTC_OK;
    if (rank == 0) {
        // ファイルからデータの読込みを行う
        PBVR_TIMER_STA( 530 );
        rstat = this->readValue(step_loop, read_elem_type);
        PBVR_TIMER_END( 530 );
        if (rstat != RTC_OK) {
            LOGOUT(FILTER_ERR, (char*)"[Error] read value data[step_loop=%d,read_elem_type=%d].\n",
                    step_loop, read_elem_type);
        }
    }

#ifdef MPI_EFFECT
    MPI_Allreduce(MPI_IN_PLACE, &rstat, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if (rstat != RTC_OK) {
        PBVR_TIMER_END(502);
        return RTC_NG;
    }

#if 0
    if (rank != 0) {
        if (info->m_values != NULL) free(info->m_values);
        info->m_values = (float*)malloc(sizeof(float)*info->m_nkinds*info->m_nnodes);
        memset(info->m_values, 0x00, sizeof(float)*info->m_nkinds*info->m_nnodes);
    }
#endif

    if (mpi_size > 1) {

        LOGOUT(FILTER_LOG, (char*)"|||| MPI_Bcast variable data start.\n");
        MPI_Bcast(&info->m_stepn, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&info->m_stept, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Bcast(info->m_veclens, info->m_ncomponents, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(info->m_nullflags, info->m_nkinds, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(info->m_nulldatas, info->m_nkinds, MPI_FLOAT, 0, MPI_COMM_WORLD);
        // modify by @hira at 2017/04/01 : MPI_BcastからMPI_Scattervに変更
        // MPI_Bcast(info->m_values, info->m_nkinds*info->m_nnodes, MPI_FLOAT, 0, MPI_COMM_WORLD);
        LOGOUT(FILTER_LOG, (char*)"|||| MPI_Bcast variable data end.\n");
     }

    // 物理量（m_values）のアロケート及びMPI_Scattervによる配信を行う。
    info->scatterValue();

#endif


#if 1	// for debug by @hira
    LOGOUT(FILTER_LOG, (char*)"[file:%s] %d : loadMpiSubvolume end.\n", __FILE__, __LINE__);
#endif

    // delete by @hira
// #ifdef _DEBUG_DELETE
#if  0

#ifdef MPI_EFFECT
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

    /* サブボリュームのループ  | KVSML format file */
    int rank_nodes = 0;
    for (int volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
        // add by @hira at 2016/12/01
        if (!division->isWriteMpiProcess(volm_loop)) {
            continue;
        }

        int subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
        int subvolume_elems = info->m_elems_per_subvolume[volm_loop];
        int subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
        int subvolume_conns = info->m_conns_per_subvolume[volm_loop];
        /* VARIABLESの出力 */
        if (RTC_OK != writer->write_kvsml_value(step_loop,
                        volm_loop + 1,
                        info->m_nvolumes,
                        subvolume_nodes,
                        rank_nodes)) {
            LOGOUT(FILTER_ERR, (char*)"write_kvsml_value error.\n");
            PBVR_TIMER_END(502);
            return RTC_NG;
        }
        rank_nodes += info->m_nodes_per_subvolume[volm_loop];
    } /* サブボリュームのループ */

    {
        LOGOUT(FILTER_LOG, (char*)"|||| MPI_Bcast variable data start.\n");
        int *sendcounts = new int[mpi_size];
        int *displs = new int[mpi_size];
        int *displs_start = new int[mpi_size];
        int *displs_end = new int[mpi_size];
        memset(sendcounts, 0x00, sizeof(int)*mpi_size);
        memset(displs, 0x00, sizeof(int)*mpi_size);
        memset(displs_start, 0x00, sizeof(int)*mpi_size);
        memset(displs_end, 0x00, sizeof(int)*mpi_size);

        if (rank == 0) {
            displs[0] = 0;
            displs_start[0] = 0;
            displs_end[0] = 0;
        }
        else {
            unsigned int subvolume_nodes = info->m_nodes_per_subvolume[rank-1];
            displs_start[rank] = info->m_subvolume_nodes[0]*info->m_nkinds;
            displs_end[rank] = info->m_subvolume_nodes[subvolume_nodes-1]*info->m_nkinds;

        }
        MPI_Allgather( &(displs_start[rank]), 1, MPI_INT, displs_start, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Allgather( &(displs_end[rank]), 1, MPI_INT, displs_end, 1, MPI_INT, MPI_COMM_WORLD);

        for (int n = 0; n < mpi_size; n++) {
            sendcounts[n] = displs_end[n] - displs_start[n] + info->m_nkinds;
            displs[n] = displs_start[n];
        }

        for (int volm_loop = 0; volm_loop < info->m_nvolumes+1; volm_loop++) {
            printf("[rank:%d] sendcounts[%d]=%d, displs[%d]=%d\n",
                    rank, volm_loop, sendcounts[volm_loop], volm_loop, displs[volm_loop]);
        }
        if (rank == 0) {
            MPI_Scatterv(info->m_values, sendcounts, displs,
                    MPI_FLOAT, MPI_IN_PLACE, sendcounts[rank],
                    MPI_FLOAT, 0, MPI_COMM_WORLD);
        }
        else {
            MPI_Scatterv(info->m_values, sendcounts, displs,
                    MPI_FLOAT, info->m_values, sendcounts[rank],
                    MPI_FLOAT, 0, MPI_COMM_WORLD);
        }

        if (rank > 0) {
            if (info->m_pbvr_values.size() != sendcounts[rank]) {
                printf("[rank:%d] not equals! : m_pbvr_values.size()=%d, sendcounts[%d]=%d \n",
                            rank, info->m_pbvr_values.size(), rank, sendcounts[rank]);
            }
            std::vector<float>::iterator itr;
            size_t value_id = 0;

            int offset = info->m_subvolume_nodes[0];
            for (int k = 0; k < info->m_nkinds; k++) {
                for (int i = 0; i < info->m_nodes_per_subvolume[rank-1]; i++) {
                    int nid = info->m_subvolume_nodes[i] - offset;
                    int j = nid*info->m_nkinds;
                    float value  = info->m_values[j + k];
                    if (info->m_pbvr_values[value_id] != value) {
                        printf("[rank:%d, value_id=%d] info->m_values[%d]=%E, m_pbvr_values[%d]=%E\n",
                                    rank, value_id, j + k, value, value_id, info->m_pbvr_values[value_id]);
                    }
                    value_id++;
                }
            }
        }
        if (sendcounts != NULL) delete sendcounts;
        if (displs != NULL) delete displs;
        if (displs_start != NULL) delete displs_start;
        if (displs_end != NULL) delete displs_end;
     }
#endif

    if (writer != NULL) delete writer;
    writer = NULL;
    if (division != NULL) delete division;
    division = NULL;
    PBVR_TIMER_END(502);
    return RTC_OK;
}

/**
 * 指定ステップ、サブボリューム、要素タイプのモデル物理量の読込の後処理を行う。
 */
void PbvrFilter::releaseMpiSubvolume() const
{
    if (this->m_filter_info == NULL) return;
    if (this->m_filter_info->m_values != NULL) {
        free(this->m_filter_info->m_values);
    }
    this->m_filter_info->m_values = NULL;
    this->m_filter_info->m_pbvr_values.clear();
    std::vector<float>().swap(this->m_filter_info->m_pbvr_values);

    return;
}


/**
 * 後処理を行う。
 */
void PbvrFilter::finalize()
{
    if (this->m_filter_info != NULL) {
        delete this->m_filter_info;
        this->m_filter_info = NULL;
    }

    std::vector<PbvrFilterInfo*>::iterator itr;
    for (itr=this->m_filter_list.begin(); itr!=this->m_filter_list.end(); itr++) {
        PbvrFilterInfo* info = (*itr);
        if (info != NULL) delete info;
        info = NULL;
    }
    this->m_filter_list.clear();

    return;
}


/**
 * 指定ステップ、要素タイプのモデル物理量をファイルから読み込みを行う。
 * @param step_loop		ステップ番号(フィルタパラメータファイル記述(start_step〜end_step)
 * @param read_elem_type	要素タイプ
 * @return		RTC_OK/RTC_NG
 */
int PbvrFilter::readValue(int step_loop, int read_elem_type)
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;

    PBVR_TIMER_STA( 531 );
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    FilterIoPlot3d *io_plot3d = new FilterIoPlot3d(info);
    FilterIoAvs *io_avs = new FilterIoAvs(info);
    FilterIoStl *io_stl = new FilterIoStl(info);
#ifdef VTK_ENABLED
    FilterIoVtk *io_vtk = new FilterIoVtk(info);
#endif
    FilterConvert *convert = new FilterConvert(info, this);


    int rstat = RTC_OK;
    Int64 count = 0;
    int element_type_num = 0;
    char m_out_prefix[LINELEN_MAX];
    char paramfile[LINELEN_MAX] = {0x00};
    Int64 size = 0;
    bool avs_unstruct_multi_type = false;

#if 1	// for debug by @hira
    LOGOUT(FILTER_LOG, (char*)"[file:%s] %d : readValue start.[step_loop=%d, read_elem_type=%d]\n",
                __FILE__, __LINE__, step_loop, read_elem_type);
#endif

    strcpy(paramfile, param->m_paramfile);
    if (RTC_OK != read_param_file(paramfile, param)) {
        LOGOUT(FILTER_ERR, (char*)"read_filter_param error.\n");
        return -1;
    }

	// 存在しないステップが指定されたらstart、end近い方にする
	int c_step_loop = step_loop;
	if (param->start_step > step_loop) c_step_loop = param->start_step;
	else if (param->end_step < step_loop) c_step_loop = param->end_step;


    switch (param->input_format) {
    case AVS_INPUT:
        this->m_filter_info->m_elementsize = ucd_elementsize;
        break;
    case STL_ASCII:
        break;
    case STL_BIN:
        break;
    case VTK_INPUT:
#ifdef VTK_ENABLED
        this->m_filter_info->m_elementsize = vtk_elementsize;
        io_vtk->vtk_get_datasetInfo();
#else
        ASSERT_FAIL("VTK Not enabled in this build\n");
#endif
        break;
    case PLOT3D_INPUT:
        FilterConvertPlot3d *plot3d = new FilterConvertPlot3d(info, this);
        plot3d->initializePlot3d(io_plot3d);
        delete plot3d;
        break;
    }

    if (strlen(param->ucd_inp) != 0) {
        if (RTC_OK != convert->read_inp_file()) {
            LOGOUT(FILTER_ERR, (char*)"read_ucd_inp_file error.\n");
            goto garbage;
        }
    }
    info->m_multi_element = param->mult_element_type;

    LOGOUT(FILTER_LOG, (char*)"filter_convert::struct_grid      : %d\n", param->struct_grid);
    LOGOUT(FILTER_LOG, (char*)"filter_convert::input_format      : %d\n", param->input_format);

    if (!param->struct_grid && param->input_format == VTK_INPUT) {
#ifdef VTK_ENABLED
        io_vtk->vtk_count_cellTypes();
        if (param->mult_element_type) {
            sprintf(m_out_prefix, "%s", param->out_prefix);
        } else {
            sprintf(m_out_prefix, "%s", "");
        }
#endif
    } else if (!param->struct_grid && param->mult_element_type) {

#ifdef MULTI_SEPARATE_FILES
        avs_unstruct_multi_type = false;
        /* 混合要素のファイル分割 */
        PbvrFilterInfo *src_info = new PbvrFilterInfo(*info);
        convert->separate_files();
        info->copyInfos(*src_info);
        delete src_info;
        param = info->m_param;
        sprintf(m_out_prefix, "%s", param->out_prefix);
#else
        avs_unstruct_multi_type = true;
        // 要素タイプを取得する
        // io_avs->read_elementtypes();
#endif
    }

    PBVR_TIMER_END( 531 );
    PBVR_TIMER_STA( 532 );
    // 読込要素タイプ(read_elem_type)を探索する
    if (!param->struct_grid && info->m_multi_element) {
        DEBUG(1, "############ unstructured multi element::::: \n");
        while (FilterConvert::m_elementtypeno[element_type_num] != read_elem_type) {
            element_type_num++;
            if (element_type_num >= MAX_ELEMENT_TYPE) {
                DEBUG(3, "############ u unstruct_element_stop;:::: \n");
                goto unstruct_element_stop;
            }
        }
        if (info->m_types[element_type_num] == 0) {
            printf("[Error] Not find read elementtype[%d]. m_types[%d]=%d\n",
                    read_elem_type, element_type_num, info->m_types[element_type_num]);
            DEBUG(3, "############ u unstruct_element_stop;:::: \n");
            goto unstruct_element_stop;
        }
        if (param->input_format != VTK_INPUT) {
#ifdef MULTI_SEPARATE_FILES
            sprintf(param->in_prefix, "%02d-%s", FilterConvert::m_elementtypeno[element_type_num], m_out_prefix);
            sprintf(param->out_prefix, "%02d-%s", FilterConvert::m_elementtypeno[element_type_num], m_out_prefix);
            sprintf(param->in_dir, "%s", param->out_dir);
#endif
        }
#ifdef VTK_ENABLED
        else {
            if (param->mult_element_type) {
                sprintf(param->out_prefix, "%02d-%s", info->m_element_type, m_out_prefix);
            }
        }
#endif
        DEBUG(1, "##############>%s\n", param->out_prefix);
    }
    else if (param->input_format == VTK_INPUT) {
        element_type_num= info->m_element_type;
    }
    // modify by @hira at 2017/04/25 : Bs, Be対応
    // if (step_loop <= param->start_step) {
    if (info->step_count == 0 || info->pre_step >= c_step_loop) {
        info->m_nsteps = param->end_step - param->start_step + 1;
        info->pre_step = param->start_step - 1;

        if (param->struct_grid) {
            if (info->m_value_fp != NULL) {
                for (int i = 0; i < info->m_nkinds; i++) {
                    if (info->m_value_fp[i] != NULL) fclose(info->m_value_fp[i]);
                    info->m_value_fp[i] = NULL;
                }
                free(info->m_value_fp);
                info->m_value_fp = NULL;
            }
            info->m_value_fp = (FILE **) malloc(sizeof (FILE*) * info->m_nkinds);
            for (int i = 0; i < info->m_nkinds; i++) {
                info->m_value_fp[i] = NULL;
            }
        }
    }
    info->step_count++;

    LOGOUT(FILTER_LOG, (char*)":::: time_step %d start.\n", c_step_loop);

    if (info->m_values != NULL) free(info->m_values);
    info->m_values = (float*)malloc(info->m_nnodes * info->m_nkinds * sizeof (float));
    memset(info->m_values, 0x00, info->m_nnodes * info->m_nkinds * sizeof (float));

    /* データ読込み  Read data*/
    LOGOUT(FILTER_LOG, (char*)"|||| read variable data start.\n");
    if (param->input_format == PLOT3D_INPUT) {
        if (RTC_OK != io_plot3d->plot3d_setBlockParameters()) {
            LOGOUT(FILTER_ERR, (char*)"read_filter_fld error.\n");
            goto garbage;
        };

        info->m_stepn = c_step_loop;
        rstat = io_plot3d->plot3d_getBlockValuesForTstep(c_step_loop);
    }
#ifdef VTK_ENABLED
    else if (param->input_format == VTK_INPUT) {
        if (param->struct_grid) {
            rstat = io_vtk->vtk_read_structured_values(c_step_loop);
        } else {
            DEBUG(1, " reading step variables for element_type_number %d\n\n", element_type_num);
            rstat = io_vtk->vtk_read_unstructured_values(c_step_loop, element_type_num);
            //		ASSERT_FAIL("F");
        }
    }
#endif
    else if (param->input_format == STL_ASCII || param->input_format == STL_BIN) {
        LOGOUT(FILTER_LOG, (char*)"ENTER STL BRANCH.\n");
        // When reading STL the there are no values
        // so set summy values
        for (count = 0; count < info->m_nnodes; count++) {
            info->m_values[count] = FLT_MIN;
        }
    } else if (param->struct_grid) {
        info->m_stepn = c_step_loop;
        if (strlen(param->in_prefix) == 0) {
            /*rstat=read_variables( step_loop );*/
            rstat = io_avs->read_variables_3(c_step_loop - param->start_step);
        } else {
            rstat = io_avs->read_variables_per_step(c_step_loop);
        }
        if (RTC_OK != rstat) {
            LOGOUT(FILTER_ERR, (char*)"read_variables_per_step error.\n");
            goto garbage;
        }
    }
#ifndef MULTI_SEPARATE_FILES
    else if (avs_unstruct_multi_type) {
        // 要素タイプを指定して読み込みを行う  add by @hira at 2017/04/15
        if (RTC_OK != io_avs->read_unstructer_multi_value(element_type_num, c_step_loop)) {
            LOGOUT(FILTER_ERR, (char*)"read unstructure multi error.[element_type_num=%d]\n", element_type_num);
            goto garbage;
        }
    }
#endif
    else {
        if (strlen(param->ucd_inp) != 0 && info->m_multi_element == 0) {
            if (info->m_ucdbin == 0) {
                rstat = io_avs->skip_ucd_ascii(c_step_loop);
                if (info->m_cycle_type == 1) {
                    rstat = io_avs->skip_ucd_ascii_geom();
                } else if (info->m_cycle_type == 2) {
                    if (c_step_loop == param->start_step) {
                        rstat = io_avs->skip_ucd_ascii_geom();
                    }
                }
                rstat = io_avs->read_ucd_ascii_value(c_step_loop);
            } else {
                rstat = io_avs->read_ucdbin_data(c_step_loop);
            }
        } else {
            rstat = io_avs->read_ucdbin_data(c_step_loop);
        }
        if (RTC_OK != rstat) {
            LOGOUT(FILTER_ERR, (char*)"read_ucdbin_data error.\n");
            goto garbage;
        }
    }
    LOGOUT(FILTER_LOG, (char*)"|||| read variable data end.\n");

unstruct_element_stop:
garbage:

    LOGOUT(FILTER_LOG, (char*)":::: time_step %d end.\n", c_step_loop);

    if (io_plot3d!= NULL) {
        FilterConvertPlot3d *plot3d = new FilterConvertPlot3d(info, this);
        plot3d->finalizePlot3d(io_plot3d);
        delete io_plot3d;
        delete plot3d;
    }
    if (io_avs != NULL) delete io_avs;
    if (io_stl != NULL) delete io_stl;
#ifdef VTK_ENABLED
    if (io_vtk != NULL) delete io_vtk;;
#endif
    if (convert != NULL) delete convert;

#if 1	// for debug by @hira
    LOGOUT(FILTER_LOG, (char*)"[file:%s] %d : readValue end.\n", __FILE__, __LINE__);
#endif

    PBVR_TIMER_END( 532 );
    return RTC_OK;
}

/**
 * サブボリュームが出力対象のプロセスであるかチェックする。
 * @param n_subvolume		サブボリュームID（0〜)
 * @return		true=出力対象のプロセス
 */
bool PbvrFilter::isWriteMpiProcess(int n_subvolume) const {
    if (this->m_filter_info == NULL) return false;
    if (this->m_filter_info->m_param == NULL) return false;

    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    bool is_write = false;
    FilterDivision *division = factoryFilterDivision(param->division_type, info);
    if (division != NULL) {
        is_write = division->isWriteMpiProcess(n_subvolume);
        delete division;
    }

    return is_write;

}

/**
 * 読込PbvrFilterInfoリストを取得する。複数要素対応
 * @return		PbvrFilterInfoリスト
 */
std::vector<PbvrFilterInfo*>& PbvrFilter::getPbvrFilterList()
{
    return this->m_filter_list;
}

/**
 * 読込PbvrFilterInfoを追加する。複数要素対応
 * @param    info		PbvrFilterInfo
 * @return		PbvrFilterInfoリストサイズ
 */
int PbvrFilter::setPbvrFilterList(PbvrFilterInfo* info)
{
    PbvrFilterInfo *copy_info = new PbvrFilterInfo(*info);
    this->m_filter_list.push_back(copy_info);

    return this->m_filter_list.size();
}


} // end of namespace filter
} // end of namespace pbvr

