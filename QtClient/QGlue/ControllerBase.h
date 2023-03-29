#pragma once
//#include <kvs/EventListener>
//#include <kvs/Scene>

//#include "headmounteddisplay.h"
//#include "Scene.h"
#include "InputDevice.h"

namespace kvs
{
namespace oculus
{
namespace jaea
{

class HeadMountedDisplay;
class Scene;
//class InputDevice;

class ControllerBase //: public kvs::EventListener
{
private:
    //    kvs::oculus::jaea::InputDevice m_input_device; ///< input device
    //    kvs::oculus::jaea::Scene *m_scene;
    InputDevice m_input_device; ///< input device
    Scene *m_scene;

public:
    ControllerBase( kvs::oculus::jaea::HeadMountedDisplay& hmd, kvs::oculus::jaea::Scene *scene );
//    ControllerBase( kvs::oculus::jaea::HeadMountedDisplay& hmd, kvs::oculus::jaea::Scene *scene ):
//        m_input_device( hmd ),
//        m_scene(scene)
//    {
//    }

    virtual void initializeEvent() {}
    virtual void frameEvent() {}

    kvs::oculus::jaea::Scene* scene() {return m_scene;}

protected:
    kvs::oculus::jaea::InputDevice& inputDevice() { return m_input_device; }

private:
//    void onEvent( kvs::EventBase* event );
};

} // end of namespace jaea

} // end of namespace oculus

} // end of namespace kvs
