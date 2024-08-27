/* 
 * File:   wrapper_vtk_data.h
 * Author: Andersson M, V.I.C Visible Information Center
 * Author: Minemoto H, V.I.C Visible Information Center
 *
 * Created on November 18, 2015, 5:14 PM
 */
#ifndef WRAPPER_VTK_DATA_H
#define WRAPPER_VTK_DATA_H

#ifdef __cplusplus
#include "vtkImageData.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellTypes.h"
#include "vtkCellData.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkStructuredPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkRectilinearGrid.h"
#include "vtkDataSetAttributes.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkCharArray.h"
#include "vtkType.h"
#endif
#include "wrapper_vtk_macros.h"
/* same values in vtkType.h */
#define PBVR_VTK_VOID            0
#define PBVR_VTK_BIT             1
#define PBVR_VTK_CHAR            2
#define PBVR_VTK_SIGNED_CHAR    15
#define PBVR_VTK_UNSIGNED_CHAR   3
#define PBVR_VTK_SHORT           4
#define PBVR_VTK_UNSIGNED_SHORT  5
#define PBVR_VTK_INT             6
#define PBVR_VTK_UNSIGNED_INT    7
#define PBVR_VTK_LONG            8
#define PBVR_VTK_UNSIGNED_LONG   9
#define PBVR_VTK_FLOAT          10
#define PBVR_VTK_DOUBLE         11
#define PBVR_VTK_ID_TYPE        12

#ifdef __cplusplus

