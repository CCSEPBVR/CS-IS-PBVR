#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "filter_convert.h"
#include "filter_log.h"
#include "filter_io_plot3d/filter_io_plot3d.h"
#include "filter_io_stl/filter_io_stl.h"
#ifdef VTK_ENABLED
#include "filter_io_vtk/filter_io_vtk.h"
#else
// Dummy define
#define PBVR_VTK_POLYDATA -1
#endif

#ifdef _OPENMP
#include "omp.h"
#endif



static int realloc_work(Int64 *,
	Int64);
#ifdef MPI_EFFECT
static void bcast_param(int);
#endif
static int separate_files();
static int read_struct_grid(int, int);
static int initialize_value();
static int initialize_memory();
static int connect_sort(void);

/******************************************************************************/
int filter_convert(int argc, char *argv[]) {
    char outfile[FILELEN_MAX];
    char infile[FILELEN_MAX];
    int rcode = RTC_NG;
    int step_loop;
    int volm_loop;
    int fnum_loop;
    int rstat;
    int count;
    int i, j, k, itmp;
    Int64 ii;
#ifdef MPI_EFFECT
    MPI_Offset offset;
    MPI_File mpifp1, mpifp2;
    MPI_Status mpistat;
    //  int         *mpiwork;
    int color;
    int key;
#else
    FILE *fp1 = NULL;
    FILE *fp2 = NULL;
#endif
    FILE *fp = NULL;
    FILE *ifp = NULL;
    int subvolume_id;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;
    Int64 rank_nodes;
    Int64 rank_elems;
    Int64 total_nodes;
    Int64 total_elems;
    unsigned int *all_subvolume_nodes;
    unsigned int *all_subvolume_trans;
    unsigned int *all_subvolume_elems;
    int ista;
    int id;
    int element_type_num;
    char first_step;
    char m_out_prefix[LINELEN_MAX];
    char m_out_dir[LINELEN_MAX];
    char m_in_prefix[LINELEN_MAX];
    char m_in_dir[LINELEN_MAX];

    // Set all values 0/NULL
    initialize_value();

    subvolume_elems = 0;
    subvolume_nodes = 0;
    subvolume_conns = 0;
    all_subvolume_nodes = NULL;
    all_subvolume_trans = NULL;
    all_subvolume_elems = NULL;

    for (i = 0; i < MAX_ELEMENT_TYPE; i++) {
	m_types[i] = 0;
    }

    // Set default values, related to OPENMP/MPI
    first_step = 1;

    /* 引数チェック */
    //    if(2 != argc){
    //        fprintf(stderr, "usage:%s [paramfile].\n", argv[0]);
    //        return(-1);
    //    }

    numpe = 1;
    mype = 0;
    numpe_f = 1;
    mype_f = 0;
    numpe_s = 1;
    mype_s = 0;
    numpe_v = 1;
    mype_s = 0;
    mpidiv_v = 1;
    mpidiv_s = 1;
    m_ucdbin = 1;

#ifdef MPI_EFFECT
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numpe);
    MPI_Comm_rank(MPI_COMM_WORLD, &mype);
    m_mpi_comm = MPI_COMM_WORLD;
#endif

    myrank = mype;

    filter_log(FILTER_LOG, "\n\n");
    filter_log(FILTER_LOG, "#### numpe   : %3d, mype   : %3d\n", numpe, mype);


#ifdef _OPENMP
#pragma omp parallel default(shared)
    {
	filter_log(FILTER_LOG, "#### Thread parallel : %d / %d\n", omp_get_thread_num(), omp_get_num_threads());
    }
#endif

#ifdef MPI_EFFECT
    if (mype == 0) {
#endif
	/* パラメータファイルの読み込み */
	//        if(RTC_OK!=read_param_file(argv[1])){
	//            filter_log(FILTER_ERR, "read_filter_param error.\n");
	//            goto garbage;
	//        }
#ifdef MPI_EFFECT
    } // if (mype==0)
    if (numpe > 1) {
	bcast_param(mype);
    }
#endif
    if (strlen(Fparam.ucd_inp) != 0) {
	if (RTC_OK != read_inp_file()) {
	    filter_log(FILTER_ERR, "read_ucd_inp_file error.\n");
	    goto garbage;
	}
    }
    m_multi_element = Fparam.mult_element_type;


    if (!Fparam.struct_grid && Fparam.input_format == VTK_INPUT) {
#ifdef VTK_ENABLED
	vtk_count_cellTypes();
	if (Fparam.mult_element_type) {
	    sprintf(m_out_prefix, "%s", Fparam.out_prefix);
	    sprintf(m_out_dir, "%s", Fparam.out_dir);
	    sprintf(m_in_prefix, "%s", Fparam.out_prefix);
	    sprintf(m_in_dir, "%s", Fparam.in_dir);
	} else {
	    sprintf(m_out_prefix, "%s", "");
	}
#endif
    } else if (!Fparam.struct_grid && Fparam.mult_element_type) {
	    /* 混合要素のファイル分割 */
	    separate_files();
#ifdef MPI_EFFECT
	    /*MPI_Finalize();*/
#endif
	    /*return (0);*/

	    sprintf(m_out_prefix, "%s", Fparam.out_prefix);
	    sprintf(m_out_dir, "%s", Fparam.out_dir);
	    sprintf(m_in_prefix, "%s", Fparam.out_prefix);
	    sprintf(m_in_dir, "%s", Fparam.in_dir);
    }

    /********************/
    element_type_num = 0;

    // Label for goto loop construct.
    // goto condition is  (!Fparam.struct_grid && m_multi_element)
unstruct_element_proc:
    if (!Fparam.struct_grid && m_multi_element) {
	DEBUG(1, "############ unstructured multi element::::: \n");
	sprintf(Fparam.in_dir, "%s", Fparam.out_dir);
	while (m_types[element_type_num] == 0) {
	    printf(" m_types[%d]:%d\n", element_type_num, m_types[element_type_num]);
	    element_type_num++;
	    if (element_type_num >= MAX_ELEMENT_TYPE) {
		DEBUG(3, "############ u unstruct_element_stop;:::: \n");
		goto unstruct_element_stop;
	    }
	}
	if (Fparam.input_format != VTK_INPUT) {
	    sprintf(Fparam.in_prefix, "%02d-%s", m_elementtypeno[element_type_num], m_out_prefix);
	    sprintf(Fparam.out_prefix, "%02d-%s", m_elementtypeno[element_type_num], m_out_prefix);
	}
#ifdef VTK_ENABLED
	else {
	    if (Fparam.mult_element_type) {
	    sprintf(Fparam.out_prefix, "%02d-%s", m_element_type, m_out_prefix);
	    }
	}
#endif
	DEBUG(1, "##############>%s\n", Fparam.out_prefix);
    }
    else if (Fparam.input_format == VTK_INPUT) {
	element_type_num= m_element_type;
    }
    /********************/

#ifdef MPI_EFFECT
    if (mype == 0) {
#endif   
	if (Fparam.struct_grid) {

	    if (Fparam.input_format == VTK_INPUT) {
#ifdef VTK_ENABLED
		if (RTC_OK != vtk_setStructGridParameters(Fparam.start_step)) {
		    filter_log(FILTER_ERR, "vtk_read_dims error.\n");
		    goto garbage;
		}
#else 
		filter_log(FILTER_ERR, "VTK not enabled in this build.\n");
#endif

	    } else if (Fparam.input_format == PLOT3D_INPUT) {
		if (RTC_OK != plot3d_setBlockParameters()) {
		    filter_log(FILTER_ERR, "read_filter_fld error.\n");
		    goto garbage;
		};
	    }/* 構造格子                           */
	    else {
		if (RTC_OK != read_fld_file()) {
		    filter_log(FILTER_ERR, "read_filter_fld error.\n");
		    goto garbage;
		}
	    }
	} else {
	    if (Fparam.input_format == STL_ASCII || Fparam.input_format == STL_BIN) {
		if (Fparam.input_format == STL_ASCII) {
		    filter_log(FILTER_LOG, "INPUT IS STL ASCII.\n");
		    if (stl_ascii_read_geom() != RTC_OK) {
			filter_log(FILTER_ERR, "stl_ascii_read_geom error in filter_convert.c .\n");
			goto garbage;
		    }
		} else if (Fparam.input_format == STL_BIN) {
		    filter_log(FILTER_LOG, "INPUT IS STL BINARY.\n");
		    if (stl_bin_read_geom() != RTC_OK) {
			filter_log(FILTER_ERR, "stl_bin_read_geom error in filter_convert.c .\n");
			goto garbage;
		    }
		}

		// contine with setup_stl_geom which is used for the ASCII and binary version
		filter_log(FILTER_LOG, "INPUT IS STL.\n");
		rstat = setup_stl_geom();

	    } else if (Fparam.input_format == VTK_INPUT) {
#ifdef VTK_ENABLED
		filter_log(FILTER_LOG, "||   read vtk unstr data start.\n");
		if (RTC_OK != vtk_readUnstructuredCoordinates(element_type_num)) {
		    filter_log(FILTER_ERR, "vtk_read_structured_points error.\n");
		    goto garbage;
		}
		filter_log(FILTER_LOG, "||   read vtk unstr data end.\n");
#else
		filter_log(FILTER_ERR, "VTK Not enabled in this build.\n");
#endif
	    } else {
		/* 非構造格子                         */
		/* Octree生成のための節点情報・要素構成情報の読み込み */
		filter_log(FILTER_LOG, "|||| Fparam.ucd_inp : %s, m_multi_element : %d\n", Fparam.ucd_inp, m_multi_element);
		if (strlen(Fparam.ucd_inp) != 0 && m_multi_element == 0) {
		    if (m_ucdbin == 0) {
			filter_log(FILTER_LOG, "||   read ucd ascii data start.\n");
			rstat = skip_ucd_ascii(1);
			rstat = read_ucd_ascii_geom();
			rstat = read_ucd_ascii_value(-1);
			fseek(m_ucd_ascii_fp, 0, SEEK_SET);
			filter_log(FILTER_LOG, "||   read ucd ascii data  end.\n");
		    } else {
			filter_log(FILTER_LOG, "||   read ucd binary data start.\n");
			rstat = read_ucdbin_geom();
			filter_log(FILTER_LOG, "||   read ucd binary data end.\n");
		    }
		} else {
		    filter_log(FILTER_LOG, "||   read ucd binary data start.\n");
		    rstat = read_ucdbin_geom();
		    filter_log(FILTER_LOG, "||   read ucd binary data end.\n");
		}
		if (RTC_OK != rstat) {
		    filter_log(FILTER_ERR, "read_ucdbin_geom error.\n");
		    goto garbage;
		}
	    }
	}
#ifdef MPI_EFFECT
    } // if (mype==0)
#endif
#ifdef MPI_EFFECT
    if (numpe > 1) {
	if (first_step) {
	    m_mpi_comm = MPI_COMM_WORLD;
	} else {
	    m_mpi_comm = MPI_COMM_FILTER;
	}
	bcast_param(mype);

	MPI_Bcast(&m_title, 70, MPI_CHAR, 0, m_mpi_comm);
	MPI_Bcast(&m_stepn, 1, MPI_INT, 0, m_mpi_comm);
	MPI_Bcast(&m_stept, 1, MPI_FLOAT, 0, m_mpi_comm);
	MPI_Bcast(&m_nnodes, 1, MPI_LONG, 0, m_mpi_comm);
	MPI_Bcast(&m_nelements, 1, MPI_LONG, 0, m_mpi_comm);
	MPI_Bcast(&m_elemcoms, 1, MPI_INT, 0, m_mpi_comm);
	MPI_Bcast(&m_connectsize, 1, MPI_LONG, 0, m_mpi_comm);
	MPI_Bcast(&m_nnodecomponents, 1, MPI_INT, 0, m_mpi_comm);
	MPI_Bcast(&m_nelemcomponents, 1, MPI_INT, 0, m_mpi_comm);
	MPI_Bcast(&m_ncomponents, 1, MPI_INT, 0, m_mpi_comm);
	MPI_Bcast(&m_nnodekinds, 1, MPI_INT, 0, m_mpi_comm);
	MPI_Bcast(&m_nelemkinds, 1, MPI_INT, 0, m_mpi_comm);
	MPI_Bcast(&m_nkinds, 1, MPI_INT, 0, m_mpi_comm);
	MPI_Bcast(&m_desctype, 1, MPI_INT, 0, m_mpi_comm);
	MPI_Bcast(&m_datatype, 1, MPI_INT, 0, m_mpi_comm);
	MPI_Bcast(&m_element_type, 1, MPI_CHAR, 0, m_mpi_comm);
	MPI_Bcast(&m_filesize, 1, MPI_LONG, 0, m_mpi_comm);
	MPI_Bcast(&m_headfoot, 1, MPI_CHAR, 0, m_mpi_comm);

	if (!Fparam.struct_grid) {
	    if (mype != 0) {
		m_coords = (float *) malloc(sizeof (float)*m_nnodes * 3);
		if (NULL == m_coords) {
		    filter_log(FILTER_ERR, "can not allocate memory(m_coords).\n");
		    goto garbage;
		}
		m_ids = (int *) malloc(sizeof (int)*m_nnodes);
		if (NULL == m_ids) {
		    filter_log(FILTER_ERR, "can not allocate memory(m_ids).\n");
		    goto garbage;
		}
		m_connections = (int *) malloc(sizeof (int)*m_connectsize);
		if (NULL == m_connections) {
		    filter_log(FILTER_ERR, "can not allocate memory(m_connections).\n");
		    goto garbage;
		}
		m_veclens = (int *) malloc(sizeof (int)*m_ncomponents);
		if (NULL == m_veclens) {
		    filter_log(FILTER_ERR, "can not allocate memory(m_veclens).\n");
		    goto garbage;
		}
		m_elemtypes = (char *) malloc(sizeof (char)*m_nelements);
		if (NULL == m_elemtypes) {
		    filter_log(FILTER_ERR, "can not allocate memory(m_elemtypes).\n");
		    goto garbage;
		}
		m_elementids = (int *) malloc(sizeof (int)*m_nelements);
		if (NULL == m_elementids) {
		    filter_log(FILTER_ERR, "can not allocate memory(m_elementid).\n");
		    goto garbage;
		}
		m_materials = (int *) malloc(sizeof (int)*m_nelements);
		if (NULL == m_materials) {
		    filter_log(FILTER_ERR, "can not allocate memory(m_materials).\n");
		    goto garbage;
		}
		pm_names = (char**) malloc(sizeof (char*)*m_ncomponents);
		for (i = 0; i < m_ncomponents; i++) {
		    pm_names[i] = (char*) malloc(sizeof (char)*COMPONENT_LEN);
		}
		pm_units = (char**) malloc(sizeof (char*)*m_ncomponents);
		for (i = 0; i < m_ncomponents; i++) {
		    pm_units[i] = (char*) malloc(sizeof (char)*COMPONENT_LEN);
		}
		m_nullflags = (int *) malloc(sizeof (int)*m_nkinds);
		if (NULL == m_nullflags) {
		    filter_log(FILTER_ERR, "can not allocate memory(m_nullflags).\n");
		    goto garbage;
		}
		m_nulldatas = (float*) malloc(sizeof (float)*m_nkinds);
		if (NULL == m_nulldatas) {
		    filter_log(FILTER_ERR, "can not allocate memory(m_nulldatas).\n");
		    goto garbage;
		}
	    }
	    MPI_Bcast(m_elemtypes, m_nelements, MPI_CHAR, 0, m_mpi_comm);
	    MPI_Bcast(m_elementids, m_nelements, MPI_INT, 0, m_mpi_comm);
	    MPI_Bcast(m_materials, m_nelements, MPI_INT, 0, m_mpi_comm);
	    MPI_Bcast(m_ids, m_nnodes, MPI_INT, 0, m_mpi_comm);
	    MPI_Bcast(m_coords, m_nnodes * 3, MPI_FLOAT, 0, m_mpi_comm);
	    MPI_Bcast(&min.x, 1, MPI_FLOAT, 0, m_mpi_comm);
	    MPI_Bcast(&min.y, 1, MPI_FLOAT, 0, m_mpi_comm);
	    MPI_Bcast(&min.z, 1, MPI_FLOAT, 0, m_mpi_comm);
	    MPI_Bcast(&max.x, 1, MPI_FLOAT, 0, m_mpi_comm);
	    MPI_Bcast(&max.y, 1, MPI_FLOAT, 0, m_mpi_comm);
	    MPI_Bcast(&max.z, 1, MPI_FLOAT, 0, m_mpi_comm);
	    MPI_Bcast(m_connections, m_connectsize, MPI_INT, 0, m_mpi_comm);
	    MPI_Bcast(m_veclens, m_ncomponents, MPI_INT, 0, m_mpi_comm);

	    MPI_Bcast(m_nullflags, m_nkinds, MPI_INT, 0, m_mpi_comm);
	    MPI_Bcast(m_nulldatas, m_nkinds, MPI_FLOAT, 0, m_mpi_comm);
	    for (i = 0; i < m_ncomponents; i++) {
		MPI_Bcast(&pm_names[i][0], COMPONENT_LEN, MPI_CHAR, 0, m_mpi_comm);
		MPI_Bcast(&pm_units[i][0], COMPONENT_LEN, MPI_CHAR, 0, m_mpi_comm);
	    }
	}
    } /* if(numpe>1) */
