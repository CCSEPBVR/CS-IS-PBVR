#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>


#include <float.h>
#include "filter_log.h"
#include "filter.h"
//#include "filter_io_avs/filter_io_avs.h"
#include "filter_convert.h"
//#include "filter_io_stl/filter_io_stl.h"

#include "filter_io_plot3d/filter_io_plot3d.h"
#include "filter_io_stl/filter_io_stl.h"
#include "filter_io_vtk/filter_io_vtk.h"
#ifdef _OPENMP
#include "omp.h"
#endif

/*****************************************************************************/
/* 
 * フィルターメインプログラム
 * Filter main program
 */


int main(int argc, char *argv[]) {
    int status = RTC_NG;
    char* param_filepath;
    current_block = 0;
    /* 引数チェック */
    if (2 != argc) {
	fprintf(stderr, "usage:%s [paramfile].\n", argv[0]);
	return (-1);
    }

    param_filepath = argv[1];

    if (RTC_OK != read_param_file(param_filepath)) {
	filter_log(FILTER_ERR, "read_filter_param error.\n");
	return -1;
    }
    ENDIAN = check_endian();
    filter_log(FILTER_LOG, "#### %s endian\n", (ENDIAN ? "Big" : "Little"));

    DEBUG(3, "File Input Format:%i \n", Fparam.input_format);

    switch (Fparam.input_format) {

	case AVS_INPUT:
	    // EXISTING FORMAT 
	    m_elementsize = ucd_elementsize;
	    status = filter_convert(argc, argv);
	    break;
	case STL_ASCII:
	    // NEW FORMAT, PARSING OK BUT 
	    //status = stl_ascii_read_geom();
	    //Fparam.input_format = STL_INPUT;
	    status = filter_convert(argc, argv);
	    break;
	case STL_BIN:
	    // NEW FORMAT, PARSING OK
	    //status = stl_bin_read_geom();
	    //Fparam.input_format = STL_INPUT;
	    status = filter_convert(argc, argv);
	    break;

	case VTK_INPUT:
#ifdef VTK_ENABLED     
	    m_elementsize = vtk_elementsize;
	    status = vtk_get_datasetInfo();
	    status |= filter_convert(argc, argv);
#else
	    ASSERT_FAIL("VTK Not enabled in this build\n");
#endif
	    break;

	case PLOT3D_INPUT:
	    //	    status = io_plot3d_multi();
	    status = plot3d_sequentialConvert(argc, argv);

	    //	    status = filter_convert(argc,argv);
	    break;
	default:
	    filter_log(FILTER_LOG, "WARNING!  Unknown input type: %s ", Fparam.input_format);
	    status = filter_convert(argc, argv);

    }
    return status;
}



