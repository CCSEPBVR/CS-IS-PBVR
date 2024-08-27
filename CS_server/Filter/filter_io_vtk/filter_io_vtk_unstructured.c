///* 
// * File:   filter_io_vtk_unstructured.h
// * Author: Andersson M, V.I.C Visible Information Center
// *
// * Created on November 18, 2015, 5:14 PM
// */
#include <float.h>

#include "filter_io_vtk.h"
#include "../filter_convert.h"
#include <string.h>

static _vtkPolyDataReader pd_reader = NULL;
static _vtkUnstructuredGridReader usg_reader = NULL;

static _vtkUnstructuredGrid unstrGridData = NULL;
static _vtkPolyData polyData = NULL;
static _vtkCellData cell_data = NULL;
static _vtkPointData point_data = NULL;
static _vtkImageData image_data = NULL;

static _vtkIdList cellIds = NULL;
static _vtkIdList pointIds = NULL;

static int n_pointData_components = 0;
static int n_pointData_arrays = 0;
static int n_pointData_tuples = 0;
static int n_cellData_arrays = 0;
static int n_cellData_components = 0;
static int n_cellData_tuples = 0;
//

static void openUnstructuredGridReader(const char* c_file_path) {
    DEBUG(3, "vtk_open_unstructuredGridReader\n");
    vtkDataSetReader_ReadFile(active_data_reader, c_file_path);
    activeDataSet = vtkDataSetReader_GetOutput(active_data_reader);
    unstrGridData = vtkDataSetReader_GetUnstructuredGridOutput(active_data_reader);
    if (DEBUG_LEVEL != 0) {
	vtkUnstructuredGrid_DumpInfo(unstrGridData);
    }
}

static void openPolyDataReader(const char* c_file_path) {
    DEBUG(1, "!!!!!!!!!!openPolyDataReader %s\n", c_file_path);
    vtkDataSetReader_ReadFile(active_data_reader, c_file_path);
    activeDataSet = vtkDataSetReader_GetOutput(active_data_reader);
    polyData = vtkDataSetReader_GetPolyDataOutput(active_data_reader);
}

/*
 Read the dimensions of unstructured griqd (or poly data)type datasets
 */
