/*
 * File:   filter_io_plot3d_coordinateBlock.h
 * Author: Martin Andersson, V.I.C
 *
 * Created on September 29, 2015, 3:59 PM
 */
#ifndef FILTER_IO_PLOT3D_COORDINATEBLOCK_H
#define	FILTER_IO_PLOT3D_COORDINATEBLOCK_H
#include "filter_io_plot3d/filter_io_types.h"
#include "filter_io_plot3d/filter_io_file_reader.h"

namespace pbvr {
namespace filter {

typedef struct Plot3DCoordinateBlock
{
    Dimension gridDimension;
    Int64 ncoords;
    Int64 diskByteSize;
    bool hasIblanks;

    DataArray coordinates;
    DataComponent* xTuples;
    DataComponent* yTuples;
    DataComponent* zTuples;
    DataComponent* ibTuples;


//    int* ibTuples;

    Range xRange;
    Range yRange;
    Range zRange;

} Plot3DCoordinateBlock;

} /* namespace filter */
} /* namespace pbvr */

#endif	/* FILTER_IO_PLOT3D_COORDINATEBLOCK_H */

