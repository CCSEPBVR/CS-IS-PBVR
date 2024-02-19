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

private:

    static int                   m_argc;             ///< argument count
    static char**                m_argv;             ///< argument value

    /* 131018 removed
        static kvs::Light*           light;            ///< light
        static kvs::Mouse*           mouse;            ///< mouse
        static kvs::Background*      background;       ///< background
    */
    // Parameters for controlling the viewer.
    static ControlTarget          m_target;           ///< control target
    static pbvr::ObjectManager*   m_object_manager;   ///< object manager
    static kvs::RendererManager*  m_renderer_manager; ///< renderer manager
    static pbvr::IDManager*       m_id_manager;       ///< ID manager ( object_id, renderer_id )

public:
    // Basic components in the viewer.
    static kvs::Camera*           m_camera;           ///< camera

public:

    GlobalCore( const int count, const char** values );

    virtual ~GlobalCore();

public:

    static void createCore( const int count, const char** values );

    static void clearCore();

    static void resetCore();
};

} // end of namespace pbvr

#endif // KVS__GLOBAL_CORE_H_INCLUDE
