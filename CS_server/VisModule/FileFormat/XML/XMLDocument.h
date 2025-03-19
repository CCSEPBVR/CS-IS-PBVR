/****************************************************************************/
/**
 *  @file XMLDocument.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XMLDocument.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__XML_DOCUMENT_H_INCLUDE
#define VIS_MODULE__XML_DOCUMENT_H_INCLUDE

#include "TinyXML.h"
#include <string>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  XML document class.
 */
/*==========================================================================*/
class XMLDocument : public TiXmlDocument
{
    visModuleClassName( vismodule::XMLDocument );

public:

    typedef TiXmlDocument SuperClass;

protected:

    std::string m_filename; ///< filename

public:

    XMLDocument( void );

    XMLDocument( const std::string& filename );

public:

    const std::string& filename( void ) const;

public:

    const char* parse( const char* text );

    void print( void );

public:

    TiXmlNode* insert( const TiXmlNode& node );

public:

    bool read( const std::string& filename );

    bool write( const std::string& filename );

public:

    static TiXmlNode* FindNode( const vismodule::XMLDocument* document, const std::string& node_name );
};

} // end of namespace vismodule

#endif // VIS_MODULE__XML_DOCUMENT_H_INCLUDE
