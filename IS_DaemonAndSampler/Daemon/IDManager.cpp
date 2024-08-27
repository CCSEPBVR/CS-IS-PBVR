/****************************************************************************/
/**
 *  @file IDManager.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: IDManager.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "IDManager.h"
#include <algorithm>
#include <functional>


namespace pbvr
{

/*==========================================================================*/
/**
 *  Default constructor.
 */
/*==========================================================================*/
IDManager::IDManager( void )
{
    m_flip_table.clear();
    clear();
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
IDManager::~IDManager( void )
{
    m_flip_table.clear();
    clear();
}

const IDPair IDManager::operator [] ( size_t index ) const
{
    const int id = m_flip_table[index];

//    ID_ptr p = begin();
    IDs::const_iterator p = begin();
    for ( int i = 0; i < id; i++ )
    {
        p++;
    }

    return ( ( *p ) );
}

const size_t IDManager::size( void ) const
{
    return ( IDs::size() );
}

void IDManager::insert( int object_id, int renderer_id )
{
    push_back( IDPair( object_id, renderer_id ) );
    update_flip_table();
}

void IDManager::insertObjectID( int object_id )
{
    insert( object_id, back().second );
    update_flip_table();
}

void IDManager::insertRendererID( int renderer_id )
{
    insert( back().first, renderer_id );
    update_flip_table();
}

const std::vector<int> IDManager::objectID( int renderer_id ) const
{
    std::vector<int> object_ids;

//    for( ID_ptr p = begin(); p != end(); p++ )
    for ( IDs::const_iterator p = begin(); p != end(); p++ )
    {
        if ( renderer_id == p->second ) object_ids.push_back( p->first );
    }

    return ( object_ids );
}

const int IDManager::objectID( void ) const
{
    return ( back().first );
}

const std::vector<int> IDManager::rendererID( int object_id ) const
{
    std::vector<int> renderer_ids;

//    for( ID_ptr p = begin(); p != end(); p++ )
    for ( IDs::const_iterator p = begin(); p != end(); p++ )
    {
        if ( object_id == p->first ) renderer_ids.push_back( p->second );
    }

    return ( renderer_ids );
}

const int IDManager::rendererID( void ) const
{
    return ( back().second );
}

void IDManager::erase( void )
{
    IDs::clear();
    m_flip_table.clear();
}

void IDManager::erase( int object_id, int renderer_id )
{
    std::vector<ID_ptr> erase_ptr;

    for ( ID_ptr p = begin(); p != end(); p++ )
    {
        if ( object_id == p->first && renderer_id == p->second )
        {
            erase_ptr.push_back( p );
        }
    }

    for ( int i = 0; i < ( int )erase_ptr.size(); i++ )
    {
        IDs::erase( erase_ptr[i] );
    }

    update_flip_table();
}

void IDManager::eraseByObjectID( int object_id )
{
    std::vector<ID_ptr> erase_ptr;

    for ( ID_ptr p = begin(); p != end(); p++ )
    {
        if ( object_id == p->first )
        {
            erase_ptr.push_back( p );
        }
    }

    for ( int i = 0; i < ( int )erase_ptr.size(); i++ )
    {
        IDs::erase( erase_ptr[i] );
    }

    update_flip_table();
}

void IDManager::eraseByRendererID( int renderer_id )
{
    std::vector<ID_ptr> erase_ptr;

    for ( ID_ptr p = begin(); p != end(); p++ )
    {
        if ( renderer_id == p->second )
        {
            erase_ptr.push_back( p );
        }
    }

    for ( int i = 0; i < ( int )erase_ptr.size(); i++ )
    {
        IDs::erase( erase_ptr[i] );
    }

    update_flip_table();
}

void IDManager::changeObject( int object_id )
{
    back().first = object_id;
}

void IDManager::changeObject( int renderer_id, int object_id )
{
    for ( ID_rptr p = rbegin(); p != rend(); p++ )
    {
        if ( renderer_id == p->second )
        {
            p->first = object_id;
            return;
        }
    }
}

void IDManager::changeObject( const IDPair& id_pair, int object_id )
{
    for ( ID_rptr p = rbegin(); p != rend(); p++ )
    {
        if ( *p == id_pair )
        {
            p->first = object_id;
            return;
        }
    }
}

void IDManager::changeRenderer( int renderer_id )
{
    back().second = renderer_id;
}

void IDManager::changeRenderer( int object_id, int renderer_id )
{
    for ( ID_rptr p = rbegin(); p != rend(); p++ )
    {
        if ( object_id == p->first )
        {
            p->second = renderer_id;
            return;
        }
    }
}

void IDManager::changeRenderer( const IDPair& id_pair, int renderer_id )
{
    for ( ID_rptr p = rbegin(); p != rend(); p++ )
    {
        if ( *p == id_pair )
        {
            p->second = renderer_id;
            return;
        }
    }
}

void IDManager::flip( void )
{
    std::vector<int> next( size() );

    std::copy( m_flip_table.begin(), m_flip_table.end(), next.begin() );

    next_permutation( next.begin(), next.end(), std::less<int>() );

    std::copy( next.begin(), next.end(), m_flip_table.begin() );
}

void IDManager::update_flip_table( void )
{
    m_flip_table.clear();

    const int n = size();

    for ( int i = 0; i < n; i++ )
    {
        m_flip_table.push_back( i );
    }
}

} // end of namespace pbvr
