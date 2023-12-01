#include <stdio.h>
#include <stdlib.h>
#include "../filter.h"
#include "filter_io_plot3d_coordinateBlock.h"
#include "filter_io_file_reader.h"
#include "filter_io_types.h"
#include "filter_log.h"

/**
 * Set the grid dimensions
 * @param cBlock Plot3DCoordinateGrid* the coordinate grid instance to work on
 * @param i, dimension i ( the other x)
 * @param j, dimension j ( the other y)
 * @param k, dimension k ( the other z)
 */
void plot3d_coordinateBlock_setDimension(Plot3DCoordinateBlock* cBlock, int i, int j, int k) {
    DEBUG(3,"##plot3d_coordinateGrid_setDimension %p [nj,ni,nk] = [%i,%i,%i]  \n", cBlock, i, j, k);
    cBlock->gridDimension.i = i;
    cBlock->gridDimension.j = j;
    cBlock->gridDimension.k = k;
    cBlock->ncoords = i * j * k;
}

/**
 * Allocate array memory for coordinate values (and iblanks if used)
 * @param cBlock Plot3DCoordinateGrid* the coordinate grid instance to work on
 * @param hasIBlank
 * @param typeId
 */
void plot3d_coordinateBlock_allocateMemory(Plot3DCoordinateBlock* cBlock, bool hasIBlank, DataTypeId typeId) {
    cBlock->coordinates = DataArray_create(cBlock->ncoords);
    cBlock->xTuples = DataArray_addComponent(&cBlock->coordinates, typeId);
    cBlock->yTuples = DataArray_addComponent(&cBlock->coordinates, typeId);
    cBlock->zTuples = DataArray_addComponent(&cBlock->coordinates, typeId);
    DEBUG(3,"##Allocated 3 tuple arrays of type %i and size %ld  \n", typeId, cBlock->ncoords);
    cBlock->hasIblanks = hasIBlank;

    if (cBlock->hasIblanks) {
        //cBlock->ibTuples = DataArray_addComponent(&cBlock->coordinates, INT_T);
        //        ref_malloc(cBlock->ibTuples, (sizeof (int) * cBlock->ncoords));
        DEBUG(3,"##Allocated IBlank array size %ld  \n", cBlock->ncoords);
    }
}

/**
 * Deallocate grid memory
 * @param cBlock Plot3DCoordinateGrid* the coordinate grid instance to work on
 */
void plot3d_coordinateBlock_deAllocateMemory(Plot3DCoordinateBlock* cBlock) {
    DataArray_destruct(&cBlock->coordinates);
}

/**
 * Calculate the block disk size in bytes.
 * @param cBlock Plot3DCoordinateGrid* the coordinate grid instance to work on
 * @param gFile
 * @param hasIBlanks
 * @return 
 */
int plot3d_coordinateBlock_setDiskByteSize(Plot3DCoordinateBlock* cBlock, GridFile* gFile, bool hasIBlanks) {
    int err_state = RTC_OK;
    cBlock->diskByteSize = 0;
    int nPointsInBlock = cBlock->ncoords;
    DataTypeId read_type = gFile->blockDataTypeId;
    if (gFile->usebytecount) {
        cBlock->diskByteSize += 8;
    }
    cBlock->diskByteSize = 3 * nPointsInBlock * dataTypes[read_type].onDiskSize;
    if (hasIBlanks == true) {
        cBlock->diskByteSize += nPointsInBlock * dataTypes[INT_T].onDiskSize;
    }
    DEBUG(3,"Setting coordinate block byte size to %ld \n", cBlock->diskByteSize);
    return err_state;
}

/**
 * Read grid data as whole
 * @param cBlock Plot3DCoordinateGrid* the coordinate grid instance to work on
 * @param gFile
 * @param hasIBlanks
 * @return 
 */
int plot3d_coordinateBlock_readWhole(Plot3DCoordinateBlock* cBlock, GridFile* gFile, bool hasIBlanks) {
    int err_state = RTC_OK;
    int nPointsInBlock = cBlock->ncoords;
    filter_io_open_FortranBlock(gFile);

    plot3d_coordinateBlock_allocateMemory(cBlock, hasIBlanks, gFile->blockDataTypeId);

    DataTypeId read_type = gFile->blockDataTypeId;
    DEBUG(3,"Reading %ld coordinate sets of type %d \n", cBlock->ncoords, gFile->blockDataTypeId);

    unsigned char* xBuff = cBlock->xTuples->ucBuffer;
    unsigned char* yBuff = cBlock->yTuples->ucBuffer;
    unsigned char* zBuff = cBlock->zTuples->ucBuffer;

    err_state |= filter_io_readBuffer(gFile, xBuff, nPointsInBlock, read_type);
    err_state |= filter_io_readBuffer(gFile, yBuff, nPointsInBlock, read_type);
    err_state |= filter_io_readBuffer(gFile, zBuff, nPointsInBlock, read_type);
    ASSERT_RTC_OK(err_state, "Failed to read coordinates");
    if (hasIBlanks == true) {
//	unsigned char* iBuff = cBlock->ibTuples->ucBuffer;
	unsigned char* iBuff = malloc(4 * nPointsInBlock);
        err_state |= filter_io_readBuffer(gFile, iBuff, nPointsInBlock, INT_T);
	free(iBuff);
        ASSERT_RTC_OK(err_state, "Failed to read Iblank information");	
    }
    filter_io_close_FortranBlock(gFile);
    return err_state;
}

