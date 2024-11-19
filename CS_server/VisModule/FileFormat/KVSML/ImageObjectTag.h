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
#ifndef KVS__KVSML__IMAGE_OBJECT_H_INCLUDE
#define KVS__KVSML__IMAGE_OBJECT_H_INCLUDE

#include <string>
#include <kvs/XMLNode>
#include <kvs/Vector3>
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <ImageObject>
 */
/*===========================================================================*/
class ImageObjectTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

public:

    bool m_has_width; ///< flag to check whether 'width' is specified or not
    size_t m_width; ///< image width
    bool m_has_height; ///< flag to check whether 'height' is specified or not
    size_t m_height; ///< image height

public:

    ImageObjectTag( void );

    virtual ~ImageObjectTag( void );

public:

    const bool hasWidth( void ) const;

    const size_t width( void ) const;

    const bool hasHeight( void ) const;

    const size_t height( void ) const;

public:

    void setWidth( const size_t width );

    void setHeight( const size_t height );

public:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__IMAGE_OBJECT_H_INCLUDE
