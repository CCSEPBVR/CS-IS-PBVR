#ifndef PBVR__KVS__VISCLIENT__TIMER_EVENT_H_INCLUDE
#define PBVR__KVS__VISCLIENT__TIMER_EVENT_H_INCLUDE

#include "kvs/TimerEventListener"
#include "kvs/PointObject"
#include "kvs/ParticleVolumeRenderer"
#include "Command.h"
#include "ComThread.h"

namespace kvs
{
namespace visclient
{
class Command;

class TimerEvent : public kvs::TimerEventListener
{
private:
    Command* m_command;
    ComThread* m_comthread;
#ifdef CPUMODE
    int m_object_id;
    kvs::PointObject* m_front_object;
    kvs::PointObject* m_back_object;
    kvs::ParticleVolumeRenderer* m_renderer;
#endif
    int m_interpolation_counter;
    int m_ninterpolation;
    int m_is_key_frame_animation;
    int m_is_ready;
    int m_xform_index;
    std::vector<kvs::Xform>* m_xforms;
    std::vector<int>* m_time_steps;

public:
    TimerEvent( Command* command, ComThread* comthread );
    void update( kvs::TimeEvent* event );
#ifdef CPUMODE
    ~TimerEvent();
    void setObject( const int& id );
    void setRenderer( kvs::ParticleVolumeRenderer* renderer );
    kvs::PointObject* getPointObject()
    {
        return m_front_object;
    }
#endif
    void enableKeyFrameAnimation()
    {
        m_is_key_frame_animation = 1;
        m_command->m_is_key_frame_animation = true;
    }
    void disableKeyFrameAnimation()
    {
        m_xform_index = 0;
        m_interpolation_counter = 0;
        m_is_key_frame_animation = 0;
        m_command->m_is_key_frame_animation = false;
    }
    void toggleKeyFrameAnimation()
    {
        m_is_key_frame_animation = ( m_is_key_frame_animation ) ? 0 : 1;
        if ( m_is_key_frame_animation )
        {
            m_command->m_is_key_frame_animation = true;
        }
        else
        {
            m_command->m_is_key_frame_animation = false;
        }
    }
    int getTimeStep()
    {
        return m_command->m_parameter.m_time_step;
    }
};

}
}

#endif    // PBVR__KVS__VISCLIENT__TIMER_EVENT_H_INCLUDE

