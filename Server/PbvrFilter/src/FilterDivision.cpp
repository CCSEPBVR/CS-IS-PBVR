/*
 * FilterDivision.cpp
 *
 *  Created on: 2016/12/24
 *      Author: hira
 */

#include <string.h>
#include "FilterDivision.h"
#include "FilterDivisionOctree.h"
#include "filter_log.h"

namespace pbvr {
namespace filter
{


FilterDivision* factoryFilterDivision(DIVISION_TYPE division_type, PbvrFilterInfo *filter_info)
{
    FilterDivision *division = NULL;
    if (division_type == MPI_DIVISION
        || division_type == MASTER_DIVISION
        || division_type == PARAM_DIVISION) {
        division = new FilterDivision(filter_info);
    }
    else if (division_type == OCTREE_DIVISION) {
        division = new FilterDivisionOctree(filter_info);
    }
    else {
        division = new FilterDivision(filter_info);
    }

    return division;
}


/**
 * コンストラクタ
 */
FilterDivision::FilterDivision() : m_filter_info(NULL) {
}

/**
 * コンストラクタ
 * @param filter_info		フィルタ情報
 */
FilterDivision::FilterDivision(PbvrFilterInfo *filter_info)
    : m_filter_info(filter_info) {

}

/**
 * デストラクタ
 */
FilterDivision::~FilterDivision() {
}

/**
 * 分割タイプを取得する。
 * 分割タイプ：0 = MPIプロセス数による分割
 * 			 1 = パラメータ:n_division設定値による分割
 * 			 2 = パラメータ:n_layer設定値による8分木分割
 * @return   分割タイプ
 */
DIVISION_TYPE FilterDivision::getDivisionType() const {
    return this->m_filter_info->m_param->division_type;
}


/*
 * 要素を分割する:structure(2d)
 *
 */
#ifdef _OPENMP
#define _OPENMP_SW_XXXX
#endif

int FilterDivision::distribution_structure_2d() {
    int state = RTC_NG;
    unsigned int inum;
    unsigned int jnum;
    Int64 ii, jj;
    Int64 nid;
    Int64 eid;
    unsigned int cnt;
    unsigned int vol_num;
    unsigned int shift;
    unsigned int size;

    if (this->m_filter_info == NULL) return RTC_NG;
    FilterParam* param = this->m_filter_info->m_param;
    int *m_subvolume_num = this->m_filter_info->m_subvolume_num;
    unsigned int *m_elems_per_subvolume = this->m_filter_info->m_elems_per_subvolume;
    int *m_conn_top_node = this->m_filter_info->m_conn_top_node;

    inum = param->dim1;
    jnum = param->dim2;
    //    knum = param->dim3;
    shift = this->m_filter_info->m_tree_node_no - this->m_filter_info->m_nvolumes;

    /* 格子点を振り分け */
#ifdef _OPENMP_SW
#pragma omp parallel default(shared)
    {
#pragma omp for private(ii,jj,nid,,eid,j,k,xx,yy,zz,vol_num)
#endif
    for (jj = 0; jj < jnum - 1; jj++) {
        for (ii = 0; ii < inum - 1; ii++) {
            nid = ii + (inum) * jj + 1;
            eid = ii + (inum - 1) * jj;
            nid = nid - 1;

            vol_num = this->get_division_id(eid, this->m_filter_info->m_nelements, this->m_filter_info->m_nvolumes);
            vol_num += 1;

            m_subvolume_num[eid] = vol_num;
#ifdef _OPENMP_SW
#pragma omp critical
    {
#endif
            m_elems_per_subvolume[vol_num - shift]++;
#ifdef _OPENMP_SW
    }
#endif
            m_conn_top_node[eid] = nid + 1;
        }
    }

#ifdef _OPENMP_SW
    }
#endif

    this->m_filter_info->m_element_array = (unsigned int **) malloc(sizeof (unsigned int*)*this->m_filter_info->m_nvolumes);
    ii = 0;
    for (jj = 0; jj < this->m_filter_info->m_nvolumes; jj++) {
        size = m_elems_per_subvolume[jj];
        m_elems_per_subvolume[jj] = 0;
        this->m_filter_info->m_element_array[jj] = (unsigned int *) malloc(sizeof (unsigned int)*size);
        ii = ii + size;
    }
    if (ii != this->m_filter_info->m_nelements) {
        return (RTC_NG);
    }

    for (jj = 0; jj < jnum - 1; jj++) {
        for (ii = 0; ii < inum - 1; ii++) {
            nid = ii + (inum) * jj + 1;
            eid = ii + (inum - 1) * jj;
            nid = nid - 1;

            vol_num = this->get_division_id(eid, this->m_filter_info->m_nelements, this->m_filter_info->m_nvolumes);
            vol_num += 1;

            cnt = m_elems_per_subvolume[vol_num - shift];
            this->m_filter_info->m_element_array[vol_num - shift][cnt] = eid;
            cnt++;
            m_elems_per_subvolume[vol_num - shift] = cnt;
        }
    }

    state = RTC_OK;
    return (state);
}
#ifdef _OPENMP_SW
#undef _OPENMP_SW
#endif

/*
 * 要素を分割する:structure
 *
 */
int FilterDivision::distribution_structure() {
    int state = RTC_NG;
    unsigned int inum;
    unsigned int jnum;
    unsigned int knum;
    Int64 ii, jj, kk;
    Int64 nid;
    Int64 eid;
    unsigned int cnt;
    unsigned int vol_num;
    unsigned int size;
    if (this->m_filter_info == NULL) return RTC_NG;

    FilterParam* param = this->m_filter_info->m_param;
    int *m_subvolume_num = this->m_filter_info->m_subvolume_num;
    unsigned int *m_elems_per_subvolume = this->m_filter_info->m_elems_per_subvolume;
    int *m_conn_top_node = this->m_filter_info->m_conn_top_node;

    inum = param->dim1;
    jnum = param->dim2;
    knum = param->dim3;

    /* 格子点を振り分け */
#ifdef _OPENMP_SW
#pragma omp parallel default(shared)
    {
#pragma omp for private(ii,jj,kk,nid,,eid,j,k,xx,yy,zz,vol_num)
#endif
    for (kk = 0; kk < knum-1; kk++) {
        for (jj = 0; jj < jnum-1; jj++) {
            for (ii = 0; ii < inum-1; ii++) {
                nid = ii + (inum) * jj + (inum)*(jnum) * kk + 1;
                eid = ii + (inum - 1) * jj + (inum - 1)*(jnum - 1) * kk;
                nid = nid - 1;

                vol_num = this->get_division_id(eid, this->m_filter_info->m_nelements, this->m_filter_info->m_nvolumes);
                vol_num += 1;

                m_subvolume_num[eid] = vol_num;
#ifdef _OPENMP_SW
#pragma omp critical
    {
#endif
                m_elems_per_subvolume[vol_num - 1]++;
#ifdef _OPENMP_SW
    }
#endif
                m_conn_top_node[eid] = nid + 1;
            }
        }
    }

#ifdef _OPENMP_SW
    }
#endif

    this->m_filter_info->m_element_array = (unsigned int **) malloc(sizeof (unsigned int*)*this->m_filter_info->m_nvolumes);
    ii = 0;
    for (jj = 0; jj < this->m_filter_info->m_nvolumes; jj++) {
        size = m_elems_per_subvolume[jj];
        m_elems_per_subvolume[jj] = 0;
        this->m_filter_info->m_element_array[jj] = (unsigned int *) malloc(sizeof (unsigned int)*size);
        ii = ii + size;
    }
    if (ii != this->m_filter_info->m_nelements) {
        return (RTC_NG);
    }

    for (kk = 0; kk < knum - 1; kk++) {
        for (jj = 0; jj < jnum - 1; jj++) {
            for (ii = 0; ii < inum - 1; ii++) {
                nid = ii + (inum) * jj + (inum)*(jnum) * kk + 1;
                eid = ii + (inum - 1) * jj + (inum - 1)*(jnum - 1) * kk;
                nid = nid - 1;

                vol_num = this->get_division_id(eid, this->m_filter_info->m_nelements, this->m_filter_info->m_nvolumes);
                vol_num += 1;

                cnt = m_elems_per_subvolume[vol_num - 1];
                this->m_filter_info->m_element_array[vol_num - 1][cnt] = eid;
                cnt++;
                m_elems_per_subvolume[vol_num - 1] = cnt;
            }
        }
    }

    state = RTC_OK;
    return (state);
}
#ifdef _OPENMP_SW
#undef _OPENMP_SW
#endif



/*
 * 要素を分割する:unstructure
 *
 */
int FilterDivision::distribution_unstructure() {
    int state = RTC_NG;
    char rcode = 0;
    Int64 ii;
    Int64 i;
    unsigned int cnt;
    unsigned int vol_num;
    unsigned int size;

    if (this->m_filter_info == NULL) return RTC_NG;

    int *m_subvolume_num = this->m_filter_info->m_subvolume_num;
    unsigned int *m_elems_per_subvolume = this->m_filter_info->m_elems_per_subvolume;

    /* 要素を振り分け */
    for (ii = 0; ii < this->m_filter_info->m_nelements; ii++) {
        vol_num = this->get_division_id(ii, this->m_filter_info->m_nelements, this->m_filter_info->m_nvolumes);
        vol_num += 1;

        m_subvolume_num[ii] = vol_num;
        m_elems_per_subvolume[vol_num - 1]++;
    }

    DEBUG(3,"############# passed branch \n");
    this->m_filter_info->m_element_array = (unsigned int **) malloc(sizeof (unsigned int*)*this->m_filter_info->m_nvolumes);
    ii = 0;
    for (i = 0; i < this->m_filter_info->m_nvolumes; i++) {
        size = m_elems_per_subvolume[i];
        m_elems_per_subvolume[i] = 0;
        this->m_filter_info->m_element_array[i] = (unsigned int *) malloc(sizeof (unsigned int)*size);
        ii = ii + size;
    }
    if (ii != this->m_filter_info->m_nelements) {
        return (RTC_NG);
    }

    for (ii = 0; ii < this->m_filter_info->m_nelements; ii++) {
        vol_num = this->get_division_id(ii, this->m_filter_info->m_nelements, this->m_filter_info->m_nvolumes);
        vol_num += 1;

        cnt = m_elems_per_subvolume[vol_num - 1];
        this->m_filter_info->m_element_array[vol_num - 1][cnt] = ii;
        cnt++;
        m_elems_per_subvolume[vol_num - 1] = cnt;
    }

    state = RTC_OK;
    if (rcode) {
        state = RTC_NG;
    } else {
        state = RTC_OK;
    }
    return (state);
}


/**
 * 要素の分割IDを取得する
 * @param  eid     要素ID
 * @param  m_nelements     要素数
 * @param  m_nvolumes     分割数
 * @return   分割ID (0〜分割数-1)
 */
unsigned int FilterDivision::get_division_id(Int64 eid, Int64 m_nelements, Int64 m_nvolumes)
{
    if (m_nvolumes == 0) return 0;
    if (m_nvolumes == 1) return 0;

    // add by @hira at 2016/12/01
    if (this->m_filter_info->m_nelements < this->m_filter_info->m_nvolumes) {
        printf("[ERROR] too small model.[m_nelements=%lld,m_nvolumes=%d]\n",
                this->m_filter_info->m_nelements, this->m_filter_info->m_nvolumes);
        exit(1);
    }

    unsigned int vol_num = eid / (m_nelements/m_nvolumes);
    if (vol_num > m_nvolumes-1) vol_num = m_nvolumes-1;
    return vol_num;
}


int FilterDivision::get_nvolumes()
{
    if (this->m_filter_info == NULL) return 0;
    if (this->m_filter_info->m_param == NULL) return 0;
    FilterParam* param = this->m_filter_info->m_param;

    int m_nvolumes = param->n_division;
    DIVISION_TYPE type = getDivisionType();

    if (m_nvolumes <= 0) {
        int mpi_size = 0;
        if (type == MPI_DIVISION || type == MASTER_DIVISION) {
#ifdef MPI_EFFECT
            MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
#endif
        }
        m_nvolumes = mpi_size;
        if (type == MASTER_DIVISION) {
            m_nvolumes--;
        }
        if (m_nvolumes <= 0) m_nvolumes = 1;
    }

    this->m_filter_info->m_nvolumes = m_nvolumes;

    return this->m_filter_info->m_nvolumes;
}


int FilterDivision::create_tree_struct(NodeInf node_min, NodeInf node_max)
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    int nvolumes = this->get_nvolumes();
    /* 先頭に全体ツリーを作成（＋１） */
    info->m_tree_node_no = 1 + nvolumes;

