/*
 * File:   filter_io_plot3d_dataSet.h
 * Author: Martin Andersson, V.I.C
 *
 * Created on October 19, 2015, 3:21 PM
 */

#ifndef FILTER_IO_PLOT3D_DATASET_H
#define FILTER_IO_PLOT3D_DATASET_H

#include "filter_io_plot3d/filter_io_plot3d_coordinateBlock.h"
#include "filter_io_plot3d/filter_io_plot3d_functionBlock.h"
#include "filter_io_plot3d/filter_io_plot3d_solutionBlock.h"
#include "filter_io_plot3d/filter_io_plot3d_config.h"

namespace pbvr {
namespace filter {

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

} /* namespace filter */
} /* namespace pbvr */

#endif /* FILTER_IO_PLOT3D_DATASET_H */