#endif

    if (!(Fparam.explicit_n_layer) && Fparam.elem_ave > 0) {
	/* 階層数決定 */
	ii = 1;
	for (i = 0; i < NLAYER_MAX + 1; i++) {
	    if (m_nelements / ii <= Fparam.elem_ave) {
		Fparam.n_layer = i;
		break;
	    }
	    ii = ii * 8;
	}
    }
    /* サブボリューム数決定 */
    m_nvolumes = 1;
    for (i = 0; i < Fparam.n_layer; i++) {
	m_nvolumes = m_nvolumes * 8;
    }
    m_nsteps = Fparam.end_step - Fparam.start_step + 1;
    pre_step = Fparam.start_step - 1;
    pre_step = -1;
    filter_log(FILTER_LOG, "#### m_nsteps         = %d, m_nvolumes  = %d\n", m_nsteps, m_nvolumes);
    filter_log(FILTER_LOG, "#### m_ncomponents    = %d, m_nkinds    = %d\n", m_ncomponents, m_nkinds);
    filter_log(FILTER_LOG, "#### m_nnodecomponents= %d, m_nnodekinds= %d\n", m_nnodecomponents, m_nnodekinds);
    filter_log(FILTER_LOG, "#### m_nelemcomponents= %d, m_nelemkinds= %d\n", m_nelemcomponents, m_nelemkinds);

    if (first_step) {
	if (!Fparam.struct_grid && Fparam.nodesearch) {
	    if (RTC_OK != connect_sort()) {
		filter_log(FILTER_ERR, "connect_osrt error.\n");
		goto garbage;
	    }
	    Fparam.nodesearch = (char) 0;
	}
#ifdef MPI_EFFECT
	if (Fparam.mpi_volume_div > 0 && Fparam.mpi_step_div > 0) {
	    if (Fparam.mpi_volume_div * Fparam.mpi_step_div != numpe) {
		filter_log(FILTER_ERR, "Disagreement. product of mpi_volume_div & mpi_step_div.\n");
		goto garbage;
	    }
	}
	if ((Fparam.mpi_volume_div * Fparam.mpi_step_div) > numpe) {
	    Fparam.mpi_volume_div = 0;
	    Fparam.mpi_step_div = 0;
	}
	if (Fparam.mpi_volume_div > 0 && Fparam.mpi_step_div > 0) {
	    color = 0;
	    if ((Fparam.mpi_volume_div * Fparam.mpi_step_div - 1) < mype) {
		color = MPI_UNDEFINED;
	    }
	    key = mype;
	    MPI_Comm_split(MPI_COMM_WORLD, color, key, &MPI_COMM_FILTER);
	    if (color == MPI_UNDEFINED) {
		goto release;
	    }
	    MPI_Comm_size(MPI_COMM_FILTER, &numpe_f);
	    MPI_Comm_rank(MPI_COMM_FILTER, &mype_f);

	    color = mype_f / Fparam.mpi_volume_div;
	    key = mype_f;
	    MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_VOLM);
	    MPI_Comm_size(MPI_COMM_VOLM, &numpe_v);
	    MPI_Comm_rank(MPI_COMM_VOLM, &mype_v);

	    color = mype % Fparam.mpi_volume_div;
	    key = mype_f;
	    MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_STEP);
	    MPI_Comm_size(MPI_COMM_STEP, &numpe_s);
	    MPI_Comm_rank(MPI_COMM_STEP, &mype_s);
	} else {

	    if (Fparam.mpi_div != 1 && Fparam.mpi_div != 2) {
		if (Fparam.file_type == 1) {
		    Fparam.mpi_div = 1;
		} else {
		    if ((numpe % m_nvolumes)<(numpe % m_nsteps)) {
			Fparam.mpi_div = 1;
		    } else {
			Fparam.mpi_div = 2;
		    }
		}
	    }
	    if (Fparam.mpi_div == 1) {
		if (m_nvolumes < numpe) {
		    color = 0;
		    if (((numpe / m_nvolumes) * m_nvolumes - 1) < mype) {
			color = MPI_UNDEFINED;
		    }
		    key = mype;
		    MPI_Comm_split(MPI_COMM_WORLD, color, key, &MPI_COMM_FILTER);
		    if (color == MPI_UNDEFINED) {
			goto release;
		    }
		    MPI_Comm_size(MPI_COMM_FILTER, &numpe_f);
		    MPI_Comm_rank(MPI_COMM_FILTER, &mype_f);
		} else {
		    color = 0;
		    key = mype;
		    MPI_Comm_split(MPI_COMM_WORLD, color, key, &MPI_COMM_FILTER);
		    MPI_Comm_size(MPI_COMM_FILTER, &numpe_f);
		    MPI_Comm_rank(MPI_COMM_FILTER, &mype_f);
		}
	    } else {
		if (m_nsteps < numpe) {
		    color = 0;
		    if (((numpe / m_nsteps) * m_nsteps - 1) < mype) {
			color = MPI_UNDEFINED;
		    }
		    key = mype;
		    MPI_Comm_split(MPI_COMM_WORLD, color, key, &MPI_COMM_FILTER);
		    if (color == MPI_UNDEFINED) {
			goto release;
		    }
		    MPI_Comm_size(MPI_COMM_FILTER, &numpe_f);
		    MPI_Comm_rank(MPI_COMM_FILTER, &mype_f);
		} else {
		    color = 0;
		    key = mype;
		    MPI_Comm_split(MPI_COMM_WORLD, color, key, &MPI_COMM_FILTER);
		    MPI_Comm_size(MPI_COMM_FILTER, &numpe_f);
		    MPI_Comm_rank(MPI_COMM_FILTER, &mype_f);
		}
	    }

	    if (Fparam.mpi_div == 1) {
		color = mype_f / m_nvolumes;
		key = mype_f;
		MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_VOLM);
		MPI_Comm_size(MPI_COMM_VOLM, &numpe_v);
		MPI_Comm_rank(MPI_COMM_VOLM, &mype_v);

		color = mype % m_nvolumes;
		key = mype_f;
		MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_STEP);
		MPI_Comm_size(MPI_COMM_STEP, &numpe_s);
		MPI_Comm_rank(MPI_COMM_STEP, &mype_s);
	    } else {
		color = mype_f / m_nsteps;
		key = mype_f;
		MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_STEP);
		MPI_Comm_size(MPI_COMM_STEP, &numpe_s);
		MPI_Comm_rank(MPI_COMM_STEP, &mype_s);

		color = mype % m_nsteps;
		key = mype_f;
		MPI_Comm_split(MPI_COMM_FILTER, color, key, &MPI_COMM_VOLM);
		MPI_Comm_size(MPI_COMM_VOLM, &numpe_v);
		MPI_Comm_rank(MPI_COMM_VOLM, &mype_v);
	    }
	}

	MPI_Allreduce(&numpe_s, &mpidiv_s, 1, MPI_INT, MPI_MAX, MPI_COMM_FILTER);
	MPI_Allreduce(&numpe_v, &mpidiv_v, 1, MPI_INT, MPI_MAX, MPI_COMM_FILTER);
	filter_log(FILTER_LOG, "#### numpe_f  : %5d, mype_f   : %5d\n", numpe_f, mype_f);
	filter_log(FILTER_LOG, "#### numpe_s  : %5d, mype_s   : %5d\n", numpe_s, mype_s);
	filter_log(FILTER_LOG, "#### numpe_v  : %5d, mype_v   : %5d\n", numpe_v, mype_v);
	filter_log(FILTER_LOG, "#### mpidiv_s : %5d, mpidiv_v : %5d\n", mpidiv_s, mpidiv_v);
