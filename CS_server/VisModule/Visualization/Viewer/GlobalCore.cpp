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
#include <kvs/DebugNew>
#include <kvs/Camera>
#include <kvs/Background>
#include <kvs/Light>
#include <kvs/Mouse>
#include <kvs/ObjectManager>
#include <kvs/RendererManager>
#include <kvs/IDManager>
#include <kvs/Message>
#include <kvs/RGBColor>


namespace kvs
{

// Parameter instantiation.
int                       GlobalCore::argc;
char**                    GlobalCore::argv;
kvs::Camera*              GlobalCore::camera;
kvs::Light*               GlobalCore::light;
kvs::Mouse*               GlobalCore::mouse;
kvs::Background*          GlobalCore::background;
GlobalCore::ControlTarget GlobalCore::target;
kvs::ObjectManager*       GlobalCore::object_manager;
kvs::RendererManager*     GlobalCore::renderer_manager;
kvs::IDManager*           GlobalCore::id_manager;

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

    camera = new kvs::Camera();
    if( !camera )
    {
        kvsMessageError("Cannot allocate memory for the camera.");
        return;
    }

    light = new kvs::Light();
    if( !light )
    {
        kvsMessageError("Cannot allocate memory for the light.");
        return;
    }

    mouse = new kvs::Mouse();
    if( !mouse )
    {
        kvsMessageError("Cannot allocate memory for the mouse.");
        return;
    }

    background = new kvs::Background( kvs::RGBColor( 212, 221, 229 ) );
    if( !background )
    {
        kvsMessageError("Cannot allocate memory for the background.");
        return;
    }

    object_manager = new kvs::ObjectManager();
    if( !object_manager )
    {
        kvsMessageError("Cannot allocate memory for the object manager.");
        return;
    }

    renderer_manager = new kvs::RendererManager();
    if( !renderer_manager )
    {
        kvsMessageError("Cannot allocate memory for the renderer manager.");
        return;
    }

    id_manager = new kvs::IDManager();
    if( !id_manager )
    {
        kvsMessageError("Cannot allocate memory for the ID manager.");
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

} // end of namespace kvs
