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

#include "kvs/LineExporter"
#include "kvs/UnstructuredVolumeObject"

#include "Exporter/StructuredVolumeObjectExporter.h"
#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "FileFormat/AVS/AvsUcd.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/Pfl.h"
#include "PBVRFileInformation/UnstructuredPfi.h"
#include "TimeSeriesFiles/AVS/AvsField.h"

#include <vtkXMLStructuredGridWriter.h>

void AvsFld2Kvsml( const std::string& directory, const std::string& base, const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::AvsField input_avs( src );

    int time_step = 0;
    int sub_volume_id = 1;
    int sub_volume_count = 1;

    if ( input_avs.isIrregular() )
    {
        for ( auto file_format : input_avs.eachTimeStepAsIrregular() )
        {
            cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( &file_format );
            importer.updateMinMaxCoords();
            importer.setMinMaxExternalCoords( importer.minObjectCoord(),
                                              importer.maxObjectCoord() );
            importer.print( std::cout, kvs::Indent( 2 ) );

            cvt::StructuredVolumeObjectExporter exporter( &importer );
            exporter.setWritingDataTypeToExternalBinary();
            exporter.write( directory, base, time_step, sub_volume_id, sub_volume_count );

            ++time_step;
        }
    }
    else if ( input_avs.isUnion() )
    {
        for ( auto file_format : input_avs.eachTimeStepAsUnion() )
        {
            cvt::VtkImporter<cvt::VtkXmlImageData> importer( &file_format );
            importer.updateMinMaxCoords();
            importer.setMinMaxExternalCoords( importer.minObjectCoord(),
                                              importer.maxObjectCoord() );
            importer.print( std::cout, kvs::Indent( 2 ) );

            cvt::StructuredVolumeObjectExporter exporter( &importer );
            exporter.setWritingDataTypeToExternalBinary();
            exporter.write( directory, base, time_step, sub_volume_id, sub_volume_count );

            ++time_step;
        }
    }
    else if ( input_avs.isRectilinear() )
    {
        for ( auto file_format : input_avs.eachTimeStepAsRectilinear() )
        {
            cvt::VtkImporter<cvt::VtkXmlRectilinearGrid> importer( &file_format );
            importer.updateMinMaxCoords();
            importer.setMinMaxExternalCoords( importer.minObjectCoord(),
                                              importer.maxObjectCoord() );
            importer.print( std::cout, kvs::Indent( 2 ) );

            cvt::StructuredVolumeObjectExporter exporter( &importer );
            exporter.setWritingDataTypeToExternalBinary();
            exporter.write( directory, base, time_step, sub_volume_id, sub_volume_count );

            ++time_step;
        }
    }
}

void AvsUcd2Kvsml( const std::string& directory, const std::string& base, const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::AvsUcd input_avs( src );

    cvt::Pfl pfl;

    for ( auto vtu : input_avs.eachCellType() )
    {
        int time_step = 0;
        int last_time_step = 0;
        int sub_volume_id = 1;
        int sub_volume_count = 1;

        cvt::VtkImporter<cvt::AvsUcd> importer( &vtu );
        std::cout << "  cell type: " << importer.cellType() << std::endl;

        kvs::UnstructuredVolumeObject* object = &importer;
        object->print( std::cout, kvs::Indent( 4 ) );

        std::cout << "  Writing to " << directory << " ..." << std::endl;
        auto local_base = std::string( base ) + "_" + std::to_string( object->cellType() );

        cvt::UnstructuredVolumeObjectExporter exporter( &importer );
        exporter.setWritingDataTypeToExternalBinary();
        exporter.write( directory, local_base, time_step, sub_volume_id, sub_volume_count );

        cvt::UnstructuredPfi pfi( object->veclen(), last_time_step, sub_volume_count );
        pfi.registerObject( &exporter, time_step, sub_volume_id );
        pfi.write( directory, local_base );
        pfi.print( std::cout, 2 );

        pfl.registerPfi( directory, local_base );
    }

    pfl.write( directory, base );
}