/*****************************************************************************/
/**
 *  @file   TableObjectTag.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#pragma once
#include <kvs/XMLNode>
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <TableObject>
 */
/*===========================================================================*/
class TableObjectTag : public kvs::kvsml::TagBase
{
public:
    typedef kvs::kvsml::TagBase BaseClass;

private:
    bool m_has_nrows; ///< flag to check whether 'nrows' is specified or not
    std::size_t m_nrows; ///< number of rows
    bool m_has_ncolumns; ///< flag to check whether 'ncolumns' is specified or not
    std::size_t m_ncolumns; ///< number of columns

public:
    TableObjectTag();

    bool hasNRows() const;
    std::size_t nrows() const;
    bool hasNColumns() const;
    std::size_t ncolumns() const;

    void setNRows( const std::size_t nrows );
    void setNColumns( const std::size_t ncolumns );

    bool read( const kvs::XMLNode::SuperClass* parent );
    bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namesapce kvsml

} // end of namespace kvs
