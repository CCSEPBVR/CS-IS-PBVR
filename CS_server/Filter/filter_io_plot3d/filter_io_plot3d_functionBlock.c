#include <stdio.h>
#include <stdlib.h>
#include "../filter.h"
#include "filter_io_plot3d_functionBlock.h"
#include "filter_io_file_reader.h"
#include "filter_io_types.h"
#include "filter_log.h"

/**
 * Set the grid dimensions
 * @param fBlock Plot3DFunctionGrid* the function grid instance to work on
 * @param i, dimension i ( the other x)
 * @param j, dimension j ( the other y)
 * @param k, dimension k ( the other z)
 * @param nvars
 */
void plot3d_functionBlock_setDimension(Plot3DFunctionBlock* fBlock, int i, int j, int k, int nvars) {
    DEBUG(3,"##plot3d_functions_setDimension [nj,ni,nk] = [%i,%i,%i,%i] \n", i, j, k, nvars);
    fBlock->gridDimension.i = i;
    fBlock->gridDimension.j = j;
    fBlock->gridDimension.k = k;
    fBlock->ncoords = i * j * k;
    fBlock->num_components = nvars;
}

/**
 * Allocate array memory for the function values
 * @param fBlock Plot3DFunctionGrid* the function grid instance to work on
 * @param typeId DataTypeId, the type of data the array will hold.
 */
void plot3d_functionBlock_allocateMemory(Plot3DFunctionBlock* fBlock, DataTypeId typeId) {
    fBlock->fValues = DataArray_create(fBlock->ncoords);
    DataArray_addComponents(&fBlock->fValues, typeId, fBlock->num_components);
    fBlock->allocatedValues = true;
    DEBUG(3,"##Allocated %i matrix arrays of type %i and size %ld  \n", fBlock->num_components, typeId, fBlock->ncoords);
}

/**
 * Free function value arrays
 * @param fBlock Plot3DFunctionGrid* the function grid instance to work on
 */
void plot3d_functionBlock_deAllocateMemory(Plot3DFunctionBlock* fBlock) {
    if (fBlock->allocatedValues == true) {
        DataArray_destruct(&fBlock->fValues);
    }
    fBlock->allocatedValues = false;
}

/**
 * Calculate the block disk size in bytes.
 * @param fBlock Plot3DFunctionGrid* the function grid instance to work on
 * @param gFile
 * @return 
 */
Int64 plot3d_functionBlock_getDiskByteSize(Plot3DFunctionBlock* fBlock, GridFile* gFile) {
    Int64 byteSize;
    byteSize = 0;
    int nPointsInBlock = byteSize;
    DataTypeId read_type = gFile->blockDataTypeId;
    if (gFile->usebytecount) {
        byteSize += 8;
    }
    byteSize += fBlock->num_components * nPointsInBlock * dataTypes[read_type].onDiskSize;
    DEBUG(3,"Setting function block byte size to %ld \n", byteSize);
    return byteSize;
}

/**
 * Read grid the grid data as whole
 * @param fBlock Plot3DFunctionGrid* the function grid instance to work on
 * @param gFile
 * @return 
 */
int plot3d_functionBlock_readWhole(Plot3DFunctionBlock* fBlock, GridFile* gFile) {
    int status = 0;
    int nx;
    int nPointsInBlock = fBlock->ncoords;
    filter_io_open_FortranBlock(gFile);

    plot3d_functionBlock_allocateMemory(fBlock, gFile->blockDataTypeId);
    DataTypeId read_type = gFile->blockDataTypeId;
    DEBUG(3,"Reading %i x %ld function grid whole of type %d \n", fBlock->num_components, fBlock->ncoords, gFile->blockDataTypeId);

    for (nx = 0; nx < fBlock->num_components; nx++) {
        DataComponent* vtArray = &(fBlock->fValues.components[nx]);
        unsigned char* ucBuffNx = vtArray->ucBuffer;
        status |= filter_io_readBuffer(gFile, ucBuffNx, nPointsInBlock, read_type);
    }

    filter_io_close_FortranBlock(gFile);
    return status;
}

/**
 * Read grid the grid data as planes
 * @param fBlock Plot3DFunctionGrid* the function grid instance to work on
 * @param gFile
 * @return 
 */
int plot3d_functionBlock_readPlanes(Plot3DFunctionBlock* fBlock, GridFile* gFile) {
    int status = 0;
    int nx, k;
    int nPointsInPlane = fBlock->gridDimension.i * fBlock->gridDimension.j;
    filter_io_open_FortranBlock(gFile);

    plot3d_functionBlock_allocateMemory(fBlock, gFile->blockDataTypeId);
    DataTypeId read_type = gFile->blockDataTypeId;
    DEBUG(3,"Reading %i x %ld function grid planes of type %d \n", fBlock->num_components, fBlock->ncoords, gFile->blockDataTypeId);
    for (k = 0; k < fBlock->gridDimension.k; k++) {
        for (nx = 0; nx < fBlock->num_components; nx++) {
            DataComponent* vtArray = &(fBlock->fValues.components[nx]);
            unsigned char* ucBuffNx = &vtArray->ucBuffer[k * nPointsInPlane];
            status |= filter_io_readBuffer(gFile, ucBuffNx, nPointsInPlane, read_type);
        }
    }
    filter_io_close_FortranBlock(gFile);
    return status;
}

/**
 * Write grid data to output array
 * @param fBlock Plot3DFunctionGrid* the function grid instance to work on
 */
void plot3d_functionBlock_writeToArray(Plot3DFunctionBlock* fBlock, int offset, int skip) {
    filter_log(FILTER_LOG, "##### plot3d_functionGrid_writeToArray\n");
    DataArray_copyAsFloat(&(fBlock->fValues), m_values, offset, skip);
};

/**
 * Dump the grid data to file
 * @param fBlock Plot3DFunctionGrid* the function grid instance to work on
 * @param file FILE* the file handle to dump to
 */
void plot3d_functionBlock_dump(Plot3DFunctionBlock* fBlock, FILE* file) {
    int n, nx;
    //fprintf(file, "##  Dumping FuncGrid ncoords=%ld\n", fBlock->ncoords);
	fprintf(file, "##  Dumping FuncGrid ncoords=%lld\n", fBlock->ncoords);
	fprintf(file, "##  Dimensions: {%i, %i, %i, (%i)}\n", fBlock->gridDimension.i, fBlock->gridDimension.j, fBlock->gridDimension.k, fBlock->num_components);
    for (n = 0; n < fBlock->ncoords; n++) {
        fprintf(file, "## %6i =>[ ", n);
        for (nx = 0; nx < fBlock->num_components; nx++) {
            //	    fprintf(file, " %i:%ld\t", nx, fBlock->vars.vtArrays[nx].data[n]);
        }
        fprintf(file, "]\n");
    }
}