#endif
	first_step = 0;
    }

    if (Fparam.struct_grid) {
	// Construct array of 3 file pointers
	m_geom_fp = (FILE **) malloc(sizeof (FILE*)*3);
	for (i = 0; i < 3; i++) {
	    m_geom_fp[i] = NULL;
	}
	// Construct array of [m_nkinds] file pointers
	m_value_fp = (FILE **) malloc(sizeof (FILE*) * m_nkinds);
	for (i = 0; i < m_nkinds; i++) {
	    m_value_fp[i] = NULL;
	}
	// Set element type to 'Quad' or 'Hexahedra', based on dimension
	if (Fparam.ndim == 2) {
	    m_element_type = 3;
	} else {
	    m_element_type = 7;
	}
	m_coords = (float *) malloc(sizeof (float)*m_nnodes * 3);
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
	for (i = 0; i < m_nnodes * 3; i++) {
	    m_coords[i] = 0.0;
	}
	// Allocate int array with  [m_nodes] elements
	if (Fparam.nodesearch || Fparam.avsucd || Fparam.datasettype == PBVR_VTK_POLYDATA) {
	    m_ids = (int *) malloc(sizeof (int)*m_nnodes);
	}
	// Allocate int array with  [m_nelements] elements
	m_conn_top_node = (int *) malloc(sizeof (int)*m_nelements);
	if (NULL == m_conn_top_node) {
	    filter_log(FILTER_ERR, "can not allocate memory(connections).\n");
	    goto garbage;
	}
	/* dummy allocate */
	if (Fparam.avsucd || Fparam.datasettype == PBVR_VTK_POLYDATA) {
	    m_elementids = (int *) malloc(sizeof (int)*m_nelements);
	    m_materials = (int *) malloc(sizeof (int)*m_nelements);
	    m_elemtypes = (char *) malloc(sizeof (char)*m_nelements);
	}
	// Allocate string array with [m_ncomponents] elements
	pm_names = (char**) malloc(sizeof (char*)*m_ncomponents);
	for (i = 0; i < m_ncomponents; i++) {
	    pm_names[i] = (char*) malloc(sizeof (char)*COMPONENT_LEN);
	}
	// Allocate string array with [m_ncomponents] elements
	pm_units = (char**) malloc(sizeof (char*)*m_ncomponents);
	for (i = 0; i < m_ncomponents; i++) {
	    pm_units[i] = (char*) malloc(sizeof (char)*COMPONENT_LEN);
	}
	// Allocate int array with [m_ncomponents] elements
	m_veclens = (int *) malloc(sizeof (int)*m_ncomponents);
	m_veclens[0] = 1;
	// Allocate int array with [m_nkinds] elements
	m_nullflags = (int *) malloc(sizeof (int)*m_nkinds);
	// Allocate float array with [m_nkinds] elements
	m_nulldatas = (float *) malloc(sizeof (float)*m_nkinds);
	if (NULL == m_nulldatas) {
	    filter_log(FILTER_ERR, "can not allocate memory(nulldatas).\n");
	    goto garbage;
	}

	if (RTC_OK != set_etc()) {
	    filter_log(FILTER_ERR, "set_etc error.\n");
	    goto garbage;
	}
	// 3D data and irregular field type
	// Fill m_coords array with IJK coordinates
	if (Fparam.ndim == 3 && strstr(Fparam.field, "irregular")) {
	    min.x = min.y = min.z = 0.0;
	    max.x = (float) (Fparam.dim1 - 1);
	    max.y = (float) (Fparam.dim2 - 1);
	    max.z = (float) (Fparam.dim3 - 1);
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,j,k,ii)
#endif
	    for (k = 0; k < Fparam.dim3; k++) {
		for (j = 0; j < Fparam.dim2; j++) {
		    for (i = 0; i < Fparam.dim1; i++) {
			ii = i + j * Fparam.dim1 + k * Fparam.dim1 * Fparam.dim2;
			m_coords[ii * 3] = (float) i;
			m_coords[ii * 3 + 1] = (float) j;
			m_coords[ii * 3 + 2] = (float) k;
		    }
		}
	    }
	} else {
	    if (RTC_OK != read_struct_grid(numpe_f, mype_f)) {
		filter_log(FILTER_ERR, "can not read struct grid.).\n");
		goto garbage;
	    }
	    min.x = Fparam.min_ext[0];
	    min.y = Fparam.min_ext[1];
	    min.z = Fparam.min_ext[2];
	    max.x = Fparam.max_ext[0];
	    max.y = Fparam.max_ext[1];
	    max.z = Fparam.max_ext[2];
	}
    } /* Fparam.struct_grid */

    filter_log(FILTER_LOG, "  total number of grids       : %10d\n", m_nnodes);
    filter_log(FILTER_LOG, "  total number of elements    : %10d\n", m_nelements);
    filter_log(FILTER_LOG, "  total number of sub-volumes : %10d\n", m_nvolumes);
    filter_log(FILTER_LOG, "  total number of steps       : %10d\n", m_nsteps);
    filter_log(FILTER_LOG, "  min.x                       : %f\n", min.x);
    filter_log(FILTER_LOG, "  min.y                       : %f\n", min.y);
    filter_log(FILTER_LOG, "  min.z                       : %f\n", min.z);
    filter_log(FILTER_LOG, "  max.x                       : %f\n", max.x);
    filter_log(FILTER_LOG, "  max.y                       : %f\n", max.y);
    filter_log(FILTER_LOG, "  max.z                       : %f\n", max.z);

    /////////////////////////////////////////////////////////////////////////////
    /* 木構造の作成  || Creating a tree structure  */
    if (RTC_OK != create_tree_struct(Fparam.n_layer,
	    min,
	    max,
	    &m_tree_node_no,
	    &m_tree)) {
	filter_log(FILTER_ERR, "create_tree_struct error.\n");
	goto garbage;
    }
    if (Fparam.file_type == 1) {
	if (Fparam.out_num == 0) Fparam.out_num = m_nvolumes;
	m_vol_sta = (int *) malloc(sizeof (int)*Fparam.out_num);
	m_vol_end = (int *) malloc(sizeof (int)*Fparam.out_num);
	m_vol_num = (int *) malloc(sizeof (int)*Fparam.out_num);
	if (NULL == m_vol_sta || NULL == m_vol_end || NULL == m_vol_num) {
	    filter_log(FILTER_ERR, "can not allocate memory(m_vol_num).\n");
	    goto garbage;
	}
	for (k = 0; k < Fparam.out_num; k++) {
	    m_vol_num[k] = 0;
	}
	ii = m_nvolumes;
	k = 0;
	while (ii > 0) {
	    m_vol_num[k]++;
	    k++;
	    if (k == Fparam.out_num) {
		k = 0;
	    }
	    ii--;
	}
	m_vol_sta[0] = 0;
	m_vol_end[0] = m_vol_num[0] - 1;
	for (k = 1; k < Fparam.out_num; k++) {
	    m_vol_sta[k] = m_vol_end[k - 1] + 1;
	    m_vol_end[k] = m_vol_sta[k] + m_vol_num[k] - 1;
	}
	free(m_vol_num);
	m_vol_num = NULL;
    }

    m_octree = (int *) malloc(sizeof (int)*m_nnodes);
    // Allocate Array of int with [m_nodes] elements
    m_subvolume_num = (int *) malloc(sizeof (int)*m_nelements);
    // Allocate Array of unsigned int with [m_nvolumes] elements
    m_elems_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*m_nvolumes);
    if (NULL == m_subvolume_num || NULL == m_octree) {
	filter_log(FILTER_ERR, "can not allocate memory(m_octree).\n");
	goto garbage;
    }
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
    // Set arrays to 0. What about m_subvolume_num?
    for (i = 0; i < m_nvolumes; i++) {
	m_elems_per_subvolume[i] = 0;
    }
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
    for (i = 0; i < m_nnodes; i++) {
	m_octree[i] = 0;
    }

    if (Fparam.struct_grid || Fparam.input_format == PLOT3D_INPUT) {
	DEBUG(3, "distribution structure enter\n");
	if (Fparam.ndim == 2) {
	    if (RTC_OK != distribution_structure_2d()) {
		filter_log(FILTER_ERR, "distribution_structure error.\n");
		goto garbage;
	    }
	} else {
	    if (RTC_OK != distribution_structure()) {
		filter_log(FILTER_ERR, "distribution_structure error.\n");
		goto garbage;
	    }
	}
    } else {
	/* 節点を木構造内に振り分け  | And distributes the node in the tree structure*/
	if (RTC_OK != distribution_unstructure()) {
	    filter_log(FILTER_ERR, "distribution_unstructure error.\n");
	    goto garbage;
	}
    }
    DEBUG(3, "distribution passed\n");
    // Free m_octree int array again.... ?
    if (m_octree != NULL) {
	free(m_octree);
	m_octree = NULL;
    }

    rank_elems = 0;
    if (numpe_s < 2) {
	if (Fparam.file_type != 1) {
	    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != volm_loop % numpe_v) {
		    continue;
		}
#endif
		rank_elems += m_elems_per_subvolume[volm_loop];
	    }
	} else {
	    for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != fnum_loop % numpe_v) {
		    continue;
		}
#endif
		for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
		    rank_elems += m_elems_per_subvolume[volm_loop];
		}
	    }
	}
    } else {
	for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
	    if (mype_f != volm_loop % numpe_f) {
		continue;
	    }
#endif
	    rank_elems += m_elems_per_subvolume[volm_loop];
	}
    }

    // Reads structured grid data
    if (Fparam.struct_grid && Fparam.ndim == 3) {
	if (RTC_OK != read_struct_grid(numpe_f, mype_f)) {
	    filter_log(FILTER_ERR, "can not read struct grid.).\n");
	    goto garbage;
	}
    }

    //  m_elems_per_subvolume = (int *)malloc(sizeof(int)*m_nvolumes);

    //Allocate connection and node per sub volume arrays, size of [m_nvolumes]
    m_conns_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*m_nvolumes);
    m_nodes_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*m_nvolumes);
    if (NULL == m_nodes_per_subvolume || NULL == m_conns_per_subvolume) {
	filter_log(FILTER_ERR, "can not allocate memory(m_nodes_per_subvolume).\n");
	goto garbage;
    }

    prov_len = m_nnodes;
    if (numpe_v > 2) {
	prov_len = m_nnodes / numpe_v * 2;
    }
    // Alloc two int arrays with [prov_len|m_nnodes] elements
    m_subvolume_nodes = (int *) malloc(sizeof (int)*prov_len);
    m_subvolume_trans = (int *) malloc(sizeof (int)*prov_len);
    if (NULL == m_subvolume_nodes || NULL == m_subvolume_trans) {
	filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_trans).\n");
	goto garbage;
    }

    prov_len_e = rank_elems;
    if (!Fparam.mult_element_type) {
	m_subvolume_elems = (int *) malloc(sizeof (int)*prov_len_e);
    }
    m_nodes_trans = (int *) malloc(sizeof (int)*m_nnodes);
    m_nodeflag = (char *) malloc(sizeof (char)*m_nnodes);
    if (NULL == m_nodes_trans || NULL == m_nodeflag) {
	filter_log(FILTER_ERR, "can not allocate memory(m_nodes_trans & m_nodeflag).\n");
	goto garbage;
    }

    for (i = 0; i < m_nvolumes; i++) {
	//      m_elems_per_subvolume[i] = 0;
	m_conns_per_subvolume[i] = 0;
	m_nodes_per_subvolume[i] = 0;
    }
    for (i = 0; i < m_nnodes; i++) {
	m_nodeflag[i] = 0;
    }

    rank_nodes = 0;
    rank_elems = 0;
    filter_log(FILTER_LOG, "|||| set sub-volume informations start.\n");
    if (numpe_s < 2) {
	if (Fparam.file_type != 1) {
	    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != volm_loop % numpe_v) {
		    continue;
		}
#endif
		subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
		/* サブボリューム内の要素・節点の判別 */
		if (RTC_OK != count_in_subvolume(subvolume_id,
			&subvolume_elems,
			&subvolume_nodes,
			&subvolume_conns,
			&prov_len,
			rank_nodes,
			rank_elems)) {
		    filter_log(FILTER_ERR, "count_in_subvolume error.\n");
		    goto garbage;
		}

		m_elems_per_subvolume[volm_loop] = subvolume_elems;
		m_nodes_per_subvolume[volm_loop] = subvolume_nodes;
		m_conns_per_subvolume[volm_loop] = subvolume_conns;
		rank_nodes += m_nodes_per_subvolume[volm_loop];
		rank_elems += m_elems_per_subvolume[volm_loop];
	    }
	} else {
	    for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != fnum_loop % numpe_v) {
		    continue;
		}
#endif
		for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
		    subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
		    /* サブボリューム内の要素・節点の判別 */
		    if (RTC_OK != count_in_subvolume(subvolume_id,
			    &subvolume_elems,
			    &subvolume_nodes,
			    &subvolume_conns,
			    &prov_len,
			    rank_nodes,
			    rank_elems)) {
			filter_log(FILTER_ERR, "count_in_subvolume error.\n");
			goto garbage;
		    }
		    m_elems_per_subvolume[volm_loop] = subvolume_elems;
		    m_nodes_per_subvolume[volm_loop] = subvolume_nodes;
		    m_conns_per_subvolume[volm_loop] = subvolume_conns;
		    rank_nodes += m_nodes_per_subvolume[volm_loop];
		    rank_elems += m_elems_per_subvolume[volm_loop];
		}
	    }
	}
    } else {
	for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
	    if (mype_f != volm_loop % numpe_f) {
		continue;
	    }
#endif
	    subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
	    /* サブボリューム内の要素・節点の判別 */
	    if (RTC_OK != count_in_subvolume(subvolume_id,
		    &subvolume_elems,
		    &subvolume_nodes,
		    &subvolume_conns,
		    &prov_len,
		    rank_nodes,
		    rank_elems)) {
		filter_log(FILTER_ERR, "count_in_subvolume error.\n");
		goto garbage;
	    }
	    //          filter_log(FILTER_LOG, " subvolume_id : %d\n",subvolume_id);
	    //          filter_log(FILTER_LOG, " subvolume_elems : %d, subvolume_nodes : %d, subvolume_conns : %d\n",subvolume_elems,subvolume_nodes,subvolume_conns);
	    m_elems_per_subvolume[volm_loop] = subvolume_elems;
	    m_nodes_per_subvolume[volm_loop] = subvolume_nodes;
	    m_conns_per_subvolume[volm_loop] = subvolume_conns;
	    rank_nodes += m_nodes_per_subvolume[volm_loop];
	    rank_elems += m_elems_per_subvolume[volm_loop];
	}

    }
    filter_log(FILTER_LOG, "     rank_nodes  : %d, rank_elems  : %d\n", rank_nodes, rank_elems);
    filter_log(FILTER_LOG, "|||| set sub-volume informations end.\n");
    free(m_nodeflag);
    m_nodeflag = NULL;
    for (i = 0; i < m_nvolumes; i++) {
	free(m_element_array[i]);
    }
    free(m_element_array);
    m_element_array = NULL;
    if (0 < rank_nodes && rank_nodes < prov_len) {
	if (RTC_OK != realloc_work(&prov_len,
		rank_nodes)) {
	    filter_log(FILTER_ERR, "realloc_work error.\n");
	    goto garbage;
	}
    }

#ifdef MPI_EFFECT
    filter_log(FILTER_LOG, "|||| mpi_allreduce start.\n");
