/*****************************************************************************/
/**
 *  @file   TagBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TagBase.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__TAG_BASE_H_INCLUDE
#define VIS_MODULE__KVSML__TAG_BASE_H_INCLUDE

#include <vismodule/XMLNode>
#include <string>


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Base class of the tag.
 */
/*===========================================================================*/
class TagBase
{
protected:

    std::string m_name; ///< tag name
    vismodule::XMLNode::SuperClass* m_node; ///< pointer to the node of the XML tree

public:

    TagBase( const std::string& name );

    virtual ~TagBase( void );

public:

    vismodule::XMLNode::SuperClass* node( void );

    const vismodule::XMLNode::SuperClass* node( void ) const;

    const std::string& name( void ) const;

    const bool isExisted( const vismodule::XMLNode::SuperClass* parent ) const;

public:

    virtual const bool read( const vismodule::XMLNode::SuperClass* parent ) = 0;

    virtual const bool write( vismodule::XMLNode::SuperClass* parent ) = 0;

private:

    TagBase( void ){}
};

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__TAG_BASE_H_INCLUDE
