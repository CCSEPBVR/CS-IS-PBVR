/****************************************************************************/
/**
 *  @file HttpRequestHeader.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: HttpRequestHeader.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__HTTP_REQUEST_HEADER_H_INCLUDE
#define KVS__HTTP_REQUEST_HEADER_H_INCLUDE

#include <string>
#include <vector>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  HTTP request header class.
 */
/*==========================================================================*/
class HttpRequestHeader
{
    kvsClassName( kvs::HttpRequestHeader );

public:

    typedef std::vector<std::string> FieldList;

protected:

    std::string m_method;        ///< method
    std::string m_path;          ///< path
    int         m_major_version; ///< HTTP major version
    int         m_minor_version; ///< HTTP minor version
    FieldList   m_field_list;    ///< field (key+value) list

public:

    HttpRequestHeader( void );

    HttpRequestHeader(
        const std::string& method,
        const std::string& path,
        const int          major_version = 1,
        const int          minor_version = 0 );

public:

    const std::string& method( void ) const;

    const std::string& path( void ) const;

    const int majorVersion( void ) const;

    const int minorVersion( void ) const;

public:

    void set(
        const std::string& method,
        const std::string& path,
        const int          major_version = 1,
        const int          minor_version = 0 );

    void addField( const std::string& key, const std::string& value );

    std::string makeMessage( void ) const;
};

} // end of namespace kvs

#endif // KVS__HTTP_REQUEST_HEADER_H_INCLUDE
