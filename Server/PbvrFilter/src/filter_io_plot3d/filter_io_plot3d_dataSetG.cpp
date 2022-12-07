#include <filter_io_plot3d/FilterIoPlot3d.h>
#include <stdlib.h>


namespace pbvr {
namespace filter {

/**
 * plot3d_gridFile_openDataFile, Opens a grid data file and stores
 * the file handle in grid->pDataFile
 * @param data_set Plot3DGridFile* instance to work on
 * @param filepath
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetG_openDataFile(Plot3Ddataset* data_set) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;

    char filePath[FILELEN_MAX];
    sprintf(filePath, "%s/%s.%s", param->in_dir, sParams.coordinate_params.file_prefix, sParams.coordinate_params.file_suffix);
    DEBUG(3,"plot3d_readGridFiles will read %s\n", filePath);
    ReadMode rMode = data_set->params->coordinate_params.readmode;
    GridFile* gridFile = &(data_set->coordinate_file);
    if (filter_io_open_DataFile(gridFile, filePath, rMode) == RTC_NG) {
        return RTC_NG;
    }
    gridFile->needsByteSwap = (check_endian() != data_set->params->coordinate_params.endian);
    gridFile->blockDataTypeId = data_set->params->coordinate_params.precision;
    gridFile->usebytecount = data_set->params->coordinate_params.use_bytecount;
    return RTC_OK;
}

/**
 * Close the grid data file
 * @param data_set Plot3DGridFile* instance to work on
 * @param filepath
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetG_closeDataFile(Plot3Ddataset* data_set) {
    int status = filter_io_close_dataFile(&(data_set->coordinate_file));
    return status;
}

/**
 * Allocate grid memory
 * @param data_set Plot3DGridFile* instance to work on
 * @param nGrids
 * @return err state
 */
int FilterIoPlot3d::plot3d_dataSetG_allocateBlockTypes(Plot3Ddataset* data_set, int nGrids) {
    // ref_malloc(data_set->coordinate_blocks, (sizeof (Plot3DCoordinateBlock) * nGrids));
    data_set->coordinate_blocks = (Plot3DCoordinateBlock*)malloc(sizeof (Plot3DCoordinateBlock) * nGrids);
    ASSERT_ALLOC(data_set->coordinate_blocks);
    DEBUG(3,"Allocated %i grids of type COORDINATE\n", nGrids);
    data_set->nBlocks = nGrids;
    DEBUG(3,"plot3d_coordinates_allocateGrids size= %lu  \n", (unsigned Int64) sizeof (Plot3DCoordinateBlock) * nGrids);
    return RTC_OK;
}

/**
 * Read the grid count from the data file
 * @param data_set Plot3DGridFile* instance to work on
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetG_readBlockCount(Plot3Ddataset* data_set) {
    int status = RTC_OK;
    int nGrids;
    if (data_set->params->coordinate_params.use_blockcount) {
        //Use block count from header
        GridFile* gFile = &(data_set->coordinate_file);
        filter_io_open_FortranBlock(gFile);
        status |= filter_io_readObject(gFile, &(nGrids), INT_T);
        plot3d_dataSetG_allocateBlockTypes(data_set, nGrids);
        filter_io_close_FortranBlock(gFile);
        DEBUG(3,"Number of grids in this file: %i  \n", data_set->nBlocks);
    } else {
        //File does not contain block count, default to single block
        plot3d_dataSetG_allocateBlockTypes(data_set, 1);
        DEBUG(3,"Use block count disabled, defaulting to %i block:  \n", data_set->nBlocks);
    }
    return status;
}

/**
 * Dump all grids in grid file to dump file
 * @param data_set Plot3DGridFile* instance to work on
 * @param filePath the dump file path
 */
void FilterIoPlot3d::plot3d_dataSetG_dumpBlocks(Plot3Ddataset *data_set, const char* filePath) {
    int n;
    printf("\n==========    Dumping Grid File       ==========\n");
    FILE* file = fopen(filePath, "w");
    fprintf(file, "## Dumping %i Grids with a total of %i coordinates", data_set->nBlocks, data_set->ncoords);
    for (n = 0; n < data_set->nBlocks; n++) {
        fprintf(file, "\n## Grid number %i", n);
        plot3d_coordinateBlock_dump(&(data_set->coordinate_blocks[n]), file);
    }
    fclose(file);
}

/**
 * Read the grid dimensions for each grid in the file
 * @param data_set Plot3DGridFile* instance to work on
 * @return error code as integer
 */
int FilterIoPlot3d::plot3d_dataSetG_readBlockDimensions(Plot3Ddataset * data_set) {
    int m;
    int err_state = RTC_OK;
    int dim3[3];
    Plot3DCoordinateBlock* cGrid;

    GridFile* gFile = &(data_set->coordinate_file);
    filter_io_open_FortranBlock(gFile);

    DEBUG(3,"-----------------------------------------------------\n");
    DEBUG(3,"##### Reading block dimensions 0-%i \n", data_set->nBlocks);
    data_set->ncoords = 0;
    // ref_malloc(data_set->coordinate_offsets, (sizeof (Int64)* data_set->nBlocks));
    data_set->coordinate_offsets = (Int64*)malloc(sizeof (Int64)* data_set->nBlocks);
    ASSERT_ALLOC(data_set->coordinate_offsets);
    int byteHeader = 0;
    int size = 0;
    Int64 offset;
    int data_type_size = dataTypes[data_set->coordinate_file.blockDataTypeId].onDiskSize;
    if (data_set->params->coordinate_params.use_bytecount) {
        byteHeader = 4;
    }
    offset = 4 + 2 * byteHeader;
    offset += (data_set->nBlocks * 4 * 3) + 2 * byteHeader;
    for (m = 0; m < data_set->nBlocks; m++) {
        data_set->coordinate_offsets[m] = offset;
        cGrid = &(data_set->coordinate_blocks[m]);
        err_state |= filter_io_readBuffer(gFile, dim3, 3, INT_T);
        plot3d_coordinateBlock_setDimension(cGrid, dim3[0], dim3[1], dim3[2]);
        data_set->ncoords += cGrid->ncoords;
        size = (cGrid->ncoords * 3 * data_type_size) + (2 * byteHeader);
        if (data_set->hasIblanks) {
            size += cGrid->ncoords * dataTypes[INT_T].onDiskSize;
        }
        offset = offset + size;
    }
    ASSERT_RTC_OK(err_state, "Failed to read block dimensions");
    filter_io_close_FortranBlock(gFile);
    DEBUG(3,"\ntotal grid coords = %i. (Est file size:%i KB)  \n", data_set->ncoords, (data_set->ncoords * 4 * 4) / 1024);
    return RTC_OK;
}

/**
 * Read the grid data from all grids in the file
 * @param data_set Plot3DGridFile* instance to work on
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetG_readBlockData(Plot3Ddataset * data_set) {
    int m;
    int err_state = RTC_OK;
    grid_params* gp = &(data_set->params->coordinate_params);
    Plot3DCoordinateBlock* cGrid;
    GridFile* gFile = &(data_set->coordinate_file);
    DEBUG(3,"----------------------------------------- \n");
    DEBUG(3,"#### Reading blocks 0 -%i\n", data_set->nBlocks);
    for (m = 0; m < data_set->nBlocks; m++) {
        cGrid = &(data_set->coordinate_blocks[m]);
        DEBUG(3,"Reading %ld grid coordinates for  block %i \n", cGrid->ncoords, m);
        err_state |= plot3d_coordinateBlock_readWhole(cGrid, gFile, gp->has_iblanks);
    }
    ASSERT_RTC_OK(err_state, "Failed to read block data");
    DEBUG(3,"\n plot3d_gridFile_readGridData complete\n");
    return err_state;
}

/**
 * Deallocate array memory for all grids
 * @param data_set Plot3DGridFile* instance to work on
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetG_deAllocateMemory(Plot3Ddataset * data_set) {

    if (data_set == NULL) return RTC_OK;
    int m;
    Plot3DCoordinateBlock* cGrid;
    if (data_set->coordinate_blocks != NULL) {
        DEBUG(3,"#### Deallocating memory for coordinate blocks: 0 -%i\n", data_set->nBlocks);
        for (m = 0; m < data_set->nBlocks; m++) {
            cGrid = &(data_set->coordinate_blocks[m]);
            plot3d_coordinateBlock_deAllocateMemory(cGrid);
        }
        ref_free(data_set->coordinate_blocks);
        ref_free(data_set->coordinate_offsets);
    }
    return RTC_OK;
}

/**
 * Skip to byte offset corresponding to block
 * @param data_set
 * @param block
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetG_skipToBlock(Plot3Ddataset* data_set, int block) {
    GridFile* gFile = &(data_set->coordinate_file);
    if (gFile->rMode == BINARY) {
    filter_io_skipToOffset(gFile, data_set->coordinate_offsets[block]);
    } else {
    LOGOUT(FILTER_ERR, (char*)"Can not skip to offset in ASCII File\n");
    }
    return RTC_OK;
}

/**
 * Example
 * Read a multi block 3DWhole with multiple structured grid with IBlanks
 * @param data_set Plot3DGridFile* instance to work on
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetG_read3DSingleBlock(Plot3Ddataset* data_set, int block) {
    int err_state = RTC_OK;
    err_state |= plot3d_dataSetG_openDataFile(data_set);
    ASSERT_RTC_OK(err_state, "Failed to open grid file");
    err_state |= plot3d_dataSetG_readBlockCount(data_set);
    err_state |= plot3d_dataSetG_readBlockDimensions(data_set);
    ASSERT_RTC_OK(err_state, "Failed to read block dimensions");
    plot3d_dataSetG_skipToBlock(data_set, block);
    err_state |= plot3d_dataSetG_readBlockData(data_set);
    plot3d_dataSetG_closeDataFile(data_set);
    return err_state;
}

/**
 * Example
 * Read a multi block 3DWhole with multiple structured grid with IBlanks
 * @param data_set Plot3DGridFile* instance to work on
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetG_read3DMultiBlock(Plot3Ddataset* data_set) {
    LOGOUT(FILTER_LOG, (char*)"=====================================================\n");
    LOGOUT(FILTER_LOG, (char*)"==========       Reading Grid File    G    ==========\n");
    LOGOUT(FILTER_LOG, (char*)"=====================================================\n");
    int err_state = RTC_OK;
    err_state |= plot3d_dataSetG_openDataFile(data_set);
    ASSERT_RTC_OK(err_state, "Failed to open grid file");
    err_state |= plot3d_dataSetG_readBlockCount(data_set);
    err_state |= plot3d_dataSetG_readBlockDimensions(data_set);
    ASSERT_RTC_OK(err_state, "Failed to read grid file block dimensions");
    err_state |= plot3d_dataSetG_readBlockData(data_set);
    plot3d_dataSetG_closeDataFile(data_set);
    return err_state;
}

/**
 * Write all grids to output array
 * @param data_set Plot3DGridFile* instance to work on
 * @param step
 * @return
 */
int FilterIoPlot3d::plot3d_dataSetG_copyToArray(Plot3Ddataset* data_set, int step, int offset, int skip) {
    int m;
    Plot3DCoordinateBlock* cGrid;
    LOGOUT(FILTER_LOG, (char*)"##### Writing grid data %d of  into target array\n", step, data_set->nBlocks);

    DEBUG(3,"----------------------------------------- \n");
    DEBUG(3,"#### Writing  array for blocks: 0 -%i\n", data_set->nBlocks);
    for (m = 0; m < data_set->nBlocks; m++) {
        cGrid = &(data_set->coordinate_blocks[step]);
        plot3d_coordinateBlock_writeToArray(cGrid, offset, skip);
    }
    DEBUG(3,"\n----------------------------------------- \n");
    return RTC_OK;
}

int FilterIoPlot3d::plot3d_dataSetG_readCopyCoords(Plot3Ddataset* data_set, int block, int step) {
    LOGOUT(FILTER_LOG, (char*)"=====================================================\n");
    LOGOUT(FILTER_LOG, (char*)"==========       Read Write Coordinates    ==========\n");
    LOGOUT(FILTER_LOG, (char*)"=====================================================\n");
    int m;
    int err_state = RTC_OK;
    err_state |= plot3d_dataSetG_openDataFile(data_set);
    ASSERT_RTC_OK(err_state, "Failed to open grid file");
    err_state |= plot3d_dataSetG_readBlockCount(data_set);
    err_state |= plot3d_dataSetG_readBlockDimensions(data_set);
    ASSERT_RTC_OK(err_state, "Failed read grid file block dimensions");
    /////////////////////
    grid_params* gp = &(data_set->params->coordinate_params);
    Plot3DCoordinateBlock* cGrid;
    GridFile* gFile = &(data_set->coordinate_file);
    DEBUG(3,"----------------------------------------- \n");
    DEBUG(3,"#### Skip blocks 0 -%i into m_coords[]\n", block);
    for (m = 0; m < block; m++) {
        cGrid = &(data_set->coordinate_blocks[m]);
        DEBUG(3,"Reading %ld grid coordinates for  block %i \n", cGrid->ncoords, m);
        err_state |= plot3d_coordinateBlock_readWhole(cGrid, gFile, gp->has_iblanks);
        plot3d_coordinateBlock_writeToArray(cGrid, 0, 0);
        plot3d_coordinateBlock_deAllocateMemory(cGrid);
    }
    DEBUG(3,"\n plot3d_gridFile_readGridData complete\n");
    plot3d_dataSetG_closeDataFile(data_set);
    return err_state;
};

/**
 * Find grid XYZ range
 * @param data_set
 */
void FilterIoPlot3d::plot3d_dataSetG_findGridDimensions(Plot3Ddataset * data_set) {
    int m;
    Plot3DCoordinateBlock* cGrid;
    cGrid = &(data_set->coordinate_blocks[0]);
    plot3d_coordinateBlock_find_dims(cGrid, data_set->coordinate_file.blockDataTypeId);

    data_set->xRange = Range_copy(cGrid->xRange);
    data_set->yRange = Range_copy(cGrid->yRange);
    data_set->zRange = Range_copy(cGrid->zRange);

    for (m = 1; m < data_set->nBlocks; m++) {
        cGrid = &(data_set->coordinate_blocks[m]);
        plot3d_coordinateBlock_find_dims(cGrid, data_set->coordinate_file.blockDataTypeId);
        Range_extend(&data_set->xRange, cGrid->xRange);
        Range_extend(&data_set->yRange, cGrid->yRange);
        Range_extend(&data_set->zRange, cGrid->zRange);
    }

    LOGOUT(FILTER_LOG, (char*)"## All blocks  =>[ min_x:%+.5lf   min_y:%+.5lf   min_z:%+.5lf] \t \n", data_set->xRange.min, data_set->yRange.min, data_set->zRange.min);
    LOGOUT(FILTER_LOG, (char*)"## All blocks  =>[ max_x:%+.5lf   max_y:%+.5lf   max_z:%+.5lf] \t \n", data_set->xRange.max, data_set->yRange.max, data_set->zRange.max);
}

} /* namespace filter */
} /* namespace pbvr */
