#pragma once
#include "HeadMountedDisplay.h"


namespace kvs
{

namespace oculus
{
namespace jaea
{

class InputDevice
{
private:
    const kvs::oculus::jaea::HeadMountedDisplay& m_hmd; ///< head mounted display (hmd)
    unsigned int m_types; ///< controller types connected to the hmd

public:
    InputDevice( const kvs::oculus::jaea::HeadMountedDisplay& hmd );

    void initialize();
    bool hasRight() const;
    bool hasLeft() const;
    ovrTrackingState trackingState( const kvs::Int64 frame_index = 0 ) const;
    ovrInputState inputState( const ovrControllerType type ) const;
};

} // end of namespace jaea

} // end of namespace oculus

} // end of namespace kvs
