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
//for (int cid =0; cid < mesh.nCells(); cid ++)
//{
//    std::cout << "cid = " << cid << std::endl; 
//    const cell &cellVertices = mesh.cells()[cid];
//    const  cellShape c = mesh.cellShapes()[cid];
//    int npoints = c.nPoints();
//    std::cout << " npoints =  " << npoints << std::endl;
//    std::cout << "  Vertex size: " << cellVertices.size() << std::endl;
//
//    if (c.model().index() == 0)
//    {
//    // 各頂点の座標を取得
//    for (label vertI = 0; vertI < cellVertices.size(); ++vertI)
//    {
//        label pointIndex = cellVertices[vertI]; // 頂点のインデックス
//        const point &vertexCoord = mesh.points()[pointIndex]; // 頂点の座標
//        std::cout << "  Vertex " << int(vertI) << ": " << double(vertexCoord[0]) << ", " <<  double(vertexCoord[1]) << ", " << double(vertexCoord[2])<< std::endl;
//    }
//    }
//
//}

    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

vtkSmartPointer<vtkUnstructuredGrid> tet_data = vtkSmartPointer<vtkUnstructuredGrid>::New();

//#pragma omp parallel  
//for (int cid = 0; cid < mesh.cellShapes().size(); cid ++)
std::cout << "mesh.nCells() = " << mesh.nCells() << std::endl; 
for (int cid =0; cid < mesh.nCells(); cid ++)
//forAll(mesh.cells(),cid)
{
    const  cellShape c = mesh.cellShapes()[cid];
    auto vcell = vtkSmartPointer<vtkCellArray>::New();
    const labelList &cellFaces = mesh.cells()[cid];

    // 4 debug
//    int nfacesd = c.nFaces();
//    for (int i = 0; i < nfacesd; i++)
//    {
//        auto Fc = c.faces()[i]; // 面iの取得 
//        int nFcpoints = Fc.size() ;// 面iの頂点数取得
//        vtkIdType face[nFcpoints] ;
//        for (int k =0; k< nFcpoints; k++ )
//        {
//            face[k] = Fc[k];
//            std::cout << "Fc[k] = " <<  Fc[k] << std::endl;
//        }
//        vcell->InsertNextCell(nFcpoints, face);
//    }

#if 0
    // 頂点indexの取得
    int npoints = c.nPoints();   
//
//    const cell &cellVertices = mesh.cells()[cid];
//    int npoints = cellVertices.size();

//    std::cout << "cid = " << cid << std::endl;
//    std::cout << " npoints =  " << npoints << std::endl;
    vtkIdType pointIds[npoints]; // = {0, 1, 2, 3, 4, 5, 6, 7};

    for (int j =0 ; j< npoints; j++ )  pointIds[j] = c[j];
//    for (int j =0 ; j< npoints; j++ )  pointIds[j] = cellVertices[j];

  
// 面情報の取得
    auto vcell = vtkSmartPointer<vtkCellArray>::New();
    const labelList &cellFaces = mesh.cells()[cid];
    int nfaces = c.nFaces();

        // **セルの面情報を取得**
//        const labelList &cellFaces = mesh.cells()[cid]; // セルが参照する面インデックスリスト
        Info << "  Number of faces: " << cellFaces.size() << endl;

        // **セルの頂点情報を取得**
        // 頂点の重複を避けるためにセットを使用
        std::set<label> vertexSet;

        for (label faceI = 0; faceI < cellFaces.size(); ++faceI)
        {
            label faceIndex = cellFaces[faceI]; // 面インデックス
            const face &f = mesh.faces()[faceIndex]; // 面オブジェクト

            // 面を構成する各頂点をセットに追加
            for (label vertI = 0; vertI < f.size(); ++vertI)
            {
                vertexSet.insert(f[vertI]);
            }
        }

        // セットから頂点を出力
        Info << "  Number of unique vertices: " << vertexSet.size() << endl;
        for (auto vertIndex : vertexSet)
        {
            const point &vertexCoord = mesh.points()[vertIndex]; // 頂点座標
            Info << "    Vertex index " << vertIndex << ": " << vertexCoord << endl;
        }

 
//    int nfaces = cellFaces.size() ;
//    std::cout << "nfaces = " << nfaces << std::endl; 
//    for (int i = 0; i < nfaces; i++)
//    {
//        label faceIndex = cellFaces[i]; // 面インデックス
//        const face &f = mesh.faces()[faceIndex]; // 面オブジェクト
//
//        int nFcpoints = f.size();// 面iの頂点数取得
//        //auto Fc = c.faces()[i]; // 面iの取得 
//        vtkIdType face[nFcpoints] ;
////        std::cout << "nFcpoints = " << nFcpoints <<std::endl;
//        for (int k =0; k< nFcpoints; k++ )
//        {
//            face[k] = f[k];
//        }
//        vcell->InsertNextCell(nFcpoints, face);
//    }

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
    //if (npoints == 0 ) std::cout << "cid = " << cid  << ", nfaces = " << nfaces << std::endl; 
    std::cout << "cid = " << cid  << ", npoints = " << npoints  << ", nfaces = " << nfaces << std::endl; 
#endif 

        // **セルの面情報を取得**
//        const labelList &cellFaces = mesh.cells()[cid]; // セルが参照する面インデックスリスト
//        Info << "  Number of faces: " << cellFaces.size() << endl;

        // **セルの頂点情報を取得**
        // 頂点の重複を避けるためにセットを使用
        std::set<label> vertexSet;

        for (label faceI = 0; faceI < cellFaces.size(); ++faceI)
        {
            label faceIndex = cellFaces[faceI]; // 面インデックス
            const face &f = mesh.faces()[faceIndex]; // 面オブジェクト

            // 面を構成する各頂点をセットに追加
            for (label vertI = 0; vertI < f.size(); ++vertI)
            {
                vertexSet.insert(f[vertI]);
            }
        }

    int npoints = vertexSet.size();
    vtkIdType pointIds[npoints]; // = {0, 1, 2, 3, 4, 5, 6, 7};
    label index = 0;
    for (auto vertIndex : vertexSet)
    {
        pointIds[index] = static_cast<vtkIdType>(vertIndex); // vtkIdTypeにキャストして格納
        ++index;
    }


    int nfaces = cellFaces.size() ;
    for (int i = 0; i < nfaces; i++)
    {
        label faceIndex = cellFaces[i]; // 面インデックス
        const face &f = mesh.faces()[faceIndex]; // 面オブジェクト

        int nFcpoints = f.size();// 面iの頂点数取得
        vtkIdType face[nFcpoints] ;
        for (int k =0; k< nFcpoints; k++ )
        {
            face[k] = f[k];
        }
        vcell->InsertNextCell(nFcpoints, face);
    }

//        // セットから頂点を出力
//        Info << "  Number of unique vertices: " << vertexSet.size() << endl;
//        for (auto vertIndex : vertexSet)
//        {
//            const point &vertexCoord = mesh.points()[vertIndex]; // 頂点座標
//            Info << "    Vertex index " << vertIndex << ": " << vertexCoord << endl;
//        }

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

// pressuror
vtkSmartPointer<vtkFloatArray> scalars_p =
vtkSmartPointer<vtkFloatArray>::New();
scalars_p->SetName("p");
forAll(mesh.cells(),cid)
{
    scalars_p->InsertNextValue(p[cid]);
}
ucd -> GetCellData() -> AddArray(scalars_p);

vtkSmartPointer<vtkCellDataToPointData> cellDataToPointData =
vtkSmartPointer<vtkCellDataToPointData>::New();
cellDataToPointData -> SetInputData(ucd);
cellDataToPointData -> Update();
vtkPointData* pointData = cellDataToPointData->GetOutput()->GetPointData();
//ucd->GetPointData()->ShallowCopy(pointData);
//generate_particles_vtk(time_step, ucd);

vtkSmartPointer<vtkDataSetTriangleFilter> triangleFilter =
vtkSmartPointer<vtkDataSetTriangleFilter>::New();
triangleFilter->SetInputData(ucd);
triangleFilter->Update();

vtkSmartPointer<vtkUnstructuredGrid> ucd_tri = vtkSmartPointer<vtkUnstructuredGrid>::New();
ucd_tri = triangleFilter-> GetOutput();
ucd_tri->GetPointData()->ShallowCopy(pointData);

generate_particles_vtk(time_step, ucd_tri);
