#pragma once
#include "headmounteddisplay.h"

class InputDevice {
private:
  const HeadMountedDisplay &m_hmd; ///< head mounted display (hmd)
  unsigned int m_types;            ///< controller types connected to the hmd

public:
  InputDevice(const HeadMountedDisplay &hmd);

  void initialize();
  bool hasRight() const;
  bool hasLeft() const;
  ovrTrackingState trackingState(const kvs::Int64 frame_index = 0) const;
  ovrInputState inputState(const ovrControllerType type) const;
};
