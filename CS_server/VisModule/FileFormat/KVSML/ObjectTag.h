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
#ifndef VIS_MODULE__KVSML__OBJECT_TAG_H_INCLUDE
#define VIS_MODULE__KVSML__OBJECT_TAG_H_INCLUDE

#include <string>
#include <vismodule/Vector3>
#include <vismodule/XMLDocument>
#include <vismodule/XMLNode>
#include "TagBase.h"


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Object>.
 */
/*===========================================================================*/
class ObjectTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    bool m_has_type; ///< flag to check whether 'type' is specified or not
    std::string m_type; ///< object type
    bool m_has_external_coord; ///< flag to check whether 'external_coord' is specified or not
    vismodule::Vector3f m_min_external_coord; ///< min. external coordinate
    vismodule::Vector3f m_max_external_coord; ///< max. external coordinate
    bool m_has_object_coord; ///< flag to check whether 'object_coord' is specified or not
    vismodule::Vector3f m_min_object_coord; ///< min. object coordinate
    vismodule::Vector3f m_max_object_coord; ///< max. object coordinate

public:

    ObjectTag( void );

    virtual ~ObjectTag( void );

public:

    const bool hasType( void ) const;

    const std::string& type( void ) const;

    const bool hasExternalCoord( void ) const;

    const vismodule::Vector3f& minExternalCoord( void ) const;

    const vismodule::Vector3f& maxExternalCoord( void ) const;

    const bool hasObjectCoord( void ) const;

    const vismodule::Vector3f& minObjectCoord( void ) const;

    const vismodule::Vector3f& maxObjectCoord( void ) const;

public:

    void setType( const std::string& type );

    void setMinMaxExternalCoords( const vismodule::Vector3f& min_coord, const vismodule::Vector3f& max_coord );

    void setMinMaxObjectCoords( const vismodule::Vector3f& min_coord, const vismodule::Vector3f& max_coord );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__OBJECT_TAG_H_INCLUDE
