#include <iostream>

#include "kvs/ObjectBase"
#include "kvs/PolygonObject"

#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "FileFormat/NumeralSequenceFiles.h"
#include "FileFormat/Pfl.h"
#include "FileFormat/UnstructuredPfi.h"
#include "FileFormat/VTK/VtkXmlImageData.h"
#include "FileFormat/VTK/VtkXmlMultiBlock.h"
#include "FileFormat/VTK/VtkXmlPolyData.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "Importer/VtkImporter.h"

void AccessToVtm( const char* src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::VtkXmlMultiBlock input_vtm( src );

    for ( auto format : input_vtm.eachBlock() )
    {
        if ( auto polydata_format = dynamic_cast<cvt::VtkXmlPolyData*>( format.get() ) )
        {
            std::cout << "A polygon object was converted" << std::endl;
            cvt::VtkImporter<cvt::VtkXmlPolyData> importer( polydata_format );
            kvs::PolygonObject* polygon_object = kvs::PolygonObject::DownCast( &importer );
            polygon_object->print( std::cout, kvs::Indent( 2 ) );
        }
        else if ( auto structured_volume_format =
                      dynamic_cast<cvt::VtkXmlRectilinearGrid*>( format.get() ) )
        {
            std::cout << "A rectilinear structured volume object was converted" << std::endl;
            cvt::VtkImporter<cvt::VtkXmlRectilinearGrid> importer( structured_volume_format );
            importer.print( std::cout, kvs::Indent( 2 ) );
        }
        else if ( auto structured_volume_format =
                      dynamic_cast<cvt::VtkXmlImageData*>( format.get() ) )
        {
            std::cout << "An uniform structured volume object was converted" << std::endl;
            cvt::VtkImporter<cvt::VtkXmlImageData> importer( structured_volume_format );
            importer.print( std::cout, kvs::Indent( 2 ) );
        }
        else if ( auto structured_volume_format =
                      dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) )
        {
            std::cout << "A curvilinear structured volume object was converted" << std::endl;
            cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( structured_volume_format );
            importer.print( std::cout, kvs::Indent( 2 ) );
        }
        else if ( auto unstructured_volume_format =
                      dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
        {
            std::cout << "An unstructured volume object was converted" << std::endl;
            cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( unstructured_volume_format );
            importer.print( std::cout, kvs::Indent( 2 ) );
        }
        else
        {
            std::cout << "An unsupported vtk data type" << std::endl;
        }
    }
}

void SeriesVtm2Kvsml( const char* directory, const char* base, const char* src )
{
    std::cout << "Reading " << src << " ..." << std::endl;

    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;

    cvt::NumeralSequenceFiles<cvt::VtkXmlMultiBlock> time_series( src );
    int last_time_step = time_series.numberOfFiles() - 1;
    int time_step = 0;
    int sub_volume_id = 1;
    std::unordered_map<int, int> sub_volume_counts;

    for ( auto input_vtm : time_series.eachTimeStep() )
    {
        // One-pass to collect information.
        if ( time_step == 0 )
        {
            std::cout << "Reading 0 step objects to collect information" << std::endl;
            std::unordered_map<int, int> veclens;

            for ( auto format : input_vtm.eachBlock() )
            {
                if ( auto input_vtu = dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
                {
                    for ( auto vtu : input_vtu->eachCellType() )
                    {
                        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( input_vtu );
                        kvs::UnstructuredVolumeObject* object = &importer;

                        auto cell_type = object->cellType();
                        sub_volume_counts[cell_type] = ( sub_volume_counts.count( cell_type ) == 0 )
                                                           ? 1
                                                           : ( sub_volume_counts[cell_type] + 1 );
                        veclens[cell_type] =
                            ( veclens.count( cell_type ) == 0 ) ? 1 : ( veclens[cell_type] + 1 );
                    }
                }
            }

            for ( auto& e : sub_volume_counts )
            {
                auto cell_type = e.first;

                pfi_map.emplace( static_cast<int>( cell_type ),
                                 cvt::UnstructuredPfi( veclens[cell_type], last_time_step,
                                                       sub_volume_counts[cell_type] ) );
            }
        }

        // Two-pass
        std::unordered_map<int, int> sub_volume_ids;
        for ( auto& e : sub_volume_counts )
        {
            sub_volume_ids[e.first] = 1;
        }
        for ( auto format : input_vtm.eachBlock() )
        {
            if ( auto input_vtu = dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
            {
                for ( auto vtu : input_vtu->eachCellType() )
                {
                    cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &vtu );
                    kvs::UnstructuredVolumeObject* object = &importer;
                    object->print( std::cout, kvs::Indent( 2 ) );

                    std::cout << "  Writing to " << directory << " ..." << std::endl;
                    auto local_base =
                        std::string( base ) + "_" + std::to_string( object->cellType() );

                    cvt::UnstructuredVolumeObjectExporter exporter( &importer );
                    exporter.setWritingDataTypeToExternalBinary();
                    exporter.write( directory, local_base, time_step,
                                    sub_volume_ids[object->cellType()],
                                    sub_volume_counts[object->cellType()] );

                    pfi_map.at( static_cast<int>( object->cellType() ) )
                        .registerObject( &exporter, time_step, sub_volume_id );

                    ++sub_volume_ids[object->cellType()];
                }
            }
            else
            {
                std::cout << "An unsupported vtk data type" << std::endl;
            }
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