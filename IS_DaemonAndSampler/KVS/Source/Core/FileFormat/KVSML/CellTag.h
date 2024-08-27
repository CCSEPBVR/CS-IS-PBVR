/*****************************************************************************/
/**
 *  @file   CellTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KVSML__CELL_TAG_H_INCLUDE
#define KVS__KVSML__CELL_TAG_H_INCLUDE

#include <kvs/XMLNode>
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Cell>
 */
/*===========================================================================*/
class CellTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

protected:

    bool m_has_ncells; ///< flag to check whether 'ncells' is specified or not
    size_t m_ncells; ///< number of cells

public:

    CellTag( void );

    virtual ~CellTag( void );

public:

    const bool hasNCells( void ) const;

    const size_t ncells( void ) const;

public:

    void setNCells( const size_t ncells );

public:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__CELL_TAG_H_INCLUDE
