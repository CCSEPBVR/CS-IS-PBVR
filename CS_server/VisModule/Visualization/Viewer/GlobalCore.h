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
#ifndef VIS_MODULE__GLOBAL_CORE_H_INCLUDE
#define VIS_MODULE__GLOBAL_CORE_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Camera>
/* 131018 removed
#include "Light.h"
#include "Mouse.h"
#include "Background.h"
*/
#include <vismodule/ObjectManager>
#include <vismodule/RendererManager>
#include <vismodule/IDManager>


namespace vismodule
{

/*==========================================================================*/
/**
*  Global core class.
*/
/*==========================================================================*/
class GlobalCore
{
    visModuleClassName( vismodule::GlobalCore );

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
        static vismodule::Light*           light;            ///< light
        static vismodule::Mouse*           mouse;            ///< mouse
        static vismodule::Background*      background;       ///< background
    */
    // Parameters for controlling the viewer.
    static vismodule::ObjectManager*   m_object_manager;   ///< object manager
    static vismodule::RendererManager*  m_renderer_manager; ///< renderer manager
    static vismodule::IDManager*       m_id_manager;       ///< ID manager ( object_id, renderer_id )

public:
    // Basic components in the viewer.
    static ControlTarget          m_target;           ///< control target
    static vismodule::Camera*           m_camera;           ///< camera
public:

    GlobalCore( const int count, const char** values );

    virtual ~GlobalCore();

public:

    static void createCore( const int count, const char** values );

    static void clearCore();

    static void resetCore();
};

} // end of namespace vismodule

#endif // VIS_MODULE__GLOBAL_CORE_H_INCLUDE
