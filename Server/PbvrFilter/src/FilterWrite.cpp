#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "filter_log.h"
#include "filter_utils.h"
#include "FilterWrite.h"

namespace pbvr
{
namespace filter
{

/*****************************************************************************/

//static void output_value(FILE*,
//	int,
//	int,
//	Int64,
//	Int64,
//	int);

/*****************************************************************************/


/*
 * サブボリューム単位の出力 |The output of the sub-volume unit
 *
 */
int FilterWrite::write_kvsml_xml(int sub,
    int all,
    Int64 subvol_nelems,
    Int64 subvol_nnodes) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    int state = RTC_NG;
    FILE *ifs = NULL;
    char ofname[FILELEN_MAX];
    char vfname[FILELEN_MAX];

    sprintf(filename, "%s/%s_%07d_%07d.kvsml", param->out_dir, param->out_prefix, sub, all);
    sprintf(ofname, "%s_%07d_%07d", param->out_prefix, sub, all);
    sprintf(vfname, "%s_%05lld_%07d_%07d", param->out_prefix, param->start_step, sub, all);

    if (param->m_iswrite == false) {
        // not write file
        return RTC_OK;
    }

    /* オープン */
    ifs = fopen(filename, "w");
    if (NULL == ifs) {
        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
        goto garbage;
    }

    fprintf(ifs, "<?xml version=\"1.0\" ?>\n");
    fprintf(ifs, "<KVSML>\n");
    fprintf(ifs, "    <Object type=\"UnstructuredVolumeObject\">\n");
    if (info->m_element_type == 4) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"tetrahedra\">\n");
    } else if (info->m_element_type == 11) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"tetrahedra2\">\n");
    } else if (info->m_element_type == 7) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"hexahedra\">\n");
    } else if (info->m_element_type == 14) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"hexahedra2\">\n");
    } else if (info->m_element_type == 5) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Pyramid\">\n");
    } else if (info->m_element_type == 6) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Prism\">\n");
    } else if (info->m_element_type == 2) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Triangle\">\n");
    } else if (info->m_element_type == 3) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Quadratic\">\n");
    } else if (info->m_element_type == 9) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Triangle2\">\n");
    } else if (info->m_element_type == 10) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Quadratic2\">\n");
    // add by @hira at 2016/12/01
    } else if (info->m_element_type == 0) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"point\">\n");
    } else if (info->m_element_type == 1) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"line\">\n");

    } else {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"unknown\">\n");
    }
    fprintf(ifs, "            <Node nnodes=\"%lld\">\n", info->m_nnodes);
    if (param->component < 0) {
        fprintf(ifs, "                <Value veclen=\"%d\">\n", info->m_ncomponents);
    } else {
        fprintf(ifs, "                <Value veclen=\"1\">\n");
    }
    fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_value.dat\" format=\"binary\" />\n", vfname);
    fprintf(ifs, "                </Value>\n");
    fprintf(ifs, "                <Coord>\n");
    fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_coord.dat\" format=\"binary\" />\n", ofname);
    fprintf(ifs, "                </Coord>\n");
    fprintf(ifs, "            </Node>\n");
    fprintf(ifs, "            <Cell ncells=\"%lld\">\n", subvol_nelems);
    fprintf(ifs, "                <Connection>\n");
    fprintf(ifs, "                    <DataArray type=\"uint\"  file=\"%s_connect.dat\" format=\"binary\" />\n", ofname);
    fprintf(ifs, "                </Connection>\n");
    fprintf(ifs, "            </Cell>\n");
    fprintf(ifs, "        </UnstructuredVolumeObject>\n");
    fprintf(ifs, "    </Object>\n");
    fprintf(ifs, "</KVSML>\n");

    state = RTC_OK;
garbage:
    if (NULL != ifs) fclose(ifs);
    return (state);
}

int FilterWrite::write_kvsml_xmls(int step,
    int sub,
    int all,
    Int64 subvol_nelems,
    Int64 subvol_nnodes) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;

    if (param->input_format == PLOT3D_INPUT) {
        return write_kvsml_xmls_multi(step, sub, all, subvol_nelems, subvol_nnodes);
    } else {
        return write_kvsml_xmls_single(step, sub, all, subvol_nelems, subvol_nnodes);
    }

}

int FilterWrite::write_kvsml_xmls_single(int step,
    int sub,
    int all,
    Int64 subvol_nelems,
    Int64 subvol_nnodes) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    int state = RTC_NG;
    FILE *ifs = NULL;
    char ofname[FILELEN_MAX];
    char vfname[FILELEN_MAX];

    sprintf(filename, "%s/%s_%05d_%07d_%07d.kvsml", param->out_dir, param->out_prefix, step, sub, all);
    sprintf(ofname, "%s_%07d_%07d", param->out_prefix, sub, all);
    sprintf(vfname, "%s_%05d_%07d_%07d", param->out_prefix, step, sub, all);
    DEBUG(1, "%s\n ", param->out_prefix);

    if (param->m_iswrite == false) {
        // not write file
        return RTC_OK;
    }

    //    ASSERT_FAIL("F");
    /* オープン */
    ifs = fopen(filename, "w");
    if (NULL == ifs) {
        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
        goto garbage;
    }

    fprintf(ifs, "<?xml version=\"1.0\" ?>\n");
    fprintf(ifs, "<KVSML>\n");
    fprintf(ifs, "    <Object type=\"UnstructuredVolumeObject\">\n");
    if (info->m_element_type == 4) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"tetrahedra\">\n");
    } else if (info->m_element_type == 11) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic tetrahedra\">\n");
    } else if (info->m_element_type == 7) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"hexahedra\">\n");
    } else if (info->m_element_type == 14) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic hexahedra\">\n");
    } else if (info->m_element_type == 5) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"pyramid\">\n");
    } else if (info->m_element_type == 6) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"prism\">\n");
    } else if (info->m_element_type == 2) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"triangle\">\n");
    } else if (info->m_element_type == 3) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic\">\n");
    } else if (info->m_element_type == 9) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"triangle2\">\n");
    } else if (info->m_element_type == 10) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic2\">\n");
    // add by @hira at 2016/12/01
    } else if (info->m_element_type == 0) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"point\">\n");
    } else if (info->m_element_type == 1) {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"line\">\n");

    } else {
        fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"unknown\">\n");
    }
    fprintf(ifs, "            <Node nnodes=\"%lld\">\n", subvol_nnodes);
    if (param->component < 0) {
        fprintf(ifs, "                <Value veclen=\"%d\">\n", info->m_nkinds);
    } else {
        fprintf(ifs, "                <Value veclen=\"1\">\n");
    }
    fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_value.dat\" format=\"binary\" />\n", vfname);
    fprintf(ifs, "                </Value>\n");
    fprintf(ifs, "                <Coord>\n");
    fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_coord.dat\" format=\"binary\" />\n", ofname);
    fprintf(ifs, "                </Coord>\n");
    fprintf(ifs, "            </Node>\n");
    fprintf(ifs, "            <Cell ncells=\"%lld\">\n", subvol_nelems);
    fprintf(ifs, "                <Connection>\n");
    fprintf(ifs, "                    <DataArray type=\"uint\"  file=\"%s_connect.dat\" format=\"binary\" />\n", ofname);
    fprintf(ifs, "                </Connection>\n");
    fprintf(ifs, "            </Cell>\n");
    fprintf(ifs, "        </UnstructuredVolumeObject>\n");
    fprintf(ifs, "    </Object>\n");
    fprintf(ifs, "</KVSML>\n");

    state = RTC_OK;
garbage:
    if (NULL != ifs) fclose(ifs);
    return (state);
}
/*****************************************************************************/

/*
 * サブボリューム単位の出力
 *
 */
int FilterWrite::write_kvsml_xmls_multi(int step,
    int sub,
    int all,
    Int64 subvol_nelems,
    Int64 subvol_nnodes) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    int state = RTC_NG;
    FILE *ifs = NULL;
    char ofname[FILELEN_MAX];
    char vfname[FILELEN_MAX];
    static int allocFlag = 0;
    static Int64 *total_nodes_counter;
    static Int64 *total_elements_counter;

    //printf("SUB: %i CURRENT_BLOCK %i \n",sub, info->info->m_nkinds);
    if (param->input_format == PLOT3D_INPUT && info->current_block == 0) {
        if (allocFlag == 0) {
            total_nodes_counter = (Int64*)malloc(info->m_nvolumes * sizeof (Int64));
            total_elements_counter = (Int64*)malloc(info->m_nvolumes * sizeof (Int64));
            allocFlag = 1;
        }
        total_nodes_counter[sub - 1] = subvol_nnodes;
        total_elements_counter[sub - 1] = subvol_nelems;
    } else {
        total_nodes_counter[sub - 1] += subvol_nnodes;
        total_elements_counter[sub - 1] += subvol_nelems;
    }

    if (info->current_block == info->number_of_blocks - 1) {
        //printf("TotalNumberOfNodes: %ld, %ld\n",total_nodes_counter[sub-1], subvol_nnodes);
        //printf("TotalNumberOfElements: %ld, %ld\n",total_elements_counter[sub-1], subvol_nelems);

        sprintf(filename, "%s/%s_%05d_%07d_%07d.kvsml", param->out_dir, param->out_prefix, step, sub, all);
        sprintf(ofname, "%s_%07d_%07d", param->out_prefix, sub, all);
        sprintf(vfname, "%s_%05d_%07d_%07d", param->out_prefix, step, sub, all);

        if (param->m_iswrite == false) {
            // not write file
            return RTC_OK;
        }

        /* オープン */
        ifs = fopen(filename, "w");
        if (NULL == ifs) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
            goto garbage;
        }

        fprintf(ifs, "<?xml version=\"1.0\" ?>\n");
        fprintf(ifs, "<KVSML>\n");
        fprintf(ifs, "    <Object type=\"UnstructuredVolumeObject\">\n");
        if (info->m_element_type == 4) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"tetrahedra\">\n");
        } else if (info->m_element_type == 11) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic tetrahedra\">\n");
        } else if (info->m_element_type == 7) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"hexahedra\">\n");
        } else if (info->m_element_type == 14) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic hexahedra\">\n");
        } else if (info->m_element_type == 5) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"pyramid\">\n");
        } else if (info->m_element_type == 6) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"prism\">\n");
        } else if (info->m_element_type == 2) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"triangle\">\n");
        } else if (info->m_element_type == 3) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic\">\n");
        } else if (info->m_element_type == 9) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"triangle2\">\n");
        } else if (info->m_element_type == 10) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic2\">\n");
        // add by @hira at 2016/12/01
        } else if (info->m_element_type == 0) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"point\">\n");
        } else if (info->m_element_type == 1) {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"line\">\n");

        } else {
            fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"unknown\">\n");
        }
        fprintf(ifs, "            <Node nnodes=\"%lld\">\n", total_nodes_counter[sub - 1]);
        if (param->component < 0) {
            fprintf(ifs, "                <Value veclen=\"%d\">\n", info->m_nkinds);
        } else {
            fprintf(ifs, "                <Value veclen=\"1\">\n");
        }
        fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_value.dat\" format=\"binary\" />\n", vfname);
        fprintf(ifs, "                </Value>\n");
        fprintf(ifs, "                <Coord>\n");
        fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_coord.dat\" format=\"binary\" />\n", ofname);
        fprintf(ifs, "                </Coord>\n");
        fprintf(ifs, "            </Node>\n");
        fprintf(ifs, "            <Cell ncells=\"%lld\">\n", total_elements_counter[sub - 1]);
        fprintf(ifs, "                <Connection>\n");
        fprintf(ifs, "                    <DataArray type=\"uint\"  file=\"%s_connect.dat\" format=\"binary\" />\n", ofname);
        fprintf(ifs, "                </Connection>\n");
        fprintf(ifs, "            </Cell>\n");
        fprintf(ifs, "        </UnstructuredVolumeObject>\n");
        fprintf(ifs, "    </Object>\n");
        fprintf(ifs, "</KVSML>\n");

        if (sub == info->m_nvolumes) {
            LOGOUT(FILTER_ERR, (char*)"write_kvsml_xmls_multi ONE TIME FREE\n");
            free(total_nodes_counter);
            free(total_elements_counter);
        }
    }
    state = RTC_OK;
