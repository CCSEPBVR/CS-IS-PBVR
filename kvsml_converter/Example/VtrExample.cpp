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

#include "kvs/StructuredVolumeObject"

#include "Exporter/StructuredVolumeObjectExporter.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/UnstructuredPfi.h"

void Vtr2Kvsml( const std::string& directory, const std::string& base, const std::string& src )
{
    int last_time_step = 0;
    int time_step = 0;
    int sub_volume_id = 1;
    int sub_volume_count = 1;

    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlRectilinearGrid input_vtr( src );
    cvt::VtkImporter<cvt::VtkXmlRectilinearGrid> importer( &input_vtr );

    kvs::StructuredVolumeObject* object = &importer;
    object->updateMinMaxCoords();
    object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );
    object->print( std::cout, kvs::Indent( 2 ) );

    auto dst = std::string( directory ) + "/" + base + "_00000_0000001_0000001.kvsml";
    std::cout << "writing " << dst << " ..." << std::endl;
    cvt::StructuredVolumeObjectExporter exporter( &importer );
    kvs::KVSMLStructuredVolumeObject* writer = &exporter;
    writer->setWritingDataTypeToExternalBinary();
    writer->write( dst );

    cvt::UnstructuredPfi pfi( last_time_step, sub_volume_count );
    pfi.registerObject( &exporter, time_step, sub_volume_id );
    pfi.write( directory, base );
}