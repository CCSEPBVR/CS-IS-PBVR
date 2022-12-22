#include <iostream>
#include <string>

#include "kvs/KVSMLLineObject"
#include "kvs/KVSMLPointObject"
#include "kvs/LineExporter"
#include "kvs/PointExporter"
#include "kvs/PolygonExporter"
#include "kvs/PolygonObject"
#include "kvs/UnstructuredVolumeObject"

#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "FileFormat/KVSML/KvsmlUnstructuredVolumeObject.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/Pfl.h"
#include "PBVRFileInformation/UnstructuredPfi.h"
#include "TimeSeriesFiles/NumeralSequenceFiles.h"

void Vtu2Kvsml( const std::string& directory, const std::string& base, const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::VtkXmlUnstructuredGrid input_vtu( src );

    cvt::Pfl pfl;

    for ( auto vtu : input_vtu.eachCellType() )
    {
        int time_step = 0;
        int last_time_step = 0;
        int sub_volume_id = 1;
        int sub_volume_count = 1;

        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &vtu );
        std::cout << "  cell type: " << importer.cellType() << std::endl;

        kvs::UnstructuredVolumeObject* object = &importer;
        object->print( std::cout, kvs::Indent( 4 ) );

        std::cout << "  Writing to " << directory << " ..." << std::endl;
        auto local_base = std::string( base ) + "_" + std::to_string( object->cellType() );

        cvt::UnstructuredVolumeObjectExporter exporter( &importer );
        exporter.setWritingDataTypeToExternalBinary();
        exporter.write( directory, local_base, time_step, sub_volume_id, sub_volume_count );
        // or
        // exporter.write( "<directory>/<local_base>_00000_0000001_0000001.kvsml" );

        cvt::UnstructuredPfi pfi( object->veclen(), last_time_step, sub_volume_count );
        pfi.registerObject( &exporter, time_step, sub_volume_id );
        pfi.write( directory, local_base );
        // or
        // pfi.write( "<directory>/<local_base>.pfi" );

        pfl.registerPfi( directory, local_base );
    }

    pfl.write( directory, base );
    // or
    // pfl.write( "<directory/<base>.pfi" );
}

void SeriesVtu2Kvsml( const std::string& directory, const std::string& base,
                      const std::string& src )
{
    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;

    cvt::NumeralSequenceFiles<cvt::VtkXmlUnstructuredGrid> time_series( src );
    int last_time_step = time_series.numberOfFiles() - 1;
    int time_step = 0;
    int sub_volume_id = 1;
    int sub_volume_count = 1;

    for ( auto whole_vtu : time_series.eachTimeStep() )
    {
        std::cout << "Reading " << whole_vtu.filename() << " ..." << std::endl;

        for ( auto vtu : whole_vtu.eachCellType() )
        {
            cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &vtu );
            std::cout << "  cell type: " << importer.cellType() << std::endl;

            kvs::UnstructuredVolumeObject* object = &importer;
            object->print( std::cout, kvs::Indent( 4 ) );

            std::cout << "  Writing to " << directory << " ..." << std::endl;
            auto local_base = std::string( base ) + "_" + std::to_string( object->cellType() );

            cvt::UnstructuredVolumeObjectExporter exporter( &importer );
            exporter.setWritingDataTypeToExternalBinary();
            exporter.write( directory, local_base, time_step, sub_volume_id, sub_volume_count,
                            // To share coords data, set the flag
                            true );

            if ( time_step == 0 )
            {
                pfi_map.emplace(
                    static_cast<int>( object->cellType() ),
                    cvt::UnstructuredPfi( object->veclen(), last_time_step, sub_volume_count ) );
            }
            pfi_map.at( static_cast<int>( object->cellType() ) )
                .registerObject( &exporter, time_step, sub_volume_id );
        }

        ++time_step;
    }

    cvt::Pfl pfl;
    for ( auto& e : pfi_map )
    {
        std::string local_base = std::string( base ) + "_" + std::to_string( e.first );
        e.second.write( directory, local_base );

        pfl.registerPfi( directory, local_base );
    }
    pfl.write( directory, base );
}

void PointVtu2Kvsml( const std::string& dst, const std::string& src )
{
    cvt::VtkXmlUnstructuredGrid input_vtu( src );
    if ( input_vtu.isPointObjectConvertible() )
    {
        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid, kvs::PointObject> importer( &input_vtu );
        importer.print( std::cout );
        importer.setSize( 0.01 );

        kvs::PointExporter<kvs::KVSMLPointObject> exporter( &importer );
        exporter.write( dst );
    }
    else
    {
        std::cerr << src << " could not export as a KVS PointObject" << std::endl;
    }
}

void LineVtu2Kvsml( const std::string& dst, const std::string& src )
{
    cvt::VtkXmlUnstructuredGrid input_vtu( src );
    if ( input_vtu.isLineObjectConvertible() )
    {
        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid, kvs::LineObject> importer( &input_vtu );
        importer.print( std::cout );

        kvs::LineExporter<kvs::KVSMLLineObject> exporter( &importer );
        exporter.write( dst );
    }
    else
    {
        std::cerr << src << " could not export as a KVS LineObject" << std::endl;
    }
}

void TriangleVtu2Kvsml( const std::string& dst, const std::string& src )
{
    cvt::VtkXmlUnstructuredGrid input_vtu( src );
    if ( input_vtu.isPolygonObjectConvertible() )
    {
        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid, kvs::PolygonObject> importer( &input_vtu );
        importer.print( std::cout );

        importer.setColor( kvs::RGBColor( 255, 255, 255 ) );

        kvs::PolygonExporter<kvs::KVSMLPolygonObject> exporter( &importer );
        exporter.write( dst );
    }
    else
    {
        std::cerr << src << " could not export as a KVS PolygonObject" << std::endl;
    }
}
