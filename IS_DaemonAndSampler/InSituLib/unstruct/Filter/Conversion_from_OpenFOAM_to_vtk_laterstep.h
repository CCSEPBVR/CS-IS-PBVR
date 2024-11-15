
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
ucd->GetPointData()->ShallowCopy(pointData);

time_step++;
generate_particles_vtk(time_step, ucd);
