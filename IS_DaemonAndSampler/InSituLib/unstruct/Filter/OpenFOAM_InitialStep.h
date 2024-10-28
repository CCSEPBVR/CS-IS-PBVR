   vtkSmartPointer<vtkUnstructuredGrid> ucd = vtkSmartPointer<vtkUnstructuredGrid>::New();

   // point
   vtkSmartPointer<vtkPoints> vpoint = vtkSmartPointer<vtkPoints>::New();
   forAll(mesh.points(),pid)
   {
       vpoint -> InsertNextPoint( double(mesh.points()[pid][0]),double(mesh.points()[pid][1]),double(mesh.points()[pid][2]));
   }
   ucd ->SetPoints(vpoint);

   //cell  
   vtkSmartPointer<vtkCellArray> vcell;
//#pragma omp parallel  
   for (int cid = 0; cid < mesh.cellShapes().size(); cid ++)
   {
       const  cellShape c = mesh.cellShapes()[cid];
       int index = c.model().index();
       switch(index)
       {
            case 3: //hex
               {
                    vtkSmartPointer<vtkHexahedron> hex = vtkSmartPointer<vtkHexahedron>::New();
                    for (unsigned int i = 0; i < 8; ++i)
                    {
                       hex -> GetPointIds()->SetId(i, c[i]);
                    }
                    ucd -> InsertNextCell(hex -> GetCellType(), hex->GetPointIds());
                 break;
               }
           case 5: //prism
               {
                    vtkSmartPointer<vtkWedge> wedge = vtkSmartPointer<vtkWedge>::New();
                    for (unsigned int i = 0; i < 6; ++i)
                    {
                       wedge -> GetPointIds()->SetId(i, c[i]);
                    }
                    ucd -> InsertNextCell(wedge -> GetCellType(), wedge->GetPointIds());
                 break;
               }
           case 6: //pyramid
               {
                    vtkSmartPointer<vtkPyramid> pyramid = vtkSmartPointer<vtkPyramid>::New();
                    for (unsigned int i = 0; i < 5; ++i)
                    {
                       pyramid -> GetPointIds()->SetId(i, c[i]);
                    }
                    ucd -> InsertNextCell( pyramid -> GetCellType(), pyramid ->GetPointIds());
                 break;
               }
           case 7: //tetra
               {
                    vtkSmartPointer<vtkTetra> tetra = vtkSmartPointer<vtkTetra>::New();
                    for (unsigned int i = 0; i < 4; ++i)
                    {
                       tetra -> GetPointIds()->SetId(i, c[i]);
                    }
                    ucd -> InsertNextCell( tetra -> GetCellType(), tetra->GetPointIds());
                 break;
               }
//           defalut:
//               {
//                   Info << "未対応のセルです"<< endl;
//                   return 0;
//               }
       }
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
    ucd->GetPointData()->ShallowCopy(pointData);

