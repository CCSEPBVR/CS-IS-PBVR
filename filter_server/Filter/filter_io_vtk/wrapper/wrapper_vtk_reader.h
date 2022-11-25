/* 
 * File:   wrapper_vtk_reader.h
 * Author: Andersson M, V.I.C Visible Information Center
 * Author: Minemoto H, V.I.C Visible Information Center
 *
 * Created on November 18, 2015, 5:14 PM
 */
#ifndef WRAPPER_VTK_READER_H
#define WRAPPER_VTK_READER_H

#ifdef __cplusplus
#include "vtkDataReader.h"
#include "vtkDataSetReader.h"
#include "vtkPolyDataReader.h"
#include "vtkStructuredPointsReader.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkStructuredGridReader.h"
#include "vtkRectilinearGridReader.h"
#endif
#include "wrapper_vtk_macros.h"
#include "wrapper_vtk_data.h"

/* */
typedef enum {
    PBVR_VTK_UNKNOWN = -1,
    PBVR_VTK_STRUCTURED_POINTS = 0,
    PBVR_VTK_RECTILINEAR_GRID,
    PBVR_VTK_STRUCTURED_GRID,
    PBVR_VTK_POLYDATA,
    PBVR_VTK_UNSTRUCTURED_GRID,
} VtkDatasetTypes;

#ifdef __cplusplus
extern "C" {
#endif

    /*  
     * Declare VTK vtkStructuredPointsReader wrappers 
     */
    WRAP_VTK_TYPE(vtkStructuredPointsReader);
    DECLARE_VTK_CTOR(vtkStructuredPointsReader);
    DECLARE_VTK_DTOR(vtkStructuredPointsReader);
    DECLARE_VTK_GETTER(vtkStructuredPointsReader, GetOutput, _vtkStructuredPoints);
    DECLARE_VTK_GETTER(vtkStructuredPointsReader, GetNumberOfScalarsInFile, int);
    DECLARE_VTK_GETTER(vtkStructuredPointsReader, GetNumberOfFieldDataInFile, int);
    //Convenience
    void vtkStructuredPointsReader_ReadFile(_vtkStructuredPointsReader, const char*);
    void StructuredPointsReader_DumpInfo(_vtkStructuredPointsReader);
    /*  
     * Declare VTK vtkStructuredGridReader wrappers 
     */
    WRAP_VTK_TYPE(vtkStructuredGridReader);
    DECLARE_VTK_CTOR(vtkStructuredGridReader);
    DECLARE_VTK_DTOR(vtkStructuredGridReader);
    DECLARE_VTK_GETTER(vtkStructuredGridReader, GetOutput, _vtkStructuredGrid);
    DECLARE_VTK_GETTER(vtkStructuredGridReader, GetNumberOfScalarsInFile, int);
    DECLARE_VTK_GETTER(vtkStructuredGridReader, GetNumberOfFieldDataInFile, int);
    //Convenience
    void vtkStructuredGridReader_ReadFile(_vtkStructuredGridReader, const char*);
    void StructuredGridReader_DumpInfo(_vtkStructuredGridReader);
    /*  
     * Declare VTK vtkRectilinearGridReader wrappers 
     */
    WRAP_VTK_TYPE(vtkRectilinearGridReader);
    DECLARE_VTK_CTOR(vtkRectilinearGridReader);
    DECLARE_VTK_DTOR(vtkRectilinearGridReader);
    DECLARE_VTK_GETTER(vtkRectilinearGridReader, GetOutput, _vtkRectilinearGrid);
    DECLARE_VTK_GETTER(vtkRectilinearGridReader, GetNumberOfScalarsInFile, int);
    DECLARE_VTK_GETTER(vtkRectilinearGridReader, GetNumberOfFieldDataInFile, int);
    //Convenience
    void RectilinearGridReader_DumpInfo(_vtkRectilinearGridReader);
    void vtkRectilinearGridReader_ReadFile(_vtkRectilinearGridReader, const char*);
    /*  
     * Declare VTK vtkUnstructuredGridReader wrappers 
     */
    WRAP_VTK_TYPE(vtkUnstructuredGridReader);
    DECLARE_VTK_CTOR(vtkUnstructuredGridReader);
    DECLARE_VTK_DTOR(vtkUnstructuredGridReader);
    DECLARE_VTK_GETTER(vtkUnstructuredGridReader, GetOutput, _vtkUnstructuredGrid);
    DECLARE_VTK_GETTER(vtkUnstructuredGridReader, GetNumberOfScalarsInFile, int);
    DECLARE_VTK_GETTER(vtkUnstructuredGridReader, GetNumberOfFieldDataInFile, int);
    //Convenience
    void UnstructuredGridReader_DumpInfo(_vtkUnstructuredGridReader);
    void vtkUnstructuredGridReader_ReadFile(_vtkUnstructuredGridReader, const char*);
    /*  
     * Declare VTK vtkPolyDataReader wrappers 
     */
    WRAP_VTK_TYPE(vtkPolyDataReader);
    DECLARE_VTK_CTOR(vtkPolyDataReader);
    DECLARE_VTK_DTOR(vtkPolyDataReader);
    DECLARE_VTK_GETTER(vtkPolyDataReader, GetOutput, _vtkPolyData);
    DECLARE_VTK_GETTER(vtkPolyDataReader, GetNumberOfScalarsInFile, int);
    DECLARE_VTK_GETTER(vtkPolyDataReader, GetNumberOfFieldDataInFile, int);
    //Convenience
    void PolygonDataReader_DumpInfo(_vtkPolyDataReader);
    void vtkPolyDataReader_ReadFile(_vtkPolyDataReader, const char*);
    /*  
     * Declare VTK vtkDataReader wrappers 
     */
    WRAP_VTK_TYPE(vtkDataReader);
    DECLARE_VTK_CTOR(vtkDataReader);
    DECLARE_VTK_DTOR(vtkDataReader);
    DECLARE_VTK_GETTER(vtkDataReader, GetNumberOfFieldDataInFile, int);
    DECLARE_VTK_GETTER(vtkDataReader, GetNumberOfScalarsInFile, int);
    //    DECLARE_VTK_GETTER(vtkDataReader, GetOutput, _vtkDataSet);

    WRAP_VTK_TYPE(vtkDataSetReader);
    DECLARE_VTK_CTOR(vtkDataSetReader);
    DECLARE_VTK_DTOR(vtkDataSetReader);
    DECLARE_VTK_GETTER(vtkDataSetReader, GetNumberOfFieldDataInFile, int);
    DECLARE_VTK_GETTER(vtkDataSetReader, GetNumberOfScalarsInFile, int);
    DECLARE_VTK_GETTER(vtkDataSetReader, GetOutput, _vtkDataSet);
    DECLARE_VTK_GETTER(vtkDataSetReader, GetPolyDataOutput, _vtkPolyData);
    DECLARE_VTK_GETTER(vtkDataSetReader, GetUnstructuredGridOutput, _vtkUnstructuredGrid);
    DECLARE_VTK_GETTER(vtkDataSetReader, GetStructuredGridOutput, _vtkStructuredGrid);
    DECLARE_VTK_GETTER(vtkDataSetReader, GetRectilinearGridOutput, _vtkRectilinearGrid);
    DECLARE_VTK_GETTER(vtkDataSetReader, GetStructuredPointsOutput, _vtkStructuredPoints);

    //Convenience
    VtkDatasetTypes vtkDataReader_getFileType(const char *filepath);
    void vtkDataSetReader_ReadFile(_vtkDataReader _reader, const char *filepath);

#ifdef __cplusplus
};
#endif

#endif // WRAPPER_VTK_READER_H

