#include <iostream>

#include "kvs/ObjectBase"
#include "kvs/PolygonObject"

#include "FileFormat/VTK/VtkXmlPolyData.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "FileFormat/VTKMultiBlock/VtkMultiBlock.h"
#include "Importer/VtkImporter.h"

void Vtm2Vtm( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkMultiBlock input_vtm( src );

    for ( auto format : input_vtm.eachBlocks() )
    {
        if ( auto polydata_format = dynamic_cast<cvt::VtkXmlPolyData*>( format.get() ) )
        {
            cvt::VtkImporter<cvt::VtkXmlPolyData> importer( polydata_format );
            if ( kvs::PolygonObject* polygon_object = kvs::PolygonObject::DownCast( &importer ) )
            {
                std::cout << "A polygon object was converted" << std::endl;
                std::cout << "#vertices: " << importer.numberOfVertices() << std::endl;
                std::cout << "#normals: " << importer.numberOfNormals() << std::endl;
            }
        }
        else if ( auto structured_volume_format =
                      dynamic_cast<cvt::VtkXmlRectilinearGrid*>( format.get() ) )
        {
            cvt::VtkImporter<cvt::VtkXmlRectilinearGrid> importer( structured_volume_format );

            std::cout << "A rectilinear structured volume object was converted" << std::endl;
            std::cout << "#nodes: " << importer.numberOfNodes() << std::endl;
            std::cout << "#cells: " << importer.numberOfCells() << std::endl;
        }
        else if ( auto unstructured_volume_format =
                      dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
        {
            cvt::VtkImporter<cvt::VtkXmlRectilinearGrid> importer( structured_volume_format );

            std::cout << "An unstructured volume object was converted" << std::endl;
            std::cout << "#nodes: " << importer.numberOfNodes() << std::endl;
            std::cout << "#cells: " << importer.numberOfCells() << std::endl;
        }
        else
        {
            std::cout << "An unsupported vtk data type" << std::endl;
        }
    }
}