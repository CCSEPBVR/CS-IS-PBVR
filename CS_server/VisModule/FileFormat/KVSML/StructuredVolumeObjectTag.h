/*****************************************************************************/
/**
 *  @file   StructuredVolumeObjectTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StructuredVolumeObjectTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__STRUCTURED_VOLUME_OBJECT_H_INCLUDE
#define VIS_MODULE__KVSML__STRUCTURED_VOLUME_OBJECT_H_INCLUDE

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
 *  @brief  Tag class for <StructuredVolumeObject>
 */
/*===========================================================================*/
class StructuredVolumeObjectTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    bool m_has_grid_type; ///< flag to check whether 'grid_type' is specified or not
    std::string m_grid_type; ///< grid type
    bool m_has_resolution; ///< flag to check whether 'resolution' is specified or not
    vismodule::Vector3ui m_resolution; ///< grid resolution

public:

    StructuredVolumeObjectTag( void );

    virtual ~StructuredVolumeObjectTag( void );

public:

    const bool hasGridType( void ) const;

    const std::string& gridType( void ) const;

    const bool hasResolution( void ) const;

    const vismodule::Vector3ui& resolution( void ) const;

public:

    void setGridType( const std::string& grid_type );

    void setResolution( const vismodule::Vector3ui& resolution );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__STRUCTURED_VOLUME_OBJECT_H_INCLUDE
