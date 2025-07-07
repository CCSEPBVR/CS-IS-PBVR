/*****************************************************************************/
/**
 *  @file   EventTimer.cpp
 *  @author Keisuke Tajiri
 */
/*****************************************************************************/
#pragma once
#include "EventTimer.h"


namespace kvs
{

namespace qt
{

/*===========================================================================*/
/**
*  @brief
*/
/*===========================================================================*/
void EventTimer::start(int msec)
{
    if (msec < 0) { return; }
    if (this->isStopped())
    {
        this->setInterval(msec);
        this->setStopped(false);
        this->setID(QObject::startTimer(msec));
    }
}

/*===========================================================================*/
/**
*  @brief
*/
/*===========================================================================*/
void EventTimer::stop()
{
    if (!this->isStopped())
    {
        this->setStopped(true);
        QObject::killTimer(this->id());
    }
}

/*===========================================================================*/
/**
*  @brief
*/
/*===========================================================================*/
void EventTimer::nortify()
{
    if (!this->isStopped())
    {
        if (this->eventListener())
        {
            this->eventListener()->onEvent(this->timeEvent());
        }
    }
}

} // end of namespace qt

} // end of namespace kvs
