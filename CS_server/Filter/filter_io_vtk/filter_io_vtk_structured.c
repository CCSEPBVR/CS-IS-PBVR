/* 
 * File:   filter_io_vtk_structured.h
 * Author: Andersson M, V.I.C Visible Information Center
 * Author: Minemoto H, V.I.C Visible Information Center
 *
 * Created on November 18, 2015, 5:14 PM
 */
#include "filter_io_vtk.h"
#include "../filter_convert.h"
#include <string.h>
//
static _vtkStructuredPointsReader sp_reader = NULL;
static _vtkStructuredGridReader sg_reader = NULL;
static _vtkRectilinearGridReader rg_reader = NULL;
static _vtkStructuredGrid sg_data = NULL;
static _vtkStructuredPoints sp_data = NULL;
static _vtkRectilinearGrid rg_data = NULL;

static _vtkCellData cell_data = NULL;
static _vtkPointData point_data = NULL;
static _vtkImageData image_data = NULL;

static int n_pointData_components = 0;
static int n_pointData_arrays = 0;
static int n_pointData_tuples = 0;
static int n_cellDataArrays = 0;
static int n_cellData_components = 0;
static int n_cellData_tuples = 0;

void vtk_open_structuredPointsReader(const char* c_file_path) {
    DEBUG(3, "############vtk_open_structuredPointsReader\n");
    sp_reader = vtkStructuredPointsReader_New();
    vtkStructuredPointsReader_ReadFile(sp_reader, c_file_path);
    if (DEBUG_LEVEL != 0) {
	StructuredPointsReader_DumpInfo(sp_reader);
    }
    sp_data = vtkStructuredPointsReader_GetOutput(sp_reader);
    activeDataSet = (_vtkDataSet) sp_data;
    active_data_reader = (_vtkDataReader) sp_reader;
    Fparam.struct_grid = CHAR_TRUE;
}
//

void vtk_open_structuredGridReader(const char* c_file_path) {
    DEBUG(3, "############vtk_open_structuredGridReader\n");
    sg_reader = vtkStructuredGridReader_New();
    vtkStructuredGridReader_ReadFile(sg_reader, c_file_path);
    if (DEBUG_LEVEL != 0) {
	StructuredGridReader_DumpInfo(sg_reader);
    }
    sg_data = vtkStructuredGridReader_GetOutput(sg_reader);
    active_data_reader = (_vtkDataReader) sg_reader;
    activeDataSet = (_vtkDataSet) sg_data;
}
//

void vtk_open_rectilinearGridReader(const char* c_file_path) {
    DEBUG(3, "############vtk_open_rectilinearGridReader\n");
    rg_reader = vtkRectilinearGridReader_New();
    vtkRectilinearGridReader_ReadFile(rg_reader, c_file_path);
    if (DEBUG_LEVEL != 0) {
	RectilinearGridReader_DumpInfo(rg_reader);
    }
    rg_data = vtkRectilinearGridReader_GetOutput(rg_reader);
    active_data_reader = (_vtkDataReader) rg_reader;
    activeDataSet = (_vtkDataSet) rg_data;
}

/*
 Read the dimensions of the three structured grid type datasets
 */
