///*
// * File:   filter_io_vtk_unstructured.h
// * Author: Andersson M, V.I.C Visible Information Center
// *
// * Created on November 18, 2015, 5:14 PM
// */
#include <stdio.h>
#include <string.h>
#include <float.h>

#include "filter_io_vtk/FilterIoVtk.h"

namespace pbvr {
namespace filter {

void FilterIoVtk::openUnstructuredGridReader(const char* c_file_path) {
    DEBUG(3, "vtk_open_unstructuredGridReader\n");

    // add by @hira at 2016/12/01
    if (active_data_reader == NULL) {
        active_data_reader = vtkDataSetReader::New();
    }
    vtkDataSetReader *reader = dynamic_cast<vtkDataSetReader*>(active_data_reader);
    if (reader == NULL) {
        ASSERT_FAIL(" Cannot cast active_data_reader to vtkDataSetReader.");
        return;
    }

    // modify from active_data_reader to reader by @hira at 2016/12/01
    vtkDataSetReader_ReadFile(reader, c_file_path);
    activeDataSet = reader->GetOutput();
    unstrGridData = reader->GetUnstructuredGridOutput();
    if (DEBUG_LEVEL != 0) {
        vtkUnstructuredGrid_DumpInfo(unstrGridData);
    }
}

void FilterIoVtk::openPolyDataReader(const char* c_file_path) {
    DEBUG(1, "!!!!!!!!!!openPolyDataReader %s\n", c_file_path);

    // add by @hira at 2016/12/01
    vtkDataSetReader *reader = dynamic_cast<vtkDataSetReader*>(active_data_reader);
    if (reader == NULL) {
        ASSERT_FAIL(" Cannot cast active_data_reader to vtkDataSetReader.");
        return;
    }
    // modify from active_data_reader to reader by @hira at 2016/12/01
    vtkDataSetReader_ReadFile(reader, c_file_path);
    activeDataSet = reader->GetOutput();
    polyData = reader->GetPolyDataOutput();

}

/*
 Read the dimensions of unstructured griqd (or poly data)type datasets
 */
RTC FilterIoVtk::vtk_SetUnstructGridParameters(int step, int element_type) {

    DEBUG(1, "############vtk_setStructGridParameters step:%d\n", step);

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

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
    c_file_path = (const char*) info->filename;

    if (active_data_reader != NULL) {
        vtk_close_activeGridFile();
    }

    switch (param->datasettype) {
    case PBVR_VTK_UNSTRUCTURED_GRID:
        openUnstructuredGridReader(c_file_path);
        break;
    case PBVR_VTK_POLYDATA:
        openPolyDataReader(c_file_path);
        break;
    default:
        DEBUG(3, "Dataset type of ( %s) is unknown \n", info->filename);
        return RTC_NG;
    }

    n_file_fields = active_data_reader->GetNumberOfFieldDataInFile();
    n_file_scalars = active_data_reader->GetNumberOfScalarsInFile();
    point_data = activeDataSet->GetPointData();
    cell_data = activeDataSet->GetCellData();
    n_pointData_arrays = point_data->GetNumberOfArrays();
    n_pointData_components = point_data->GetNumberOfComponents();
    n_pointData_tuples = point_data->GetNumberOfTuples();
    n_cellData_tuples = cell_data->GetNumberOfTuples();
    n_cellData_components = cell_data->GetNumberOfComponents();
    n_cellData_arrays = cell_data->GetNumberOfArrays();

    int count = 0;
    param->veclen = n_pointData_components + n_cellData_components;
    info->m_nnodes = activeDataSet->GetNumberOfPoints();
    info->m_nelements = activeDataSet->GetNumberOfCells();
    if (info->m_ids != NULL) free(info->m_ids);
    info->m_ids = (int *) malloc(sizeof (int)*info->m_nnodes);
    ASSERT_ALLOC(info->m_ids);
    if (info->m_elementids != NULL) free(info->m_elementids);
    info->m_elementids = (int *) malloc(sizeof (int)*info->m_nelements);
    if (info->m_materials != NULL) free(info->m_materials);
    info->m_materials = (int *) malloc(sizeof (int)*info->m_nelements);

    count = 0;
    // delete by @hira at 2017/03/15
    // info->m_connectsize = n_cellData_tuples * vtk_elementsize[element_type];
    info->m_elemcoms = vtk_elementsize[element_type];

    info->m_nkinds = param->veclen;

    DEBUG(3, "ndim=%ld  [ %ld, %ld, %ld]\n", param->ndim, param->dim1, param->dim2, param->dim3);
    DEBUG(3, "m_nnodes=%ld\n", info->m_nnodes);
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

RTC FilterIoVtk::setup_mids(int element_type) {
    int n;
    int numPoints = elementType_numCells[element_type] * elementType_numPoints[element_type];
    return RTC_OK;
}

/**
 * Find coordinate range for a specific element type
 * @param element_type
 * @return
 */
RTC FilterIoVtk::findRangeForType(int element_type) {
    int e;

    DEBUG(3, "###find_range:Finding cardinal axes min-max \n");

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    double* bounds = NULL;
    double xMin = FLT_MAX;
    double yMin = FLT_MAX;
    double zMin = FLT_MAX;
    double xMax = FLT_MIN;
    double yMax = FLT_MIN;
    double zMax = FLT_MIN;
    //
    vtkIdList *cellIDs = elementType_cellIDs[element_type];
    int n_elements = cellIDs->GetNumberOfIds();
    for (e = 0; e < n_elements; e++) {
        vtkCell *element = activeDataSet->GetCell(e);
        bounds = element->GetBounds();
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

    info->node_min.x = (float) xMin;
    info->node_min.y = (float) yMin;
    info->node_min.z = (float) zMin;

    info->node_max.x = (float) xMax;
    info->node_max.y = (float) yMax;
    info->node_max.z = (float) zMax;
    DEBUG(3, "###find_range: done \n");
    return RTC_OK;
}

/**
 * Find coordinate range for all types
 * @param element_type
 * @return
 */
RTC FilterIoVtk::findRangeForAllTypes() {
    int e;

    DEBUG(3, "###find_range:Finding cardinal axes min-max \n");

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    double* bounds = NULL;
    double xMin = FLT_MAX;
    double yMin = FLT_MAX;
    double zMin = FLT_MAX;
    double xMax = -FLT_MAX;
    double yMax = -FLT_MAX;
    double zMax = -FLT_MAX;

    for (e = 0; e < info->m_nelements; e++) {
        vtkCell *element = activeDataSet->GetCell(e);
        bounds = element->GetBounds();
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

    info->node_min.x = (float) xMin;
    info->node_min.y = (float) yMin;
    info->node_min.z = (float) zMin;
    info->node_max.x = (float) xMax;
    info->node_max.y = (float) yMax;
    info->node_max.z = (float) zMax;

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
RTC FilterIoVtk::getPointAndConnectForAllTypes(int eType) {
    DEBUG(3, "#### construct_connectionList begin\n");

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    int c, e, n;
    int i = 0;
    int new_id;
    int ucd_type = get_ucdElementType(eType);

    info->m_connectsize = 0;
    DEBUG(3, "type:%d size:%d count:%d\n", eType, vtk_elementsize[eType], info->m_types[eType]);
    info->m_connectsize += (vtk_elementsize[eType] *info-> m_types[ucd_type]);
    vtkIdList *included_points = vtkIdList::New();
    if (info->m_connections != NULL) free(info->m_connections);
    info->m_connections = (int*)malloc(info->m_connectsize * sizeof (*info->m_connections));
    ASSERT_ALLOC(info->m_connections);
    DEBUG(3, "Searching %ld unstructured vtk cell connections\n", info->m_nelements);
    for (c = 0; c < info->m_nelements; c++) {
        vtkCell *element = activeDataSet->GetCell(c);
        int cellType = element->GetCellType();
        // modify by @hira at 2016/12/01
        // if (cellType == eType) {
        if (equals_vtkCellType(cellType, ucd_type)) {
            int n_edges = element->GetNumberOfEdges();
            int n_points = element->GetNumberOfPoints();
            for (e = 0; e < n_points; e++) {
                int id1 = element->GetPointId(e);
                // modify by @hira at 2016/12/01
                // new_id = 1 + included_points->InsertNextId(id1);
                // info->m_connections[i] = new_id;
                info->m_connections[i] = id1 + 1;
                i++;
            }
        }
    }
    info->m_nelements = info->m_types[ucd_type];
    if (info->m_coords != NULL) free(info->m_coords);
    info->m_coords = (float*)malloc(sizeof (float) * info->m_nnodes * 3);
    int numPoints = included_points->GetNumberOfIds();
    int p;
    DEBUG(3, "Copying %ld unstructured vtk point coordinates\n", info->m_nelements);

    // modify @hira at 2016/12/01
//    for (p = 0; p < numPoints; p++) {
//        int pid = included_points->GetId(p);
//        double* point = activeDataSet->GetPoint(pid);
//        info->m_coords[p * 3] = point[0];
//        info->m_coords[p * 3 + 1] = point[1];
//        info->m_coords[p * 3 + 2] = point[2];
//    }
    for (p = 0; p < info->m_nnodes; p++) {
        double* point = activeDataSet->GetPoint(p);
        info->m_coords[p * 3] = point[0];
        info->m_coords[p * 3 + 1] = point[1];
        info->m_coords[p * 3 + 2] = point[2];
    }

    for (n = 0; n < info->m_nnodes; n++) {
        info->m_ids[n] = n + 1;
    }

    included_points->Delete();

    if (i <= 0) return RTC_NG;
    return RTC_OK;
}

/**
 * get_PointAndCellValues
 * @param target float* pointer to target array
 * @param cellType, vtkCellType enumeration
 * @return
 */
RTC FilterIoVtk::get_PointAndCellValues(float* target, int ucdElementType) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    int c, e, n, t, a;
    int i = 0;
    int numPointTuples;
    // delete by @hira at 2017/03/15
    int vtkCellType=get_vtkCellType(ucdElementType);

    vtkDataArray *pointData_array = NULL;
    vtkDataArray *cellData_array = NULL;
    vtkPointData *pointData = activeDataSet->GetPointData();
    vtkCellData *cellData = activeDataSet->GetCellData();

    vtkIdList *pidList = vtkIdList::New();

    int numPointArrays = pointData->GetNumberOfArrays();
    int numCellArrays = cellData->GetNumberOfArrays();
    ASSERT_ALLOC(info->m_values);
    for (c = 0; c < info->m_nelements; c++) {
        vtkCell *element = activeDataSet->GetCell(c);
        int cellType = element->GetCellType();
        // printf("checking cell %d %d\n",c,vcellType );

        // modify by @hira at 2016/12/01
        // if (cellType == vtkCellType) {
        if (equals_vtkCellType(cellType, ucdElementType)) {

            //int n_edges = element->GetNumberOfEdges();
            //for (e = 0; e < n_edges; e++) {
            //    vtkCell *edge = element->GetEdge(e);
            //    int pid = edge->GetPointId(0);
            long long num_point = element->GetNumberOfPoints();
            for (int n=0; n<num_point; n++) {
                long long pid = element->GetPointId(n);
                // printf( "Including cell data %d(%d) \n", pid, e);
                pidList->InsertNextId(pid);
            }
        }
    }

    int id;
    int numPointComponents;
    double* pointComponents = NULL;
    double cellComponents[128];
    double* range = NULL;
    int numIds = pidList->GetNumberOfIds();
    int numCellComponents;
    int numCellTuples;
    int cmp_id;
    int pid;

    for (id = 0; id < numIds; id++) {
        pid = pidList->GetId(id);
        i = 0;
        // printf( "Including point data %d \n", pid);
        for (a = 0; a < numPointArrays; a++) {
            pointData_array = pointData->GetArray(a);
            numPointComponents = pointData_array->GetNumberOfComponents();
            numPointTuples = pointData_array->GetNumberOfTuples();
            pointComponents = pointData_array->GetTuple(pid);
            for (cmp_id = 0; cmp_id < numPointComponents; cmp_id++) {
//				DEBUG(3, "Including point data for point %d of value %lf\n", pid, pointComponents[cmp_id]);
                // modify by @hira at 2016/12/01
                // target[i] = (float) pointComponents[cmp_id];
                target[pid*param->veclen+i] = (float) pointComponents[cmp_id];
                i++;
            }
        }
        for (a = 0; a < numCellArrays; a++) {
            cellData_array = cellData->GetArray(a);
            numCellComponents = cellData_array->GetNumberOfComponents();
            numCellTuples = cellData_array->GetNumberOfTuples();
            vtk_getCellDataForPoint(pointData_array, pid, cellComponents);
            for (cmp_id = 0; cmp_id < numCellComponents; cmp_id++) {
                DEBUG(3, "Including cell data for point %d of value %lf\n", pid, cellComponents[cmp_id]);
                // modify by @hira at 2016/12/01
                // target[i] = (float) cellComponents[cmp_id];
                target[pid*param->veclen+i] = (float) cellComponents[cmp_id];
                i++;
            }
        }

        int kind = 0;
        for (a = 0; a < numPointArrays; a++) {
            pointData_array = pointData->GetArray(a);
            numPointComponents = pointData_array->GetNumberOfComponents();
            for (c = 0; c < numPointComponents; c++) {
                double* range = pointData_array->GetRange(c);
                info->m_value_min[kind] = range[0];
                info->m_value_max[kind] = range[1];
                kind++;
            }
        }
        for (a = 0; a < numCellArrays; a++) {
            cellData_array = cellData->GetArray(a);
            numCellComponents = cellData_array->GetNumberOfComponents();
            for (c = 0; c < numCellComponents; c++) {
                double* range = cellData_array->GetRange(c);
                info->m_value_min[kind] = range[0];
                info->m_value_max[kind] = range[1];
                kind++;
            }
        }
    }

    pidList->Delete();

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
RTC FilterIoVtk::_getCellDataForPointId(vtkDataArray *dataArray, int pid, double pointComponents[]) {
    int c, cn;
    int numComponents;
    int numCells;

    double* cellComponents;
    vtkIdList *cidList = vtkIdList::New();
    activeDataSet->GetPointCells(pid, cidList);
    numComponents = dataArray->GetNumberOfComponents();
    numCells = cidList->GetNumberOfIds();
    memset(pointComponents, 0, numComponents * (sizeof *pointComponents));
    for (cn = 0; cn < numCells; cn++) {
        int cid = cidList->GetId(cn);
        cellComponents = dataArray->GetTuple(cid);
        for (c = 0; c < numComponents; c++) {
            pointComponents[c] += cellComponents[c];
        }
    }
    for (c = 0; c < numComponents; c++) {
        pointComponents[c] = pointComponents[c] / numCells;
    }

    cidList->Delete();
    return RTC_OK;
}

/*
 * Get the coordinates of a structured grid
 * and copy them into the global m_coords array
 */
RTC FilterIoVtk::vtk_readUnstructuredCoordinates(int ucd_element_type) {
    int element_type = get_vtkCellType(ucd_element_type);
    DEBUG(3, "############# vtk_readUnstructuredCoordinates, element_type %d\n", element_type);

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;
    // add by @hira at 2016/12/01
    int step = param->start_step;
    int ret = vtk_SetUnstructGridParameters(step, element_type);

    findRangeForAllTypes();
    ret = getPointAndConnectForAllTypes(element_type);
    // add by @hira at 2016/12/01
    if (ret == RTC_NG) {
        return RTC_NG;
    }

    param->mult_element_type = (char) 0;
    return (RTC_OK);
}

/*
 *  Read values from vtk unstructured file
 */
RTC FilterIoVtk::vtk_read_unstructured_values(int step, int ucd_element_type) {
    DEBUG(1, "##########vtk_read_unstructured_values for eType %d step:%d\n", ucd_element_type, step);

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    // add by @hira at 2017/03/15
    int element_type = get_vtkCellType(ucd_element_type);
    vtk_SetUnstructGridParameters(step, element_type);

    get_PointAndCellValues(info->m_values, ucd_element_type);
    return RTC_OK;
}

} /* namespace filter */
} /* namespace pbvr */
