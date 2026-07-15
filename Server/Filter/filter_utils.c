#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "filter.h"
#include "filter_log.h"

/*****************************************************************************/

static int realloc_work(Int64);

/*****************************************************************************/

/*
 * パラメータの読み込み
 *
 */
int read_param_file(char* filename) {
    DEBUG(3,"### Reading ParameterFile: %s \n", filename);
    int state = RTC_NG;
    FILE *fp;
    int n;
    char line[LINELEN_MAX];
    char work[LINELEN_MAX];
    char *ch;
    char flag0 = 0;
    char flag1 = 0;
    FilterParam* param;

    param = &Fparam;

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
	    m_ucdbin = 0;
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
	    ch = strchr(line, '=') + 1;
	    param->file_type = atoi(ch);
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
	    filter_log(FILTER_LOG, "paramter (mpi_volume_div) is ignored.\n");
#endif
	} else if (strstr(line, "mpi_step_div")) {
#ifdef MPI_EFFECT
	    ch = strchr(line, '=') + 1;
	    param->mpi_step_div = atoi(ch);
#else
	    filter_log(FILTER_LOG, "paramter (mpi_step_div) is ignored.\n");
#endif
	} else if (strstr(line, "mpi_div")) {
#ifdef MPI_EFFECT
	    ch = strchr(line, '=') + 1;
	    param->mpi_div = atoi(ch);
#else
	    filter_log(FILTER_LOG, "paramter (mpi_div) is ignored.\n");
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
	} else {
	    ;
	}

    }

    fclose(fp);

#ifdef KEI
    sprintf(param->tmp_dir, "..");
#else
    strcpy(param->tmp_dir, param->out_dir);
#endif
    filter_log(FILTER_LOG, "NEW:param->input_format     : %i\n", param->input_format);
    filter_log(FILTER_LOG, "param->in_dir     : %s\n", param->in_dir);
    filter_log(FILTER_LOG, "param->in_prefix  : %s\n", param->in_prefix);
    filter_log(FILTER_LOG, "param->in_suffix  : %s\n", param->in_suffix);
    filter_log(FILTER_LOG, "param->out_dir    : %s\n", param->out_dir);
    filter_log(FILTER_LOG, "param->out_prefix : %s\n", param->out_prefix);
    filter_log(FILTER_LOG, "param->out_suffix : %s\n", param->out_suffix);
    filter_log(FILTER_LOG, "param->tmp_dir    : %s\n", param->tmp_dir);
    filter_log(FILTER_LOG, "param->format     : %s\n", param->format);
    filter_log(FILTER_LOG, "param->start_step : %d\n", param->start_step);
    filter_log(FILTER_LOG, "param->end_step   : %d\n", param->end_step);
    filter_log(FILTER_LOG, "param->n_layer    : %d\n", param->n_layer);
    filter_log(FILTER_LOG, "param->elem_ave   : %d\n", param->elem_ave);
    filter_log(FILTER_LOG, "param->file_type  : %d\n", param->file_type);
    filter_log(FILTER_LOG, "param->out_num    : %d\n", param->out_num);
    filter_log(FILTER_LOG, "param->field_file : %s\n", param->field_file);
    filter_log(FILTER_LOG, "param->ucd_inp    : %s\n", param->ucd_inp);
    filter_log(FILTER_LOG, "param->geom_file  : %s\n", param->geom_file);
    filter_log(FILTER_LOG, "param->avsucd     : %d\n", param->avsucd);
    filter_log(FILTER_LOG, "param->mpi_div          : %d\n", param->mpi_div);
    filter_log(FILTER_LOG, "param->mpi_volume_div   : %d\n", param->mpi_volume_div);
    filter_log(FILTER_LOG, "param->mpi_step_div     : %d\n", param->mpi_step_div);
    filter_log(FILTER_LOG, "param->mult_element_type: %d\n", param->mult_element_type);
    filter_log(FILTER_LOG, "param->temp_delete      : %d\n", param->temp_delete);
    filter_log(FILTER_LOG, "param->blocksize        : %d\n", param->blocksize);
    //filter_log(FILTER_LOG,"param->component  : %d\n",param->component);
    //filter_log(FILTER_LOG,"param->nodesearch : %d\n",param->nodesearch);
    //filter_log(FILTER_LOG,"param->headfoot   : %d\n",param->headfoot  );

    if (param->n_layer > NLAYER_MAX) {
	param->n_layer = NLAYER_MAX;
	filter_log(FILTER_LOG, "The value of specified n_layer exceeded the limit value.\n");
    }
    param->struct_grid = flag0 | flag1;
    filter_log(FILTER_LOG, "param->struct_grid      : %d\n", param->struct_grid);

    state = RTC_OK;

    param->nstep = param->end_step - param->start_step + 1;
    if (param->end_step < param->start_step) {
	filter_log(FILTER_ERR, "The laege and small relation is wrong.\n");
	state = RTC_NG;
    }
    return (state);

}
/*****************************************************************************/

