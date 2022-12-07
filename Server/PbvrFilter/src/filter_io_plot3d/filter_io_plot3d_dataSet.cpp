#include <filter_io_plot3d/FilterIoPlot3d.h>
#include <stdlib.h>


namespace pbvr {
namespace filter {

/**
 * Allocate a new Plot3Ddataset* data set
 * @param err_state
 * @param params
 * @return
 */
Plot3Ddataset* FilterIoPlot3d::plot3d_dataSet_allocateNew(int* err_state, plot3d_params* params) {

    if (this->m_filter_info == NULL) return NULL;
    if (this->m_filter_info->m_param == NULL) return NULL;
    FilterParam *param = this->m_filter_info->m_param;

    // ref_malloc(data_set, sizeof (Plot3Ddataset));
    Plot3Ddataset* data_set = (Plot3Ddataset*)malloc(sizeof (Plot3Ddataset));

    if (data_set == NULL) {
        *err_state |= RTC_NG;
    }
    data_set->params = params;
    data_set->nSteps = (param->end_step - param->start_step) + 1;
    data_set->coordinate_blocks = NULL;
    data_set->function_blocks = NULL;
    data_set->solution_blocks = NULL;
    data_set->coordinate_offsets = NULL;
    data_set->function_offsets = NULL;
    data_set->solution_offsets = NULL;
    data_set->timeSteps = NULL;
    data_set->hasIblanks = params->coordinate_params.has_iblanks;
    // ref_malloc(data_set->timeSteps, (sizeof (timeStep) * data_set->nSteps));
    data_set->timeSteps = (timeStep*)malloc(sizeof (timeStep) * data_set->nSteps);
    return data_set;
}

/**
 * Index solution and function step files
 * @param data_set
 */
void FilterIoPlot3d::plot3d_dataSet_indexStepFiles(Plot3Ddataset* data_set) {

    if (data_set->params->solution_params.enabled) {
        plot3d_dataSetS_readBlockCounts(data_set);
        plot3d_dataSetS_indexBlocks(data_set);
    }
    if (data_set->params->function_params.enabled) {
        plot3d_dataSetF_readBlockCounts(data_set);
        plot3d_dataSetF_indexBlocks(data_set);
    }
}

/**
 * Not used any more
 * @param data_set
 */
void FilterIoPlot3d::plot3d_dataSet_closeAllFiles(Plot3Ddataset* data_set) {
    DEBUG(3,"CLOSING ALL FILES !!!\n");
}

/**
 * Get the block coordinates for a specified block from cache.
 * @param data_set Plot3Ddataset*
 * @param target float* float array to copy data to
 * @param step	int
 * @param block int
 * @return err_status int
 */
int FilterIoPlot3d::plot3d_dataSet_getBlockCoordinates(Plot3Ddataset* data_set, float* target, int block) {
    Plot3DCoordinateBlock* cGrid;
    cGrid = &(data_set->coordinate_blocks[block]);
    DataArray_copyAsFloat(&(cGrid->coordinates), target, 0, 0);
    return RTC_OK;
}

/**
 * Read solution and function data for a select step and block
 * @param data_set Plot3Ddataset*
 * @param target float* float array to copy data to
 * @param step	int
 * @param block int
 * @return err_status int
 */
int FilterIoPlot3d::plot3d_dataSet_readStepData(Plot3Ddataset* data_set, float* target, int step, int block) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;

    int skip = 0; // Number of function values
    int offset = 0; // Number of Qvalues
    int step_index = step - param->start_step;
    // Find offset and stride
    if (data_set->params->function_params.enabled) {
        skip = data_set->function_blocks[block].num_components;
    }
    if (data_set->params->solution_params.enabled) {
        offset = data_set->params->solution_params.num_vars; // Defaults to 5
    }
    // Read function and solution blocks
    if (data_set->params->function_params.enabled) {
        plot3d_dataSetF_readCopyFunctionBlock(data_set, target, step_index, block, offset);
    }
    if (data_set->params->solution_params.enabled) {
        plot3d_dataSetS_readCopySolutionBlock(data_set, target, step_index, block, skip);
    }
    return RTC_OK;
}

void FilterIoPlot3d::plot3d_dataSet_deallocate(Plot3Ddataset* data_set) {

    if (this->m_filter_info == NULL) return;
    if (this->m_filter_info->m_param == NULL) return;
    PbvrFilterInfo *info = this->m_filter_info;

    plot3d_dataSet_closeAllFiles(data_set);
    plot3d_dataSetG_deAllocateMemory(data_set);
    plot3d_dataSetF_deAllocateMemory(data_set);
    plot3d_dataSetS_deAllocateMemory(data_set);
    if (data_set != NULL) {
        ref_free(data_set->timeSteps);
        ref_free(data_set);
        ref_free(info->offset_values);
    }
}

} /* namespace filter */
} /* namespace pbvr */
