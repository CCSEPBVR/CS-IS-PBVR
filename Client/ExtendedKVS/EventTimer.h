/*****************************************************************************/
/**
 *  @file   EventTimer.h
 *  @author Keisuke Tajiri
 */
/*****************************************************************************/
#pragma once
#include <kvs/EventTimer>
#include <kvs/EventListener>
#include <kvs/qt/Qt>


namespace kvs
{

namespace qt
{

/*===========================================================================*/
/**
 *  @brief  EventTimer class.
 */
 /*===========================================================================*/
class EventTimer : public QObject, public kvs::EventTimer
{
public:
    EventTimer(kvs::EventListener* listener) : kvs::EventTimer(listener) {}
    virtual ~EventTimer() {}

public:
    void start(int msec);
    void stop();
    void nortify();

private:
    void timerEvent(QTimerEvent* e) { this->nortify(); }
};

} // end of namespace qt

} // end of namespace kvs