/*
 * 構造格子の場合に情報をセットする
 *
 */
int set_etc(void) {
    Int64 i, j;
    char names[16] = "VARIABLES       ";
    char units[16] = "variables       ";
    char etype;

    etype = (char) 7;
    if (Fparam.avsucd) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
	for (i = 0; i < m_nelements; i += 1) {
	    m_elemtypes[i] = etype;
	    m_materials[i] = 1;
	}
    }
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,j)
#endif
    for (i = 0; i < m_ncomponents; i += 1) {
	m_veclens[i] = Fparam.veclen;
	m_nullflags[i] = 0;
	m_nulldatas[i] = 0.0f;
	for (j = 0; j < COMPONENT_LEN; j++) {
	    pm_names[i][j] = names[j];
	    pm_units[i][j] = units[j];
	}
    }
    return (RTC_OK);
}
/*****************************************************************************/

/*
 * 構造格子から要素を構成する
 *
 */
int create_hexahedron(void) {
    int state = RTC_NG;
    Int64 i, j;
    unsigned int inum;
    unsigned int jnum;
    unsigned int knum;
    unsigned int nelements;
    char names[16] = "VARIABLES       ";
    char units[16] = "variables       ";
    Int64 k;
    unsigned int jj;
    unsigned int node;
    unsigned int eids;
    unsigned int conn = 8;
    unsigned int connect[8];

    inum = Fparam.dim1;
    jnum = Fparam.dim2;
    knum = Fparam.dim3;
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
		    m_connections[eids * conn + jj] = connect[jj];
		}
		if (Fparam.avsucd) {
		    m_elementids[eids] = eids + 1;
		}
	    }
	}
    }

    if (Fparam.avsucd) {
	for (i = 0; i < nelements; i += 1) {
	    m_elemtypes[i] = (char) 7;
	    m_materials[i] = 1;
	}
    }
    for (i = 0; i < m_ncomponents; i += 1) {
	m_veclens[i] = Fparam.veclen;
	m_nullflags[i] = 0;
	m_nulldatas[i] = 0.0f;
	for (j = 0; j < COMPONENT_LEN; j++) {
	    pm_names[i][j] = names[j];
	    pm_units[i][j] = units[j];
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
int create_tetrahedron(void) {
    int state = RTC_NG;
    Int64 i, j;
    unsigned int inum;
    unsigned int jnum;
    unsigned int knum;
    unsigned int nelements;
    char names[16] = "VARIABLES       ";
    char units[16] = "variables       ";
    Int64 k;
    unsigned int ii, jj;
    unsigned int node;
    unsigned int eids;
    unsigned int conn = 4;
    unsigned int connect[8];

    inum = Fparam.dim1;
    jnum = Fparam.dim2;
    knum = Fparam.dim3;
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
			m_connections[eids * 20 + (jj * conn) + ii] = connect[ hexdiv[jj][ii] ];
		    }
		    if (Fparam.avsucd) {
			m_elementids[eids * 5 + jj] = eids * 5 + jj + 1;
		    }
		}
	    }
	}
    }

    if (Fparam.avsucd) {
	for (i = 0; i < nelements; i += 1) {
	    m_elemtypes[i] = (char) 4;
	    m_materials[i] = 1;
	}
    }
    for (i = 0; i < m_ncomponents; i += 1) {
	m_veclens[i] = Fparam.veclen;
	m_nullflags[i] = 0;
	m_nulldatas[i] = 0.0f;
	for (j = 0; j < COMPONENT_LEN; j++) {
	    pm_names[i][j] = names[j];
	    pm_units[i][j] = units[j];
	}
    }

    state = RTC_OK;
    return (state);
}
/*****************************************************************************/
/*
 * 要素を木構造内に振り分け
 *
 */
