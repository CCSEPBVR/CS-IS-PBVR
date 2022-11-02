#include <iostream>
#include <string>

#include "kvs/StructuredVolumeExporter"
#include "kvs/StructuredVolumeObject"

#include "Exporter/VtkExporter.h"
#include "FileFormat/VTKXMLRectilinearGrid/VtkXmlRectilinearGrid.h"
#include "Importer/VtkImporter.h"

void Vtr2Vtr( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlRectilinearGrid input_vtr( src );
    cvt::VtkImporter<cvt::VtkXmlRectilinearGrid> importer( &input_vtr );

    kvs::StructuredVolumeObject* rectilinear_object = &importer;
    std::cout << "#nodes: " << rectilinear_object->numberOfNodes() << std::endl;
    std::cout << "#cells: " << rectilinear_object->numberOfCells() << std::endl;

    std::cout << "writing " << dst << " ..." << std::endl;
    cvt::VtkExporter<cvt::VtkXmlRectilinearGrid> exporter( &importer );
    cvt::VtkXmlRectilinearGrid* output_vtr = &exporter;
    output_vtr->write( dst );
}