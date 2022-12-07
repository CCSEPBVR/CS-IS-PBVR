/*
 * FilterSubVolume.cpp
 *
 *  Created on: 2016/12/24
 *      Author: hira
 */

#include <math.h>
#include <float.h>
#include "FilterSubVolume.h"
#include "filter_log.h"

namespace pbvr {
namespace filter {

/**
 * コンストラクタ
 * @param filter_info		フィルタ情報
 */
FilterSubVolume::FilterSubVolume()
    : m_filter_info(NULL), rank_nodes(0), rank_elems(0) {


}

/**
 * コンストラクタ
 * @param filter_info		フィルタ情報
 */
FilterSubVolume::FilterSubVolume(PbvrFilterInfo *filter_info)
    : m_filter_info(filter_info), rank_nodes(0), rank_elems(0) {

}

/**
 * デストラクタ
 */
FilterSubVolume::~FilterSubVolume() {
}

/*
 * サブボリューム内の要素・節点
 *
 */
int FilterSubVolume::count_in_subvolume(
                int subvolume_no,
                Int64* subelemnum,
                Int64* subnodenum,
                Int64* subconnnum)
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

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo* info = this->m_filter_info;
    FilterParam* param = info->m_param;
    int *m_subvolume_num = info->m_subvolume_num;
    unsigned int *m_elems_per_subvolume = info->m_elems_per_subvolume;
    int *m_conn_top_node = info->m_conn_top_node;
    char *m_elemtypes = info->m_elemtypes;
    const int *m_elementsize = info->m_elementsize;
    int *m_connections = info->m_connections;
    char *m_nodeflag = info->m_nodeflag;
    int *m_subvolume_elems = info->m_subvolume_elems;

    shift = info->m_tree_node_no - info->m_nvolumes;
    *subelemnum = 0;
    *subnodenum = 0;
    *subconnnum = 0;

    if (param->mult_element_type) {
        DEBUG(4,"######## count in subvolume multi element type %ld \n", info->m_nelements);
        Int64 addr;
        addr = 0;
        for (i = 0; i < info->m_nelements; i++) {
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
                    elem_id = info->m_element_array[subvolume_no - shift][i];
                    m_subvolume_elems[this->rank_elems + i] = elem_id;
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
                    elem_id = info->m_element_array[subvolume_no - shift][i];
                    m_subvolume_elems[this->rank_elems + i] = elem_id;
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
            size = info->m_elemcoms;
            for (i = 0; i < elem_num; i++) {
                elem_id = info->m_element_array[subvolume_no - shift][i];
                m_subvolume_elems[this->rank_elems + i] = elem_id;
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

    if ((this->rank_nodes + *subnodenum) > info->prov_len) {
        if (RTC_OK != this->realloc_work(this->rank_nodes + (*subnodenum))) {
            LOGOUT(FILTER_ERR, (char*)"realloc_work error.\n");
            return (RTC_NG);
        }
        info->prov_len = this->rank_nodes + (*subnodenum);
    }


    node_num = 0;
    for (i = 0; i < info->m_nnodes; i++) {
        if (m_nodeflag[i]) {
            info->m_subvolume_nodes[this->rank_nodes + node_num] = i;
            info->m_subvolume_trans[this->rank_nodes + node_num] = node_num + 1;
            node_num++;
            m_nodeflag[i] = 0;
        }
    }

    return (stat);
}

int FilterSubVolume::realloc_work(Int64 new_len)
{
    int *p;
    if (this->m_filter_info == NULL) return RTC_NG;

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
    return (RTC_OK);
}


int FilterSubVolume::get_coords_minmax(
        int volume_id,
        Int64 rank_nodes,
        float min[3], float max[3])
{

    Int64 i, j;
    unsigned int nid;
    float coord[3] = {0.0};

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    if (this->m_filter_info->m_nodes_per_subvolume[volume_id] <= 0) return RTC_NG;

    PbvrFilterInfo* info = this->m_filter_info;
    FilterParam* param = info->m_param;
    Int64 subvolume_nodes = info->m_nodes_per_subvolume[volume_id];

    // coordinates
    min[0] = min[1] = min[2] = FLT_MAX;
    max[0] = max[1] = max[2] = (-1)*FLT_MAX;

#ifdef _OPENMP
    {
#pragma omp parallel for default(shared) private(i,nid,j,coord)
#endif
    for (Int64 i = 0; i < subvolume_nodes; i++) {
        nid = info->m_subvolume_nodes[rank_nodes + i];
        j = nid * 3;
        coord[0] = info->m_coords[j ];
        coord[1] = info->m_coords[j + 1];
        coord[2] = info->m_coords[j + 2];
        if (min[0] > coord[0]) min[0] = coord[0];
        if (min[1] > coord[1]) min[1] = coord[1];
        if (min[2] > coord[2]) min[2] = coord[2];
        if (max[0] < coord[0]) max[0] = coord[0];
        if (max[1] < coord[1]) max[1] = coord[1];
        if (max[2] < coord[2]) max[2] = coord[2];
    }
#ifdef _OPENMP
    }
#endif

    return RTC_OK;
}


} /* namespace filter */
} /* namespace pbvr */
