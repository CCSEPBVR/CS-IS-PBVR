#pragma once
// #include <kvs/EventListener>
// #include "headmounteddisplay.h"
// #include <kvs/Scene>
#include "inputdevice.h"

class HeadMountedDisplay;

namespace kvs {
namespace jaea {
namespace pbvr {
class Scene;
} // end of namespace pbvr
} // end of namespace jaea
} // end of namespace kvs

class ControllerBase //: public kvs::EventListener
{
private:
  InputDevice m_input_device; ///< input device
  kvs::jaea::pbvr::Scene *m_scene;

public:
  //    ControllerBase( Screen* screen );
  ControllerBase(HeadMountedDisplay &hmd, kvs::jaea::pbvr::Scene *scene);
  virtual void initializeEvent() {}
  virtual void frameEvent() {}

  kvs::jaea::pbvr::Scene *scene() { return m_scene; }

protected:
  InputDevice &inputDevice() { return m_input_device; }

private:
  //    void onEvent( kvs::EventBase* event );
};