garbage:
    if (NULL != ifs) fclose(ifs);
    return (state);
}

typedef enum {
    mode_create, mode_update
} modes;
/*****************************************************************************/

/*
 * サブボリューム単位の出力 |The output of the sub-volume unit
 *
 */
int FilterWrite::write_kvsml_geom(int sub,
    int all,
    int subvol_no,
    Int64 subvol_nelems,
    Int64 volume_nodes,
    Int64 rank_nodes,
    Int64 rank_elems) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    int state = RTC_NG;
    FILE *ifs = NULL;
    Int64 i;
    Int64 j;
    Int64 k;
    float coord[3];
    int *ivalue = NULL;
    int size;
    unsigned int inum;
    unsigned int jnum;
    //  unsigned int  knum;
    unsigned int org_id;
    unsigned int new_id;
    unsigned int nid;
    unsigned int eid;
    unsigned int connect[8];
    modes mode = mode_create;
    int subvol_index = 0;
    int offset_value = 0;
    int count = 0;
    //if (param->input_format == PLOT3D_INPUT && m_nvolumes > 8) {
    if (param->input_format == PLOT3D_INPUT) {
        if (info->current_block > 0) {
            mode = mode_update;
        }
        if (param->n_layer == 0) {
            subvol_index = 0;
        } else {
            subvol_index = subvol_no - 1;
        }
        offset_value = info->offset_values[subvol_index];
        info->m_pbvr_plot3d_offsetvalue.push_back(offset_value);
        //LOGOUT(FILTER_, "OFFSET %i.\n", offset_value);
    //	printf("OFFSET %i.\n", offset_value);
    }

    sprintf(filename, "%s/%s_%07d_%07d_coord.dat", param->out_dir, param->out_prefix, sub, all);
    /* オープン */
    if (param->m_iswrite == true) {
        if (mode == mode_create) {
            ifs = fopen(filename, "wb");
            DEBUG(3, "The current block is 0, creating coordinate file %i\n", subvol_no);
        } else {
            ifs = fopen(filename, "ab");
        }
        if (NULL == ifs) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
            goto garbage;
        }
    }

    // coordinates
    if (volume_nodes > 0) {
        DEBUG(3, "Writing %ld values from block %i to coordinate file: %i \n", volume_nodes, info->current_block, subvol_no);
        info->m_subvolume_coord = (float *) malloc(sizeof (int)*volume_nodes * 3);
        info->m_subvolume_coord_size = volume_nodes * 3;

#ifdef _OPENMP
        {
#pragma omp parallel for default(shared) private(i,nid,j,coord)
#endif
        for (i = 0; i < volume_nodes; i++) {
            nid = info->m_subvolume_nodes[rank_nodes + i];
            j = nid * 3;
            coord[0] = info->m_coords[j ];
            coord[1] = info->m_coords[j + 1];
            coord[2] = info->m_coords[j + 2];
          //if (info->ENDIAN) {
          //    coord[0] = ConvertEndianF(coord[0]);
          //    coord[1] = ConvertEndianF(coord[1]);
          //    coord[2] = ConvertEndianF(coord[2]);
          //}
            info->m_subvolume_coord[i * 3 ] = coord[0];
            info->m_subvolume_coord[i * 3 + 1] = coord[1];
            info->m_subvolume_coord[i * 3 + 2] = coord[2];
        }
#ifdef _OPENMP
        }
#endif

        // delete by @hira at 2017/04/01 m_coordsをfreeする代わりにm_subvolume_coordを生かす。
        // free(info->m_subvolume_coord);
        // info->m_subvolume_coord = NULL;
    }
    if (ifs != NULL) fclose(ifs);

    sprintf(filename, "%s/%s_%07d_%07d_connect.dat", param->out_dir, param->out_prefix, sub, all);

    /* オープン */
    if (param->m_iswrite == true) {
        // if the current clock is 0 create the file, otherwise append to the file
        if (mode == mode_create) {
            ifs = fopen(filename, "wb");
            DEBUG(3, "The current block is 0, creating connection file %i\n", subvol_no);
        } else {
            ifs = fopen(filename, "ab");
        }
        if (NULL == ifs) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
            goto garbage;
        }
    }

    // vector.reserve
    count = 0;
    if (volume_nodes > 0) {
        size = 0;
        if (param->struct_grid) {
            if (param->ndim == 2) {
                size = 4;
                count = subvol_nelems * size;
            } else {
                size = 8;
                count = subvol_nelems * size;
            }
        } else {
            size = info->m_elemcoms;
            count = subvol_nelems * size;
        }
    }
    info->m_pbvr_connections.reserve(count);

    count = 0;
    if (volume_nodes > 0) {
        DEBUG(3,"Writing %ld connections from block %i to connection file: %i \n", volume_nodes * 8, info->current_block, subvol_no);
        ivalue = (int *) malloc(sizeof (int) * 20);
        size = 0;
        j = 0;
        if (param->struct_grid) {
            inum = param->dim1;
            jnum = param->dim2;
            /*knum = param->dim3;*/
            if (param->ndim == 2) {
                size = 4;
                for (i = 0; i < subvol_nelems; i++) {
                    eid = (Int64) info->m_subvolume_elems[rank_elems + i];
                    nid = info->m_conn_top_node[eid];

                    connect[0] = nid;
                    connect[1] = connect[0] + 1;
                    connect[2] = connect[1] + inum;
                    connect[3] = connect[0] + inum;

                    k = 0;
                    while (k < size) {
                        /* 節点番号でなく節点の定義順番号 */
                        org_id = connect[k];
                        new_id = info->m_nodes_trans[org_id - 1];
                        ivalue[k] = new_id - 1;
                      //if (info->ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
                        k++;
                    }
                    for (int n=0;n<size; n++) info->m_pbvr_connections.push_back(ivalue[n]);
                    if (info->ENDIAN) 
                        for (int n=0;n<size; n++) ivalue[n] = ConvertEndianI(ivalue[n]);
                    if (NULL != ifs) fwrite(ivalue, 4, size, ifs);
                    count += size;
                }
            } else {
                size = 8;
                //                printf("NUMBER OF subvol_nelems: %ld %ld \n", subvol_nelems*8, offset_values[subvol_no-1]);
                for (i = 0; i < subvol_nelems; i++) {
                    eid = (Int64) info->m_subvolume_elems[rank_elems + i];
                    nid = info->m_conn_top_node[eid];

                    connect[0] = nid;
                    connect[1] = connect[0] + 1;
                    connect[2] = connect[1] + inum;
                    connect[3] = connect[0] + inum;
                    connect[4] = connect[0] + inum*jnum;
                    connect[5] = connect[4] + 1;
                    connect[6] = connect[5] + inum;
                    connect[7] = connect[4] + inum;

                    k = 0;
                    while (k < size) {
                        /* 節点番号でなく節点の定義順番号 */
                        org_id = connect[k];
                        new_id = info->m_nodes_trans[org_id - 1];
                        if (mode == mode_update) {
                            //ivalue[k] = new_id - 1 + offset_values[subvol_no - 1]; // add the offset here
                            ivalue[k] = new_id - 1 + offset_value; // add the offset here
                        } else {
                            ivalue[k] = new_id - 1;
                        }

                      //if (info->ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
                        k++;
                    }
                    for (int n=0;n<size; n++) info->m_pbvr_connections.push_back(ivalue[n]);
                    if (info->ENDIAN) 
                        for (int n=0;n<size; n++) ivalue[n] = ConvertEndianI(ivalue[n]);
                    if (NULL != ifs) fwrite(ivalue, 4, size, ifs);
                    count += size;
                }
                if (param->input_format == PLOT3D_INPUT) {

                    //offset_values[subvol_no - 1] += volume_nodes;
                    info->offset_values[subvol_index] += volume_nodes;
                    DEBUG(3,"OFFSET UPDATE %ld %ld %d.\n", info->offset_values[subvol_index], volume_nodes, subvol_index);
                }
                else if (param->input_format == PLOT3D_INPUT && subvol_no == 0) {
                    //LOGOUT(FILTER_ERR, (char*)"IS CALLED WITH subvol_no=0  %i.\n", subvol_no);
                }
            }
        } else {
            size = info->m_elemcoms;
            for (i = 0; i < subvol_nelems; i++) {
                k = 0;
                j = (Int64) info->m_subvolume_elems[rank_elems + i]*(Int64) size;
                while (k < size) {
                    /* 節点番号でなく節点の定義順番号 */
                    org_id = info->m_connections[j + k];
                    new_id = info->m_nodes_trans[org_id - 1];
                    ivalue[k] = new_id - 1;
                  //if (info->ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
                    k++;
                }
                for (int n=0;n<size; n++) info->m_pbvr_connections.push_back(ivalue[n]);
                if (info->ENDIAN) 
                    for (int n=0;n<size; n++) ivalue[n] = ConvertEndianI(ivalue[n]);
                if (NULL != ifs) fwrite(ivalue, 4, size, ifs);
                count += size;
            }
        }
        free(ivalue);
        ivalue = NULL;
    }
    info->m_pbvr_connect_count.push_back(count);

    DEBUG(3, "Done Writing %ld connections from block %i to connection file: %i \n", volume_nodes * 8, info->current_block, subvol_no);
    state = RTC_OK;
