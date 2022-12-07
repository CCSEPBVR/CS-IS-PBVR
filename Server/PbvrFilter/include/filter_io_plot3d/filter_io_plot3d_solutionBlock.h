/*
 * File:   filter_io_plot3d_solution.h
 * Author: Martin Andersson, V.I.C
 *
 * Created on September 29, 2015, 3:59 PM
 */

#ifndef FILTER_IO_PLOT3D_SOLUTION_H
#define	FILTER_IO_PLOT3D_SOLUTION_H
#include "filter_io_plot3d/filter_io_types.h"
#include "filter_io_plot3d/filter_io_file_reader.h"

namespace pbvr {
namespace filter {

typedef struct Plot3DSolutionBlock
{
    Dimension gridDimension;
    Int64 ncoords;
    Int64 diskByteSize;
    double mach; //free stream Mach number
    double alpha; //free stream angle-of-attack
    double reyn; //free stream Reynolds number
    double time; //time

    DataComponent* density;
    DataComponent* momentumX;
    DataComponent* momentumY;
    DataComponent* momentumZ;
    DataComponent* energyTotal;

    DataArray QValues;
    bool allocatedQValues;

} Plot3DSolutionBlock;

} /* namespace filter */
} /* namespace pbvr */

#endif	/* FILTER_IO_PLOT3D_SOLUTION_H */