/**
 * Read grid data as planes
 * @param cBlock Plot3DCoordinateGrid* the coordinate grid instance to work on
 * @param gFile
 * @param hasIBlanks
 * @return 
 */
int plot3d_coordinateBlock_readPlanes(Plot3DCoordinateBlock* cBlock, GridFile* gFile, bool hasIBlanks) {
    int k;
    int status = 0;
    int nPointsInPlane = cBlock->gridDimension.i * cBlock->gridDimension.j;
    filter_io_open_FortranBlock(gFile);
    plot3d_coordinateBlock_allocateMemory(cBlock, hasIBlanks, gFile->blockDataTypeId);
    DataTypeId read_type = gFile->blockDataTypeId;
    DEBUG(3,"Reading %ld coordinate planes of type %d \n", cBlock->ncoords, gFile->blockDataTypeId);
    for (k = 0; k < cBlock->gridDimension.k; k++) {

        unsigned char* xBuff =  DataComponent_getPointerToTuple(cBlock->xTuples, k*nPointsInPlane);
        unsigned char* yBuff =  DataComponent_getPointerToTuple(cBlock->yTuples, k*nPointsInPlane);
        unsigned char* zBuff =  DataComponent_getPointerToTuple(cBlock->zTuples, k*nPointsInPlane);

        status |= filter_io_readBuffer(gFile, xBuff, nPointsInPlane, read_type);
        status |= filter_io_readBuffer(gFile, yBuff, nPointsInPlane, read_type);
        status |= filter_io_readBuffer(gFile, zBuff, nPointsInPlane, read_type);

        if (cBlock->hasIblanks) {
            unsigned char* ibBuff =  DataComponent_getPointerToTuple(cBlock->ibTuples, k*nPointsInPlane);
            status |= filter_io_readBuffer(gFile, ibBuff, nPointsInPlane, INT_T);
        }
    }

    filter_io_close_FortranBlock(gFile);
    return status;
}

/**
 * Write grid data to output array
 * @param cBlock Plot3DCoordinateGrid* the coordinate grid instance to work on
 */
void plot3d_coordinateBlock_writeToArray(Plot3DCoordinateBlock* cBlock, int offset, int skip) {
    filter_log(FILTER_LOG, "##### plot3d_functionGrid_writeToArray\n");
    DataArray_copyAsFloat(&(cBlock->coordinates), m_coords, offset, skip);
};

/**
 * Dump grid data to file
 * @param cBlock Plot3DCoordinateGrid* the coordinate grid instance to work on
 * @param file FILE* the file handle to work on
 */
void plot3d_coordinateBlock_dump(Plot3DCoordinateBlock* cBlock, FILE* file) {
    int n;
    double x, y, z;
    int ib = -1;
    //fprintf(file, "\n##  Dumping Whole ncoords=%ld, Iblanks=%i \n", cBlock->ncoords, ib);
	fprintf(file, "\n##  Dumping Whole ncoords=%lld, Iblanks=%i \n", cBlock->ncoords, ib);
	fprintf(file, "##  Dimensions: {%i, %i, %i}\n", cBlock->gridDimension.i, cBlock->gridDimension.j, cBlock->gridDimension.k);
    for (n = 0; n < cBlock->ncoords; n++) {
        x = cBlock->xTuples->dData[n];
        y = cBlock->yTuples->dData[n];
        z = cBlock->zTuples->dData[n];
        if (cBlock->hasIblanks) {
            cBlock->ibTuples->iData[n];
        }
        fprintf(file, "## %6i =>[ x:%lf \t y:%lf \t z:%lf] \t (ib:%i) \n", n, x, y, z, ib);
    }
}

void plot3d_coordinateBlock_find_dims(Plot3DCoordinateBlock* cBlock, DataTypeId dataTypeId) {
    DataArray_updateRange(&cBlock->coordinates);
    cBlock->xRange = cBlock->xTuples->valueRange;
    cBlock->yRange = cBlock->yTuples->valueRange;
    cBlock->zRange = cBlock->zTuples->valueRange;
    filter_log(FILTER_LOG, "## block =>[ min_x:%+.5lf   min_y:%+.5lf   min_z:%+.5lf]   \n", cBlock->xRange.min, cBlock->yRange.min, cBlock->zRange.min);
    filter_log(FILTER_LOG, "## block =>[ max_x:%+.5lf   max_y:%+.5lf   max_z:%+.5lf]   \n", cBlock->xRange.max, cBlock->yRange.max, cBlock->zRange.max);
}
