/*
 * FilterDivisionOctree.cpp
 *
 *  Created on: 2016/12/24
 *      Author: hira
 */

#include "FilterDivisionOctree.h"
#include "filter_log.h"

namespace pbvr {
namespace filter {

/**
 * コンストラクタ
 */
FilterDivisionOctree::FilterDivisionOctree() : FilterDivision() {

}

/**
 * コンストラクタ
 * @param filter_info		フィルタ情報
 */
FilterDivisionOctree::FilterDivisionOctree(PbvrFilterInfo *filter_info)
    : FilterDivision(filter_info) {

}

/**
 * デストラクタ
 */
FilterDivisionOctree::~FilterDivisionOctree() {

}

/*****************************************************************************/
/*
 * 要素を木構造内に振り分け
 *
 */
#ifdef _OPENMP
#define _OPENMP_SW_XXXX
#endif

int FilterDivisionOctree::distribution_structure() {
    int state = RTC_NG;
    int n_layer;
    float xx;
    float yy;
    float zz;
    unsigned int inum;
    unsigned int jnum;
    unsigned int knum;
    Int64 ii, jj, kk;
    unsigned int j;
	int k;
    Int64 nid;
    Int64 eid;
    unsigned int cnt;
    unsigned int vol_num;
    unsigned int shift;
    unsigned int size;
    if (this->m_filter_info == NULL) return RTC_NG;

    FilterParam* param = this->m_filter_info->m_param;
    HierCell *m_tree = this->m_filter_info->m_tree;
    float *m_coords = this->m_filter_info->m_coords;
    int *m_subvolume_num = this->m_filter_info->m_subvolume_num;
    unsigned int *m_elems_per_subvolume = this->m_filter_info->m_elems_per_subvolume;
    int *m_conn_top_node = this->m_filter_info->m_conn_top_node;

    if (this->m_filter_info->m_octree != NULL) free(this->m_filter_info->m_octree);
    this->m_filter_info->m_octree = (int *) malloc(sizeof (int)*this->m_filter_info->m_nnodes);
    int *m_octree = this->m_filter_info->m_octree;

    n_layer = param->n_layer;
    inum = param->dim1;
    jnum = param->dim2;
    knum = param->dim3;
    shift = this->m_filter_info->m_tree_node_no - this->m_filter_info->m_nvolumes;

    /* 格子点を振り分け */
#ifdef _OPENMP_SW
#pragma omp parallel default(shared)
    {
#pragma omp for private(ii,jj,kk,nid,,eid,j,k,xx,yy,zz,vol_num)
#endif
    for (kk = 0; kk < knum - 1; kk++) {
        for (jj = 0; jj < jnum - 1; jj++) {
            for (ii = 0; ii < inum - 1; ii++) {
                nid = ii + (inum) * jj + (inum)*(jnum) * kk + 1;
                eid = ii + (inum - 1) * jj + (inum - 1)*(jnum - 1) * kk;
                nid = nid - 1;
                m_octree[nid] = 0;

                for (k = 0; k < n_layer; k++) {
                    j = m_octree[nid];
                    xx = (m_tree[j].max.x + m_tree[j].min.x) / 2.0;
                    yy = (m_tree[j].max.y + m_tree[j].min.y) / 2.0;
                    zz = (m_tree[j].max.z + m_tree[j].min.z) / 2.0;

                    if (m_coords[nid * 3] > xx) {
                        if (m_coords[nid * 3 + 1] > yy) {
                            if (m_coords[nid * 3 + 2] > zz) {
                                m_octree[nid] = m_tree[j].child[6];
                            } else {
                                m_octree[nid] = m_tree[j].child[3];
                            }
                        } else {
                            if (m_coords[nid * 3 + 2] > zz) {
                                m_octree[nid] = m_tree[j].child[7];
                            } else {
                                m_octree[nid] = m_tree[j].child[2];
                            }
                        }
                    } else {
                        if (m_coords[nid * 3 + 1] > yy) {
                            if (m_coords[nid * 3 + 2] > zz) {
                                m_octree[nid] = m_tree[j].child[5];
                            } else {
                                m_octree[nid] = m_tree[j].child[4];
                            }
                        } else {
                            if (m_coords[nid * 3 + 2] > zz) {
                                m_octree[nid] = m_tree[j].child[8];
                            } else {
                                m_octree[nid] = m_tree[j].child[1];
                            }
                        }
                    }
                }
                vol_num = m_octree[nid];
                m_subvolume_num[eid] = vol_num;
#ifdef _OPENMP_SW
#pragma omp critical
    {
#endif
                    this->m_filter_info->m_elems_per_subvolume[vol_num - shift]++;
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
                m_octree[nid] = 0;

                for (k = 0; k < n_layer; k++) {
                    j = m_octree[nid];
                    xx = (m_tree[j].max.x + m_tree[j].min.x) / 2.0;
                    yy = (m_tree[j].max.y + m_tree[j].min.y) / 2.0;
                    zz = (m_tree[j].max.z + m_tree[j].min.z) / 2.0;

                    if (m_coords[nid * 3] > xx) {
                        if (m_coords[nid * 3 + 1] > yy) {
                            if (m_coords[nid * 3 + 2] > zz) {
                                m_octree[nid] = m_tree[j].child[6];
                            } else {
                                m_octree[nid] = m_tree[j].child[3];
                            }
                        } else {
                            if (m_coords[nid * 3 + 2] > zz) {
                                m_octree[nid] = m_tree[j].child[7];
                            } else {
                                m_octree[nid] = m_tree[j].child[2];
                            }
                        }
                    } else {
                        if (m_coords[nid * 3 + 1] > yy) {
                            if (m_coords[nid * 3 + 2] > zz) {
                                m_octree[nid] = m_tree[j].child[5];
                            } else {
                                m_octree[nid] = m_tree[j].child[4];
                            }
                        } else {
                            if (m_coords[nid * 3 + 2] > zz) {
                                m_octree[nid] = m_tree[j].child[8];
                            } else {
                                m_octree[nid] = m_tree[j].child[1];
                            }
                        }
                    }
                }
                vol_num = m_octree[nid];
                cnt = m_elems_per_subvolume[vol_num - shift];
                this->m_filter_info->m_element_array[vol_num - shift][cnt] = eid;
                cnt++;
                m_elems_per_subvolume[vol_num - shift] = cnt;
            }
        }
    }

    state = RTC_OK;
    return (state);
}
#ifdef _OPENMP_SW
#undef _OPENMP_SW
#endif

/*****************************************************************************/
/*
 * 要素を木構造内に振り分け
 *
 */
#ifdef _OPENMP
#define _OPENMP_SW_XXXX
#endif

int FilterDivisionOctree::distribution_structure_2d() {
    int state = RTC_NG;
    int n_layer;
    float xx;
    float yy;
    float zz;
    unsigned int inum;
    unsigned int jnum;
    //    unsigned int knum;
    Int64 ii, jj;
    unsigned int j;
	int k;
    Int64 nid;
    Int64 eid;
    unsigned int cnt;
    unsigned int vol_num;
    unsigned int shift;
    unsigned int size;

    if (this->m_filter_info == NULL) return RTC_NG;
    FilterParam* param = this->m_filter_info->m_param;
    HierCell *m_tree = this->m_filter_info->m_tree;
    float *m_coords = this->m_filter_info->m_coords;
    int *m_subvolume_num = this->m_filter_info->m_subvolume_num;
    unsigned int *m_elems_per_subvolume = this->m_filter_info->m_elems_per_subvolume;
    int *m_conn_top_node = this->m_filter_info->m_conn_top_node;

    if (this->m_filter_info->m_octree != NULL) free(this->m_filter_info->m_octree);
    this->m_filter_info->m_octree = (int *) malloc(sizeof (int)*this->m_filter_info->m_nnodes);
    int *m_octree = this->m_filter_info->m_octree;

    n_layer = param->n_layer;
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
            m_octree[nid] = 0;

            for (k = 0; k < n_layer; k++) {
                j = m_octree[nid];
                xx = (m_tree[j].max.x + m_tree[j].min.x) / 2.0;
                yy = (m_tree[j].max.y + m_tree[j].min.y) / 2.0;
                zz = (m_tree[j].max.z + m_tree[j].min.z) / 2.0;

                if (m_coords[nid * 3] > xx) {
                    if (m_coords[nid * 3 + 1] > yy) {
                        if (m_coords[nid * 3 + 2] > zz) {
                            m_octree[nid] = m_tree[j].child[6];
                        } else {
                            m_octree[nid] = m_tree[j].child[3];
                        }
                    } else {
                        if (m_coords[nid * 3 + 2] > zz) {
                            m_octree[nid] = m_tree[j].child[7];
                        } else {
                            m_octree[nid] = m_tree[j].child[2];
                        }
                    }
                } else {
                    if (m_coords[nid * 3 + 1] > yy) {
                        if (m_coords[nid * 3 + 2] > zz) {
                            m_octree[nid] = m_tree[j].child[5];
                        } else {
                            m_octree[nid] = m_tree[j].child[4];
                        }
                    } else {
                        if (m_coords[nid * 3 + 2] > zz) {
                            m_octree[nid] = m_tree[j].child[8];
                        } else {
                            m_octree[nid] = m_tree[j].child[1];
                        }
                    }
                }
            }
            vol_num = m_octree[nid];
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
            m_octree[nid] = 0;

            for (k = 0; k < n_layer; k++) {
                j = m_octree[nid];
                xx = (m_tree[j].max.x + m_tree[j].min.x) / 2.0;
                yy = (m_tree[j].max.y + m_tree[j].min.y) / 2.0;
                zz = (m_tree[j].max.z + m_tree[j].min.z) / 2.0;

                if (m_coords[nid * 3] > xx) {
                    if (m_coords[nid * 3 + 1] > yy) {
                        if (m_coords[nid * 3 + 2] > zz) {
                            m_octree[nid] = m_tree[j].child[6];
                        } else {
                            m_octree[nid] = m_tree[j].child[3];
                        }
                    } else {
                        if (m_coords[nid * 3 + 2] > zz) {
                            m_octree[nid] = m_tree[j].child[7];
                        } else {
                            m_octree[nid] = m_tree[j].child[2];
                        }
                    }
                } else {
                    if (m_coords[nid * 3 + 1] > yy) {
                        if (m_coords[nid * 3 + 2] > zz) {
                            m_octree[nid] = m_tree[j].child[5];
                        } else {
                            m_octree[nid] = m_tree[j].child[4];
                        }
                    } else {
                        if (m_coords[nid * 3 + 2] > zz) {
                            m_octree[nid] = m_tree[j].child[8];
                        } else {
                            m_octree[nid] = m_tree[j].child[1];
                        }
                    }
                }
            }
            vol_num = m_octree[nid];
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

/*****************************************************************************/

/*
 * 要素を木構造内に振り分け
 *
 */
int FilterDivisionOctree::distribution_unstructure() {
    int state = RTC_NG;
    char rcode = 0;
    //    char flag0;
    int n_layer;
    float xx;
    float yy;
    float zz;
    Int64 ii;
    unsigned int id, cnt;
    unsigned int j;
	int k;
    Int64 i;
    unsigned int addr;
    unsigned int vol_num;
    unsigned int shift;
    unsigned int size;
    int type;

    if (this->m_filter_info == NULL) return RTC_NG;
    FilterParam* param = this->m_filter_info->m_param;
    HierCell *m_tree = this->m_filter_info->m_tree;
    float *m_coords = this->m_filter_info->m_coords;
    int *m_subvolume_num = this->m_filter_info->m_subvolume_num;
    unsigned int *m_elems_per_subvolume = this->m_filter_info->m_elems_per_subvolume;

    if (this->m_filter_info->m_octree != NULL) free(this->m_filter_info->m_octree);
    this->m_filter_info->m_octree = (int *) malloc(sizeof (int)*this->m_filter_info->m_nnodes);
    int *m_octree = this->m_filter_info->m_octree;

    char *m_elemtypes = this->m_filter_info->m_elemtypes;
    const int *m_elementsize = this->m_filter_info->m_elementsize;
    int *m_connections = this->m_filter_info->m_connections;

    n_layer = param->n_layer;
    shift = this->m_filter_info->m_tree_node_no - this->m_filter_info->m_nvolumes;
    //    flag0 = param->nodesearch;

    /* 要素を振り分け */
    if (param->mult_element_type) {
        DEBUG(3,"#########Entered multi element branch \n");
        addr = 0;
        size = 0;
        DEBUG(4,"looping over %ld elements \n", this->m_filter_info->m_nelements);
        for (ii = 0; ii < this->m_filter_info->m_nelements; ii++) {
            DEBUG(4,"ii : %ld\n", ii);
            type = (int) m_elemtypes[ii];
            size = m_elementsize[type]; /// <------------- THIS GOES HORRIBLY WRONG
            id = m_connections[addr]; // <-------------- m_connections looks empty, why?

            i = id - 1;
            DEBUG(3,"element %ld: Addr:%d Type:%d,  Size:%d, id:%d i:%ld \n", ii, addr, type, size, id, i);
            addr += size;
            m_octree[i] = 0;
            DEBUG(7,"D:\n");
            for (k = 0; k < n_layer; k++) {
                j = m_octree[i];
                xx = (m_tree[j].max.x + m_tree[j].min.x) / 2.0;
                yy = (m_tree[j].max.y + m_tree[j].min.y) / 2.0;
                zz = (m_tree[j].max.z + m_tree[j].min.z) / 2.0;
                if (m_coords[i * 3] > xx) {
                    if (m_coords[i * 3 + 1] > yy) {
                        if (m_coords[i * 3 + 2] > zz) {
                            m_octree[i] = m_tree[j].child[6];
                        } else {
                            m_octree[i] = m_tree[j].child[3];
                        }
                    } else {
                        if (m_coords[i * 3 + 2] > zz) {
                            m_octree[i] = m_tree[j].child[7];
                        } else {
                            m_octree[i] = m_tree[j].child[2];
                        }
                    }
                } else {
                    if (m_coords[i * 3 + 1] > yy) {
                        if (m_coords[i * 3 + 2] > zz) {
                            m_octree[i] = m_tree[j].child[5];
                        } else {
                            m_octree[i] = m_tree[j].child[4];
                        }
                    } else {
                        if (m_coords[i * 3 + 2] > zz) {
                            m_octree[i] = m_tree[j].child[8];
                        } else {
                            m_octree[i] = m_tree[j].child[1];
                        }
                    }
                }
                //		DEBUG(7,"end of for k\n");
            }

            vol_num = m_octree[i];
            m_subvolume_num[ii] = vol_num;
            m_elems_per_subvolume[vol_num - (this->m_filter_info->m_tree_node_no - this->m_filter_info->m_nvolumes)]++;
            //	    DEBUG(7,"end of for ii\n");
        }
    } else {
        DEBUG(3,"########### entered single element type branch \n");
        for (ii = 0; ii < this->m_filter_info->m_nelements; ii++) {
            id = m_connections[ii * (Int64) this->m_filter_info->m_elemcoms];
            i = id - 1;
            DEBUG(3,"i=%ld, ii=%ld, m_elemcoms=%d \n", i, ii, this->m_filter_info->m_elemcoms);
            if (m_octree[i] == 0) {
                DEBUG(3,"m_octree[i] == 0 branch \n");
                for (k = 0; k < n_layer; k++) {
                    j = m_octree[i];
                    xx = (m_tree[j].max.x + m_tree[j].min.x) / 2.0;
                    yy = (m_tree[j].max.y + m_tree[j].min.y) / 2.0;
                    zz = (m_tree[j].max.z + m_tree[j].min.z) / 2.0;
                    if (m_coords[i * 3] > xx) {
                        if (m_coords[i * 3 + 1] > yy) {
                            if (m_coords[i * 3 + 2] > zz) {
                                m_octree[i] = m_tree[j].child[6];
                            } else {
                                m_octree[i] = m_tree[j].child[3];
                            }
                        } else {
                            if (m_coords[i * 3 + 2] > zz) {
                                m_octree[i] = m_tree[j].child[7];
                            } else {
                                m_octree[i] = m_tree[j].child[2];
                            }
                        }
                    } else {
                        if (m_coords[i * 3 + 1] > yy) {
                            if (m_coords[i * 3 + 2] > zz) {
                                m_octree[i] = m_tree[j].child[5];
                            } else {
                                m_octree[i] = m_tree[j].child[4];
                            }
                        } else {
                            if (m_coords[i * 3 + 2] > zz) {
                                m_octree[i] = m_tree[j].child[8];
                            } else {
                                m_octree[i] = m_tree[j].child[1];
                            }
                        }
                    }
                }
            }


            vol_num = m_octree[i];
            m_subvolume_num[ii] = vol_num;
            m_elems_per_subvolume[vol_num - (this->m_filter_info->m_tree_node_no - this->m_filter_info->m_nvolumes)]++;
        }
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

    if (param->mult_element_type) {
        addr = 0;
        size = 0;
        for (ii = 0; ii < this->m_filter_info->m_nelements; ii++) {
            size = m_elementsize[(int) m_elemtypes[ii]];
            id = m_connections[addr];
            addr += size;
            i = id - 1;
            m_octree[i] = 0;

            for (k = 0; k < n_layer; k++) {
                j = m_octree[i];
                xx = (m_tree[j].max.x + m_tree[j].min.x) / 2.0;
                yy = (m_tree[j].max.y + m_tree[j].min.y) / 2.0;
                zz = (m_tree[j].max.z + m_tree[j].min.z) / 2.0;
                if (m_coords[i * 3] > xx) {
                    if (m_coords[i * 3 + 1] > yy) {
                        if (m_coords[i * 3 + 2] > zz) {
                            m_octree[i] = m_tree[j].child[6];
                        } else {
                            m_octree[i] = m_tree[j].child[3];
                        }
                    } else {
                        if (m_coords[i * 3 + 2] > zz) {
                            m_octree[i] = m_tree[j].child[7];
                        } else {
                            m_octree[i] = m_tree[j].child[2];
                        }
                    }
                } else {
                    if (m_coords[i * 3 + 1] > yy) {
                        if (m_coords[i * 3 + 2] > zz) {
                            m_octree[i] = m_tree[j].child[5];
                        } else {
                            m_octree[i] = m_tree[j].child[4];
                        }
                    } else {
                        if (m_coords[i * 3 + 2] > zz) {
                            m_octree[i] = m_tree[j].child[8];
                        } else {
                            m_octree[i] = m_tree[j].child[1];
                        }
                    }
                }
            }
            vol_num = m_octree[i];
            cnt = m_elems_per_subvolume[vol_num - shift];
            this->m_filter_info->m_element_array[vol_num - shift][cnt] = ii;
            cnt++;
            m_elems_per_subvolume[vol_num - shift] = cnt;
        }
    } else {
        for (ii = 0; ii < this->m_filter_info->m_nelements; ii++) {
            id = m_connections[ii * (Int64) this->m_filter_info->m_elemcoms];
            i = id - 1;
            if (m_octree[i] == 0) {
                for (k = 0; k < n_layer; k++) {
                    j = m_octree[i];
                    xx = (m_tree[j].max.x + m_tree[j].min.x) / 2.0;
                    yy = (m_tree[j].max.y + m_tree[j].min.y) / 2.0;
                    zz = (m_tree[j].max.z + m_tree[j].min.z) / 2.0;
                    if (m_coords[i * 3] > xx) {
                        if (m_coords[i * 3 + 1] > yy) {
                            if (m_coords[i * 3 + 2] > zz) {
                                m_octree[i] = m_tree[j].child[6];
                            } else {
                                m_octree[i] = m_tree[j].child[3];
                            }
                        } else {
                            if (m_coords[i * 3 + 2] > zz) {
                                m_octree[i] = m_tree[j].child[7];
                            } else {
                                m_octree[i] = m_tree[j].child[2];
                            }
                        }
                    } else {
                        if (m_coords[i * 3 + 1] > yy) {
                            if (m_coords[i * 3 + 2] > zz) {
                                m_octree[i] = m_tree[j].child[5];
                            } else {
                                m_octree[i] = m_tree[j].child[4];
                            }
                        } else {
                            if (m_coords[i * 3 + 2] > zz) {
                                m_octree[i] = m_tree[j].child[8];
                            } else {
                                m_octree[i] = m_tree[j].child[1];
                            }
                        }
                    }
                }
            }
            vol_num = m_octree[i];
            cnt = m_elems_per_subvolume[vol_num - shift];
            this->m_filter_info->m_element_array[vol_num - shift][cnt] = ii;
            cnt++;
            m_elems_per_subvolume[vol_num - shift] = cnt;
        }
    }

    state = RTC_OK;
    if (rcode) {
        state = RTC_NG;
    } else {
        state = RTC_OK;
    }
    return (state);
}

int FilterDivisionOctree::get_nvolumes()
{
    if (this->m_filter_info == NULL) return 0;
    if (this->m_filter_info->m_param == NULL) return 0;
    FilterParam* param = this->m_filter_info->m_param;

    int ii;
    if (!(param->explicit_n_layer) && param->elem_ave > 0) {
        /* 階層数決定 */
        ii = 1;
        for (int i = 0; i < NLAYER_MAX + 1; i++) {
            if (this->m_filter_info->m_nelements / ii <= param->elem_ave) {
                param->n_layer = i;
                break;
            }
            ii = ii * 8;
        }
    }
    /* サブボリューム数決定 */
    this->m_filter_info->m_nvolumes = 1;
    for (int i = 0; i < param->n_layer; i++) {
        this->m_filter_info->m_nvolumes = this->m_filter_info->m_nvolumes * 8;
    }

    return this->m_filter_info->m_nvolumes;
}


int FilterDivisionOctree::create_tree_struct(NodeInf node_min, NodeInf node_max)
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam* param = this->m_filter_info->m_param;

    int ret = this->create_tree_struct(
                    param->n_layer,
                    node_min,
                    node_max,
                    &info->m_tree_node_no,
                    &info->m_tree);
    return ret;
}

/*
 * 木構造の生成
 *
 */
int FilterDivisionOctree::create_tree_struct(
                Int64       n_layer      ,
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
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(tree).\n");
        goto garbage;
    }

    stack_val = (int *)malloc(sizeof(int)*(*tree_node_no)*8+1);
    if (NULL == stack_val)
    {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(stack_val).\n");
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

} /* namespace filter */
} /* namespace pbvr */
