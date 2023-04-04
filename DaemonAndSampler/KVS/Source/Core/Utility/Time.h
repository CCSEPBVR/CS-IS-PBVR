/****************************************************************************/
/**
 *  @file Time.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Time.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__TIME_H_INCLUDE
#define KVS__TIME_H_INCLUDE

#include <iostream>
#include <string>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Time class.
 */
/*===========================================================================*/
class Time
{
    kvsClassName( kvs::Time );

protected:

    int  m_hour;          ///< hour
    int  m_minute;        ///< minute
    int  m_second;        ///< second
    long m_total_seconds; ///< total seconds

public:

    Time( void );

    Time( const int hour, const int minute, const int second );

    virtual ~Time( void );

public:

    Time& operator = ( const Time& time );

    Time& operator += ( const int seconds );

    Time& operator -= ( const int seconds );

    friend Time operator + ( const Time& time, const int seconds );

    friend Time operator - ( const Time& time, const int seconds );

    friend int operator - ( const Time& time1, const Time& time2 );

    friend bool operator > ( const Time& time1, const Time& time2 );

    friend bool operator >= ( const Time& time1, const Time& time2 );

    friend bool operator < ( const Time& time1, const Time& time2 );

    friend bool operator <= ( const Time& time1, const Time& time2 );

    friend bool operator == ( const Time& time1, const Time& time2 );

    friend bool operator != ( const Time& time1, const Time& time2 );

    friend std::ostream& operator << ( std::ostream& os, const Time& time );

public:

    const int hour( void ) const;

    const int minute( void ) const;

    const int second( void ) const;

    const long totalSeconds( void ) const;

public:

    const Time& now( void );

public:

    const std::string toString( const std::string sep = ":" ) const;

    void fromString( const std::string time, const std::string sep = ":" );

public:

    void addHours( const int hour );

    void subtractHours( const int hour );

    void addMinutes( const int minute );

    void subtractMinutes( const int minute );

    void addSeconds( const int second );

    void subtractSeconds( const int second );

protected:

    const long convert_to_seconds( const int hour, const int minute, const int second );

    const Time convert_from_seconds( const int seconds );
};

} // end of namespace kvs

#endif // KVS__TIME_H_INCLUDE
