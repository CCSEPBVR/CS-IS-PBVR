/****************************************************************************/
/**
 *  @file XMLDeclaration.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XMLDeclaration.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "XMLDeclaration.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
XMLDeclaration::XMLDeclaration( void )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param version [in] version
 *  @param encoding [in] encoding
 *  @param standalone [in] stand alone
 */
/*==========================================================================*/
XMLDeclaration::XMLDeclaration(
    const std::string& version,
    const std::string& encoding,
    const std::string& standalone ):
    TiXmlDeclaration( version, encoding, standalone )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
XMLDeclaration::~XMLDeclaration( void )
{
}

} // end of namespace kvs
