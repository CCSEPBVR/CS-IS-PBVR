/****************************************************************************/
/**
 *  @file SocketTimer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SocketTimer.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "SocketTimer.h"


namespace kvs
{

const SocketTimer SocketTimer::Zero;

SocketTimer::SocketTimer( const double seconds )
{
    this->setValue( seconds );
}

const SocketTimer& SocketTimer::operator = ( const SocketTimer& other )
{
    m_value.tv_sec  = other.m_value.tv_sec;
    m_value.tv_usec = other.m_value.tv_usec;

    return( *this );
}

bool operator == ( const SocketTimer& other1, const SocketTimer& other2 )
{
    return( other1.m_value.tv_sec  == other2.m_value.tv_sec &&
            other1.m_value.tv_usec == other2.m_value.tv_usec );
}

const struct timeval& SocketTimer::value( void ) const
{
    return( m_value );
}

struct timeval& SocketTimer::value( void )
{
    return( m_value );
}

void SocketTimer::setValue( double seconds )
{
    m_value.tv_sec  = static_cast<long>( seconds );
    m_value.tv_usec = static_cast<long>( ( seconds - m_value.tv_sec ) * 1000000 );
}

bool SocketTimer::isZero( void ) const
{
    return( m_value.tv_sec == 0 && m_value.tv_usec == 0 );
}

} // end of namespace kvs
