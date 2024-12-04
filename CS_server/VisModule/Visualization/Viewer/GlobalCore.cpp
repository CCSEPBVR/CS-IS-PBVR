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
/* 131018 removed
#include "Light.h"
#include "Background.h"
#include "Mouse.h"
*/
#include "ObjectManager.h"
#include <vismodule/RendererManager>
#include "IDManager.h"
#include <vismodule/Message>
#include <vismodule/RGBColor>


namespace pbvr
{

// Parameter instantiation.
int                       GlobalCore::m_argc;
char**                    GlobalCore::m_argv;
vismodule::Camera*              GlobalCore::m_camera;
/* 131018 removed
vismodule::Light*               GlobalCore::light;
vismodule::Mouse*               GlobalCore::mouse;
vismodule::Background*          GlobalCore::background;
*/
GlobalCore::ControlTarget GlobalCore::m_target;
pbvr::ObjectManager*       GlobalCore::m_object_manager;
vismodule::RendererManager*     GlobalCore::m_renderer_manager;
pbvr::IDManager*           GlobalCore::m_id_manager;

/*==========================================================================*/
/**
 *  Consturctor.
 *  @param count [in] argument count
 *  @param values [in] argument values
 */
/*==========================================================================*/
GlobalCore::GlobalCore( const int count, const char** values )
{
    createCore( count, values );
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
GlobalCore::~GlobalCore()
{
    clearCore();
}

/*==========================================================================*/
/**
 *  Create the global core class.
 *  @param count [in] argument count
 *  @param values [in] argument values
 */
/*==========================================================================*/
void GlobalCore::createCore( const int count, const char** values )
{
    m_argc   = count;
    m_argv   = ( char** )values;
    m_target = TargetObject;

    m_camera = new vismodule::Camera();
    if ( !m_camera )
    {
        visModuleMessageError( "Cannot allocate memory for the camera." );
        return;
    }
    /* 131018 removed
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
    */
    m_object_manager = new pbvr::ObjectManager();
    if ( !m_object_manager )
    {
        visModuleMessageError( "Cannot allocate memory for the object manager." );
        return;
    }

    m_renderer_manager = new vismodule::RendererManager();
    if ( !m_renderer_manager )
    {
        visModuleMessageError( "Cannot allocate memory for the renderer manager." );
        return;
    }

    m_id_manager = new pbvr::IDManager();
    if ( !m_id_manager )
    {
        visModuleMessageError( "Cannot allocate memory for the ID manager." );
        return;
    }
}

/*==========================================================================*/
/**
 *  Clear the global core class.
 */
/*==========================================================================*/
void GlobalCore::clearCore()
{
    if ( m_camera           )
    {
        delete m_camera;
        m_camera           = NULL;
    }
    /* 131018 removed
        if( light            ){ delete light;            light            = NULL; }
        if( mouse            ){ delete mouse;            mouse            = NULL; }
        if( background       ){ delete background;       background       = NULL; }
    */
    if ( m_object_manager   )
    {
        delete m_object_manager;
        m_object_manager   = NULL;
    }
    if ( m_renderer_manager )
    {
        delete m_renderer_manager;
        m_renderer_manager = NULL;
    }
    if ( m_id_manager       )
    {
        delete m_id_manager;
        m_id_manager       = NULL;
    }
}

/*==========================================================================*/
/**
 *  Reset the global core class.
 */
/*==========================================================================*/
void GlobalCore::resetCore()
{
    // Reset viewpoint to the initial position.
//    mouse->reset(); /* 131018 removed */

    // Reset the xform of the object.
    if ( m_object_manager->hasActiveObject() )
    {
        m_object_manager->resetActiveObjectXform();
    }
    else
    {
        m_object_manager->resetXform();
    }

    // Reset the xform of the camera.
    m_camera->resetXform();

    // Reset the xform of the light.
//    light->resetXform(); /* 131018 removed */
}

} // end of namespace pbvr