RTC vtk_SetUnstructGridParameters(int step, int element_type) {

    DEBUG(1, "############vtk_setStructGridParameters step:%d\n", step);
    FilterParam *param = &Fparam;
    int n_file_fields = 0;
    int n_file_scalars = 0;
    n_pointData_components = 0;
    n_pointData_arrays = 0;
    n_pointData_tuples = 0;
    n_cellData_arrays = 0;
    n_cellData_components = 0;
    n_cellData_tuples = 0;
    int dim_default[3] = {0, 0, 0};
    int* dims = dim_default;
    const char* c_file_path;

    point_data = NULL;
    cell_data = NULL;
    vtk_getStepFileName(step);
    c_file_path = (const char*) filename;
    switch (param->datasettype) {
	case PBVR_VTK_UNSTRUCTURED_GRID:
	    openUnstructuredGridReader(c_file_path);
	    break;
	case PBVR_VTK_POLYDATA:
	    openPolyDataReader(c_file_path);
	    break;
	default:
	    DEBUG(3, "Dataset type of ( %s) is unknown \n", filename);
	    return RTC_NG;
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
    n_cellData_arrays = vtkCellData_GetNumberOfArrays(cell_data);

    int count = 0;
    param->veclen = n_pointData_components + n_cellData_components;
    m_nnodes = vtkDataSet_GetNumberOfPoints(activeDataSet);
    m_nelements = vtkDataSet_GetNumberOfCells(activeDataSet);
    m_ids = (int *) malloc(sizeof (int)*m_nnodes);
    ASSERT_ALLOC(m_ids);
    m_elementids = (int *) malloc(sizeof (int)*m_nelements);
    m_materials = (int *) malloc(sizeof (int)*m_nelements);

    count = 0;
    m_connectsize = n_cellData_tuples * vtk_elementsize[element_type];
    m_elemcoms = vtk_elementsize[element_type];

    m_nkinds = param->veclen;

    DEBUG(3, "ndim=%ld  [ %ld, %ld, %ld]\n", param->ndim, param->dim1, param->dim2, param->dim3);
    DEBUG(3, "m_nnodes=%ld\n", m_nnodes);
    DEBUG(3, "n_file_fields=%d\n", n_file_fields);
    DEBUG(3, "n_file_scalars=%d\n", n_file_scalars);
    DEBUG(3, "n_pointData_arrays=%d\n", n_pointData_arrays);
    DEBUG(3, "n_pointData_components=%d\n", n_pointData_components);
    DEBUG(3, "n_pointData_tuples=%d\n", n_pointData_tuples);
    DEBUG(3, "n_cellData_arrays=%d\n", n_cellData_arrays);
    DEBUG(3, "n_cellData_components=%d\n", n_cellData_components);
    DEBUG(3, "n_cellData_tuples=%d\n", n_cellData_tuples);
    DEBUG(1, "veclen=%ld\n", param->veclen);

    DEBUG(1, "vtk_read_unstructured_dims end\n");
    return RTC_OK;
}

static RTC setup_mids(int element_type) {
    int n;
    int numPoints = elementType_numCells[element_type] * elementType_numPoints[element_type];
    return RTC_OK;
}

/**
 * Find coordinate range for a specific element type
 * @param element_type
 * @return 
 */
static RTC findRangeForType(int element_type) {
    int e;

    DEBUG(3, "###find_range:Finding cardinal axes min-max \n");
    double* bounds = NULL;
    double xMin = FLT_MAX;
    double yMin = FLT_MAX;
    double zMin = FLT_MAX;
    double xMax = FLT_MIN;
    double yMax = FLT_MIN;
    double zMax = FLT_MIN;
    //
    _vtkIdList cellIDs = elementType_cellIDs[element_type];
    int n_elements = vtkIdList_GetNumberOfIds(cellIDs);
    for (e = 0; e < n_elements; e++) {
	_vtkCell element = vtkDataSet_GetCell(activeDataSet, e);
	bounds = vtkCell_GetBounds(element);
	xMin = (xMin < bounds[0]) ? xMin : bounds[0];
	xMax = (xMax > bounds[1]) ? xMax : bounds[1];
	yMin = (yMin < bounds[2]) ? yMin : bounds[2];
	yMax = (yMax > bounds[3]) ? yMax : bounds[3];
	zMin = (zMin < bounds[4]) ? zMin : bounds[4];
	zMax = (zMax > bounds[5]) ? zMax : bounds[5];
    }
    DEBUG(3, "x range %lf - %lf \n", xMin, xMax);
    DEBUG(3, "y range %lf - %lf \n", yMin, yMax);
    DEBUG(3, "z range %lf - %lf \n", zMin, zMax);

    min.x = (float) xMin;
    min.y = (float) yMin;
    min.z = (float) zMin;

    max.x = (float) xMax;
    max.y = (float) yMax;
    max.z = (float) zMax;
    DEBUG(3, "###find_range: done \n");
    return RTC_OK;
}

/**
 * Find coordinate range for all types
 * @param element_type
 * @return 
 */
static RTC findRangeForAllTypes() {
    int e;

    DEBUG(3, "###find_range:Finding cardinal axes min-max \n");
    double* bounds = NULL;
    double xMin = FLT_MAX;
    double yMin = FLT_MAX;
    double zMin = FLT_MAX;
    double xMax = -FLT_MAX;
    double yMax = -FLT_MAX;
    double zMax = -FLT_MAX;

    for (e = 0; e < m_nelements; e++) {
	_vtkCell element = vtkDataSet_GetCell(activeDataSet, e);
	bounds = vtkCell_GetBounds(element);
	xMin = (xMin < bounds[0]) ? xMin : bounds[0];
	xMax = (xMax > bounds[1]) ? xMax : bounds[1];
	yMin = (yMin < bounds[2]) ? yMin : bounds[2];
	yMax = (yMax > bounds[3]) ? yMax : bounds[3];
	zMin = (zMin < bounds[4]) ? zMin : bounds[4];
	zMax = (zMax > bounds[5]) ? zMax : bounds[5];
    }
    DEBUG(3, "x range %lf - %lf \n", xMin, xMax);
    DEBUG(3, "y range %lf - %lf \n", yMin, yMax);
    DEBUG(3, "z range %lf - %lf \n", zMin, zMax);

    min.x = (float) xMin;
    min.y = (float) yMin;
    min.z = (float) zMin;

    max.x = (float) xMax;
    max.y = (float) yMax;
    max.z = (float) zMax;
    return RTC_OK;
}

/**
 * OBSOLETE NOT USED ANY MORE,  copyPoints HANDLES THIS
 * Constructs connection list using all cells
 * @param pointIDs
 * @param cellIDs
 * @param elementType
 * @return 
 */
static RTC getPointAndConnectForAllTypes(int eType) {
    DEBUG(3, "#### construct_connectionList begin\n");
    int c, e, n;
    int i = 0;
    int new_id;
    int ucd_type = get_ucdElementType(eType);
    m_connectsize = 0;
    DEBUG(3, "type:%d size:%d count:%d\n", eType, vtk_elementsize[eType], m_types[eType]);
    m_connectsize += (vtk_elementsize[eType] * m_types[ucd_type]);
    _vtkIdList included_points = vtkIdList_New();
    if (m_connections != NULL) free(m_connections);
    m_connections = malloc(m_connectsize * sizeof (*m_connections));
    ASSERT_ALLOC(m_connections);
    DEBUG(3, "Searching %ld unstructured vtk cell connections\n", m_nelements);
    for (c = 0; c < m_nelements; c++) {
	_vtkCell element = vtkDataSet_GetCell(activeDataSet, c);
	int cellType = vtkCell_GetCellType(element);
	if (cellType == eType) {
	    int n_edges = vtkCell_GetNumberOfEdges(element);
	    int n_points = vtkCell_GetNumberOfPoints(element);
	    for (e = 0; e < n_points; e++) {
		int id1 = vtkCell_GetPointId(element, e);
		new_id = 1 + vtkIdList_InsertNextId(included_points, id1);
		m_connections[i] = new_id;
		i++;
	    }
	}
    }
    m_nelements = m_types[ucd_type];
    if (m_coords != NULL) free(m_coords);
    m_coords = malloc(sizeof (float) * m_nnodes * 3);
    int numPoints = vtkIdList_GetNumberOfIds(included_points);
    int p;
    DEBUG(3, "Copying %ld unstructured vtk point coordinates\n", m_nelements);
    for (p = 0; p < numPoints; p++) {
	int pid = vtkIdList_GetId(included_points, p);
	double* point = vtkDataSet_GetPoint(activeDataSet, pid);
	m_coords[p * 3] = point[0];
	m_coords[p * 3 + 1] = point[1];
	m_coords[p * 3 + 2] = point[2];
    }
    for (n = 0; n < m_nnodes; n++) {
	m_ids[n] = n + 1;
    }
    return RTC_OK;
}

/**
 * get_PointAndCellValues
 * @param target float* pointer to target array
 * @param cellType, vtkCellType enumeration 
 * @return 
 */
static RTC get_PointAndCellValues(float* target, int ucdElementType) {
    DEBUG(3, "#### get_PointValues for element type %d\n",ucdElementType);
    int c, e, n, t, a;
    int i = 0;
    int numPointTuples;
    int vtkCellType=get_vtkCellType(ucdElementType);
    _vtkDataArray pointData_array = NULL;
    _vtkDataArray cellData_array = NULL;
    _vtkPointData pointData = vtkDataSet_GetPointData(activeDataSet);
    _vtkCellData cellData = vtkDataSet_GetCellData(activeDataSet);

    _vtkIdList pidList = vtkIdList_New();

    int numPointArrays = vtkPointData_GetNumberOfArrays(pointData);
    int numCellArrays = vtkCellData_GetNumberOfArrays(cellData);
    ASSERT_ALLOC(m_values);
    for (c = 0; c < m_nelements; c++) {
	
	_vtkCell element = vtkDataSet_GetCell(activeDataSet, c);
	int cellType = vtkCell_GetCellType(element);
//	printf("checking cell %d %d\n",c,vcellType );
	if (cellType == vtkCellType) {
	    int n_edges = vtkCell_GetNumberOfEdges(element);
	    for (e = 0; e < n_edges; e++) {
		_vtkCell edge = vtkCell_GetEdge(element, e);
		int pid = vtkCell_GetPointId(edge, 0);
//		printf( "Including cell data %d(%d) \n", pid, e);
		vtkIdList_InsertNextId(pidList, pid);
	    }
	}
    }
    int id;
    int numPointComponents;
    double* pointComponents = NULL;
    double cellComponents[128];
    double* range = NULL;
    int numIds = vtkIdList_GetNumberOfIds(pidList);
    int numCellComponents;
    int numCellTuples;
    int cmp_id;
    int pid;

    for (id = 0; id < numIds; id++) {
	pid = vtkIdList_GetId(pidList, id);
//	printf( "Including point data %d \n", pid);
	for (a = 0; a < numPointArrays; a++) {
	    pointData_array = vtkPointData_GetArray(pointData, a);
	    numPointComponents = vtkDataArray_GetNumberOfComponents(pointData_array);
	    numPointTuples = vtkDataArray_GetNumberOfTuples(pointData_array);
	    pointComponents = vtkDataArray_GetTuple(pointData_array, pid);
	    for (cmp_id = 0; cmp_id < numPointComponents; cmp_id++) {
//		DEBUG(3, "Including point data for point %d of value %lf\n", pid, pointComponents[cmp_id]);
		target[i] = (float) pointComponents[cmp_id];
		i++;
	    }
	}
	for (a = 0; a < numCellArrays; a++) {
	    cellData_array = vtkCellData_GetArray(cellData, a);
	    numCellComponents = vtkDataArray_GetNumberOfComponents(cellData_array);
	    numCellTuples = vtkDataArray_GetNumberOfTuples(cellData_array);
	    vtk_getCellDataForPoint(pointData_array, pid, cellComponents);
	    for (cmp_id = 0; cmp_id < numCellComponents; cmp_id++) {
		DEBUG(3, "Including cell data for point %d of value %lf\n", pid, cellComponents[cmp_id]);
		target[i] = (float) cellComponents[cmp_id];
		i++;
	    }
	}

	int kind = 0;
	for (a = 0; a < numPointArrays; a++) {
	    pointData_array = vtkPointData_GetArray(pointData, a);
	    numPointComponents = vtkDataArray_GetNumberOfComponents(pointData_array);
	    for (c = 0; c < numPointComponents; c++) {
		double* range = vtkDataArray_GetRange(pointData_array, c);
		m_value_min[kind] = range[0];
		m_value_max[kind] = range[1];
		kind++;
	    }
	}
	for (a = 0; a < numCellArrays; a++) {
	    cellData_array = vtkCellData_GetArray(cellData, a);
	    numCellComponents = vtkDataArray_GetNumberOfComponents(cellData_array);
	    for (c = 0; c < numCellComponents; c++) {
		double* range = vtkDataArray_GetRange(cellData_array, c);
		m_value_min[kind] = range[0];
		m_value_max[kind] = range[1];
		kind++;
	    }
	}
    }
    DEBUG(1, "#### get_PointValues  end\n");
    return RTC_OK;
}

/**
 * Get Cell data for point id and convert it to point data
 * @param dataArray
 * @param pid
 * @param pointComponents
 * @return 
 */
static inline RTC _getCellDataForPointId(_vtkDataArray dataArray, int pid, double pointComponents[]) {
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

/*
 * Get the coordinates of a structured grid 
 * and copy them into the global m_coords array
 */
RTC vtk_readUnstructuredCoordinates(int ucd_element_type) {
    int element_type = get_vtkCellType(ucd_element_type);
    DEBUG(3, "############# vtk_readUnstructuredCoordinates, element_type %d\n", element_type);
    vtk_SetUnstructGridParameters(0, element_type);
    findRangeForAllTypes();
    getPointAndConnectForAllTypes(element_type);
    Fparam.mult_element_type = (char) 0;
    return (RTC_OK);
}

/*
 *  Read values from vtk unstructured file
 */
RTC vtk_read_unstructured_values(int step, int element_type) {
    DEBUG(1, "##########vtk_read_unstructured_values for eType %d step:%d\n", element_type, step);
    m_values = malloc(m_nnodes * Fparam.veclen * sizeof (float));
    vtk_SetUnstructGridParameters(step, element_type);
    get_PointAndCellValues(m_values, element_type);
    return RTC_OK;
}
