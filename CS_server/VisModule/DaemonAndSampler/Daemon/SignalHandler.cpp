#include "SignalHandler.h"
void SignalHandler( const int sig )
{
    SigServer = true;
}

