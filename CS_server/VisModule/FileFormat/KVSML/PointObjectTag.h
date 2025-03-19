/*****************************************************************************/
/**
 *  @file   PointObjectTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PointObjectTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__POINT_OBJECT_H_INCLUDE
#define VIS_MODULE__KVSML__POINT_OBJECT_H_INCLUDE

#include <string>
#include <vismodule/XMLNode>
#include <vismodule/Vector3>
#include "TagBase.h"


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <PointObject>
 */
/*===========================================================================*/
class PointObjectTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

public:

    PointObjectTag( void );

    virtual ~PointObjectTag( void );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__POINT_OBJECT_H_INCLUDE
