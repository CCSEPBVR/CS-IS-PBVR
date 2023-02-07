/*
 * Created by Japan Atomic Energy Agency
 *
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legal code along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
#include <iostream>
#include <string>

#include "kvs/PolygonExporter"
#include "kvs/PolygonObject"

#include "Exporter/VtkExporter.h"
#include "FileFormat/STL/Stl.h"
#include "Importer/VtkImporter.h"

void Stl2Stl( const std::string& dst, const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::Stl input_stl( src, []( vtkSTLReader* stl_reader ) { stl_reader->MergingOn(); } );
    cvt::VtkImporter<cvt::Stl> importer( &input_stl );

    kvs::PolygonObject* polygon_object = &importer;
    polygon_object->print( std::cout, kvs::Indent( 2 ) );

    std::cout << "Writing " << dst << " ..." << std::endl;
    kvs::PolygonExporter<kvs::Stl> exporter( &importer );
    kvs::Stl* output_stl = &exporter;
    output_stl->write( dst );
}

void Stl2Kvsml( const std::string& dst, const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::Stl input_stl( src, []( vtkSTLReader* stl_reader ) { stl_reader->MergingOn(); } );
    cvt::VtkImporter<cvt::Stl> importer( &input_stl );

    kvs::PolygonObject* polygon_object = &importer;
    polygon_object->print( std::cout, kvs::Indent( 2 ) );

    std::cout << "Writing " << dst << " ..." << std::endl;
    kvs::PolygonExporter<kvs::KVSMLPolygonObject> exporter( &importer );
    exporter.setWritingDataTypeToExternalBinary();
    exporter.write( dst );
}
