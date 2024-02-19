/****************************************************************************/
/**
 *  @file Url.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Url.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__URL_H_INCLUDE
#define KVS__URL_H_INCLUDE

#include <iostream>
#include <string>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  URL class.
 */
/*==========================================================================*/
class Url
{
    kvsClassName_without_virtual( kvs::Url );

protected:

    std::string m_protocol; ///< protocol (e.g. "http", "ftp")
    std::string m_username; ///< user name
    std::string m_password; ///< password
    std::string m_hostname; ///< hostname
    int         m_port;     ///< port number
    std::string m_path;     ///< path

public:

    Url( void );

    Url( const std::string& url );

public:

    friend std::ostream& operator << ( std::ostream& os, const Url& other );

public:

    const std::string& protocol( void ) const;

    const std::string& username( void ) const;

    const std::string& password( void ) const;

    const std::string& hostname( void ) const;

    int port( void ) const;

    const std::string& path( void ) const;

private:

    void parse_url( const std::string& url );

    void split_protocol( std::string url );

    void split_authority( std::string sub_url );

    void split_user_info( std::string sub_url );

    void split_hostname( std::string sub_url );
};

} // end of namespace kvs

#endif // KVS__URL_H_INCLUDE
