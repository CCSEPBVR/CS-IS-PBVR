#include <stdlib.h>
#include "filter_log.h"
#include "filter_io_plot3d_config.h"
#include "filter_io_plot3d_dataSet.h"

/**
 * plot3d_gridFile_openDataFile, Opens a grid data file and stores
 * the file handle in grid->pDataFile
 * @param data_set Plot3DGridFile* instance to work on
 * @param filepath
 * @return 
 */
int plot3d_dataSetS_openStepFile(Plot3Ddataset* data_set, int step) {
    int status = RTC_OK;
    char filePath[FILELEN_MAX];
    ReadMode rMode = data_set->params->solution_params.readmode;
    if (Fparam.end_step + Fparam.start_step > 0) {
        //sprintf(filePath, "%s/%s%06ld.%s", Fparam.in_dir, data_set->params->solution_params.file_prefix, step + Fparam.start_step, data_set->params->solution_params.file_suffix);
		sprintf(filePath, "%s/%s%06lld.%s", Fparam.in_dir, data_set->params->solution_params.file_prefix, step + Fparam.start_step, data_set->params->solution_params.file_suffix);
	} else {
        sprintf(filePath, "%s/%s.%s", Fparam.in_dir, data_set->params->solution_params.file_prefix, data_set->params->solution_params.file_suffix);
    }
    DEBUG(3," Trying to open Solution %s\n", filePath);
    timeStep* tStep = &(data_set->timeSteps[step]);
    GridFile* gridFile = &(tStep->solution_file);
    status |= filter_io_open_DataFile(gridFile, filePath, rMode);
    ASSERT_RTC_OK(status, "Failed to open solution step file");
    gridFile->needsByteSwap = (check_endian() != data_set->params->solution_params.endian);
    gridFile->blockDataTypeId = data_set->params->solution_params.precision;
    gridFile->usebytecount = data_set->params->solution_params.use_bytecount;
    return RTC_OK;
}

/**
 * plot3d_gridFile_openDataFile, Opens a grid data file and stores
 * the file handle in grid->pDataFile
 * @param data_set Plot3DGridFile* instance to work on
 * @param filepath
 * @return 
 */
int plot3d_dataSetS_openStepFiles(Plot3Ddataset* data_set) {
    int step;
    int status = RTC_OK;
    for (step = 0; step < data_set->nSteps; step++) {
        status |= plot3d_dataSetS_openStepFile(data_set, step);
    }
    return status;
}

/**
 * Close the grid data file
 * @param data_set Plot3DGridFile* instance to work on
 * @param filepath
 * @return 
 */
void plot3d_dataSetS_closeStepFile(Plot3Ddataset* data_set, int step) {
    timeStep* tStep = &(data_set->timeSteps[step]);
    GridFile* gridFile = &(tStep->solution_file);
    filter_io_close_dataFile(gridFile);
}

/**
 * Close the grid data file
 * @param data_set Plot3DGridFile* instance to work on
 * @param filepath
 * @return 
 */
int plot3d_dataSetS_closeStepFiles(Plot3Ddataset* data_set) {
    int step;
    for (step = 0; step < data_set->nSteps; step++) {
        plot3d_dataSetS_closeStepFile(data_set, step);
    }
    return RTC_OK;
}

/**
 * Allocate grid memory 
 * @param data_set Plot3DGridFile* instance to work on
 * @param nGrids
 * @return 
 */
int plot3d_dataSetS_allocateBlockTypes(Plot3Ddataset* data_set, int nGrids) {
    ref_malloc(data_set->solution_blocks, (sizeof (Plot3DSolutionBlock) * nGrids));
    ASSERT_ALLOC(data_set->solution_blocks);
    DEBUG(3,"Allocated %i grids of type SOLUTION\n", nGrids);
    data_set->nBlocks = nGrids;
    DEBUG(3,"plot3d_solutions_allocateGrids size= %lu  \n", (unsigned Int64) sizeof (Plot3DSolutionBlock) * nGrids);
    return 0;
}

/**
 * Read the grid count from the data file
 * @param data_set Plot3DGridFile* instance to work on
 * @return 
 */
