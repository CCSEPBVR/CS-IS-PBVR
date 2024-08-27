/*****************************************************************************/
/**
 *  @file   Rectangle.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Rectangle.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLUT__RECTANGLE_H_INCLUDE
#define KVS__GLUT__RECTANGLE_H_INCLUDE


#include <kvs/ClassName>


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Rectangle class.
 */
/*===========================================================================*/
class Rectangle
{
    // Class name.
    kvsClassName( kvs::glut::Rectangle );

protected:

    int  m_x; ///< x position of widget on the window coordinate (org: upper-left)
    int  m_y; ///< y position of widet on the window coordinate (org: upper-left)
    int  m_width; ///< widget width
    int  m_height; ///< widget height
    bool m_is_active; ///< check flag for widget activation

public:

    Rectangle( void );

    Rectangle( const int width, const int height );

    Rectangle( const int x, const int y, const int width, const int height );

    virtual ~Rectangle( void );

public:

    const int x( void ) const;

    const int y( void ) const;

    const int width( void ) const;

    const int height( void ) const;

    const int x0( void ) const;

    const int y0( void ) const;

    const int x1( void ) const;

    const int y1( void ) const;

    const bool isActive( void ) const;

public:

    void setX( const int x );

    void setY( const int y );

    void setWidth( const int width );

    void setHeight( const int height );

    void setPosition( const int x, const int y );

    void setSize( const int width, const int height );

    void setGeometry( const int x, const int y, const int width, const int height );

public:

    void activate( void );

    void deactivate( void );

    const bool contains( const int x, const int y, const bool proper = false );
};

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__RECTANGLE_H_INCLUDE
