/* 
 * File:   filter_io_plot3d_solution.h
 * Author: Martin Andersson, V.I.C 
 *
 * Created on September 29, 2015, 3:59 PM
 */

#ifndef FILTER_IO_PLOT3D_SOLUTION_H
#define	FILTER_IO_PLOT3D_SOLUTION_H
#include "filter_io_types.h"
#include "filter_io_file_reader.h"

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

void plot3d_solutionBlock_setDimension(Plot3DSolutionBlock* sBlock, int i, int j, int k);
void plot3d_solutionBlock_allocateMemory(Plot3DSolutionBlock* sGrid, DataTypeId typeId);
void plot3d_solutionBlock_deAllocateMemory(Plot3DSolutionBlock* sGrid);
int plot3d_solutionBlock_readWhole(Plot3DSolutionBlock* sGrid, GridFile* gFile);
void plot3d_solutionBlock_writeToArray(Plot3DSolutionBlock* sGrid,int offset, int skip);
void plot3d_solutionBlock_dump(Plot3DSolutionBlock* sGrid, FILE* file);
Int64 plot3d_solutionBlock_getDiskByteSize(Plot3DSolutionBlock* cGrid, GridFile* gFile);

#endif	/* FILTER_IO_PLOT3D_SOLUTION_H */

