/*****************************************************************************/
/**
 *  @file   ObjectTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ObjectTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KVSML__OBJECT_TAG_H_INCLUDE
#define KVS__KVSML__OBJECT_TAG_H_INCLUDE

#include <string>
#include <kvs/Vector3>
#include <kvs/XMLDocument>
#include <kvs/XMLNode>
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Object>.
 */
/*===========================================================================*/
class ObjectTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

protected:

    bool m_has_type; ///< flag to check whether 'type' is specified or not
    std::string m_type; ///< object type
    bool m_has_external_coord; ///< flag to check whether 'external_coord' is specified or not
    kvs::Vector3f m_min_external_coord; ///< min. external coordinate
    kvs::Vector3f m_max_external_coord; ///< max. external coordinate
    bool m_has_object_coord; ///< flag to check whether 'object_coord' is specified or not
    kvs::Vector3f m_min_object_coord; ///< min. object coordinate
    kvs::Vector3f m_max_object_coord; ///< max. object coordinate

public:

    ObjectTag( void );

    virtual ~ObjectTag( void );

public:

    const bool hasType( void ) const;

    const std::string& type( void ) const;

    const bool hasExternalCoord( void ) const;

    const kvs::Vector3f& minExternalCoord( void ) const;

    const kvs::Vector3f& maxExternalCoord( void ) const;

    const bool hasObjectCoord( void ) const;

    const kvs::Vector3f& minObjectCoord( void ) const;

    const kvs::Vector3f& maxObjectCoord( void ) const;

public:

    void setType( const std::string& type );

    void setMinMaxExternalCoords( const kvs::Vector3f& min_coord, const kvs::Vector3f& max_coord );

    void setMinMaxObjectCoords( const kvs::Vector3f& min_coord, const kvs::Vector3f& max_coord );

public:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__OBJECT_TAG_H_INCLUDE
