#include <filter_io_vtk/FilterIoVtk.h>

using namespace std;
using namespace pbvr::filter;


void FilterIoVtk::vtkDataReader_DumpInfo(vtkDataReader* generic) {
    cout << "GENERIC vtkDataReaderInfo:" << endl;
    int num_scalars = generic->GetNumberOfScalarsInFile();
    int num_fields = generic->GetNumberOfFieldDataInFile();
    int num_vecs = generic->GetNumberOfVectorsInFile();
    int num_tens = generic->GetNumberOfTensorsInFile();
    int num_inports = generic->GetNumberOfInputPorts();
    int num_outorts = generic->GetNumberOfOutputPorts();

    cout << "vtkDataReader(" << generic->GetFileName() << "):" << endl;
    cout << "    GetHeader:" << generic->GetHeader() << endl;
    cout << "    GetNumberOfScalarsInFile:" << num_scalars << endl;
    cout << "    GetNumberOfFieldDataInFile:" << num_fields << endl;
    cout << "    GetNumberOfVectorsInFile:" << num_vecs << endl;
    cout << "    GetNumberOfTensorsInFile:" << num_tens << endl;
    cout << "    GetNumberOfInputPorts:" << num_inports << endl;
    cout << "    GetNumberOfOutputPorts:" << num_outorts << endl;
}

void FilterIoVtk::RectilinearGridReader_DumpInfo(vtkRectilinearGridReader *rgr) {
    cout << "get_RectilinearGridFileInfo:" << rgr->GetFileName() << endl;
    vtkRectilinearGrid* rg = rgr->GetOutput();
    vtkDataReader_DumpInfo(rgr);
    vtkRectilinearGrid_DumpInfo(rg);

}

void FilterIoVtk::StructuredPointsReader_DumpInfo(vtkStructuredPointsReader *spr) {
    cout << "get_StructuredPointsFileInfo:" << spr->GetFileName() << endl;
    vtkStructuredPoints* sp = spr->GetOutput();
    vtkDataReader_DumpInfo(spr);
    vtkStructuredPoints_DumpInfo(sp);
}

void FilterIoVtk::StructuredGridReader_DumpInfo(vtkStructuredGridReader *sgr) {
    cout << "get_StructuredGridFileInfo:" << sgr->GetFileName() << endl;
    vtkStructuredGrid* sg = sgr->GetOutput();
    vtkDataReader_DumpInfo(sgr);
    vtkStructuredGrid_DumpInfo(sg);
}

void FilterIoVtk::UnstructuredGridReader_DumpInfo(vtkUnstructuredGridReader *usgr) {
    cout << "get_UnstructuredGridFileInfo:" << usgr->GetFileName() << endl;
    vtkUnstructuredGrid* usg = usgr->GetOutput();
    vtkDataReader_DumpInfo(usgr);
    vtkUnstructuredGrid_DumpInfo(usg);
}

void FilterIoVtk::PolygonDataReader_DumpInfo(vtkPolyDataReader* pdr) {
    cout << "get_PolygonDataFileInfo:" << pdr->GetFileName() << endl;
    vtkDataReader_DumpInfo(pdr);
    vtkPolyData* polydata = pdr->GetOutput();
    vtkPolyData_DumpInfo(polydata);
}

void FilterIoVtk::vtkPolyDataReader_ReadFile(vtkPolyDataReader *reader, const char *filepath) {
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

void FilterIoVtk::vtkStructuredPointsReader_ReadFile(vtkStructuredPointsReader *reader, const char *filepath) {
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

void FilterIoVtk::vtkStructuredGridReader_ReadFile(vtkStructuredGridReader *reader, const char *filepath) {
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

void FilterIoVtk::vtkUnstructuredGridReader_ReadFile(vtkUnstructuredGridReader *reader, const char *filepath) {
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

void FilterIoVtk::vtkRectilinearGridReader_ReadFile(vtkRectilinearGridReader *reader, const char *filepath) {
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

void FilterIoVtk::vtkDataSetReader_ReadFile(vtkDataReader *reader, const char *filepath) {
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

VtkDatasetTypes FilterIoVtk::vtkDataReader_getFileType(const char *filepath) {
    VtkDatasetTypes fileType = PBVR_VTK_UNKNOWN;

    vtkDataReader *reader = vtkDataReader::New();
    reader->SetFileName(filepath);

    if (reader->IsFileStructuredPoints()) {
    fileType = PBVR_VTK_STRUCTURED_POINTS;
    } else if (reader->IsFileStructuredGrid()) {
    fileType = PBVR_VTK_STRUCTURED_GRID;
    } else if (reader->IsFileRectilinearGrid()) {
    fileType = PBVR_VTK_RECTILINEAR_GRID;
    } else if (reader->IsFilePolyData()) {
    fileType = PBVR_VTK_POLYDATA;
    } else if (reader->IsFileUnstructuredGrid()) {
    fileType = PBVR_VTK_UNSTRUCTURED_GRID;
    }

    reader->Delete();
    return fileType;
}

