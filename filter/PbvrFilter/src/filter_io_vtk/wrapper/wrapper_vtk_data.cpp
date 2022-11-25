
#include <filter_io_vtk/FilterIoVtk.h>
#include <vtkDataReader.h>
#include <vtkStructuredPointsReader.h>
#include "filter_io_vtk/wrapper/wrapper_vtk_data.h"

using namespace std;
using namespace pbvr::filter;

/*
 vtkImageData Dump information
 */
void FilterIoVtk::vtkImageData_DumpInfo(vtkImageData* id) {
    cout << "VTK IMAGE DATA INFO:" << endl;
    int id_dim[3] = {0, 0, 0};
    id->GetDimensions(&id_dim[0]);
    int id_npoints = id->GetNumberOfPoints();
    int id_ncells = id->GetNumberOfCells();
    int id_nscomp = id->GetNumberOfScalarComponents();
    int id_ddim = id->GetDataDimension();
    cout << "    GetNumberOfScalarComponents:" << id_nscomp << endl;
    cout << "    GetNumberOfPoints:" << id_npoints << endl;
    cout << "    GetNumberOfCells:" << id_ncells << endl;
    cout << "    GetDataDimension:" << id_ddim << endl;
    cout << "    GetDimension:" << id_dim[0] << ", " << id_dim[1] << ", " << id_dim[2] << endl;
}

/*
 vtkStructuredPoints Dump information
 */
void FilterIoVtk::vtkStructuredPoints_DumpInfo(vtkStructuredPoints* sp) {
    cout << "VTK STRUCTURED POINTS INFO:" << endl;
    int sp_dim[3];
    sp->GetDimensions(&sp_dim[0]);
    int sp_npoints = sp->GetNumberOfPoints();
    int sp_ncells = sp->GetNumberOfCells();
    int sp_nscomp = sp->GetNumberOfScalarComponents();
    int sp_ddim = sp->GetDataDimension();
    cout << "    GetNumberOfScalarComponents:" << sp_nscomp << endl;
    cout << "    GetNumberOfPoints:" << sp_npoints << endl;
    cout << "    GetNumberOfCells:" << sp_ncells << endl;
    cout << "    GetDataDimension:" << sp_ddim << endl;
    cout << "    GetDimension:" << sp_dim[0] << ", " << sp_dim[1] << ", " << sp_dim[2] << endl;

    vtkPointData* pd = sp->GetPointData();
    vtkCellData* cd = sp->GetCellData();
    vtkImageData* id = (vtkImageData*) sp;
    vtkImageData_DumpInfo(id);
    vtkPointData_DumpInfo(pd);
    vtkCellData_DumpInfo(cd);
    vtkDataSet_DumpCellTypeInfo(sp);
}

/*
 vtkRectilinearGrid Dump information
 */
void FilterIoVtk::vtkRectilinearGrid_DumpInfo(vtkRectilinearGrid* rg) {
    cout << "VTK RECTILINEAR GRID INFO:" << endl;
    int dim[3] = {0, 0, 0};
    rg->GetDimensions(&dim[0]);
    int npoints = rg->GetNumberOfPoints();
    int ncells = rg->GetNumberOfCells();
    int ddim = rg->GetDataDimension();
    cout << "    GetNumberOfPoints:" << npoints << endl;
    cout << "    GetNumberOfCells:" << ncells << endl;
    cout << "    GetDataDimension:" << ddim << endl;
    cout << "    GetDimension:" << dim[0] << ", " << dim[1] << ", " << dim[2] << endl;

    vtkPointData* pd = rg->GetPointData();
    vtkCellData* cd = rg->GetCellData();
    vtkPointData_DumpInfo(pd);
    vtkCellData_DumpInfo(cd);
    vtkDataSet_DumpCellTypeInfo(rg);
}

/*
 vtkStructuredGrid Dump information
 */
void FilterIoVtk::vtkStructuredGrid_DumpInfo(vtkStructuredGrid* sg) {
    cout << "VTK STRUCTURED GRID INFO:" << endl;
    int dim[3] = {0, 0, 0};
    sg->GetDimensions(&dim[0]);
    int npoints = sg->GetNumberOfPoints();
    int ncells = sg->GetNumberOfCells();
    int ddim = sg->GetDataDimension();
    cout << "    GetNumberOfPoints:" << npoints << endl;
    cout << "    GetNumberOfCells:" << ncells << endl;
    cout << "    GetDataDimension:" << ddim << endl;
    cout << "    GetDimension:" << dim[0] << ", " << dim[1] << ", " << dim[2] << endl;

    vtkPointData* pd = sg->GetPointData();
    vtkCellData* cd = sg->GetCellData();
    vtkImageData* id = (vtkImageData*) sg;
    vtkImageData_DumpInfo(id);
    vtkPointData_DumpInfo(pd);
    vtkCellData_DumpInfo(cd);
    vtkDataSet_DumpCellTypeInfo(sg);
}

/*
 vtkUnstructuredGrid Dump information
 */