int plot3d_dataSetS_readBlockCounts(Plot3Ddataset* data_set) {
    DEBUG(3,"plot3d_dataSetS_readBlockCounts  \n");
    int status = RTC_OK;
    int nBlocks = 0;
    int step;
    if (data_set->params->solution_params.use_blockcount) {
        // Verify that block count matches
        for (step = 0; step < data_set->nSteps; step++) {
            plot3d_dataSetS_openStepFile(data_set, step);
            timeStep* tStep = &(data_set->timeSteps[step]);
            GridFile* gFile = &(tStep->solution_file);
            filter_io_open_FortranBlock(gFile);
            status |= filter_io_readObject(gFile, &(nBlocks), INT_T);
            DEBUG(3,"plot3d_dataSetS_readBlockCounts %i \n", nBlocks);
            if (nBlocks != data_set->nBlocks) {
                filter_log(FILTER_ERR, "ERROR: Number of blocks not matching: %i != %i \n", data_set->nBlocks, nBlocks);
                return RTC_NG;
            }
            filter_io_close_FortranBlock(gFile);
            plot3d_dataSetS_closeStepFile(data_set, step);
        }
        DEBUG(3,"Number of Q blocks matches in all step files: %i  \n", data_set->nBlocks);
    } else {
        // Just use block count from coordinate file
        nBlocks = data_set->nBlocks;
    }
    plot3d_dataSetS_allocateBlockTypes(data_set, nBlocks);
    ASSERT_RTC_OK(status, "Failed to read solution file block counts");


    return status;
}

/**
 * Dump all grids in grid file to dump file
 * @param data_set Plot3DGridFile* instance to work on
 * @param filePath the dump file path
 */
void plot3d_dataSetS_dumpBlocks(Plot3Ddataset *data_set, const char* filePath) {
    int n;
    printf("\n==========    Dumping Grid File       ==========\n");
    FILE* file = fopen(filePath, "w");
    fprintf(file, "## Dumping %i Grids with a total of %i solutions", data_set->nBlocks, data_set->ncoords);
    for (n = 0; n < data_set->nBlocks; n++) {
        fprintf(file, "\n## Grid number %i", n);
        plot3d_solutionBlock_dump(&(data_set->solution_blocks[n]), file);
    }
    fclose(file);
}

/**
 * Read the grid dimensions for each grid in the file
 * @param data_set Plot3DGridFile* instance to work on
 * @return error code as integer
 */
int plot3d_dataSetS_indexBlocks(Plot3Ddataset* data_set) {
    int m;
    int dim[3];
    Plot3DSolutionBlock* sGrid;
    int ncoords;
    Int64 offset;
    int nBlocks;

    int byteHeader = 0;
    int size = 0;



    plot3d_dataSetS_openStepFile(data_set, 0);
    timeStep* tStep = &(data_set->timeSteps[0]);
    GridFile* gFile = &(tStep->solution_file);

    filter_io_open_FortranBlock(gFile);
    filter_io_readObject(gFile, &(nBlocks), INT_T);
    filter_io_close_FortranBlock(gFile);

    filter_io_open_FortranBlock(gFile);

    DEBUG(3,"-----------------------------------------------------\n");
    DEBUG(3,"##### Reading S block dimensions 0-%i \n", data_set->nBlocks);
    ncoords = 0;
    ref_malloc(data_set->solution_offsets, (sizeof (Int64)* data_set->nBlocks));


    int data_type_size = dataTypes[gFile->blockDataTypeId].onDiskSize;
    if (data_set->params->solution_params.use_bytecount) {
        byteHeader = 4;
    }
    offset = 4 + 2 * byteHeader;
    offset += (data_set->nBlocks * 4 * 3) + 2 * byteHeader;

    for (m = 0; m < data_set->nBlocks; m++) {
        data_set->solution_offsets[m] = offset;
        DEBUG(3,"SO %d=> %ld \n", m, data_set->solution_offsets[m]);
        sGrid = &(data_set->solution_blocks[m]);
        filter_io_readBuffer(gFile, dim, 3, INT_T);
        plot3d_solutionBlock_setDimension(sGrid, dim[0], dim[1], dim[2]);
        ncoords += sGrid->ncoords;
        size = (4 * data_type_size) + (2 * byteHeader)+(sGrid->ncoords * 5 * data_type_size) + (2 * byteHeader);
        offset = offset + size;
    }

    filter_io_close_FortranBlock(gFile);
    plot3d_dataSetS_closeStepFile(data_set, 0);
    DEBUG(3,"\ntotal grid coords = %i. (Est file size:%i KB)  \n", data_set->ncoords, (data_set->ncoords * 4 * 4) / 1024);
    return RTC_OK;
}

/**
 * Read the grid data from all grids in the file
 * @param data_set Plot3DGridFile* instance to work on
 * @return 
 */
