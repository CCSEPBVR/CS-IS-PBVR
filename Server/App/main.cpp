#include "Server.h"
int main(int argc, char *argv[])
{
    int port = 60000;

    if( argc >= 2 )
    {
        port = std::atoi(argv[1]);
    }

    Server server( port );
}
