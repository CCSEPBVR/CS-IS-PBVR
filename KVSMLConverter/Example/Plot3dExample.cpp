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

#include "kvs/StructuredVolumeObject"

#include "Exporter/StructuredVolumeObjectExporter.h"
#include "FileFormat/PLOT3D/Plot3d.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/Pfl.h"
#include "PBVRFileInformation/UnstructuredPfi.h"

void Plot3d2Kvsml( const std::string& directory, const std::string& base, const std::string& xyz,
                   const std::string& q, const std::string& f )
{
    std::cout << "Reading " << xyz << " ..." << std::endl;

    cvt::Plot3d input_vtm( xyz, [=]( vtkMultiBlockPLOT3DReader* reader ) {
        reader->AutoDetectFormatOn();
        reader->SetQFileName( q.c_str() );
        reader->SetFunctionFileName( f.c_str() );
    } );

    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;

    int cell_type = 7;
    int last_time_step = 0;
    int time_step = 0;
    int sub_volume_id = 1;
    std::unordered_map<int, int> sub_volume_counts;

    // One-pass to collect information.
    if ( time_step == 0 )
    {
        std::cout << "Reading 0 step objects to collect information" << std::endl;
        std::unordered_map<int, int> veclens;

        for ( auto format : input_vtm.eachBlock() )
        {
            if ( auto input_vtu = dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) )
            {
                cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( input_vtu );

                sub_volume_counts[cell_type] = ( sub_volume_counts.count( cell_type ) == 0 )
                                                   ? 1
                                                   : ( sub_volume_counts[cell_type] + 1 );
                veclens[cell_type] =
                    ( veclens.count( cell_type ) == 0 ) ? 1 : ( veclens[cell_type] + 1 );
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
        if ( auto input_vts = dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) )
        {
            cvt::VtkImporter<cvt::VtkXmlStructuredGrid> importer( input_vts );
            kvs::StructuredVolumeObject* object = &importer;
            object->print( std::cout, kvs::Indent( 2 ) );

            std::cout << "  Writing to " << directory << " ..." << std::endl;
            auto local_base = std::string( base ) + "_" + std::to_string( cell_type );

            cvt::StructuredVolumeObjectExporter exporter( &importer );
            exporter.setWritingDataTypeToExternalBinary();
            exporter.write( directory, local_base, time_step, sub_volume_ids[cell_type],
                            sub_volume_counts[cell_type] );

            pfi_map.at( cell_type ).registerObject( &exporter, time_step, sub_volume_id );

            ++sub_volume_ids[cell_type];
        }
        else
        {
            std::cout << "An unsupported vtk data type" << std::endl;
        }
    }

    cvt::Pfl pfl;
    for ( auto& e : pfi_map )
    {
        std::string local_base = std::string( base ) + "_" + std::to_string( e.first );
        e.second.write( directory, local_base );
        e.second.print( std::cout );

        pfl.registerPfi( directory, local_base );
    }
    pfl.write( directory, base );
}