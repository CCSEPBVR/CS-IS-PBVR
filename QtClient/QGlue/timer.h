#ifndef QGLUE_TIMER_H_INCLUDE
#define QGLUE_TIMER_H_INCLUDE

#include <QTimer>

#include <kvs/EventListener>
#include <kvs/EventHandler>
//#include <kvs/TimeEvent>
#include "Client/TimerEvent.h"
#include <kvs/ClassName>
#include "kvs/PointObject"

#include "Client/ComThread.h"
#include "extCommand.h"

class RenderArea;
using namespace kvs::visclient;
namespace QGlue
{


/*===========================================================================*/
/**
 *  @brief  Timer class.
 */
/*===========================================================================*/
class Timer : public QTimer
{


protected:

    int                 m_id;             ///< timer ID
    int                 m_interval;       ///< interval time in millisecond
    bool                m_is_stopped;     ///< checl flag whether the time is stopped
    kvs::TimeEvent*     m_time_event;     ///< time event
    kvs::visclient::TimerEvent* m_event_listener; ///< event listener
    kvs::EventHandler*  m_event_handler;  ///< event handler
private:
    Command* m_command;
    ComThread* m_comthread;
    bool oneShot=true;

//    kvs::visclient::PBRProxy* m_renderer;

    int m_interpolation_counter;
    int m_ninterpolation;
    int m_is_key_frame_animation;
    int m_is_ready;
    int m_xform_index;
    std::vector<kvs::Xform>* m_xforms;
    std::vector<int>* m_time_steps;
    ::RenderArea* m_screen;

public:

    Timer( int msec = 0 );
	
    Timer(int msec, ExtCommand* command);

    Timer( kvs::EventListener* listener, int msec = 0 );

    Timer( kvs::EventHandler* handler, int msec = 0 );

    virtual ~Timer( void );

    void startSingleShot(){
//        if (!this) return;
//        if(m_is_stopped){
//            oneShot=true;
//            m_is_stopped = false;
//            extCommand->m_is_under_communication = true;
//            QTimer::start(1);
//        }
//        else{
//            qInfo("SingleShot canceled");
//        }
    }
    void start( int msec );

    void start( void );

    void stop( void );

    void onTimerUpdate( );
    void setInterval( int msec );

    void setEventListener(kvs::visclient::TimerEvent *listener );

    void setEventHandler( kvs::EventHandler* handler );

    const bool isStopped( void ) const;

    const int interval( void ) const;

//    void setRenderer( kvs::visclient::ExtendedParticleVolumeRenderer* renderer )
//    {
//        m_renderer = renderer;
//    }
protected:

    virtual void timerEvent( QTimerEvent* event );
    void animate();

    void comThreadIdleEvent();

    void comThreadExitEvent();
};

} // end of namespace QGLUE

#endif // QGLUE_TIMER_H_INCLUDE