garbage:
    if (NULL != ifs) fclose(ifs);
    return (state);
}


/*****************************************************************************/

/*
 * サブボリューム単位の出力
 *
 */
int FilterWrite::write_kvsml_value_single(int step_loop,
    int sub,
    int all,
    Int64 volume_nodes,
    Int64 rank_nodes,
    int node_offset /* = 0 */ ) {
    DEBUG(3, "##############write_kvsml_value\n");

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    int state = RTC_NG;
    FILE *ifs = NULL;
    modes mode = mode_create;
    //if (param->input_format == PLOT3D_INPUT && m_nvolumes > 8) {
    if (param->input_format == PLOT3D_INPUT) {
        if (info->current_block > 0) {
            mode = mode_update;
        }
    }
    sprintf(filename, "%s/%s_%05d_%07d_%07d_value.dat", param->out_dir, param->out_prefix, step_loop, sub, all);
    /* オープン */
    if (param->m_iswrite == true) {
        //ifs = fopen(filename, "wb");
        if (mode == mode_create) {
            ifs = fopen(filename, "wb");
            DEBUG(3, "The current block is 0, creating value file %i\n", sub);
        } else {
            ifs = fopen(filename, "ab");
        }
        if (NULL == ifs) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
            goto garbage;
        }
    }

    if (volume_nodes > 0) {
        DEBUG(3, "Writing %ld values from block %i to value file: %i \n", volume_nodes, info->current_block, sub);
        output_value_single(ifs,
                    info->m_nkinds,
                    param->component,
                    volume_nodes,
                    rank_nodes,
                    node_offset);
    }
    state = RTC_OK;
garbage:
    if (NULL != ifs) fclose(ifs);
    return (state);
}
/*****************************************************************************/

/*
 * サブボリューム単位の出力
 *
 */
int FilterWrite::write_kvsml_value_multi(int step_loop,
    int sub,
    int all,
    Int64 volume_nodes,
    Int64 rank_nodes,
    int node_offset /* = 0 */ ) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    char tmp_filename[LINELEN_MAX];
    int state = RTC_NG;
    FILE *ifs_tmp = NULL;
    FILE *ifs = NULL;
    modes mode = mode_create;
    //if (param->input_format == PLOT3D_INPUT && m_nvolumes > 8) {
    if (param->input_format == PLOT3D_INPUT) {
    if (info->current_block > 0) {
        mode = mode_update;
    }
    }
    int k;

    for (k = 0; k < info->m_nkinds; k++) {
        sprintf(tmp_filename, "%s/%s_%05d_%07d_%07d_value.%03d.tmp", param->out_dir, param->out_prefix, step_loop, sub, all, k);

        if (param->m_iswrite == true) {
            /* オープン */
            //ifs = fopen(filename, "wb");
            if (mode == mode_create) {
        //	    printf("Creating file:%s\n", filename);
                ifs_tmp = fopen(tmp_filename, "wb");
            } else {
        //	    printf("Appending to  file:%s\n", tmp_filename);
                ifs_tmp = fopen(tmp_filename, "ab");
            }
            if (NULL == ifs_tmp) {
                LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", tmp_filename);
                goto garbage;
            }
        }

        if (volume_nodes > 0) {
            DEBUG(3, "Writing %ld values from block %i to value file: %i \n", volume_nodes, info->current_block, sub);
            output_value_multi(ifs_tmp,
                    info->m_nkinds,
                param->component,
                volume_nodes,
                rank_nodes, k,
                node_offset);
        }
        if (ifs_tmp != NULL) fclose(ifs_tmp); // <---- Close directly after loop
    }

    if (param->m_iswrite == true) {
        char c;
        if (info->current_block == info->number_of_blocks - 1) {
            printf("\nConcatenating multi grid components...\n");
            sprintf(filename, "%s/%s_%05d_%07d_%07d_value.dat", param->out_dir, param->out_prefix, step_loop, sub, all);
            ifs = fopen(filename, "wb");
            if (NULL == ifs) {
                LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
                goto garbage;
            }
            for (k = 0; k < info->m_nkinds; k++) {
                sprintf(tmp_filename, "%s/%s_%05d_%07d_%07d_value.%03d.tmp", param->out_dir, param->out_prefix, step_loop, sub, all, k);
                ifs_tmp = fopen(tmp_filename, "rb");
                if (NULL == ifs_tmp) {
                    LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", tmp_filename);
                    goto garbage;
                }
                do {
                    c = getc(ifs_tmp);
                    if (!feof(ifs_tmp)) {
                        fputc(c, ifs);
                    }
                }//      while (c != EOF);
                while (!feof(ifs_tmp));
                fclose(ifs_tmp);
            }
            fclose(ifs);
        }
    }

    state = RTC_OK;
garbage:
    //if (NULL != ifs_tmp) fclose(ifs_tmp);
    return (state);
}

int FilterWrite::write_kvsml_value(int step_loop,
    int sub,
    int all,
    Int64 volume_nodes,
    Int64 rank_nodes,
    int node_offset /* = 0 */ ) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    // add by @hira at 2016/12/01
    info->m_pbvr_values.clear();
    std::vector<float>().swap(info->m_pbvr_values);

    int err_status;
    if (param->input_format == PLOT3D_INPUT && info->number_of_blocks >1) {
        err_status = write_kvsml_value_multi(step_loop, sub, all, volume_nodes, rank_nodes, node_offset);
    } else {
        err_status = write_kvsml_value_single(step_loop, sub, all, volume_nodes, rank_nodes, node_offset);
    }
    return err_status;
}
/*****************************************************************************/

/*
 * サブボリューム単位の出力
 *
 */
int FilterWrite::write_value_per_vol(int fnum_loop,
    Int64 volume_nodes,
    Int64 rank_nodes) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    int state = RTC_NG;
    FILE *ifs = NULL;

    sprintf(filename, "%s/%s_%07d_%07lld.dat", param->out_dir, param->out_prefix, fnum_loop + 1, param->out_num);

    if (param->m_iswrite == true) {
        /* オープン */
        ifs = fopen(filename, "ab");
        if (NULL == ifs) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
            goto garbage;
        }
    }

    if (volume_nodes > 0) {
        output_value_single(ifs,
            info->m_nkinds,
            param->component,
            volume_nodes,
            rank_nodes);
    }

    state = RTC_OK;
garbage:
    if (NULL != ifs) fclose(ifs);
    return (state);
}

/*****************************************************************************/

/*
 * サブボリューム単位の出力
 *
 */
int FilterWrite::write_value_tmp1(FILE *tmpfp,
    int fnum_loop,
    Int64 volume_nodes,
    Int64 rank_nodes) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    if (volume_nodes > 0) {
        output_value_single(tmpfp,
            info->m_nkinds,
            param->component,
            volume_nodes,
            rank_nodes);
    }
    return (RTC_OK);
}

/*****************************************************************************/

/*
 * サブボリューム単位の出力
 *
 */
int FilterWrite::write_pfi(NodeInf min, NodeInf max) {
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;

    if (param->input_format == PLOT3D_INPUT) {
        return write_pfi_multi(min, max);
    } else {
        return write_pfi_single(min, max);
    }

    return (RTC_OK);
}

int FilterWrite::write_pfi_single(NodeInf min, NodeInf max)
{

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    int state = RTC_NG;
    FILE *pfi = NULL;
    Int64 i;
    Int64 j;
    int id;
    int itmp;
    float ftmp[6];

    sprintf(filename, "%s/%s.pfi", param->out_dir, param->out_prefix);
    if (param->m_iswrite == true) {
        /* オープン */
        pfi = fopen(filename, "wb");
        if (NULL == pfi) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
            goto garbage;
        }
    }
    itmp = info->m_nnodes;
    if (info->ENDIAN) itmp = ConvertEndianI(itmp);
    if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

    itmp = info->m_nelements;
    if (info->ENDIAN) itmp = ConvertEndianI(itmp);
    if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

    // modify by @hira at 2016/12/01
    if (param->struct_grid) {
        itmp = info->m_element_type;
    }
    else if (param->input_format == VTK_INPUT) {
        itmp = info->m_element_type;
    } else {
        itmp = info->m_elemtypes[0];
    }

    if (info->ENDIAN) itmp = ConvertEndianI(itmp);
    if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

    if (param->file_type == 0) {
        itmp = 0;
    } else {
    if (param->file_type == 1) {
        itmp = 1;
    } else {
        itmp = 2;
    }
    if (info->ENDIAN) itmp = ConvertEndianI(itmp);
    }
    if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

    itmp = param->out_num;
    if (info->ENDIAN) itmp = ConvertEndianI(itmp);
    if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

    itmp = info->m_nkinds;
    if (info->ENDIAN) itmp = ConvertEndianI(itmp);
    if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

    itmp = param->start_step;
    if (info->ENDIAN) itmp = ConvertEndianI(itmp);
    if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

    itmp = param->end_step;
    if (info->ENDIAN) itmp = ConvertEndianI(itmp);
    if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

    itmp = info->m_nvolumes;
    if (info->ENDIAN) itmp = ConvertEndianI(itmp);
    if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

    ftmp[0] = min.x;
    ftmp[1] = min.y;
    ftmp[2] = min.z;
    ftmp[3] = max.x;
    ftmp[4] = max.y;
    ftmp[5] = max.z;
    LOGOUT(FILTER_LOG, (char*)"all min-max\n");
    LOGOUT(FILTER_LOG, (char*)"%20.8f,%20.8f,%20.8f\n", ftmp[0], ftmp[1], ftmp[2]);
    LOGOUT(FILTER_LOG, (char*)"%20.8f,%20.8f,%20.8f\n", ftmp[3], ftmp[4], ftmp[5]);
    if (info->ENDIAN) {
        for (i = 0; i < 6; i++) {
            ftmp[i] = ConvertEndianF(ftmp[i]);
        }
    }
    if (pfi != NULL) fwrite(ftmp, 4, 6, pfi);

    for (i = 0; i < info->m_nvolumes; i++) {
        itmp = info->m_nodes_per_subvolume[i];
        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);
    }

    for (i = 0; i < info->m_nvolumes; i++) {
        itmp = info->m_elems_per_subvolume[i];
        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);
    }

    for (j = 0; j < info->m_nvolumes; j++) {
        id = info->m_tree_node_no - info->m_nvolumes + j;
        ftmp[0] = info->m_tree[id].min.x;
        ftmp[1] = info->m_tree[id].min.y;
        ftmp[2] = info->m_tree[id].min.z;
        ftmp[3] = info->m_tree[id].max.x;
        ftmp[4] = info->m_tree[id].max.y;
        ftmp[5] = info->m_tree[id].max.z;
        LOGOUT(FILTER_LOG, (char*)"min-max %04d:\n", id);
        LOGOUT(FILTER_LOG, (char*)"%20.8f,%20.8f,%20.8f\n", ftmp[0], ftmp[1], ftmp[2]);
        LOGOUT(FILTER_LOG, (char*)"%20.8f,%20.8f,%20.8f\n", ftmp[3], ftmp[4], ftmp[5]);
        if (info->ENDIAN) {
            for (i = 0; i < 6; i++) {
                ftmp[i] = ConvertEndianF(ftmp[i]);
            }
        }
        if (pfi != NULL) fwrite(ftmp, 4, 6, pfi);
    }

    state = RTC_OK;
