/*
 * File:   wrapper_vtk_data.h
 * Author: Andersson M, V.I.C Visible Information Center
 * Author: Minemoto H, V.I.C Visible Information Center
 *
 * Created on November 18, 2015, 5:14 PM
 */
#ifndef WRAPPER_VTK_DATA_H
#define WRAPPER_VTK_DATA_H

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

namespace pbvr
{
namespace filter
{
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


} // end of namespace filter
} // end of namespace pbvr

#endif //WRAPPER_VTK_DATA_Hf
