/****************************************************************************/
/**
 *  @file Tokenizer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Tokenizer.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Tokenizer.h"
#include <string>
#include <algorithm>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
Tokenizer::Tokenizer( void ):
    m_first( std::string::npos ),
    m_last( std::string::npos )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param source [in] source
 *  @param delimiter [in] delimiters
 */
/*==========================================================================*/
Tokenizer::Tokenizer( const std::string& source, const std::string& delimiter ):
    m_source( source ),
    m_delimiter( delimiter )
{
    m_last = 0;
    this->get_token();
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param t [in] tokenizer
 */
/*==========================================================================*/
Tokenizer::Tokenizer( const Tokenizer& t ):
    m_source( t.m_source ),
    m_delimiter( t.m_delimiter ),
    m_token( t.m_token ),
    m_first( t.m_first ),
    m_last( t.m_last )
{
}

/*==========================================================================*/
/**
 *  Substitution operator
 *  @param t [in] tokenizer
 */
/*==========================================================================*/
Tokenizer& Tokenizer::operator = ( const Tokenizer& t )
{
    m_source    = t.m_source;
    m_delimiter = t.m_delimiter;
    m_token     = t.m_token;
    m_first     = t.m_first;
    m_last      = t.m_last;

    return *this;
}

/*==========================================================================*/
/**
 *  Substitution oeprator.
 *  @param source [in] source
 */
/*==========================================================================*/
Tokenizer& Tokenizer::operator = ( const std::string& source )
{
    m_source = source;
    m_last   = 0;
    this->get_token();

    return( *this );
}

/*==========================================================================*/
/**
 *  Test method whether the token is last.
 *  @return true, if the node is last
 */
/*==========================================================================*/
const bool Tokenizer::isLast( void ) const
{
    return( m_first == std::string::npos && m_last == std::string::npos );
}

/*==========================================================================*/
/**
 *  Get the delimitted token.
 *  @return token
 */
/*==========================================================================*/
const std::string Tokenizer::token( void )
{
    std::string ret = m_token;
    this->get_token();

    return( ret );
}

/*==========================================================================*/
/**
 *  Get the delimitted current token.
 */
/*==========================================================================*/
void Tokenizer::get_token( void )
{
    if( m_last == std::string::npos )
    {
        m_first = std::string::npos;
        return;
    }

    m_first = m_source.find_first_not_of( m_delimiter, m_last );
    if( m_first == std::string::npos )
    {
        m_last = std::string::npos;
        return;
    }

    m_last = m_source.find_first_of( m_delimiter, m_first );
    std::string::size_type length = ( m_last == std::string::npos ) ?
        m_last :
        m_last - m_first;

    m_token = m_source.substr( m_first, length );
}

} // end of namespace kvs
