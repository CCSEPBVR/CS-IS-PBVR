/*
 * PbvrObjectUnstructuredVolume.cpp
 *
 *  Created on: 2016/12/07
 *      Author: hira
 */

#include "PbvrObjectUnstructuredVolume.h"
#include "UnstructuredVolumeImporter.h"
#include "filter_utils.h"

namespace pbvr {

PbvrObjectUnstructuredVolume::PbvrObjectUnstructuredVolume() {

}

PbvrObjectUnstructuredVolume::PbvrObjectUnstructuredVolume( const PbvrFilterInformationFile *pbvr_fi, int step, int subvolume)
{
    if (pbvr_fi == NULL) return;
    if (pbvr_fi->getPbvrFilter() == NULL) return;
    pbvr::filter::PbvrFilterInfo *info = pbvr_fi->getPbvrFilter()->getPbvrFilterInfo();
    pbvr::filter::FilterParam *param = info->m_param;

    char cell_type[64] = {0x00};
    pbvr::filter::getFilterCellTypeText(pbvr_fi->getElementType(), cell_type);
    this->m_cell_type = std::string(cell_type);
    if (param->component < 0) {
        this->m_veclen = info->m_nkinds;
    } else {
        this->m_veclen = 1;
    }
    this->m_nnodes = pbvr_fi->getNodesSubvolume(subvolume);
    this->m_has_label = false;
    this->m_has_min_value = false;
    this->m_has_max_value = false;
    this->m_ncells  = pbvr_fi->getElemsSubvolume(subvolume);


    // <DataArray>
    this->readValue(pbvr_fi, step, subvolume);
    this->readCoords(pbvr_fi, step, subvolume);
    this->readConnection(pbvr_fi, step, subvolume);


    // this->m_values = new kvs::AnyValueArray(value_nelements);

}


PbvrObjectUnstructuredVolume::~PbvrObjectUnstructuredVolume() {

}

const bool PbvrObjectUnstructuredVolume::readValue(const PbvrFilterInformationFile *pbvr_fi, int step, int subvolume)
{
    pbvr::filter::PbvrFilter *pbvr_filter = pbvr_fi->getPbvrFilter();
    if (pbvr_filter == NULL) return false;

    pbvr_filter->readStepValue(step, subvolume, pbvr_fi->getElementType());

    pbvr::filter::PbvrFilterInfo *info = pbvr_filter->getPbvrFilterInfo();
    if (info == NULL) return false;

    //Int64 n = 0;
	size_t n = 0;
	const size_t value_nelements = this->m_nnodes * this->m_veclen;
    this->m_values.allocate<kvs::Real32>(value_nelements);

    if (info->m_values == NULL) {
        // for debug by @hira
        printf("[rank:%d] PbvrObjectUnstructuredVolume::readValue :: step=%d,subvolume=%d, elem_type=%d, m_values is null\n",
                info->mype, step, subvolume, pbvr_fi->getElementType());
        return false;
    }
    // for debug by @hira
    printf("[rank:%d] PbvrObjectUnstructuredVolume::readValue :: step=%d,subvolume=%d, elem_type=%d, m_values is not null\n",
            info->mype, step, subvolume, pbvr_fi->getElementType());

    for (n=0; n<value_nelements; n++) {
        // modify by @hira at 2017/03/20 : m_pbvr_valuesからm_valueにコピーする. m_valueから取得を行う
        // if (info->m_pbvr_values.size() <= n) break;
        // this->m_values.at<kvs::Real32>(n) = info->m_pbvr_values[n];
        this->m_values.at<kvs::Real32>(n) = info->m_values[n];
    }

    return true;
}


const bool PbvrObjectUnstructuredVolume::readCoords( const PbvrFilterInformationFile *pbvr_fi, int step, int subvolume)
{
    const pbvr::filter::PbvrFilter *pbvr_filter = pbvr_fi->getPbvrFilter();
    if (pbvr_filter == NULL) return false;
    pbvr::filter::PbvrFilterInfo *info = pbvr_filter->getPbvrFilterInfo();
    if (info == NULL) return false;

    const size_t dimension = 3;
    const size_t coord_nelements = this->m_nnodes * dimension;
    this->m_coords.allocate(coord_nelements);
    int rank_nodes = 0, rank_elems = 0;
    for (int volm_loop = 0; volm_loop < pbvr_fi->getNumSubvoume(); volm_loop++) {
        if (subvolume == volm_loop) break;
        if (pbvr_fi->hasSubvolmueId(volm_loop)) {
            rank_nodes += pbvr_fi->getNodesSubvolume(volm_loop);
            rank_elems += pbvr_fi->getElemsSubvolume(volm_loop);
        }
    }

    for (size_t i = 0; i < this->m_nnodes; i++) {
        // modify by @hira at 2017/04/01 m_coordsからm_subvolume_coordからの取得に変更する
        // int nid = pbvr_fi->getSubvolumeNode(rank_nodes + i);
        // int j = nid * 3;
        // this->m_coords[i*3] = info->m_coords[j];
        // this->m_coords[i*3+1] = info->m_coords[j+1];
        // this->m_coords[i*3+2] = info->m_coords[j+2];
        this->m_coords[i*3] = info->m_subvolume_coord[i*3];
        this->m_coords[i*3+1] = info->m_subvolume_coord[i*3+1];
        this->m_coords[i*3+2] = info->m_subvolume_coord[i*3+2];
    }

    return true;
}

const bool PbvrObjectUnstructuredVolume::readConnection(const PbvrFilterInformationFile *pbvr_fi, int step, int subvolume)
{
    const pbvr::filter::PbvrFilter *pbvr_filter = pbvr_fi->getPbvrFilter();
    if (pbvr_filter == NULL) return false;
    if (pbvr_filter->getPbvrFilterInfo() == NULL) return false;
    if (pbvr_filter->getPbvrFilterInfo()->m_param == NULL) return false;
    if (this->m_ncells <= 0) return false;

    Int64 inum, jnum, knum;
    int size;
    Int64 n = 0;
    unsigned int connect[32] = {0x00};
    int rank_nodes = 0, rank_elems = 0;
    unsigned int subvolume_nodes = pbvr_fi->getNodesSubvolume(subvolume);
    int connect_pos = 0;
    int connect_count = pbvr_fi->getConnsSubvolume(subvolume);
    int num_subvolumes = pbvr_fi->getNumSubvoume();
    for (int volm_loop = 0; volm_loop < num_subvolumes; volm_loop++) {
        if (subvolume == volm_loop) break;
        if (pbvr_fi->hasSubvolmueId(volm_loop)) {
            rank_nodes += pbvr_fi->getNodesSubvolume(volm_loop);
            rank_elems += pbvr_fi->getElemsSubvolume(volm_loop);
            connect_pos += pbvr_fi->getConnsSubvolume(volm_loop);
        }
    }

    this->m_connections.allocate(connect_count);

#if 1
    for (n=0; n<connect_count; n++) {
        this->m_connections[n] = pbvr_fi->getConnection(n+connect_pos);
    }
#else

    pbvr::filter::PbvrFilterInfo *info = pbvr_filter->getPbvrFilterInfo();
    const pbvr::filter::FilterParam *param = pbvr_filter->getPbvrFilterInfo()->m_param;

    int subvol_index = 0;
    int offset_value = 0;

    /*** delete by @hira at 2017/0315
    if (param->input_format == pbvr::filter::PLOT3D_INPUT) {
        if (param->n_layer == 0) subvol_index = 0;
        else subvol_index = info->m_nvolumes - 1;
        offset_value = info->m_pbvr_plot3d_offsetvalue[subvol_index];
    }
    *****/

    size = 0;
    int j = 0;
    if (param->struct_grid) {
        inum = param->dim1;
        jnum = param->dim2;
        if (param->ndim == 2) {
            size = 4;
            n = 0;
            for (Int64 i = 0; i < this->m_ncells ; i++) {
                Int64 eid = (Int64) info->m_subvolume_elems[rank_elems + i];
                Int64 nid = info->m_conn_top_node[eid];

                connect[0] = nid;
                connect[1] = connect[0] + 1;
                connect[2] = connect[1] + inum;
                connect[3] = connect[0] + inum;

                int k = 0;
                while (k < size) {
                    /* 節点番号でなく節点の定義順番号 */
                    int org_id = connect[k];
                    int new_id = info->m_nodes_trans[org_id - 1];
                    this->m_connections[n++] = new_id - 1;
                    k++;
                }
            }
        } else {
            size = 8;
            n = 0;
            for (Int64 i = 0; i < this->m_ncells; i++) {
                Int64 eid = (Int64) info->m_subvolume_elems[rank_elems + i];
                Int64 nid = info->m_conn_top_node[eid];

                connect[0] = nid;
                connect[1] = connect[0] + 1;
                connect[2] = connect[1] + inum;
                connect[3] = connect[0] + inum;
                connect[4] = connect[0] + inum*jnum;
                connect[5] = connect[4] + 1;
                connect[6] = connect[5] + inum;
                connect[7] = connect[4] + inum;

                int k = 0;
                while (k < size) {
                    /* 節点番号でなく節点の定義順番号 */
                    int org_id = connect[k];
                    int new_id = info->m_nodes_trans[org_id - 1];
                    this->m_connections[n++] = new_id - 1 + offset_value; // add the offset here
                    k++;
                }
            }

            /*** delete by @hira at 2017/0315
            if (param->input_format == pbvr::filter::PLOT3D_INPUT) {
                info->m_pbvr_plot3d_offsetvalue[subvol_index] += subvolume_nodes;
            }
            ***************/
        }
    }
    else {
        size = info->m_elemcoms;
        n = 0;
        for (Int64 i = 0; i < this->m_ncells; i++) {
            int k = 0;
            j = (Int64) info->m_subvolume_elems[rank_elems + i]*(Int64) size;
            while (k < size) {
                /* 節点番号でなく節点の定義順番号 */
                int org_id = info->m_connections[j + k];
                int new_id = info->m_nodes_trans[org_id - 1];
                this->m_connections[n++] = new_id - 1;
                k++;
            }
        }
    }
#endif

    return true;
}

} /* namespace pbvr */
