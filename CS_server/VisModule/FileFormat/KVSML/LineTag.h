/*****************************************************************************/
/**
 *  @file   LineTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LineTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KVSML__LINE_TAG_H_INCLUDE
#define KVS__KVSML__LINE_TAG_H_INCLUDE

#include <kvs/XMLNode>
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Line>
 */
/*===========================================================================*/
class LineTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

protected:

    bool m_has_nlines; ///< flag to check whether 'nlines' is specified or not
    size_t m_nlines; ///< number of lines

public:

    LineTag( void );

    virtual ~LineTag( void );

public:

    const bool hasNLines( void ) const;

    const size_t nlines( void ) const;

public:

    void setNLines( const size_t nlines );

public:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__LINE_TAG_H_INCLUDE
