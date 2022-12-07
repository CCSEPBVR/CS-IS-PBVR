/*****************************************************************************/
/**
 *  @file   Window.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Window.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__QT__WINDOW_H_INCLUDE
#define KVS__QT__WINDOW_H_INCLUDE

#include <kvs/WindowBase>
#include <kvs/qt/Qt>
#include <kvs/ClassName>


namespace kvs
{

namespace qt
{

/*===========================================================================*/
/**
 *  @brief  Qt window class.
 */
/*===========================================================================*/
class Window : public QGLWidget , public kvs::WindowBase
{
    kvsClassName( kvs::qt::Window );

public:

    Window( QWidget* parent = 0 );

    virtual ~Window( void );

public:

    void setTitle( const std::string& title );

public:

    virtual void create( void );

    virtual void showFullScreen( void );

    virtual void showNormal( void );

    virtual void popUp( void );

    virtual void pushDown( void );

    virtual void hide( void );

    virtual void showWindow( void );

    virtual void redraw( void );

    virtual void resize( int width, int height );

public:

    virtual void initializeEvent( void );

    virtual void paintEvent( void );

    virtual void resizeEvent( int width, int height );

    virtual void mousePressEvent( kvs::MouseEvent* event );

    virtual void mouseMoveEvent( kvs::MouseEvent* event );

    virtual void mouseReleaseEvent( kvs::MouseEvent* event );

    virtual void mouseDoubleClickEvent( kvs::MouseEvent* event );

    virtual void wheelEvent( kvs::WheelEvent* event );

    virtual void keyPressEvent( kvs::KeyEvent* event );

public:

    // Callback functions for QGLWidget.

    virtual void initializeGL( void );

    virtual void paintGL( void );

    virtual void resizeGL( int width, int height );

    virtual void mousePressEvent( QMouseEvent* event );

    virtual void mouseMoveEvent( QMouseEvent* event );

    virtual void mouseReleaseEvent( QMouseEvent* event );

    virtual void mouseDoubleClickEvent( QMouseEvent* event );

    virtual void wheelEvent( QWheelEvent* event );

    virtual void keyPressEvent( QKeyEvent* event );
};

} // end of namespace qt

} // end of namespace kvs

#endif // KVS__QT__WINDOW_H_INCLUDE
