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
#ifndef VIS_MODULE__XML_COMMENT_H_INCLUDE
#define VIS_MODULE__XML_COMMENT_H_INCLUDE

#include "TinyXML.h"
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  XML comment class.
 */
/*==========================================================================*/
class XMLComment : public TiXmlComment
{
    visModuleClassName( vismodule::XMLComment );

public:

    typedef TiXmlComment SuperClass;

public:

    XMLComment( void );

    XMLComment( const char* comment );

    virtual ~XMLComment( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__XML_COMMENT_H_INCLUDE
