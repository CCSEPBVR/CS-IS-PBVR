#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "ControllerBase.h"

// ControllerBase::ControllerBase( Screen* screen ):
//     m_input_device( screen->headMountedDisplay() )
//{
//     setScreen( screen );
//     setScene( screen->scene() );
//     //setEventType( kvs::EventBase::InitializeEvent | kvs::EventBase::PaintEvent );
//     setEventType( kvs::EventBase::InitializeEvent | kvs::EventBase::FrameEvent );
//     screen->addEvent( this );
// }
ControllerBase::ControllerBase(HeadMountedDisplay &hmd, kvs::jaea::pbvr::Scene *scene) : m_input_device(hmd), m_scene(scene) {}

// void ControllerBase::onEvent( kvs::EventBase* event )
//{
//     switch ( event->type() )
//     {
//     case kvs::EventBase::InitializeEvent:
//     {
//         m_input_device.initialize();
//         this->initializeEvent();
//         break;
//     }
//     //case kvs::EventBase::PaintEvent:
//     case kvs::EventBase::FrameEvent:
//     {
//         this->frameEvent();
//         break;
//     }
//     default: break;
//     }
// }
