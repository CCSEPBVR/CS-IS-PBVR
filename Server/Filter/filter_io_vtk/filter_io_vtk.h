/* 
 * File:   filter_io_vtk.h
 * Author: Andersson M, V.I.C Visible Information Center
 * Author: Minemoto H, V.I.C Visible Information Center
 *
 * Created on November 18, 2015, 5:14 PM
 */
#ifndef FILTER_IO_VTK_H_
#define FILTER_IO_VTK_H_

#define CHAR_TRUE (char) 1
#define CHAR_FALSE (char) 0

#include "filter.h"
#include "filter_utils.h"
#include "filter_log.h"
#include "wrapper/wrapper_vtk_reader.h"
#include "wrapper/wrapper_vtk_data.h"
//
char input_dir[128];

typedef struct cellTypeEntry {
    int ucdElementType;
    int n_verts;
} cellTypeEntry;

static const cellTypeEntry cellTypeMap[] = {
    [EMPTY_CELL] =
    {
	.ucdElementType = 100,
	.n_verts = 0
    },
    [VERTEX] =
    {
	.ucdElementType = 0,
	.n_verts = 1
    },
    [POLY_VERTEX] =
    {
	.ucdElementType = 102,
	.n_verts = -1
    },
    [LINE] =
    {
	.ucdElementType = 1,
	.n_verts = 2
    },
    [POLY_LINE] =
    {
	.ucdElementType = 104,
	.n_verts = -1
    },
    [TRIANGLE] =
    {
	.ucdElementType = 2,
	.n_verts = 3
    },
    [TRIANGLE_STRIP] =
    {
	.ucdElementType = 106,
	.n_verts = -1
    },
    [POLYGON] =
    {
	.ucdElementType = 107,
	.n_verts = -1
    },
    [PIXEL] =
    {
	.ucdElementType = 3,
	.n_verts = 4
    },
    [QUAD] =
    {
	.ucdElementType = 3,
	.n_verts = 4
    },
    [TETRA] =
    {
	.ucdElementType = 4,
	.n_verts = 4
    },
    [VOXEL] =
    {
	.ucdElementType = 7,
	.n_verts = 8
    },
    [HEXAHEDRON] =
    {
	.ucdElementType = 7,
	.n_verts = 8
    },
    [WEDGE] =
    {
	.ucdElementType = 6,
	.n_verts = 6
    },
    [PYRAMID] =
    {
	.ucdElementType = 5,
	.n_verts = 5
    },
};

int elementType_numPoints[MAX_ELEMENT_TYPE];
int elementType_numCells[MAX_ELEMENT_TYPE];
_vtkIdList elementType_cellIDs[MAX_ELEMENT_TYPE];
_vtkIdList elementType_pointIDs[MAX_ELEMENT_TYPE];
//
_vtkDataSet activeDataSet;
_vtkDataSetReader active_data_reader;
//
int n_cellTypes;
_vtkCellTypes cell_types;
//
//
RTC vtk_get_datasetInfo();
void vtk_close_activeGridFile();
RTC vtk_getStepFileName(int step);
////RTC vtk_read_dims();
RTC vtk_count_cellTypes();
RTC vtk_count_cellTypes_minimal();

// VTK <-> UCD element type conversion
char get_ucdElementType(int vtkCellType);
int get_vtkCellType(int ucdElementType);
RTC vtk_getCellDataForPoint(_vtkDataArray dataArray, int pid, double pointComponents[]);

//
RTC vtk_setStructGridParameters(int step);
RTC vtk_read_structured_coordinates();
RTC vtk_read_structured_values(int step);

void vtk_open_rectilinearGridReader(const char* c_file_path);
void vtk_open_structuredGridReader(const char* c_file_path);
void vtk_open_structuredPointsReader(const char* c_file_path);
//
RTC vtk_SetUnstructGridParameters(int step, int element_type);
RTC vtk_readUnstructuredCoordinates(int element_type);
RTC vtk_read_unstructured_values(int step, int element_type);

//
#endif /* FILTER_IO_VTK_H_ */
//
