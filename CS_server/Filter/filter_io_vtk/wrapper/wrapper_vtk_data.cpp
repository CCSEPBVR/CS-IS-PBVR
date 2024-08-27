#include "wrapper_vtk_data.h"

#include "vtkDataReader.h"
#include "vtkStructuredPointsReader.h"
/*
	C-tors
 */
WRAP_VTK_CTOR(vtkCellTypes);
WRAP_VTK_DTOR(vtkCellTypes);

/*
	vtkIdList Wrappers
 */
WRAP_VTK_CTOR(vtkIdList);
WRAP_VTK_DTOR(vtkIdList);
WRAP_VTK_GETTER(vtkIdList, GetNumberOfIds, int);
WRAP_VTK_GETTER_1ARG(vtkIdList, GetId, int, int);
WRAP_VTK_GETTER_1ARG(vtkIdList, InsertUniqueId, int, int);
WRAP_VTK_GETTER_1ARG(vtkIdList, InsertNextId, int, int);
/*
       vtkStructuredPoints Wrappers
 */
WRAP_VTK_GETTER(vtkStructuredPoints, GetDataDimension, int);
WRAP_VTK_GETTER(vtkStructuredPoints, GetDimensions, int*);
WRAP_VTK_GETTER(vtkStructuredPoints, GetNumberOfPoints, int);
WRAP_VTK_GETTER(vtkStructuredPoints, GetNumberOfScalarComponents, int);
WRAP_VTK_GETTER(vtkStructuredPoints, GetNumberOfCells, int);
WRAP_VTK_GETTER(vtkStructuredPoints, GetPointData, _vtkPointData);
WRAP_VTK_GETTER(vtkStructuredPoints, GetCellData, _vtkCellData);
WRAP_VTK_GETTER_1ARG(vtkStructuredPoints, GetCell, _vtkCell, int);
WRAP_VTK_GETTER_1ARG(vtkStructuredPoints, GetPoint, double*, int);
/*
    vtkStructuredGrid Getters
 */
WRAP_VTK_GETTER(vtkStructuredGrid, GetDataDimension, int);
WRAP_VTK_GETTER(vtkStructuredGrid, GetDimensions, int*);
WRAP_VTK_GETTER(vtkStructuredGrid, GetNumberOfPoints, int);
WRAP_VTK_GETTER(vtkStructuredGrid, GetNumberOfCells, int);
WRAP_VTK_GETTER(vtkStructuredGrid, GetPointData, _vtkPointData);
WRAP_VTK_GETTER(vtkStructuredGrid, GetCellData, _vtkCellData);
WRAP_VTK_GETTER_1ARG(vtkStructuredGrid, GetCell, _vtkCell, int);
WRAP_VTK_GETTER_1ARG(vtkStructuredGrid, GetPoint, double*, int);
/*
    vtkRectilinearGrid Getters
 */
WRAP_VTK_GETTER(vtkRectilinearGrid, GetDataDimension, int);
WRAP_VTK_GETTER(vtkRectilinearGrid, GetDimensions, int*);
WRAP_VTK_GETTER(vtkRectilinearGrid, GetNumberOfPoints, int);
WRAP_VTK_GETTER(vtkRectilinearGrid, GetNumberOfCells, int);
WRAP_VTK_GETTER(vtkRectilinearGrid, GetPointData, _vtkPointData);
WRAP_VTK_GETTER(vtkRectilinearGrid, GetCellData, _vtkCellData);
WRAP_VTK_GETTER_1ARG(vtkRectilinearGrid, GetCell, _vtkCell, int);
WRAP_VTK_GETTER_1ARG(vtkRectilinearGrid, GetPoint, double*, int);
/*
    vtkUnstructuredGrid Getters
 */
WRAP_VTK_GETTER(vtkUnstructuredGrid, GetNumberOfPoints, int);
WRAP_VTK_GETTER(vtkUnstructuredGrid, GetNumberOfCells, int);
WRAP_VTK_GETTER(vtkUnstructuredGrid, GetPointData, _vtkPointData);
WRAP_VTK_GETTER(vtkUnstructuredGrid, GetCellData, _vtkCellData);
WRAP_VTK_GETTER_1ARG(vtkUnstructuredGrid, GetCell, _vtkCell, int);
WRAP_VTK_GETTER_1ARG(vtkUnstructuredGrid, GetPoint, double*, int);

/*
    vtkPolyData Getters
 */
WRAP_VTK_GETTER(vtkPolyData, GetNumberOfPoints, int);
WRAP_VTK_GETTER(vtkPolyData, GetNumberOfPolys, int);
WRAP_VTK_GETTER(vtkPolyData, GetNumberOfVerts, int);
WRAP_VTK_GETTER(vtkPolyData, GetNumberOfLines, int);
WRAP_VTK_GETTER(vtkPolyData, GetNumberOfCells, int);
WRAP_VTK_GETTER(vtkPolyData, GetPointData, _vtkPointData);
WRAP_VTK_GETTER(vtkPolyData, GetCellData, _vtkCellData);
WRAP_VTK_GETTER_1ARG(vtkPolyData, GetCell, _vtkCell, int);
/*
    vtkDataArray Getters
 */