void FilterIoVtk::vtkUnstructuredGrid_DumpInfo(vtkUnstructuredGrid *usg) {

    cout << "VTK UNSTRUCTURED GRID INFO:" << endl;

    int npoints = usg->GetNumberOfPoints();
    int ncells = usg->GetNumberOfCells();

    cout << "    GetNumberOfPoints:" << npoints << endl;
    cout << "    GetNumberOfCells:" << ncells << endl;

    vtkPointData* pd = usg->GetPointData();
    vtkPointData_DumpInfo(pd);

    vtkCellData* cd = usg->GetCellData();
    vtkCellData_DumpInfo(cd);

    vtkDataSet_DumpCellTypeInfo(usg);

}

/*
 vtkPointData Dump information
 */
void FilterIoVtk::vtkPointData_DumpInfo(vtkPointData* pd) {
    cout << "VTK POINT DATA INFO:" << endl;
    int pd_numa = pd->GetNumberOfArrays();
    int pd_numc = pd->GetNumberOfComponents();
    int pd_numt = pd->GetNumberOfTuples();

    cout << "    GetNumberOfArrays:" << pd_numa << endl;
    cout << "    GetNumberOfComponents:" << pd_numc << endl;
    cout << "    GetNumberOfTuples:" << pd_numt << endl;
    cout << "    ArrayNames:" << endl;
    vtkDataArray* da;
    for (int i = 0; i < pd_numa; i++) {
    da = pd->GetArray(i);
    cout << "      array(" << i << ") =>name:" << da->GetName() << " [type, components, tuples]:[";
    cout << da->GetDataTypeAsString() << "," << da->GetNumberOfComponents() << "," << da->GetNumberOfTuples() << "]" << endl;
    }
}

/*
 vtkCellData Dump information
 */
void FilterIoVtk::vtkDataSet_DumpCellTypeInfo(vtkDataSet* data_set) {
    int c, t;
    int type_id;
    int type_count = -1;
    int type_counts[99] = {0};
    int type_points[99] = {0};
    const char* type_name;
    vtkCell* cell;
    vtkCellTypes* cell_types = vtkCellTypes::New();
    data_set->GetCellTypes(cell_types);
    cout << "VTK CELL TYPES INFO:" << endl;
    int n_cell_types = cell_types->GetNumberOfTypes();
    cout << "    NumberOfCellTypes:" << n_cell_types << endl;
    int cell_count = data_set->GetNumberOfCells();
    for (c = 0; c < cell_count; c++) {
        cell = data_set->GetCell(c);
        type_id = cell->GetCellType();
        type_points[type_id] = cell->GetNumberOfPoints();
        type_counts[type_id]++;
    }
    for (t = 0; t < n_cell_types; t++) {
        type_id = cell_types->GetCellType(t);
        type_name = cell_types->GetClassNameFromTypeId(type_id);
        cout << "      type_name(" << t << "):" << type_name << " (type_id: " << type_id << ", count:" << type_counts[type_id] << ", points:" << type_points[type_id] << " )" << endl;
    }

    cell_types->Delete();
}

/*
 vtkCellData Dump information
 */
void FilterIoVtk::vtkCellData_DumpInfo(vtkCellData* cd) {
    cout << "VTK CELL DATA INFO:" << endl;
    int cd_numa = cd->GetNumberOfArrays();
    int cd_numc = cd->GetNumberOfComponents();
    int cd_numt = cd->GetNumberOfTuples();

    cout << "    GetNumberOfArrays:" << cd_numa << endl;
    cout << "    GetNumberOfComponents:" << cd_numc << endl;
    cout << "    GetNumberOfTuples:" << cd_numt << endl;
    cout << "    ArrayNames:" << endl;
    vtkDataArray* da;
    for (int i = 0; i < cd_numa; i++) {
    da = cd->GetArray(i);
    cout << "      array(" << i << ") =>name:" << da->GetName() << " [type, components, tuples]:[";
    cout << da->GetDataTypeAsString() << "," << da->GetNumberOfComponents() << "," << da->GetNumberOfTuples() << "]" << endl;
    }
}

/*
 vtkPolyData Dump information
 */
void FilterIoVtk::vtkPolyData_DumpInfo(vtkPolyData *pd) {
    cout << "VTK POLY DATA INFO:" << endl;
    int pd_nump = pd->GetNumberOfPoints();
    int pd_numpoly = pd->GetNumberOfPolys();
    int pd_numv = pd->GetNumberOfVerts();
    int pd_numl = pd->GetNumberOfLines();
    int pd_numcells = pd->GetNumberOfCells();
    cout << "    GetNumberOfPoints:" << pd_nump << endl;
    cout << "    GetNumberOfPolys:" << pd_numpoly << endl;
    cout << "    GetNumberOfVerts:" << pd_numv << endl;
    cout << "    GetNumberOfLines:" << pd_numl << endl;
    cout << "    GetNumberOfCells:" << pd_numcells << endl;

    vtkPointData* pntd = pd->GetPointData();
    vtkCellData* cd = pd->GetCellData();

    vtkPointData_DumpInfo(pntd);
    vtkCellData_DumpInfo(cd);

    vtkDataSet_DumpCellTypeInfo(pd);
}

