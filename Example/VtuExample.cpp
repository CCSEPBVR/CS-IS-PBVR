#include <iostream>
#include <string>

/*
#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/StructuredVolumeExporter"
*/
#include "kvs/UnstructuredVolumeObject"

#include "Exporter/VtkExporter.h"
#include "FileFormat/VTKXMLUnstructuredGrid/VtkXmlUntructuredGrid.h"
#include "Importer/VtkImporter.h"

#include "kvs/KVSMLUnstructuredVolumeObject"
#include "kvs/UnstructuredVolumeExporter"

void Vtu2Kvsml( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlUnstructuredGrid input_vtu( src );

    for ( auto vtu : input_vtu )
    {
        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &vtu );

        kvs::UnstructuredVolumeObject* object = &importer;
        std::cout << "#nodes: " << object->numberOfNodes() << std::endl;
        std::cout << "#cellType: " << object->cellType() << std::endl;

        std::cout << "writing " << dst << " ..." << std::endl;

        kvs::UnstructuredVolumeExporter<kvs::KVSMLUnstructuredVolumeObject> exporter( &importer );
        exporter.setWritingDataTypeToExternalBinary();
        exporter.write( dst );
    }
}