WRAP_VTK_GETTER(vtkDataArray, GetName, const char*);
WRAP_VTK_GETTER(vtkDataArray, GetDataTypeAsString, const char*);
WRAP_VTK_GETTER(vtkDataArray, GetNumberOfComponents, int);
WRAP_VTK_GETTER(vtkDataArray, GetNumberOfTuples, int);
WRAP_VTK_GETTER_1ARG(vtkDataArray, GetTuple, double*, int);
WRAP_VTK_GETTER_1ARG(vtkDataArray, GetRange, double*, int);
/*
    vtkPointData Getters
 */
WRAP_VTK_GETTER(vtkPointData, GetNumberOfArrays, int);
WRAP_VTK_GETTER(vtkPointData, GetNumberOfComponents, int);
WRAP_VTK_GETTER(vtkPointData, GetNumberOfTuples, int);
WRAP_VTK_GETTER_1ARG(vtkPointData, GetArray, _vtkDataArray, int);

/*
    vtkCell Getters
 */
WRAP_VTK_GETTER(vtkCell, GetCellType, int);
WRAP_VTK_GETTER(vtkCell, GetNumberOfPoints, int);
WRAP_VTK_GETTER(vtkCell, GetNumberOfEdges, int);
WRAP_VTK_GETTER(vtkCell, GetPoints, _vtkPoints);
WRAP_VTK_GETTER(vtkCell, GetBounds, double*);
WRAP_VTK_GETTER_1ARG(vtkCell, GetPointId, int, int);
WRAP_VTK_GETTER_1ARG(vtkCell, GetEdge, _vtkCell, int);

/*
    vtkCellTypes Getters
 */
WRAP_VTK_GETTER(vtkCellTypes, GetNumberOfTypes, int);
WRAP_VTK_GETTER_1ARG(vtkCellTypes, GetCellType, int, int);
WRAP_VTK_GETTER_1ARG(vtkCellTypes, GetClassNameFromTypeId, const char*, int);
/*
    vtkCellData Getters
 */
WRAP_VTK_GETTER(vtkCellData, GetNumberOfArrays, int);
WRAP_VTK_GETTER(vtkCellData, GetNumberOfComponents, int);
WRAP_VTK_GETTER(vtkCellData, GetNumberOfTuples, int);
WRAP_VTK_GETTER_1ARG(vtkCellData, GetArray, _vtkDataArray, int);

/*
    vtkImageData Getters
 */
WRAP_VTK_GETTER(vtkImageData, GetDataDimension, int);
WRAP_VTK_GETTER(vtkImageData, GetDimensions, int*);
WRAP_VTK_GETTER(vtkImageData, GetNumberOfPoints, int);
WRAP_VTK_GETTER(vtkImageData, GetNumberOfScalarComponents, int);
WRAP_VTK_GETTER(vtkImageData, GetNumberOfCells, int);
WRAP_VTK_GETTER_1ARG(vtkImageData, GetPoint, double*, int);
/*
    vtkDataSet Getters
 */
WRAP_VTK_GETTER(vtkDataSet, GetNumberOfCells, int);
WRAP_VTK_GETTER(vtkDataSet, GetNumberOfPoints, int);
WRAP_VTK_GETTER(vtkDataSet, GetPointData, _vtkPointData);
WRAP_VTK_GETTER(vtkDataSet, GetCellData, _vtkCellData);
WRAP_VTK_GETTER(vtkDataSet, GetBounds, double*);
WRAP_VTK_GETTER_1ARG(vtkDataSet, GetPoint, double*, int);
WRAP_VTK_GETTER_1ARG(vtkDataSet, GetCell, _vtkCell, int);
WRAP_VTK_GETTER_1ARG(vtkDataSet, GetCellTypes, void, _vtkCellTypes);
WRAP_VTK_GETTER_2ARG(vtkDataSet, GetPointCells, void, int, _vtkIdList);

/*
    vtkDataSetAttributes Getters
 */
WRAP_VTK_GETTER(vtkDataSetAttributes, GetScalars, _vtkDataArray);

/*
 vtkImageData Dump information
 */
void vtkImageData_DumpInfo(vtkImageData* id) {
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
void vtkStructuredPoints_DumpInfo(vtkStructuredPoints* sp) {
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
void vtkRectilinearGrid_DumpInfo(vtkRectilinearGrid* rg) {
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
void vtkStructuredGrid_DumpInfo(vtkStructuredGrid* sg) {
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
void vtkUnstructuredGrid_DumpInfo(_vtkUnstructuredGrid usg) {

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
void vtkPointData_DumpInfo(vtkPointData* pd) {
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
void vtkDataSet_DumpCellTypeInfo(vtkDataSet* data_set) {
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
}

/*
 vtkCellData Dump information
 */
void vtkCellData_DumpInfo(vtkCellData* cd) {
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
void vtkPolyData_DumpInfo(_vtkPolyData pd) {
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