#ifdef MPI_MEM_SAVE
    MPI_Allreduce(MPI_IN_PLACE, m_elems_per_subvolume, m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
    MPI_Allreduce(MPI_IN_PLACE, m_nodes_per_subvolume, m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
    MPI_Allreduce(MPI_IN_PLACE, m_conns_per_subvolume, m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
#else
    unsigned int *mpiwork = (unsigned int *) malloc(sizeof (unsigned int)*m_nvolumes);
    memset(mpiwork, 0x00, sizeof (float)*m_nvolumes);
    if (NULL == mpiwork) {
	filter_log(FILTER_ERR, "mpiwork allocate error.\n");
	goto garbage;
    }
    //  MPI_Allreduce(m_elems_per_subvolume,mpiwork,m_nvolumes,MPI_FLOAT,MPI_MAX,MPI_COMM_FILTER);
    MPI_Allreduce(m_elems_per_subvolume, mpiwork, m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
    filter_log(FILTER_LOG, "|||| mpi_allreduce 1.\n");
    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
	m_elems_per_subvolume[volm_loop] = mpiwork[volm_loop];
    }
    //  MPI_Allreduce(m_nodes_per_subvolume,mpiwork,m_nvolumes,MPI_FLOAT,MPI_MAX,MPI_COMM_FILTER);
    MPI_Allreduce(m_nodes_per_subvolume, mpiwork, m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
    filter_log(FILTER_LOG, "|||| mpi_allreduce 2.\n");
    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
	m_nodes_per_subvolume[volm_loop] = mpiwork[volm_loop];
    }
    //  MPI_Allreduce(m_conns_per_subvolume,mpiwork,m_nvolumes,MPI_FLOAT,MPI_MAX,MPI_COMM_FILTER);
    filter_log(FILTER_LOG, "|||| mpi_allreduce 3.\n");
    MPI_Allreduce(m_conns_per_subvolume, mpiwork, m_nvolumes, MPI_UNSIGNED, MPI_MAX, MPI_COMM_FILTER);
    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
	m_conns_per_subvolume[volm_loop] = mpiwork[volm_loop];
    }
    free(mpiwork);
    mpiwork = NULL;
#endif
    filter_log(FILTER_LOG, "|||| mpi_allreduce end.\n");
#endif

    if (Fparam.file_type == 2 || numpe_s > 1) {
	m_conns_accumulate = (unsigned int *) malloc(sizeof (unsigned int)*m_nvolumes);
	m_nodes_accumulate = (unsigned int *) malloc(sizeof (unsigned int)*m_nvolumes);
	if (NULL == m_conns_accumulate || NULL == m_nodes_accumulate) {
	    filter_log(FILTER_ERR, "can not allocate memory(m_nodes_accumulate).\n");
	    goto garbage;
	}
	m_nodes_accumulate[0] = 0;
	m_conns_accumulate[0] = 0;
	for (volm_loop = 1; volm_loop < m_nvolumes; volm_loop++) {
	    m_nodes_accumulate[volm_loop] = m_nodes_accumulate[volm_loop - 1] + m_nodes_per_subvolume[volm_loop - 1];
	    m_conns_accumulate[volm_loop] = m_conns_accumulate[volm_loop - 1] + m_conns_per_subvolume[volm_loop - 1];
	}
	if (numpe_s > 1) {
	    m_elems_accumulate = (unsigned int *) malloc(sizeof (unsigned int)*m_nvolumes);
	    if (NULL == m_elems_accumulate) {
		filter_log(FILTER_ERR, "can not allocate memory(m_elems_accumulate).\n");
		goto garbage;
	    }
	    m_elems_accumulate[0] = 0;
	    for (volm_loop = 1; volm_loop < m_nvolumes; volm_loop++) {
		m_elems_accumulate[volm_loop] = m_elems_accumulate[volm_loop - 1] + m_elems_per_subvolume[volm_loop - 1];
	    }
	}
    }
    ////////
    if (numpe_s > 1) {
#ifdef MPI_EFFECT
	MPI_Allreduce(&rank_nodes, &total_nodes, 1, MPI_LONG, MPI_SUM, MPI_COMM_FILTER);
	MPI_Allreduce(&rank_elems, &total_elems, 1, MPI_LONG, MPI_SUM, MPI_COMM_FILTER);
#else
	total_nodes = rank_nodes;
	total_elems = rank_elems;
#endif
	filter_log(FILTER_LOG, "     total_nodes : %d, total_elems : %d\n", total_nodes, total_elems);
	all_subvolume_nodes = (unsigned int *) malloc(sizeof (unsigned int)*total_nodes);
	if (NULL == all_subvolume_nodes) {
	    filter_log(FILTER_ERR, "can not allocate memory(all_subvolume_nodes).\n");
	    goto garbage;
	}
	all_subvolume_trans = (unsigned int *) malloc(sizeof (unsigned int)*total_nodes);
	if (NULL == all_subvolume_trans) {
	    filter_log(FILTER_ERR, "can not allocate memory(all_subvolume_trans).\n");
	    goto garbage;
	}
	all_subvolume_elems = (unsigned int *) malloc(sizeof (unsigned int)*total_elems);
	if (NULL == all_subvolume_elems) {
	    filter_log(FILTER_ERR, "can not allocate memory(all_subvolume_elems).\n");
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
	for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
	    if (mype_f != volm_loop % numpe_f) {
		continue;
	    }
#endif
	    subvolume_elems = m_elems_per_subvolume[volm_loop];
	    subvolume_nodes = m_nodes_per_subvolume[volm_loop];
	    subvolume_conns = m_conns_per_subvolume[volm_loop];
	    ista = m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
	    for (i = 0; i < subvolume_nodes; i++) {
		all_subvolume_nodes[ista + i] = m_subvolume_nodes[rank_nodes + i];
		all_subvolume_trans[ista + i] = m_subvolume_trans[rank_nodes + i];
	    }
	    ista = m_elems_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
	    for (i = 0; i < subvolume_elems; i++) {
		all_subvolume_elems[ista + i] = m_subvolume_elems[rank_elems + i];
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
	mpiwork = (int *) malloc(sizeof (int)*total_nodes);
	if (NULL == mpiwork) {
	    filter_log(FILTER_ERR, "can not allocate memory(mpiwork). [%10d]\n", total_nodes);
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
	mpiwork = (int *) malloc(sizeof (int)*total_elems);
	if (NULL == mpiwork) {
	    filter_log(FILTER_ERR, "can not allocate memory( mpiwork ). [%10d]\n", total_elems);
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

	free(m_subvolume_nodes);
	free(m_subvolume_trans);
	free(m_subvolume_elems);
	m_subvolume_nodes = NULL;
	m_subvolume_trans = NULL;
	m_subvolume_elems = NULL;

	if (Fparam.file_type != 1) {
	    rank_nodes = 0;
	    rank_elems = 0;
	    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
		if (mype_f != volm_loop % numpe_f) {
		    continue;
		}
#endif
		rank_nodes += m_nodes_per_subvolume[volm_loop];
		rank_elems += m_elems_per_subvolume[volm_loop];
	    }
	    m_subvolume_nodes = (int *) malloc(sizeof (int)*rank_nodes);
	    if (NULL == m_subvolume_nodes) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_nodes).\n");
		goto garbage;
	    }
	    m_subvolume_trans = (int *) malloc(sizeof (int)*rank_nodes);
	    if (NULL == m_subvolume_trans) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_trans).\n");
		goto garbage;
	    }
	    m_subvolume_elems = (int *) malloc(sizeof (int)*rank_elems);
	    if (NULL == m_subvolume_elems) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_elems).\n");
		goto garbage;
	    }

	    rank_nodes = 0;
	    rank_elems = 0;
	    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
		if (mype_f != volm_loop % numpe_f) {
		    continue;
		}
#endif
		subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		subvolume_conns = m_conns_per_subvolume[volm_loop];
		subvolume_elems = m_elems_per_subvolume[volm_loop];
		ista = m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		for (i = 0; i < subvolume_nodes; i++) {
		    m_subvolume_nodes[rank_nodes + i] = all_subvolume_nodes[ista + i];
		    m_subvolume_trans[rank_nodes + i] = all_subvolume_trans[ista + i];
		}
		ista = m_elems_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		for (i = 0; i < subvolume_elems; i++) {
		    m_subvolume_elems[rank_elems + i] = all_subvolume_elems[ista + i];
		}
		rank_nodes += m_nodes_per_subvolume[volm_loop];
		rank_elems += m_elems_per_subvolume[volm_loop];
	    }
	} else {
	    rank_nodes = 0;
	    rank_elems = 0;
	    for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
#ifdef MPI_EFFECT
		/////////////   if(mype_v!=fnum_loop%numpe_v){
		if (mype_f != fnum_loop % numpe_f) {
		    continue;
		}
#endif
		for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
		    rank_nodes += m_nodes_per_subvolume[volm_loop];
		    rank_elems += m_elems_per_subvolume[volm_loop];
		}
	    }
	    m_subvolume_nodes = (int *) malloc(sizeof (int)*rank_nodes);
	    if (NULL == m_subvolume_nodes) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_nodes) .\n");
		goto garbage;
	    }
	    m_subvolume_trans = (int *) malloc(sizeof (int)*rank_nodes);
	    if (NULL == m_subvolume_trans) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_trans) .\n");
		goto garbage;
	    }
	    m_subvolume_elems = (int *) malloc(sizeof (int)*rank_elems);
	    if (NULL == m_subvolume_elems) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_elems) .\n");
		goto garbage;
	    }

	    rank_nodes = 0;
	    rank_elems = 0;

	    for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
#ifdef MPI_EFFECT
		////////////    if(mype_v!=fnum_loop%numpe_v){
		if (mype_f != fnum_loop % numpe_f) {
		    continue;
		}
#endif
		for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
		    subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		    subvolume_conns = m_conns_per_subvolume[volm_loop];
		    subvolume_elems = m_elems_per_subvolume[volm_loop];
		    ista = m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		    for (i = 0; i < subvolume_nodes; i++) {
			m_subvolume_nodes[rank_nodes + i] = all_subvolume_nodes[ista + i];
			m_subvolume_trans[rank_nodes + i] = all_subvolume_trans[ista + i];
		    }
		    ista = m_elems_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		    for (i = 0; i < subvolume_elems; i++) {
			m_subvolume_elems[rank_elems + i] = all_subvolume_elems[ista + i];
		    }
		    rank_nodes += m_nodes_per_subvolume[volm_loop];
		    rank_elems += m_elems_per_subvolume[volm_loop];
		}
	    }
	}
	//      free(all_subvolume_nodes);
	//      free(all_subvolume_trans);
	//      free(all_subvolume_elems);
    }
    ////////

    rank_nodes = 0;
    rank_elems = 0;
    if (Fparam.file_type == 0) {
	filter_log(FILTER_LOG, "|||| output (file_type=0) start.\n");
	for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
	    if (numpe_s > 1) {
		if (mype_f != volm_loop % numpe_f) {
		    continue;
		}
	    } else {
		if (mype_v != volm_loop % numpe_v) {
		    continue;
		}
	    }
#endif
	    subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
	    /* サブボリューム内の要素・節点の判別 */
	    subvolume_elems = m_elems_per_subvolume[volm_loop];
	    subvolume_nodes = m_nodes_per_subvolume[volm_loop];
	    subvolume_conns = m_conns_per_subvolume[volm_loop];
	    if (RTC_OK != set_nodes_in_subvolume(subvolume_nodes,
		    rank_nodes)) {
		filter_log(FILTER_ERR, "set_node_in_subvolume error.\n");
		goto garbage;
	    }
	    /* サブボリューム単位の出力 */
	    if (RTC_OK != write_kvsml_geom(volm_loop + 1,
		    m_nvolumes,
		    subvolume_id,
		    subvolume_elems,
		    subvolume_nodes,
		    rank_nodes,
		    rank_elems)) {
		filter_log(FILTER_ERR, "write_geom error.\n");
		goto garbage;
	    }
	    rank_nodes += m_nodes_per_subvolume[volm_loop];
	    rank_elems += m_elems_per_subvolume[volm_loop];
	}
	filter_log(FILTER_LOG, "|||| output (file_type=0) end.\n");
    }

    if (Fparam.file_type == 2) {
	filter_log(FILTER_LOG, "|||| output (file_type=2) start.\n");
#ifdef MPI_EFFECT
	sprintf(outfile, "%s/%s_coord.dat", Fparam.out_dir, Fparam.out_prefix);
	MPI_File_open(MPI_COMM_FILTER, outfile, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &mpifp1);
	sprintf(outfile, "%s/%s_connect.dat", Fparam.out_dir, Fparam.out_prefix);
	MPI_File_open(MPI_COMM_FILTER, outfile, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &mpifp2);
#else
	sprintf(outfile, "%s/%s_coord.dat", Fparam.out_dir, Fparam.out_prefix);
	fp1 = fopen(outfile, "wb");
	if (NULL == fp1) {
	    filter_log(FILTER_ERR, "can not open %s.\n", outfile);
	    goto garbage;
	}
	sprintf(outfile, "%s/%s_connect.dat", Fparam.out_dir, Fparam.out_prefix);
	fp2 = fopen(outfile, "wb");
	if (NULL == fp1) {
	    filter_log(FILTER_ERR, "can not open %s.\n", outfile);
	    goto garbage;
	}
#endif
	rank_nodes = 0;
	rank_elems = 0;

	for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
	    if (numpe_s > 1) {
		if (mype_f != volm_loop % numpe_f) {
		    continue;
		}
	    } else {
		if (mype_v != volm_loop % numpe_v) {
		    continue;
		}
	    }
#endif
	    subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;

	    subvolume_elems = m_elems_per_subvolume[volm_loop];
	    subvolume_nodes = m_nodes_per_subvolume[volm_loop];
	    subvolume_conns = m_conns_per_subvolume[volm_loop];
	    /* サブボリューム内の要素・節点の判別 */
	    if (RTC_OK != set_nodes_in_subvolume(subvolume_nodes,
		    rank_nodes)) {
		filter_log(FILTER_ERR, "set_node_in_subvolume error.\n");
		goto garbage;
	    }
	    if (subvolume_nodes > 0) {
		m_subvolume_coord = (float *) malloc(sizeof (int)*subvolume_nodes * 3);
		m_subvolume_connect = (int *) malloc(sizeof (int)*subvolume_conns);
		if (NULL == m_subvolume_connect || NULL == m_subvolume_coord) {
		    filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_connect).\n");
		    goto garbage;
		}
		/* サブボリューム単位のセット */
		if (RTC_OK != set_geom(subvolume_id,
			subvolume_nodes,
			subvolume_elems,
			rank_nodes,
			rank_elems)) {
		    filter_log(FILTER_ERR, "set_geom error.\n");
		    goto garbage;
		}
		count = m_nodes_per_subvolume[volm_loop]*3;
#ifdef MPI_EFFECT
		offset = (MPI_Offset) m_nodes_accumulate[volm_loop]*3 * sizeof (float);
		MPI_File_write_at(mpifp1, offset, m_subvolume_coord, count, MPI_FLOAT, &mpistat);
#else
		fwrite(m_subvolume_coord, 4, count, fp1);
#endif
		count = m_conns_per_subvolume[volm_loop];
#ifdef MPI_EFFECT
		offset = (MPI_Offset) m_conns_accumulate[volm_loop] * sizeof (int);
		MPI_File_write_at(mpifp2, offset, m_subvolume_connect, count, MPI_INT, &mpistat);
#else
		fwrite(m_subvolume_connect, 4, count, fp2);
#endif
		free(m_subvolume_coord);
		free(m_subvolume_connect);
		m_subvolume_coord = NULL;
		m_subvolume_connect = NULL;
	    }
	    rank_nodes += m_nodes_per_subvolume[volm_loop];
	    rank_elems += m_elems_per_subvolume[volm_loop];
	}
#ifdef MPI_EFFECT
	MPI_File_sync(mpifp1);
	MPI_File_sync(mpifp2);
	MPI_File_close(&mpifp1);
	MPI_File_close(&mpifp2);
#else
	fclose(fp1);
	fclose(fp2);
