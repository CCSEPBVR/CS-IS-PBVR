
#include "ComThread.h"
#include <QMutex>
extern QMutex paint_mutex;

kvs::visclient::ComThread::ComThread( Command* command ) :
    m_command( command ),
    m_status( Idle )
{
}

kvs::visclient::ComThread::~ComThread()
{
#ifdef IS_MODE
    kvs::Thread::m_handler=0x0;
    this->quit();
#endif
}

bool kvs::visclient::ComThread::start()
{
    bool ret;
    make_param();
    ret = kvs::Thread::start();

#ifdef IS_MODE
     if ( ret ) set_status( Running );
#else
    // >> modify by @hira at 2017/05/22
    // if ( ret ) set_status( Running );
    ComStatus stat = Idle;
    while(!(stat == Running || stat == Exit)) {
        stat = getStatus();
    }
    // << modify by @hira at 2017/05/22
#endif

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
    qInfo ("**** kvs::visclient::ComThread::run starts ****");
    set_status( Running );

    m_mutex_param.lock();
    m_mutex_result.lock();
//    paint_mutex.lock();
    m_command->update( &m_param, &m_result );
//    paint_mutex.unlock();
    m_mutex_result.unlock();
    m_mutex_param.unlock();

    set_status( Exit );
    qInfo ("**** kvs::visclient::ComThread::run ends ###\n");
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
//     qInfo ("****  kvs::visclient::ComThread::emit_result BEGIN###\n");
    m_mutex_result.lock();
    m_command->m_result = m_result;
    m_mutex_result.unlock();
//     qInfo ("****  kvs::visclient::ComThread::emit_result DONE ###\n");
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

