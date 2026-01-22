/* 
 * File:   filter_io_plot3d_config.h
 * Author: Martin Andersson, V.I.C 
 *
 * Created on October 20, 2015, 9:01 AM
 */

#ifndef FILTER_IO_PLOT3D_CONFIG_H
#define	FILTER_IO_PLOT3D_CONFIG_H
#include <stdbool.h>
#include "filter.h"
#include "filter_io_types.h"

// Grid type enumeration
typedef enum GridType
{
    COORDINATE_GRID, SOLUTION_GRID, FUNCTION_GRID
} grid_type;
// Grid parameter struct
typedef struct grid_params
{
    char file_prefix[FILELEN_MAX];  //File name prefix (prepend to step num)
    char file_suffix[12];           //File name suffix (append to step num)
    bool has_iblanks;               //Grid file has IBLANK information
    ReadMode readmode;              //Read mode (ASCII/BINARY)
    DataTypeId precision;           //Precision (FLOAT_T/DOUBLE_T) 
    bool use_bytecount;             //File has Fortran style byte array headers
    EndianType endian;              //File endian (BIG_ENDIAN_T|LITTLE_ENDIAN_T)
    grid_type gridType;             //File gridType (COORDINATE_GRID|SOLUTION_GRID|FUNCTION_GRID)
    int num_vars;                   //Function file number of variables
    bool use_blockcount;            //File has block count header (true|false)
    bool enabled;                   //File in use, enabled if suffix or prefix is defined
} grid_params;

// Plot3d param struct
typedef struct plot3d_params
{
    grid_params coordinate_params;
    grid_params solution_params;
    grid_params function_params;
    char fieldType[16];
} plot3d_params;
plot3d_params sParams;

/**
 * Read the plot3d configuration file
 * @return err_status
 */
int plot3d_config_read();
#endif	/* FILTER_IO_PLOT3D_CONFIG_H */

