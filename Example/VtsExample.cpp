#include <iostream>
#include <string>

#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/StructuredVolumeExporter"
#include "kvs/StructuredVolumeObject"

#include "CvtMode.h"
#include "Exporter/VtkExporter.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "Importer/VtkImporter.h"

void UniformVts2Vts( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlStructuredGrid input_vts( src );
    cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &input_vts, cvt::UNIFORM_GRID_MODE );

    kvs::StructuredVolumeObject* object = &importer;
    std::cout << "#nodes: " << object->numberOfNodes() << std::endl;

    std::cout << "writing " << dst << " ..." << std::endl;
    cvt::VtkExporter<cvt::VtkXmlStructuredGrid> exporter( &importer );
    cvt::VtkXmlStructuredGrid* output_vts = &exporter;
    output_vts->write( dst );
}

void UniformVts2Kvsml( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlStructuredGrid input_vts( src );
    cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &input_vts, cvt::UNIFORM_GRID_MODE );

    kvs::StructuredVolumeObject* object = &importer;
    std::cout << "#nodes: " << object->numberOfNodes() << std::endl;

    std::cout << "writing " << dst << " ..." << std::endl;
    kvs::StructuredVolumeExporter<kvs::KVSMLStructuredVolumeObject> exporter( &importer );
    kvs::KVSMLStructuredVolumeObject* output_kvsml = &exporter;
    output_kvsml->write( dst );
}

void IrregularVts2Vts( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlStructuredGrid input_vts( src );
    cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &input_vts, cvt::CURVILINEAR_GRID_MODE );

    kvs::StructuredVolumeObject* object = &importer;
    std::cout << "#nodes: " << object->numberOfNodes() << std::endl;

    std::cout << "writing " << dst << " ..." << std::endl;
    cvt::VtkExporter<cvt::VtkXmlStructuredGrid> exporter( &importer );
    cvt::VtkXmlStructuredGrid* output_vts = &exporter;
    output_vts->write( dst );
}

void IrregularVts2Avs( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlStructuredGrid input_vts( src );
    cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &input_vts, cvt::CURVILINEAR_GRID_MODE );

    kvs::StructuredVolumeObject* object = &importer;
    std::cout << "#nodes: " << object->numberOfNodes() << std::endl;

    std::cout << "writing " << dst << " ..." << std::endl;
    kvs::StructuredVolumeExporter<kvs::AVSField> exporter( &importer );
    kvs::AVSField* output_avs = &exporter;
    output_avs->write( dst );
}