/****************************************************************************/
/**
 *  @file SocketSelector.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SocketSelector.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__SOCKET_SELECTOR_H_INCLUDE
#define VIS_MODULE__SOCKET_SELECTOR_H_INCLUDE

#include "Socket.h"
#include "SocketTimer.h"
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Socket select class.
 */
/*==========================================================================*/
class SocketSelector
{
    visModuleClassName( vismodule::SocketSelector );

protected:

    fd_set m_readable; ///< readable descriptor
    fd_set m_writable; ///< writable descriptor

public:

    SocketSelector( void );

    SocketSelector( const SocketSelector& other );

    virtual ~SocketSelector( void );

public:

    void setReadable( const vismodule::Socket::id_type& socket_id );

    void setWritable( const vismodule::Socket::id_type& socket_id );

    bool isReadable( const vismodule::Socket::id_type& socket_id );

    bool isWritable( const vismodule::Socket::id_type& socket_id );

    void clearReadable( const vismodule::Socket::id_type& socket_id );

    void clearWritable( const vismodule::Socket::id_type& socket_id );

    void clear( void );

    int select( const vismodule::SocketTimer& timeout );
};

} // end of namespace vismodule

#endif // VIS_MODULE__SOCKET_SELECTOR_H_INCLUDE
