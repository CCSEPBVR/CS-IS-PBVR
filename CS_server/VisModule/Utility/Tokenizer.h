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
#ifndef VIS_MODULE__TOKENIZER_H_INCLUDE
#define VIS_MODULE__TOKENIZER_H_INCLUDE

#include <string>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Delimit tokens.
 */
/*==========================================================================*/
class Tokenizer
{
    visModuleClassName_without_virtual( vismodule::Tokenizer );

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

} // end of namespace vismodule

#endif // VIS_MODULE__TOKENIZER_H_INCLUDE
