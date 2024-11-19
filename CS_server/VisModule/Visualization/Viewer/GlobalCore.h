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
#include <vismodule/Light>
#include <vismodule/Mouse>
#include <vismodule/Background>
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
        TargetLight,
        NumberOfTargets
    };

public:

    static int                   argc;             ///< argument count
    static char**                argv;             ///< argument value

    // Basic components in the viewer.
    static vismodule::Camera*          camera;           ///< camera
    static vismodule::Light*           light;            ///< light
    static vismodule::Mouse*           mouse;            ///< mouse
    static vismodule::Background*      background;       ///< background

    // Parameters for controlling the viewer.
    static ControlTarget         target;           ///< control target
    static vismodule::ObjectManager*   object_manager;   ///< object manager
    static vismodule::RendererManager* renderer_manager; ///< renderer manager
    static vismodule::IDManager*       id_manager;       ///< ID manager ( object_id, renderer_id )

public:

    GlobalCore( int count, char** values );

    virtual ~GlobalCore( void );

public:

    static void create_core( int count, char** values );

    static void clear_core( void );

    static void reset_core( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__GLOBAL_CORE_H_INCLUDE
