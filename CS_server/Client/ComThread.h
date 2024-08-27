#ifndef PBVR__KVS__VISCLIENT__COMTHREAD_H_INCLUDE
#define PBVR__KVS__VISCLIENT__COMTHREAD_H_INCLUDE

#include <kvs/Thread>
#include "Command.h"
#include "VisualizationParameter.h"

namespace kvs
{
namespace visclient
{

class ComThread : public kvs::Thread
{

public:
    enum ComStatus
    {
        Idle    = 0,
        Ready   = 1,
        Running = 2,
        Exit    = 3,
    };

private:
    Command* m_command;
    VisualizationParameter m_param;
    ReceivedMessage m_result;
    ComStatus m_status;

    kvs::Mutex m_mutex_param;
    kvs::Mutex m_mutex_result;
    kvs::Mutex m_mutex_status;

public:
    ComThread( Command* command );
    virtual ~ComThread();

    bool start();
    bool wait();
    void quit();
    void run();

    ComStatus getStatus();

private:
    void make_param();
    void emit_result();
    void set_status( const ComStatus& status );
};

}
}

#endif // PBVR__KVS__VISCLIENT__COMTHREAD_H_INCLUDE

