/****************************************************************************/
/**
 *  @file XMLComment.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XMLComment.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__XML_COMMENT_H_INCLUDE
#define KVS__XML_COMMENT_H_INCLUDE

#include "TinyXML.h"
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  XML comment class.
 */
/*==========================================================================*/
class XMLComment : public TiXmlComment
{
    kvsClassName( kvs::XMLComment );

public:

    typedef TiXmlComment SuperClass;

public:

    XMLComment( void );

    XMLComment( const char* comment );

    virtual ~XMLComment( void );
};

} // end of namespace kvs

#endif // KVS__XML_COMMENT_H_INCLUDE
