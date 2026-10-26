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
#ifndef VIS_MODULE__KVSML__CELL_TAG_H_INCLUDE
#define VIS_MODULE__KVSML__CELL_TAG_H_INCLUDE

#include <vismodule/XMLNode>
#include "TagBase.h"


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Cell>
 */
/*===========================================================================*/
class CellTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    bool m_has_ncells; ///< flag to check whether 'ncells' is specified or not
    std::size_t m_ncells; ///< number of cells

public:

    CellTag( void );

    virtual ~CellTag( void );

public:

    const bool hasNCells( void ) const;

    const std::size_t ncells( void ) const;

public:

    void setNCells( const std::size_t ncells );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__CELL_TAG_H_INCLUDE
