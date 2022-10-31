#include <iostream>
#include <string>

#include "kvs/ObjectBase"
#include "kvs/PolygonObject"

#include "FileFormat/VTKMultiBlock/VtkMultiBlock.h"
#include "FileFormat/VTKXMLPolyData/VtkXmlPolyData.h"
#include "Importer/VtkImporter.h"

void Vtm2Vtm( const char* dst, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkMultiBlock input_vtm( src );

    for ( auto format : input_vtm )
    {
        if ( auto polydata_format = dynamic_cast<cvt::VtkXmlPolyData*>( format.get() ) )
        {
            cvt::VtkImporter<cvt::VtkXmlPolyData> importer( polydata_format );
            if ( kvs::PolygonObject* polygon_object = kvs::PolygonObject::DownCast( &importer ) )
            {
                std::cout << "polygon object was converted" << std::endl;
                std::cout << "#vertices: " << polygon_object->numberOfVertices() << std::endl;
                std::cout << "#normals: " << polygon_object->numberOfNormals() << std::endl;
            }
        }
        else
        {
            std::cout << "failed to read" << std::endl;
        }
    }
}