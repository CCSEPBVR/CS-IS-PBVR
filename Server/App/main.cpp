#include "Server.h"
#include "ServerWorker.h"
// #include "ServerWIP.h"

#ifndef CPU_VER
#include "mpi.h"
#endif

int main( int argc, char *argv[] )
{
    int port = 60000; // デフォルトポート番号

    if( argc >= 2 )
    {
        port = std::atoi( argv[1] );
    }

#ifndef CPU_VER
    MPI_Init( &argc, &argv );
#endif

#ifndef CPU_VER
    int rank;
    int mpi_size;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    int rank = 0;
	int mpi_size = 1;
#endif

    if ( rank == 0 )
    {
        Server server( port );
    }
    else
    {
        ServerWorker server_worker;
        server_worker.Run();
    }

#ifndef CPU_VER
    MPI_Finalize();
#endif

    return 0;
}
