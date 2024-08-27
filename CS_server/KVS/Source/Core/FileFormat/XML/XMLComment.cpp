/****************************************************************************/
/**
 *  @file XMLComment.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XMLComment.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "XMLComment.h"

namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
XMLComment::XMLComment( void )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param comment [in] comments
 */
/*==========================================================================*/
XMLComment::XMLComment( const char* comment ):
    TiXmlComment()
{
    this->SetValue( comment );
}

/*==========================================================================*/
/**
 *  Desturctor.
 */
/*==========================================================================*/
XMLComment::~XMLComment( void )
{
}

} // end of namespace kvs