#ifdef _OPENMP
#define _OPENMP_SW_XXXX
#endif

int distribution_structure(void) {
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

    n_layer = Fparam.n_layer;
    inum = Fparam.dim1;
    jnum = Fparam.dim2;
    knum = Fparam.dim3;
    shift = m_tree_node_no - m_nvolumes;

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
			m_elems_per_subvolume[vol_num - shift]++;
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

    m_element_array = (unsigned int **) malloc(sizeof (unsigned int*)*m_nvolumes);
    ii = 0;
    for (jj = 0; jj < m_nvolumes; jj++) {
	size = m_elems_per_subvolume[jj];
	m_elems_per_subvolume[jj] = 0;
	m_element_array[jj] = (unsigned int *) malloc(sizeof (unsigned int)*size);
	ii = ii + size;
    }
    if (ii != m_nelements) {
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
		m_element_array[vol_num - shift][cnt] = eid;
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

int distribution_structure_2d(void) {
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

    n_layer = Fparam.n_layer;
    inum = Fparam.dim1;
    jnum = Fparam.dim2;
    //    knum = Fparam.dim3;
    shift = m_tree_node_no - m_nvolumes;

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

    m_element_array = (unsigned int **) malloc(sizeof (unsigned int*)*m_nvolumes);
    ii = 0;
    for (jj = 0; jj < m_nvolumes; jj++) {
	size = m_elems_per_subvolume[jj];
	m_elems_per_subvolume[jj] = 0;
	m_element_array[jj] = (unsigned int *) malloc(sizeof (unsigned int)*size);
	ii = ii + size;
    }
    if (ii != m_nelements) {
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
	    m_element_array[vol_num - shift][cnt] = eid;
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
int distribution_unstructure(void) {
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
    n_layer = Fparam.n_layer;
    shift = m_tree_node_no - m_nvolumes;
    //    flag0 = Fparam.nodesearch;

    /* 要素を振り分け */
    if (Fparam.mult_element_type) {
	DEBUG(3,"#########Entered multi element branch \n");
	addr = 0;
	size = 0;
	DEBUG(4,"looping over %ld elements \n", m_nelements);
	for (ii = 0; ii < m_nelements; ii++) {
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
	    m_elems_per_subvolume[vol_num - (m_tree_node_no - m_nvolumes)]++;
//	    DEBUG(7,"end of for ii\n");
	}
    } else {
	DEBUG(3,"########### entered single element type branch \n");
	for (ii = 0; ii < m_nelements; ii++) {
	    id = m_connections[ii * (Int64) m_elemcoms];
	    i = id - 1;
	    DEBUG(3,"i=%ld, ii=%ld, m_elemcoms=%d", i, ii, m_elemcoms);
	    if (m_octree[i] == 0) {
		DEBUG(3,"m_octree[i] == 0 branch ");
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
	    m_elems_per_subvolume[vol_num - (m_tree_node_no - m_nvolumes)]++;
	}
    }
    DEBUG(3,"############# passed branch \n");
    m_element_array = (unsigned int **) malloc(sizeof (unsigned int*)*m_nvolumes);
    ii = 0;
    for (i = 0; i < m_nvolumes; i++) {
	size = m_elems_per_subvolume[i];
	m_elems_per_subvolume[i] = 0;
	m_element_array[i] = (unsigned int *) malloc(sizeof (unsigned int)*size);
	ii = ii + size;
    }
    if (ii != m_nelements) {
	return (RTC_NG);
    }

    if (Fparam.mult_element_type) {
	addr = 0;
	size = 0;
	for (ii = 0; ii < m_nelements; ii++) {
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
	    m_element_array[vol_num - shift][cnt] = ii;
	    cnt++;
	    m_elems_per_subvolume[vol_num - shift] = cnt;
	}
    } else {
	for (ii = 0; ii < m_nelements; ii++) {
	    id = m_connections[ii * (Int64) m_elemcoms];
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
	    m_element_array[vol_num - shift][cnt] = ii;
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

/*****************************************************************************/

/*
 * サブボリューム内の要素・節点
 *
 */
int count_in_subvolume_b(int subvolume_no,
	Int64* subelemnum,
	Int64* subnodenum,
	Int64* subconnnum) {
    int stat = RTC_OK;
    Int64 i, j, k;
    int node_id;
    int size;

    *subelemnum = 0;
    *subnodenum = 0;
    *subconnnum = 0;

    for (i = 0; i < m_nnodes; i++) {
	m_nodeflag[i] = 0;
	m_nodes_trans[i] = 0;
    }

    if (Fparam.mult_element_type) {
	Int64 addr;
	addr = 0;
	for (i = 0; i < m_nelements; i++) {
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
	size = m_elemcoms;
	elem_num = 0;
#ifdef _OPENMP
	{
#pragma omp parallel for default(shared) private(i,j,node_id,k) reduction(+:elem_num)
#endif
	    for (i = 0; i < m_nelements; i++) {
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

    for (i = 0; i < m_nnodes; i++) {
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
	Int64 rank_elems) {
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

    shift = m_tree_node_no - m_nvolumes;
    *subelemnum = 0;
    *subnodenum = 0;
    *subconnnum = 0;

    if (Fparam.mult_element_type) {
	DEBUG(4,"######## count in subvolume multi element type %ld \n", m_nelements);
	Int64 addr;
	addr = 0;
	for (i = 0; i < m_nelements; i++) {
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
	if (Fparam.struct_grid) {
	    DEBUG(4,"######## count in struct grid\n");
	    inum = Fparam.dim1;
	    jnum = Fparam.dim2;
	    /* knum = Fparam.dim3; */

	    elem_num = m_elems_per_subvolume[subvolume_no - shift];
	    if (Fparam.ndim == 2) {
		size = 4;
		for (i = 0; i < elem_num; i++) {
		    elem_id = m_element_array[subvolume_no - shift][i];
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
		    elem_id = m_element_array[subvolume_no - shift][i];
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
	    size = m_elemcoms;
	    for (i = 0; i < elem_num; i++) {
		elem_id = m_element_array[subvolume_no - shift][i];
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
	if (RTC_OK != realloc_work(rank_nodes + (*subnodenum))) {
	    filter_log(FILTER_ERR, "realloc_work error.\n");
	    return (RTC_NG);
	}
	*prov_len = rank_nodes + (*subnodenum);
    }


    node_num = 0;
    for (i = 0; i < m_nnodes; i++) {
	if (m_nodeflag[i]) {
	    m_subvolume_nodes[rank_nodes + node_num] = i;
	    m_subvolume_trans[rank_nodes + node_num] = node_num + 1;
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
	Int64 rank_nodes) {
    int stat = RTC_OK;
    Int64 i;
    int node_id;

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
    filter_log(FILTER_ERR, "error not found node id %d\n", node_id);
    return -1;
#else
    size_t i;
    for (i = 0; i < m_nnodes; i++) {
	if (node_id == (*m_ids)[i]) {
	    return (int) i;
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

static int realloc_work(
	Int64 new_len) {
    int *p;
    p = (int *) realloc(m_subvolume_nodes, sizeof (int)*new_len);
    if (NULL == p) {
	filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_nodes).\n");
	return (RTC_NG);
    }
    m_subvolume_nodes = p;
    p = (int *) realloc(m_subvolume_trans, sizeof (int)*new_len);
    if (NULL == p) {
	filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_trans).\n");
	return (RTC_NG);
    }
    m_subvolume_trans = p;
    return (RTC_OK);
}

/*****************************************************************************/

/*
 * 節点を含む要素
 *
 */
int set_element_in_node() {
    int stat = RTC_OK;
    Int64 i, j, k;
    int node_id;
    int conn_size;
    int conn_addr;
    int *work;

    conn_addr = 0;
    for (i = 0; i < m_nelements; i++) {
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
    for (i = 1; i < m_nnodes; i++) {
	m_elem_in_node_addr[i] = m_elem_in_node_addr[i - 1] + m_elem_in_node[i - 1];
    }

    work = (int *) malloc(sizeof (int)*m_nnodes);
    memset(work, 0x00, sizeof (int)*m_nnodes);
    m_elem_in_node_array = (int *) malloc(sizeof (int)*conn_addr);

    conn_addr = 0;
    for (i = 0; i < m_nelements; i++) {
	conn_size = m_elementsize[(int) m_elemtypes[i]];
	for (j = 0; j < conn_size; j++) {
	    /* node id */
	    node_id = m_connections[conn_addr + j];
	    k = node_id - 1;
	    m_elem_in_node_array[m_elem_in_node_addr[k] + work[k]] = i + 1;
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
int set_nodeflag_per_element() {
    int stat = RTC_OK;
    Int64 i, j;
    int node_id;
    int conn_size;
    int conn_addr;
    int *work;

    conn_addr = 0;
    for (i = 0; i < m_nelements; i++) {
	conn_size = m_elementsize[(int) m_elemtypes[i]];
	for (j = 0; j < conn_size; j++) {
	    /* node id */
	    node_id = m_connections[conn_addr + j];
	    m_elem_in_node[node_id - 1]++;
	}
	conn_addr += conn_size;
    }

    m_elem_in_node_addr[0] = 0;
    for (i = 1; i < m_nnodes; i++) {
	m_elem_in_node_addr[i] = m_elem_in_node_addr[i - 1] + m_elem_in_node[i - 1];
    }

    work = (int *) malloc(sizeof (int)*m_nnodes);
    memset(work, 0x00, sizeof (int)*m_nnodes);
    m_elem_in_node_array = (int *) malloc(sizeof (int)*conn_addr);

    conn_addr = 0;
    for (i = 0; i < m_nelements; i++) {
	conn_size = m_elementsize[(int) m_elemtypes[i]];
	for (j = 0; j < conn_size; j++) {
	    /* node id */
	    node_id = m_connections[conn_addr + j];
	    m_elem_in_node_array[m_elem_in_node_addr[node_id - 1] + work[node_id - 1]] = i + 1;
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
	Int64 rank_elems) {
    int stat = RTC_OK;
    Int64 i;
    Int64 j;
    Int64 k;
    unsigned int node_id;
    unsigned int node_num;
    char elem_type;
    int size;
    Int64 addr;

    *subelemnum = 0;
    *subnodenum = 0;
    *subconnnum = 0;

    //  elem_type = m_types[elem_id];
    elem_type = elem_id;

    size = 0;
    addr = 0;
    for (i = 0; i < m_nelements; i++) {
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
	if (RTC_OK != realloc_work(rank_nodes + (*subnodenum))) {
	    filter_log(FILTER_ERR, "realloc_work error.\n");
	    return (RTC_NG);
	}
	*prov_len = rank_nodes + (*subnodenum);
    }


    node_num = 0;
    for (i = 0; i < m_nnodes; i++) {
	if (m_nodeflag[i]) {
	    m_subvolume_nodes[rank_nodes + node_num] = i;
	    m_subvolume_trans[rank_nodes + node_num] = node_num + 1;
	    node_num++;
	    m_nodeflag[i] = 0;
	}
    }

    filter_log(FILTER_LOG, "||<> elem_id      .: %5d\n", elem_id);
    filter_log(FILTER_LOG, "||<> node_num     .: %5d\n", node_num);
    filter_log(FILTER_LOG, "||<> nodes    / all nodes    : %9d / %9d\n", *subnodenum, m_nnodes);
    filter_log(FILTER_LOG, "||<> elements / all elements : %9d / %9d\n", *subelemnum, m_nelements);
    filter_log(FILTER_LOG, "||<> subconnnum              : %9d      \n", *subconnnum);
    return (stat);
}
