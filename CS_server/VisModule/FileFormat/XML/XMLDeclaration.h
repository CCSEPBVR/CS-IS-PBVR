/****************************************************************************/
/**
 *  @file XMLDeclaration.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XMLDeclaration.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__XML_DECLARATION_H_INCLUDE
#define VIS_MODULE__XML_DECLARATION_H_INCLUDE

#include "TinyXML.h"
#include <string>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  XML declaration class.
 */
/*==========================================================================*/
class XMLDeclaration : public TiXmlDeclaration
{
    visModuleClassName( vismodule::XMLDeclaration );

public:

    typedef TiXmlDeclaration SuperClass;

public:

    XMLDeclaration( void );

    XMLDeclaration(
        const std::string& version,
        const std::string& encoding = "",
        const std::string& standalone = "" );

    virtual ~XMLDeclaration( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__XML_DECLARATION_H_INCLUDE
