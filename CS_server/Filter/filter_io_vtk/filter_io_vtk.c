/* 
 * File:   filter_io_vtk.c
 * Author: Andersson M, V.I.C Visible Information Center
 * Author: Minemoto H, V.I.C Visible Information Center
 *
 * Created on November 18, 2015, 5:14 PM
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "filter_io_vtk.h"

/**
 * read type of vtk dataset.
 */
RTC vtk_get_datasetInfo() {
    FilterParam *param;
    param = &Fparam;

    activeDataSet = NULL;
    active_data_reader = NULL;
    cell_types = NULL;
    n_cellTypes = 0;
    strcpy(input_dir, param->in_dir);
    vtk_getStepFileName(param->start_step);
    VtkDatasetTypes fileType = vtkDataReader_getFileType(filename);
    active_data_reader = vtkDataSetReader_New();
    vtkDataSetReader_ReadFile(active_data_reader, filename);

    filter_log(FILTER_LOG, "param->datasettype: %d\n", fileType);
    const char* c_file_path = (const char*) filename;
    switch (fileType) {
	case PBVR_VTK_STRUCTURED_POINTS:
	    Fparam.struct_grid = CHAR_TRUE;
	    activeDataSet = vtkDataSetReader_GetStructuredPointsOutput(active_data_reader);
	    break;
	case PBVR_VTK_STRUCTURED_GRID:
	    Fparam.struct_grid = CHAR_TRUE;
	    activeDataSet = vtkDataSetReader_GetStructuredGridOutput(active_data_reader);
	    break;
	case PBVR_VTK_RECTILINEAR_GRID:
	    Fparam.struct_grid = CHAR_TRUE;
	    activeDataSet = vtkDataSetReader_GetRectilinearGridOutput(active_data_reader);
	    break;
	case PBVR_VTK_UNSTRUCTURED_GRID:
	    activeDataSet = vtkDataSetReader_GetUnstructuredGridOutput(active_data_reader);
	    Fparam.struct_grid = CHAR_FALSE;
	    DEBUG(3, "Unstructured grid type\n");
	    break;
	case PBVR_VTK_POLYDATA:
	    activeDataSet = vtkDataSetReader_GetPolyDataOutput(active_data_reader);
	    Fparam.struct_grid = CHAR_FALSE;
	    DEBUG(3, "PolyData grid type\n");
	    break;
	default:
	    filter_log(FILTER_ERR, "Dataset type of ( %s) is unknown \n", filename);
	    break;
    }

    //    
    param->datasettype = fileType;
    return (RTC_OK);
}

char get_ucdElementType(int vtkCellType) {
    if (vtkCellType < MAX_ELEMENT_TYPE) {
	cellTypeEntry vtkTypeEntry = cellTypeMap[vtkCellType];
	if (vtkTypeEntry.n_verts > -1) {
	    DEBUG(1, "Converting vtk cell type %d to %d\n", vtkCellType, vtkTypeEntry.ucdElementType);
	    return cellTypeMap[vtkCellType].ucdElementType;
	}
    }
    filter_log(FILTER_ERR, "This version of PBVR filter can not convert VTK cell type %d to UCD element type\n", vtkCellType);
    ASSERT_FAIL("UCD TYPE NOT FOUND");
}

int get_vtkCellType(int ucdElementType) {
    int i;
    if (ucdElementType > 100) {
	return ucdElementType - 100;
    }
    for (i = 0; i < MAX_ELEMENT_TYPE; i++) {
	if (cellTypeMap[i].ucdElementType == ucdElementType) {
	    return i+1;
	}
    }
    DEBUG(1, "No vtk element type match for ucd element type %d\n", ucdElementType);
    ASSERT_FAIL("Can not convert unknown UCD element type to VTK cell type\n");
}

RTC vtk_getStepFileName(int step) {
    FilterParam *param;
    param = &Fparam;
    DEBUG(3, "vtk_getStepFileName %d\n", step);
    if (strlen(param->field_file) > 0) {
	sprintf(filename, "%s/%s", input_dir, param->field_file);
    } else if (strlen(param->in_prefix) > 0) {
	sprintf(filename, "%s/%s%06d.%s", input_dir, param->in_prefix, step, param->in_suffix);
    } else {
	filter_log(FILTER_ERR, "no input filename.\n");
	return (RTC_NG);
    }
    return RTC_OK;
}

/*
 Count the number of different cell types in the file.
 */
RTC vtk_count_cellTypes() {
    int c, t;
    int cell_count;
    int ucd_type_id,vtk_type_id;
    _vtkCell cell;
    cell_types = vtkCellTypes_New();

    vtkDataSet_GetCellTypes(activeDataSet, cell_types);
    n_cellTypes = vtkCellTypes_GetNumberOfTypes(cell_types);
    cell_count = vtkDataSet_GetNumberOfCells(activeDataSet);
    m_elemtypes = (char *) malloc(sizeof (char)*cell_count);
    for (t = 0; t < MAX_ELEMENT_TYPE; t++) {
	elementType_cellIDs[t] = vtkIdList_New();
	elementType_numCells[t] = 0;
	elementType_numPoints[t] = 0;
	m_types[t] = 0;
    }
    for (c = 0; c < cell_count; c++) {
	cell = vtkDataSet_GetCell(activeDataSet, c);
	vtk_type_id = vtkCell_GetCellType(cell);
	ucd_type_id = get_ucdElementType(vtk_type_id);
	int numCellPoints = vtkCell_GetNumberOfPoints(cell);
	elementType_numPoints[vtk_type_id] += numCellPoints;
	elementType_numCells[vtk_type_id]++;
	m_elemtypes[c] = (char) ucd_type_id;
	m_types[ucd_type_id]++;
    }
    for (t = 0; t < MAX_ELEMENT_TYPE; t++) {
	if (m_types[t] > 0) {
	    m_element_type=t;
	    vtkIdList_InsertNextId(elementType_cellIDs[t], c);
	    DEBUG(0, "|||| VTK_TYPE:%d size:%d count:%d\n", t, vtk_elementsize[t], m_types[t]);
	}
    }

    if (n_cellTypes > 1) {
	Fparam.mult_element_type = CHAR_TRUE;
	m_multi_element = CHAR_TRUE;	
    }
    return RTC_OK;
}

/*
 * Close the currently active grid file.
 */
void vtk_close_activeGridFile() {
    FilterParam *param;
    param = &Fparam;
    DEBUG(3, "Closing active grid file\n");
    switch (param->datasettype) {
	case PBVR_VTK_STRUCTURED_POINTS:
	    vtkStructuredPointsReader_Delete((_vtkStructuredPointsReader) active_data_reader);
	    break;
	case PBVR_VTK_STRUCTURED_GRID:
	    vtkStructuredGridReader_Delete((_vtkStructuredGridReader) active_data_reader);
	    break;
	case PBVR_VTK_RECTILINEAR_GRID:
	    vtkRectilinearGridReader_Delete((_vtkRectilinearGridReader) active_data_reader);
	    break;
	case PBVR_VTK_UNSTRUCTURED_GRID:
	    vtkUnstructuredGridReader_Delete((_vtkUnstructuredGridReader) active_data_reader);
	    break;
	case PBVR_VTK_POLYDATA:
	    vtkPolyDataReader_Delete((_vtkPolyDataReader) active_data_reader);
	    break;
    }
    active_data_reader = NULL;
}

