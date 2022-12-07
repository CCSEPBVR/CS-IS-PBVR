/*
 * File:   filter_io_plot3d_function.h
 * Author: Martin Andersson, V.I.C
 *
 * Created on September 29, 2015, 4:00 PM
 */

#ifndef FILTER_IO_PLOT3D_FUNCTION_H
#define	FILTER_IO_PLOT3D_FUNCTION_H

#include "filter_io_plot3d/filter_io_types.h"
#include "filter_io_plot3d/filter_io_file_reader.h"

namespace pbvr {
namespace filter {

typedef struct Plot3DFunctionBlock
{
    Dimension gridDimension;
    Int64 diskByteSize;
    Int64 ncoords;
    int num_components;
    DataArray fValues;
    bool allocatedValues;
} Plot3DFunctionBlock;

} /* namespace filter */
} /* namespace pbvr */

#endif	/* FILTER_IO_PLOT3D_FUNCTION_H */

