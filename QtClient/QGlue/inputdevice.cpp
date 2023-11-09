#include "inputdevice.h"
#include "OVR.h"
#include "call.h"

InputDevice::InputDevice(const HeadMountedDisplay &hmd) : m_hmd(hmd), m_types(0) {}

void InputDevice::initialize() { KVS_OVR_CALL(m_types = ovr_GetConnectedControllerTypes(m_hmd.handler())); }

bool InputDevice::hasRight() const { return (m_types & ovrControllerType_RTouch) == ovrControllerType_RTouch; }

bool InputDevice::hasLeft() const { return (m_types & ovrControllerType_LTouch) == ovrControllerType_LTouch; }

ovrTrackingState InputDevice::trackingState(const kvs::Int64 frame_index) const {
  const double frame_timing = m_hmd.frameTiming(frame_index);
  return m_hmd.trackingState(frame_timing);
}

ovrInputState InputDevice::inputState(const ovrControllerType type) const { return m_hmd.inputState(type); }
