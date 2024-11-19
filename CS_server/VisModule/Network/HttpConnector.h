/****************************************************************************/
/**
 *  @file HttpConnector.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: HttpConnector.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__HTTP_CONNECTOR_H_INCLUDE
#define VIS_MODULE__HTTP_CONNECTOR_H_INCLUDE

#include "TCPSocket.h"
#include "Url.h"
#include "SocketTimer.h"
#include <iostream>
#include <fstream>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  HTTP connector class.
 */
/*==========================================================================*/
class HttpConnector
{
    visModuleClassName( vismodule::HttpConnector );

protected:

    vismodule::TCPSocket m_connector; ///< TCP socket connector
    std::string    m_hostname;  ///< hostname
    int            m_port;      ///< port number

public:

    HttpConnector( void );

    HttpConnector( const vismodule::Url& url, const vismodule::SocketTimer* timeout = 0 );

    virtual ~HttpConnector( void );

public:

    bool connect( const vismodule::Url& url, const vismodule::SocketTimer* timeout = 0 );

    void close( void );

    bool get( const std::string& path, const std::string& filename );

    bool get( const std::string& path, std::ostream& output_stream );
};

} // end of namespace vismodule

#endif // VIS_MODULE__HTTP_CONNECTOR_H_INCLUDE
