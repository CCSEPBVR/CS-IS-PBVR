
#include "ComThread.h"

kvs::visclient::ComThread::ComThread( Command* command ) :
    m_command( command ),
    m_status( Idle )
{
}

kvs::visclient::ComThread::~ComThread()
{
}

bool kvs::visclient::ComThread::start()
{
    bool ret;
    make_param();
    ret = kvs::Thread::start();

    // >> modify by @hira at 2017/05/22
    // if ( ret ) set_status( Running );
    ComStatus stat = Idle;
    while(!(stat == Running || stat == Exit)) {
        stat = getStatus();
    }
    // << modify by @hira at 2017/05/22

    return ret;
}

bool kvs::visclient::ComThread::wait()
{
    bool ret = kvs::Thread::wait();
    if ( ret )
    {
        set_status( Idle );
        emit_result();
    }
    return ret;
}

void kvs::visclient::ComThread::quit()
{
    kvs::Thread::quit();
    set_status( Idle );
}

void kvs::visclient::ComThread::run()
{
    set_status( Running );

    m_mutex_param.lock();
    m_mutex_result.lock();

    m_command->update( &m_param, &m_result );

    m_mutex_result.unlock();
    m_mutex_param.unlock();

    set_status( Exit );
}

void kvs::visclient::ComThread::make_param()
{
    m_mutex_param.lock();
    m_param = m_command->m_parameter;
    m_mutex_param.unlock();

    set_status( Ready );
}

void kvs::visclient::ComThread::emit_result()
{
    m_mutex_result.lock();
    m_command->m_result = m_result;
    m_mutex_result.unlock();

    set_status( Ready );
}

void kvs::visclient::ComThread::set_status( const ComStatus& status )
{
    m_mutex_status.lock();
    m_status = status;
    m_mutex_status.unlock();
}

kvs::visclient::ComThread::ComStatus kvs::visclient::ComThread::getStatus()
{
    ComStatus ret;
    m_mutex_status.lock();
    ret = m_status;
    m_mutex_status.unlock();
    return ret;
}

