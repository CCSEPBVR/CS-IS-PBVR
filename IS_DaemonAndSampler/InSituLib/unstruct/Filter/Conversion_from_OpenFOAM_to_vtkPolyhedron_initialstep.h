
// time_step =0 のみ座標情報を入力する！！
int time_step = 0;
vtkSmartPointer<vtkUnstructuredGrid> ucd = vtkSmartPointer<vtkUnstructuredGrid>::New();
// point
vtkSmartPointer<vtkPoints> vpoint = vtkSmartPointer<vtkPoints>::New();
forAll(mesh.points(),pid)
{
    vpoint -> InsertNextPoint( double(mesh.points()[pid][0]),double(mesh.points()[pid][1]),double(mesh.points()[pid][2]));
}
ucd ->SetPoints(vpoint);

//cell  
//#pragma omp parallel  
for (int cid = 0; cid < mesh.cellShapes().size(); cid ++)
{
    const  cellShape c = mesh.cellShapes()[cid];
    int npoints = c.nPoints();
    vtkIdType pointIds[npoints]; // = {0, 1, 2, 3, 4, 5, 6, 7};

    for (int j =0 ; j< npoints; j++ )  pointIds[j] = c[j];

    int nfaces = c.nFaces();
    auto vcell = vtkSmartPointer<vtkCellArray>::New();
    for (int i = 0; i < nfaces; i++)
    {
        auto Fc = c.faces()[i]; // 面iの取得 
        int nFcpoints = Fc.size() ;// 面iの頂点数取得
        vtkIdType face[nFcpoints] ;
        for (int k =0; k< nFcpoints; k++ )
        {
            face[k] = Fc[k];
        }
        vcell->InsertNextCell(nFcpoints, face);
    }
    vtkSmartPointer<vtkIdTypeArray> legacyFaces = vtkSmartPointer<vtkIdTypeArray>::New();
    vcell->ExportLegacyFormat(legacyFaces);
    ucd->InsertNextCell(VTK_POLYHEDRON, npoints, pointIds, nfaces, legacyFaces -> GetPointer(0));
}

//variable
// u
vtkSmartPointer<vtkFloatArray> scalars_u =
vtkSmartPointer<vtkFloatArray>::New();
scalars_u->SetName("U");
forAll(mesh.cells(),cid)
{
    scalars_u->InsertNextValue(U[cid].x());
}

ucd -> GetCellData() -> AddArray(scalars_u);

// v
vtkSmartPointer<vtkFloatArray> scalars_v =
vtkSmartPointer<vtkFloatArray>::New();
scalars_v->SetName("V");
forAll(mesh.cells(),cid)
{
    scalars_v->InsertNextValue(U[cid].y());
}
ucd -> GetCellData() -> AddArray(scalars_v);

// w
vtkSmartPointer<vtkFloatArray> scalars_w =
vtkSmartPointer<vtkFloatArray>::New();
scalars_w->SetName("W");
forAll(mesh.cells(),cid)
{
    scalars_w->InsertNextValue(U[cid].z());
}
ucd -> GetCellData() -> AddArray(scalars_w);

vtkSmartPointer<vtkCellDataToPointData> cellDataToPointData =
vtkSmartPointer<vtkCellDataToPointData>::New();
cellDataToPointData -> SetInputData(ucd);
cellDataToPointData -> Update();
vtkPointData* pointData = cellDataToPointData->GetOutput()->GetPointData();
//ucd->GetPointData()->ShallowCopy(pointData);
//generate_particles_vtk(time_step, ucd);

std::cout << __LINE__ <<std::endl;

vtkSmartPointer<vtkDataSetTriangleFilter> triangleFilter =
vtkSmartPointer<vtkDataSetTriangleFilter>::New();
triangleFilter->SetInputData(ucd);
triangleFilter->Update();

vtkSmartPointer<vtkUnstructuredGrid> ucd_tri = vtkSmartPointer<vtkUnstructuredGrid>::New();
ucd_tri = triangleFilter-> GetOutput();
ucd_tri->GetPointData()->ShallowCopy(pointData);

generate_particles_vtk(time_step, ucd_tri);