garbage:
    if (NULL != pfi) fclose(pfi);
    return (state);
}

int FilterWrite::write_pfi_multi(NodeInf min, NodeInf max) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    int state = RTC_NG;
    FILE *pfi = NULL;
    Int64 i;
    Int64 j;
    int id;
    int itmp;
    float ftmp[6];
    static float minMax[6];
    static int *nodesPerSubVolume = NULL;
    static int *elementsPerSubVolume = NULL;
    static int total_nodes_counter;
    static int total_elements_counter;

    typedef struct range {
        float minX;
        float maxX;
        float minY;
        float maxY;
        float minZ;
        float maxZ;
    } range;
    static range *ranges = NULL; // TODO free memory

    sprintf(filename, "%s/%s.pfi", param->out_dir, param->out_prefix);

    if (param->m_iswrite == true) {
        /* オープン */
        pfi = fopen(filename, "wb");
        if (NULL == pfi) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
            goto garbage;
        }
    }

    //LOGOUT(FILTER_ERR, (char*)"write_pfi:  %i.\n", info->m_nnodes);
    if (param->input_format == PLOT3D_INPUT && info->current_block == 0) {
        total_nodes_counter = info->m_nnodes;
        total_elements_counter = info->m_nelements;

        minMax[0] = min.x;
        minMax[1] = min.y;
        minMax[2] = min.z;
        minMax[3] = max.x;
        minMax[4] = max.y;
        minMax[5] = max.z;

        // this branch will be entered only once, so will be initialized only once
        nodesPerSubVolume = (int*)malloc(info->m_nvolumes * sizeof (int));
        elementsPerSubVolume = (int*)malloc(info->m_nvolumes * sizeof (int));
        ranges = (range*)malloc(info->m_nvolumes * sizeof (range));

        for (i = 0; i < info->m_nvolumes; i++) {
            nodesPerSubVolume[i] = info->m_nodes_per_subvolume[i];
            elementsPerSubVolume[i] = info->m_elems_per_subvolume[i];
        }

        for (j = 0; j < info->m_nvolumes; j++) {
            id = info->m_tree_node_no - info->m_nvolumes + j;
            ranges[j].minX = info->m_tree[id].min.x;
            ranges[j].maxX = info->m_tree[id].max.x;
            ranges[j].minY = info->m_tree[id].min.y;
            ranges[j].maxY = info->m_tree[id].max.y;
            ranges[j].minZ = info->m_tree[id].min.z;
            ranges[j].maxZ = info->m_tree[id].max.z;
        }
    } else {
        total_nodes_counter += info->m_nnodes;
        total_elements_counter += info->m_nelements;

        if (min.x < minMax[0]) {
            minMax[0] = min.x;
        }
        if (min.y < minMax[1]) {
            minMax[1] = min.y;
        }
        if (min.z < minMax[2]) {
            minMax[2] = min.z;
        }
        if (max.x > minMax[3]) {
            minMax[3] = max.x;
        }
        if (max.y > minMax[4]) {
            minMax[4] = max.y;
        }
        if (max.z > minMax[5]) {
            minMax[5] = max.z;
        }

        for (i = 0; i < info->m_nvolumes; i++) {
            nodesPerSubVolume[i] += info->m_nodes_per_subvolume[i];
            elementsPerSubVolume[i] += info->m_elems_per_subvolume[i];
        }

        for (j = 0; j < info->m_nvolumes; j++) {
            id = info->m_tree_node_no - info->m_nvolumes + j;
            ranges[j].minX = info->m_tree[id].min.x < ranges[j].minX ? info->m_tree[id].min.x : ranges[j].minX;
            ranges[j].maxX = info->m_tree[id].max.x > ranges[j].maxX ? info->m_tree[id].max.x : ranges[j].maxX;
            ranges[j].minY = info->m_tree[id].min.y < ranges[j].minY ? info->m_tree[id].min.y : ranges[j].minY;
            ranges[j].maxY = info->m_tree[id].max.y > ranges[j].maxY ? info->m_tree[id].max.y : ranges[j].maxY;
            ranges[j].minZ = info->m_tree[id].min.z < ranges[j].minZ ? info->m_tree[id].min.z : ranges[j].minZ;
            ranges[j].maxZ = info->m_tree[id].max.z > ranges[j].maxZ ? info->m_tree[id].max.z : ranges[j].maxZ;
        }
    }


    if (info->current_block == info->number_of_blocks - 1) {
        //itmp = info->m_nnodes;
        itmp = total_nodes_counter;
        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

        //itmp = info->m_nelements;
        itmp = total_elements_counter;
        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

        if (param->struct_grid) {
            itmp = info->m_element_type;
        } else {
            itmp = info->m_elemtypes[0];
        }

        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

        if (param->file_type == 0) {
            itmp = 0;
        } else {
            if (param->file_type == 1) {
            itmp = 1;
            } else {
            itmp = 2;
            }
            if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        }
        if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

        itmp = param->out_num;
        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

        itmp = info->m_nkinds;
        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

        itmp = param->start_step;
        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

        itmp = param->end_step;
        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

        itmp = info->m_nvolumes;
        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);

        //ftmp[0] = min.x;
        //ftmp[1] = min.y;
        //ftmp[2] = min.z;
        //ftmp[3] = max.x;
        //ftmp[4] = max.y;
        //ftmp[5] = max.z;
        LOGOUT(FILTER_LOG, (char*)"all min-max\n");
        LOGOUT(FILTER_LOG, (char*)"%20.8f,%20.8f,%20.8f\n", minMax[0], minMax[1], minMax[2]);
        LOGOUT(FILTER_LOG, (char*)"%20.8f,%20.8f,%20.8f\n", minMax[3], minMax[4], minMax[5]);
        if (info->ENDIAN) {
            for (i = 0; i < 6; i++) {
                minMax[i] = ConvertEndianF(minMax[i]);
            }
        }
        if (pfi != NULL) fwrite(minMax, 4, 6, pfi);

        for (i = 0; i < info->m_nvolumes; i++) {
            //itmp = m_nodes_per_subvolume[i];
            itmp = nodesPerSubVolume[i];
            if (info->ENDIAN) itmp = ConvertEndianI(itmp);
            if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);
        }

        for (i = 0; i < info->m_nvolumes; i++) {
            //itmp = m_elems_per_subvolume[i];
            itmp = elementsPerSubVolume[i];
            if (info->ENDIAN) itmp = ConvertEndianI(itmp);
            if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);
        }
        //  ------------------



        //  ------------------
        for (j = 0; j < info->m_nvolumes; j++) {
            id = info->m_tree_node_no - info->m_nvolumes + j;
            ftmp[0] = ranges[id].minX;
            ftmp[1] = ranges[id].minY;
            ftmp[2] = ranges[id].minZ;
            ftmp[3] = ranges[id].maxX;
            ftmp[4] = ranges[id].maxY;
            ftmp[5] = ranges[id].maxZ;
            LOGOUT(FILTER_LOG, (char*)"min-max %04d:\n", id);
            LOGOUT(FILTER_LOG, (char*)"%20.8f,%20.8f,%20.8f\n", ftmp[0], ftmp[1], ftmp[2]);
            LOGOUT(FILTER_LOG, (char*)"%20.8f,%20.8f,%20.8f\n", ftmp[3], ftmp[4], ftmp[5]);
            if (info->ENDIAN) {
                for (i = 0; i < 6; i++) {
                    ftmp[i] = ConvertEndianF(ftmp[i]);
                }
            }
            if (pfi != NULL) fwrite(ftmp, 4, 6, pfi);
        }
        LOGOUT(FILTER_ERR, (char*)"write_pfi_multi ONE TIME FREE\n");
        free(nodesPerSubVolume);
        free(elementsPerSubVolume);
        free(ranges);
    }
    state = RTC_OK;
garbage:
    if (NULL != pfi) fclose(pfi);
    return (state);
}

/*****************************************************************************/

/*
 * サブボリューム単位の出力
 *
 */
int FilterWrite::write_pfi_value(void) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;

    if (param->input_format == PLOT3D_INPUT) {
        return write_pfi_value_multi();
    } else {
        return write_pfi_value_single();
    }
}

