/*****************************************************************************/
/**
 *  @file   PolygonTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KVSML__POLYGON_TAG_H_INCLUDE
#define KVS__KVSML__POLYGON_TAG_H_INCLUDE

#include <kvs/XMLNode>
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Polygon>
 */
/*===========================================================================*/
class PolygonTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

protected:

    bool m_has_npolygons; ///< flag to check whether 'npolygons' is specified or not
    size_t m_npolygons; ///< number of polygons

public:

    PolygonTag( void );

    virtual ~PolygonTag( void );

public:

    const bool hasNPolygons( void ) const;

    const size_t npolygons( void ) const;

public:

    void setNPolygons( const size_t npolygons );

public:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__POLYGONS_TAG_H_INCLUDE
