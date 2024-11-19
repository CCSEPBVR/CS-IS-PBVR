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
#ifndef VIS_MODULE__KVSML__COLOR_MAP_TAG_H_INCLUDE
#define VIS_MODULE__KVSML__COLOR_MAP_TAG_H_INCLUDE

#include <vismodule/XMLNode>
#include <vismodule/XMLElement>
#include <vismodule/RGBColor>
#include "TagBase.h"


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <ColorMap>
 */
/*===========================================================================*/
class ColorMapTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

public:

    ColorMapTag( void );

    virtual ~ColorMapTag( void );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};


/*===========================================================================*/
/**
 *  @brief  <ColorMapValue> tag class.
 */
/*===========================================================================*/
class ColorMapValueTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    float m_scalar; ///< scalr value
    vismodule::RGBColor m_color; ///< color value

public:

    ColorMapValueTag( void );

    virtual ~ColorMapValueTag( void );

public:

    const float scalar( void ) const;

    const vismodule::RGBColor color( void ) const;

    void setScalar( const float scalar );

    void setColor( const vismodule::RGBColor color );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool read( const vismodule::XMLElement::SuperClass* element );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__COLOR_MAP_TAG_H_INCLUDE
