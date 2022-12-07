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

    IDManager();

    virtual ~IDManager();

public:

    const IDPair operator [] ( const size_t index ) const;

    const size_t size() const;

    void insert( const int object_id, const int renderer_id );

    void insertObjectID( const int object_id );

    void insertRendererID( const int renderer_id );

    const std::vector<int> objectID( const int renderer_id ) const;

    const int objectID() const;

    const std::vector<int> rendererID( const int object_id ) const;

    const int rendererID() const;

    void erase();

    void erase( const int object_id, const int renderer_id );

    void eraseByObjectID( const int object_id );

    void eraseByRendererID( const int renderer_id );

    void changeObject( const int object_id );

    void changeObject( const int renderer_id, const int object_id );

    void changeObject( const IDPair& id_pair, const int object_id );

    void changeRenderer( const int renderer_id );

    void changeRenderer( const int object_id, const int renderer_id );

    void changeRenderer( const IDPair& id_pair, const int renderer_id );

    void flip();

private:

    void update_flip_table();
};

} // end of namespace pbvr

#endif // KVS__ID_MANAGER_H_INCLUDE
