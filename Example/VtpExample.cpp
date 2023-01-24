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

#include "kvs/KVSMLPolygonObject"
#include "kvs/PolygonExporter"
#include "kvs/PolygonObject"

#include "Exporter/VtkExporter.h"
#include "FileFormat/VTK/VtkXmlPolyData.h"
#include "Importer/VtkImporter.h"

void Vtp2Kvsml( const std::string& dst, const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::VtkXmlPolyData input_vtp( src );
    cvt::VtkImporter<cvt::VtkXmlPolyData> importer( &input_vtp );

    kvs::PolygonObject* polygon_object = &importer;
    polygon_object->print( std::cout, kvs::Indent( 2 ) );

    std::cout << "Writing " << dst << " ..." << std::endl;
    kvs::PolygonExporter<kvs::KVSMLPolygonObject> exporter( &importer );
    exporter.setWritingDataTypeToExternalBinary();
    exporter.write( dst );
}