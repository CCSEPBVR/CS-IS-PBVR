#include <filter_io_plot3d/FilterIoPlot3d.h>
#include <stdio.h>
#include <stdlib.h>


namespace pbvr {
namespace filter {

/**
 * Set the solution grid dimensions
 * @param sBlock Plot3DSolutionBlock* the solution grid struct to work on
 * @param i, dimension i ( the other x)
 * @param j, dimension j ( the other y)
 * @param k, dimension k ( the other z)
 */
void FilterIoPlot3d::plot3d_solutionBlock_setDimension(Plot3DSolutionBlock* sBlock, int i, int j, int k) {
    DEBUG(3,"##plot3d_solutionFile_setDimension [nj,ni,nk] = [%i,%i,%i] (%ld) \n", i, j, k, sBlock->ncoords);
    sBlock->gridDimension.i = i;
    sBlock->gridDimension.j = j;
    sBlock->gridDimension.k = k;
    sBlock->ncoords = i * j * k;
}

/**
 * Allocate array memory for density, energy and momentum values
 * @param sBlock Plot3DSolutionBlock* the solution grid instance to work on
 * @param typeId
 */
void FilterIoPlot3d::plot3d_solutionBlock_allocateMemory(Plot3DSolutionBlock* sBlock, DataTypeId typeId) {
    sBlock->QValues = DataArray_create(sBlock->ncoords);
    sBlock->density = DataArray_addComponent(&sBlock->QValues, typeId);
    sBlock->momentumX = DataArray_addComponent(&sBlock->QValues, typeId);
    sBlock->momentumY = DataArray_addComponent(&sBlock->QValues, typeId);
    sBlock->momentumZ = DataArray_addComponent(&sBlock->QValues, typeId);
    sBlock->energyTotal = DataArray_addComponent(&sBlock->QValues, typeId);
    sBlock->allocatedQValues = true;
    DEBUG(3,"##Allocated Solution Q value arrays of size %ld \n", sBlock->ncoords);
}

/**
 * Deallocate value arrays for this grid
 * @param sBlock Plot3DSolutionBlock* the solution grid struct to work on
 */
void FilterIoPlot3d::plot3d_solutionBlock_deAllocateMemory(Plot3DSolutionBlock* sBlock) {
    if (sBlock->allocatedQValues) {
        DataArray_destruct(&sBlock->QValues);
    }
    sBlock->allocatedQValues = false;
}

/**
 * Calculate the block disk size in bytes.
 * @param cGrid Plot3DCoordinateGrid* the coordinate grid instance to work on
 * @param gFile
 * @return
 */
Int64 FilterIoPlot3d::plot3d_solutionBlock_getDiskByteSize(Plot3DSolutionBlock* cGrid, GridFile* gFile) {
    Int64 byteSize = 0;
    int nPointsInBlock = cGrid->ncoords;
    DataTypeId read_type = gFile->blockDataTypeId;
    if (gFile->usebytecount) {
        byteSize += 8;
    }
    byteSize += 5 * nPointsInBlock * dataTypes[read_type].onDiskSize;
    DEBUG(3,"Setting function block byte size to %ld \n", byteSize);
    return byteSize;
}

/**
 * Read the grid data as whole
 * @param sBlock Plot3DSolutionBlock* the solution grid struct to work on
 * @param gFile GridFile* instance to read from
 * @return int status code as
 *
 */
int FilterIoPlot3d::plot3d_solutionBlock_readWhole(Plot3DSolutionBlock* sBlock, GridFile* gFile) {
    int err_state = RTC_OK;

    // Read block solution headers
    int blockSize = filter_io_open_FortranBlock(gFile);
    UNUSED(blockSize);
    DEBUG(3,"reading Q Solution headers, block size = %d\n", blockSize);
    plot3d_solutionBlock_allocateMemory(sBlock, gFile->blockDataTypeId);
    DataTypeId read_type = gFile->blockDataTypeId;
    err_state |= filter_io_readObject(gFile, &(sBlock->mach), read_type);
    err_state |= filter_io_readObject(gFile, &(sBlock->alpha), read_type);
    err_state |= filter_io_readObject(gFile, &(sBlock->reyn), read_type);
    err_state |= filter_io_readObject(gFile, &(sBlock->time), read_type);
    filter_io_close_FortranBlock(gFile);
    ASSERT_RTC_OK(err_state, "Failed to read solution block header (mach,alpha,reyn,time");
    unsigned char* dnBuff = sBlock->density->ucBuffer;
    unsigned char* mxBuff = sBlock->momentumX->ucBuffer;
    unsigned char* myBuff = sBlock->momentumY->ucBuffer;
    unsigned char* mzBuff = sBlock->momentumZ->ucBuffer;
    unsigned char* etBuff = sBlock->energyTotal->ucBuffer;
    filter_io_open_FortranBlock(gFile);
    DEBUG(3,"reading %ld Q Solution values \n", sBlock->ncoords);
    err_state |= filter_io_readBuffer(gFile, dnBuff, sBlock->ncoords, read_type);
    err_state |= filter_io_readBuffer(gFile, mxBuff, sBlock->ncoords, read_type);
    err_state |= filter_io_readBuffer(gFile, myBuff, sBlock->ncoords, read_type);
    err_state |= filter_io_readBuffer(gFile, mzBuff, sBlock->ncoords, read_type);
    err_state |= filter_io_readBuffer(gFile, etBuff, sBlock->ncoords, read_type);
    ASSERT_RTC_OK(err_state, "Failed to read solution block data");
    filter_io_close_FortranBlock(gFile);
    return err_state;
}

/**
 * Write grid data to output array
 * @param cGrid Plot3DCoordinateGrid* the coordinate grid instance to work on
 */
void FilterIoPlot3d::plot3d_solutionBlock_writeToArray(Plot3DSolutionBlock* sBlock, int offset, int skip) {
    LOGOUT(FILTER_LOG, (char*)"##### plot3d_functionGrid_writeToArray\n");

    if (this->m_filter_info == NULL) return;
    if (this->m_filter_info->m_param == NULL) return;
    PbvrFilterInfo *info = this->m_filter_info;

    DataArray_copyAsFloat(&(sBlock->QValues), info->m_values, offset, skip);
};

/**
 * Dump the grid data to file
 * @param sBlock Plot3DSolutionBlock* the solution grid struct to work on
 * @param file FILE* file handle to dump to
 */
void FilterIoPlot3d::plot3d_solutionBlock_dump(Plot3DSolutionBlock* sBlock, FILE* file) {
    int n;
    double d, e, mx, my, mz;
    fprintf(file, "##  Dumping Solution ncoords=%lld  \n", sBlock->ncoords);
    fprintf(file, "##  Dimensions: {%i, %i, %i}\n", sBlock->gridDimension.i, sBlock->gridDimension.j, sBlock->gridDimension.k);
    fprintf(file, "##  [mach:%f , alpha:%f , reyn:%f, time:%f]  \n",
            sBlock->mach, sBlock->alpha, sBlock->reyn, sBlock->time);
    for (n = 0; n < sBlock->ncoords; n++) {
        d = sBlock->density->fData[n];
        mx = sBlock->momentumX->fData[n];
        my = sBlock->momentumY->fData[n];
        mz = sBlock->momentumZ->fData[n];
        e = sBlock->energyTotal->fData[n];
        fprintf(file, "## %6i =>[ d:%lf \t x:%lf \t y:%lf \t z:%lf \t e:%lf] \n", n, d, mx, my, mz, e);
    }
}

} /* namespace filter */
} /* namespace pbvr */
