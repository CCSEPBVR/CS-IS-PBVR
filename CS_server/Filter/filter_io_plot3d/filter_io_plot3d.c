#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "filter_io_plot3d_config.h"
#include "filter_io_plot3d_dataSet.h"
#include "filter_io_plot3d.h"
#include "filter_io_plot3d_solutionBlock.h"
#include "filter_io_plot3d_coordinateBlock.h"
#include "filter_io_plot3d_functionBlock.h"
#include "filter_io_types.h"

#include "../filter.h"
#include "../filter_convert.h"
#include "../filter_utils.h"
#include "../filter_log.h"

const bool irregular = true;
Plot3Ddataset* data_set;

/**
 * Setup Fparam parameters and other variables needed by PBVR - convert chain 
 * @return error state
 */
int plot3d_setBlockParameters(void) {
    int f_num_vars = 0;
    int s_num_vars = 0;
    // Set Fparam dimensions to current block dimensions
    Fparam.dim1 = data_set->coordinate_blocks[current_block].gridDimension.i;
    Fparam.dim2 = data_set->coordinate_blocks[current_block].gridDimension.j;
    Fparam.dim3 = data_set->coordinate_blocks[current_block].gridDimension.k;
    // Calculate number of nodes, elements, and connection size.
    m_nnodes = Fparam.dim1 * Fparam.dim2 * Fparam.dim3;
    m_nelements = (Fparam.dim1 - 1)*(Fparam.dim2 - 1)*(Fparam.dim3 - 1);
    m_elemcoms = 8;
    m_element_type = 7;
    m_connectsize = m_nelements*m_elemcoms;
    //Calculate veclen
    if (sParams.function_params.enabled) {
        f_num_vars = data_set->function_blocks[current_block].num_components;
    }
    if (sParams.solution_params.enabled) {
        s_num_vars = sParams.solution_params.num_vars;
    }
    Fparam.veclen = f_num_vars + s_num_vars;

    // A bit unsure about these
    m_ncomponents = Fparam.veclen;
    m_nkinds = Fparam.veclen;
    m_desctype = 1;
    m_datatype = 1;

    if (strstr(sParams.fieldType, "irregular")) {
        strcpy(Fparam.field, "irregular");
        min.x = 0;
        min.y = 0;
        min.z = 0;

        max.x = Fparam.dim1 - 1;
        max.y = Fparam.dim2 - 1;
        max.z = Fparam.dim3 - 1;

        Fparam.min_ext[0] = data_set->xRange.min;
        Fparam.min_ext[1] = data_set->yRange.min;
        Fparam.min_ext[2] = data_set->zRange.min;
        Fparam.max_ext[0] = data_set->xRange.max;
        Fparam.max_ext[1] = data_set->yRange.max;
        Fparam.max_ext[2] = data_set->zRange.max;
    } else {
        min.x = Fparam.min_ext[0] = data_set->xRange.min;
        min.y = Fparam.min_ext[1] = data_set->yRange.min;
        min.z = Fparam.min_ext[2] = data_set->zRange.min;
        max.x = Fparam.max_ext[0] = data_set->xRange.max;
        max.y = Fparam.max_ext[1] = data_set->yRange.max;
        max.z = Fparam.max_ext[2] = data_set->zRange.max;
    }
    DEBUG(3,"Setting Fparams for block: %i \n", current_block);
    DEBUG(3," dim1:%ld dim2:%ld dim3:%ld,  vec-len:%ld, m_nelements:%ld, m_nnodes:%ld\n", Fparam.dim1, Fparam.dim2, Fparam.dim3, Fparam.veclen, m_nelements, m_nnodes);
    return RTC_OK;
}

/**
 * Read the plot3d configuration and do some basic initialization
 * @return 
 */
int read_plot3d_config(void) {
    int err_state = RTC_OK;
    FilterParam *param;
    param = &Fparam;
    param->ndim = 3;
    param->struct_grid = 1;
    param->ndim = 1;
    param->dim1 = 1;
    param->dim2 = 1;
    param->dim3 = 1;
    param->nspace = 1;
    param->ndim = 3;
    plot3d_config_read();
    return (err_state);
}

