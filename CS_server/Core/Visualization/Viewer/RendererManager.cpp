/****************************************************************************/
/**
 *  @file RendererManager.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RendererManager.cpp 621 2010-09-30 08:04:55Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "RendererManager.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
RendererManager::RendererManager( void )
{
    RendererManagerBase::clear();
    m_renderer_map.clear();
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
RendererManager::~RendererManager( void )
{
    this->erase();
}

/*==========================================================================*/
/**
 *  Insert the renderer.
 *  @param renderer [in] pointer to the renderer
 *  @return renderer ID
 */
/*==========================================================================*/
int RendererManager::insert( kvs::RendererBase* renderer )
{
    static int renderer_id = 0;
    renderer_id++;

    m_renderer_map[ renderer_id ] =
        RendererManagerBase::insert( this->end(), renderer );

    return( renderer_id );
}

/*==========================================================================*/
/**
 *  Erase the renderer.
 *  @param delete_flg [in] deleting the allocated memory flag
 */
/*==========================================================================*/
void RendererManager::erase( bool delete_flg )
{
    if ( delete_flg )
    {
        for ( RendererIterator p = begin(); p != end(); p++ )
        {
            if ( *p ) { delete( *p ); *p = NULL; }
        }
    }

    RendererManagerBase::clear();
    m_renderer_map.clear();
}

/*==========================================================================*/
/**
 *  Erase the renderer which is specified by the given ID.
 *  @param renderer_id [in] renderer ID
 *  @param delete_flg [in] deleting the allocated memory flag
 */
/*==========================================================================*/
void RendererManager::erase( int renderer_id, bool delete_flg )
{
    RendererMap::iterator map_id = m_renderer_map.find( renderer_id );
    if ( map_id == m_renderer_map.end() ) return;

    RendererIterator renderer_ptr = map_id->second; // pointer to the renderer
    kvs::RendererBase* renderer = *renderer_ptr;     // renderer

    if ( delete_flg )
    {
        if ( renderer )
        {
            delete( renderer );
            renderer = NULL;
        }
    }

    // Erase the renderer in the renderer master.
    RendererManagerBase::erase( renderer_ptr );

    // Erase the map component, which is specified by map_id.
    m_renderer_map.erase( map_id );
}

/*==========================================================================*/
/**
 *  Erase the renderer which is specified by the given name.
 *  @param renderer_name [in] renderer name
 *  @param delete_flg [in] deleting the allocated memory flag
 */
/*==========================================================================*/
void RendererManager::erase( std::string renderer_name, bool delete_flg )
{
    RendererMap::iterator map_id = m_renderer_map.begin();
    RendererMap::iterator map_end = m_renderer_map.end();

    while ( map_id != map_end )
    {
        RendererIterator renderer_ptr = map_id->second;
        kvs::RendererBase* renderer = *renderer_ptr;
        if ( renderer->name() == renderer_name )
        {
            if ( delete_flg ) { if ( renderer ) delete( renderer ); }

            // Erase the renderer in the renderer master.
            RendererManagerBase::erase( renderer_ptr );

            // Erase the map component, which is specified by map_id.
            m_renderer_map.erase( map_id );

            break;
        }

        ++map_id;
    }
}

/*==========================================================================*/
/**
 *  Change the renderer by the specificated renderer ID.
 *  @param renderer_id [in] renderer ID stored in the renderer manager
 *  @param renderer [in] pointer to the inserting renderer
 *  @param delete_flg [in] deleting the allocated memory flag
 */
/*==========================================================================*/
void RendererManager::change( int renderer_id, kvs::RendererBase* renderer, bool delete_flg )
{
    /* Search the object which is specified by given renderer ID in the
     * renderer pointer map. If it isn't found, this method executes nothing.
     */
    RendererMap::iterator map_id = m_renderer_map.find( renderer_id );
    if( map_id == m_renderer_map.end() ) return;

    // Change the renderer.
    RendererIterator ptr = map_id->second; // pointer to the renderer
    kvs::RendererBase* old_renderer = *ptr;

    // Erase the old renderer.
    if ( delete_flg )
    {
        if ( old_renderer )
        {
            delete( old_renderer );
            old_renderer = NULL;
        }
    }

    // Insert the new object
    *ptr = renderer;
}

/*==========================================================================*/
/**
 *  Change the renderer by the specificated renderer name.
 *  @param renderer_name [in] renderer name stored in the renderer manager
 *  @param renderer [in] pointer to the inserting renderer
 *  @param delete_flg [in] deleting the allocated memory flag
 */
/*==========================================================================*/
void RendererManager::change( std::string renderer_name, kvs::RendererBase* renderer, bool delete_flg )
{
    RendererMap::iterator map_id = m_renderer_map.begin();
    RendererMap::iterator map_end = m_renderer_map.end();

    while ( map_id != map_end )
    {
        RendererIterator old_renderer_ptr = map_id->second;
        kvs::RendererBase* old_renderer = *old_renderer_ptr;
        if ( old_renderer->name() == renderer_name )
        {
            if ( delete_flg ) { if ( old_renderer ) delete( old_renderer ); }

            // Insert the new renderer
            *old_renderer_ptr = renderer;

            break;
        }

        ++map_id;
    }
}

/*==========================================================================*/
/**
 *  Get the number of the stored renderers.
 *  @return number of the stored renderers
 */
/*==========================================================================*/
const int RendererManager::nrenderers( void ) const
{
    return( RendererManagerBase::size() );
}

/*==========================================================================*/
/**
 *  Get the renderer.
 *  @return pointer to the renderer
 */
/*==========================================================================*/
kvs::RendererBase* RendererManager::renderer( void )
{
    // Pointer to the renderer.
    RendererIterator renderer_ptr = begin();

    if( !*renderer_ptr ) return( NULL );
    else                 return( *renderer_ptr );
}

/*==========================================================================*/
/**
 *  Get the renderer which is specified by the given ID.
 *  @param renderer_id [in] renderer ID
 *  @return pointer to the renderer
 */
/*==========================================================================*/
kvs::RendererBase* RendererManager::renderer( int renderer_id )
{
    RendererMap::iterator map_id = m_renderer_map.find( renderer_id );
    if( map_id == m_renderer_map.end() )   return( NULL );

    // Pointer to the renderer.
    RendererIterator renderer_ptr = map_id->second;

    return( *renderer_ptr );
}

/*==========================================================================*/
/**
 *  Get the renderer which is specified by the given name.
 *  @param renderer_name [in] renderer name
 *  @return pointer to the renderer
 */
/*==========================================================================*/
kvs::RendererBase* RendererManager::renderer( std::string renderer_name )
{
    RendererMap::iterator map_id = m_renderer_map.begin();
    RendererMap::iterator map_end = m_renderer_map.end();

    while ( map_id != map_end )
    {
        RendererIterator renderer_ptr = map_id->second;
        kvs::RendererBase* renderer = *renderer_ptr;
        if ( renderer->name() == renderer_name )
        {
            return( renderer );
        }

        ++map_id;
    }

    return( NULL );
}

/*==========================================================================*/
/**
 *  Test whether the renderer manager has renderers.
 *  @return true, if the renderer manager has renderers.
 */
/*==========================================================================*/
const bool RendererManager::hasRenderer( void ) const
{
    return( RendererManagerBase::size() != 0 );
}

} // end of namespace kvs
