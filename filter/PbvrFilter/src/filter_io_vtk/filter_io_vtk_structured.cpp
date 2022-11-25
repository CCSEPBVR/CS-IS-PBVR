/*
 * File:   filter_io_vtk_structured.h
 * Author: Andersson M, V.I.C Visible Information Center
 * Author: Minemoto H, V.I.C Visible Information Center
 *
 * Created on November 18, 2015, 5:14 PM
 */

#include <stdio.h>
#include <string.h>

#include "filter_io_vtk/FilterIoVtk.h"


namespace pbvr {
namespace filter {

void FilterIoVtk::vtk_open_structuredPointsReader(const char* c_file_path) {
    DEBUG(3, "############vtk_open_structuredPointsReader\n");
    if (this->m_filter_info == NULL) return;
    if (this->m_filter_info->m_param == NULL) return;
    FilterParam *param = this->m_filter_info->m_param;

    vtkStructuredPointsReader *sp_reader = vtkStructuredPointsReader::New();
    vtkStructuredPointsReader_ReadFile(sp_reader, c_file_path);
    if (DEBUG_LEVEL != 0) {
        StructuredPointsReader_DumpInfo(sp_reader);
    }
    sp_data = sp_reader->GetOutput();
    activeDataSet = (vtkDataSet*) sp_data;
    active_data_reader = sp_reader;
    param->struct_grid = CHAR_TRUE;
}


void FilterIoVtk::vtk_open_structuredGridReader(const char* c_file_path) {
    DEBUG(3, "############vtk_open_structuredGridReader\n");
    sg_reader = vtkStructuredGridReader::New();
    vtkStructuredGridReader_ReadFile(sg_reader, c_file_path);
    if (DEBUG_LEVEL != 0) {
    StructuredGridReader_DumpInfo(sg_reader);
    }
    sg_data = sg_reader->GetOutput();
    active_data_reader = sg_reader;
    activeDataSet = (vtkDataSet*) sg_data;
}
//

void FilterIoVtk::vtk_open_rectilinearGridReader(const char* c_file_path) {
    DEBUG(3, "############vtk_open_rectilinearGridReader\n");
    rg_reader = vtkRectilinearGridReader::New();
    vtkRectilinearGridReader_ReadFile(rg_reader, c_file_path);
    if (DEBUG_LEVEL != 0) {
    RectilinearGridReader_DumpInfo(rg_reader);
    }
    rg_data = rg_reader->GetOutput();
    active_data_reader = rg_reader;
    activeDataSet = (vtkDataSet*) rg_data;
}

/**
 * Read the dimensions of the three structured grid type datasets
 */
RTC FilterIoVtk::vtk_setStructGridParameters(int step) {
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
    n_cellDataArrays = 0;
    n_cellData_components = 0;
    n_cellData_tuples = 0;
    int dim_default[3] = {0, 0, 0};
    int* dims = dim_default;
    const char* c_file_path;
    int grid_data_dim = 0;

    point_data = NULL;
    cell_data = NULL;
    vtk_getStepFileName(step);
    c_file_path = (const char*) info->filename;
    DEBUG(3, "\n\n\n =============== Opening %d, %s \n\n", step, c_file_path);
    if (active_data_reader != NULL) {
        vtk_close_activeGridFile();
    }
    switch (param->datasettype) {
    case PBVR_VTK_STRUCTURED_POINTS:
        vtk_open_structuredPointsReader(c_file_path);
        dims = sp_data->GetDimensions();
        grid_data_dim = sp_data->GetDataDimension();
        break;
    case PBVR_VTK_STRUCTURED_GRID:
        vtk_open_structuredGridReader(c_file_path);
        dims = sg_data->GetDimensions();
        grid_data_dim = sg_data->GetDataDimension();
        break;
    case PBVR_VTK_RECTILINEAR_GRID:
        vtk_open_rectilinearGridReader(c_file_path);
        dims = rg_data->GetDimensions();
        grid_data_dim = rg_data->GetDataDimension();
        break;
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
    n_cellDataArrays = cell_data->GetNumberOfArrays();

    param->ndim = (dims[2] == 0) ? 2 : 3;

    if (param->ndim == 2) {
        param->dim1 = dims[0];
        param->dim2 = dims[1];
        info->m_nnodes = param->dim1 * param->dim2;
        info->m_nelements = (param->dim1 - 1)*(param->dim2 - 1);
        info->m_elemcoms = 4;
        info->m_element_type = 3;
    } else if (param->ndim == 3) {
        param->dim1 = dims[0];
        param->dim2 = dims[1];
        param->dim3 = dims[2];
        info->m_nnodes = param->dim1 * param->dim2 * param->dim3;
        info->m_nelements = (param->dim1 - 1)*(param->dim2 - 1)*(param->dim3 - 1);
        info->m_elemcoms = 8;
        info->m_element_type = 7;
    } else {
        LOGOUT(FILTER_ERR, (char*)"Not support dimension : %ld\n", param->ndim);
    return (RTC_NG);
    }

    // modify by @hira min/max to info->node_min/node_max
    info->node_min.x = param->min_ext[0] = 0;
    info->node_min.y = param->min_ext[1] = 0;
    info->node_min.z = param->min_ext[2] = 0;
    info->node_max.x = param->max_ext[0] = param->dim1 - 1;
    info->node_max.y = param->max_ext[1] = param->dim2 - 1;
    info->node_max.z = param->max_ext[2] = param->dim3 - 1;

    param->veclen = n_pointData_components + n_cellData_components;
    param->nspace = grid_data_dim;
    info->m_connectsize = info->m_nelements*info->m_elemcoms;
    info->m_ncomponents = param->veclen;
    info->m_nkinds = param->veclen;
    info->m_desctype = 1;
    info->m_datatype = 1;

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

/**
 * Get the coordinates of a structured grid
 * and copy them into the global m_coords array
 */
RTC FilterIoVtk::vtk_read_structured_coordinates() {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    double* point = NULL;
    double* bounds = NULL;
    int n, ci, i, j, k;

    if (info->m_coords != NULL) free(info->m_coords);
    info->m_coords = (float*)malloc(sizeof (float) * info->m_nnodes * 3);

    bounds = activeDataSet->GetBounds();

    // modify by @hira min/max to info->node_min/node_max
    info->node_min.x = bounds[0];
    info->node_max.x = bounds[1];
    info->node_min.y = bounds[2];
    info->node_max.y = bounds[3];
    info->node_min.z = bounds[4];
    info->node_max.z = bounds[5];

    DEBUG(3, "Copying  %ld structured vtk point nodes \n", info->m_nnodes);
    n = 0;
    for (k = 0; k < param->dim3; k++) {
        for (j = 0; j < param->dim2; j++) {
            for (i = 0; i < param->dim1; i++) {
                ci = n * 3;
                point = activeDataSet->GetPoint(n);
                info->m_coords[ci] = (float) point[0];
                info->m_coords[ci + 1] = (float) point[1];
                info->m_coords[ci + 2] = (float) point[2];
                n++;
            }
        }
    }

    return (RTC_OK);
}

RTC FilterIoVtk::vtk_getCellDataForPoint(vtkDataArray *dataArray, int pid, double pointComponents[]) {
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
    return RTC_OK;
}

int FilterIoVtk::vtk_copy_cellDataComponents(float* target, int* offset, int stride) {

    if (this->m_filter_info == NULL) return -1;
    if (this->m_filter_info->m_param == NULL) return -1;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    vtkDataArray *dataArray;
    int c = 0;
    int n = 0;
    int i = 0;
    int a = 0;
    int t = 0;

    long int nTotalCellComponents = 0;
    int numTuples = 0;
    int numComponents;
//    double components[n_cellData_components];
    double *components = (double*)malloc(n_cellData_components);
    for (a = 0; a < n_cellDataArrays; a++) {
        dataArray = cell_data->GetArray(a);
        numTuples = dataArray->GetNumberOfTuples();
        if (numTuples == info->m_nelements) {
            i = *offset;
            numComponents = dataArray->GetNumberOfComponents();
            DEBUG(3, "  #:CellDataArray offset; %d  numComponents:%d, n_tuples:%d\n", *offset, numComponents, numTuples);
            for (n = 0; n < info->m_nnodes; n++) {
                vtk_getCellDataForPoint(dataArray, n, components);
                for (c = 0; c < numComponents; c++) {
                    info->m_values[i] = (float) components[c];
                    i++;
                }
                i += stride - numComponents;
            }
            for (c = 0; c < numComponents; c++) {
                double* range = dataArray->GetRange(c);
                info->m_value_min[*offset + c] = range[0];
                info->m_value_max[*offset + c] = range[1];
            }
            *offset += numComponents;
        }
    }
    DEBUG(3, "  #:Done Copying  %ld x %ld vtk point data values. (offset:%d, n:%d) \n", info->m_nnodes, param->veclen, *offset, n);
    free(components);

    return nTotalCellComponents;
}

RTC FilterIoVtk::vtk_copy_pointDataComponents(float* target, int* offset, int stride) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    vtkDataArray *data_array = NULL;
    double* components = NULL;
    int c = 0;
    int n = 0;
    int i = 0;
    int a = 0;
    int numComponents = 0;
    int numTuples = 0;

    // Copy array data to m_values
    for (a = 0; a < n_pointData_arrays; a++) {
        data_array = point_data->GetArray(a);
        numTuples = data_array->GetNumberOfTuples();
    if (numTuples == info->m_nnodes) {
        i = *offset;
        numComponents = data_array->GetNumberOfComponents();
        DEBUG(3, "  #:PointDataArray offset; %d  numComponents:%d, n_tuples:%d\n", *offset, numComponents, numTuples);
        for (n = 0; n < info->m_nnodes; n++) {
            components = data_array->GetTuple(n);
            for (c = 0; c < numComponents; c++) {
                info->m_values[i] = (float) components[c];
                i++;
            }
            i += stride - numComponents;
        }
        for (c = 0; c < numComponents; c++) {
            double* range = data_array->GetRange(c);
            info->m_value_min[*offset + c] = range[0];
            info->m_value_max[*offset + c] = range[1];
        }
        *offset += numComponents;
    }
    }
    DEBUG(3, "  #:Done Copying  %ld x %ld vtk point data values. (offset:%d, n:%d) \n", info->m_nnodes, param->veclen, *offset, n);
    return RTC_OK;
}

/*
 * Get the values from a structured grid
 * and copy them into the global m_values array
 */
RTC FilterIoVtk::vtk_read_structured_values(int step) {
    DEBUG(3, "#############vtk_read_structured_values\n");

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    int offset = 0;

    vtk_setStructGridParameters(step);
    // Count valid point data arrays (where number of tuples equal m_nodes)

    DEBUG(3, " ##:step:%d::Copying  %ld x %ld vtk point data values \n", step, info->m_nnodes, param->veclen);
    vtk_copy_pointDataComponents(info->m_values, &offset, param->veclen);
    DEBUG(3, " ##:step:%d::Deriving  %ld x %ld vtk point data values from %ld cell data values \n", step, info->m_nnodes, param->veclen, info->m_nelements);
    vtk_copy_cellDataComponents(info->m_values, &offset, param->veclen);
    fflush(stdout);
    return (RTC_OK);
}

} /* namespace filter */
} /* namespace pbvr */
