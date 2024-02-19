/****************************************************************************/
/**
 *  @file Tokenizer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Tokenizer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__TOKENIZER_H_INCLUDE
#define KVS__TOKENIZER_H_INCLUDE

#include <string>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  Delimit tokens.
 */
/*==========================================================================*/
class Tokenizer
{
    kvsClassName_without_virtual( kvs::Tokenizer );

protected:

    std::string            m_source;    ///< source std::string
    std::string            m_delimiter; ///< delimiter set
    std::string            m_token;     ///< token
    std::string::size_type m_first;     ///< first position
    std::string::size_type m_last;      ///< last position

public:

    Tokenizer( void );

    Tokenizer( const std::string& source, const std::string& delimiter );

    Tokenizer( const Tokenizer& t );

public:

    Tokenizer& operator = ( const Tokenizer& t );

    Tokenizer& operator = ( const std::string& source );

public:

    const bool isLast( void ) const;

    const std::string token( void );

protected:

    void get_token( void );
};

} // end of namespace kvs

#endif // KVS__TOKENIZER_H_INCLUDE
