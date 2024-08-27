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
#ifndef KVS__KVSML__OPACITY_MAP_TAG_H_INCLUDE
#define KVS__KVSML__OPACITY_MAP_TAG_H_INCLUDE

#include <kvs/XMLNode>
#include <kvs/XMLElement>
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <OpacityMap>
 */
/*===========================================================================*/
class OpacityMapTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

public:

    OpacityMapTag( void );

    virtual ~OpacityMapTag( void );

public:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
};


/*===========================================================================*/
/**
 *  @brief  <OpacityMapValue> tag class.
 */
/*===========================================================================*/
class OpacityMapValueTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

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

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool read( const kvs::XMLElement::SuperClass* element );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__OPACITY_MAP_TAG_H_INCLUDE
