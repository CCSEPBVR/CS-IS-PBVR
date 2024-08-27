/****************************************************************************/
/**
 *  @file GlobalCore.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GlobalCore.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__GLOBAL_CORE_H_INCLUDE
#define PBVR__GLOBAL_CORE_H_INCLUDE

#include "ClassName.h"
#include <kvs/Camera>
/* 131018 removed
#include "Light.h"
#include "Mouse.h"
#include "Background.h"
*/
#include "ObjectManager.h"
#include <kvs/RendererManager>
#include "IDManager.h"


namespace pbvr
{

/*==========================================================================*/
/**
*  Global core class.
*/
/*==========================================================================*/
class GlobalCore
{
    kvsClassName( pbvr::GlobalCore );

public:

    enum ControlTarget
    {
        TargetObject = 0,
        TargetCamera,
//        TargetLight, /* 131018 removed */
        NumberOfTargets
    };

public:

    static int                   argc;             ///< argument count
    static char**                argv;             ///< argument value

    // Basic components in the viewer.
    static kvs::Camera*          camera;           ///< camera
    /* 131018 removed
        static kvs::Light*           light;            ///< light
        static kvs::Mouse*           mouse;            ///< mouse
        static kvs::Background*      background;       ///< background
    */
    // Parameters for controlling the viewer.
    static ControlTarget         target;           ///< control target
    static pbvr::ObjectManager*   object_manager;   ///< object manager
    static kvs::RendererManager* renderer_manager; ///< renderer manager
    static pbvr::IDManager*       id_manager;       ///< ID manager ( object_id, renderer_id )

public:

    GlobalCore( int count, char** values );

    virtual ~GlobalCore( void );

public:

    static void create_core( int count, char** values );

    static void clear_core( void );

    static void reset_core( void );
};

} // end of namespace pbvr

#endif // KVS__GLOBAL_CORE_H_INCLUDE
