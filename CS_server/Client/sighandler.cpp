#include <signal.h>
#include "sighandler.h"
#include <iostream>
#include <kvs/Mutex>

kvs::Mutex sigmx;
bool SigClient = false;

void InitializeSignalHandler()
{
    signal( SIGABRT, &SignalHandler );
    signal( SIGTERM, &SignalHandler );
    signal( SIGINT,  &SignalHandler );
//  signal(SIGSEGV, &SignalHandler);
}

void SignalHandler( const int sig )
{
    sigmx.lock();
    if ( !SigClient )
    {
        std::cout << "SIGNALED!!!  ";
        if ( sig == SIGABRT )
        {
            std::cout << "SIGABRT";
        }
        else if ( sig == SIGINT )
        {
            std::cout << "SIGINT";
        }
        else if ( sig == SIGTERM )
        {
            std::cout << "SIGTERM";
        }
        else if ( sig == SIGSEGV )
        {
            std::cout << "SIGSEGV";
        }
        std::cout << "(" << sig << ")" << std::endl;
    }
    SigClient = true;
    sigmx.unlock();
}

bool IsSignal()
{
    bool ret;
    sigmx.lock();
    ret = SigClient;
    sigmx.unlock();
    return ret;
}

