#ifndef PBVR__KVS__VISCLIENT__KEY_PRESS_EVENT_H_INCLUDE
#define PBVR__KVS__VISCLIENT__KEY_PRESS_EVENT_H_INCLUDE

#include "kvs/Message"
#include "kvs/Date"
#include "kvs/Time"
#include "kvs/KeyEvent"
#include "kvs/KeyPressEventListener"
#include "KeyFrameAnimation.h"

namespace kvs
{
namespace visclient
{

class KeyPressEvent : public kvs::KeyPressEventListener
{

public:
    KeyPressEvent()
    {
    }

    void update( kvs::KeyEvent* event )
    {
        switch ( event->key() )
        {
        case kvs::Key::l:
            screen()->controlTarget() = kvs::ScreenBase::TargetLight;
            break;
        case kvs::Key::c:
            screen()->controlTarget() = kvs::ScreenBase::TargetCamera;
            break;
        case kvs::Key::o:
            screen()->controlTarget() = kvs::ScreenBase::TargetObject;
            break;

        case kvs::Key::x:
            /* printf(" [debug] 'x' pressed. (add Xform)\n"); */
            KeyFrameAnimationAdd();
            break;
        case kvs::Key::d:
            /* printf(" [debug] 'd' pressed. (delete last Xform)\n"); */
            KeyFrameAnimationErase();
            break;
        case kvs::Key::D:
            /* printf(" [debug] 'e' pressed. (delete all Xform)\n"); */
            KeyFrameAnimationDelete();
            break;
        case kvs::Key::M:
            /* printf(" [debug] 'M' pressed. (toggle animation with Xform)\n"); */
            /* KeyFrameAnimationStart(); */
            KeyFrameAnimationToggle();
            break;
        /* case kvs::Key::m: */
        /*    printf(" [debug] 'm' pressed. (stop animation with Xform)\n"); */
        /*    KeyFrameAnimationStop(); */
        /*    break; */
        case kvs::Key::S:
            /* printf(" [debug] 'S' pressed. (write Xform to file)\n"); */
            KeyFrameAnimationWrite();
            break;
        case kvs::Key::F:
            /* printf(" [debug] 'F' pressed. (read Xform from file)\n"); */
            KeyFrameAnimationRead();
            break;

        default:
            break;
        }
    }
};
}
}

#endif    // PBVR__KVS__VISCLIENT__KEY_PRESS_EVENT_H_INCLUDE
