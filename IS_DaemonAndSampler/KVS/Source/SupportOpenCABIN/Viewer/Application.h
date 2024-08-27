/*****************************************************************************/
/**
 *  @file   Application.h
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
#ifndef KVS__OPENCABIN__APPLICATION_H_INCLUDE
#define KVS__OPENCABIN__APPLICATION_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/ApplicationBase>
#include <kvs/opencabin/Master>

/* In order to create the execution file (shared object file) for the OpenCABIN,
 * the name of the main function in the user program coded by using KVS rename
 * to 'kvsOpenCABINMainLoop'.
 */
#if !defined( KVS_DISABLE_OPENCABIN_MAIN_LOOP )
#define main kvsOpenCABINMainLoop
#endif


namespace kvs
{

namespace opencabin
{

class Screen;
class Trackpad;

/*===========================================================================*/
/**
 *  @brief  Application class for OpenCABIN.
 */
/*===========================================================================*/
class Application : public kvs::ApplicationBase
{
    friend class kvs::opencabin::Master;
    friend class kvs::opencabin::Screen;
    friend class kvs::opencabin::Trackpad;

    typedef kvs::ApplicationBase BaseClass;

    kvsClassName( kvs::opencabin::Application );

protected:

    kvs::opencabin::Master m_master;
    kvs::opencabin::Trackpad* m_trackpad;

public:

    Application( int argc, char** argv );

    virtual ~Application( void );

public:

    kvs::opencabin::Master& master( void );

    const kvs::opencabin::Master& master( void ) const;

public:

    virtual int run( void );

    virtual void quit( void );

private:

    void attach_trackpad( kvs::opencabin::Trackpad* trackpad );

private:

    static void InitializeTrackpadForMaster( void );

    static void InitializeTrackpadForRenderer( void );

    static float* GetTrackpadScaling( void );

    static float* GetTrackpadTranslation( void );

    static float* GetTrackpadRotation( void );

    static void UpdateTrackpad( void );

    static void ReferenceTrackpad( void );

public:

    static const bool HasTrackpad( void );

    static const std::string Name( void );

    static const bool IsDone( void );

    static const bool IsMaster( void );

    static const bool IsRenderer( void );

    static const bool IsEnabledBarrier( void );

    static void EnableBarrier( void );

    static void DisableBarrier( void );

    static void SetAsMaster( void );

    static void SetAsRenderer( void );
};

} // end of namespace opencabin

} // end of namespace kvs

#endif // KVS__OPENCABIN__APPLICATION_H_INCLUDE
