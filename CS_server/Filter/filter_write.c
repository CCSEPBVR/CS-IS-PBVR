#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "filter.h"
#include "filter_write.h"
#include "filter_log.h"

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
int write_kvsml_xml(int sub,
	int all,
	Int64 subvol_nelems,
	Int64 subvol_nnodes) {
    int state = RTC_NG;
    FILE *ifs = NULL;
    char ofname[FILELEN_MAX];
    char vfname[FILELEN_MAX];

    sprintf(filename, "%s/%s_%07d_%07d.kvsml", Fparam.out_dir, Fparam.out_prefix, sub, all);
    sprintf(ofname, "%s_%07d_%07d", Fparam.out_prefix, sub, all);
    //sprintf(vfname, "%s_%05ld_%07d_%07d", Fparam.out_prefix, Fparam.start_step, sub, all);
	sprintf(vfname, "%s_%05lld_%07d_%07d", Fparam.out_prefix, Fparam.start_step, sub, all);

    /* オープン */
    ifs = fopen(filename, "w");
    if (NULL == ifs) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }

    fprintf(ifs, "<?xml version=\"1.0\" ?>\n");
    fprintf(ifs, "<KVSML>\n");
    fprintf(ifs, "    <Object type=\"UnstructuredVolumeObject\">\n");
    if (m_element_type == 4) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"tetrahedra\">\n");
    } else if (m_element_type == 11) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"tetrahedra2\">\n");
    } else if (m_element_type == 7) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"hexahedra\">\n");
    } else if (m_element_type == 14) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"hexahedra2\">\n");
    } else if (m_element_type == 5) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Pyramid\">\n");
    } else if (m_element_type == 6) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Prism\">\n");
    } else if (m_element_type == 2) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Triangle\">\n");
    } else if (m_element_type == 3) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Quadratic\">\n");
    } else if (m_element_type == 9) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Triangle2\">\n");
    } else if (m_element_type == 10) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"Quadratic2\">\n");
    } else {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"unknown\">\n");
    }
    //fprintf(ifs, "            <Node nnodes=\"%ld\">\n", m_nnodes);
	fprintf(ifs, "            <Node nnodes=\"%lld\">\n", m_nnodes);
	if (Fparam.component < 0) {
	fprintf(ifs, "                <Value veclen=\"%d\">\n", m_ncomponents);
    } else {
	fprintf(ifs, "                <Value veclen=\"1\">\n");
    }
    fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_value.dat\" format=\"binary\" />\n", vfname);
    fprintf(ifs, "                </Value>\n");
    fprintf(ifs, "                <Coord>\n");
    fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_coord.dat\" format=\"binary\" />\n", ofname);
    fprintf(ifs, "                </Coord>\n");
    fprintf(ifs, "            </Node>\n");
    //fprintf(ifs, "            <Cell ncells=\"%ld\">\n", subvol_nelems);
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

int write_kvsml_xmls(int step,
	int sub,
	int all,
	Int64 subvol_nelems,
	Int64 subvol_nnodes) {


    if (Fparam.input_format == PLOT3D_INPUT) {
	return write_kvsml_xmls_multi(step, sub, all, subvol_nelems, subvol_nnodes);
    } else {
	return write_kvsml_xmls_single(step, sub, all, subvol_nelems, subvol_nnodes);
    }

}

