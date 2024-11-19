/*****************************************************************************/
/**
 *  @file   Acceptor.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Acceptor.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__ACCEPTOR_H_INCLUDE
#define VIS_MODULE__ACCEPTOR_H_INCLUDE

#include <vismodule/TCPServer>
#include <vismodule/IPAddress>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Acceptor class.
 */
/*===========================================================================*/
class Acceptor
{
    visModuleClassName_without_virtual( vismodule::Acceptor );

private:

    vismodule::TCPServer* m_handler; //< handler

public:

    Acceptor( void );

    Acceptor( const int port, const size_t ntrials );

    ~Acceptor( void );

public:

    const bool open( void );

    void close( void );

    const bool bind( const int port, const size_t ntrials );

    vismodule::TCPSocket* newConnection( void );

    const int receive( vismodule::MessageBlock* block, vismodule::SocketAddress* client_address = 0 );

private:

    const bool create_handler( void );

    void delete_handler( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__ACCEPTOR_H_INCLUDE