int plot3d_dataSetS_readBlockData(Plot3Ddataset* data_set, int step) {
    int m;
    int err_state = RTC_OK;
    Plot3DSolutionBlock* sGrid;
    timeStep tStep = data_set->timeSteps[step];
    GridFile* gFile = &(tStep.solution_file);
    DEBUG(3,"----------------------------------------- \n");
    DEBUG(3,"#### Reading solution blocks 0 -%i\n", data_set->nBlocks);
    for (m = 0; m < data_set->nBlocks; m++) {
        sGrid = &(data_set->solution_blocks[m]);
        DEBUG(3,"Reading %ld solution values for  block %i \n", sGrid->ncoords, m);
        err_state |= plot3d_solutionBlock_readWhole(sGrid, gFile);
        ASSERT_RTC_OK(err_state, "Failed to read solution block");
    }

    DEBUG(3,"\n plot3d_gridFile_readGridData complete\n");
    return err_state;
}

/**
 * Deallocate array memory for all grids
 * @param data_set Plot3DGridFile* instance to work on
 * @return 
 */
int plot3d_dataSetS_deAllocateMemory(Plot3Ddataset* data_set) {
    int m;
    Plot3DSolutionBlock* sGrid;
    if (data_set->solution_blocks != NULL) {
        DEBUG(3,"#### Deallocating memory for blocks: 0 -%i\n", data_set->nBlocks);

        for (m = 0; m < data_set->nBlocks; m++) {
            sGrid = &(data_set->solution_blocks[m]);
            plot3d_solutionBlock_deAllocateMemory(sGrid);
        }
        ref_free(data_set->solution_blocks);
        ref_free(data_set->solution_offsets);
    }
    return RTC_OK;
}

int plot3d_dataSetS_skipToBlock(Plot3Ddataset* data_set, int block, int step) {

    timeStep* tStep = &(data_set->timeSteps[step]);
    GridFile* gFile = &(tStep->solution_file);
    if (gFile->rMode == BINARY) {
	int offset = data_set->solution_offsets[block];
	DEBUG(3,"plot3d_dataSetS_skipToBlock %i at %i  (tstep %i) \n", block, offset, step);
	filter_io_skipToOffset(gFile, offset);
    } else {
	filter_log(FILTER_ERR, "Can not skip to offset in ASCII File\n");
    }
    return RTC_OK;
}

/**
 * Example
 * Read a multi block 3DWhole with multiple structured grid with IBlanks
 * @param data_set Plot3DGridFile* instance to work on
 * @return 
 */
int plot3d_dataSetS_read3DSingleBlock(Plot3Ddataset* data_set, int block, int step) {
    int err_state = RTC_OK;
    char filePath[FILELEN_MAX];
    sprintf(filePath, "%s/%s.%s", Fparam.in_dir, data_set->params->solution_params.file_prefix, data_set->params->solution_params.file_suffix);
    DEBUG(3,"#### Trying to open filepath %s\n", filePath);
    err_state |= plot3d_dataSetS_openStepFiles(data_set);
    ASSERT_RTC_OK(err_state, "Failed to open solution step file");
    err_state |= plot3d_dataSetS_readBlockCounts(data_set);
    err_state |= plot3d_dataSetS_indexBlocks(data_set);
    ASSERT_RTC_OK(err_state, "Failed to index blocks in solution step file");
    plot3d_dataSetS_skipToBlock(data_set, block, step);
    err_state |= plot3d_dataSetS_readBlockData(data_set, step);
    plot3d_dataSetS_closeStepFiles(data_set);
    return err_state;
}

/**
 * Example
 * Read a multi block 3DWhole with multiple structured grid with IBlanks
 * @param data_set Plot3DGridFile* instance to work on
 * @return 
 */
int plot3d_dataSetS_read3DMultiBlock(Plot3Ddataset* data_set, int step) {
    filter_log(FILTER_LOG, "=====================================================\n");
    filter_log(FILTER_LOG, "==========       Reading Grid File    S    ==========\n");
    filter_log(FILTER_LOG, "=====================================================\n");
    int err_state = RTC_OK;
    err_state |= plot3d_dataSetS_openStepFile(data_set, step);
    ASSERT_RTC_OK(err_state, "Failed to open solution step file");
    err_state |= plot3d_dataSetS_readBlockCounts(data_set);
    err_state |= plot3d_dataSetS_indexBlocks(data_set);
    ASSERT_RTC_OK(err_state, "Failed to index solution blocks");
    err_state |= plot3d_dataSetS_readBlockData(data_set, step);
    plot3d_dataSetS_closeStepFile(data_set, step);
    return err_state;
}

/**
 * Write all grids to output array
 * @param data_set Plot3DGridFile* instance to work on
 * @param step
 * @return 
 */
