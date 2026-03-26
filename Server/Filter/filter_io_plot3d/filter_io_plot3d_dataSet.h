/* 
 * File:   filter_io_plot3d_gridFile.h
 * Author: Martin Andersson, V.I.C 
 *
 * Created on October 19, 2015, 3:21 PM
 */

#ifndef FILTER_IO_PLOT3D_GRIDFILE_H
#define FILTER_IO_PLOT3D_GRIDFILE_H

#include "filter_io_plot3d_coordinateBlock.h"
#include "filter_io_plot3d_functionBlock.h"
#include "filter_io_plot3d_solutionBlock.h"
#include "filter_io_plot3d.h"
#include "filter_io_plot3d_config.h"

typedef struct timeStep {
    GridFile function_file;
    GridFile solution_file;
} timeStep;

typedef struct Plot3Ddataset {
    Plot3DCoordinateBlock* coordinate_blocks;
    Plot3DFunctionBlock* function_blocks;
    Plot3DSolutionBlock* solution_blocks;

    Plot3DCoordinateBlock* active_coordinateBlock;
    Plot3DFunctionBlock* active_functionBlock;
    Plot3DSolutionBlock* active_solutionBlock;

    GridFile coordinate_file;
    timeStep* timeSteps;

    Int64* function_offsets;
    Int64* solution_offsets;
    int nSteps;
    plot3d_params* params;

    int nBlocks;
    int ncoords;

    Int64* coordinate_offsets;


    bool hasIblanks;

    Range xRange;
    Range yRange;
    Range zRange;

} Plot3Ddataset;


Plot3Ddataset* plot3d_dataSet_allocateNew(int* err_state, plot3d_params* params);
void plot3d_dataSet_closeAllFiles(Plot3Ddataset* data_set);
int plot3d_dataSet_getBlockCoordinates(Plot3Ddataset* data_set, float* target, int block);
int plot3d_dataSet_getStepData(Plot3Ddataset* data_set, float* target, int block);
int plot3d_dataSet_readStepData(Plot3Ddataset* data_set, float* target, int step, int block);
void plot3d_dataSet_indexStepFiles(Plot3Ddataset* data_set);
void plot3d_dataSet_deallocate(Plot3Ddataset* data_set);

int plot3d_dataSetG_openDataFile(Plot3Ddataset* data_set);
int plot3d_dataSetG_closeDataFile(Plot3Ddataset* data_set);
int plot3d_dataSetG_allocateBlockTypes(Plot3Ddataset* data_set, int nGrids);
int plot3d_dataSetG_readBlockCount(Plot3Ddataset* data_set);
void plot3d_dataSetG_dumpBlocks(Plot3Ddataset *data_set, const char* filePath);
int plot3d_dataSetG_readBlockDimensions(Plot3Ddataset * data_set);
int plot3d_dataSetG_readBlockData(Plot3Ddataset * data_set);
int plot3d_dataSetG_deAllocateMemory(Plot3Ddataset * data_set);
int plot3d_dataSetG_skipToBlock(Plot3Ddataset* data_set, int block);
int plot3d_dataSetG_read3DSingleBlock(Plot3Ddataset* data_set, int block);
int plot3d_dataSetG_read3DMultiBlock(Plot3Ddataset* data_set);
int plot3d_dataSetG_copyToArray(Plot3Ddataset* data_set, int step, int offset, int skip);
int plot3d_dataSetG_readCopyCoords(Plot3Ddataset* data_set, int block, int step);
void plot3d_dataSetG_findGridDimensions(Plot3Ddataset * data_set);
void plot3d_dataSetF_findValueRanges(Plot3Ddataset * data_set);

int plot3d_dataSetF_openStepFiles(Plot3Ddataset* data_set);
int plot3d_dataSetF_closeStepFiles(Plot3Ddataset* data_set);
void plot3d_dataSetS_closeStepFile(Plot3Ddataset* data_set, int step);
int plot3d_dataSetF_allocateBlockTypes(Plot3Ddataset* data_set, int nGrids);
int plot3d_dataSetF_readBlockCounts(Plot3Ddataset* data_set);
void plot3d_dataSetF_dumpBlocks(Plot3Ddataset *data_set, const char* filePath);
int plot3d_dataSetF_indexBlocks(Plot3Ddataset* data_set);
int plot3d_dataSetF_readBlockData(Plot3Ddataset* data_set, int step);
int plot3d_dataSetF_deAllocateMemory(Plot3Ddataset* data_set);
int plot3d_dataSetF_skipToBlock(Plot3Ddataset* data_set, int block, int step);
int plot3d_dataSetF_read3DSingleBlock(Plot3Ddataset* data_set, int block, int step);
int plot3d_dataSetF_read3DMultiBlock(Plot3Ddataset* data_set, int step);
int plot3d_dataSetF_copyToArray(Plot3Ddataset* data_set, int step, int offset, int skip);
int plot3d_dataSetF_readCopyFunctions(Plot3Ddataset* data_set, int step, int block);
int plot3d_dataSetF_readCopyFunctionBlock(Plot3Ddataset* data_set, float* target, int step, int block, int offset);
int plot3d_dataSetF_getCopyFunctionBlock(Plot3Ddataset* data_set, float* target, int block, int offset);


int plot3d_dataSetS_openStepFiles(Plot3Ddataset* data_set);
int plot3d_dataSetS_closeStepFiles(Plot3Ddataset* data_set);
int plot3d_dataSetS_allocateBlockTypes(Plot3Ddataset* data_set, int nGrids);
int plot3d_dataSetS_readBlockCounts(Plot3Ddataset* data_set);
void plot3d_dataSetS_dumpBlocks(Plot3Ddataset *data_set, const char* filePath);
int plot3d_dataSetS_indexBlocks(Plot3Ddataset* data_set);
int plot3d_dataSetS_readBlockData(Plot3Ddataset* data_set, int step);
int plot3d_dataSetS_deAllocateMemory(Plot3Ddataset* data_set);
int plot3d_dataSetS_skipToBlock(Plot3Ddataset* data_set, int block, int step);
int plot3d_dataSetS_read3DSingleBlock(Plot3Ddataset* data_set, int block, int step);
int plot3d_dataSetS_read3DMultiBlock(Plot3Ddataset* data_set, int step);
int plot3d_dataSetS_copyToArray(Plot3Ddataset* data_set, int step, int offset, int skip);
int plot3d_dataSetS_readCopySolution(Plot3Ddataset* data_set, int step, int block);
int plot3d_dataSetS_readCopySolutionBlock(Plot3Ddataset* data_set, float* target, int step, int block, int skip);

#endif /* FILTER_IO_PLOT3D_GRIDFILE_H */