int FilterWrite::write_pfi_value_single(void) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    int state = RTC_NG;
    FILE *pfi = NULL;
    Int64 i;
    Int64 j;
    float ftmp[2];
    int itmp;

    sprintf(filename, "%s/%s.pfi", param->out_dir, param->out_prefix);
    if (param->m_iswrite == true) {
        /* オープン */
        pfi = fopen(filename, "ab");
        if (NULL == pfi) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
            goto garbage;
        }
    }

    for (j = 0; j < info->m_nsteps; j++) {
        LOGOUT(FILTER_LOG, (char*)"step:%i\n", j);
        for (i = 0; i < info->m_nkinds; i++) {
            itmp = info->mt_nullflag[info->m_nkinds * j + i];
            ftmp[0] = info->mt_nulldata[info->m_nkinds * j + i];
            LOGOUT(FILTER_LOG, (char*)"flag :  %19d, data : %20.12f\n", itmp, ftmp[0]);
            if (info->ENDIAN) {
                itmp = ConvertEndianI(itmp);
                ftmp[0] = ConvertEndianF(ftmp[0]);
            }
#ifdef RELEASE_NULLFLAG
            if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);
            if (pfi != NULL) fwrite(&ftmp[0], 4, 1, pfi);
#endif

            ftmp[0] = info->m_value_min[info->m_nkinds * j + i];
            ftmp[1] = info->m_value_max[info->m_nkinds * j + i];
            LOGOUT(FILTER_LOG, (char*)"[%i] maxv : %20.12f, minv : %20.12f\n", info->m_nkinds * j + i, ftmp[0], ftmp[1]);
            if (info->ENDIAN) {
                ftmp[0] = ConvertEndianF(ftmp[0]);
                ftmp[1] = ConvertEndianF(ftmp[1]);
            }
            if (pfi != NULL) fwrite(ftmp, 4, 2, pfi);
        }
    }
    state = RTC_OK;
garbage:
    if (NULL != pfi) fclose(pfi);
    return (state);
}

int FilterWrite::write_pfi_value_multi(void) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    int state = RTC_NG;
    FILE *pfi = NULL;
    Int64 i;
    Int64 j;
    float ftmp[2];
    int itmp;
    static int allocFlag = 0;
    static float *min;
    static float *max;

    if (info->current_block == 0) {
        if (allocFlag == 0) {
            min = (float*)malloc(info->m_nsteps * info->m_nkinds * sizeof (float));
            max = (float*)malloc(info->m_nsteps * info->m_nkinds * sizeof (float));
            allocFlag = 1;
        }

        for (j = 0; j < info->m_nsteps; j++) {
            for (i = 0; i < info->m_nkinds; i++) {
            min[info->m_nkinds * j + i] = info->m_value_min[info->m_nkinds * j + i];
            max[info->m_nkinds * j + i] = info->m_value_max[info->m_nkinds * j + i];
            }
        }
    } else {
        for (j = 0; j < info->m_nsteps; j++) {
            for (i = 0; i < info->m_nkinds; i++) {
                min[info->m_nkinds * j + i] = info->m_value_min[info->m_nkinds * j + i] < min[info->m_nkinds * j + i] ? info->m_value_min[info->m_nkinds * j + i] : min[info->m_nkinds * j + i];
                max[info->m_nkinds * j + i] = info->m_value_max[info->m_nkinds * j + i] > max[info->m_nkinds * j + i] ? info->m_value_max[info->m_nkinds * j + i] : max[info->m_nkinds * j + i];
            }
        }
    }
    // The next if clause will only entered once when processing a multi block plot3d file
    if (info->current_block == info->number_of_blocks - 1) {
        sprintf(filename, "%s/%s.pfi", param->out_dir, param->out_prefix);
        LOGOUT(FILTER_ERR, (char*)"Write_pfi_value %s.\n", filename);
        if (param->m_iswrite == true) {
            /* オープン */
            pfi = fopen(filename, "ab");
            if (NULL == pfi) {
                LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
                goto garbage;
            }
        }

        for (j = 0; j < info->m_nsteps; j++) {
            LOGOUT(FILTER_LOG, (char*)"step:%i\n", j);
            for (i = 0; i < info->m_nkinds; i++) {
                itmp = info->mt_nullflag[info->m_nkinds * j + i];
                ftmp[0] = info->mt_nulldata[info->m_nkinds * j + i];
                LOGOUT(FILTER_LOG, (char*)"flag :  %19d, data : %20.12f\n", itmp, ftmp[0]);
                if (info->ENDIAN) {
                    itmp = ConvertEndianI(itmp);
                    ftmp[0] = ConvertEndianF(ftmp[0]);
                }
#ifdef RELEASE_NULLFLAG
                if (pfi != NULL) fwrite(&itmp, 4, 1, pfi);
                if (pfi != NULL) fwrite(&ftmp[0], 4, 1, pfi);
#endif

                ftmp[0] = min[info->m_nkinds * j + i];
                ftmp[1] = max[info->m_nkinds * j + i];
                LOGOUT(FILTER_LOG, (char*)"[%i] maxv : %20.12f, minv : %20.12f\n", info->m_nkinds * j + i, ftmp[0], ftmp[1]);
                if (info->ENDIAN) {
                    ftmp[0] = ConvertEndianF(ftmp[0]);
                    ftmp[1] = ConvertEndianF(ftmp[1]);
                }
                if (pfi != NULL) fwrite(ftmp, 4, 2, pfi);
            }
        }
        LOGOUT(FILTER_ERR, (char*)"write_pfi_value_multi ONE TIME FREE\n");
        free(min);
        free(max);
    }
    //    ASSERT_FAIL("F");
    state = RTC_OK;
garbage:
    if (NULL != pfi) fclose(pfi);
    return (state);
}
/*****************************************************************************/

/*
 * サブボリューム単位の出力
 *
 */
int FilterWrite::set_geom(int subvol_no,
    Int64 volume_nodes,
    Int64 subvol_nelems,
    Int64 rank_nodes,
    Int64 rank_elems) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    int state = RTC_NG;
    Int64 i;
    Int64 j;
    Int64 k;
    unsigned int iadd;
    unsigned int inum;
    unsigned int jnum;
    //  unsigned int  knum;
    unsigned int nid;
    unsigned int eid;
    unsigned int org_id;
    unsigned int new_id;
    unsigned int connect[8];
    int size;
    int value;
    float coord[3];
    modes mode = mode_create;
    //if (param->input_format == PLOT3D_INPUT && info->m_nvolumes > 8) {
    if (param->input_format == PLOT3D_INPUT) {
        if (info->current_block > 0) {
            mode = mode_update;
        }
    }
    iadd = 0;
    // coordintes
#ifdef _OPENMP
    {
#pragma omp parallel for default(shared) private(i,nid,j,coord)
#endif
    for (i = 0; i < volume_nodes; i++) {
        nid = info->m_subvolume_nodes[rank_nodes + i];
        j = nid * 3;
        coord[0] = info->m_coords[j ];
        coord[1] = info->m_coords[j + 1];
        coord[2] = info->m_coords[j + 2];
      //if (info->ENDIAN) {
      //coord[0] = ConvertEndianF(coord[0]);
      //coord[1] = ConvertEndianF(coord[1]);
      //coord[2] = ConvertEndianF(coord[2]);
      //}
        info->m_subvolume_coord[i * 3 ] = coord[0];
        info->m_subvolume_coord[i * 3 + 1] = coord[1];
        info->m_subvolume_coord[i * 3 + 2] = coord[2];
    }
#ifdef _OPENMP
    }
#endif

    size = 0;
    j = 0;
    iadd = 0;
    if (param->mult_element_type) {
        for (i = 0; i < info->m_nelements; i++) {
            size = info->m_elementsize[(int) info->m_elemtypes[i]];
            if (subvol_no == info->m_subvolume_num[i]) {
                k = 0;
                while (k < size) {
                    /* 節点番号でなく節点の定義順番号 */
                    org_id = info->m_connections[j + k];
                    new_id = info->m_nodes_trans[org_id - 1];
                    value = new_id - 1;
                  //if (info->ENDIAN) value = ConvertEndianI(value);
                    info->m_subvolume_connect[iadd] = value;
                    k++;
                    iadd++;
                }
            }
            j += size;
        }
    } else {
        if (param->struct_grid) {
            inum = param->dim1;
            jnum = param->dim2;
            //          knum = param->dim3;
            if (param->ndim == 2) {
                size = 4;
                for (i = 0; i < subvol_nelems; i++) {
                    eid = (Int64) info->m_subvolume_elems[rank_elems + i];
                    nid = info->m_conn_top_node[eid];

                    connect[0] = nid;
                    connect[1] = connect[0] + 1;
                    connect[2] = connect[1] + inum;
                    connect[3] = connect[0] + inum;

                    k = 0;
                    while (k < size) {
                        /* 節点番号でなく節点の定義順番号 */
                        org_id = connect[k];
                        new_id = info->m_nodes_trans[org_id - 1];
                        value = new_id - 1;
                      //if (info->ENDIAN) value = ConvertEndianI(value);
                        info->m_subvolume_connect[size * i + k] = value;
                        k++;
                    }
                }
            } else {
                size = 8;
                for (i = 0; i < subvol_nelems; i++) {
                    eid = (Int64) info->m_subvolume_elems[rank_elems + i];
                    nid = info->m_conn_top_node[eid];

                    connect[0] = nid;
                    connect[1] = connect[0] + 1;
                    connect[2] = connect[1] + inum;
                    connect[3] = connect[0] + inum;
                    connect[4] = connect[0] + inum*jnum;
                    connect[5] = connect[4] + 1;
                    connect[6] = connect[5] + inum;
                    connect[7] = connect[4] + inum;

                    k = 0;
                    while (k < size) {
                        /* 節点番号でなく節点の定義順番号 */
                        org_id = connect[k];
                        new_id = info->m_nodes_trans[org_id - 1];
                        if (mode == mode_update) {
                            value = new_id - 1 + info->offset_values[subvol_no - 1];
                        } else {
                            value = new_id - 1;
                        }
                      //if (info->ENDIAN) value = ConvertEndianI(value);
                        info->m_subvolume_connect[size * i + k] = value;
                        k++;
                    }
                }
                if (param->input_format == PLOT3D_INPUT && subvol_no > 0) {
                    info->offset_values[subvol_no - 1] += volume_nodes;
                }
            }
        } else {
            size = info->m_elemcoms;
            for (i = 0; i < subvol_nelems; i++) {
                k = 0;
                j = (Int64) info->m_subvolume_elems[rank_elems + i]*(Int64) size;
                while (k < size) {
                    /* 節点番号でなく節点の定義順番号 */
                    org_id = info->m_connections[j + k];
                    new_id = info->m_nodes_trans[org_id - 1];
                    value = new_id - 1;
                  //if (info->ENDIAN) value = ConvertEndianI(value);
                    info->m_subvolume_connect[size * i + k] = value;
                    k++;
                }
            }
        }
    }

    state = RTC_OK;
    return (state);
}

