/****************************************************************************/
/**
 *  @file FastTokenizer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FastTokenizer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__FAST_TOKENIZER_H_INCLUDE
#define KVS__FAST_TOKENIZER_H_INCLUDE

#include <cstring>
#include <kvs/ClassName>
#include <kvs/Assert>


namespace kvs
{

/*==========================================================================*/
/**
 *  FastTokenizer.
 */
/*==========================================================================*/
class FastTokenizer
{
    kvsClassName_without_virtual( kvs::FastTokenizer );

private:

    enum { BufferSize = 256 };

private:

    size_t      m_ntokens;
    const char* m_tokens[BufferSize];

public:

    FastTokenizer( void );

    FastTokenizer( char* const source, const char* const delimiter );

    ~FastTokenizer( void );

public:

    void set( char* const source, const char* const delimiter );

    const char* const get( size_t index ) const;

public:

    const size_t ntokens( void ) const;

public:

};

inline void FastTokenizer::set( char* const source, const char* const delimiter )
{
    size_t ntokens = 0;

    for ( const char* p = strtok( source, delimiter ); p != NULL; p = strtok( NULL, delimiter ) )
    {
        m_tokens[ntokens] = p;
        ++ntokens;

        KVS_ASSERT( ntokens < BufferSize );
    }

    m_ntokens = ntokens;
}

inline const char* const FastTokenizer::get( size_t index ) const
{
    KVS_ASSERT( index < m_ntokens );

    return( m_tokens[index] );
}

inline const size_t FastTokenizer::ntokens( void ) const
{
    return( m_ntokens );
}

} // end of namespace kvs

#endif // KVS__FAST_TOKENIZER_H_INCLUDE
