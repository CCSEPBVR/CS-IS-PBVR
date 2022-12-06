#include <iostream>
#include <string>

#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/StructuredVolumeExporter"
#include "kvs/StructuredVolumeObject"

#include "CvtMode.h"
#include "Exporter/VtkExporter.h"
#include "FileFormat/VTK/VtkXmlPStructuredGrid.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "Importer/VtkImporter.h"

void UniformPvts2Vts( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlPStructuredGrid input_pvts( src, true );
    std::cout << "#piece: " << input_pvts.number_of_pieces() << std::endl;

    int count = 0;
    for ( auto file_format : input_pvts )
    {
        std::cout << "importing ..." << std::endl;
        cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( file_format.get(),
                                                              cvt::UNIFORM_GRID_MODE );
        kvs::StructuredVolumeObject* object = &importer;
        std::cout << "#nodes: " << object->numberOfNodes() << std::endl;
        std::cout << "#resolution: [" << object->resolution()[0] << ", " << object->resolution()[1]
                  << ", " << object->resolution()[2] << "]" << std::endl;

        std::string output_filename =
            std::string( dst ) + "." + std::to_string( count++ ) + ".kvsml";
        std::cout << "writing " << output_filename << " ..." << std::endl;
        kvs::StructuredVolumeExporter<kvs::KVSMLStructuredVolumeObject> exporter( &importer );
        kvs::KVSMLStructuredVolumeObject* output_kvsml = &exporter;
        output_kvsml->write( output_filename );
    }
}
