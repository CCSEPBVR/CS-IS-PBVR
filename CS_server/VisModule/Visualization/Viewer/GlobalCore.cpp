/****************************************************************************/
/**
 *  @file GlobalCore.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GlobalCore.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "GlobalCore.h"
#include <vismodule/DebugNew>
#include <vismodule/Camera>
#include <vismodule/Background>
#include <vismodule/Light>
#include <vismodule/Mouse>
#include <vismodule/ObjectManager>
#include <vismodule/RendererManager>
#include <vismodule/IDManager>
#include <vismodule/Message>
#include <vismodule/RGBColor>


namespace vismodule
{

// Parameter instantiation.
int                       GlobalCore::argc;
char**                    GlobalCore::argv;
vismodule::Camera*              GlobalCore::camera;
vismodule::Light*               GlobalCore::light;
vismodule::Mouse*               GlobalCore::mouse;
vismodule::Background*          GlobalCore::background;
GlobalCore::ControlTarget GlobalCore::target;
vismodule::ObjectManager*       GlobalCore::object_manager;
vismodule::RendererManager*     GlobalCore::renderer_manager;
vismodule::IDManager*           GlobalCore::id_manager;

/*==========================================================================*/
/**
 *  Consturctor.
 *  @param count [in] argument count
 *  @param values [in] argument values
 */
/*==========================================================================*/
GlobalCore::GlobalCore( int count, char** values )
{
    create_core( count, values );
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
GlobalCore::~GlobalCore( void )
{
    clear_core();
}

/*==========================================================================*/
/**
 *  Create the global core class.
 *  @param count [in] argument count
 *  @param values [in] argument values
 */
/*==========================================================================*/
void GlobalCore::create_core( int count, char** values )
{
    argc   = count;
    argv   = values;
    target = TargetObject;

    camera = new vismodule::Camera();
    if( !camera )
    {
        visModuleMessageError("Cannot allocate memory for the camera.");
        return;
    }

    light = new vismodule::Light();
    if( !light )
    {
        visModuleMessageError("Cannot allocate memory for the light.");
        return;
    }

    mouse = new vismodule::Mouse();
    if( !mouse )
    {
        visModuleMessageError("Cannot allocate memory for the mouse.");
        return;
    }

    background = new vismodule::Background( vismodule::RGBColor( 212, 221, 229 ) );
    if( !background )
    {
        visModuleMessageError("Cannot allocate memory for the background.");
        return;
    }

    object_manager = new vismodule::ObjectManager();
    if( !object_manager )
    {
        visModuleMessageError("Cannot allocate memory for the object manager.");
        return;
    }

    renderer_manager = new vismodule::RendererManager();
    if( !renderer_manager )
    {
        visModuleMessageError("Cannot allocate memory for the renderer manager.");
        return;
    }

    id_manager = new vismodule::IDManager();
    if( !id_manager )
    {
        visModuleMessageError("Cannot allocate memory for the ID manager.");
        return;
    }
}

/*==========================================================================*/
/**
 *  Clear the global core class.
 */
/*==========================================================================*/
void GlobalCore::clear_core( void )
{
    if( camera           ){ delete camera;           camera           = NULL; }
    if( light            ){ delete light;            light            = NULL; }
    if( mouse            ){ delete mouse;            mouse            = NULL; }
    if( background       ){ delete background;       background       = NULL; }
    if( object_manager   ){ delete object_manager;   object_manager   = NULL; }
    if( renderer_manager ){ delete renderer_manager; renderer_manager = NULL; }
    if( id_manager       ){ delete id_manager;       id_manager       = NULL; }
}

/*==========================================================================*/
/**
 *  Reset the global core class.
 */
/*==========================================================================*/
void GlobalCore::reset_core( void )
{
    // Reset viewpoint to the initial position.
    mouse->reset();

    // Reset the xform of the object.
    if( object_manager->hasActiveObject() )
    {
        object_manager->resetActiveObjectXform();
    }
    else
    {
        object_manager->resetXform();
    }

    // Reset the xform of the camera.
    camera->resetXform();

    // Reset the xform of the light.
    light->resetXform();
}

} // end of namespace vismodule
