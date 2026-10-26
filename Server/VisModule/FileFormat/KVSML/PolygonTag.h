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
#ifndef VIS_MODULE__KVSML__POLYGON_TAG_H_INCLUDE
#define VIS_MODULE__KVSML__POLYGON_TAG_H_INCLUDE

#include <vismodule/XMLNode>
#include "TagBase.h"


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Polygon>
 */
/*===========================================================================*/
class PolygonTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    bool m_has_npolygons; ///< flag to check whether 'npolygons' is specified or not
    std::size_t m_npolygons; ///< number of polygons

public:

    PolygonTag( void );

    virtual ~PolygonTag( void );

public:

    const bool hasNPolygons( void ) const;

    const std::size_t npolygons( void ) const;

public:

    void setNPolygons( const std::size_t npolygons );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__POLYGONS_TAG_H_INCLUDE
