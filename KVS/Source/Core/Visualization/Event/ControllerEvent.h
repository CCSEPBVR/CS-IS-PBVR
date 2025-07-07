/****************************************************************************/
/**
 *  @file   ControllerEvent.h
 *  @author Keisuke Tajiri
 */
/****************************************************************************/
#pragma once
#include <kvs/EventBase>
#include <kvs/Controller>
#include <kvs/Xform>
#include <kvs/Side>


namespace kvs
{

/*==========================================================================*/
/**
 *  Controller event class.
 */
/*==========================================================================*/
class ControllerEvent : public kvs::EventBase
{
private:
    kvs::Controller::ControllerStatus m_controller_status = kvs::Controller::ControllerStatus();
    int m_action = kvs::Controller::NoAction;

public:
    ControllerEvent() = default;
    virtual ~ControllerEvent() = default;

    const kvs::Controller::ControllerStatus& controllerStatus() const { return m_controller_status; }
    int action() const { return m_action; }

    void setControllerStatus( const kvs::Controller::ControllerStatus& cs ) { m_controller_status = kvs::Controller::ControllerStatus(cs); }
    void setAction( int action ) { m_action = action; }

    int type() const
    {
        return
            m_action == kvs::Controller::Pressed ? EventBase::ControllerPressEvent :
            m_action == kvs::Controller::Moved ? EventBase::ControllerMoveEvent :
            m_action == kvs::Controller::Released ? EventBase::ControllerReleaseEvent :
            m_action == kvs::Controller::AxisChanged ? EventBase::ControllerAxisEvent : 0;
    }
};

} // end of namespace kvs
