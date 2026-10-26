/*****************************************************************************/
/**
 *  @file   ImageObjectTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImageObjectTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__IMAGE_OBJECT_H_INCLUDE
#define VIS_MODULE__KVSML__IMAGE_OBJECT_H_INCLUDE

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
 *  @brief  Tag class for <ImageObject>
 */
/*===========================================================================*/
class ImageObjectTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

public:

    bool m_has_width; ///< flag to check whether 'width' is specified or not
    std::size_t m_width; ///< image width
    bool m_has_height; ///< flag to check whether 'height' is specified or not
    std::size_t m_height; ///< image height

public:

    ImageObjectTag( void );

    virtual ~ImageObjectTag( void );

public:

    const bool hasWidth( void ) const;

    const std::size_t width( void ) const;

    const bool hasHeight( void ) const;

    const std::size_t height( void ) const;

public:

    void setWidth( const std::size_t width );

    void setHeight( const std::size_t height );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__IMAGE_OBJECT_H_INCLUDE
