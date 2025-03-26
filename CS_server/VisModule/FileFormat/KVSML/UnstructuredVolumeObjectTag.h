/*****************************************************************************/
/**
 *  @file   UnstructuredVolumeObjectTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: UnstructuredVolumeObjectTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE
#define VIS_MODULE__KVSML__UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE

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
 *  @brief  Tag class for <UnstructuredVolumeObject>
 */
/*===========================================================================*/
class UnstructuredVolumeObjectTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    bool m_has_cell_type; ///< flag to check whether 'cell_type' is specified or not
    std::string m_cell_type; ///< cell type

public:

    UnstructuredVolumeObjectTag( void );

    virtual ~UnstructuredVolumeObjectTag( void );

public:

    const bool hasCellType( void ) const;

    const std::string& cellType( void ) const;

public:

    void setCellType( const std::string& cell_type );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE
