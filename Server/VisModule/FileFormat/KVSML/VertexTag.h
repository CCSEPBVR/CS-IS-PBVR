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
#ifndef VIS_MODULE__KVSML__VERTEX_TAG_H_INCLUDE
#define VIS_MODULE__KVSML__VERTEX_TAG_H_INCLUDE

#include <vismodule/XMLNode>
#include "TagBase.h"


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Vertex>
 */
/*===========================================================================*/
class VertexTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    bool m_has_nvertices; ///< flag to check whether 'nvertices' is specified or not
    std::size_t m_nvertices; ///< number of vertices

public:

    VertexTag( void );

    virtual ~VertexTag( void );

public:

    const bool hasNVertices( void ) const;

    const std::size_t nvertices( void ) const;

public:

    void setNVertices( const std::size_t nvertices );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__VERTEX_TAG_H_INCLUDE
