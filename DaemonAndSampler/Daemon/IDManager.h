/****************************************************************************/
/**
 *  @file IDManager.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: IDManager.h 994 2011-10-18 08:33:16Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__ID_MANAGER_H_INCLUDE
#define PBVR__ID_MANAGER_H_INCLUDE

#include <list>
#include <vector>
#include <utility>
#include <cstdlib>
#include "ClassName.h"


namespace pbvr
{

typedef std::pair<int, int>                  IDPair;
typedef std::list<IDPair>                   IDs;
typedef std::list<IDPair>::iterator         ID_ptr;
typedef std::list<IDPair>::reverse_iterator ID_rptr;

/*==========================================================================*/
/**
 *  ID manager class.
 */
/*==========================================================================*/
class IDManager : public IDs
{
    kvsClassName( pbvr::IDManager );

protected:

    std::vector<int> m_flip_table;

public:

    IDManager( void );

    virtual ~IDManager( void );

public:

    const IDPair operator [] ( size_t index ) const;

    const size_t size( void ) const;

    void insert( int object_id, int renderer_id );

    void insertObjectID( int object_id );

    void insertRendererID( int renderer_id );

    const std::vector<int> objectID( int renderer_id ) const;

    const int objectID( void ) const;

    const std::vector<int> rendererID( int object_id ) const;

    const int rendererID( void ) const;

    void erase( void );

    void erase( int object_id, int renderer_id );

    void eraseByObjectID( int object_id );

    void eraseByRendererID( int renderer_id );

    void changeObject( int object_id );

    void changeObject( int renderer_id, int object_id );

    void changeObject( const IDPair& id_pair, int object_id );

    void changeRenderer( int renderer_id );

    void changeRenderer( int object_id, int renderer_id );

    void changeRenderer( const IDPair& id_pair, int renderer_id );

    void flip( void );

private:

    void update_flip_table( void );
};

} // end of namespace pbvr

#endif // KVS__ID_MANAGER_H_INCLUDE
