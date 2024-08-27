/****************************************************************************/
/**
 *  @file Global.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Global.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Global.h"
#include <kvs/ObjectManager>
#include <kvs/RendererManager>
#include <kvs/IDManager>


namespace kvs
{

namespace glut
{

/*==========================================================================*/
/**
 *  Construct a new Global class.
 *  @param count [in] argument count
 *  @param values [in] argument values
 */
/*==========================================================================*/
Global::Global( int count, char** values ):
    kvs::glut::GlobalBase( count, values )
{
}

/*==========================================================================*/
/**
 *  Destruct.
 */
/*==========================================================================*/
Global::~Global( void )
{
}

/*==========================================================================*/
/**
 *  Insert a object and renderer to the managers.
 *  @param pipeline [in] visualization pipeline
 */
/*==========================================================================*/
void Global::insert( const kvs::VisualizationPipeline& pipeline )
{
    /* WARNING */
    // It is necessary to increment the reference counter of the pipeline.object()
    // and the pipeline.renderer().
    kvs::ObjectBase*   object   = const_cast<kvs::ObjectBase*>(pipeline.object());
    kvs::RendererBase* renderer = const_cast<kvs::RendererBase*>(pipeline.renderer());

    const size_t object_id   = kvs::glut::GlobalBase::object_manager->insert( object );
    const size_t renderer_id = kvs::glut::GlobalBase::renderer_manager->insert( renderer );
    kvs::glut::GlobalBase::id_manager->insert( object_id, renderer_id );
}

} // end of namespace glut

} // end of namespace kvs
