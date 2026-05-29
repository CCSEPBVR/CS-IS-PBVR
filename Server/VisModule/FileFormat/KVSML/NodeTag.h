/*****************************************************************************/
/**
 *  @file   NodeTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: NodeTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__NODE_TAG_H_INCLUDE
#define VIS_MODULE__KVSML__NODE_TAG_H_INCLUDE

#include <vismodule/XMLNode>
#include "TagBase.h"


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Node>
 */
/*===========================================================================*/
class NodeTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    bool m_has_nnodes; ///< flag to check whether 'nnodes' is specified or not
    std::size_t m_nnodes; ///< number of nodes

public:

    NodeTag( void );

    virtual ~NodeTag( void );

public:

    const bool hasNNodes( void ) const;

    const std::size_t nnodes( void ) const;

public:

    void setNNodes( const std::size_t nnodes );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__NODE_TAG_H_INCLUDE
