/****************************************************************************/
/**
 *  @file Date.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Date.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__DATE_H_INCLUDE
#define KVS__DATE_H_INCLUDE

#include <iostream>
#include <string>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Date class.
 */
/*===========================================================================*/
class Date
{
    kvsClassName( kvs::Date );

public:

    enum DayOfWeek
    {
        UnknownDay   = -1,
        Sun          =  0,
        Mon          =  1,
        Tue          =  2,
        Wed          =  3,
        Thu          =  4,
        Fri          =  5,
        Sat          =  6,
        NumberOfDays =  7
    };

    enum MonthOfYear
    {
        UnknownMonth   = -1,
        Jan            =  1,
        Feb            =  2,
        Mar            =  3,
        Apr            =  4,
        May            =  5,
        Jun            =  6,
        Jul            =  7,
        Aug            =  8,
        Sep            =  9,
        Oct            = 10,
        Nov            = 11,
        Dec            = 12,
        NumberOfMonths = 12
    };

protected:

    int  m_year;       ///< year
    int  m_month;      ///< month
    int  m_day;        ///< day
    long m_julian_day; ///< julian day

public:

    Date( void );

    Date( const long julian_day );

    Date( const int year, const int month, const int day );

    Date( const Date& date );

    virtual ~Date( void );

public:

    Date& operator = ( const Date& date );

    Date& operator += ( const int days );

    Date& operator -= ( const int days );

    friend Date operator + ( const Date& date, const int days );

    friend Date operator - ( const Date& date, const int days );

    friend int operator - ( const Date& date1, const Date& date2 );

    friend bool operator > ( const Date& date1, const Date& date2 );

    friend bool operator >= ( const Date& date1, const Date& date2 );

    friend bool operator < ( const Date& date1, const Date& date2 );

    friend bool operator <= ( const Date& date1, const Date& date2 );

    friend bool operator == ( const Date& date1, const Date& date2 );

    friend bool operator != ( const Date& date1, const Date& date2 );

    friend std::ostream& operator << ( std::ostream& os, const Date& date );

public:

    const int year( void ) const;

    const int month( void ) const;

    const int day( void ) const;

    const long julianDay( void ) const;

    const std::string monthString( const bool abbrevition = true ) const;

public:

    const Date& today( void );

    const int yearsOld( void ) const;

public:

    const DayOfWeek dayOfWeek( void ) const;

    const std::string dayOfWeekString( const bool abbr = true ) const;

    const int daysInMonth( void ) const;

    const std::string toString( const std::string sep = "" ) const;

    void fromString( const std::string date, const std::string sep = "" );

public:

    const bool isLeepYear( void ) const;

    const bool isValid( void ) const;

public:

    void addYears( const int year );

    void subtractYears( const int year );

    void addMonths( const int month );

    void subtractMonths( const int month );

    void addDays( const int days );

    void subtractDays( const int days );

protected:

    void adjust_days( void );

    const long convert_to_julian_date( const int year, const int month, const int day ) const;

    const Date convert_from_julian_date( const long julian_day ) const;
};

} // end of namespace kvs

#endif // KVS__DATE_H_INCLUDE
