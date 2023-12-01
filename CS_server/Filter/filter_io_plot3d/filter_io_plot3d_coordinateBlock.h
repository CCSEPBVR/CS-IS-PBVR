/* 
 * File:   filter_io_plot3d_whole.h
 * Author: Martin Andersson, V.I.C 
 *
 * Created on September 29, 2015, 3:59 PM
 */
#ifndef FILTER_IO_PLOT3D_WHOLE_H
#define	FILTER_IO_PLOT3D_WHOLE_H
#include "filter_io_types.h"
#include "filter_io_file_reader.h"

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

void plot3d_coordinateBlock_setDimension(Plot3DCoordinateBlock* cGrid, int i, int j, int k);
void plot3d_coordinateBlock_allocateMemory(Plot3DCoordinateBlock* cGrid, bool hasIBlank, DataTypeId typeId);
void plot3d_coordinateBlock_deAllocateMemory(Plot3DCoordinateBlock* cGrid);
int plot3d_coordinateBlock_readWhole(Plot3DCoordinateBlock* cGrid, GridFile* gFile, bool hasIBlanks);
void plot3d_coordinateBlock_writeToArray(Plot3DCoordinateBlock* cGrid, int offset, int skip);
void plot3d_coordinateBlock_dump(Plot3DCoordinateBlock* cGrid, FILE* file);
int setDiskByteSize(Plot3DCoordinateBlock* cGrid, GridFile* gFile, bool hasIBlanks) ;
void plot3d_coordinateBlock_find_dims(Plot3DCoordinateBlock* cBlock, DataTypeId dataTypeId);
   
#endif	/* FILTER_IO_PLOT3D_WHOLE_H */

