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

#include "PbvrFilter.h"
#include "filter_utils.h"
#include "filter_log.h"
#include "filter_io_vtk/wrapper/wrapper_vtk_reader.h"
#include "filter_io_vtk/wrapper/wrapper_vtk_data.h"


namespace pbvr {
namespace filter {

typedef struct cellTypeEntry {
    int ucdElementType;
    int n_verts;
} cellTypeEntry;

class FilterIoVtk
{
public:
    FilterIoVtk(PbvrFilterInfo *filter_info);
    virtual ~FilterIoVtk();

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
    void vtkUnstructuredGrid_DumpInfo(vtkUnstructuredGrid *usg);
    void vtkPolyData_DumpInfo(vtkPolyData *pd);
    void vtkImageData_DumpInfo(vtkImageData* id);
    void vtkPointData_DumpInfo(vtkPointData* pd);
    void vtkCellData_DumpInfo(vtkCellData* cd);
    void vtkDataSet_DumpCellTypeInfo(vtkDataSet* data_Set);
    void vtkStructuredPoints_DumpInfo(vtkStructuredPoints* sp);
    void vtkRectilinearGrid_DumpInfo(vtkRectilinearGrid* rg);
    void vtkStructuredGrid_DumpInfo(vtkStructuredGrid* sg);

    // wapper::reader
    void vtkDataReader_DumpInfo(vtkDataReader* generic);
    void RectilinearGridReader_DumpInfo(vtkRectilinearGridReader *rgr);
    void StructuredPointsReader_DumpInfo(vtkStructuredPointsReader *spr);
    void StructuredGridReader_DumpInfo(vtkStructuredGridReader *sgr);
    void UnstructuredGridReader_DumpInfo(vtkUnstructuredGridReader *usgr);
    void PolygonDataReader_DumpInfo(vtkPolyDataReader* _pdr);
    void vtkPolyDataReader_ReadFile(vtkPolyDataReader *reader, const char *filepath);
    void vtkStructuredPointsReader_ReadFile(vtkStructuredPointsReader *reader, const char *filepath);
    void vtkStructuredGridReader_ReadFile(vtkStructuredGridReader *reader, const char *filepath);
    void vtkUnstructuredGridReader_ReadFile(vtkUnstructuredGridReader *reader, const char *filepath);
    void vtkRectilinearGridReader_ReadFile(vtkRectilinearGridReader *reader, const char *filepath);
    void vtkDataSetReader_ReadFile(vtkDataReader *reader, const char *filepath);
    VtkDatasetTypes vtkDataReader_getFileType(const char *filepath);

    // filter_io_vtk_structured.cpp
    void vtk_open_structuredPointsReader(const char* c_file_path);
    void vtk_open_structuredGridReader(const char* c_file_path);
    void vtk_open_rectilinearGridReader(const char* c_file_path);
    RTC vtk_setStructGridParameters(int step);
    RTC vtk_read_structured_coordinates();
    RTC vtk_getCellDataForPoint(vtkDataArray *dataArray, int pid, double pointComponents[]);
    int vtk_copy_cellDataComponents(float* target, int* offset, int stride);
    RTC vtk_copy_pointDataComponents(float* target, int* offset, int stride);
    RTC vtk_read_structured_values(int step);

    // filter_io_vtk_unstructured.cpp
    void openUnstructuredGridReader(const char* c_file_path);
    void openPolyDataReader(const char* c_file_path);
    RTC vtk_SetUnstructGridParameters(int step, int element_type);
    RTC setup_mids(int element_type);
    RTC findRangeForType(int element_type);
    RTC findRangeForAllTypes();
    RTC getPointAndConnectForAllTypes(int eType);
    RTC get_PointAndCellValues(float* target, int ucdElementType);
    RTC _getCellDataForPointId(vtkDataArray *dataArray, int pid, double pointComponents[]);
    RTC vtk_readUnstructuredCoordinates(int ucd_element_type);
    RTC vtk_read_unstructured_values(int step, int element_type);
    // add by @hira at 2016/12/01
    bool equals_vtkCellType(int vtk_type, int ucd_type);

public:

    PbvrFilterInfo *m_filter_info;
    char input_dir[128];
    int elementType_numPoints[MAX_ELEMENT_TYPE];
    int elementType_numCells[MAX_ELEMENT_TYPE];
    vtkIdList *elementType_cellIDs[MAX_ELEMENT_TYPE];
    vtkIdList *elementType_pointIDs[MAX_ELEMENT_TYPE];
    vtkDataSet *activeDataSet;
    vtkDataReader *active_data_reader;       // vtkDataSetReader -> vtkDataReader by @hira
    vtkCellTypes *cell_types;
    int n_cellTypes;

    vtkStructuredGridReader *sg_reader;
    vtkRectilinearGridReader *rg_reader;
    vtkStructuredGrid *sg_data;
    vtkStructuredPoints *sp_data;
    vtkRectilinearGrid *rg_data;

    vtkPolyDataReader *pd_reader;
    vtkUnstructuredGridReader *usg_reader;

    vtkCellData *cell_data;
    vtkPointData *point_data;
    vtkImageData *image_data;
    vtkUnstructuredGrid *unstrGridData;
    vtkPolyData *polyData;
    vtkIdList *cellIds;
    vtkIdList *pointIds;

    int n_pointData_components;
    int n_pointData_arrays;
    int n_pointData_tuples;
    int n_cellDataArrays;
    int n_cellData_components;
    int n_cellData_tuples;
    int n_cellData_arrays;

protected:

private:
    void initialize();
    static const cellTypeEntry cellTypeMap[];

};

} // end of namespace filter
} // end of namespace pbvr
#endif /* FILTER_IO_VTK_H_ */
//
