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

#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/StructuredVolumeExporter"
#include "kvs/StructuredVolumeObject"

#include "Exporter/StructuredVolumeObjectExporter.h"
#include "Exporter/VtkExporter.h"
#include "FileFormat/KVSML/KvsmlStructuredVolumeObject.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/UnstructuredPfi.h"

void Vts2Vts( const std::string& dst, const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::VtkXmlStructuredGrid input_vts( src );
    cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &input_vts );

    kvs::StructuredVolumeObject* object = &importer;
    object->print( std::cout, kvs::Indent( 2 ) );

    std::cout << "Writing " << dst << " ..." << std::endl;
    cvt::VtkExporter<cvt::VtkXmlStructuredGrid> exporter( &importer );
    cvt::VtkXmlStructuredGrid* output_vts = &exporter;
    output_vts->write( dst );
}

void Vts2Kvsml( const std::string& directory, const std::string& base, const std::string& src )
{
    int last_time_step = 0;
    int time_step = 0;
    int sub_volume_id = 1;
    int sub_volume_count = 1;

    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::VtkXmlStructuredGrid input_vts( src );
    cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &input_vts );

    kvs::StructuredVolumeObject* object = &importer;
    object->updateMinMaxCoords();
    object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );
    object->print( std::cout, kvs::Indent( 2 ) );

    std::cout << "Writing to " << directory << " ..." << std::endl;
    cvt::StructuredVolumeObjectExporter exporter( &importer );
    exporter.setWritingDataTypeToExternalBinary();
    exporter.write( directory, base, time_step, sub_volume_id, sub_volume_count );

    cvt::UnstructuredPfi pfi( exporter.veclen(), last_time_step, sub_volume_count );
    pfi.registerObject( &exporter, time_step, sub_volume_id );
    pfi.write( directory, base );
}