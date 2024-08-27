/****************************************************************************/
/**
 *  @file Window.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Window.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__DCM__WINDOW_H_INCLUDE
#define KVS__DCM__WINDOW_H_INCLUDE


namespace kvs
{

namespace dcm
{

/*===========================================================================*/
/**
 *  @brief  DICOM window information class.
 */
/*===========================================================================*/
class Window
{
protected:

    unsigned short m_bits;      ///< number of bits
    bool           m_sign;      ///< true: signed, false: unsigned
    int            m_min_value; ///< maximum value of the window's range
    int            m_max_value; ///< minimum value of the window's range
    int            m_range;     ///< range of the window
    int            m_level;     ///< current window level
    int            m_width;     ///< current window width

public:

    Window( void );

    Window( const unsigned short bits, const bool sign );

public:

    Window& operator = ( const Window& w );

public:

    void set( const unsigned short bits, const bool sign );

    void rescale( const double slope, const double intersept );

    void setLevel( const int level );

    void setWidth( const int width );

public:

    const int minValue( void ) const;

    const int maxValue( void ) const;

    const int level( void ) const;

    const int width( void ) const;

public:

    const int clampLevel( const int level );

    const int clampWidth( const int width );

protected:

    void set_range_8bit( const bool sign );

    void set_range_12bit( const bool sign );

    void set_range_16bit( const bool sign );
};

} // end of namespace dcm

} // end of namespace kvs

#endif // KVS__DCM__WINDOW_H_INCLUDE
