#include <cstdlib>
#include <iostream>
#include <mpi.h>

#include <vtkXMLDataElement.h>

#include "Converter/ConverterInputs.h"
#include "Converter/ConverterTask.h"
#include "MPIRunner/MpiMainProcess.h"
#include "MPIRunner/MpiSubProcess.h"

int main( int argc, char* argv[] )
{
    MPI_Init( &argc, &argv );
    int my_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );

    std::optional<cvt::filesystem::path> input;
    int code = EXIT_SUCCESS;

    int main_process_id = 0;

    if ( argc >= 2 && ( input = cvt::CheckConfigFile( argv[1] ) ) )
    {
        if ( my_rank == main_process_id )
        {
            cvt::MpiMainProcess main_process( argc, argv );
            main_process.execute();
        }
        else
        {
            cvt::MpiSubProcess sub_process( argc, argv, main_process_id );
            sub_process.execute( cvt::Convert );
        }
    }
    else
    {
        if ( my_rank == main_process_id )
        {
            std::cout << "A file converter to a KVSML file" << std::endl;
            std::cout << "USAGE" << std::endl;
            std::cout << "  mpirun -np N kvsml-converter-mpi <XML_PATH>" << std::endl;
        }
        code = EXIT_FAILURE;
    }

    MPI_Finalize();
    return code;
}