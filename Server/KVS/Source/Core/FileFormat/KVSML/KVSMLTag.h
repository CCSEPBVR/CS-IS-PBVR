/*****************************************************************************/
/**
 *  @file   KVSMLTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KVSML__KVSML_TAG_H_INCLUDE
#define KVS__KVSML__KVSML_TAG_H_INCLUDE

#include <string>
#include <kvs/XMLDocument>
#include <kvs/XMLNode>
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <KVSML>.
 */
/*===========================================================================*/
class KVSMLTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

protected:

    bool m_has_version; ///< flag to check whether 'version' is specified or not
    std::string m_version; ///< KVSML version

public:

    KVSMLTag( void );

    virtual ~KVSMLTag( void );

public:

    const bool hasVersion( void ) const;

    const std::string& version( void ) const;

public:

    void setVersion( const std::string& version );

public:

    const bool read( const kvs::XMLDocument* document );

    const bool write( kvs::XMLDocument* document );

private:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__KVSML_TAG_H_INCLUDE
