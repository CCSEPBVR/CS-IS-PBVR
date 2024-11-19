/*****************************************************************************/
/**
 *  @file   OpacityMapTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: OpacityMapTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__OPACITY_MAP_TAG_H_INCLUDE
#define VIS_MODULE__KVSML__OPACITY_MAP_TAG_H_INCLUDE

#include <vismodule/XMLNode>
#include <vismodule/XMLElement>
#include "TagBase.h"


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <OpacityMap>
 */
/*===========================================================================*/
class OpacityMapTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

public:

    OpacityMapTag( void );

    virtual ~OpacityMapTag( void );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};


/*===========================================================================*/
/**
 *  @brief  <OpacityMapValue> tag class.
 */
/*===========================================================================*/
class OpacityMapValueTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    float m_scalar; ///< scalr value
    float m_opacity; ///< opacity value

public:

    OpacityMapValueTag( void );

    virtual ~OpacityMapValueTag( void );

public:

    const float scalar( void ) const;

    const float opacity( void ) const;

    void setScalar( const float scalar );

    void setOpacity( const float opacity );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool read( const vismodule::XMLElement::SuperClass* element );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__OPACITY_MAP_TAG_H_INCLUDE
