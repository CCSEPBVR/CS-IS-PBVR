#include "MPIRunner/MpiMainProcess.h"

#include <algorithm>
#include <cstring>
#include <set>

#include "mpi.h"
#include <vtkXMLDataElement.h>

#include "Converter/ConverterInputs.h"
#include "Converter/ConverterTaskInput.h"
#include "Converter/ConverterTaskOutput.h"
#include "MPIRunner/MpiTag.h"
#include "PBVRFileInformation/Pfl.h"
#include "PBVRFileInformation/UnstructuredPfi.h"

namespace
{
void SendInvalidPathLength( int rank, MPI_Comm comm )
{
    auto length = -1;
    MPI_Send( &length, 1, MPI_INT, rank, cvt::MPI_SERIALIZED_INPUT_LENGTH_TAG, comm );
}
} // namespace

cvt::MpiMainProcess::MpiMainProcess( int argc, char** argv )
{
    controller = vtkSmartPointer<vtkMPIController>::New();
    controller->Initialize( &argc, &argv, 1 );
    input_xml_path = *cvt::CheckConfigFile( argv[1] );
}

void cvt::MpiMainProcess::execute(
    std::function<std::optional<cvt::ConverterTaskOutput>( cvt::ConverterTaskInput )>&& f )
{
    auto paths = cvt::ListInputFilesFromConfigFile( input_xml_path.u8string().c_str() );
    auto current = paths.begin();
    auto path_end = paths.end();
    std::unordered_map<int, cvt::ConverterTaskInput> input_table;

    std::unordered_map<int, std::list<cvt::ConverterTaskOutput>> outputs;

    auto number_of_process = controller->GetNumberOfProcesses();
    auto my_process_id = controller->GetLocalProcessId();
    // A list of an active process rank
    std::set<int> running_rank;
    std::vector<int> target_indices( number_of_process );
    // An array to finish signals
    std::vector<int> serialized_output_length( number_of_process );
    // A MPI_Request list to MPI_Waitany
    std::vector<MPI_Request> request_list;
    request_list.reserve( number_of_process );
    // A MPI_Request list for finish signals from a sub process
    std::vector<MPI_Request> finished_request_list( number_of_process );

    int rank = 0;
    for ( ; rank < number_of_process && current != path_end; ++rank )
    {
        if ( rank == my_process_id )
        {
            continue;
        }
        auto serialized = current->serialize();
        target_indices[rank] = current->target_index;
        input_table.emplace( current->target_index, *current );
        kvsMessage( ( std::string( "Sub process " ) + std::to_string( rank ) +
                      " starts a convert task including " + current->source_file_paths[0] )
                        .c_str() );

        auto length = serialized.length() + 1;
        MPI_Send( &length, 1, MPI_INT, rank, cvt::MPI_SERIALIZED_INPUT_LENGTH_TAG, MPI_COMM_WORLD );
        MPI_Send( serialized.c_str(), length, MPI_CHAR, rank, cvt::MPI_SERIALIZED_INPUT_TAG,
                  MPI_COMM_WORLD );
        MPI_Irecv( &( serialized_output_length.data()[rank] ), 1, MPI_INT, rank,
                   cvt::MPI_SERIALIZED_OUTPUT_LENGTH_TAG, MPI_COMM_WORLD,
                   &( finished_request_list.data()[rank] ) );

        running_rank.insert( rank );
        ++current;
    }
    // Send -1 to finish sub processes because no task
    for ( ; rank < number_of_process; ++rank )
    {
        if ( rank == my_process_id )
        {
            continue;
        }
        ::SendInvalidPathLength( rank, MPI_COMM_WORLD );
    }

    std::string serialized_output;

    while ( !running_rank.empty() )
    {
        // Also, execute f in the main process
        if ( f && current != path_end )
        {
            input_table.emplace( current->target_index, *current );
            kvsMessage( ( std::string( "The main process " ) + " starts a convert task including " +
                          current->source_file_paths[0] )
                            .c_str() );
            auto output = f( *current );
            if ( output )
            {
                outputs[current->target_index].push_back( *output );
            }
            ++current;
            kvsMessage( std::string( "The main process ends a convert task" ).c_str() );
        }

        MPI_Status mpi_status;

        request_list.clear();
        std::transform( running_rank.begin(), running_rank.end(),
                        std::inserter( request_list, request_list.end() ),
                        [&]( int r ) { return finished_request_list[r]; } );
        int finished_index = -1;
        MPI_Waitany( running_rank.size(), request_list.data(), &finished_index, &mpi_status );
        rank = mpi_status.MPI_SOURCE;
        if ( serialized_output_length[rank] > 0 )
        {
            auto length = serialized_output_length[rank];
            serialized_output.resize( length - 1 );
            MPI_Recv( serialized_output.data(), length, MPI_CHAR, rank,
                      cvt::MPI_SERIALIZED_OUTPUT_TAG, MPI_COMM_WORLD, &mpi_status );

            cvt::ConverterTaskOutput output( target_indices[rank], 0 );
            output.deserialize( serialized_output );

            outputs[target_indices[rank]].push_back( output );
            kvsMessage(
                ( std::string( "Sub process " ) + std::to_string( rank ) + " ends a convert task" )
                    .c_str() );
        }
        else if ( serialized_output_length[rank] == 0 )
        {
            kvsMessage(
                ( std::string( "Sub process " ) + std::to_string( rank ) + " ends a convert task" )
                    .c_str() );
        }
        else
        {
            kvsMessageError( ( std::string( "Sub process " ) + std::to_string( rank ) +
                               " ends a convert task with some errors" )
                                 .c_str() );
        }

        if ( current != path_end )
        {
            auto serialized = current->serialize();
            target_indices[rank] = current->target_index;
            input_table.emplace( current->target_index, *current );
            kvsMessage( ( std::string( "Sub process " ) + std::to_string( rank ) +
                          " starts a convert task including " + current->source_file_paths[0] )
                            .c_str() );

            auto length = serialized.length() + 1;
            MPI_Send( &length, 1, MPI_INT, rank, cvt::MPI_SERIALIZED_INPUT_LENGTH_TAG,
                      MPI_COMM_WORLD );
            MPI_Send( serialized.c_str(), length, MPI_CHAR, rank, cvt::MPI_SERIALIZED_INPUT_TAG,
                      MPI_COMM_WORLD );
            MPI_Irecv( &( serialized_output_length.data()[rank] ), 1, MPI_INT, rank,
                       cvt::MPI_SERIALIZED_OUTPUT_LENGTH_TAG, MPI_COMM_WORLD,
                       &( finished_request_list.data()[rank] ) );
            ++current;
        }
        else
        {
            // Send -1 to finish sub processes because no task
            ::SendInvalidPathLength( rank, MPI_COMM_WORLD );
            running_rank.erase( rank );
        }
    }

    for ( auto& o : outputs )
    {
        int target_index = o.first;
        auto& output_list = o.second;

        if ( output_list.size() != 0 )
        {
            output_list.sort( []( auto& o0, auto& o1 ) { return o0.time_step < o1.time_step; } );
            std::unordered_map<std::string, cvt::UnstructuredPfi> pfi_map;
            int last_time_step = output_list.size() - 1;
            int number_of_components =
                std::get<0>( output_list.begin()->value_minmax.begin()->second ).size();
            for ( auto& k : output_list.begin()->sub_volume_counts )
            {
                pfi_map.emplace( k.first, cvt::UnstructuredPfi( number_of_components,
                                                                last_time_step, k.second ) );
            }
            for ( auto& out : output_list )
            {
                for ( auto& k : out.sub_volume_counts )
                {
                    pfi_map.at( k.first ).registerObject( &out, k.first );
                }
            }

            cvt::Pfl pfl;
            for ( auto& pfi : pfi_map )
            {
                pfi.second.write( input_table.at( target_index ).destination_directory, pfi.first );
                pfl.registerPfi( input_table.at( target_index ).destination_directory, pfi.first );
            }
            pfl.write( input_table.at( target_index ).destination_directory,
                       input_table.at( target_index ).destination_prefix );
            kvsMessage( ( std::string( "The main process outputs PFI and PFL to " ) +
                          input_table.at( target_index ).destination_directory )
                            .c_str() );
        }
    }
}