/*****************************************************************************/

/*
 * サブボリューム単位のセット
 *
 */
int FilterWrite::set_value(Int64 volume_nodes,
    Int64 rank_nodes) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    int state = RTC_NG;
    Int64 i;
    Int64 j;
    Int64 k;
    int component_id;
    int sta;
    int end;
    //  unsigned int  iadd;
    unsigned int nid;
    //  float         *value = NULL;

    // value
    component_id = param->component;
    sta = 0;
    end = info->m_nkinds - 1;
    if (component_id>-1) {
    j = 0;
    for (i = 0; i < info->m_ncomponents; i++) {
        if (i == component_id) {
        sta = j;
        end = sta + info->m_veclens[i] - 1;
        break;
        }
        j += info->m_veclens[i];
    }
    } else {
    sta = 0;
    end = info->m_nkinds - 1;
    }

#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,nid,j,k)
#endif
    for (i = 0; i < volume_nodes; i++) {
        nid = info->m_subvolume_nodes[rank_nodes + i];
        j = nid*info->m_nkinds;
        for (k = sta; k <= end; k++) {
            info->m_subvolume_value[volume_nodes * k + i] = info->m_values[j + k];
        }
    }
    if (info->ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,nid,j,k)
#endif
        for (i = 0; i < volume_nodes; i++) {
            nid = info->m_subvolume_nodes[rank_nodes + i];
            j = nid*info->m_nkinds;
            for (k = sta; k <= end; k++) {
                info->m_subvolume_value[volume_nodes * k + i] = ConvertEndianF(info->m_subvolume_value[volume_nodes * k + i]);
            }
        }
    }
    state = RTC_OK;
    return (state);
}

void FilterWrite::output_value_multi(FILE *ifs,
    int nkind,
    int component_id,
    Int64 volume_nodes,
    Int64 rank_nodes, int k,
    int node_offset /* = 0 */ ) {

    if (this->m_filter_info == NULL) return;
    if (this->m_filter_info->m_param == NULL) return;
    PbvrFilterInfo *info = this->m_filter_info;

    Int64 i;
    Int64 j;
    //Int64 k;
    int sta;
    int end;
    unsigned int nid;
    float *value;

    //fprintf(stdout  , "Number of kinds  %i\n" ,nkind );

    value = NULL;
    sta = 0;
    end = nkind - 1;
    if (component_id>-1) {
    j = 0;
    for (i = 0; i < info->m_ncomponents; i++) {
        if (i == component_id) {
            sta = j;
            end = sta + info->m_veclens[i] - 1;
            break;
        }
        j += info->m_veclens[i];
    }
    }
    if (component_id>-1) {
        value = (float *) malloc(sizeof (float) * nkind);
        for (i = 0; i < volume_nodes; i++) {
            nid = info->m_subvolume_nodes[rank_nodes + i];
            // add by @hira at 2017/04/01
            nid = nid - node_offset;
            j = nid*nkind;
            for (k = sta; k <= end; k++) {
                value[k - sta] = info->m_values[j + k];
                if (ifs != NULL) if (info->ENDIAN) value[k - sta] = ConvertEndianF(value[k - sta]);
            }
            if (ifs != NULL) fwrite(value, 4, end - sta + 1, ifs);
            // add by @hira at 2016/12/01
            for (k = sta; k <= end; k++) info->m_pbvr_values.push_back(value[k - sta]);
        }
    } else {
        value = (float *) malloc(sizeof (float) * volume_nodes);
        // close the file handle which was passed to this function
        //fclose(ifs);
        //for (k = 0; k < nkind; k++) {
        // create a filename like: k_currentblock
        //sprintf(filename, "%i", k);
        //fprintf(stdout  , "%s\n" , filename);
        //FILE *ifs1 = NULL;
        //if(info->current_block == 0)
        //{
        //  ifs1 = fopen(filename,"wb"); // open the new file
        //}
        //else{
        //  ifs1 = fopen(filename,"ab"); // open the new file
        //}

        for (i = 0; i < volume_nodes; i++) {
            nid = info->m_subvolume_nodes[rank_nodes + i];
            // add by @hira at 2017/04/01
            nid = nid - node_offset;
            j = nid*nkind;
            value[i] = info->m_values[j + k];
            //fprintf(stdout,"%f\n",value[i]);
        }
        if (ifs != NULL) {
            if (info->ENDIAN) {
                for (i = 0; i < volume_nodes; i++) {
                    value[i] = ConvertEndianF(value[i]);
                }
            }
        }
        if (ifs != NULL) fwrite(value, 4, volume_nodes, ifs);
        // add by @hira at 2016/12/01
        for (i = 0; i < volume_nodes; i++) info->m_pbvr_values.push_back(value[i]);

        //fclose(ifs); // close the file
        //}
    }

    free(value);
    value = NULL;
}

void FilterWrite::output_value_single(FILE *ifs,
            int nkind,
            int component_id,
            Int64 volume_nodes,
            Int64 rank_nodes,
            int node_offset /* = 0 */ ) {

    if (this->m_filter_info == NULL) return;
    if (this->m_filter_info->m_param == NULL) return;
    PbvrFilterInfo *info = this->m_filter_info;

    Int64 i;
    Int64 j;
    Int64 k;
    int sta;
    int end;
    unsigned int nid;
    float *value;

    value = NULL;
    sta = 0;
    end = nkind - 1;
    if (component_id>-1) {
        j = 0;
        for (i = 0; i < info->m_ncomponents; i++) {
            if (i == component_id) {
            sta = j;
            end = sta + info->m_veclens[i] - 1;
            break;
            }
            j += info->m_veclens[i];
        }
    }
    if (component_id>-1) {
        value = (float *) malloc(sizeof (float) * nkind);
        for (i = 0; i < volume_nodes; i++) {
            nid = info->m_subvolume_nodes[rank_nodes + i];
            // add by @hira at 2017/04/01
            nid = nid - node_offset;
            j = nid*nkind;
            for (k = sta; k <= end; k++) {
                value[k - sta] = info->m_values[j + k];
                if (ifs != NULL) if (info->ENDIAN) value[k - sta] = ConvertEndianF(value[k - sta]);
            }
            if (ifs != NULL) fwrite(value, 4, end - sta + 1, ifs);
            // add by @hira at 2016/12/01
            for (k = sta; k <= end; k++) info->m_pbvr_values.push_back(value[k - sta]);
        }
    } else {
        value = (float *) malloc(sizeof (float) * volume_nodes);
        for (k = 0; k < nkind; k++) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,nid,j)
#endif
            for (i = 0; i < volume_nodes; i++) {
                nid = info->m_subvolume_nodes[rank_nodes + i];
                // add by @hira at 2017/04/01
                nid = nid - node_offset;
                j = nid*nkind;
                value[i] = info->m_values[j + k];
            }
            if (ifs != NULL) {
                if (info->ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                    for (i = 0; i < volume_nodes; i++) {
                        value[i] = ConvertEndianF(value[i]);
                    }
                }
            }
            if (ifs != NULL) fwrite(value, 4, volume_nodes, ifs);
            // add by @hira at 2016/12/01
            for (i = 0; i < volume_nodes; i++) info->m_pbvr_values.push_back(value[i]);
        }
    }

    free(value);
    value = NULL;
}

int FilterWrite::write_nodes_split() {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    int volm_loop;
    int subvolume_id;
    Int64 rank_nodes;
    Int64 rank_elems;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;

    rank_nodes = 0;
    rank_elems = 0;

    for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0		// delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
        if (info->numpe_s > 1) {
            if (info->mype_f != volm_loop % info->numpe_f) {
                continue;
            }
        } else {
            if (info->mype_v != volm_loop % info->numpe_v) {
                continue;
            }
        }
#endif
#endif
        // add by @hira at 2016/12/01
        if (!this->m_filter_division->isWriteMpiProcess(volm_loop)) {
            continue;
        }

        subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
        /* サブボリューム内の要素・節点の判別 */
        subvolume_elems = info->m_elems_per_subvolume[volm_loop];
        subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
        subvolume_conns = info->m_conns_per_subvolume[volm_loop];
        if (RTC_OK != set_nodes_in_subvolume(subvolume_nodes,
                            rank_nodes, info)) {
            LOGOUT(FILTER_ERR, (char*)"set_node_in_subvolume error.\n");
            goto garbage;
        }

        /* サブボリューム単位の出力 */
        if (RTC_OK != this->write_kvsml_geom(volm_loop + 1,
                                info->m_nvolumes,
                                subvolume_id,
                                subvolume_elems,
                                subvolume_nodes,
                                rank_nodes,
                                rank_elems)) {
            LOGOUT(FILTER_ERR, (char*)"write_geom error.\n");
            goto garbage;
        }
        rank_nodes += info->m_nodes_per_subvolume[volm_loop];
        rank_elems += info->m_elems_per_subvolume[volm_loop];

        // ランク担当のサブボリュームIDを格納する。
        info->m_pbvr_subvolumeids.push_back(volm_loop);
    }

    return RTC_OK;

garbage:
    return RTC_NG;
}