#endif
	free(m_conns_accumulate);
	m_conns_accumulate = NULL;
	filter_log(FILTER_LOG, "|||| output (file_type=2) end.\n");
    }
    if (Fparam.file_type == 1) {
	filter_log(FILTER_LOG, "|||| output (file_type=1) start.\n");
	rank_nodes = 0;
	rank_elems = 0;
	/* ファイル数のループ */
	for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
#ifdef MPI_EFFECT
	    if (numpe_s > 1) {
		if (mype_f != fnum_loop % numpe_f) {
		    continue;
		}
	    } else {
		if (mype_v != fnum_loop % numpe_v) {
		    continue;
		}
	    }
#endif
	    // add 2014.11
	    //          sprintf(outfile, "%s/%s.%07d.%07d.vol",Fparam.out_dir,Fparam.out_prefix,fnum_loop+1,Fparam.out_num);
	    if (mpidiv_s > 1) {
		//sprintf(outfile, "%s/%s_%07d_%07ld.tmp0", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
		sprintf(outfile, "%s/%s_%07d_%07lld.tmp0", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
		} else {
		//sprintf(outfile, "%s/%s_%07d_%07ld.dat", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
		sprintf(outfile, "%s/%s_%07d_%07lld.dat", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
		}
	    // add 2014.11
	    fp = fopen(outfile, "wb");
	    if (NULL == fp) {
		filter_log(FILTER_ERR, "can not open %s.\n", outfile);
		goto garbage;
	    }
	    itmp = m_vol_sta[fnum_loop];
	    if (ENDIAN) itmp = ConvertEndianI(itmp);
	    fwrite(&itmp, 4, 1, fp);
	    itmp = m_vol_end[fnum_loop];
	    if (ENDIAN) itmp = ConvertEndianI(itmp);
	    fwrite(&itmp, 4, 1, fp);
	    /* サブボリュームのループ */
	    for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
		subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
		subvolume_elems = m_elems_per_subvolume[volm_loop];
		subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		subvolume_conns = m_conns_per_subvolume[volm_loop];
		/* サブボリューム内の要素・節点の判別 */
		if (RTC_OK != set_nodes_in_subvolume(subvolume_nodes,
			rank_nodes)) {
		    filter_log(FILTER_ERR, "set_node_in_subvolume error.\n");
		    goto garbage;
		}
		if (subvolume_nodes > 0) {
		    m_subvolume_coord = (float *) malloc(sizeof (int)*subvolume_nodes * 3);
		    m_subvolume_connect = (int *) malloc(sizeof (int)*subvolume_conns);
		    if (NULL == m_coords) {
			filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_connect).\n");
			goto garbage;
		    }
		    /* サブボリューム単位のセット */
		    if (RTC_OK != set_geom(subvolume_id,
			    subvolume_nodes,
			    subvolume_elems,
			    rank_nodes,
			    rank_elems)) {
			filter_log(FILTER_ERR, "set_geom error.\n");
			goto garbage;
		    }
		    count = m_conns_per_subvolume[volm_loop];
		    fwrite(m_subvolume_connect, 4, count, fp);
		    count = m_nodes_per_subvolume[volm_loop]*3;
		    fwrite(m_subvolume_coord, 4, count, fp);
		    free(m_subvolume_coord);
		    free(m_subvolume_connect);
		    m_subvolume_coord = NULL;
		    m_subvolume_connect = NULL;
		}
		rank_nodes += m_nodes_per_subvolume[volm_loop];
		rank_elems += m_elems_per_subvolume[volm_loop];
	    } /* サブボリュームのループ */
	    fclose(fp);
	} /* ファイル数のループ */
	filter_log(FILTER_LOG, "|||| output (file_type=1) end.\n");
    }

    free(m_nodes_trans);
    m_nodes_trans = NULL;
    free(m_subvolume_trans);
    m_subvolume_trans = NULL;
    // 2014.11
    free(m_subvolume_elems);
    m_subvolume_elems = NULL;
    if (Fparam.avsucd == 1 || Fparam.datasettype == PBVR_VTK_POLYDATA) {
	free(m_conn_top_node);
	m_conn_top_node = NULL;
    }
    // 2014.11
    ///////////////////////////////
    if (numpe_s > 1) {
	free(m_subvolume_nodes);
	free(m_subvolume_trans);
	free(m_subvolume_elems);
	m_subvolume_nodes = NULL;
	m_subvolume_trans = NULL;
	m_subvolume_elems = NULL;

	if (Fparam.file_type != 1) {
	    rank_nodes = 0;
	    rank_elems = 0;
	    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != volm_loop % numpe_v) {
		    continue;
		}
#endif
		rank_nodes += m_nodes_per_subvolume[volm_loop];
		rank_elems += m_elems_per_subvolume[volm_loop];
	    }
	    m_subvolume_nodes = (int *) malloc(sizeof (int)*rank_nodes);
	    if (NULL == m_subvolume_nodes) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_nodes). [%d]\n", rank_nodes);
		goto garbage;
	    }
	    rank_nodes = 0;
	    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != volm_loop % numpe_v) {
		    continue;
		}
#endif
		subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		ista = m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		for (i = 0; i < subvolume_nodes; i++) {
		    m_subvolume_nodes[rank_nodes + i] = all_subvolume_nodes[ista + i];
		}
		rank_nodes += m_nodes_per_subvolume[volm_loop];
	    }
	    free(all_subvolume_nodes);
	    all_subvolume_nodes = NULL;

	    m_subvolume_trans = (int *) malloc(sizeof (int)*rank_nodes);
	    if (NULL == m_subvolume_trans) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_trans). [%d]\n", rank_nodes);
		goto garbage;
	    }
	    rank_nodes = 0;
	    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != volm_loop % numpe_v) {
		    continue;
		}
#endif
		subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		ista = m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		for (i = 0; i < subvolume_nodes; i++) {
		    m_subvolume_trans[rank_nodes + i] = all_subvolume_trans[ista + i];
		}
		rank_nodes += m_nodes_per_subvolume[volm_loop];
	    }
	    free(all_subvolume_trans);
	    all_subvolume_trans = NULL;

	    m_subvolume_elems = (int *) malloc(sizeof (int)*rank_elems);
	    if (NULL == m_subvolume_elems) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_elems). [%d]\n", rank_elems);
		goto garbage;
	    }

	    rank_elems = 0;
	    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != volm_loop % numpe_v) {
		    continue;
		}
#endif
		subvolume_elems = m_elems_per_subvolume[volm_loop];
		ista = m_elems_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		for (i = 0; i < subvolume_elems; i++) {
		    m_subvolume_elems[rank_elems + i] = all_subvolume_elems[ista + i];
		}
		rank_elems += m_elems_per_subvolume[volm_loop];
	    }
	    free(all_subvolume_elems);
	    all_subvolume_elems = NULL;
	} else {
	    rank_nodes = 0;
	    rank_elems = 0;
	    for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != fnum_loop % numpe_v) {
		    continue;
		}
#endif
		for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
		    rank_nodes += m_nodes_per_subvolume[volm_loop];
		    rank_elems += m_elems_per_subvolume[volm_loop];
		}
	    }
	    m_subvolume_nodes = (int *) malloc(sizeof (int)*rank_nodes);
	    if (NULL == m_subvolume_nodes) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_trans).\n");
		goto garbage;
	    }

	    rank_nodes = 0;
	    for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != fnum_loop % numpe_v) {
		    continue;
		}
#endif
		for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
		    subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		    ista = m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		    for (i = 0; i < subvolume_nodes; i++) {
			m_subvolume_nodes[rank_nodes + i] = all_subvolume_nodes[ista + i];
		    }
		    rank_nodes += m_nodes_per_subvolume[volm_loop];
		}
	    }
	    free(all_subvolume_nodes);
	    all_subvolume_nodes = NULL;

	    m_subvolume_trans = (int *) malloc(sizeof (int)*rank_nodes);
	    if (NULL == m_subvolume_trans) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_trans).\n");
		goto garbage;
	    }
	    rank_nodes = 0;
	    for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != fnum_loop % numpe_v) {
		    continue;
		}
#endif
		for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
		    subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		    ista = m_nodes_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		    for (i = 0; i < subvolume_nodes; i++) {
			m_subvolume_trans[rank_nodes + i] = all_subvolume_trans[ista + i];
		    }
		    rank_nodes += m_nodes_per_subvolume[volm_loop];
		}
	    }
	    free(all_subvolume_trans);
	    all_subvolume_trans = NULL;

	    m_subvolume_elems = (int *) malloc(sizeof (int)*rank_elems);
	    if (NULL == m_subvolume_elems) {
		filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_elems).\n");
		goto garbage;
	    }
	    rank_elems = 0;
	    for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != fnum_loop % numpe_v) {
		    continue;
		}
#endif
		for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
		    subvolume_elems = m_elems_per_subvolume[volm_loop];
		    ista = m_elems_accumulate[volm_loop];
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		    for (i = 0; i < subvolume_elems; i++) {
			m_subvolume_elems[rank_elems + i] = all_subvolume_elems[ista + i];
		    }
		    rank_elems += m_elems_per_subvolume[volm_loop];
		}
	    }
	    free(all_subvolume_elems);
	    all_subvolume_elems = NULL;
	}
    }
    if (Fparam.struct_grid) {
	float x_min, y_min, z_min;
	float x_max, y_max, z_max;
	m_value_max = (float*) malloc(sizeof (float)*m_nvolumes * 3);
	m_value_min = (float*) malloc(sizeof (float)*m_nvolumes * 3);
	if (NULL == m_value_min || NULL == m_value_max) {
	    filter_log(FILTER_ERR, "can not allocate memory(m_value_min&max).\n");
	    goto garbage;
	}
	memset(m_value_max, 0x00, sizeof (float)*m_nvolumes * 3);
	memset(m_value_min, 0x00, sizeof (float)*m_nvolumes * 3);
	for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
	    m_value_min[volm_loop * 3 ] = FLT_MAX;
	    m_value_min[volm_loop * 3 + 1] = FLT_MAX;
	    m_value_min[volm_loop * 3 + 2] = FLT_MAX;
	    m_value_max[volm_loop * 3 ] = FLT_MIN;
	    m_value_max[volm_loop * 3 + 1] = FLT_MIN;
	    m_value_max[volm_loop * 3 + 2] = FLT_MIN;
	}
	if (Fparam.file_type != 1) {
	    rank_nodes = 0;
	    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
		if (numpe_v > 1) {
		    if (mype_v != volm_loop % numpe_v) {
			continue;
		    }
		}
#endif
		subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
		subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		if (subvolume_nodes == 0) {
		    continue;
		}
		x_min = y_min = z_min = FLT_MAX;
		x_max = y_max = z_max = FLT_MIN;
		for (i = 0; i < subvolume_nodes; i++) {
		    id = m_subvolume_nodes[rank_nodes + i];
		    j = id * 3;
		    if (m_coords[j ] < x_min) x_min = m_coords[j ];
		    if (m_coords[j ] > x_max) x_max = m_coords[j ];
		    if (m_coords[j + 1] < y_min) y_min = m_coords[j + 1];
		    if (m_coords[j + 1] > y_max) y_max = m_coords[j + 1];
		    if (m_coords[j + 2] < z_min) z_min = m_coords[j + 2];
		    if (m_coords[j + 2] > z_max) z_max = m_coords[j + 2];
		}
		m_value_min[volm_loop * 3 ] = x_min;
		m_value_min[volm_loop * 3 + 1] = y_min;
		m_value_min[volm_loop * 3 + 2] = z_min;
		m_value_max[volm_loop * 3 ] = x_max;
		m_value_max[volm_loop * 3 + 1] = y_max;
		m_value_max[volm_loop * 3 + 2] = z_max;
		rank_nodes += m_nodes_per_subvolume[volm_loop];
	    }
	} else {
	    rank_nodes = 0;
	    for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != fnum_loop % numpe_v) {
		    continue;
		}
#endif
		for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
		    subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
		    subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		    x_min = y_min = z_min = FLT_MAX;
		    x_max = y_max = z_max = FLT_MIN;
		    for (i = 0; i < subvolume_nodes; i++) {
			id = m_subvolume_nodes[rank_nodes + i];
			j = id * 3;
			if (m_coords[j ] < x_min) x_min = m_coords[j ];
			if (m_coords[j ] > x_max) x_max = m_coords[j ];
			if (m_coords[j + 1] < y_min) y_min = m_coords[j + 1];
			if (m_coords[j + 1] > y_max) y_max = m_coords[j + 1];
			if (m_coords[j + 2] < z_min) z_min = m_coords[j + 2];
			if (m_coords[j + 2] > z_max) z_max = m_coords[j + 2];
		    }
		    m_value_min[volm_loop * 3 ] = x_min;
		    m_value_min[volm_loop * 3 + 1] = y_min;
		    m_value_min[volm_loop * 3 + 2] = z_min;
		    m_value_max[volm_loop * 3 ] = x_max;
		    m_value_max[volm_loop * 3 + 1] = y_max;
		    m_value_max[volm_loop * 3 + 2] = z_max;
		    rank_nodes += m_nodes_per_subvolume[volm_loop];
		}
	    }
	}
#ifdef MPI_EFFECT
	MPI_Allreduce(MPI_IN_PLACE, m_value_min, m_nvolumes * 3, MPI_FLOAT, MPI_MIN, MPI_COMM_FILTER);
	MPI_Allreduce(MPI_IN_PLACE, m_value_max, m_nvolumes * 3, MPI_FLOAT, MPI_MAX, MPI_COMM_FILTER);
#endif
	for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
	    subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
	    m_tree[subvolume_id].min.x = m_value_min[volm_loop * 3 ];
	    m_tree[subvolume_id].min.y = m_value_min[volm_loop * 3 + 1];
	    m_tree[subvolume_id].min.z = m_value_min[volm_loop * 3 + 2];
	    m_tree[subvolume_id].max.x = m_value_max[volm_loop * 3 ];
	    m_tree[subvolume_id].max.y = m_value_max[volm_loop * 3 + 1];
	    m_tree[subvolume_id].max.z = m_value_max[volm_loop * 3 + 2];
	}
	free(m_value_min);
	free(m_value_max);
	m_value_min = NULL;
	m_value_max = NULL;
    }

    /* データ書き込み */
#ifdef MPI_EFFECT
    if (mype_f == 0) {
#endif
	filter_log(FILTER_LOG, "|||| output pfi file start.\n");
	if (RTC_OK != write_pfi(min,
		max)) {
	    filter_log(FILTER_ERR, "write_pfi error.\n");
	    goto garbage;
	}
	filter_log(FILTER_LOG, "|||| output pfi file end.\n");
#ifdef MPI_EFFECT
    }

    free(m_tree);
    m_tree = NULL;
