/*****************************************************************************/
/**
 *  @file   TableObjectTag.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TableObjectTag.h 846 2011-06-21 07:04:44Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__TABLE_OBJECT_TAG_H_INCLUDE
#define VIS_MODULE__KVSML__TABLE_OBJECT_TAG_H_INCLUDE

#include <vismodule/XMLNode>
#include "TagBase.h"


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <TableObject>
 */
/*===========================================================================*/
class TableObjectTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    bool m_has_nrows; ///< flag to check whether 'nrows' is specified or not
    std::size_t m_nrows; ///< number of rows
    bool m_has_ncolumns; ///< flag to check whether 'ncolumns' is specified or not
    std::size_t m_ncolumns; ///< number of columns

public:

    TableObjectTag( void );

public:

    const bool hasNRows( void ) const;

    const std::size_t nrows( void ) const;

    const bool hasNColumns( void ) const;

    const std::size_t ncolumns( void ) const;

public:

    void setNRows( const std::size_t nrows );

    void setNColumns( const std::size_t ncolumns );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namesapce kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__TABLE_OBJECT_TAG_H_INCLUDE
