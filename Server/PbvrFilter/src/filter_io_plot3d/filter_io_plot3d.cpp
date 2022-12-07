#include <filter_io_plot3d/FilterIoPlot3d.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "FilterDivision.h"


namespace pbvr {
namespace filter {


//#ifdef WIN32
#if 1
const DataType FilterIoPlot3d::dataTypes[4] = {
    {
        4,
        sizeof (int),
        sizeof (int *),
        "%d", "%d "
    },
    {
        8,
        sizeof (Int64),
        sizeof (Int64 *),
        "%ld", "%ld "
    },
    {
        4,
        sizeof (float),
        sizeof (float *),
        "%f", "%f "
    },
    {
        8,
        sizeof (double),
        sizeof (double *),
        "%lf", "%lf "
    }
};
#else
const DataType FilterIoPlot3d::dataTypes[4] = {
    [INT_T] =
    {
        .onDiskSize = 4,
        .inMemSize = sizeof (int),
        .ptrSize = sizeof (int *),
        "%d", "%d "
    },
    [LONG_T] =
    {
        .onDiskSize = 8,
        .inMemSize = sizeof (Int64),
        .ptrSize = sizeof (Int64 *),
        "%ld", "%ld "
    },
    [FLOAT_T] =
    {
        .onDiskSize = 4,
        .inMemSize = sizeof (float),
        .ptrSize = sizeof (float *),
        "%f", "%f "
    },
    [DOUBLE_T] =
    {
        .onDiskSize = 8,
        .inMemSize = sizeof (double),
        .ptrSize = sizeof (double *),
        "%lf", "%lf "
    }
};
#endif

/**
 * Setup Fparam parameters and other variables needed by PBVR - convert chain
 * @return error state
 */
int FilterIoPlot3d::plot3d_setBlockParameters(void) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    NodeInf *min = &info->node_min;
    NodeInf *max = &info->node_max;

    int f_num_vars = 0;
    int s_num_vars = 0;
    // Set Fparam dimensions to current block dimensions
    param->dim1 = data_set->coordinate_blocks[info->current_block].gridDimension.i;
    param->dim2 = data_set->coordinate_blocks[info->current_block].gridDimension.j;
    param->dim3 = data_set->coordinate_blocks[info->current_block].gridDimension.k;
    // Calculate number of nodes, elements, and connection size.
    info->m_nnodes = param->dim1 * param->dim2 * param->dim3;
    info->m_nelements = (param->dim1 - 1)*(param->dim2 - 1)*(param->dim3 - 1);
    info->m_elemcoms = 8;
    info->m_element_type = 7;
    info->m_connectsize = info->m_nelements*info->m_elemcoms;
    //Calculate veclen
    if (sParams.function_params.enabled) {
        f_num_vars = data_set->function_blocks[info->current_block].num_components;
    }
    if (sParams.solution_params.enabled) {
        s_num_vars = sParams.solution_params.num_vars;
    }
    param->veclen = f_num_vars + s_num_vars;

    // A bit unsure about these
    info->m_ncomponents = param->veclen;
    info->m_nkinds = param->veclen;
    info->m_desctype = 1;
    info->m_datatype = 1;

    if (strstr(sParams.fieldType, "irregular")) {
        strcpy(param->field, "irregular");
        min->x = 0;
        min->y = 0;
        min->z = 0;

        max->x = param->dim1 - 1;
        max->y = param->dim2 - 1;
        max->z = param->dim3 - 1;

        param->min_ext[0] = data_set->xRange.min;
        param->min_ext[1] = data_set->yRange.min;
        param->min_ext[2] = data_set->zRange.min;
        param->max_ext[0] = data_set->xRange.max;
        param->max_ext[1] = data_set->yRange.max;
        param->max_ext[2] = data_set->zRange.max;
    } else {
        min->x = param->min_ext[0] = data_set->xRange.min;
        min->y = param->min_ext[1] = data_set->yRange.min;
        min->z = param->min_ext[2] = data_set->zRange.min;
        max->x = param->max_ext[0] = data_set->xRange.max;
        max->y = param->max_ext[1] = data_set->yRange.max;
        max->z = param->max_ext[2] = data_set->zRange.max;
    }
    DEBUG(3,"Setting Fparams for block: %i \n", info->current_block);
    DEBUG(3," dim1:%ld dim2:%ld dim3:%ld,  vec-len:%ld, m_nelements:%ld, m_nnodes:%ld\n", param->dim1, param->dim2, param->dim3, param->veclen, info->m_nelements, info->m_nnodes);
    return RTC_OK;
}

/**
 * Read the plot3d configuration and do some basic initialization
 * @return
 */
int FilterIoPlot3d::read_plot3d_config(void) {
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;

    int err_state = RTC_OK;
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
int FilterIoPlot3d::plot3d_readGridCoordinateFile() {
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
int FilterIoPlot3d::plot3d_initOffsetValueArray() {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    FilterDivision *division = factoryFilterDivision(param->division_type, info);

    int v, i;

    /**  modify by @hira at 2016/12/01
    info->m_nvolumes = 1;
    for (i = 0; i < param->n_layer; i++) {
        info->m_nvolumes = info->m_nvolumes * 8;
    }
    ***/
    info->m_nvolumes = division->get_nvolumes();

    info->offset_values = (Int64*)malloc(info->m_nvolumes * sizeof (Int64));
    DEBUG(3," Initializing %i offset_values[] for %ld layers to zero\n", info->m_nvolumes, param->n_layer);
    for (v = 0; v < info->m_nvolumes; v++) {
        info->offset_values[v] = 0;
    }

    delete division;

    return RTC_OK;
}


/**
 * Copy the current block of coordinate data into m_coords[]
 *
 * @return err_state
 */
int FilterIoPlot3d::plot3d_getBlockCoordinates() {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    NodeInf *min = &info->node_min;
    NodeInf *max = &info->node_max;

    int err_state = RTC_OK;
    DEBUG(3,"plot3d_multi_readCoord for block %i\n", info->current_block);
    printf("c");
    fflush(stdout);
    plot3d_dataSet_getBlockCoordinates(data_set, info->m_coords, info->current_block);
    min->x = data_set->xRange.min;
    min->y = data_set->yRange.min;
    min->z = data_set->zRange.min;
    max->x = data_set->xRange.max;
    max->y = data_set->yRange.max;
    max->z = data_set->zRange.max;
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
int FilterIoPlot3d::plot3d_getBlockValuesForTstep(int step) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    NodeInf *min = &info->node_min;
    NodeInf *max = &info->node_max;

    int err_state = RTC_OK;
    int i, j;
    LOGOUT(FILTER_LOG, (char*)"## plot3d_getBlockValuesForTstep for block: %06d, step:%06d\n", info->current_block, step);
    printf("v");
    fflush(stdout);

    min->x = data_set->xRange.min;
    min->y = data_set->yRange.min;
    min->z = data_set->zRange.min;
    max->x = data_set->xRange.max;
    max->y = data_set->yRange.max;
    max->z = data_set->zRange.max;
    err_state |= plot3d_dataSet_readStepData(data_set, info->m_values, step, info->current_block);
    ASSERT_RTC_OK(err_state, "Failed to read step data");
    DEBUG(3,"Waiting for filter_convert...\n");
    printf("..");
    fflush(stdout);
    return err_state;
}

} /* namespace filter */
} /* namespace pbvr */


