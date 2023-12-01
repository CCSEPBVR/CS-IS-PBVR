#include "wrapper_vtk_reader.h"
#include "wrapper_vtk_data.h"


/*  Wrap VTK Methods*/
WRAP_VTK_CTOR(vtkStructuredPointsReader);
WRAP_VTK_DTOR(vtkStructuredPointsReader);
WRAP_VTK_GETTER(vtkStructuredPointsReader, GetOutput, _vtkStructuredPoints);
WRAP_VTK_GETTER(vtkStructuredPointsReader, GetNumberOfScalarsInFile, int);
WRAP_VTK_GETTER(vtkStructuredPointsReader, GetNumberOfFieldDataInFile, int);

WRAP_VTK_CTOR(vtkStructuredGridReader);
WRAP_VTK_DTOR(vtkStructuredGridReader);
WRAP_VTK_GETTER(vtkStructuredGridReader, GetOutput, _vtkStructuredGrid);
WRAP_VTK_GETTER(vtkStructuredGridReader, GetNumberOfScalarsInFile, int);
WRAP_VTK_GETTER(vtkStructuredGridReader, GetNumberOfFieldDataInFile, int);

WRAP_VTK_CTOR(vtkRectilinearGridReader);
WRAP_VTK_DTOR(vtkRectilinearGridReader);
WRAP_VTK_GETTER(vtkRectilinearGridReader, GetOutput, _vtkRectilinearGrid);
WRAP_VTK_GETTER(vtkRectilinearGridReader, GetNumberOfScalarsInFile, int);
WRAP_VTK_GETTER(vtkRectilinearGridReader, GetNumberOfFieldDataInFile, int);

WRAP_VTK_CTOR(vtkUnstructuredGridReader);
WRAP_VTK_DTOR(vtkUnstructuredGridReader);
WRAP_VTK_GETTER(vtkUnstructuredGridReader, GetOutput, _vtkUnstructuredGrid);
WRAP_VTK_GETTER(vtkUnstructuredGridReader, GetNumberOfScalarsInFile, int);
WRAP_VTK_GETTER(vtkUnstructuredGridReader, GetNumberOfFieldDataInFile, int);

WRAP_VTK_CTOR(vtkPolyDataReader);
WRAP_VTK_DTOR(vtkPolyDataReader);
WRAP_VTK_GETTER(vtkPolyDataReader, GetOutput, _vtkPolyData);
WRAP_VTK_GETTER(vtkPolyDataReader, GetNumberOfScalarsInFile, int);
WRAP_VTK_GETTER(vtkPolyDataReader, GetNumberOfFieldDataInFile, int);

WRAP_VTK_CTOR(vtkDataReader);
WRAP_VTK_DTOR(vtkDataReader);
//WRAP_VTK_GETTER(vtkDataReader, GetOutput, _vtkDataSet);
WRAP_VTK_GETTER(vtkDataReader, GetNumberOfFieldDataInFile, int);
WRAP_VTK_GETTER(vtkDataReader, GetNumberOfScalarsInFile, int);


WRAP_VTK_CTOR(vtkDataSetReader);
WRAP_VTK_DTOR(vtkDataSetReader);
WRAP_VTK_GETTER(vtkDataSetReader, GetNumberOfFieldDataInFile, int);
WRAP_VTK_GETTER(vtkDataSetReader, GetNumberOfScalarsInFile, int);
WRAP_VTK_GETTER(vtkDataSetReader, GetOutput, _vtkDataSet);
WRAP_VTK_GETTER(vtkDataSetReader, GetPolyDataOutput, _vtkPolyData);
WRAP_VTK_GETTER(vtkDataSetReader, GetUnstructuredGridOutput, _vtkUnstructuredGrid);
WRAP_VTK_GETTER(vtkDataSetReader, GetStructuredGridOutput, _vtkStructuredGrid);
WRAP_VTK_GETTER(vtkDataSetReader, GetRectilinearGridOutput, _vtkRectilinearGrid);
WRAP_VTK_GETTER(vtkDataSetReader, GetStructuredPointsOutput, _vtkStructuredPoints);

void vtkDataReader_DumpInfo(vtkDataReader* generic) {
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

void RectilinearGridReader_DumpInfo(_vtkRectilinearGridReader _rgr) {
    vtkRectilinearGridReader* rgr = _rgr;
    cout << "get_RectilinearGridFileInfo:" << rgr->GetFileName() << endl;
    vtkRectilinearGrid* rg = rgr->GetOutput();
    vtkDataReader_DumpInfo(rgr);
    vtkRectilinearGrid_DumpInfo(rg);
}

void StructuredPointsReader_DumpInfo(_vtkStructuredPointsReader _spr) {
    vtkStructuredPointsReader* spr = _spr;
    cout << "get_StructuredPointsFileInfo:" << spr->GetFileName() << endl;
    vtkStructuredPoints* sp = spr->GetOutput();
    vtkDataReader_DumpInfo(spr);
    vtkStructuredPoints_DumpInfo(sp);
}

void StructuredGridReader_DumpInfo(_vtkStructuredGridReader _sgr) {
    vtkStructuredGridReader* sgr = _sgr;
    cout << "get_StructuredGridFileInfo:" << sgr->GetFileName() << endl;
    vtkStructuredGrid* sg = sgr->GetOutput();
    vtkDataReader_DumpInfo(sgr);
    vtkStructuredGrid_DumpInfo(sg);
}

void UnstructuredGridReader_DumpInfo(_vtkUnstructuredGridReader _usgr) {
    vtkUnstructuredGridReader* usgr = _usgr;
    cout << "get_UnstructuredGridFileInfo:" << usgr->GetFileName() << endl;
    vtkUnstructuredGrid* usg = usgr->GetOutput();
    vtkDataReader_DumpInfo(usgr);
    vtkUnstructuredGrid_DumpInfo(usg);
}

void PolygonDataReader_DumpInfo(vtkPolyDataReader* _pdr) {
    vtkPolyDataReader* pdr = _pdr;
    cout << "get_PolygonDataFileInfo:" << pdr->GetFileName() << endl;
    vtkDataReader_DumpInfo(pdr);
    vtkPolyData* polydata = pdr->GetOutput();
    vtkPolyData_DumpInfo(polydata);
}

void vtkPolyDataReader_ReadFile(_vtkPolyDataReader _reader, const char *filepath) {
    vtkPolyDataReader* reader = _reader;
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

void vtkStructuredPointsReader_ReadFile(_vtkStructuredPointsReader _reader, const char *filepath) {
    vtkStructuredPointsReader* reader = _reader;
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

void vtkStructuredGridReader_ReadFile(_vtkStructuredGridReader _reader, const char *filepath) {
    vtkStructuredGridReader* reader = _reader;
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

void vtkUnstructuredGridReader_ReadFile(_vtkUnstructuredGridReader _reader, const char *filepath) {
    vtkUnstructuredGridReader* reader = _reader;
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

void vtkRectilinearGridReader_ReadFile(_vtkRectilinearGridReader _reader, const char *filepath) {
    vtkRectilinearGridReader* reader = _reader;
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

void vtkDataSetReader_ReadFile(_vtkDataReader _reader, const char *filepath) {
    vtkDataReader* reader = _reader;
    reader->SetFileName(filepath);
    reader->ReadAllVectorsOn();
    reader->ReadAllScalarsOn();
    reader->ReadAllFieldsOn();
    reader->Update();
}

VtkDatasetTypes vtkDataReader_getFileType(const char *filepath) {
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