#endif
    ///////////////////////////////
    m_value_max = (float *) malloc(sizeof (float)*m_nkinds * m_nsteps);
    if (NULL == m_value_max) {
	filter_log(FILTER_ERR, "can not allocate memory(m_value_min).\n");
	goto garbage;
    }
    m_value_min = (float *) malloc(sizeof (float)*m_nkinds * m_nsteps);
    if (NULL == m_value_min) {
	filter_log(FILTER_ERR, "can not allocate memory(m_value_min).\n");
	goto garbage;
    }
    mt_nulldata = (float *) malloc(sizeof (float)*m_nkinds * m_nsteps);
    if (NULL == mt_nulldata) {
	filter_log(FILTER_ERR, "can not allocate memory(mt_nulldata).\n");
	goto garbage;
    }
    mt_nullflag = (int *) malloc(sizeof (int)*m_nkinds * m_nsteps);
    if (NULL == mt_nullflag) {
	filter_log(FILTER_ERR, "can not allocate memory(mt_nullflag).\n");
	goto garbage;
    }
    for (i = 0; i < m_nkinds * m_nsteps; i++) {
	m_value_max[i] = 0.0;
	m_value_min[i] = 0.0;
	mt_nulldata[i] = 0.0;
	mt_nullflag[i] = 0;
    }
    m_values = (float *) malloc(sizeof (float)*m_nkinds * m_nnodes);
    if (NULL == m_values) {
	filter_log(FILTER_ERR, "can not allocate memory(values).\n");
	goto garbage;
    }

    // 2014.11
    filter_log(FILTER_LOG, "|||| m_nelemcomponents.\n");
    if (m_nelemcomponents > 0) {
	m_elem_in_node = (int *) malloc(sizeof (int)*m_nnodes);
	m_elem_in_node_addr = (int *) malloc(sizeof (int)*m_nnodes);
	memset(m_elem_in_node, 0x00, sizeof (float)*m_nnodes);
	if (RTC_OK != set_element_in_node()) {
	    filter_log(FILTER_ERR, "set_element_in_node error.\n");
	    goto garbage;
	}
    }
    // 2014.11
    /* ステップ  || Step*/
    ////////////////////
    if (Fparam.struct_grid) {
	m_value_fp = (FILE **) malloc(sizeof (FILE*) * m_nkinds);
	for (i = 0; i < m_nkinds; i++) {
	    m_value_fp[i] = NULL;
	}
    }
    ////////////////////

    for (step_loop = Fparam.start_step; step_loop <= Fparam.end_step; step_loop++) {
#ifdef MPI_EFFECT
	if (mpidiv_s > 1) {
	    if (mype_s != (step_loop - Fparam.start_step) % mpidiv_s) {
		continue;
	    }
	}
#endif
	filter_log(FILTER_LOG, ":::: time_step %d start.\n", step_loop);
#ifdef MPI_EFFECT
	if (mpidiv_v > 1) {
	    if (mype_v > 0) {
		goto skip_read_value;
	    }
	}
#endif
	/* データ読込み  Read data*/
	filter_log(FILTER_LOG, "|||| read variable data start.\n");
	if (Fparam.input_format == PLOT3D_INPUT) {
	    m_stepn = step_loop;
	    rstat = plot3d_getBlockValuesForTstep(step_loop);
	}
#ifdef VTK_ENABLED
	else if (Fparam.input_format == VTK_INPUT) {
	    if (Fparam.struct_grid) {
		rstat = vtk_read_structured_values(step_loop);
	    } else {
		DEBUG(1, " reading step variables for element_type_number %d\n\n", element_type_num);
		rstat = vtk_read_unstructured_values(step_loop, element_type_num);
		//		ASSERT_FAIL("F");
	    }
	}
#endif
	else if (Fparam.input_format == STL_ASCII || Fparam.input_format == STL_BIN) {
	    filter_log(FILTER_LOG, "ENTER STL BRANCH.\n");
	    // When reading STL the there are no values
	    // so set summy values
	    for (count = 0; count < m_nnodes; count++) {
		m_values[count] = FLT_MIN;
	    }
	} else if (Fparam.struct_grid) {
	    m_stepn = step_loop;
	    if (strlen(Fparam.in_prefix) == 0) {
		/*rstat=read_variables( step_loop );*/
		rstat = read_variables_3(step_loop - Fparam.start_step);
	    } else {
		rstat = read_variables_per_step(step_loop);
	    }
	    if (RTC_OK != rstat) {
		filter_log(FILTER_ERR, "read_variables_per_step error.\n");
		goto garbage;
	    }
	} else {
	    if (strlen(Fparam.ucd_inp) != 0 && m_multi_element == 0) {
		if (m_ucdbin == 0) {
		    rstat = skip_ucd_ascii(step_loop);
		    if (m_cycle_type == 1) {
			rstat = skip_ucd_ascii_geom();
		    } else if (m_cycle_type == 2) {
			if (step_loop == Fparam.start_step) {
			    rstat = skip_ucd_ascii_geom();
			}
		    }
		    rstat = read_ucd_ascii_value(step_loop);
		} else {
		    rstat = read_ucdbin_data(step_loop);
		}
	    } else {
		rstat = read_ucdbin_data(step_loop);
	    }
	    if (RTC_OK != rstat) {
		filter_log(FILTER_ERR, "read_ucdbin_data error.\n");
		goto garbage;
	    }
	}
	filter_log(FILTER_LOG, "|||| read variable data end.\n");
#ifdef MPI_EFFECT
skip_read_value:
	if (mpidiv_v > 1) {
	    filter_log(FILTER_LOG, "|||| MPI_Bcast variable data start.\n");
	    MPI_Bcast(&m_stepn, 1, MPI_INT, 0, MPI_COMM_VOLM);
	    MPI_Bcast(&m_stept, 1, MPI_FLOAT, 0, MPI_COMM_VOLM);
	    MPI_Bcast(m_veclens, m_ncomponents, MPI_INT, 0, MPI_COMM_VOLM);
	    MPI_Bcast(m_nullflags, m_nkinds, MPI_INT, 0, MPI_COMM_VOLM);
	    MPI_Bcast(m_nulldatas, m_nkinds, MPI_FLOAT, 0, MPI_COMM_VOLM);
	    MPI_Bcast(m_values, m_nkinds*m_nnodes, MPI_FLOAT, 0, MPI_COMM_VOLM);
	    filter_log(FILTER_LOG, "|||| MPI_Bcast variable data end.\n");
	}
#endif
	/* KVSML形式ファイル  |  KVSML format file */
	if (Fparam.file_type == 0) {
	    /* サブボリュームのループ  | KVSML format file */
	    rank_nodes = 0;
	    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
		if (numpe_v > 1) {
		    if (mype_v != volm_loop % numpe_v) {
			continue;
		    }
		}
#endif
		subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
		subvolume_elems = m_elems_per_subvolume[volm_loop];
		subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		subvolume_conns = m_conns_per_subvolume[volm_loop];
		filter_log(FILTER_LOG, " ||| volm_loop    : %d\n", volm_loop);
		filter_log(FILTER_LOG, " ||| subvolume_id : %d\n", subvolume_id);
		filter_log(FILTER_LOG, " |||  subvolume_elems : %d\n", subvolume_elems);
		filter_log(FILTER_LOG, " |||  subvolume_nodes : %d\n", subvolume_nodes);
		filter_log(FILTER_LOG, " |||  subvolume_conns : %d\n", subvolume_conns);
		/* サブボリューム単位の出力    The output of the sub-volume unit */
		/* XML形式ファイルの出力      The output of the XML format file */
		if (RTC_OK != write_kvsml_xmls(step_loop,
			volm_loop + 1,
			m_nvolumes,
			subvolume_elems,
			subvolume_nodes)) {
		    filter_log(FILTER_ERR, "write_kvsml_xmls error.\n");
		    goto garbage;
		}
		/* VARIABLESの出力 */
		if (RTC_OK != write_kvsml_value(step_loop,
			volm_loop + 1,
			m_nvolumes,
			subvolume_nodes,
			rank_nodes)) {
		    filter_log(FILTER_ERR, "write_kvsml_value error.\n");
		    goto garbage;
		}
		rank_nodes += m_nodes_per_subvolume[volm_loop];
	    } /* サブボリュームのループ */
	}

	/* ステップ集約ファイル */
	if (Fparam.file_type == 2) {
	    //          sprintf(buff,Fparam.format,step_loop );
#ifdef MPI_EFFECT
	    sprintf(outfile, "%s/%s_%05d_value.dat", Fparam.out_dir, Fparam.out_prefix, step_loop);
	    MPI_File_open(MPI_COMM_VOLM, outfile, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &mpifp1);
#else
	    sprintf(outfile, "%s/%s_%05d_value.dat", Fparam.out_dir, Fparam.out_prefix, step_loop);
	    fp1 = fopen(outfile, "wb");
	    if (NULL == fp1) {
		filter_log(FILTER_ERR, "can not open %s.\n", outfile);
		goto garbage;
	    }
#endif
	    /* サブボリュームのループ */
	    rank_nodes = 0;
	    for (volm_loop = 0; volm_loop < m_nvolumes; volm_loop++) {
#ifdef MPI_EFFECT
		if (numpe_v > 1) {
		    if (mype_v != volm_loop % numpe_v) {
			continue;
		    }
		}
#endif
		subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
		subvolume_elems = m_elems_per_subvolume[volm_loop];
		subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		subvolume_conns = m_conns_per_subvolume[volm_loop];
		m_subvolume_value = (float *) malloc(sizeof (int)*subvolume_nodes * m_nkinds);
		/* サブボリューム単位のセット */
		if (RTC_OK != set_value(subvolume_nodes,
			rank_nodes)) {
		    filter_log(FILTER_ERR, "set_value error.\n");
		    goto garbage;
		}
		count = m_nodes_per_subvolume[volm_loop] * m_nkinds;
#ifdef MPI_EFFECT
		offset = (MPI_Offset) m_nodes_accumulate[volm_loop] * m_nkinds * sizeof (float);
		MPI_File_write_at(mpifp1, offset, m_subvolume_value, count, MPI_FLOAT, &mpistat);
#else
		fwrite(m_subvolume_value, 4, count, fp1);
#endif
		free(m_subvolume_value);
		m_subvolume_value = NULL;
		rank_nodes += m_nodes_per_subvolume[volm_loop];
	    } /* サブボリュームのループ */
#ifdef MPI_EFFECT
	    MPI_File_sync(mpifp1);
	    MPI_File_close(&mpifp1);
#else
	    fclose(fp1);
#endif
	}

	/* サブボリューム集約ファイル形式 (Fparam.collect==1) */
	if (Fparam.file_type == 1) {
	    /* ファイル数のループ */
	    rank_nodes = 0;
	    for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
#ifdef MPI_EFFECT
		if (mype_v != fnum_loop % numpe_v) {
		    continue;
		}
#endif
		// 2014.11
		if (mpidiv_s > 1) {
		    //sprintf(outfile, "%s/%s_%07d_%07ld-%05d.tmp1", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num, step_loop);
			sprintf(outfile, "%s/%s_%07d_%07lld-%05d.tmp1", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num, step_loop);
			fp = fopen(outfile, "wb");
		    /* サブボリュームのループ */
		    for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
			subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
			subvolume_elems = m_elems_per_subvolume[volm_loop];
			subvolume_nodes = m_nodes_per_subvolume[volm_loop];
			subvolume_conns = m_conns_per_subvolume[volm_loop];
			/* サブボリューム単位の出力 */
			if (RTC_OK != write_value_tmp1(fp,
				fnum_loop,
				subvolume_nodes,
				rank_nodes)) {
			    filter_log(FILTER_ERR, "write_value_per_vol error.\n");
			    goto garbage;
			}
			rank_nodes += m_nodes_per_subvolume[volm_loop];
		    } /* サブボリュームのループ */
		    fclose(fp);
		} else {
		    /* サブボリュームのループ */
		    for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
			subvolume_id = m_tree_node_no - m_nvolumes + volm_loop;
			subvolume_elems = m_elems_per_subvolume[volm_loop];
			subvolume_nodes = m_nodes_per_subvolume[volm_loop];
			subvolume_conns = m_conns_per_subvolume[volm_loop];
			/* サブボリューム単位の出力 */
			if (RTC_OK != write_value_per_vol(fnum_loop,
				subvolume_nodes,
				rank_nodes)) {
			    filter_log(FILTER_ERR, "write_value_per_vol error.\n");
			    goto garbage;
			}
			rank_nodes += m_nodes_per_subvolume[volm_loop];
		    } /* サブボリュームのループ */
		}
		//              fclose(fp);
	    } /* ファイル数のループ */
	} /* サブボリューム集約ファイル形式 (Fparam.collect==1) */

	if (!Fparam.struct_grid && m_multi_element && Fparam.temp_delete) {
	    sprintf(Fparam.in_dir, "%s", Fparam.tmp_dir);
	    sprintf(buff, Fparam.format, step_loop);
	    sprintf(filename, "%s/%02d-%s%s%s", Fparam.tmp_dir, m_elementtypeno[element_type_num], m_out_prefix, buff, Fparam.in_suffix);
	    remove(filename);
	}
	filter_log(FILTER_LOG, ":::: time_step %d end.\n", step_loop);
    }

