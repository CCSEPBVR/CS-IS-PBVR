/*****************************************************************************/
/**
 *  @file   VertexTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VertexTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KVSML__VERTEX_TAG_H_INCLUDE
#define KVS__KVSML__VERTEX_TAG_H_INCLUDE

#include <kvs/XMLNode>
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Vertex>
 */
/*===========================================================================*/
class VertexTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

protected:

    bool m_has_nvertices; ///< flag to check whether 'nvertices' is specified or not
    size_t m_nvertices; ///< number of vertices

public:

    VertexTag( void );

    virtual ~VertexTag( void );

public:

    const bool hasNVertices( void ) const;

    const size_t nvertices( void ) const;

public:

    void setNVertices( const size_t nvertices );

public:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__VERTEX_TAG_H_INCLUDE