int plot3d_dataSetS_copyToArray(Plot3Ddataset* data_set, int step, int offset, int skip) {
    int m;
    Plot3DSolutionBlock* sGrid;
    filter_log(FILTER_LOG, "##### Writing grid data %d of  into target array\n", step);
    DEBUG(3,"----------------------------------------- \n");
    DEBUG(3,"#### Writing  array for blocks: 0 -%i\n", data_set->nBlocks);
    for (m = 0; m < data_set->nBlocks; m++) {
        sGrid = &(data_set->solution_blocks[step]);
        plot3d_solutionBlock_writeToArray(sGrid, offset, skip);
    }
    DEBUG(3,"\n----------------------------------------- \n");
    return RTC_OK;
}

int plot3d_dataSetS_readCopySolution(Plot3Ddataset* data_set, int step, int block) {
    filter_log(FILTER_LOG, "=====================================================\n");
    filter_log(FILTER_LOG, "==========       Read Write Solution       ==========\n");
    filter_log(FILTER_LOG, "=====================================================\n");
    int m;
    int err_state = RTC_OK;
    char filePath[FILELEN_MAX];
    sprintf(filePath, "%s/%s%06d.%s", Fparam.in_dir, data_set->params->solution_params.file_prefix, step, data_set->params->solution_params.file_suffix);
    DEBUG(3," Trying to open filepath %s\n", filePath);
    err_state |= plot3d_dataSetS_openStepFiles(data_set);
    ASSERT_RTC_OK(err_state, "Failed to open solution step data file");
    err_state |= plot3d_dataSetS_readBlockCounts(data_set);
    err_state |= plot3d_dataSetS_indexBlocks(data_set);
    ASSERT_RTC_OK(err_state, "Failed to index solution blocks");
    /////////////////////

    Plot3DSolutionBlock* sGrid;
    timeStep tStep = data_set->timeSteps[step];
    GridFile* gFile = &(tStep.solution_file);
    DEBUG(3,"----------------------------------------- \n");
    DEBUG(3,"#### Read blocks 0 -%i into m_coords[]\n", data_set->nBlocks);
    for (m = 0; m < data_set->nBlocks; m++) {
        sGrid = &(data_set->solution_blocks[m]);
        DEBUG(3,"Reading %ld grid solutions for  block %i \n", sGrid->ncoords, m);
        err_state |= plot3d_solutionBlock_readWhole(sGrid, gFile);
        plot3d_solutionBlock_writeToArray(sGrid, 0, data_set->params->solution_params.num_vars);
        plot3d_solutionBlock_deAllocateMemory(sGrid);
    }
    DEBUG(3,"\n plot3d_gridFile_readGridData complete\n");

    plot3d_dataSetS_closeStepFiles(data_set);
    return err_state;
};

int plot3d_dataSetS_readCopySolutionBlock(Plot3Ddataset* data_set, float* target, int step, int block, int skip) {
    DEBUG(3,"----------------------------------------- \n");
    int err_state = RTC_OK;
    Plot3DSolutionBlock* sGrid;
    plot3d_dataSetS_openStepFile(data_set, step);
    timeStep* tStep = &(data_set->timeSteps[step]);
    GridFile* gFile = &(tStep->solution_file);
    plot3d_dataSetS_skipToBlock(data_set, block, step);
    DEBUG(3,"#### Read solution block  %i step %i  into m_values[]\n", block, step);
    sGrid = &(data_set->solution_blocks[block]);
    err_state |= plot3d_solutionBlock_readWhole(sGrid, gFile);
    ASSERT_RTC_OK(err_state, "Failed to read solution block");
    DataArray_copyAsFloat(&(sGrid->QValues), target, 0, skip);
    varTypeDataArray_getComponentRanges(&(sGrid->QValues), m_value_min, m_value_max, step * 5);
    plot3d_solutionBlock_deAllocateMemory(sGrid);
    plot3d_dataSetS_closeStepFile(data_set, step);
    return err_state;
};

int plot3d_dataSetS_getCopySolutionBlock(Plot3Ddataset* data_set, float* target, int block, int skip) {
    DEBUG(3,"----------------------------------------- \n");
    int err_state = RTC_OK;
    Plot3DSolutionBlock* sGrid;
    filter_log(FILTER_LOG, "#### Copy cached solution block  %i   into m_values[]\n", block);
    sGrid = &(data_set->solution_blocks[block]);
    DataArray_copyAsFloat(&(sGrid->QValues), target, 0, skip);
    return err_state;
};

