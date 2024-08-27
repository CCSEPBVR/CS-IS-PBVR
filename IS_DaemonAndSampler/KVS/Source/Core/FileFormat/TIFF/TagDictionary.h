/****************************************************************************/
/**
 *  @file TagDictionary.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TagDictionary.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__TIFF__TAG_DICTIONARY_H_INCLUDE
#define KVS__TIFF__TAG_DICTIONARY_H_INCLUDE

#include <kvs/Type>
#include <map>
#include "Tag.h"


namespace kvs
{

namespace tiff
{

class TagDictionary
{
public:

    typedef std::map<kvs::UInt16,std::string> Container;

protected:

    Container m_container;

public:

    TagDictionary( void );

public:

    kvs::tiff::Tag find( const kvs::UInt16 tag_id ) const;

private:

    void create_dictionary( void );
};

} // end of namespace tiff

} // end of namespace kvs

#endif // KVS__TIFF__TAG_DICTIONARY_H_INCLUDE
