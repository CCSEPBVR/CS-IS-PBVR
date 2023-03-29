#include "ControllerBase.h"
//#include "Scene.h"

namespace kvs
{
namespace oculus
{
namespace jaea
{

//ControllerBase::ControllerBase( Screen* screen ):
//    m_input_device( screen->headMountedDisplay() )
//{
//    setScreen( screen );
//    setScene( screen->scene() );
//    //setEventType( kvs::EventBase::InitializeEvent | kvs::EventBase::PaintEvent );
//    setEventType( kvs::EventBase::InitializeEvent | kvs::EventBase::FrameEvent );
//    screen->addEvent( this );
//}
ControllerBase::ControllerBase( kvs::oculus::jaea::HeadMountedDisplay& hmd, kvs::oculus::jaea::Scene *scene ):
    m_input_device( hmd ),
    m_scene(scene)
{
}

//void ControllerBase::onEvent( kvs::EventBase* event )
//{
//    switch ( event->type() )
//    {
//    case kvs::EventBase::InitializeEvent:
//    {
//        m_input_device.initialize();
//        this->initializeEvent();
//        break;
//    }
//    //case kvs::EventBase::PaintEvent:
//    case kvs::EventBase::FrameEvent:
//    {
//        this->frameEvent();
//        break;
//    }
//    default: break;
//    }
//}

} // end of namespace jaea

} // end of namespace oculus

} // end of namespace kvs
