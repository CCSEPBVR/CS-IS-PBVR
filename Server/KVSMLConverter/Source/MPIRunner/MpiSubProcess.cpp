#include "MpiSubProcess.h"

#include <optional>

#include "mpi.h"
#include <vtkMPIController.h>
#include <vtkSmartPointer.h>

#include "Converter/ConverterTaskInput.h"
#include "Converter/ConverterTaskOutput.h"
#include "MPIRunner/MpiTag.h"

cvt::MpiSubProcess::MpiSubProcess( int argc, char** argv, int main_process_id ):
    main_process_id( main_process_id )
{
    controller = vtkSmartPointer<vtkMPIController>::New();
    controller->Initialize( &argc, &argv, 1 );
}

void cvt::MpiSubProcess::execute(
    std::function<std::optional<cvt::ConverterTaskOutput>( cvt::ConverterTaskInput )>&& f )
{
    MPI_Status mpi_status;
    int length = 1;
    std::string serialized_data;

    do
    {
        // Receive string length + \0
        MPI_Recv( &length, 1, MPI_INT, main_process_id, cvt::MPI_SERIALIZED_INPUT_LENGTH_TAG,
                  MPI_COMM_WORLD, &mpi_status );

        if ( length > 0 )
        {
            serialized_data.resize( length - 1 );

            // Receive a XML string
            MPI_Recv( serialized_data.data(), length + 1, MPI_CHAR, main_process_id,
                      cvt::MPI_SERIALIZED_INPUT_TAG, MPI_COMM_WORLD, &mpi_status );

            cvt::ConverterTaskInput input;
            input.deserialize( serialized_data );
            auto output = f( input );

            if ( output )
            {
                if ( output->isEmpty() )
                {
                    int length = 0;
                    MPI_Send( &length, 1, MPI_INT, main_process_id,
                              cvt::MPI_SERIALIZED_OUTPUT_LENGTH_TAG, MPI_COMM_WORLD );
                }
                else
                {
                    serialized_data = output->serialize();
                    int length = serialized_data.length() + 1;
                    MPI_Send( &length, 1, MPI_INT, main_process_id,
                              cvt::MPI_SERIALIZED_OUTPUT_LENGTH_TAG, MPI_COMM_WORLD );
                    MPI_Send( serialized_data.c_str(), length, MPI_CHAR, main_process_id,
                              cvt::MPI_SERIALIZED_OUTPUT_TAG, MPI_COMM_WORLD );
                }
            }
            else
            {
                int length = -1;
                MPI_Send( &length, 1, MPI_INT, main_process_id,
                          cvt::MPI_SERIALIZED_OUTPUT_LENGTH_TAG, MPI_COMM_WORLD );
            }
        }
    } while ( length > 0 );
}