    /* 領域確保 */
    info->m_tree = (HierCell *)malloc(sizeof(HierCell)*(info->m_tree_node_no));
    if (NULL == info->m_tree) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(tree).\n");
        return RTC_NG;
    }
    memset(info->m_tree, 0x00, sizeof(HierCell)*(info->m_tree_node_no));

    int i = 0;
    info->m_tree[i].parent = -1;
    info->m_tree[i].max.x = node_max.x;
    info->m_tree[i].min.x = node_min.x;
    info->m_tree[i].max.y = node_max.y;
    info->m_tree[i].min.y = node_min.y;
    info->m_tree[i].max.z = node_max.z;
    info->m_tree[i].min.z = node_min.z;


    return RTC_OK;
}

bool FilterDivision::isWriteMpiProcess(int volm_loop, int rank /*=-1*/) const
{
    if (this->m_filter_info == NULL) return false;
    FilterParam* param = this->m_filter_info->m_param;
    int numpe_v = this->m_filter_info->numpe_v;
    int mype_v = this->m_filter_info->mype_v;
    if (rank >= 0) {
        mype_v = rank;
    }
    if (numpe_v <= 0) return false;

    if (mype_v != volm_loop % numpe_v) {
        return false;
    }

    return true;
}



bool FilterDivision::getWriteMpiProcess(int volm_loop, int &rank, int &count) const
{
    if (this->m_filter_info == NULL) return false;
    int numpe_v = this->m_filter_info->numpe_v;
    if (numpe_v <= 0) return false;

    rank = volm_loop % numpe_v;
    count = (int)(volm_loop/numpe_v);

    return true;
}


int FilterDivision::distribution_subvolume(char struct_grid, int ndim) {

    if (struct_grid) {
        DEBUG(3, "distribution structure enter\n");
        if (ndim == 2) {
            if (RTC_OK != this->distribution_structure_2d()) {
                LOGOUT(FILTER_ERR, (char*)"distribution_structure error.\n");
                return RTC_NG;
            }
        } else {
            if (RTC_OK != this->distribution_structure()) {
                LOGOUT(FILTER_ERR, (char*)"distribution_structure error.\n");
                return RTC_NG;
            }
        }
    } else {
        if (RTC_OK != this->distribution_unstructure()) {
            LOGOUT(FILTER_ERR, (char*)"distribution_unstructure error.\n");
            return RTC_NG;
        }
    }

    return RTC_OK;
}
} /* namespace filter */
} /* namespace pbvr */
