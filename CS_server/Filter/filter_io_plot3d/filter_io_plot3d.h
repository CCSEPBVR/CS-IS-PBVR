/* 
 * File:   filter_io_plot3d_solution.h
 * Author: Martin Andersson, V.I.C 
 *
 * Created on September 25, 2015, 4:51 PM
 */

#ifndef NEWP3D_H
#define	NEWP3D_H
#include <stdbool.h>
#include <stdio.h>
#include "filter.h"
#include "filter_io_types.h"
#include "filter_io_file_reader.h"


int plot3d_sequentialConvert(int argc, char *argv[]);
int plot3d_setBlockParameters(void);
int plot3d_getBlockCoordinates();
int plot3d_getBlockValuesForTstep(int step);
#endif	/* NEWP3D_H */