extern "C" {
#endif
    // Re export these to C
    typedef  enum {
	// Linear cells
	EMPTY_CELL = 0,
	VERTEX = 1,
	POLY_VERTEX = 2,
	LINE = 3,
	POLY_LINE = 4,
	TRIANGLE = 5,
	TRIANGLE_STRIP = 6,
	POLYGON = 7,
	PIXEL = 8,
	QUAD = 9,
	TETRA = 10,
	VOXEL = 11,
	HEXAHEDRON = 12,
	WEDGE = 13,
	PYRAMID = 14,
	PENTAGONAL_PRISM = 15,
	HEXAGONAL_PRISM = 16,

	// Quadratic, isoparametric cells
	QUADRATIC_EDGE = 21,
	QUADRATIC_TRIANGLE = 22,
	QUADRATIC_QUAD = 23,
	QUADRATIC_TETRA = 24,
	QUADRATIC_HEXAHEDRON = 25,
	QUADRATIC_WEDGE = 26,
	QUADRATIC_PYRAMID = 27,
	BIQUADRATIC_QUAD = 28,
	TRIQUADRATIC_HEXAHEDRON = 29,
	QUADRATIC_LINEAR_QUAD = 30,
	QUADRATIC_LINEAR_WEDGE = 31,
	BIQUADRATIC_QUADRATIC_WEDGE = 32,
	BIQUADRATIC_QUADRATIC_HEXAHEDRON = 33,
	BIQUADRATIC_TRIANGLE = 34
    } vtkExportTypes;

    WRAP_VTK_TYPE(vtkPoints);

    WRAP_VTK_TYPE(vtkIdList);
    DECLARE_VTK_CTOR(vtkIdList);
    DECLARE_VTK_DTOR(vtkIdList);
    DECLARE_VTK_GETTER(vtkIdList, GetNumberOfIds, int);
    DECLARE_VTK_GETTER_1ARG(vtkIdList, GetId, int, int);
    DECLARE_VTK_GETTER_1ARG(vtkIdList, InsertUniqueId, int, int);
    DECLARE_VTK_GETTER_1ARG(vtkIdList, InsertNextId, int, int);
    /*
	vtkCell Wrappers
     */
    WRAP_VTK_TYPE(vtkCell);
    DECLARE_VTK_CTOR(vtkCell);
    DECLARE_VTK_DTOR(vtkCell);
    DECLARE_VTK_GETTER(vtkCell, GetPoints, _vtkPoints);
    DECLARE_VTK_GETTER(vtkCell, GetCellType, int);
    DECLARE_VTK_GETTER(vtkCell, GetNumberOfPoints, int);
    DECLARE_VTK_GETTER(vtkCell, GetNumberOfEdges, int);
    DECLARE_VTK_GETTER(vtkCell, GetBounds, double*);
    DECLARE_VTK_GETTER_1ARG(vtkCell, GetEdge, _vtkCell, int);
    DECLARE_VTK_GETTER_1ARG(vtkCell, GetPointId, int, int);
    /*
	 vtkCellTypes Wrapper
     */
    WRAP_VTK_TYPE(vtkCellTypes);
    DECLARE_VTK_CTOR(vtkCellTypes);
    DECLARE_VTK_DTOR(vtkCellTypes);
    DECLARE_VTK_GETTER(vtkCellTypes, GetNumberOfTypes, int);
    DECLARE_VTK_GETTER_1ARG(vtkCellTypes, GetCellType, int, int);
    DECLARE_VTK_GETTER_1ARG(vtkCellTypes, GetClassNameFromTypeId, const char*, int);
    /*
	vtkDataArray Wrappers
     */
    WRAP_VTK_TYPE(vtkDataArray);
    DECLARE_VTK_CTOR(vtkDataArray);
    DECLARE_VTK_DTOR(vtkDataArray);
    DECLARE_VTK_GETTER(vtkDataArray, GetName, const char*);
    DECLARE_VTK_GETTER(vtkDataArray, GetDataTypeAsString, const char*);
    DECLARE_VTK_GETTER(vtkDataArray, GetNumberOfComponents, int);
    DECLARE_VTK_GETTER(vtkDataArray, GetNumberOfTuples, int);
    DECLARE_VTK_GETTER_1ARG(vtkDataArray, GetTuple, double*, int);
    DECLARE_VTK_GETTER_1ARG(vtkDataArray, GetRange, double*, int);
    /*
	vtkPointData Wrappers
     */
    WRAP_VTK_TYPE(vtkPointData);
    DECLARE_VTK_CTOR(vtkPointData);
    DECLARE_VTK_DTOR(vtkPointData);
    DECLARE_VTK_GETTER(vtkPointData, GetNumberOfArrays, int);
    DECLARE_VTK_GETTER(vtkPointData, GetNumberOfComponents, int);
    DECLARE_VTK_GETTER(vtkPointData, GetNumberOfTuples, int);
    DECLARE_VTK_GETTER_1ARG(vtkPointData, GetArray, _vtkDataArray, int);
    /*
	vtkCellData Wrappers
     */
    WRAP_VTK_TYPE(vtkCellData);
    DECLARE_VTK_CTOR(vtkCellData);
    DECLARE_VTK_DTOR(vtkCellData);
    DECLARE_VTK_GETTER(vtkCellData, GetNumberOfArrays, int);
    DECLARE_VTK_GETTER(vtkCellData, GetNumberOfComponents, int);
    DECLARE_VTK_GETTER(vtkCellData, GetNumberOfTuples, int);
    DECLARE_VTK_GETTER_1ARG(vtkCellData, GetArray, _vtkDataArray, int);
    /*
	   vtkStructuredPoints Wrappers
     */
    WRAP_VTK_TYPE(vtkStructuredPoints);
    DECLARE_VTK_CTOR(vtkStructuredPoints);
    DECLARE_VTK_DTOR(vtkStructuredPoints);
    DECLARE_VTK_GETTER(vtkStructuredPoints, GetDataDimension, int);
    DECLARE_VTK_GETTER(vtkStructuredPoints, GetDimensions, int*);
    DECLARE_VTK_GETTER(vtkStructuredPoints, GetNumberOfPoints, int);
    DECLARE_VTK_GETTER(vtkStructuredPoints, GetNumberOfScalarComponents, int);
    DECLARE_VTK_GETTER(vtkStructuredPoints, GetNumberOfCells, int);
    DECLARE_VTK_GETTER(vtkStructuredPoints, GetPointData, _vtkPointData);
    DECLARE_VTK_GETTER(vtkStructuredPoints, GetCellData, _vtkCellData);
    DECLARE_VTK_GETTER_1ARG(vtkStructuredPoints, GetCell, _vtkCell, int);
    DECLARE_VTK_GETTER_1ARG(vtkStructuredPoints, GetPoint, double*, int);
    /*
	vtkStructuredGrid Wrappers
     */
    WRAP_VTK_TYPE(vtkStructuredGrid);
    DECLARE_VTK_CTOR(vtkStructuredGrid);
    DECLARE_VTK_DTOR(vtkStructuredGrid);
    DECLARE_VTK_GETTER(vtkStructuredGrid, GetDataDimension, int);
    DECLARE_VTK_GETTER(vtkStructuredGrid, GetDimensions, int*);
    DECLARE_VTK_GETTER(vtkStructuredGrid, GetNumberOfPoints, int);
    DECLARE_VTK_GETTER(vtkStructuredGrid, GetNumberOfCells, int);
    DECLARE_VTK_GETTER(vtkStructuredGrid, GetPointData, _vtkPointData);
    DECLARE_VTK_GETTER(vtkStructuredGrid, GetCellData, _vtkCellData);
    DECLARE_VTK_GETTER_1ARG(vtkStructuredGrid, GetCell, _vtkCell, int);
    DECLARE_VTK_GETTER_1ARG(vtkStructuredGrid, GetPoint, double*, int);
    /*
	vtkRectilinearGrid Wrappers
     */
    WRAP_VTK_TYPE(vtkRectilinearGrid);
    DECLARE_VTK_CTOR(vtkRectilinearGrid);
    DECLARE_VTK_DTOR(vtkRectilinearGrid);
    DECLARE_VTK_GETTER(vtkRectilinearGrid, GetDataDimension, int);
    DECLARE_VTK_GETTER(vtkRectilinearGrid, GetDimensions, int*);
    DECLARE_VTK_GETTER(vtkRectilinearGrid, GetNumberOfPoints, int);
    DECLARE_VTK_GETTER(vtkRectilinearGrid, GetNumberOfCells, int);
    DECLARE_VTK_GETTER(vtkRectilinearGrid, GetPointData, _vtkPointData);
    DECLARE_VTK_GETTER(vtkRectilinearGrid, GetCellData, _vtkCellData);
    DECLARE_VTK_GETTER_1ARG(vtkRectilinearGrid, GetCell, _vtkCell, int);
    DECLARE_VTK_GETTER_1ARG(vtkRectilinearGrid, GetPoint, double*, int);
    /*
	vtkUnstructuredGrid Wrappers
     */
    WRAP_VTK_TYPE(vtkUnstructuredGrid);
    DECLARE_VTK_CTOR(vtkUnstructuredGrid);
    DECLARE_VTK_DTOR(vtkUnstructuredGrid);
    DECLARE_VTK_GETTER(vtkUnstructuredGrid, GetNumberOfPoints, int);
    DECLARE_VTK_GETTER(vtkUnstructuredGrid, GetNumberOfCells, int);
    DECLARE_VTK_GETTER(vtkUnstructuredGrid, GetPointData, _vtkPointData);
    DECLARE_VTK_GETTER(vtkUnstructuredGrid, GetCellData, _vtkCellData);
    DECLARE_VTK_GETTER_1ARG(vtkUnstructuredGrid, GetCell, _vtkCell, int);
    DECLARE_VTK_GETTER_1ARG(vtkUnstructuredGrid, GetPoint, double*, int);

    /*
	vtkPolyData Wrappers
     */
    WRAP_VTK_TYPE(vtkPolyData);
    DECLARE_VTK_CTOR(vtkPolyData);
    DECLARE_VTK_DTOR(vtkPolyData);
    DECLARE_VTK_GETTER(vtkPolyData, GetNumberOfPoints, int);
    DECLARE_VTK_GETTER(vtkPolyData, GetNumberOfPolys, int);
    DECLARE_VTK_GETTER(vtkPolyData, GetNumberOfVerts, int);
    DECLARE_VTK_GETTER(vtkPolyData, GetNumberOfLines, int);
    DECLARE_VTK_GETTER(vtkPolyData, GetNumberOfCells, int);
    DECLARE_VTK_GETTER(vtkPolyData, GetPointData, _vtkPointData);
    DECLARE_VTK_GETTER(vtkPolyData, GetCellData, _vtkCellData);
    DECLARE_VTK_GETTER_1ARG(vtkPolyData, GetCell, _vtkCell, int);
    /*
	vtkImageData Wrappers
     */
    WRAP_VTK_TYPE(vtkImageData);
    DECLARE_VTK_CTOR(vtkImageData);
    DECLARE_VTK_DTOR(vtkImageData);
    DECLARE_VTK_GETTER(vtkImageData, GetDataDimension, int);
    DECLARE_VTK_GETTER(vtkImageData, GetDimensions, int*);
    DECLARE_VTK_GETTER(vtkImageData, GetNumberOfPoints, int);
    DECLARE_VTK_GETTER(vtkImageData, GetNumberOfScalarComponents, int);
    DECLARE_VTK_GETTER(vtkImageData, GetNumberOfCells, int);
    DECLARE_VTK_GETTER_1ARG(vtkImageData, GetPoint, double*, int);
    /*
	vtkDataSet Wrappers
     */
    WRAP_VTK_TYPE(vtkDataSet);
    DECLARE_VTK_CTOR(vtkDataSet);
    DECLARE_VTK_DTOR(vtkDataSet);
    DECLARE_VTK_GETTER(vtkDataSet, GetNumberOfCells, int);
    DECLARE_VTK_GETTER(vtkDataSet, GetNumberOfPoints, int);
    DECLARE_VTK_GETTER(vtkDataSet, GetPointData, _vtkPointData);
    DECLARE_VTK_GETTER(vtkDataSet, GetCellData, _vtkCellData);
    DECLARE_VTK_GETTER(vtkDataSet, GetBounds, double*);
    DECLARE_VTK_GETTER_1ARG(vtkDataSet, GetPoint, double*, int);
    DECLARE_VTK_GETTER_1ARG(vtkDataSet, GetCell, _vtkCell, int);
    DECLARE_VTK_GETTER_1ARG(vtkDataSet, GetCellTypes, void, _vtkCellTypes);
    DECLARE_VTK_GETTER_2ARG(vtkDataSet, GetPointCells, void, int, _vtkIdList);
    
    /*
	vtkDataSetAttributes Wrappers
     */
    WRAP_VTK_TYPE(vtkDataSetAttributes);
    DECLARE_VTK_CTOR(vtkDataSetAttributes);
    DECLARE_VTK_DTOR(vtkDataSetAttributes);
    DECLARE_VTK_GETTER(vtkDataSetAttributes, GetScalars, _vtkDataArray);

    void vtkUnstructuredGrid_DumpInfo(_vtkUnstructuredGrid usg);
    void vtkPolyData_DumpInfo(_vtkPolyData pd);
#ifdef __cplusplus
};

//Internal use (C++ only)
void vtkImageData_DumpInfo(vtkImageData* id);
void vtkPointData_DumpInfo(vtkPointData* pd);
void vtkCellData_DumpInfo(vtkCellData* cd);
void vtkDataSet_DumpCellTypeInfo(vtkDataSet* data_Set);
void vtkStructuredPoints_DumpInfo(vtkStructuredPoints* sp);
void vtkRectilinearGrid_DumpInfo(vtkRectilinearGrid* rg);
void vtkStructuredGrid_DumpInfo(vtkStructuredGrid* sg);


#endif


#endif //WRAPPER_VTK_DATA_Hf