int write_kvsml_xmls_single(int step,
	int sub,
	int all,
	Int64 subvol_nelems,
	Int64 subvol_nnodes) {
    int state = RTC_NG;
    FILE *ifs = NULL;
    char ofname[FILELEN_MAX];
    char vfname[FILELEN_MAX];

    sprintf(filename, "%s/%s_%05d_%07d_%07d.kvsml", Fparam.out_dir, Fparam.out_prefix, step, sub, all);
    sprintf(ofname, "%s_%07d_%07d", Fparam.out_prefix, sub, all);
    sprintf(vfname, "%s_%05d_%07d_%07d", Fparam.out_prefix, step, sub, all);
    DEBUG(1, "%s\n ", Fparam.out_prefix);
    //    ASSERT_FAIL("F");
    /* オープン */
    ifs = fopen(filename, "w");
    if (NULL == ifs) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }

    fprintf(ifs, "<?xml version=\"1.0\" ?>\n");
    fprintf(ifs, "<KVSML>\n");
    fprintf(ifs, "    <Object type=\"UnstructuredVolumeObject\">\n");
    if (m_element_type == 4) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"tetrahedra\">\n");
    } else if (m_element_type == 11) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic tetrahedra\">\n");
    } else if (m_element_type == 7) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"hexahedra\">\n");
    } else if (m_element_type == 14) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic hexahedra\">\n");
    } else if (m_element_type == 5) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"pyramid\">\n");
    } else if (m_element_type == 6) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"prism\">\n");
    } else if (m_element_type == 2) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"triangle\">\n");
    } else if (m_element_type == 3) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic\">\n");
    } else if (m_element_type == 9) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"triangle2\">\n");
    } else if (m_element_type == 10) {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic2\">\n");
    } else {
	fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"unknown\">\n");
    }
    //fprintf(ifs, "            <Node nnodes=\"%ld\">\n", subvol_nnodes);
	fprintf(ifs, "            <Node nnodes=\"%lld\">\n", subvol_nnodes);
	if (Fparam.component < 0) {
	fprintf(ifs, "                <Value veclen=\"%d\">\n", m_nkinds);
    } else {
	fprintf(ifs, "                <Value veclen=\"1\">\n");
    }
    fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_value.dat\" format=\"binary\" />\n", vfname);
    fprintf(ifs, "                </Value>\n");
    fprintf(ifs, "                <Coord>\n");
    fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_coord.dat\" format=\"binary\" />\n", ofname);
    fprintf(ifs, "                </Coord>\n");
    fprintf(ifs, "            </Node>\n");
    //fprintf(ifs, "            <Cell ncells=\"%ld\">\n", subvol_nelems);
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
int write_kvsml_xmls_multi(int step,
	int sub,
	int all,
	Int64 subvol_nelems,
	Int64 subvol_nnodes) {
    int state = RTC_NG;
    FILE *ifs = NULL;
    char ofname[FILELEN_MAX];
    char vfname[FILELEN_MAX];
    static int allocFlag = 0;
    static Int64 *total_nodes_counter;
    static Int64 *total_elements_counter;

    //printf("SUB: %i CURRENT_BLOCK %i \n",sub, current_block);
    if (Fparam.input_format == PLOT3D_INPUT && current_block == 0) {
	if (allocFlag == 0) {
	    total_nodes_counter = malloc(m_nvolumes * sizeof (Int64));
	    total_elements_counter = malloc(m_nvolumes * sizeof (Int64));
	    allocFlag = 1;
	}

	total_nodes_counter[sub - 1] = subvol_nnodes;
	total_elements_counter[sub - 1] = subvol_nelems;
    } else {
	total_nodes_counter[sub - 1] += subvol_nnodes;
	total_elements_counter[sub - 1] += subvol_nelems;
    }

    if (current_block == number_of_blocks - 1) {
	//printf("TotalNumberOfNodes: %ld, %ld\n",total_nodes_counter[sub-1], subvol_nnodes);
	//printf("TotalNumberOfElements: %ld, %ld\n",total_elements_counter[sub-1], subvol_nelems);

	sprintf(filename, "%s/%s_%05d_%07d_%07d.kvsml", Fparam.out_dir, Fparam.out_prefix, step, sub, all);
	sprintf(ofname, "%s_%07d_%07d", Fparam.out_prefix, sub, all);
	sprintf(vfname, "%s_%05d_%07d_%07d", Fparam.out_prefix, step, sub, all);
	/* オープン */
	ifs = fopen(filename, "w");
	if (NULL == ifs) {
	    filter_log(FILTER_ERR, "can not open %s.\n", filename);
	    goto garbage;
	}

	fprintf(ifs, "<?xml version=\"1.0\" ?>\n");
	fprintf(ifs, "<KVSML>\n");
	fprintf(ifs, "    <Object type=\"UnstructuredVolumeObject\">\n");
	if (m_element_type == 4) {
	    fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"tetrahedra\">\n");
	} else if (m_element_type == 11) {
	    fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic tetrahedra\">\n");
	} else if (m_element_type == 7) {
	    fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"hexahedra\">\n");
	} else if (m_element_type == 14) {
	    fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic hexahedra\">\n");
	} else if (m_element_type == 5) {
	    fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"pyramid\">\n");
	} else if (m_element_type == 6) {
	    fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"prism\">\n");
	} else if (m_element_type == 2) {
	    fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"triangle\">\n");
	} else if (m_element_type == 3) {
	    fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic\">\n");
	} else if (m_element_type == 9) {
	    fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"triangle2\">\n");
	} else if (m_element_type == 10) {
	    fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"quadratic2\">\n");
	} else {
	    fprintf(ifs, "        <UnstructuredVolumeObject cell_type=\"unknown\">\n");
	}
	//fprintf(ifs, "            <Node nnodes=\"%ld\">\n", total_nodes_counter[sub - 1]);
	fprintf(ifs, "            <Node nnodes=\"%lld\">\n", total_nodes_counter[sub - 1]);
	if (Fparam.component < 0) {
	    fprintf(ifs, "                <Value veclen=\"%d\">\n", m_nkinds);
	} else {
	    fprintf(ifs, "                <Value veclen=\"1\">\n");
	}
	fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_value.dat\" format=\"binary\" />\n", vfname);
	fprintf(ifs, "                </Value>\n");
	fprintf(ifs, "                <Coord>\n");
	fprintf(ifs, "                    <DataArray type=\"float\" file=\"%s_coord.dat\" format=\"binary\" />\n", ofname);
	fprintf(ifs, "                </Coord>\n");
	fprintf(ifs, "            </Node>\n");
	//fprintf(ifs, "            <Cell ncells=\"%ld\">\n", total_elements_counter[sub - 1]);
	fprintf(ifs, "            <Cell ncells=\"%lld\">\n", total_elements_counter[sub - 1]);
	fprintf(ifs, "                <Connection>\n");
	fprintf(ifs, "                    <DataArray type=\"uint\"  file=\"%s_connect.dat\" format=\"binary\" />\n", ofname);
	fprintf(ifs, "                </Connection>\n");
	fprintf(ifs, "            </Cell>\n");
	fprintf(ifs, "        </UnstructuredVolumeObject>\n");
	fprintf(ifs, "    </Object>\n");
	fprintf(ifs, "</KVSML>\n");

	if (sub == m_nvolumes) {
	    filter_log(FILTER_ERR, "write_kvsml_xmls_multi ONE TIME FREE\n");
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
int write_kvsml_geom(int sub,
	int all,
	int subvol_no,
	Int64 subvol_nelems,
	Int64 volume_nodes,
	Int64 rank_nodes,
	Int64 rank_elems) {
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
    //if (Fparam.input_format == PLOT3D_INPUT && m_nvolumes > 8) {
    if (Fparam.input_format == PLOT3D_INPUT) {
	if (current_block > 0) {
	    mode = mode_update;
	}
	if (Fparam.n_layer == 0) {
	    subvol_index = 0;
	} else {
	    subvol_index = subvol_no - 1;
	}
	offset_value = offset_values[subvol_index];
	//filter_log(FILTER_, "OFFSET %i.\n", offset_value);
//	printf("OFFSET %i.\n", offset_value);

    }
    sprintf(filename, "%s/%s_%07d_%07d_coord.dat", Fparam.out_dir, Fparam.out_prefix, sub, all);
    /* オープン */
    if (mode == mode_create) {
	ifs = fopen(filename, "wb");
	DEBUG(3, "The current block is 0, creating coordinate file %i\n", subvol_no);
    } else {
	ifs = fopen(filename, "ab");
    }

    if (NULL == ifs) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }


    // coordinates
    if (volume_nodes > 0) {
	DEBUG(3, "Writing %ld values from block %i to coordinate file: %i \n", volume_nodes, current_block, subvol_no);
	m_subvolume_coord = (float *) malloc(sizeof (int)*volume_nodes * 3);
#ifdef _OPENMP
	{
#pragma omp parallel for default(shared) private(i,nid,j,coord)
#endif
	    for (i = 0; i < volume_nodes; i++) {
		nid = m_subvolume_nodes[rank_nodes + i];
		j = nid * 3;
		coord[0] = m_coords[j ];
		coord[1] = m_coords[j + 1];
		coord[2] = m_coords[j + 2];
		if (ENDIAN) {
		    coord[0] = ConvertEndianF(coord[0]);
		    coord[1] = ConvertEndianF(coord[1]);
		    coord[2] = ConvertEndianF(coord[2]);
		}
		m_subvolume_coord[i * 3 ] = coord[0];
		m_subvolume_coord[i * 3 + 1] = coord[1];
		m_subvolume_coord[i * 3 + 2] = coord[2];
	    }
#ifdef _OPENMP
	}
#endif
	fwrite(m_subvolume_coord, 4, volume_nodes * 3, ifs);
	free(m_subvolume_coord);
	m_subvolume_coord = NULL;
    }
    fclose(ifs);

    sprintf(filename, "%s/%s_%07d_%07d_connect.dat", Fparam.out_dir, Fparam.out_prefix, sub, all);

    /* オープン */
    //ifs = fopen(filename, "wb");
    // if the current clock is 0 create the file, otherwise append to the file
    if (mode == mode_create) {
	ifs = fopen(filename, "wb");
	DEBUG(3, "The current block is 0, creating connection file %i\n", subvol_no);
    } else {
	ifs = fopen(filename, "ab");
    }

    if (NULL == ifs) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }

    if (volume_nodes > 0) {
        DEBUG(3,"Writing %ld connections from block %i to connection file: %i \n", volume_nodes * 8, current_block, subvol_no);
        ivalue = (int *) malloc(sizeof (int) * 20);
        size = 0;
        j = 0;
        if (Fparam.struct_grid) {
            inum = Fparam.dim1;
            jnum = Fparam.dim2;
            /*knum = Fparam.dim3;*/
            if (Fparam.ndim == 2) {
                size = 4;
                for (i = 0; i < subvol_nelems; i++) {
                    eid = (Int64) m_subvolume_elems[rank_elems + i];
                    nid = m_conn_top_node[eid];

                    connect[0] = nid;
                    connect[1] = connect[0] + 1;
                    connect[2] = connect[1] + inum;
                    connect[3] = connect[0] + inum;

                    k = 0;
                    while (k < size) {
                        /* 節点番号でなく節点の定義順番号 */
                        org_id = connect[k];
                        new_id = m_nodes_trans[org_id - 1];
                        ivalue[k] = new_id - 1;
                        if (ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
                        k++;
                    }
                    fwrite(ivalue, 4, size, ifs);
                }
            } else {
                size = 8;
                //                printf("NUMBER OF subvol_nelems: %ld %ld \n", subvol_nelems*8, offset_values[subvol_no-1]);
                for (i = 0; i < subvol_nelems; i++) {
                    eid = (Int64) m_subvolume_elems[rank_elems + i];
                    nid = m_conn_top_node[eid];

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
                        new_id = m_nodes_trans[org_id - 1];
                        if (mode == mode_update) {
                            //ivalue[k] = new_id - 1 + offset_values[subvol_no - 1]; // add the offset here
                            ivalue[k] = new_id - 1 + offset_value; // add the offset here
                        } else {
                            ivalue[k] = new_id - 1;
                        }

                        if (ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
                        k++;
                    }
                    fwrite(ivalue, 4, size, ifs);
                }
                if (Fparam.input_format == PLOT3D_INPUT) {
                    
                    //offset_values[subvol_no - 1] += volume_nodes;
                    offset_values[subvol_index] += volume_nodes;
                    DEBUG(3,"OFFSET UPDATE %ld %ld %d.\n", offset_values[subvol_index], volume_nodes, subvol_index);
                }
                else if (Fparam.input_format == PLOT3D_INPUT && subvol_no == 0) {
                    //filter_log(FILTER_ERR, "IS CALLED WITH subvol_no=0  %i.\n", subvol_no);
                }
            }
        } else {
            size = m_elemcoms;
            for (i = 0; i < subvol_nelems; i++) {
                k = 0;
                j = (Int64) m_subvolume_elems[rank_elems + i]*(Int64) size;
                while (k < size) {
                    /* 節点番号でなく節点の定義順番号 */
                    org_id = m_connections[j + k];
                    new_id = m_nodes_trans[org_id - 1];
                    ivalue[k] = new_id - 1;
                    if (ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
                    k++;
                }
                fwrite(ivalue, 4, size, ifs);
            }
        }
        free(ivalue);
        ivalue = NULL;
    }
    DEBUG(3, "Done Writing %ld connections from block %i to connection file: %i \n", volume_nodes * 8, current_block, subvol_no);
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
int write_kvsml_value_single(int step_loop,
	int sub,
	int all,
	Int64 volume_nodes,
	Int64 rank_nodes) {
    DEBUG(3, "##############write_kvsml_value\n");
    int state = RTC_NG;
    FILE *ifs = NULL;
    modes mode = mode_create;
    //if (Fparam.input_format == PLOT3D_INPUT && m_nvolumes > 8) {
    if (Fparam.input_format == PLOT3D_INPUT) {
	if (current_block > 0) {
	    mode = mode_update;
	}
    }
    sprintf(filename, "%s/%s_%05d_%07d_%07d_value.dat", Fparam.out_dir, Fparam.out_prefix, step_loop, sub, all);
    /* オープン */
    //ifs = fopen(filename, "wb");
    if (mode == mode_create) {
	ifs = fopen(filename, "wb");
	DEBUG(3, "The current block is 0, creating value file %i\n", sub);
    } else {
	ifs = fopen(filename, "ab");
    }
    if (NULL == ifs) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }

    if (volume_nodes > 0) {
	DEBUG(3, "Writing %ld values from block %i to value file: %i \n", volume_nodes, current_block, sub);
	output_value_single(ifs,
		m_nkinds,
		Fparam.component,
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
int write_kvsml_value_multi(int step_loop,
	int sub,
	int all,
	Int64 volume_nodes,
	Int64 rank_nodes) {

    char tmp_filename[LINELEN_MAX];
    int state = RTC_NG;
    FILE *ifs_tmp = NULL;
    FILE *ifs = NULL;
    modes mode = mode_create;
    //if (Fparam.input_format == PLOT3D_INPUT && m_nvolumes > 8) {
    if (Fparam.input_format == PLOT3D_INPUT) {
	if (current_block > 0) {
	    mode = mode_update;
	}
    }
    int k;

    for (k = 0; k < m_nkinds; k++) {
	sprintf(tmp_filename, "%s/%s_%05d_%07d_%07d_value.%03d.tmp", Fparam.out_dir, Fparam.out_prefix, step_loop, sub, all, k);
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
	    filter_log(FILTER_ERR, "can not open %s.\n", tmp_filename);
	    goto garbage;
	}

	if (volume_nodes > 0) {
	    DEBUG(3, "Writing %ld values from block %i to value file: %i \n", volume_nodes, current_block, sub);
	    output_value_multi(ifs_tmp,
		    m_nkinds,
		    Fparam.component,
		    volume_nodes,
		    rank_nodes, k);
	}
	fclose(ifs_tmp); // <---- Close directly after loop 
    }

    char c;
    if (current_block == number_of_blocks - 1) {
	printf("\nConcatenating multi grid components...\n");
	sprintf(filename, "%s/%s_%05d_%07d_%07d_value.dat", Fparam.out_dir, Fparam.out_prefix, step_loop, sub, all);
	ifs = fopen(filename, "wb");
	if (NULL == ifs) {
	    filter_log(FILTER_ERR, "can not open %s.\n", filename);
	    goto garbage;
	}
	for (k = 0; k < m_nkinds; k++) {
	    sprintf(tmp_filename, "%s/%s_%05d_%07d_%07d_value.%03d.tmp", Fparam.out_dir, Fparam.out_prefix, step_loop, sub, all, k);
	    ifs_tmp = fopen(tmp_filename, "rb");
	    if (NULL == ifs_tmp) {
		filter_log(FILTER_ERR, "can not open %s.\n", tmp_filename);
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
    state = RTC_OK;
garbage:
    //if (NULL != ifs_tmp) fclose(ifs_tmp);
    return (state);
}

int write_kvsml_value(int step_loop,
	int sub,
	int all,
	Int64 volume_nodes,
	Int64 rank_nodes) {
    int err_status;
    if (Fparam.input_format == PLOT3D_INPUT && number_of_blocks >1) {
	err_status = write_kvsml_value_multi(step_loop, sub, all, volume_nodes, rank_nodes);
    } else {
	err_status = write_kvsml_value_single(step_loop, sub, all, volume_nodes, rank_nodes);
    }
    return err_status;
}
/*****************************************************************************/

/*
 * サブボリューム単位の出力
 *
 */
int write_value_per_vol(int fnum_loop,
	Int64 volume_nodes,
	Int64 rank_nodes) {
    int state = RTC_NG;
    FILE *ifs = NULL;

    //sprintf(filename, "%s/%s_%07d_%07ld.dat", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
	sprintf(filename, "%s/%s_%07d_%07lld.dat", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
	/* オープン */
    ifs = fopen(filename, "ab");
    if (NULL == ifs) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }

    if (volume_nodes > 0) {
	output_value_single(ifs,
		m_nkinds,
		Fparam.component,
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
int write_value_tmp1(FILE *tmpfp,
	int fnum_loop,
	Int64 volume_nodes,
	Int64 rank_nodes) {
    if (volume_nodes > 0) {
	output_value_single(tmpfp,
		m_nkinds,
		Fparam.component,
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
int write_pfi(NodeInf min, NodeInf max) {
    if (Fparam.input_format == PLOT3D_INPUT) {
	return write_pfi_multi(min, max);

    } else {
	return write_pfi_single(min, max);
    }

}

int write_pfi_single(NodeInf min, NodeInf max)
{
    int state = RTC_NG;
    FILE *pfi = NULL;
    Int64 i;
    Int64 j;
    int id;
    int itmp;
    float ftmp[6];

    sprintf(filename, "%s/%s.pfi", Fparam.out_dir, Fparam.out_prefix);
    /* オープン */
    pfi = fopen(filename, "wb");
    if (NULL == pfi) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }

    itmp = m_nnodes;
    if (ENDIAN) itmp = ConvertEndianI(itmp);
    fwrite(&itmp, 4, 1, pfi);

    itmp = m_nelements;
    if (ENDIAN) itmp = ConvertEndianI(itmp);
    fwrite(&itmp, 4, 1, pfi);

    if (Fparam.struct_grid) {
	itmp = m_element_type;
    } else {
	itmp = m_elemtypes[0];
    }

    if (ENDIAN) itmp = ConvertEndianI(itmp);
    fwrite(&itmp, 4, 1, pfi);

    if (Fparam.file_type == 0) {
	itmp = 0;
    } else {
	if (Fparam.file_type == 1) {
	    itmp = 1;
	} else {
	    itmp = 2;
	}
	if (ENDIAN) itmp = ConvertEndianI(itmp);
    }
    fwrite(&itmp, 4, 1, pfi);

    itmp = Fparam.out_num;
    if (ENDIAN) itmp = ConvertEndianI(itmp);
    fwrite(&itmp, 4, 1, pfi);

    itmp = m_nkinds;
    if (ENDIAN) itmp = ConvertEndianI(itmp);
    fwrite(&itmp, 4, 1, pfi);

    itmp = Fparam.start_step;
    if (ENDIAN) itmp = ConvertEndianI(itmp);
    fwrite(&itmp, 4, 1, pfi);

    itmp = Fparam.end_step;
    if (ENDIAN) itmp = ConvertEndianI(itmp);
    fwrite(&itmp, 4, 1, pfi);

    itmp = m_nvolumes;
    if (ENDIAN) itmp = ConvertEndianI(itmp);
    fwrite(&itmp, 4, 1, pfi);

    ftmp[0] = min.x;
    ftmp[1] = min.y;
    ftmp[2] = min.z;
    ftmp[3] = max.x;
    ftmp[4] = max.y;
    ftmp[5] = max.z;
    filter_log(FILTER_LOG, "all min-max\n");
    filter_log(FILTER_LOG, "%20.8f,%20.8f,%20.8f\n", ftmp[0], ftmp[1], ftmp[2]);
    filter_log(FILTER_LOG, "%20.8f,%20.8f,%20.8f\n", ftmp[3], ftmp[4], ftmp[5]);
    if (ENDIAN) {
	for (i = 0; i < 6; i++) {
	    ftmp[i] = ConvertEndianF(ftmp[i]);
	}
    }
    fwrite(ftmp, 4, 6, pfi);

    for (i = 0; i < m_nvolumes; i++) {
	itmp = m_nodes_per_subvolume[i];
	if (ENDIAN) itmp = ConvertEndianI(itmp);
	fwrite(&itmp, 4, 1, pfi);
    }

    for (i = 0; i < m_nvolumes; i++) {
	itmp = m_elems_per_subvolume[i];
	if (ENDIAN) itmp = ConvertEndianI(itmp);
	fwrite(&itmp, 4, 1, pfi);
    }

    for (j = 0; j < m_nvolumes; j++) {
	id = m_tree_node_no - m_nvolumes + j;
	ftmp[0] = m_tree[id].min.x;
	ftmp[1] = m_tree[id].min.y;
	ftmp[2] = m_tree[id].min.z;
	ftmp[3] = m_tree[id].max.x;
	ftmp[4] = m_tree[id].max.y;
	ftmp[5] = m_tree[id].max.z;
	filter_log(FILTER_LOG, "min-max %04d:\n", id);
	filter_log(FILTER_LOG, "%20.8f,%20.8f,%20.8f\n", ftmp[0], ftmp[1], ftmp[2]);
	filter_log(FILTER_LOG, "%20.8f,%20.8f,%20.8f\n", ftmp[3], ftmp[4], ftmp[5]);
	if (ENDIAN) {
	    for (i = 0; i < 6; i++) {
		ftmp[i] = ConvertEndianF(ftmp[i]);
	    }
	}
	fwrite(ftmp, 4, 6, pfi);
    }

    state = RTC_OK;
garbage:
    if (NULL != pfi) fclose(pfi);
    return (state);
}

int write_pfi_multi(NodeInf min,
	NodeInf max) {
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

    sprintf(filename, "%s/%s.pfi", Fparam.out_dir, Fparam.out_prefix);
    /* オープン */
    pfi = fopen(filename, "wb");
    if (NULL == pfi) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }

    //filter_log(FILTER_ERR, "write_pfi:  %i.\n", m_nnodes);
    if (Fparam.input_format == PLOT3D_INPUT && current_block == 0) {
	total_nodes_counter = m_nnodes;
	total_elements_counter = m_nelements;

	minMax[0] = min.x;
	minMax[1] = min.y;
	minMax[2] = min.z;
	minMax[3] = max.x;
	minMax[4] = max.y;
	minMax[5] = max.z;

	// this branch will be entered only once, so will be initialized only once
	nodesPerSubVolume = malloc(m_nvolumes * sizeof (int));
	elementsPerSubVolume = malloc(m_nvolumes * sizeof (int));
	ranges = malloc(m_nvolumes * sizeof (range));

	for (i = 0; i < m_nvolumes; i++) {
	    nodesPerSubVolume[i] = m_nodes_per_subvolume[i];
	    elementsPerSubVolume[i] = m_elems_per_subvolume[i];
	}

	for (j = 0; j < m_nvolumes; j++) {
	    id = m_tree_node_no - m_nvolumes + j;
	    ranges[j].minX = m_tree[id].min.x;
	    ranges[j].maxX = m_tree[id].max.x;
	    ranges[j].minY = m_tree[id].min.y;
	    ranges[j].maxY = m_tree[id].max.y;
	    ranges[j].minZ = m_tree[id].min.z;
	    ranges[j].maxZ = m_tree[id].max.z;
	}
    } else {
	total_nodes_counter += m_nnodes;
	total_elements_counter += m_nelements;

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

	for (i = 0; i < m_nvolumes; i++) {
	    nodesPerSubVolume[i] += m_nodes_per_subvolume[i];
	    elementsPerSubVolume[i] += m_elems_per_subvolume[i];
	}

	for (j = 0; j < m_nvolumes; j++) {
	    id = m_tree_node_no - m_nvolumes + j;
	    ranges[j].minX = m_tree[id].min.x < ranges[j].minX ? m_tree[id].min.x : ranges[j].minX;
	    ranges[j].maxX = m_tree[id].max.x > ranges[j].maxX ? m_tree[id].max.x : ranges[j].maxX;
	    ranges[j].minY = m_tree[id].min.y < ranges[j].minY ? m_tree[id].min.y : ranges[j].minY;
	    ranges[j].maxY = m_tree[id].max.y > ranges[j].maxY ? m_tree[id].max.y : ranges[j].maxY;
	    ranges[j].minZ = m_tree[id].min.z < ranges[j].minZ ? m_tree[id].min.z : ranges[j].minZ;
	    ranges[j].maxZ = m_tree[id].max.z > ranges[j].maxZ ? m_tree[id].max.z : ranges[j].maxZ;
	}
    }


    if (current_block == number_of_blocks - 1) {
	//itmp = m_nnodes;
	itmp = total_nodes_counter;
	if (ENDIAN) itmp = ConvertEndianI(itmp);
	fwrite(&itmp, 4, 1, pfi);

	//itmp = m_nelements;
	itmp = total_elements_counter;
	if (ENDIAN) itmp = ConvertEndianI(itmp);
	fwrite(&itmp, 4, 1, pfi);

	if (Fparam.struct_grid) {
	    itmp = m_element_type;
	} else {
	    itmp = m_elemtypes[0];
	}

	if (ENDIAN) itmp = ConvertEndianI(itmp);
	fwrite(&itmp, 4, 1, pfi);

	if (Fparam.file_type == 0) {
	    itmp = 0;
	} else {
	    if (Fparam.file_type == 1) {
		itmp = 1;
	    } else {
		itmp = 2;
	    }
	    if (ENDIAN) itmp = ConvertEndianI(itmp);
	}
	fwrite(&itmp, 4, 1, pfi);

	itmp = Fparam.out_num;
	if (ENDIAN) itmp = ConvertEndianI(itmp);
	fwrite(&itmp, 4, 1, pfi);

	itmp = m_nkinds;
	if (ENDIAN) itmp = ConvertEndianI(itmp);
	fwrite(&itmp, 4, 1, pfi);

	itmp = Fparam.start_step;
	if (ENDIAN) itmp = ConvertEndianI(itmp);
	fwrite(&itmp, 4, 1, pfi);

	itmp = Fparam.end_step;
	if (ENDIAN) itmp = ConvertEndianI(itmp);
	fwrite(&itmp, 4, 1, pfi);

	itmp = m_nvolumes;
	if (ENDIAN) itmp = ConvertEndianI(itmp);
	fwrite(&itmp, 4, 1, pfi);

	//ftmp[0] = min.x;
	//ftmp[1] = min.y;
	//ftmp[2] = min.z;
	//ftmp[3] = max.x;
	//ftmp[4] = max.y;
	//ftmp[5] = max.z;
	filter_log(FILTER_LOG, "all min-max\n");
	filter_log(FILTER_LOG, "%20.8f,%20.8f,%20.8f\n", minMax[0], minMax[1], minMax[2]);
	filter_log(FILTER_LOG, "%20.8f,%20.8f,%20.8f\n", minMax[3], minMax[4], minMax[5]);
	if (ENDIAN) {
	    for (i = 0; i < 6; i++) {
		minMax[i] = ConvertEndianF(minMax[i]);
	    }
	}
	fwrite(minMax, 4, 6, pfi);

	for (i = 0; i < m_nvolumes; i++) {
	    //itmp = m_nodes_per_subvolume[i];
	    itmp = nodesPerSubVolume[i];
	    if (ENDIAN) itmp = ConvertEndianI(itmp);
	    fwrite(&itmp, 4, 1, pfi);
	}

	for (i = 0; i < m_nvolumes; i++) {
	    //itmp = m_elems_per_subvolume[i];
	    itmp = elementsPerSubVolume[i];
	    if (ENDIAN) itmp = ConvertEndianI(itmp);
	    fwrite(&itmp, 4, 1, pfi);
	}
	//  ------------------



	//  ------------------
	for (j = 0; j < m_nvolumes; j++) {
	    id = m_tree_node_no - m_nvolumes + j;
	    ftmp[0] = ranges[id].minX;
	    ftmp[1] = ranges[id].minY;
	    ftmp[2] = ranges[id].minZ;
	    ftmp[3] = ranges[id].maxX;
	    ftmp[4] = ranges[id].maxY;
	    ftmp[5] = ranges[id].maxZ;
	    filter_log(FILTER_LOG, "min-max %04d:\n", id);
	    filter_log(FILTER_LOG, "%20.8f,%20.8f,%20.8f\n", ftmp[0], ftmp[1], ftmp[2]);
	    filter_log(FILTER_LOG, "%20.8f,%20.8f,%20.8f\n", ftmp[3], ftmp[4], ftmp[5]);
	    if (ENDIAN) {
		for (i = 0; i < 6; i++) {
		    ftmp[i] = ConvertEndianF(ftmp[i]);
		}
	    }
	    fwrite(ftmp, 4, 6, pfi);
	}
	filter_log(FILTER_ERR, "write_pfi_multi ONE TIME FREE\n");
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
int write_pfi_value(void) {

    if (Fparam.input_format == PLOT3D_INPUT) {
	return write_pfi_value_multi();
    } else {
	return write_pfi_value_single();
    }
}

int write_pfi_value_single(void) {
    int state = RTC_NG;
    FILE *pfi = NULL;
    Int64 i;
    Int64 j;
    float ftmp[2];
    int itmp;

    sprintf(filename, "%s/%s.pfi", Fparam.out_dir, Fparam.out_prefix);
    /* オープン */
    pfi = fopen(filename, "ab");
    if (NULL == pfi) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }

    for (j = 0; j < m_nsteps; j++) {
	filter_log(FILTER_LOG, "step:%i\n", j);
	for (i = 0; i < m_nkinds; i++) {
	    itmp = mt_nullflag[m_nkinds * j + i];
	    ftmp[0] = mt_nulldata[m_nkinds * j + i];
	    filter_log(FILTER_LOG, "flag :  %19d, data : %20.12f\n", itmp, ftmp[0]);
	    if (ENDIAN) {
		itmp = ConvertEndianI(itmp);
		ftmp[0] = ConvertEndianF(ftmp[0]);
	    }
#ifdef RELEASE_NULLFLAG 
	    fwrite(&itmp, 4, 1, pfi);
	    fwrite(&ftmp[0], 4, 1, pfi);
#endif

	    ftmp[0] = m_value_min[m_nkinds * j + i];
	    ftmp[1] = m_value_max[m_nkinds * j + i];
	    filter_log(FILTER_LOG, "[%i] maxv : %20.12f, minv : %20.12f\n", m_nkinds * j + i, ftmp[0], ftmp[1]);
	    if (ENDIAN) {
		ftmp[0] = ConvertEndianF(ftmp[0]);
		ftmp[1] = ConvertEndianF(ftmp[1]);
	    }
	    fwrite(ftmp, 4, 2, pfi);
	}
    }
    state = RTC_OK;
garbage:
    if (NULL != pfi) fclose(pfi);
    return (state);
}

int write_pfi_value_multi(void) {
    int state = RTC_NG;
    FILE *pfi = NULL;
    Int64 i;
    Int64 j;
    float ftmp[2];
    int itmp;
    static int allocFlag = 0;
    static float *min;
    static float *max;

    if (current_block == 0) {
	if (allocFlag == 0) {
	    min = malloc(m_nsteps * m_nkinds * sizeof (float));
	    max = malloc(m_nsteps * m_nkinds * sizeof (float));
	    allocFlag = 1;
	}

	for (j = 0; j < m_nsteps; j++) {
	    for (i = 0; i < m_nkinds; i++) {
		min[m_nkinds * j + i] = m_value_min[m_nkinds * j + i];
		max[m_nkinds * j + i] = m_value_max[m_nkinds * j + i];
	    }
	}
    } else {
	for (j = 0; j < m_nsteps; j++) {
	    for (i = 0; i < m_nkinds; i++) {
		min[m_nkinds * j + i] = m_value_min[m_nkinds * j + i] < min[m_nkinds * j + i] ? m_value_min[m_nkinds * j + i] : min[m_nkinds * j + i];
		max[m_nkinds * j + i] = m_value_max[m_nkinds * j + i] > max[m_nkinds * j + i] ? m_value_max[m_nkinds * j + i] : max[m_nkinds * j + i];
	    }
	}

    }
    // The next if clause will only entered once when processing a multi block plot3d file
    if (current_block == number_of_blocks - 1) {
	sprintf(filename, "%s/%s.pfi", Fparam.out_dir, Fparam.out_prefix);
	filter_log(FILTER_ERR, "Write_pfi_value %s.\n", filename);
	/* オープン */
	pfi = fopen(filename, "ab");
	if (NULL == pfi) {
	    filter_log(FILTER_ERR, "can not open %s.\n", filename);
	    goto garbage;
	}

	for (j = 0; j < m_nsteps; j++) {
	    filter_log(FILTER_LOG, "step:%i\n", j);
	    for (i = 0; i < m_nkinds; i++) {
		itmp = mt_nullflag[m_nkinds * j + i];
		ftmp[0] = mt_nulldata[m_nkinds * j + i];
		filter_log(FILTER_LOG, "flag :  %19d, data : %20.12f\n", itmp, ftmp[0]);
		if (ENDIAN) {
		    itmp = ConvertEndianI(itmp);
		    ftmp[0] = ConvertEndianF(ftmp[0]);
		}
#ifdef RELEASE_NULLFLAG 
		fwrite(&itmp, 4, 1, pfi);
		fwrite(&ftmp[0], 4, 1, pfi);
#endif

		ftmp[0] = min[m_nkinds * j + i];
		ftmp[1] = max[m_nkinds * j + i];
		filter_log(FILTER_LOG, "[%i] maxv : %20.12f, minv : %20.12f\n", m_nkinds * j + i, ftmp[0], ftmp[1]);
		if (ENDIAN) {
		    ftmp[0] = ConvertEndianF(ftmp[0]);
		    ftmp[1] = ConvertEndianF(ftmp[1]);
		}
		fwrite(ftmp, 4, 2, pfi);
	    }
	}
	filter_log(FILTER_ERR, "write_pfi_value_multi ONE TIME FREE\n");
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
int set_geom(int subvol_no,
	Int64 volume_nodes,
	Int64 subvol_nelems,
	Int64 rank_nodes,
	Int64 rank_elems) {
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
    //if (Fparam.input_format == PLOT3D_INPUT && m_nvolumes > 8) {
    if (Fparam.input_format == PLOT3D_INPUT) {
	if (current_block > 0) {
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
	    nid = m_subvolume_nodes[rank_nodes + i];
	    j = nid * 3;
	    coord[0] = m_coords[j ];
	    coord[1] = m_coords[j + 1];
	    coord[2] = m_coords[j + 2];
	    if (ENDIAN) {
		coord[0] = ConvertEndianF(coord[0]);
		coord[1] = ConvertEndianF(coord[1]);
		coord[2] = ConvertEndianF(coord[2]);
	    }
	    m_subvolume_coord[i * 3 ] = coord[0];
	    m_subvolume_coord[i * 3 + 1] = coord[1];
	    m_subvolume_coord[i * 3 + 2] = coord[2];
	}
#ifdef _OPENMP
    }
#endif

    size = 0;
    j = 0;
    iadd = 0;
    if (Fparam.mult_element_type) {
	for (i = 0; i < m_nelements; i++) {
	    size = m_elementsize[(int) m_elemtypes[i]];
	    if (subvol_no == m_subvolume_num[i]) {
		k = 0;
		while (k < size) {
		    /* 節点番号でなく節点の定義順番号 */
		    org_id = m_connections[j + k];
		    new_id = m_nodes_trans[org_id - 1];
		    value = new_id - 1;
		    if (ENDIAN) value = ConvertEndianI(value);
		    m_subvolume_connect[iadd] = value;
		    k++;
		    iadd++;
		}
	    }
	    j += size;
	}
    } else {
	if (Fparam.struct_grid) {
	    inum = Fparam.dim1;
	    jnum = Fparam.dim2;
	    //          knum = Fparam.dim3;
	    if (Fparam.ndim == 2) {
		size = 4;
		for (i = 0; i < subvol_nelems; i++) {
		    eid = (Int64) m_subvolume_elems[rank_elems + i];
		    nid = m_conn_top_node[eid];

		    connect[0] = nid;
		    connect[1] = connect[0] + 1;
		    connect[2] = connect[1] + inum;
		    connect[3] = connect[0] + inum;

		    k = 0;
		    while (k < size) {
			/* 節点番号でなく節点の定義順番号 */
			org_id = connect[k];
			new_id = m_nodes_trans[org_id - 1];
			value = new_id - 1;
			if (ENDIAN) value = ConvertEndianI(value);
			m_subvolume_connect[size * i + k] = value;
			k++;
		    }
		}
	    } else {
		size = 8;
		for (i = 0; i < subvol_nelems; i++) {
		    eid = (Int64) m_subvolume_elems[rank_elems + i];
		    nid = m_conn_top_node[eid];

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
			new_id = m_nodes_trans[org_id - 1];
			if (mode == mode_update) {
			    value = new_id - 1 + offset_values[subvol_no - 1];
			} else {
			    value = new_id - 1;
			}

			if (ENDIAN) value = ConvertEndianI(value);
			m_subvolume_connect[size * i + k] = value;
			k++;
		    }
		}
		if (Fparam.input_format == PLOT3D_INPUT && subvol_no > 0) {
		    offset_values[subvol_no - 1] += volume_nodes;
		}
	    }
	} else {
	    size = m_elemcoms;
	    for (i = 0; i < subvol_nelems; i++) {
		k = 0;
		j = (Int64) m_subvolume_elems[rank_elems + i]*(Int64) size;
		while (k < size) {
		    /* 節点番号でなく節点の定義順番号 */
		    org_id = m_connections[j + k];
		    new_id = m_nodes_trans[org_id - 1];
		    value = new_id - 1;
		    if (ENDIAN) value = ConvertEndianI(value);
		    m_subvolume_connect[size * i + k] = value;
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
int set_value(Int64 volume_nodes,
	Int64 rank_nodes) {
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
    component_id = Fparam.component;
    sta = 0;
    end = m_nkinds - 1;
    if (component_id>-1) {
	j = 0;
	for (i = 0; i < m_ncomponents; i++) {
	    if (i == component_id) {
		sta = j;
		end = sta + m_veclens[i] - 1;
		break;
	    }
	    j += m_veclens[i];
	}
    } else {
	sta = 0;
	end = m_nkinds - 1;
    }

#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,nid,j,k)
#endif
    for (i = 0; i < volume_nodes; i++) {
	nid = m_subvolume_nodes[rank_nodes + i];
	j = nid*m_nkinds;
	for (k = sta; k <= end; k++) {
	    m_subvolume_value[volume_nodes * k + i] = m_values[j + k];
	}
    }
    if (ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,nid,j,k)
#endif
	for (i = 0; i < volume_nodes; i++) {
	    nid = m_subvolume_nodes[rank_nodes + i];
	    j = nid*m_nkinds;
	    for (k = sta; k <= end; k++) {
		m_subvolume_value[volume_nodes * k + i] = ConvertEndianF(m_subvolume_value[volume_nodes * k + i]);
	    }
	}
    }
    state = RTC_OK;
    return (state);
}

void output_value_multi(FILE *ifs,
	int nkind,
	int component_id,
	Int64 volume_nodes,
	Int64 rank_nodes, int k) {
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
	for (i = 0; i < m_ncomponents; i++) {
	    if (i == component_id) {
		sta = j;
		end = sta + m_veclens[i] - 1;
		break;
	    }
	    j += m_veclens[i];
	}
    }
    if (component_id>-1) {
	value = (float *) malloc(sizeof (float) * nkind);
	for (i = 0; i < volume_nodes; i++) {
	    nid = m_subvolume_nodes[rank_nodes + i];
	    j = nid*nkind;
	    for (k = sta; k <= end; k++) {
		value[k - sta] = m_values[j + k];
		if (ENDIAN) value[k - sta] = ConvertEndianF(value[k - sta]);
	    }
	    fwrite(value, 4, end - sta + 1, ifs);
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
	//if(current_block == 0)
	//{
	//  ifs1 = fopen(filename,"wb"); // open the new file
	//}
	//else{
	//  ifs1 = fopen(filename,"ab"); // open the new file
	//}

	for (i = 0; i < volume_nodes; i++) {
	    nid = m_subvolume_nodes[rank_nodes + i];
	    j = nid*nkind;
	    value[i] = m_values[j + k];
	    //fprintf(stdout,"%f\n",value[i]);
	}
	if (ENDIAN) {
	    for (i = 0; i < volume_nodes; i++) {
		value[i] = ConvertEndianF(value[i]);
	    }
	}
	fwrite(value, 4, volume_nodes, ifs);

	//fclose(ifs); // close the file
	//}
    }

    free(value);
    value = NULL;
}

void output_value_single(FILE *ifs,
	int nkind,
	int component_id,
	Int64 volume_nodes,
	Int64 rank_nodes) {
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
	for (i = 0; i < m_ncomponents; i++) {
	    if (i == component_id) {
		sta = j;
		end = sta + m_veclens[i] - 1;
		break;
	    }
	    j += m_veclens[i];
	}
    }
    if (component_id>-1) {
	value = (float *) malloc(sizeof (float) * nkind);
	for (i = 0; i < volume_nodes; i++) {
	    nid = m_subvolume_nodes[rank_nodes + i];
	    j = nid*nkind;
	    for (k = sta; k <= end; k++) {
		value[k - sta] = m_values[j + k];
		if (ENDIAN) value[k - sta] = ConvertEndianF(value[k - sta]);
	    }
	    fwrite(value, 4, end - sta + 1, ifs);
	}
    } else {
	value = (float *) malloc(sizeof (float) * volume_nodes);
	for (k = 0; k < nkind; k++) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,nid,j)
#endif
	    for (i = 0; i < volume_nodes; i++) {
		nid = m_subvolume_nodes[rank_nodes + i];
		j = nid*nkind;
		value[i] = m_values[j + k];
	    }
	    if (ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		for (i = 0; i < volume_nodes; i++) {
		    value[i] = ConvertEndianF(value[i]);
		}
	    }
	    fwrite(value, 4, volume_nodes, ifs);
	}
    }

    free(value);
    value = NULL;
}
