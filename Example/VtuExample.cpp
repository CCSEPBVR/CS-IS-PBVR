#include <iostream>
#include <string>

#include "kvs/UnstructuredVolumeObject"

#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "FileFormat/KVSML/KvsmlUnstructuredVolumeObject.h"
#include "FileFormat/UnstructuredPfi.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "Importer/VtkImporter.h"

void Vtu2Kvsml( const char* directory, const char* base, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlUnstructuredGrid input_vtu( src );

    for ( auto vtu : input_vtu )
    {
        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &vtu );

        kvs::UnstructuredVolumeObject* object = &importer;
        std::cout << "#nodes: " << object->numberOfNodes() << std::endl;
        std::cout << "#cellType: " << object->cellType() << std::endl;

        std::cout << "writing to " << directory << std::endl;

        cvt::UnstructuredVolumeObjectExporter exporter( &importer );
        exporter.setWritingDataTypeToExternalBinary();
        exporter.writeForPbvr( directory, base,
                               0, // time step
                               1, // sub volume id
                               1  // sub volume count
        );
        // or
        // exporter.write( "<directory/<base>_00000_0000001_0000001.kvsml" );

        cvt::UnstructuredPfi pfi( object->veclen(),
                                  0, // last time step
                                  1  // sub volume count
        );
        pfi.registerObject( &exporter,
                            0, // time step
                            1  // sub volume id
        );
        pfi.write( directory, base );
        // or
        // pfi.write( "<directory/<base>.pfi" );
    }
}
