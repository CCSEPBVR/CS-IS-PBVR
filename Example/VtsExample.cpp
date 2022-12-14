#include <iostream>
#include <string>

#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/StructuredVolumeExporter"
#include "kvs/StructuredVolumeObject"

#include "Exporter/VtkExporter.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "Importer/VtkImporter.h"

void Vts2Vts( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlStructuredGrid input_vts( src );
    cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &input_vts );

    kvs::StructuredVolumeObject* object = &importer;
    object->print( std::cout, kvs::Indent( 2 ) );

    std::cout << "writing " << dst << " ..." << std::endl;
    cvt::VtkExporter<cvt::VtkXmlStructuredGrid> exporter( &importer );
    cvt::VtkXmlStructuredGrid* output_vts = &exporter;
    output_vts->write( dst );
}
