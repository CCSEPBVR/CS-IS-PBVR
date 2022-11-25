/*****************************************************************************/
/**
 *  @file   ColorMapTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ColorMapTag.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KVSML__COLOR_MAP_TAG_H_INCLUDE
#define KVS__KVSML__COLOR_MAP_TAG_H_INCLUDE

#include <kvs/XMLNode>
#include <kvs/XMLElement>
#include <kvs/RGBColor>
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <ColorMap>
 */
/*===========================================================================*/
class ColorMapTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

public:

    ColorMapTag( void );

    virtual ~ColorMapTag( void );

public:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
};


/*===========================================================================*/
/**
 *  @brief  <ColorMapValue> tag class.
 */
/*===========================================================================*/
class ColorMapValueTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

protected:

    float m_scalar; ///< scalr value
    kvs::RGBColor m_color; ///< color value

public:

    ColorMapValueTag( void );

    virtual ~ColorMapValueTag( void );

public:

    const float scalar( void ) const;

    const kvs::RGBColor color( void ) const;

    void setScalar( const float scalar );

    void setColor( const kvs::RGBColor color );

public:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool read( const kvs::XMLElement::SuperClass* element );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__COLOR_MAP_TAG_H_INCLUDE
