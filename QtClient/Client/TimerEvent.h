#ifndef PBVR__KVS__VISCLIENT__TIMER_EVENT_H_INCLUDE
#define PBVR__KVS__VISCLIENT__TIMER_EVENT_H_INCLUDE

#include "kvs/TimerEventListener"
#include "kvs/PointObject"

#include "Command.h"
#include "ComThread.h"


namespace kvs
{
namespace visclient
{
class Command;

class TimerEvent //: public kvs::TimerEventListener
{
private:
    Command* m_command;
    ComThread* m_comthread;

    RenderArea* m_screen;
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
    void setScreen(RenderArea* screen){m_screen=screen;}

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
        if (m_is_key_frame_animation){
            disableKeyFrameAnimation();
        }
        else{
            enableKeyFrameAnimation();
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