RTC vtk_setStructGridParameters(int step) {

    DEBUG(1, "############vtk_setStructGridParameters step:%d\n", step);
    int n_file_fields = 0;
    int n_file_scalars = 0;
    n_pointData_components = 0;
    n_pointData_arrays = 0;
    n_pointData_tuples = 0;
    n_cellDataArrays = 0;
    n_cellData_components = 0;
    n_cellData_tuples = 0;
    int dim_default[3] = {0, 0, 0};
    int* dims = dim_default;
    const char* c_file_path;
    int grid_data_dim = 0;
    FilterParam *param;
    param = &Fparam;

    point_data = NULL;
    cell_data = NULL;
    vtk_getStepFileName(step);
    c_file_path = (const char*) filename;
    DEBUG(3, "\n\n\n =============== Opening %d, %s \n\n", step, c_file_path);
    if (active_data_reader != NULL) {
	vtk_close_activeGridFile();
    }
    switch (param->datasettype) {
	case PBVR_VTK_STRUCTURED_POINTS:
	    vtk_open_structuredPointsReader(c_file_path);
	    dims = vtkStructuredPoints_GetDimensions(sp_data);
	    grid_data_dim = vtkStructuredPoints_GetDataDimension(sp_data);
	    break;
	case PBVR_VTK_STRUCTURED_GRID:
	    vtk_open_structuredGridReader(c_file_path);
	    dims = vtkStructuredGrid_GetDimensions(sg_data);
	    grid_data_dim = vtkStructuredGrid_GetDataDimension(sg_data);
	    break;
	case PBVR_VTK_RECTILINEAR_GRID:
	    vtk_open_rectilinearGridReader(c_file_path);
	    dims = vtkRectilinearGrid_GetDimensions(rg_data);
	    grid_data_dim = vtkStructuredGrid_GetDataDimension(rg_data);
	    break;
    }

    n_file_fields = vtkDataReader_GetNumberOfFieldDataInFile(active_data_reader);
    n_file_scalars = vtkDataReader_GetNumberOfScalarsInFile(active_data_reader);
    point_data = vtkDataSet_GetPointData(activeDataSet);
    cell_data = vtkDataSet_GetCellData(activeDataSet);
    n_pointData_arrays = vtkPointData_GetNumberOfArrays(point_data);
    n_pointData_components = vtkPointData_GetNumberOfComponents(point_data);
    n_pointData_tuples = vtkPointData_GetNumberOfTuples(point_data);
    n_cellData_tuples = vtkCellData_GetNumberOfTuples(cell_data);
    n_cellData_components = vtkCellData_GetNumberOfComponents(cell_data);
    n_cellDataArrays = vtkCellData_GetNumberOfArrays(cell_data);

    param->ndim = (dims[2] == 0) ? 2 : 3;

    if (param->ndim == 2) {
	param->dim1 = dims[0];
	param->dim2 = dims[1];
	m_nnodes = param->dim1 * param->dim2;
	m_nelements = (param->dim1 - 1)*(param->dim2 - 1);
	m_elemcoms = 4;
	m_element_type = 3;
    } else if (param->ndim == 3) {
	param->dim1 = dims[0];
	param->dim2 = dims[1];
	param->dim3 = dims[2];
	m_nnodes = param->dim1 * param->dim2 * param->dim3;
	m_nelements = (param->dim1 - 1)*(param->dim2 - 1)*(param->dim3 - 1);
	m_elemcoms = 8;
	m_element_type = 7;
    } else {
	filter_log(FILTER_ERR, "Not support dimension : %ld\n", param->ndim);
	return (RTC_NG);
    }

    min.x = Fparam.min_ext[0] = 0;
    min.y = Fparam.min_ext[1] = 0;
    min.z = Fparam.min_ext[2] = 0;
    max.x = Fparam.max_ext[0] = param->dim1 - 1;
    max.y = Fparam.max_ext[1] = param->dim2 - 1;
    max.z = Fparam.max_ext[2] = param->dim3 - 1;
    param->veclen = n_pointData_components + n_cellData_components;
    param->nspace = grid_data_dim;
    m_connectsize = m_nelements*m_elemcoms;
    m_ncomponents = param->veclen;
    m_nkinds = param->veclen;
    m_desctype = 1;
    m_datatype = 1;

    DEBUG(3, "ndim=%ld  [ %ld, %ld, %ld]\n", param->ndim, param->dim1, param->dim2, param->dim3);
    DEBUG(3, "nspace=%d\n", grid_data_dim);
    DEBUG(3, "n_file_fields=%d\n", n_file_fields);
    DEBUG(3, "n_file_scalars=%d\n", n_file_scalars);
    DEBUG(3, "n_pointData_arrays=%d\n", n_pointData_arrays);
    DEBUG(3, "n_pointData_components=%d\n", n_pointData_components);
    DEBUG(3, "n_pointData_tuples=%d\n", n_pointData_tuples);
    DEBUG(3, "n_cellData_arrays=%d\n", n_cellDataArrays);
    DEBUG(3, "n_cellData_components=%d\n", n_cellData_components);
    DEBUG(3, "n_cellData_tuples=%d\n", n_cellData_tuples);
    DEBUG(3, "veclen=%ld\n", param->veclen);

    return RTC_OK;
}

/*
 * Get the coordinates of a structured grid 
 * and copy them into the global m_coords array
 */
RTC vtk_read_structured_coordinates() {
    FilterParam *param;
    double* point = NULL;
    double* bounds = NULL;
    int n, ci, i, j, k;
    param = &Fparam;
    m_coords = malloc(sizeof (float) * m_nnodes * 3);
    bounds = vtkDataSet_GetBounds(activeDataSet);

    min.x = bounds[0];
    max.x = bounds[1];
    min.y = bounds[2];
    max.y = bounds[3];
    min.z = bounds[4];
    max.z = bounds[5];
    DEBUG(3, "Copying  %ld structured vtk point nodes \n", m_nnodes);
    n = 0;
    for (k = 0; k < Fparam.dim3; k++) {
	for (j = 0; j < Fparam.dim2; j++) {
	    for (i = 0; i < Fparam.dim1; i++) {
		ci = n * 3;
		point = vtkDataSet_GetPoint(activeDataSet, n);
		m_coords[ci] = (float) point[0];
		m_coords[ci + 1] = (float) point[1];
		m_coords[ci + 2] = (float) point[2];
		n++;
	    }
	}
    }

    return (RTC_OK);
}