#ifdef MPI_EFFECT
    if (numpe_s > 1) {
	if (mype_v == 0) {

#ifdef MPI_MEM_SAVE
	    MPI_Allreduce(MPI_IN_PLACE, m_value_max, m_nkinds*m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
	    MPI_Allreduce(MPI_IN_PLACE, m_value_min, m_nkinds*m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
	    MPI_Allreduce(MPI_IN_PLACE, mt_nulldata, m_nkinds*m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
	    MPI_Allreduce(MPI_IN_PLACE, mt_nullflag, m_nkinds*m_nsteps, MPI_INT, MPI_SUM, MPI_COMM_STEP);
#else
	    float *mpiworkf = (float *) malloc(sizeof (float)*m_nkinds * m_nsteps);
	    for (i = 0; i < m_nkinds * m_nsteps; i++) {
		mpiworkf[i] = 0.0;
	    }
	    MPI_Allreduce(m_value_max, mpiworkf, m_nkinds*m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
	    for (i = 0; i < m_nkinds * m_nsteps; i++) {
		m_value_max[i] = mpiworkf[i];
	    }
	    for (i = 0; i < m_nkinds * m_nsteps; i++) {
		mpiworkf[i] = 0.0;
	    }
	    MPI_Allreduce(m_value_min, mpiworkf, m_nkinds*m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
	    for (i = 0; i < m_nkinds * m_nsteps; i++) {
		m_value_min[i] = mpiworkf[i];
	    }
	    free(mpiworkf);
	    for (i = 0; i < m_nkinds * m_nsteps; i++) {
		mpiworkf[i] = 0.0;
	    }
	    MPI_Allreduce(mt_nulldata, mpiworkf, m_nkinds*m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
	    for (i = 0; i < m_nkinds * m_nsteps; i++) {
		mt_nulldata[i] = mpiworkf[i];
	    }
	    for (i = 0; i < m_nkinds * m_nsteps; i++) {
		mpiworkf[i] = 0.0;
	    }
	    MPI_Allreduce(mt_nullflag, mpiworkf, m_nkinds*m_nsteps, MPI_FLOAT, MPI_SUM, MPI_COMM_STEP);
	    for (i = 0; i < m_nkinds * m_nsteps; i++) {
		mt_nullflag[i] = mpiworkf[i];
	    }
	    free(mpiworkf);
	    mpiworkf = NULL;
#endif

	}
    }
#endif
#ifdef MPI_EFFECT
    if (mype_f == 0) {
#endif
	if (RTC_OK != write_pfi_value()) {
	    filter_log(FILTER_ERR, "write_pfi_value error.\n");
	    goto garbage;
	}
#ifdef MPI_EFFECT
    }
#endif
#ifdef MPI_EFFECT
    MPI_Barrier(MPI_COMM_FILTER);
#endif

    filter_log(FILTER_LOG, "Fparam.file_type %d\n", Fparam.file_type);
    filter_log(FILTER_LOG, "mpidiv_v         %d\n", mpidiv_v);
    /* サブボリューム集約ファイル形式 (Fparam.collect==1) */
    if (Fparam.file_type == 1 && mpidiv_s > 1) {
	/* ファイル数のループ */
	float *value;
	for (step_loop = Fparam.start_step; step_loop <= Fparam.end_step; step_loop++) {
	    for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
		if (mype != fnum_loop % numpe) {
		    continue;
		}
		//sprintf(outfile, "%s/%s_%07d_%07ld.tmp0", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
		//sprintf(infile, "%s/%s_%07d_%07ld-%05d.tmp1", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num, step_loop);
		sprintf(outfile, "%s/%s_%07d_%07lld.tmp0", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
		sprintf(infile, "%s/%s_%07d_%07lld-%05d.tmp1", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num, step_loop);
		fp = fopen(outfile, "ab");
		if (fp == NULL) {
		    filter_log(FILTER_ERR, "file open error. %s\n", outfile);
		    goto garbage;
		}
		ifp = fopen(infile, "rb");
		if (ifp == NULL) {
		    filter_log(FILTER_ERR, "file open error. %s\n", infile);
		    goto garbage;
		}
		/* サブボリュームのループ */
		for (volm_loop = m_vol_sta[fnum_loop]; volm_loop <= m_vol_end[fnum_loop]; volm_loop++) {
		    subvolume_nodes = m_nodes_per_subvolume[volm_loop];
		    value = (float *) malloc(sizeof (float) * subvolume_nodes * m_nkinds);
		    fread(value, sizeof (float), subvolume_nodes*m_nkinds, ifp);
		    fwrite(value, sizeof (float), subvolume_nodes*m_nkinds, fp);
		    free(value);
		    value = NULL;
		} /* サブボリュームのループ */
		fclose(ifp);
		fclose(fp);
		/* **.tmp1ファイル削除 */
		remove(infile);
	    }
	}
	for (fnum_loop = 0; fnum_loop < Fparam.out_num; fnum_loop++) {
	    if (mype != fnum_loop % numpe) {
		continue;
	    }
	    //sprintf(outfile, "%s/%s_%07d_%07ld.tmp0", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
	    //sprintf(infile, "%s/%s_%07d_%07ld.dat", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
		sprintf(outfile, "%s/%s_%07d_%07lld.tmp0", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
		sprintf(infile, "%s/%s_%07d_%07lld.dat", Fparam.out_dir, Fparam.out_prefix, fnum_loop + 1, Fparam.out_num);
		/* tmp0ファイルをvolファイルに */
	    rename(outfile, infile);
	}
    }

    /* メモリ開放 */
    initialize_memory(); // Frees pointers if they are null
    initialize_value(); // Sets variables to 0|NULL

    subvolume_elems = 0;
    subvolume_nodes = 0;
    subvolume_conns = 0;
    all_subvolume_nodes = NULL;
    all_subvolume_trans = NULL;
    all_subvolume_elems = NULL;
    /********************/
    DEBUG(3, ">>>>>>>>>>>> End of convert chain here!!!\n");
    // Loop back to unstruct_element_proc
    if (!Fparam.struct_grid && m_multi_element) {
	element_type_num++;
	goto unstruct_element_proc;
    }
unstruct_element_stop:
    /********************/
#ifdef MPI_EFFECT
    release :
	    MPI_Barrier(MPI_COMM_WORLD);
#endif
    initialize_memory();
    initialize_value();
    rcode = RTC_OK;
garbage:
    if (rcode == RTC_NG) {
#ifdef MPI_EFFECT
	MPI_Abort(MPI_COMM_WORLD, 99999);
#endif
    }
#ifdef MPI_EFFECT
    MPI_Finalize();
#endif
    return (0);
}



/*****************************************************************************/
/* 
 * フィルターメインプログラム
 * 
 */

/*****************************************************************************/

static int realloc_work(Int64 *prov_len, Int64 new_len) {
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
    *prov_len = new_len;
    return (RTC_OK);
}
#ifdef MPI_EFFECT

/*****************************************************************************/
static void bcast_param(int mype) {
    int i, j;
    Fdetail *fdc;
    Fdetail *fdv;
    Stepfile *sf;

    MPI_Bcast(&Fparam.in_dir, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.in_prefix, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.in_suffix, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.field_file, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.ucd_inp, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.geom_file, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.out_dir, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.out_prefix, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.out_suffix, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.format, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.tmp_dir, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.start_step, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.end_step, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.n_layer, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.component, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.elem_ave, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.out_num, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.file_type, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.nstep, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.ndim, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.dim1, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.dim2, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.dim3, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.nspace, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.veclen, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.data, 16, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.field, 16, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.coordfile, 512 * 3, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.coordtype, 512 * 3, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.coordskip, 3, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.coordstride, 3, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.valuefile, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.valuetype, 512, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.valueskip, 1, MPI_LONG, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.struct_grid, 1, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.explicit_n_layer, 1, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.nodesearch, 1, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.headfoot, 1, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.avsucd, 1, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.mult_element_type, 1, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.temp_delete, 1, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.mpi_volume_div, 1, MPI_INT, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.mpi_step_div, 1, MPI_INT, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.mpi_div, 1, MPI_CHAR, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.min_ext, 3, MPI_FLOAT, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.max_ext, 3, MPI_FLOAT, 0, m_mpi_comm);
    MPI_Bcast(&Fparam.blocksize, 1, MPI_LONG, 0, m_mpi_comm);
    if (Fparam.struct_grid) {
	if (mype > 0) {
	    sf = (Stepfile*) malloc(sizeof (Stepfile) * Fparam.nstep);
	    for (i = 0; i < Fparam.nstep; i++) {
		fdc = (Fdetail*) malloc(sizeof (Fdetail) * Fparam.ndim);
		fdv = (Fdetail*) malloc(sizeof (Fdetail) * Fparam.veclen);
		for (j = 0; j < Fparam.nspace; j++) {
		    memset(fdc[j].fname, 0x00, LINELEN_MAX);
		}
		for (j = 0; j < Fparam.veclen; j++) {
		    memset(fdv[j].fname, 0x00, LINELEN_MAX);
		}
		sf[i].coord = fdc;
		sf[i].value = fdv;
	    }
	    Fparam.stepf = sf;
	}
	
	if (Fparam.input_format== AVS_INPUT){
	    for (i = 0; i < Fparam.nstep; i++) {
		for (j = 0; j < Fparam.nspace; j++) {
		    MPI_Bcast(&Fparam.stepf[i].coord[j].fname, LINELEN_MAX, MPI_CHAR, 0, m_mpi_comm);
		    MPI_Bcast(&Fparam.stepf[i].coord[j].ftype, 16, MPI_CHAR, 0, m_mpi_comm);
		    MPI_Bcast(&Fparam.stepf[i].coord[j].skip, 1, MPI_INT, 0, m_mpi_comm);
		    MPI_Bcast(&Fparam.stepf[i].coord[j].offset, 1, MPI_INT, 0, m_mpi_comm);
		    MPI_Bcast(&Fparam.stepf[i].coord[j].stride, 1, MPI_LONG, 0, m_mpi_comm);
		    MPI_Bcast(&Fparam.stepf[i].coord[j].close, 1, MPI_CHAR, 0, m_mpi_comm);
		}
		for (j = 0; j < Fparam.veclen; j++) {
		    MPI_Bcast(&Fparam.stepf[i].value[j].fname, LINELEN_MAX, MPI_CHAR, 0, m_mpi_comm);
		    MPI_Bcast(&Fparam.stepf[i].value[j].ftype, 16, MPI_CHAR, 0, m_mpi_comm);
		    MPI_Bcast(&Fparam.stepf[i].value[j].skip, 1, MPI_INT, 0, m_mpi_comm);
		    MPI_Bcast(&Fparam.stepf[i].value[j].offset, 1, MPI_INT, 0, m_mpi_comm);
		    MPI_Bcast(&Fparam.stepf[i].value[j].stride, 1, MPI_LONG, 0, m_mpi_comm);
		    MPI_Bcast(&Fparam.stepf[i].value[j].close, 1, MPI_CHAR, 0, m_mpi_comm);
		}
	    }	    
	}

    }
}
#endif

static int separate_files() {
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

    filter_log(FILTER_LOG, "|||| read multi ucd binary data start.\n");
    /* 非構造格子                         */
    /* 節点情報・要素構成情報の読み込み */
    if (m_ucdbin) { // read the binary ucd file
	rstat = read_ucdbin_geom();
    } else {
	rstat = skip_ucd_ascii(1);
	rstat = read_ucd_ascii_geom();
	rstat = read_ucd_ascii_value(-1);
	fseek(m_ucd_ascii_fp, 0, SEEK_SET);
    }
    if (RTC_OK != rstat) {
	filter_log(FILTER_ERR, "read_ucd_ascii_geom error.\n");
	goto garbage;
    }
    filter_log(FILTER_LOG, "|||| read multi ucd binary data end.\n");
    if (Fparam.nodesearch) {
	if (RTC_OK != connect_sort()) {
	    filter_log(FILTER_ERR, "connect_sort error.\n");
	    goto garbage;
	}
	Fparam.nodesearch = (char) 0;
    }

    rank_elems = 0;
    for (i = 0; i < MAX_ELEMENT_TYPE; i++) {
	rank_elems += m_types[i];
    }
    filter_log(FILTER_LOG, "|||  rank_elems = %d\n", rank_elems);

    m_elems_per_subvolume = (unsigned int *) malloc(sizeof (int)*MAX_ELEMENT_TYPE);
    if (NULL == m_elems_per_subvolume) {
	filter_log(FILTER_ERR, "can not allocate memory(m_elems_per_subvolume).\n");
	goto garbage;
    }
    m_conns_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*MAX_ELEMENT_TYPE);
    if (NULL == m_conns_per_subvolume) {
	filter_log(FILTER_ERR, "can not allocate memory(m_conns_per_subvolume).\n");
	goto garbage;
    }
    m_nodes_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*MAX_ELEMENT_TYPE);
    if (NULL == m_nodes_per_subvolume) {
	filter_log(FILTER_ERR, "can not allocate memory(m_nodes_per_subvolume).\n");
	goto garbage;
    }

    prov_len = m_nnodes;
    prov_len = m_nnodes / MAX_ELEMENT_TYPE * 2;

    filter_log(FILTER_LOG, "|||  prov_len = %d\n", prov_len);

    m_subvolume_nodes = (int *) malloc(sizeof (int)*prov_len);
    if (NULL == m_subvolume_nodes) {
	filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_nodes).\n");
	goto garbage;
    }
    m_subvolume_trans = (int *) malloc(sizeof (int)*prov_len);
    if (NULL == m_subvolume_trans) {
	filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_trans).\n");
	goto garbage;
    }
    prov_len_e = rank_elems;
    filter_log(FILTER_LOG, "|||  Fparam.mult_element_type   = %d\n", Fparam.mult_element_type);
    filter_log(FILTER_LOG, "|||  prov_len_e = %d\n", prov_len_e);
    m_subvolume_elems = (int *) malloc(sizeof (int)*prov_len_e);
    if (NULL == m_subvolume_elems) {
	filter_log(FILTER_ERR, "can not allocate memory(m_subvolume_elems).\n");
	goto garbage;
    }
    m_nodes_trans = (int *) malloc(sizeof (int)*m_nnodes);
    if (NULL == m_nodes_trans) {
	filter_log(FILTER_ERR, "can not allocate memory(m_nodes_trans).\n");
	goto garbage;
    }
    m_nodeflag = (char *) malloc(sizeof (char)*m_nnodes);
    if (NULL == m_nodeflag) {
	filter_log(FILTER_ERR, "can not allocate memory(m_nodeflag).\n");
	goto garbage;
    }

    for (i = 0; i < MAX_ELEMENT_TYPE; i++) {
	//      m_elems_per_subvolume[i] = 0;
	m_conns_per_subvolume[i] = 0;
	m_nodes_per_subvolume[i] = 0;
    }
    for (i = 0; i < m_nnodes; i++) {
	m_nodeflag[i] = 0;
    }

    rank_nodes = 0;
    rank_elems = 0;
    filter_log(FILTER_LOG, "|||  count_in_elements start.\n");
    for (elem_loop = 0; elem_loop < MAX_ELEMENT_TYPE; elem_loop++) {
	/* サブボリューム内の要素・節点の判別 */
	if (RTC_OK != count_in_elements(elem_loop,
		&subvolume_elems,
		&subvolume_nodes,
		&subvolume_conns,
		&prov_len,
		rank_nodes,
		rank_elems)) {
	    filter_log(FILTER_ERR, "count_in_subvolume error.\n");
	    goto garbage;
	}
	filter_log(FILTER_LOG, "|||   elem_loop : %7d , subvolume_elems : %7d\n", elem_loop, subvolume_elems);
	m_elems_per_subvolume[elem_loop] = subvolume_elems;
	m_nodes_per_subvolume[elem_loop] = subvolume_nodes;
	m_conns_per_subvolume[elem_loop] = subvolume_conns;
	rank_nodes += m_nodes_per_subvolume[elem_loop];
	rank_elems += m_elems_per_subvolume[elem_loop];
    }
    filter_log(FILTER_LOG, "|||  count_in_elements ended.\n");

    m_nsteps = Fparam.end_step - Fparam.start_step + 1;
    m_value_max = (float *) malloc(sizeof (float)*m_nkinds * m_nsteps);
    if (NULL == m_value_max) {
	filter_log(FILTER_ERR, "can not allocate memory(m_value_min).\n");
	goto garbage;
    }
    m_value_min = (float *) malloc(sizeof (float)*m_nkinds * m_nsteps);
    if (NULL == m_value_min) {
	filter_log(FILTER_ERR, "can not allocate memory(m_value_min).\n");
	goto garbage;
    }
    mt_nulldata = (float *) malloc(sizeof (float)*m_nkinds * m_nsteps);
    if (NULL == mt_nulldata) {
	filter_log(FILTER_ERR, "can not allocate memory(mt_nulldata).\n");
	goto garbage;
    }
    mt_nullflag = (int *) malloc(sizeof (int)*m_nkinds * m_nsteps);
    if (NULL == mt_nullflag) {
	filter_log(FILTER_ERR, "can not allocate memory(mt_nullflag).\n");
	goto garbage;
    }
    for (i = 0; i < m_nkinds * m_nsteps; i++) {
	m_value_max[i] = 0.0;
	m_value_min[i] = 0.0;
    }
    m_values = (float *) malloc(sizeof (float)*m_nkinds * m_nnodes);
    if (NULL == m_values) {
	filter_log(FILTER_ERR, "can not allocate memory(values).\n");
	goto garbage;
    }
    if (m_nelemcomponents > 0) {
	m_elem_in_node = (int *) malloc(sizeof (int)*m_nnodes);
	m_elem_in_node_addr = (int *) malloc(sizeof (int)*m_nnodes);
	memset(m_elem_in_node, 0x00, sizeof (float)*m_nnodes);
	filter_log(FILTER_LOG, "|||  set_element_in_node start.\n");
	if (RTC_OK != set_element_in_node()) {
	    filter_log(FILTER_ERR, "set_element_in_node error.\n");
	    goto garbage;
	}
	filter_log(FILTER_LOG, "|||  set_element_in_node ended.\n");
    }
    for (step_loop = Fparam.start_step; step_loop <= Fparam.end_step; step_loop++) {
#ifdef MPI_EFFECT
	if (mype != (step_loop - Fparam.start_step) % numpe) {
	    continue;
	}
#endif
	filter_log(FILTER_LOG, "<><> time_step %d start.\n", step_loop);
	if (strlen(Fparam.ucd_inp) != 0) {
	    if (m_ucdbin == 0) {
		rstat = skip_ucd_ascii(step_loop);
		if (m_cycle_type == 1) {
		    rstat = skip_ucd_ascii_geom();
		} else if (m_cycle_type == 2) {
		    if (step_loop == Fparam.start_step) {
			rstat = skip_ucd_ascii_geom();
		    }
		}
		rstat = read_ucd_ascii_value(step_loop);
	    } else {
		rstat = read_ucdbin_data(step_loop);
	    }
	} else {
	    rstat = read_ucdbin_data(step_loop);
	}
	if (RTC_OK != rstat) {
	    filter_log(FILTER_ERR, "read_ucdbin_data error.\n");
	    goto garbage;
	}
	rank_nodes = 0;
	rank_elems = 0;
	for (elem_loop = 0; elem_loop < MAX_ELEMENT_TYPE; elem_loop++) {
	    subvolume_id = m_tree_node_no - m_nvolumes + elem_loop;
	    /* サブボリューム内の要素・節点の判別 */
	    subvolume_elems = m_elems_per_subvolume[elem_loop];
	    subvolume_nodes = m_nodes_per_subvolume[elem_loop];
	    subvolume_conns = m_conns_per_subvolume[elem_loop];
	    /* サブボリューム内の要素・節点の判別 */
	    if (RTC_OK != set_nodes_in_subvolume(subvolume_nodes,
		    rank_nodes)) {
		filter_log(FILTER_ERR, "set_node_in_subvolume error.\n");
		goto garbage;
	    }

	    if (m_types[elem_loop] != 0) {
		if (RTC_OK != write_ucdbin_elem(step_loop,
			elem_loop,
			subvolume_elems,
			subvolume_nodes,
			rank_nodes,
			rank_elems)) {
		    filter_log(FILTER_ERR, "write_ucdbin error.\n");
		    goto garbage;
		}
	    }
	    rank_nodes += m_nodes_per_subvolume[elem_loop];
	    rank_elems += m_elems_per_subvolume[elem_loop];
	    for (i = 0; i < m_nnodes; i++) {
		m_nodes_trans[i] = 0;
	    }
	}
	filter_log(FILTER_LOG, "<><> time_step %d ended.\n", step_loop);
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

    return (rstat);
}

/*****************************************************************************/

static int read_struct_grid(int numpe_f, int mype_f) {
    Int64 ii, jj, kk, id;
    int rstat = RTC_NG;

    if (!strstr(Fparam.field, "uniform")) {
	if (mype_f == 0) {
	    filter_log(FILTER_LOG, "|||| read geom data start.\n");
	    /* Octree生成のための節点情報読み込み */
	    if (Fparam.input_format == VTK_INPUT) {

#ifdef VTK_ENABLED
		rstat = vtk_read_structured_coordinates();
#endif
	    } else if (Fparam.input_format == PLOT3D_INPUT) {
		rstat = plot3d_getBlockCoordinates();
	    } else if (strlen(Fparam.coordfile[0]) > 0) {
		rstat = read_coord_1(0);
	    } else {
		rstat = read_geom();
	    }
	    filter_log(FILTER_LOG, "|||| read geom data end.\n");
	    if (RTC_OK != rstat) {
		filter_log(FILTER_ERR, "filter_binary error.\n");
		goto garbage;
	    }
	}
#ifdef MPI_EFFECT
	if (numpe_f > 1) {
	    /* 本来の座標値に更新 */
	    //          MPI_Bcast(m_coords,      m_nnodes*3,   MPI_FLOAT,0,MPI_COMM_FILTER);
	    for (ii = 0; ii < 3; ii++) {
		MPI_Bcast(&m_coords[ii * m_nnodes], m_nnodes, MPI_FLOAT, 0, MPI_COMM_FILTER);
	    }
	}
#endif
    } else {
	if (Fparam.min_ext[0] == 0.0 && Fparam.min_ext[1] == 0.0 && Fparam.min_ext[2] == 0.0 &&
		Fparam.max_ext[0] == 0.0 && Fparam.max_ext[1] == 0.0 && Fparam.max_ext[2] == 0.0) {
	    Fparam.min_ext[0] = 0.0;
	    Fparam.min_ext[1] = 0.0;
	    Fparam.min_ext[2] = 0.0;
	    Fparam.max_ext[0] = Fparam.dim1 - 1;
	    Fparam.max_ext[1] = Fparam.dim2 - 1;
	    Fparam.max_ext[2] = Fparam.dim3 - 1;
	}
	if (Fparam.ndim == 3) {
	    for (kk = 0; kk < Fparam.dim3; kk++) {
		for (jj = 0; jj < Fparam.dim2; jj++) {
		    for (ii = 0; ii < Fparam.dim1; ii++) {
			id = ii + (Fparam.dim1) * jj + (Fparam.dim1)*(Fparam.dim2) * kk;
			m_coords[id * 3 + 0] = Fparam.min_ext[0]+(Fparam.max_ext[0] - Fparam.min_ext[0]) / (Fparam.dim1 - 1) * ii;
			m_coords[id * 3 + 1] = Fparam.min_ext[1]+(Fparam.max_ext[1] - Fparam.min_ext[1]) / (Fparam.dim2 - 1) * jj;
			m_coords[id * 3 + 2] = Fparam.min_ext[2]+(Fparam.max_ext[2] - Fparam.min_ext[2]) / (Fparam.dim3 - 1) * kk;
		    }
		}
	    }
	} else if (Fparam.ndim == 2) {
	    for (jj = 0; jj < Fparam.dim2; jj++) {
		for (ii = 0; ii < Fparam.dim1; ii++) {
		    id = ii + (Fparam.dim1) * jj;
		    m_coords[id * 3 + 0] = Fparam.min_ext[0]+(Fparam.max_ext[0] - Fparam.min_ext[0]) / (Fparam.dim1 - 1) * ii;
		    m_coords[id * 3 + 1] = Fparam.min_ext[1]+(Fparam.max_ext[1] - Fparam.min_ext[1]) / (Fparam.dim2 - 1) * jj;
		    m_coords[id * 3 + 2] = 0;
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

static int connect_sort(void) {
	unsigned int div, mod;
	int ista, iend;
    int i, j;
    div = m_connectsize / numpe;
    mod = m_connectsize % numpe;
    if (mod > 0) div++;
    ista = mype * div;
    iend = ista + div;
    if (iend > m_connectsize) {
	iend = m_connectsize;
    }
    if ((iend - ista) > 0) {
	m_connections_o = m_connections;
	m_connections = (int *) malloc(sizeof (int)*m_connectsize);
	if (NULL == m_connections) {
	    filter_log(FILTER_ERR, "can not allocate memory(m_connections).\n");
	    return RTC_NG;
	}
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,j)
#endif
	for (i = 0; i < m_connectsize; i++) {
	    m_connections[i] = 0;
	}
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i,j)
#endif
	for (i = ista; i < iend; i++) {
	    for (j = 0; j < m_nnodes; j++) {
			if (m_connections_o[i] == m_ids[j]) {
				m_connections[i] = j + 1;
				break;
			}
	    }
	}
	free(m_connections_o);
	m_connections_o = NULL;
    }
#ifdef MPI_EFFECT
    MPI_Allreduce(MPI_IN_PLACE, m_connections, m_connectsize, MPI_INT, MPI_MAX, m_mpi_comm);
#endif
    return RTC_OK;
}

/*****************************************************************************/

static int initialize_memory(void) {
    DEBUG(3, "!!!!!! initialize_memory (Free if NULL) !!!!!!!\n");
    int i;
    /* メモリ開放 */
    if (NULL != m_geom_fp) free(m_geom_fp);
    if (NULL != m_value_fp) free(m_value_fp);
    if (NULL != m_octree) free(m_octree);
    if (NULL != m_tree) free(m_tree);
    if (NULL != m_ids) free(m_ids);
    if (NULL != m_coords) free(m_coords);
    if (NULL != m_connections) free(m_connections);
    if (NULL != m_connections_o) free(m_connections_o);
    if (NULL != m_conn_top_node) free(m_conn_top_node);
    if (NULL != m_materials) free(m_materials);
    if (NULL != m_elemtypes) free(m_elemtypes);
    if (NULL != m_values) free(m_values);
    if (NULL != mt_nullflag) free(mt_nullflag);
    if (NULL != mt_nulldata) free(mt_nulldata);
    if (NULL != m_value_max) free(m_value_max);
    if (NULL != m_value_min) free(m_value_min);
    if (NULL != m_veclens) free(m_veclens);
    if (NULL != m_nullflags) free(m_nullflags);
    if (NULL != m_nulldatas) free(m_nulldatas);
    if (NULL != m_subvolume_num) free(m_subvolume_num);
    if (NULL != m_nodeflag) free(m_nodeflag);
    if (NULL != m_nodes_trans) free(m_nodes_trans);
    if (NULL != m_elems_per_subvolume) free(m_elems_per_subvolume);
    if (NULL != m_nodes_per_subvolume) free(m_nodes_per_subvolume);
    if (NULL != m_conns_per_subvolume) free(m_conns_per_subvolume);
    if (NULL != m_elems_accumulate) free(m_elems_accumulate);
    if (NULL != m_nodes_accumulate) free(m_nodes_accumulate);
    if (NULL != m_conns_accumulate) free(m_conns_accumulate);
    if (NULL != m_elementids) free(m_elementids);
    if (NULL != m_subvolume_nodes) free(m_subvolume_nodes);
    if (NULL != m_elem_in_node_array) free(m_elem_in_node_array);
    if (NULL != m_elem_in_node_addr) free(m_elem_in_node_addr);
    if (NULL != m_elem_in_node) free(m_elem_in_node);
    if (pm_units != NULL) {
	for (i = 0; i < m_ncomponents; i++) {
	    free(pm_units[i]);
	}
	free(pm_units);
    }
    if (pm_names != NULL) {
	for (i = 0; i < m_ncomponents; i++) {
	    free(pm_names[i]);
	}
	free(pm_names);
    }
    if (m_subvolume_coord != NULL) free(m_subvolume_coord);
    if (m_subvolume_connect != NULL) free(m_subvolume_connect);
    if (m_subvolume_trans != NULL) free(m_subvolume_trans);
    if (m_subvolume_elems != NULL) free(m_subvolume_elems);

    return (RTC_OK);
}

/*****************************************************************************/

static int initialize_value(void) {
    DEBUG(3, "!!!!!!!!!!initialize_value (Set To NULL) !!!!!\n");
    m_tree_node_no = 0;
    m_stepn = 0;
    m_stept = 0.0f;
    m_nnodes = 0;
    m_nelements = 0;
    m_nnodekinds = 0;
    m_nelemkinds = 0;
    m_nkinds = 0;
    m_nnodecomponents = 0;
    m_nelemcomponents = 0;
    m_ncomponents = 0;
    m_elemcoms = 0;
    m_desctype = 0;
    m_datatype = 0;
    m_component_id = 0;
    m_nvalues_per_node = 0;
    m_ncomponents_per_node = 0;
    m_headfoot = 0;
    m_filesize = 0;
    /*
	subvolume_elems = 0;
	subvolume_nodes = 0;
	subvolume_conns = 0;
     */
    m_geom_fp = NULL;
    m_value_fp = NULL;
    m_octree = NULL;
    m_tree = NULL;
    m_ids = NULL;
    m_coords = NULL;
    m_connections = NULL;
    m_connections_o = NULL;
    m_conn_top_node = NULL;
    m_materials = NULL;
    m_elemtypes = NULL;
    m_values = NULL;
    m_value_max = NULL;
    m_value_min = NULL;
    m_veclens = NULL;
    m_nullflags = NULL;
    m_nulldatas = NULL;
    mt_nullflag = NULL;
    mt_nulldata = NULL;
    m_subvolume_num = NULL;
    m_nodeflag = NULL;
    m_nodes_trans = NULL;
    m_elems_per_subvolume = NULL;
    m_conns_per_subvolume = NULL;
    m_nodes_per_subvolume = NULL;
    m_elems_accumulate = NULL;
    m_conns_accumulate = NULL;
    m_nodes_accumulate = NULL;
    m_elementids = NULL;
    m_subvolume_nodes = NULL;
    m_elem_in_node_array = NULL;
    m_elem_in_node_addr = NULL;
    m_elem_in_node = NULL;
    pm_names = NULL;
    pm_units = NULL;
    m_subvolume_coord = NULL;
    m_subvolume_connect = NULL;
    m_subvolume_trans = NULL;
    m_subvolume_elems = NULL;
    /*
	all_subvolume_nodes = NULL;
	all_subvolume_trans = NULL;
	all_subvolume_elems = NULL;
     */
    return (RTC_OK);
}
