#include <iostream>
#include <string>

#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/StructuredVolumeExporter"
#include "kvs/StructuredVolumeObject"

#include "CvtTag.h"
#include "Exporter/VtkExporter.h"
#include "FileFormat/UniformVTKXMLStructuredGrid/UniformVtkXmlStructuredGrid.h"
#include "Importer/VtkImporter.h"

void Vts2Vts( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::UniformVtkXmlStructuredGrid input_vts( src );
    cvt::VtkImporter<cvt::UniformVtkXmlStructuredGrid> importer( &input_vts );

    kvs::StructuredVolumeObject* object = &importer;
    std::cout << "#nodes: " << object->numberOfNodes() << std::endl;

    std::cout << "writing " << dst << " ..." << std::endl;
    cvt::VtkExporter<cvt::UniformVtkXmlStructuredGrid> exporter( &importer );
    cvt::UniformVtkXmlStructuredGrid* output_vts = &exporter;
    output_vts->write( dst );
}

void Vts2Kvsml( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::UniformVtkXmlStructuredGrid input_vts( src );
    cvt::VtkImporter<cvt::UniformVtkXmlStructuredGrid> importer( &input_vts );

    kvs::StructuredVolumeObject* object = &importer;
    std::cout << "#nodes: " << object->numberOfNodes() << std::endl;

    std::cout << "writing " << dst << " ..." << std::endl;
    kvs::StructuredVolumeExporter<kvs::KVSMLStructuredVolumeObject> exporter( &importer );
    kvs::KVSMLStructuredVolumeObject* output_kvsml = &exporter;
    output_kvsml->write( dst );
}