RTC vtk_getCellDataForPoint(_vtkDataArray dataArray, int pid, double pointComponents[]) {
    int c, cn;
    int numComponents;
    int numCells;

    double* cellComponents;
    _vtkIdList cidList = vtkIdList_New();
    vtkDataSet_GetPointCells(activeDataSet, pid, cidList);
    numComponents = vtkDataArray_GetNumberOfComponents(dataArray);
    numCells = vtkIdList_GetNumberOfIds(cidList);
    memset(pointComponents, 0, numComponents * (sizeof *pointComponents));
    for (cn = 0; cn < numCells; cn++) {
	int cid = vtkIdList_GetId(cidList, cn);
	cellComponents = vtkDataArray_GetTuple(dataArray, cid);
	for (c = 0; c < numComponents; c++) {
	    pointComponents[c] += cellComponents[c];
	}
    }
    for (c = 0; c < numComponents; c++) {
	pointComponents[c] = pointComponents[c] / numCells;
    }
    return RTC_OK;
}

int vtk_copy_cellDataComponents(float* target, int* offset, int stride) {
    FilterParam *param;
    param = &Fparam;
    _vtkDataArray dataArray;
    int c = 0;
    int n = 0;
    int i = 0;
    int a = 0;
    int t = 0;

    long int nTotalCellComponents = 0;
    int numTuples = 0;
    int numComponents;
//    double components[n_cellData_components];
    double *components = malloc(n_cellData_components);
    for (a = 0; a < n_cellDataArrays; a++) {
	dataArray = vtkCellData_GetArray(cell_data, a);
	numTuples = vtkDataArray_GetNumberOfTuples(dataArray);
	if (numTuples == m_nelements) {
	    i = *offset;
	    numComponents = vtkDataArray_GetNumberOfComponents(dataArray);
	    DEBUG(3, "  #:CellDataArray offset; %d  numComponents:%d, n_tuples:%d\n", *offset, numComponents, numTuples);
	    for (n = 0; n < m_nnodes; n++) {
		vtk_getCellDataForPoint(dataArray, n, components);
		for (c = 0; c < numComponents; c++) {
		    m_values[i] = (float) components[c];
		    i++;
		}
		i += stride - numComponents;
	    }
	    for (c = 0; c < numComponents; c++) {
		double* range = vtkDataArray_GetRange(dataArray, c);
		m_value_min[*offset + c] = range[0];
		m_value_max[*offset + c] = range[1];
	    }
	    *offset += numComponents;
	}
    }
    DEBUG(3, "  #:Done Copying  %ld x %ld vtk point data values. (offset:%d, n:%d) \n", m_nnodes, Fparam.veclen, *offset, n);
    free(components);
    return nTotalCellComponents;
}

RTC vtk_copy_pointDataComponents(float* target, int* offset, int stride) {
    FilterParam *param;
    param = &Fparam;
    _vtkDataArray data_array;
    double* components = NULL;
    int c = 0;
    int n = 0;
    int i = 0;
    int a = 0;
    int numComponents = 0;
    int numTuples = 0;

    // Copy array data to m_values
    for (a = 0; a < n_pointData_arrays; a++) {
	data_array = vtkPointData_GetArray(point_data, a);
	numTuples = vtkDataArray_GetNumberOfTuples(data_array);
	if (numTuples == m_nnodes) {
	    i = *offset;
	    numComponents = vtkDataArray_GetNumberOfComponents(data_array);
	    DEBUG(3, "  #:PointDataArray offset; %d  numComponents:%d, n_tuples:%d\n", *offset, numComponents, numTuples);
	    for (n = 0; n < m_nnodes; n++) {
		components = vtkDataArray_GetTuple(data_array, n);
		for (c = 0; c < numComponents; c++) {
		    m_values[i] = (float) components[c];
		    i++;
		}
		i += stride - numComponents;
	    }
	    for (c = 0; c < numComponents; c++) {
		double* range = vtkDataArray_GetRange(data_array, c);
		m_value_min[*offset + c] = range[0];
		m_value_max[*offset + c] = range[1];
	    }
	    *offset += numComponents;
	}
    }
    DEBUG(3, "  #:Done Copying  %ld x %ld vtk point data values. (offset:%d, n:%d) \n", m_nnodes, Fparam.veclen, *offset, n);
    return RTC_OK;
}

/*
 * Get the values from a structured grid 
 * and copy them into the global m_values array
 */
RTC vtk_read_structured_values(int step) {
    DEBUG(3, "#############vtk_read_structured_values\n");
    FilterParam *param;
    param = &Fparam;
    int offset = 0;

    vtk_setStructGridParameters(step);
    // Count valid point data arrays (where number of tuples equal m_nodes)
    m_values = malloc(m_nnodes * Fparam.veclen * sizeof (float));
    DEBUG(3, " ##:step:%d::Copying  %ld x %ld vtk point data values \n", step, m_nnodes, Fparam.veclen);
    vtk_copy_pointDataComponents(m_values, &offset, Fparam.veclen);
    DEBUG(3, " ##:step:%d::Deriving  %ld x %ld vtk point data values from %ld cell data values \n", step, m_nnodes, Fparam.veclen, m_nelements);
    vtk_copy_cellDataComponents(m_values, &offset, Fparam.veclen);
    fflush(stdout);
    return (RTC_OK);
}
