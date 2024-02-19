/* 
 * File:   filter_io_plot3d_function.h
 * Author: Martin Andersson, V.I.C 
 *
 * Created on September 29, 2015, 4:00 PM
 */

#ifndef FILTER_IO_PLOT3D_FUNCTION_H
#define	FILTER_IO_PLOT3D_FUNCTION_H
#include "filter_io_types.h"
#include "filter_io_file_reader.h"

typedef struct Plot3DFunctionBlock
{
    Dimension gridDimension;
    Int64 diskByteSize;
    Int64 ncoords;
    int num_components;
    DataArray fValues;
    bool allocatedValues;
} Plot3DFunctionBlock;


void plot3d_functionBlock_setDimension(Plot3DFunctionBlock* fGrid, int i, int j, int k, int nvars);
void plot3d_functionBlock_allocateMemory(Plot3DFunctionBlock* fGrid, DataTypeId typeId);
void plot3d_functionBlock_deAllocateMemory(Plot3DFunctionBlock* fGrid);
int plot3d_functionBlock_readWhole(Plot3DFunctionBlock* fGrid, GridFile* gFile);
void plot3d_functionBlock_writeToArray(Plot3DFunctionBlock* fGrid, int offset, int skip);
void plot3d_functionBlock_dump(Plot3DFunctionBlock* fGrid, FILE* file);
Int64 plot3d_functionBlock_getDiskByteSize(Plot3DFunctionBlock* fBlock, GridFile* gFile);
#endif	/* FILTER_IO_PLOT3D_FUNCTION_H */

