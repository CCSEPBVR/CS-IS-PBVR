/****************************************************************************/
/**
 *  @file FastTokenizer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FastTokenizer.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "FastTokenizer.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructs a new empty FastTokenizer.
 */
/*==========================================================================*/
FastTokenizer::FastTokenizer( void )
    : m_ntokens( 0 )
{
}

/*==========================================================================*/
/**
 *  Constructs a new FastTokenizer.
 */
/*==========================================================================*/
FastTokenizer::FastTokenizer( char* const source, const char* const delimiter )
    : m_ntokens( 0 )
{
    this->set( source, delimiter );
}

/*==========================================================================*/
/**
 *  Destroys the FastTokenizer.
 */
/*==========================================================================*/
FastTokenizer::~FastTokenizer( void )
{
}

} // end of namespace test
