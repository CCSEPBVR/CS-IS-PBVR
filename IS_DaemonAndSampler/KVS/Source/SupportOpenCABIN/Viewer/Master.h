/*****************************************************************************/
/**
 *  @file   Master.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#ifndef KVS__OPENCABIN__MASTER_H_INCLUDE
#define KVS__OPENCABIN__MASTER_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/EventHandler>
#include <kvs/InitializeEventListener>
#include <kvs/IdleEventListener>


namespace kvs
{

namespace opencabin
{

/*===========================================================================*/
/**
 *  @brief  Master class.
 */
/*===========================================================================*/
class Master
{
    kvsClassName( kvs::opencabin::Master );

protected:

    kvs::EventHandler* m_initialize_event_handler; ///< initialize event handler
    kvs::EventHandler* m_idle_event_handler; ///< idle event handler

public:

    Master( void );

    virtual ~Master( void );

public:

    virtual void initializeEvent( void );

    virtual void idleEvent( void );

public:

    void addInitializeEvent( kvs::InitializeEventListener* event );

    void addIdleEvent( kvs::IdleEventListener* event );

public:

    void initializeTrackpad( void );

    void updateTrackpad( void );
};

} // end of namespace opencabin

} // end of namespace kvs

#endif // KVS__OPENCABIN__MASTER_H_INCLUDE
