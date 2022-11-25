#include <filter_io_plot3d/FilterIoPlot3d.h>
#include <stdlib.h>


namespace pbvr {
namespace filter {

/**
 * plot3d_gridFile_openStepFile, Opens a grid data file and stores
 * the file handle in grid->pDataFile
 * @param data_set Plot3DGridFile* instance to work on
 * @param filepath
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetF_openStepFile(Plot3Ddataset* data_set, int step) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;

    int status = RTC_OK;
    char filePath[FILELEN_MAX];
    ReadMode rMode = data_set->params->function_params.readmode;
    if (param->end_step + param->start_step > 0) {
        DEBUG(3,"step:%d start_step:%ld end_step:%ld\n", step, param->start_step, param->end_step);
        sprintf(filePath, "%s/%s%06lld.%s", param->in_dir, data_set->params->function_params.file_prefix, step + param->start_step, data_set->params->function_params.file_suffix);
    } else {
        sprintf(filePath, "%s/%s.%s", param->in_dir, data_set->params->function_params.file_prefix, data_set->params->function_params.file_suffix);
    }
    DEBUG(3," Trying to open step %i Function file: %s\n", step, filePath);
    timeStep* tStep = &(data_set->timeSteps[step]);
    GridFile* gridFile = &(tStep->function_file);
    status |= filter_io_open_DataFile(gridFile, filePath, rMode);
    gridFile->needsByteSwap = (check_endian() != data_set->params->function_params.endian);
    gridFile->blockDataTypeId = data_set->params->function_params.precision;
    gridFile->usebytecount = data_set->params->function_params.use_bytecount;
    return status;
}

/**
 * plot3d_gridFile_openStepFiles, Opens the data files of all steps
 * the file handle in grid->pDataFile
 * @param data_set Plot3DGridFile* instance to work on
 * @param filepath
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetF_openStepFiles(Plot3Ddataset* data_set) {
    int step;
    int status = RTC_OK;
    for (step = 0; step < data_set->nSteps; step++) {
        status |= plot3d_dataSetF_openStepFile(data_set, step);
    }
    ASSERT_RTC_OK(status, "Failed to open step file");
    return status;
}

/**
 * Close a step data file
 * @param data_set Plot3DGridFile* instance to work on
 * @param filepath
 */
void FilterIoPlot3d::plot3d_dataSetF_closeStepFile(Plot3Ddataset* data_set, int step) {
    timeStep* tStep = &(data_set->timeSteps[step]);
    GridFile* gridFile = &(tStep->function_file);
    filter_io_close_dataFile(gridFile);
}

/**
 * Close the grid data file
 * @param data_set Plot3DGridFile* instance to work on
 * @param filepath
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetF_closeStepFiles(Plot3Ddataset* data_set) {
    int step;
    for (step = 0; step < data_set->nSteps; step++) {
        plot3d_dataSetF_closeStepFile(data_set, step);
    }
    return RTC_OK;
}

/**
 * Allocate grid memory
 * @param data_set Plot3DGridFile* instance to work on
 * @param nGrids
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetF_allocateBlockTypes(Plot3Ddataset* data_set, int nGrids) {
    int n;
    // ref_malloc(data_set->function_blocks, (sizeof (Plot3DFunctionBlock) * nGrids));
    data_set->function_blocks = (Plot3DFunctionBlock*)malloc(sizeof (Plot3DFunctionBlock) * nGrids);
    // add by @hira at 2016/12/01
    memset(data_set->function_blocks, 0x00, sizeof (Plot3DFunctionBlock) * nGrids);

    for (n = 0; n < nGrids; n++) {
        data_set->function_blocks[n].allocatedValues = false;
        data_set->function_blocks[n].num_components = 0;
    }
    ASSERT_ALLOC(data_set->function_blocks);
    DEBUG(3,"Allocated %i grids of type FUNCTION\n", nGrids);
    data_set->nBlocks = nGrids;
    DEBUG(3,"plot3d_functions_allocateGrids size= %lu  \n", (unsigned Int64) sizeof (Plot3DFunctionBlock) * nGrids);
    return 0;
}

/**
 * Read the grid count from the data file
 * @param data_set Plot3DGridFile* instance to work on
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetF_readBlockCounts(Plot3Ddataset* data_set) {
    DEBUG(3,"plot3d_dataSetF_readBlockCounts  \n");
    int status = RTC_OK;
    int nBlocks = 0;
    int step;
    if (data_set->params->function_params.use_blockcount) {

        // Verify that block count matches
        for (step = 0; step < data_set->nSteps; step++) {
            status |= plot3d_dataSetF_openStepFile(data_set, step);
            ASSERT_RTC_OK(status, "Failed to open function step data file");
            timeStep* tStep = &(data_set->timeSteps[step]);
            GridFile* gFile = &(tStep->function_file);
            filter_io_open_FortranBlock(gFile);
            status |= filter_io_readObject(gFile, &(nBlocks), INT_T);
            DEBUG(3,"plot3d_dataSetF_readBlockCounts %i \n", nBlocks);
            if (nBlocks != data_set->nBlocks) {
                LOGOUT(FILTER_ERR, (char*)"ERROR: Number of blocks not matching: %i != %i \n", data_set->nBlocks, nBlocks);
                return RTC_NG;
            }
            filter_io_close_FortranBlock(gFile);
            plot3d_dataSetF_closeStepFile(data_set, step);
        }
        DEBUG(3,"Number of F blocks matches in all step files: %i  \n", data_set->nBlocks);
    } else {
        // Just use block count from coordinate file
        nBlocks = data_set->nBlocks;
    }
    plot3d_dataSetF_allocateBlockTypes(data_set, nBlocks);
    ASSERT_RTC_OK(status, "Failed to read function file block counts");
    return status;
}

/**
 * Dump all grids in grid file to dump file
 * @param data_set Plot3DGridFile* instance to work on
 * @param filePath the dump file path
 */
void FilterIoPlot3d::plot3d_dataSetF_dumpBlocks(Plot3Ddataset *data_set, const char* filePath) {
    int n;
    printf("\n==========    Dumping Grid File       ==========\n");
    FILE* file = fopen(filePath, "w");
    fprintf(file, "## Dumping %i Grids with a total of %i functions", data_set->nBlocks, data_set->ncoords);
    for (n = 0; n < data_set->nBlocks; n++) {
        fprintf(file, "\n## Grid number %i", n);
        plot3d_functionBlock_dump(&(data_set->function_blocks[n]), file);
    }
    fclose(file);
}

/**
 * Read the grid dimensions for each grid in the file
 * @param data_set Plot3DGridFile* instance to work on
 * @return error code as integer
 */
int FilterIoPlot3d::plot3d_dataSetF_indexBlocks(Plot3Ddataset* data_set) {
    int m;
    int dim4[4];
    Plot3DFunctionBlock* fGrid;
    int ncoords;
    int nBlocks;
    Int64 offset;
    int byteHeader = 0;
    int size = 0;

    plot3d_dataSetF_openStepFile(data_set, 0);
    timeStep* tStep = &(data_set->timeSteps[0]);
    GridFile* gFile = &(tStep->function_file);

    filter_io_open_FortranBlock(gFile);
    filter_io_readObject(gFile, &(nBlocks), INT_T);
    filter_io_close_FortranBlock(gFile);

    filter_io_open_FortranBlock(gFile);

    DEBUG(3,"-----------------------------------------------------\n");
    DEBUG(3,"##### Indexing Function blocks 0-%i \n", data_set->nBlocks);
    ncoords = 0;
    // ref_malloc(data_set->function_offsets, (sizeof (Int64)* data_set->nBlocks));
    data_set->function_offsets = (Int64*)malloc(sizeof (Int64)* data_set->nBlocks);

    int data_type_size = dataTypes[gFile->blockDataTypeId].onDiskSize;
    if (data_set->params->function_params.use_bytecount) {
        byteHeader = 4;
    }
    offset = 4 + 2 * byteHeader;
    offset += (data_set->nBlocks * 4 * 4) + 2 * byteHeader;
    for (m = 0; m < data_set->nBlocks; m++) {
        DEBUG(3,"FO %i => %ld\n", m, offset);
        data_set->function_offsets[m] = offset;
        fGrid = &(data_set->function_blocks[m]);
        filter_io_readBuffer(gFile, dim4, 4, INT_T);
        plot3d_functionBlock_setDimension(fGrid, dim4[0], dim4[1], dim4[2], dim4[3]);
        ncoords += fGrid->ncoords;
        size = (fGrid->ncoords * fGrid->num_components * data_type_size) + (2 * byteHeader);
        offset = offset + size;
    }

    filter_io_close_FortranBlock(gFile);
    plot3d_dataSetF_closeStepFile(data_set, 0);
    DEBUG(3,"total grid coords = %i. (Est file size:%i KB)  \n", data_set->ncoords, (data_set->ncoords * 4 * 4) / 1024);
    return RTC_OK;
}

/**
 * Read the grid data from all grids in the file
 * @param data_set Plot3DGridFile* instance to work on
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetF_readBlockData(Plot3Ddataset * data_set, int step) {
    int m;
    int err_state = RTC_OK;
    Plot3DFunctionBlock* fGrid;
    timeStep tStep = data_set->timeSteps[step];
    GridFile* gFile = &(tStep.function_file);
    DEBUG(3,"----------------------------------------- \n");
    LOGOUT(FILTER_LOG, (char*)"#### Reading function blocks 0 -%i\n", data_set->nBlocks);
    for (m = 0; m < data_set->nBlocks; m++) {
        fGrid = &(data_set->function_blocks[m]);
        DEBUG(3,"Reading %ld function values for  block %i \n", fGrid->ncoords, m);
        err_state |= plot3d_functionBlock_readWhole(fGrid, gFile);
        ASSERT_RTC_OK(err_state, "Failed to read function block");
    }
    DEBUG(3,"\n plot3d_gridFile_readGridData complete\n");
    return err_state;
}

/**
 * Deallocate array memory for all grids
 * @param data_set Plot3DGridFile* instance to work on
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetF_deAllocateMemory(Plot3Ddataset * data_set) {
    int m;
    if (data_set == NULL) return RTC_OK;

    Plot3DFunctionBlock* fGrid;
    if (data_set->function_blocks != NULL) {
        DEBUG(3,"#### Deallocating memory for function blocks: 0 -%i\n", data_set->nBlocks);
        for (m = 0; m < data_set->nBlocks; m++) {
            fGrid = &(data_set->function_blocks[m]);
            plot3d_functionBlock_deAllocateMemory(fGrid);
        }
        ref_free(data_set->function_blocks);
        ref_free(data_set->function_offsets);
    }
    return RTC_OK;
}

int FilterIoPlot3d::plot3d_dataSetF_skipToBlock(Plot3Ddataset* data_set, int block, int step) {
    if (data_set == NULL) return RTC_OK;

    timeStep* tStep = &(data_set->timeSteps[step]);
    GridFile* gFile = &(tStep->function_file);
    filter_io_skipToOffset(gFile, data_set->function_offsets[block]);
    return RTC_OK;
}

/**
 * Example
 * Read a multi block 3DWhole with multiple structured grid with IBlanks
 * @param data_set Plot3DGridFile* instance to work on
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetF_read3DSingleBlock(Plot3Ddataset* data_set, int block, int step) {
    int err_state = RTC_OK;
    err_state |= plot3d_dataSetF_openStepFiles(data_set);
    ASSERT_RTC_OK(err_state, "Failed to open function file");
    err_state |= plot3d_dataSetF_readBlockCounts(data_set);
    err_state |= plot3d_dataSetF_indexBlocks(data_set);
    ASSERT_RTC_OK(err_state, "Failed to index function blocks");
    plot3d_dataSetF_skipToBlock(data_set, block, step);
    err_state |= plot3d_dataSetF_readBlockData(data_set, step);
    plot3d_dataSetF_closeStepFiles(data_set);
    return err_state;
}

/**
 * Example
 * Read a multi block 3DWhole with multiple structured grid with IBlanks
 * @param data_set Plot3DGridFile* instance to work on
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetF_read3DMultiBlock(Plot3Ddataset* data_set, int step) {
    LOGOUT(FILTER_LOG, (char*)"=====================================================\n");
    LOGOUT(FILTER_LOG, (char*)"==========       Reading Grid File     F   ==========\n");
    LOGOUT(FILTER_LOG, (char*)"=====================================================\n");
    int err_state = RTC_OK;
    plot3d_dataSetF_openStepFile(data_set, step);
    err_state |= plot3d_dataSetF_openStepFiles(data_set);
    ASSERT_RTC_OK(err_state, "Failed to open function step file");
    err_state |= plot3d_dataSetF_readBlockCounts(data_set);
    err_state |= plot3d_dataSetF_indexBlocks(data_set);
    ASSERT_RTC_OK(err_state, "Failed to index function blocks");
    err_state |= plot3d_dataSetF_readBlockData(data_set, step);
    plot3d_dataSetF_closeStepFile(data_set, step);
    return err_state;
}

/**
 * Write all grids to output array
 * @param data_set Plot3DGridFile* instance to work on
 * @param step
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetF_copyToArray(Plot3Ddataset* data_set, int step, int offset, int skip) {
    int m;
    Plot3DFunctionBlock* fGrid;
    LOGOUT(FILTER_LOG, (char*)"##### Writing grid data %d of  into target array\n", step, data_set->nBlocks);
    DEBUG(3,"----------------------------------------- \n");
    DEBUG(3,"#### Writing  array for blocks: 0 -%i\n", data_set->nBlocks);
    for (m = 0; m < data_set->nBlocks; m++) {
        fGrid = &(data_set->function_blocks[step]);
        plot3d_functionBlock_writeToArray(fGrid, offset, skip);
    }
    DEBUG(3,"\n----------------------------------------- \n");
    return RTC_OK;
}

int FilterIoPlot3d::plot3d_dataSetF_readCopyFunctions(Plot3Ddataset* data_set, int step, int block) {
    LOGOUT(FILTER_LOG, (char*)"=====================================================\n");
    LOGOUT(FILTER_LOG, (char*)"==========       Read Write Functions      ==========\n");
    LOGOUT(FILTER_LOG, (char*)"=====================================================\n");
    int m;
    int err_state = RTC_OK;
    err_state |= plot3d_dataSetF_openStepFiles(data_set);
    ASSERT_RTC_OK(err_state, "Failed to open function step data file");
    err_state |= plot3d_dataSetF_readBlockCounts(data_set);
    err_state |= plot3d_dataSetF_indexBlocks(data_set);
    ASSERT_RTC_OK(err_state, "Failed to index function blocks");
    /////////////////////

    Plot3DFunctionBlock* fGrid;
    timeStep tStep = data_set->timeSteps[step];
    GridFile* gFile = &(tStep.function_file);

    DEBUG(3,"----------------------------------------- \n");
    DEBUG(3,"#### Read blocks 0 -%i into m_coords[]\n", data_set->nBlocks);
    for (m = 0; m < data_set->nBlocks; m++) {
        fGrid = &(data_set->function_blocks[m]);
        DEBUG(3,"Reading %ld grid functions for  block %i \n", fGrid->ncoords, m);
        err_state |= plot3d_functionBlock_readWhole(fGrid, gFile);
        plot3d_functionBlock_writeToArray(fGrid, 5, 0);
        plot3d_functionBlock_deAllocateMemory(fGrid);
    }
    DEBUG(3,"plot3d_gridFile_readGridData complete\n");

    plot3d_dataSetF_closeStepFiles(data_set);
    return err_state;
};

int FilterIoPlot3d::plot3d_dataSetF_readCopyFunctionBlock(Plot3Ddataset* data_set, float* target, int step, int block, int offset) {
    DEBUG(3,"----------------------------------------- \n");

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    int err_state = RTC_OK;
    Plot3DFunctionBlock* fGrid;
    plot3d_dataSetF_openStepFile(data_set, step);
    timeStep* tStep = &(data_set->timeSteps[step]);
    GridFile* gFile = &(tStep->function_file);
    plot3d_dataSetF_skipToBlock(data_set, block, step);
    LOGOUT(FILTER_LOG, (char*)"#### Read function block  %i step %i  into m_values[]\n", block, step);
    fGrid = &(data_set->function_blocks[block]);
    err_state |= plot3d_functionBlock_readWhole(fGrid, gFile);
    ASSERT_RTC_OK(err_state, "Failed to read function block");
    DataArray_copyAsFloat(&(fGrid->fValues), target, offset, 0);
    varTypeDataArray_getComponentRanges(&(fGrid->fValues), info->m_value_min, info->m_value_max, offset + step * fGrid->num_components);
    plot3d_functionBlock_deAllocateMemory(fGrid);
    plot3d_dataSetF_closeStepFile(data_set, step);
    return err_state;
};

int FilterIoPlot3d::plot3d_dataSetF_getCopyFunctionBlock(Plot3Ddataset* data_set, float* target, int block, int offset) {
    DEBUG(3,"----------------------------------------- \n");
    int err_state = RTC_OK;
    Plot3DFunctionBlock* fGrid;
    LOGOUT(FILTER_LOG, (char*)"#### Copy cached function block  %i  into m_values[]\n", block);
    fGrid = &(data_set->function_blocks[block]);
    DataArray_copyAsFloat(&(fGrid->fValues), target, offset, 0);
    return err_state;
};


} /* namespace filter */
} /* namespace pbvr */