/**
 * Reads the coordinate file and caches all blocks 
 * If only one time step is selected, the solution and function files
 * are also read and all blocks cached.
 * If multiple time steps, the files will be read on demand instead.
 * @return 
 */
int plot3d_readGridCoordinateFile() {
    int err_state = RTC_OK;
    if (sParams.coordinate_params.enabled == false) {
        ASSERT_FAIL("Cannot work without a coordinate data file");
    }
    data_set = plot3d_dataSet_allocateNew(&err_state, &sParams);
    plot3d_dataSetG_read3DMultiBlock(data_set);
    plot3d_dataSetG_findGridDimensions(data_set);
    plot3d_dataSet_indexStepFiles(data_set);
    return err_state;
}

/**
 * Initialize offset value array. 
 * The offset_values array keeps an offset for each sub_volume which is used
 * during output when appending subsequent block output to the sub volume data.
 * @return status 
 */
int plot3d_initOffsetValueArray() {
    int v, i;
    m_nvolumes = 1;
    for (i = 0; i < Fparam.n_layer; i++) {
        m_nvolumes = m_nvolumes * 8;
    }
    ref_malloc(offset_values, m_nvolumes * sizeof (Int64));
    DEBUG(3," Initializing %i offset_values[] for %ld layers to zero\n", m_nvolumes, Fparam.n_layer);
    for (v = 0; v < m_nvolumes; v++) {
        offset_values[v] = 0;
    }
    return RTC_OK;
}

/**
 * This is the entry point from the main() process.
 * @param argc
 * @param argv
 * @return 
 */
int plot3d_sequentialConvert(int argc, char *argv[]) {
    allocationList_create(2000);
    printf("Initializing\n");
    fflush(stdout);
    ASSERT_RTC_OK(read_plot3d_config(), "Could not read plot3d config.");
    plot3d_initOffsetValueArray();
    printf("Reading Grid Coordinates\n");
    fflush(stdout);
    plot3d_readGridCoordinateFile();
    fflush(stdout);
    number_of_blocks = data_set->nBlocks;
    for (current_block = 0; current_block < data_set->nBlocks; current_block++) {
        printf("\r%80s\r", " ");
        printf("Converting block %i of %i => ", current_block+1, data_set->nBlocks);
        filter_convert(argc, argv);
    }
    plot3d_dataSet_deallocate(data_set);
    allocationList_destruct();
    return RTC_OK;
}

/**
 * Copy the current block of coordinate data into m_coords[]
 * 
 * @return err_state
 */
int plot3d_getBlockCoordinates() {
    int err_state = RTC_OK;
    DEBUG(3,"plot3d_multi_readCoord for block %i\n", current_block);
    printf("c");
    fflush(stdout);
    plot3d_dataSet_getBlockCoordinates(data_set, m_coords, current_block);
    min.x = data_set->xRange.min;
    min.y = data_set->yRange.min;
    min.z = data_set->zRange.min;
    max.x = data_set->xRange.max;
    max.y = data_set->yRange.max;
    max.z = data_set->zRange.max;
    DEBUG(3,"Waiting for filter_convert...\n");
    printf("..");
    fflush(stdout);
    return (err_state);
}

/**
 * Copy the current block of function and solution data into m_values[]
 * @param step int The time step to read
 * @return 
 */
int plot3d_getBlockValuesForTstep(int step) {
    int err_state = RTC_OK;
    int i, j;
    filter_log(FILTER_LOG, "## plot3d_getBlockValuesForTstep for block: %06d, step:%06d\n", current_block, step);
    printf("v");
    fflush(stdout);
       min.x = data_set->xRange.min;
    min.y = data_set->yRange.min;
    min.z = data_set->zRange.min;
    max.x = data_set->xRange.max;
    max.y = data_set->yRange.max;
    max.z = data_set->zRange.max;
    err_state |= plot3d_dataSet_readStepData(data_set, m_values, step, current_block);
    ASSERT_RTC_OK(err_state, "Failed to read step data");
    DEBUG(3,"Waiting for filter_convert...\n");
    printf("..");
    fflush(stdout);
    return err_state;
}
