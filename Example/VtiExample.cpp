#include <iostream>
#include <string>

#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/StructuredVolumeObject"

#include "Exporter/StructuredVolumeObjectExporter.h"
#include "Exporter/VtkExporter.h"
#include "FileFormat/VTK/VtkXmlImageData.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/UnstructuredPfi.h"
#include "TimeSeriesFiles/NumeralSequenceFiles.h"

void Vti2Kvsml( const char* directory, const char* base, const char* src )
{
    int last_time_step = 0;
    int time_step = 0;
    int sub_volume_id = 1;
    int sub_volume_count = 1;

    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::VtkXmlImageData input_vti( src );
    cvt::VtkImporter<cvt::VtkXmlImageData> importer( &input_vti );

    kvs::StructuredVolumeObject* object = &importer;
    object->updateMinMaxCoords();
    object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );
    object->print( std::cout, kvs::Indent( 2 ) );

    std::cout << "Writing ..." << std::endl;
    cvt::StructuredVolumeObjectExporter exporter( &importer );
    exporter.setWritingDataTypeToExternalBinary();
    exporter.write( directory, base, time_step, sub_volume_id, sub_volume_count );

    cvt::UnstructuredPfi pfi( exporter.veclen(), last_time_step, sub_volume_count );
    pfi.registerObject( &exporter, time_step, sub_volume_id );
    pfi.write( directory, base );
}

void SeriesVti2Kvsml( const char* directory, const char* base, const char* src )
{
    std::cout << "Reading " << src << " ..." << std::endl;

    cvt::NumeralSequenceFiles<cvt::VtkXmlImageData> time_series( src );
    int last_time_step = time_series.numberOfFiles() - 1;
    int time_step = 0;
    int sub_volume_id = 1;
    int sub_volume_count = 1;

    std::shared_ptr<cvt::UnstructuredPfi> pfi;

    for ( auto input_vti : time_series.eachTimeStep() )
    {
        cvt::VtkImporter<cvt::VtkXmlImageData> importer( &input_vti );

        kvs::StructuredVolumeObject* object = &importer;
        object->updateMinMaxCoords();
        object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );
        object->print( std::cout, kvs::Indent( 2 ) );

        std::cout << "Writing ..." << std::endl;
        cvt::StructuredVolumeObjectExporter exporter( &importer );
        exporter.setWritingDataTypeToExternalBinary();
        exporter.write( directory, base, time_step, sub_volume_id, sub_volume_count );

        if ( time_step == 0 )
        {
            pfi = std::make_shared<cvt::UnstructuredPfi>( exporter.veclen(), last_time_step,
                                                          sub_volume_count );
        }

        pfi->registerObject( &exporter, time_step, sub_volume_id );

        ++time_step;
    }

    pfi->write( directory, base );
}
