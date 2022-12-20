#include <iostream>
#include <string>

#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "FileFormat/VTK/VtkXmlPolyData.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/Pfl.h"
#include "PBVRFileInformation/UnstructuredPfi.h"
#include "TimeSeriesFiles/EnSight/EnSightGoldBinary.h"

void Case2Kvsml( const char* directory, const char* base, const char* src )
{
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::EnSightGoldBinary input_case( src );

    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;

    auto time_steps_container = input_case.eachTimeStep();
    int last_time_step = time_steps_container.lastTimeStep();
    int time_step = 0;
    int sub_volume_count = 0;

    for ( auto time_and_format : time_steps_container )
    {
        auto time = time_and_format.first;
        auto& multi_block_format = time_and_format.second;

        std::cout << "  time: " << time << std::endl;
        if ( time_step == 0 )
        {
            // Count sub volumes at first
            for ( auto format : multi_block_format.eachBlock() )
            {
                if ( format )
                {
                    if ( auto unstructured_volume_format =
                             dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
                    {
                        try
                        {
                            cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer(
                                unstructured_volume_format );
                            ++sub_volume_count;
                        }
                        catch ( ... )
                        {
                            // Do nothing
                        }
                    }
                }
            }
        }

        int sub_volume_id = 1;
        for ( auto format : multi_block_format.eachBlock() )
        {
            std::cout << "    sub volume id:" << sub_volume_id << std::endl;
            if ( !format )
            {
                std::cout << "      Unsupported VTK data type" << std::endl;
            }
            else if ( auto unstructured_volume_format =
                          dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
            {
                try
                {
                    cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer(
                        unstructured_volume_format );

                    kvs::UnstructuredVolumeObject* object = &importer;
                    object->print( std::cout, kvs::Indent( 6 ) );

                    auto local_base =
                        std::string( base ) + "_" + std::to_string( object->cellType() );

                    cvt::UnstructuredVolumeObjectExporter exporter( &importer );
                    exporter.setWritingDataTypeToExternalBinary();
                    // Write
                    exporter.write( directory, local_base, time_step, sub_volume_id,
                                    sub_volume_count );
                    // or
                    // exporter.write(
                    //    "<directory>/<local_base>_<time_step>_<sub_volume_id>_<sub_volume_count>.kvsml"
                    // );

                    if ( time_step == 0 )
                    {
                        pfi_map.emplace( static_cast<int>( object->cellType() ),
                                         cvt::UnstructuredPfi( object->veclen(), last_time_step,
                                                               sub_volume_count ) );
                    }
                    pfi_map.at( static_cast<int>( object->cellType() ) )
                        .registerObject( &exporter, time_step, sub_volume_id );
                }
                catch ( std::runtime_error& e )
                {
                    std::cout << "      " << e.what() << std::endl;
                }
            }
            else
            {
                std::cout << "      Supported, but non-unstructured grid" << std::endl;
            }

            ++sub_volume_id;
        }

        ++time_step;
    }

    cvt::Pfl pfl;
    for ( auto& e : pfi_map )
    {
        std::string local_base = std::string( base ) + "_" + std::to_string( e.first );
        e.second.write( directory, local_base );
        // or
        // e.second.write( "<directory>/<local_base>.pfi" );

        pfl.registerPfi( directory, local_base );
    }
    pfl.write( directory, base );
}