int FilterWrite::write_nodes_subvolume() {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    char outfile[FILELEN_MAX];
    int volm_loop;
    int fnum_loop;
    int subvolume_id;
    int count;
    int itmp;
    Int64 rank_nodes;
    Int64 rank_elems;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;
    FILE *fp = NULL;

    rank_nodes = 0;
    rank_elems = 0;
    /* ファイル数のループ */
    for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {
#ifdef MPI_EFFECT
        if (info->numpe_s > 1) {
            if (info->mype_f != fnum_loop % info->numpe_f) {
                continue;
            }
        } else {
            if (info->mype_v != fnum_loop % info->numpe_v) {
                continue;
            }
        }
#endif
// add 2014.11
//          sprintf(outfile, "%s/%s.%07d.%07d.vol",param->out_dir,param->out_prefix,fnum_loop+1,param->out_num);
        if (info->mpidiv_s > 1) {
            sprintf(outfile, "%s/%s_%07d_%07lld.tmp0", param->out_dir, param->out_prefix, fnum_loop + 1, param->out_num);
        } else {
            sprintf(outfile, "%s/%s_%07d_%07lld.dat", param->out_dir, param->out_prefix, fnum_loop + 1, param->out_num);
        }

        if (param->m_iswrite == true) {
            // add 2014.11
            fp = fopen(outfile, "wb");
            if (NULL == fp) {
                LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", outfile);
                goto garbage;
            }
        }
        itmp = info->m_vol_sta[fnum_loop];
        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (fp != NULL) fwrite(&itmp, 4, 1, fp);
        itmp = info->m_vol_end[fnum_loop];
        if (info->ENDIAN) itmp = ConvertEndianI(itmp);
        if (fp != NULL) fwrite(&itmp, 4, 1, fp);
        /* サブボリュームのループ */
        for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
            subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
            subvolume_elems = info->m_elems_per_subvolume[volm_loop];
            subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
            subvolume_conns = info->m_conns_per_subvolume[volm_loop];
            /* サブボリューム内の要素・節点の判別 */
            if (RTC_OK != set_nodes_in_subvolume(
                            subvolume_nodes,
                            rank_nodes,
                            info)) {
                LOGOUT(FILTER_ERR, (char*)"set_node_in_subvolume error.\n");
                goto garbage;
            }
            if (subvolume_nodes > 0) {
                info->m_subvolume_coord = (float *) malloc(sizeof (int)*subvolume_nodes * 3);
                info->m_subvolume_connect = (int *) malloc(sizeof (int)*subvolume_conns);
                info->m_subvolume_coord_size = subvolume_nodes * 3;
                info->m_subvolume_connect_size = subvolume_conns;

                if (NULL == info->m_coords) {
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_connect).\n");
                    goto garbage;
                }
                /* サブボリューム単位のセット */
                if (RTC_OK != this->set_geom(subvolume_id,
                        subvolume_nodes,
                        subvolume_elems,
                        rank_nodes,
                        rank_elems)) {
                    LOGOUT(FILTER_ERR, (char*)"set_geom error.\n");
                    goto garbage;
                }
                count = info->m_conns_per_subvolume[volm_loop];
                for (int n=0;n<count; n++) info->m_pbvr_connections.push_back(info->m_subvolume_connect[n]);
                if (info->ENDIAN) 
                    for (int n=0;n<count; n++) info->m_subvolume_connect[n] = ConvertEndianI(info->m_subvolume_connect[n]);
                if (fp != NULL) fwrite(info->m_subvolume_connect, 4, count, fp);
                info->m_pbvr_connect_count.push_back(count);

                count = info->m_nodes_per_subvolume[volm_loop]*3;
                if (info->ENDIAN) 
                    for (int n=0;n<count; n++) info->m_subvolume_coord[n] = ConvertEndianI(info->m_subvolume_coord[n]);
                if (fp != NULL) fwrite(info->m_subvolume_coord, 4, count, fp);
                free(info->m_subvolume_coord);
                free(info->m_subvolume_connect);
                info->m_subvolume_coord = NULL;
                info->m_subvolume_connect = NULL;
            }
            rank_nodes += info->m_nodes_per_subvolume[volm_loop];
            rank_elems += info->m_elems_per_subvolume[volm_loop];
        } /* サブボリュームのループ */
        if (fp != NULL) fclose(fp);
    } /* ファイル数のループ */

    return RTC_OK;

garbage:
    return RTC_NG;
}


int FilterWrite::write_nodes_step(
#ifdef MPI_EFFECT
        MPI_Comm mpi_comm
#endif
        ) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    char outfile[FILELEN_MAX];

#ifdef MPI_EFFECT
    MPI_Offset offset;
    MPI_File mpifp1, mpifp2;
    MPI_Status mpistat;
    //  int         *mpiwork;
#else
    FILE *fp1 = NULL;
    FILE *fp2 = NULL;
#endif
    int volm_loop;
    int subvolume_id;
    int count;
    Int64 rank_nodes;
    Int64 rank_elems;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;

    if (param->m_iswrite == true) {

#ifdef MPI_EFFECT
        sprintf(outfile, "%s/%s_coord.dat", param->out_dir, param->out_prefix);
        MPI_File_open(mpi_comm, outfile, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &mpifp1);
        sprintf(outfile, "%s/%s_connect.dat", param->out_dir, param->out_prefix);
        MPI_File_open(mpi_comm, outfile, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &mpifp2);
#else
        sprintf(outfile, "%s/%s_coord.dat", param->out_dir, param->out_prefix);
        fp1 = fopen(outfile, "wb");
        if (NULL == fp1) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", outfile);
            goto garbage;
        }
        sprintf(outfile, "%s/%s_connect.dat", param->out_dir, param->out_prefix);
        fp2 = fopen(outfile, "wb");
        if (NULL == fp1) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", outfile);
            goto garbage;
        }
#endif
    }

    rank_nodes = 0;
    rank_elems = 0;

    for (volm_loop = 0; volm_loop < info->m_nvolumes; volm_loop++) {
#if 0		// delete by @hira at 2016/12/01
#ifdef MPI_EFFECT
        if (info->numpe_s > 1) {
            if (info->mype_f != volm_loop % info->numpe_f) {
                continue;
            }
        } else {
            if (info->mype_v != volm_loop % info->numpe_v) {
                continue;
            }
        }
#endif
#endif
        // add by @hira at 2016/12/01
        if (!this->m_filter_division->isWriteMpiProcess(volm_loop)) {
            continue;
        }

        subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;

        subvolume_elems = info->m_elems_per_subvolume[volm_loop];
        subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
        subvolume_conns = info->m_conns_per_subvolume[volm_loop];
        /* サブボリューム内の要素・節点の判別 */
        if (RTC_OK != set_nodes_in_subvolume(subvolume_nodes,
                rank_nodes,
                info)) {
            LOGOUT(FILTER_ERR, (char*)"set_node_in_subvolume error.\n");
            goto garbage;
        }
        count = 0;
        if (subvolume_nodes > 0) {
            info->m_subvolume_coord = (float *) malloc(sizeof (int)*subvolume_nodes * 3);
            info->m_subvolume_connect = (int *) malloc(sizeof (int)*subvolume_conns);
            info->m_subvolume_coord_size = subvolume_nodes * 3;
            info->m_subvolume_connect_size = subvolume_conns;

            if (NULL == info->m_subvolume_connect || NULL == info->m_subvolume_coord) {
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_connect).\n");
                goto garbage;
            }
            /* サブボリューム単位のセット */
            if (RTC_OK != this->set_geom(subvolume_id,
                    subvolume_nodes,
                    subvolume_elems,
                    rank_nodes,
                    rank_elems)) {
                LOGOUT(FILTER_ERR, (char*)"set_geom error.\n");
                goto garbage;
            }
            count = info->m_nodes_per_subvolume[volm_loop]*3;
            if (info->ENDIAN) 
                for (int n=0;n<count; n++) info->m_subvolume_coord[n] = ConvertEndianI(info->m_subvolume_coord[n]);
#ifdef MPI_EFFECT
            offset = (MPI_Offset) info->m_nodes_accumulate[volm_loop]*3 * sizeof (float);
            if (mpifp1 != NULL) MPI_File_write_at(mpifp1, offset, info->m_subvolume_coord, count, MPI_FLOAT, &mpistat);
#else
            if (fp1 != NULL) fwrite(info->m_subvolume_coord, 4, count, fp1);
#endif
            count = info->m_conns_per_subvolume[volm_loop];

            for (int n=0;n<count; n++) info->m_pbvr_connections.push_back(info->m_subvolume_connect[n]);
            if (info->ENDIAN) 
                for (int n=0;n<count; n++) info->m_subvolume_connect[n] = ConvertEndianI(info->m_subvolume_connect[n]);
#ifdef MPI_EFFECT
            offset = (MPI_Offset) info->m_conns_accumulate[volm_loop] * sizeof (int);
            if (mpifp2 != NULL) MPI_File_write_at(mpifp2, offset, info->m_subvolume_connect, count, MPI_INT, &mpistat);
#else
            if (fp2 != NULL) fwrite(info->m_subvolume_connect, 4, count, fp2);
#endif

            free(info->m_subvolume_coord);
            free(info->m_subvolume_connect);
            info->m_subvolume_coord = NULL;
            info->m_subvolume_connect = NULL;
        }
        info->m_pbvr_connect_count.push_back(count);

        rank_nodes += info->m_nodes_per_subvolume[volm_loop];
        rank_elems += info->m_elems_per_subvolume[volm_loop];
    }

#ifdef MPI_EFFECT
    if (mpifp1 != NULL) MPI_File_sync(mpifp1);
    if (mpifp2 != NULL) MPI_File_sync(mpifp2);
    if (mpifp1 != NULL) MPI_File_close(&mpifp1);
    if (mpifp2 != NULL) MPI_File_close(&mpifp2);
#else
    if (fp1 != NULL) fclose(fp1);
    if (fp2 != NULL) fclose(fp2);
#endif

    return RTC_OK;

garbage:
    return RTC_NG;
}


int FilterWrite::write_values_split(int step_loop) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    char outfile[FILELEN_MAX];
    int volm_loop;
    int fnum_loop;
    int subvolume_id;
    int count;
    int itmp;
    Int64 rank_nodes;
    Int64 rank_elems;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;

    rank_nodes = info->getSubVolumeNodeCount();
    info->m_pbvr_values.reserve(rank_nodes);

    /* サブボリュームのループ  | KVSML format file */
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
        // delete by @hira at 2016/12/01 : ランク０がすべてm_valueに出力する
        if (!this->m_filter_division->isWriteMpiProcess(volm_loop)) {
            continue;
         }

        subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
        subvolume_elems = info->m_elems_per_subvolume[volm_loop];
        subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
        subvolume_conns = info->m_conns_per_subvolume[volm_loop];
        LOGOUT(FILTER_LOG, (char*)" ||| volm_loop    : %d\n", volm_loop);
        LOGOUT(FILTER_LOG, (char*)" ||| subvolume_id : %d\n", subvolume_id);
        LOGOUT(FILTER_LOG, (char*)" |||  subvolume_elems : %d\n", subvolume_elems);
        LOGOUT(FILTER_LOG, (char*)" |||  subvolume_nodes : %d\n", subvolume_nodes);
        LOGOUT(FILTER_LOG, (char*)" |||  subvolume_conns : %d\n", subvolume_conns);
        /* サブボリューム単位の出力    The output of the sub-volume unit */
        /* XML形式ファイルの出力      The output of the XML format file */
        if (RTC_OK != this->write_kvsml_xmls(step_loop,
                        volm_loop + 1,
                        info->m_nvolumes,
                        subvolume_elems,
                        subvolume_nodes)) {
            LOGOUT(FILTER_ERR, (char*)"write_kvsml_xmls error.\n");
            goto garbage;
        }
        // add by @hira at 2017/04/01
        int node_offset = info->m_subvolume_nodes[0];
        /* VARIABLESの出力 */
        if (RTC_OK != this->write_kvsml_value(step_loop,
                        volm_loop + 1,
                        info->m_nvolumes,
                        subvolume_nodes,
                        rank_nodes,
                        node_offset)) {
            LOGOUT(FILTER_ERR, (char*)"write_kvsml_value error.\n");
            goto garbage;
        }
        rank_nodes += info->m_nodes_per_subvolume[volm_loop];
    } /* サブボリュームのループ */

    return RTC_OK;

