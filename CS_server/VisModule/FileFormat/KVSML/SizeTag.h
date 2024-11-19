/*****************************************************************************/
/**
 *  @file   SizeTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SizeTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__SIZE_TAG_H_INCLUDE
#define VIS_MODULE__KVSML__SIZE_TAG_H_INCLUDE

#include <vismodule/XMLNode>
#include "TagBase.h"


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Size>
 */
/*===========================================================================*/
class SizeTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

public:

    SizeTag( void );

    virtual ~SizeTag( void );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__SIZE_TAG_H_INCLUDE
