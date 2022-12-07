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
#include "filter_io_vtk/FilterIoVtk.h"

namespace pbvr {
namespace filter {

#if 1
const cellTypeEntry FilterIoVtk::cellTypeMap[] = {
    {
     100,
     0
    },
    {
     0,
     1
    },
    {
     102,
     -1
    },
    {
     1,
     2
    },
    {
     104,
     -1
    },
    {
     2,
     3
    },
    {
     106,
     -1
    },
    {
     107,
     -1
    },
    {
     3,
     4
    },
    {
     3,
     4
    },
    {
     4,
     4
    },
    {
     7,
     8
    },
    {
     7,
     8
    },
    {
     6,
     6
    },
    {
     5,
     5
    },
};
#else
const cellTypeEntry FilterIoVtk::cellTypeMap[] = {
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
#endif

/**
 * コンストラクター
 */
FilterIoVtk::FilterIoVtk(PbvrFilterInfo *filter_info) : m_filter_info(filter_info) {
    this->initialize();
}

/**
 * デストラクタ
 */
FilterIoVtk::~FilterIoVtk() {
    if (active_data_reader != NULL) {
        vtk_close_activeGridFile();
    }
    this->n_cellTypes = 0;
}


void FilterIoVtk::initialize() {
    activeDataSet = NULL;
    active_data_reader = NULL;
    cell_types = NULL;
    n_cellTypes = 0;
    sg_reader = NULL;
    rg_reader = NULL;
    sg_data = NULL;
    sp_data = NULL;
    rg_data = NULL;
    pd_reader = NULL;
    usg_reader = NULL;
    cell_data = NULL;
    point_data = NULL;
    image_data = NULL;
    unstrGridData = NULL;
    polyData = NULL;
    cellIds = NULL;
    pointIds = NULL;
    n_pointData_components = 0;
    n_pointData_arrays = 0;
    n_pointData_tuples = 0;
    n_cellDataArrays = 0;
    n_cellData_components = 0;
    n_cellData_tuples = 0;
    n_cellData_arrays = 0;
}


/**
 * read type of vtk dataset.
 */
RTC FilterIoVtk::vtk_get_datasetInfo() {
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = this->m_filter_info->filename;

    this->activeDataSet = NULL;
    if (active_data_reader != NULL) {
        vtk_close_activeGridFile();
    }
    this->active_data_reader = NULL;
    this->cell_types = NULL;
    this->n_cellTypes = 0;
    strcpy(this->input_dir, param->in_dir);
    this->vtk_getStepFileName(param->start_step);
    VtkDatasetTypes fileType = this->vtkDataReader_getFileType(filename);
    vtkDataSetReader *reader = vtkDataSetReader::New();
    // this->active_data_reader = vtkDataSetReader::New();
    this->vtkDataSetReader_ReadFile(reader, filename);

    LOGOUT(FILTER_LOG, (char*)"param->datasettype: %d\n", fileType);
    const char* c_file_path = (const char*) filename;
    switch (fileType) {
    case PBVR_VTK_STRUCTURED_POINTS:
        param->struct_grid = CHAR_TRUE;
        this->activeDataSet = reader->GetStructuredPointsOutput();
        break;
    case PBVR_VTK_STRUCTURED_GRID:
        param->struct_grid = CHAR_TRUE;
        this->activeDataSet = reader->GetStructuredGridOutput();
        break;
    case PBVR_VTK_RECTILINEAR_GRID:
        param->struct_grid = CHAR_TRUE;
        this->activeDataSet = reader->GetRectilinearGridOutput();
        break;
    case PBVR_VTK_UNSTRUCTURED_GRID:
        this->activeDataSet = reader->GetUnstructuredGridOutput();
        param->struct_grid = CHAR_FALSE;
        DEBUG(3, "Unstructured grid type\n");
        break;
    case PBVR_VTK_POLYDATA:
        this->activeDataSet = reader->GetPolyDataOutput();
        param->struct_grid = CHAR_FALSE;
        DEBUG(3, "PolyData grid type\n");
        break;
    default:
        LOGOUT(FILTER_ERR, (char*)"Dataset type of ( %s) is unknown \n", filename);
        break;
    }

    this->active_data_reader = reader;
    param->datasettype = fileType;
    return (RTC_OK);
}

char FilterIoVtk::get_ucdElementType(int vtkCellType) {
    if (vtkCellType < MAX_ELEMENT_TYPE) {
        cellTypeEntry vtkTypeEntry = cellTypeMap[vtkCellType];
        if (vtkTypeEntry.n_verts > -1) {
            DEBUG(1, "Converting vtk cell type %d to %d\n", vtkCellType, vtkTypeEntry.ucdElementType);
            return cellTypeMap[vtkCellType].ucdElementType;
        }
    }
    LOGOUT(FILTER_ERR, (char*)"This version of PBVR filter can not convert VTK cell type %d to UCD element type\n", vtkCellType);
    ASSERT_FAIL("UCD TYPE NOT FOUND");

    return -1;
}

int FilterIoVtk::get_vtkCellType(int ucdElementType) {
    int i;
    if (ucdElementType > 100) {
        return ucdElementType - 100;
    }
    for (i = 0; i < MAX_ELEMENT_TYPE; i++) {
        if (cellTypeMap[i].ucdElementType == ucdElementType) {
            // modify by @hira at 2016/12/01
            // return i+1;
            return i;
        }
    }
    DEBUG(1, "No vtk element type match for ucd element type %d\n", ucdElementType);
    ASSERT_FAIL("Can not convert unknown UCD element type to VTK cell type\n");

    return -1;
}

RTC FilterIoVtk::vtk_getStepFileName(int step) {
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = this->m_filter_info->filename;

    DEBUG(3, "vtk_getStepFileName %d\n", step);
    if (strlen(param->field_file) > 0) {
        sprintf(filename, "%s/%s", input_dir, param->field_file);
    } else if (strlen(param->in_prefix) > 0) {
        sprintf(filename, "%s/%s%06d.%s", input_dir, param->in_prefix, step, param->in_suffix);
    } else {
        LOGOUT(FILTER_ERR, (char*)"no input filename.\n");
        return (RTC_NG);
    }
    return RTC_OK;
}

/*
 Count the number of different cell types in the file.
 */
RTC FilterIoVtk::vtk_count_cellTypes() {
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    int c, t;
    int cell_count;
    int ucd_type_id,vtk_type_id;
    vtkCell *cell;
    this->cell_types = vtkCellTypes::New();
    // add by @hira at 2016/12/01
    if (this->activeDataSet == NULL || this->active_data_reader == NULL) {
        this->vtk_get_datasetInfo();
    }

    if (this->activeDataSet == NULL || this->active_data_reader == NULL) {
        LOGOUT(FILTER_ERR, (char*)"\n:::READ:::ERROR Could not get dataset in %s \n",
                this->m_filter_info->filename);
        exit(0);
    }

    this->activeDataSet->GetCellTypes(this->cell_types);
    this->n_cellTypes = this->cell_types->GetNumberOfTypes();
    cell_count = this->activeDataSet->GetNumberOfCells();
    info->m_elemtypes = (char *) malloc(sizeof (char)*cell_count);
    for (t = 0; t < MAX_ELEMENT_TYPE; t++) {
        this->elementType_cellIDs[t] = vtkIdList::New();
        this->elementType_numCells[t] = 0;
        this->elementType_numPoints[t] = 0;
        info->m_types[t] = 0;
    }
    for (c = 0; c < cell_count; c++) {
        cell = this->activeDataSet->GetCell(c);
        vtk_type_id = cell->GetCellType();
        ucd_type_id = get_ucdElementType(vtk_type_id);
        int numCellPoints = cell->GetNumberOfPoints();
        this->elementType_numPoints[vtk_type_id] += numCellPoints;
        this->elementType_numCells[vtk_type_id]++;
        info->m_elemtypes[c] = (char) ucd_type_id;
        info->m_types[ucd_type_id]++;
    }
    for (t = 0; t < MAX_ELEMENT_TYPE; t++) {
        if (info->m_types[t] > 0) {
            this->m_filter_info->m_element_type=t;
            this->elementType_cellIDs[t]->InsertNextId(c);
            DEBUG(0, "|||| VTK_TYPE:%d size:%d count:%d\n", t, vtk_elementsize[t], info->m_types[t]);
        }
    }

    if (this->n_cellTypes > 1) {
        param->mult_element_type = CHAR_TRUE;
        this->m_filter_info->m_multi_element = CHAR_TRUE;
    }
    return RTC_OK;
}

/*
 * Close the currently active grid file.
 */
void FilterIoVtk::vtk_close_activeGridFile() {

    if (this->m_filter_info == NULL) return;
    if (this->m_filter_info->m_param == NULL) return;
    FilterParam *param = this->m_filter_info->m_param;

    DEBUG(3, "Closing active grid file\n");
    switch (param->datasettype) {
    case PBVR_VTK_STRUCTURED_POINTS:
        ((vtkStructuredPointsReader*)this->active_data_reader)->Delete();
        break;
    case PBVR_VTK_STRUCTURED_GRID:
        ((vtkStructuredGridReader*)this->active_data_reader)->Delete();
        break;
    case PBVR_VTK_RECTILINEAR_GRID:
        ((vtkRectilinearGridReader*)this->active_data_reader)->Delete();
        break;
    case PBVR_VTK_UNSTRUCTURED_GRID:
        ((vtkUnstructuredGridReader*)this->active_data_reader)->Delete();
        break;
    case PBVR_VTK_POLYDATA:
        ((vtkPolyDataReader*)this->active_data_reader)->Delete();
        break;
    }

    this->active_data_reader = NULL;
}

bool FilterIoVtk::equals_vtkCellType(int vtk_type, int ucd_type) {
    int i;

    for (i = 0; i < MAX_ELEMENT_TYPE; i++) {
        if (cellTypeMap[i].ucdElementType == ucd_type && i == vtk_type) {
            return true;
        }
    }

    return false;
}

} /* namespace filter */
} /* namespace pbvr */

