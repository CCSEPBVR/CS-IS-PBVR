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
#include "FileFormat/VTK/VtkXmlPStructuredGrid.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/UnstructuredPfi.h"

void Pvts2Kvsml( const std::string& directory, const std::string& base, const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::VtkXmlPStructuredGrid input_pvts( src );
    std::cout << "#piece: " << input_pvts.numberOfPieces() << std::endl;

    int last_time_step = 0;
    int time_step = 0;
    int sub_volume_count = input_pvts.numberOfPieces();
    int sub_volume_id = 1;

    std::shared_ptr<cvt::UnstructuredPfi> pfi;

    for ( auto file_format : input_pvts.eachPiece() )
    {
        std::cout << "Importing ..." << std::endl;
        cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &file_format );
        kvs::StructuredVolumeObject* object = &importer;
        object->print( std::cout, kvs::Indent( 2 ) );

        std::cout << "Writing ..." << std::endl;
        cvt::StructuredVolumeObjectExporter exporter( &importer );
        exporter.write( directory, base, time_step, sub_volume_id, sub_volume_count );

        if ( !pfi )
        {
            pfi = std::make_shared<cvt::UnstructuredPfi>( object->veclen(), last_time_step,
                                                          sub_volume_count );
        }
        pfi->registerObject( &exporter, time_step, sub_volume_id );

        ++sub_volume_id;
    }

    pfi->write( directory, base );
}
