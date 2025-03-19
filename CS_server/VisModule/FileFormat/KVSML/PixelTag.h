/*****************************************************************************/
/**
 *  @file   PixelTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PixelTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__PIXEL_TAG_H_INCLUDE
#define VIS_MODULE__KVSML__PIXEL_TAG_H_INCLUDE

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
 *  @brief  Tag class for <Pixel>
 */
/*===========================================================================*/
class PixelTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    bool m_has_type; ///< flag to check whether 'type' is specified or not
    std::string m_type; ///< pixel type

public:

    PixelTag( void );

    virtual ~PixelTag( void );

public:

    const bool hasType( void ) const;

    const std::string& type( void ) const;

public:

    void setType( const std::string& type );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__PIXEL_TAG_H_INCLUDE
