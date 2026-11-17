/*****************************************************************************/
/**
 *  @file   Connector.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Connector.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__CONNECTOR_H_INCLUDE
#define VIS_MODULE__CONNECTOR_H_INCLUDE

#include <vismodule/TCPSocket>
#include <vismodule/IPAddress>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Connector class.
 */
/*===========================================================================*/
class Connector
{
    visModuleClassName_without_virtual( vismodule::Connector );

private:

    vismodule::TCPSocket*  m_handler; //< handler
    vismodule::IPAddress   m_ip;      //< IP address
    int              m_port;    //< port number
    std::size_t           m_ntrials; //< number of trials

public:

    Connector( void );

    Connector( vismodule::TCPSocket* handler );

    Connector( const vismodule::IPAddress& ip, const int port, const std::size_t ntrials );

    ~Connector( void );

public:

    const bool open( void );

    void close( void );

    const bool connect( const vismodule::IPAddress& ip, const int port, const std::size_t ntrials );

    const bool reconnect();

    void send( const vismodule::MessageBlock& block );

private:

    const bool create_handler( void );

    void delete_handler( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__CONNECTOR_H_INCLUDE
