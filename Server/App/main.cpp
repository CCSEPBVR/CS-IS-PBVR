// #include "Server.h"
#include "ServerWIP.h"
int main( int argc, char *argv[] )
{
    int port = 60000; // デフォルトポート番号

    if( argc >= 2 )
    {
        port = std::atoi( argv[1] );
    }

    ServerWIP server( port );
}
