/****************************************************************************/
/**
 *  @file String.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: String.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__STRING_H_INCLUDE
#define KVS__STRING_H_INCLUDE

#include <cstdio>
#include <iostream>
#include <string>
#include <cstdarg>
#include <sstream>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  String class.
 */
/*==========================================================================*/
class String
{
    kvsClassName_without_virtual( kvs::String );

private:

    char*  m_data; ///< string data
    size_t m_size; ///< string size (not include '\0')

public:

    String( void );

    String( const String& str );

    explicit String( const std::string& str );

    template<typename T>
    explicit String( T number );

    ~String( void );

public:

    char& operator []( size_t index );

    const char operator []( size_t index ) const;

    String& operator =( const std::string& str );

    String& operator =( const String& str );

    String& operator +=( const std::string& str );

    String& operator +=( const String& str );

    const bool operator ==( const String& str );

    const bool operator !=( const String& str );

    const bool operator <( const String& str );

    const bool operator >( const String& str );

    const bool operator <=( const String& str );

    const bool operator >=( const String& str );

    friend const String operator +( const String& str1, const String& str2 );

    friend std::ostream& operator <<( std::ostream& os, const String& str );

public:

    char* data( void );

    const char* data( void ) const;

    const size_t size( void ) const;

    char& at( size_t index );

    const char at( size_t index ) const;

    void upper( void );

    void upper( size_t index );

    void lower( void );

    void lower( size_t index );

    void format( const char* str, ... );

    void replace( const std::string& pattern, const std::string& placement );

    template<typename T>
    void setNumber( T number );

    template<typename T>
    const T toNumber( int base = 10 ) const;

    template<typename T>
    static const T toNumber( const std::string& str, int base = 10 );

    const std::string toStdString( void ) const;
};

template<typename T>
String::String( T number )
{
    this->setNumber<T>( number );
}

template<typename T>
void String::setNumber( T number )
{
    std::stringstream s;
    s << number;

    *this = s.str();
}

template<typename T>
const T String::toNumber( const std::string& str, int base )
{
    return( String( str ).toNumber<T>( base ) );
}

} // end of namespace kvs

#endif // KVS__STRING_H_INCLUDE