garbage:

    return RTC_NG;
}

int FilterWrite::write_valuetmp1_subvolume(int step_loop) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    char outfile[FILELEN_MAX];
    int volm_loop;
    int fnum_loop;
    int subvolume_id;
    Int64 rank_nodes;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;
    FILE *fp = NULL;

    /* ファイル数のループ */
    rank_nodes = 0;
    for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {
#ifdef MPI_EFFECT
        if (info->mype_v != fnum_loop % info->numpe_v) {
            continue;
        }
#endif
        // 2014.11
        if (info->mpidiv_s > 1) {
            sprintf(outfile, "%s/%s_%07d_%07lld-%05d.tmp1", param->out_dir, param->out_prefix, fnum_loop + 1, param->out_num, step_loop);
            if (param->m_iswrite == true) {
                fp = fopen(outfile, "wb");
            }
            /* サブボリュームのループ */
            for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
                subvolume_elems = info->m_elems_per_subvolume[volm_loop];
                subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
                subvolume_conns = info->m_conns_per_subvolume[volm_loop];
                /* サブボリューム単位の出力 */
                if (RTC_OK != this->write_value_tmp1(fp,
                        fnum_loop,
                        subvolume_nodes,
                        rank_nodes)) {
                    LOGOUT(FILTER_ERR, (char*)"write_value_per_vol error.\n");
                    goto garbage;
                }
                rank_nodes += info->m_nodes_per_subvolume[volm_loop];
            } /* サブボリュームのループ */
            if (fp != NULL) fclose(fp);
        } else {
            /* サブボリュームのループ */
            for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
                subvolume_elems = info->m_elems_per_subvolume[volm_loop];
                subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
                subvolume_conns = info->m_conns_per_subvolume[volm_loop];
                /* サブボリューム単位の出力 */
                if (RTC_OK != this->write_value_per_vol(fnum_loop,
                        subvolume_nodes,
                        rank_nodes)) {
                    LOGOUT(FILTER_ERR, (char*)"write_value_per_vol error.\n");
                    goto garbage;
                }
                rank_nodes += info->m_nodes_per_subvolume[volm_loop];
            } /* サブボリュームのループ */
        }
        //              fclose(fp);
    } /* ファイル数のループ */

    return RTC_OK;

garbage:
    return RTC_NG;
}


int FilterWrite::write_values_subvolume() {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    char outfile[FILELEN_MAX];
    char infile[FILELEN_MAX];
    int step_loop;
    int volm_loop;
    int fnum_loop;
    FILE *fp = NULL;
    FILE *ifp = NULL;
    Int64 subvolume_nodes;
    float *value;

    if (param->m_iswrite == false) return RTC_OK;

    /* ファイル数のループ */
    for (step_loop = param->start_step; step_loop <= param->end_step; step_loop++) {
        for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {
            if (info->mype != fnum_loop % info->numpe) {
                continue;
            }
            sprintf(outfile, "%s/%s_%07d_%07lld.tmp0", param->out_dir, param->out_prefix, fnum_loop + 1, param->out_num);
            sprintf(infile, "%s/%s_%07d_%07lld-%05d.tmp1", param->out_dir, param->out_prefix, fnum_loop + 1, param->out_num, step_loop);
            fp = fopen(outfile, "ab");
            if (fp == NULL) {
                LOGOUT(FILTER_ERR, (char*)"file open error. %s\n", outfile);
                goto garbage;
            }
            ifp = fopen(infile, "rb");
            if (ifp == NULL) {
                LOGOUT(FILTER_ERR, (char*)"file open error. %s\n", infile);
                goto garbage;
            }
            /* サブボリュームのループ */
            for (volm_loop = info->m_vol_sta[fnum_loop]; volm_loop <= info->m_vol_end[fnum_loop]; volm_loop++) {
                subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
                value = (float *) malloc(sizeof (float) * subvolume_nodes * info->m_nkinds);
                fread(value, sizeof (float), subvolume_nodes*info->m_nkinds, ifp);
                fwrite(value, sizeof (float), subvolume_nodes*info->m_nkinds, fp);
                free(value);
                value = NULL;
            } /* サブボリュームのループ */
            fclose(ifp);
            fclose(fp);
            /* **.tmp1ファイル削除 */
            remove(infile);
        }
    }
    for (fnum_loop = 0; fnum_loop < param->out_num; fnum_loop++) {
        if (info->mype != fnum_loop % info->numpe) {
            continue;
        }
        sprintf(outfile, "%s/%s_%07d_%07lld.tmp0", param->out_dir, param->out_prefix, fnum_loop + 1, param->out_num);
        sprintf(infile, "%s/%s_%07d_%07lld.dat", param->out_dir, param->out_prefix, fnum_loop + 1, param->out_num);
        /* tmp0ファイルをvolファイルに */
        rename(outfile, infile);
    }

    return RTC_OK;

garbage:
    return RTC_NG;
}

int FilterWrite::write_values_step(
                    int step_loop
#ifdef MPI_EFFECT
                    ,MPI_Comm  mpi_comm
#endif
                                    ) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    char outfile[FILELEN_MAX];
#ifdef MPI_EFFECT
    MPI_Offset offset;
    MPI_File mpifp1;
    MPI_Status mpistat;
    //  int         *mpiwork;
#else
    FILE *fp1 = NULL;
#endif
    int volm_loop;
    int subvolume_id;
    int count;
    Int64 rank_nodes;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;

#ifdef MPI_EFFECT
    if (mpi_comm == MPI_COMM_NULL) return RTC_NG;
#endif

    if (param->m_iswrite == true) {
#ifdef MPI_EFFECT
        sprintf(outfile, "%s/%s_%05d_value.dat", param->out_dir, param->out_prefix, step_loop);
        MPI_File_open(mpi_comm, outfile, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &mpifp1);
#else
        sprintf(outfile, "%s/%s_%05d_value.dat", param->out_dir, param->out_prefix, step_loop);
        fp1 = fopen(outfile, "wb");
        if (NULL == fp1) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", outfile);
            goto garbage;
        }
#endif
    }

    /* サブボリュームのループ */
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
        if (!this->m_filter_division->isWriteMpiProcess(volm_loop)) {
            continue;
        }

        subvolume_id = info->m_tree_node_no - info->m_nvolumes + volm_loop;
        subvolume_elems = info->m_elems_per_subvolume[volm_loop];
        subvolume_nodes = info->m_nodes_per_subvolume[volm_loop];
        subvolume_conns = info->m_conns_per_subvolume[volm_loop];
        info->m_subvolume_value = (float *) malloc(sizeof (int)*subvolume_nodes * info->m_nkinds);
        /* サブボリューム単位のセット */
        if (RTC_OK != this->set_value(subvolume_nodes,
                rank_nodes)) {
            LOGOUT(FILTER_ERR, (char*)"set_value error.\n");
            goto garbage;
        }
        count = info->m_nodes_per_subvolume[volm_loop] * info->m_nkinds;
#ifdef MPI_EFFECT
        offset = (MPI_Offset) info->m_nodes_accumulate[volm_loop] * info->m_nkinds * sizeof (float);
        if (mpifp1 != NULL)  MPI_File_write_at(mpifp1, offset, info->m_subvolume_value, count, MPI_FLOAT, &mpistat);
#else
        if (fp1 != NULL) fwrite(info->m_subvolume_value, 4, count, fp1);
#endif
        // add by @hira at 2016/12/01
        for (int n = 0; n< count; n++) info->m_pbvr_values.push_back(info->m_subvolume_value[n]);

        free(info->m_subvolume_value);
        info->m_subvolume_value = NULL;
        rank_nodes += info->m_nodes_per_subvolume[volm_loop];
    } /* サブボリュームのループ */

#ifdef MPI_EFFECT
    if (mpifp1 != NULL) MPI_File_sync(mpifp1);
    if (mpifp1 != NULL) MPI_File_close(&mpifp1);
#else
    if (fp1 != NULL) fclose(fp1);
#endif

    return RTC_OK;

garbage:
    return RTC_NG;
}

int FilterWrite::remove_tmpfile(int m_elementtypeno,
            const char *m_out_prefix,
            int step_loop)
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    char buff[LINELEN_MAX];
    char filename[LINELEN_MAX];

    if (!param->struct_grid && info->m_multi_element && param->temp_delete) {
        // sprintf(param->in_dir, "%s", param->tmp_dir);
        sprintf(buff, param->format, step_loop);
        sprintf(filename, "%s/%02d-%s%s%s", param->tmp_dir, m_elementtypeno, m_out_prefix, buff, param->in_suffix);
        remove(filename);
    }

    return RTC_OK;
}

void FilterWrite::setFilterDivision(FilterDivision *division)
{
    this->m_filter_division = division;
}


void FilterWrite::copyNodeValue(float* dest, float* src, int size, int subvolume)
{

    if (this->m_filter_info == NULL) return;
    if (this->m_filter_info->m_param == NULL) return;
    PbvrFilterInfo *info = this->m_filter_info;

    // データコピー先
    int rank = 0;
    int count = 0;
    Int64 pos = info->getNodePosition(subvolume);
    memcpy(dest+pos, src, sizeof(float)*size);

    return;
}

} // end of namespace filter
} // end of namespace pbvr
