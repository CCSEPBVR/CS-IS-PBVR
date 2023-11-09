/*****************************************************************************/
/**
 *  @file   Screen.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#ifndef KVS__OPENCABIN__SCREEN_H_INCLUDE
#define KVS__OPENCABIN__SCREEN_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/ScreenBase>
#include <kvs/EventHandler>
#include <kvs/InitializeEventListener>
#include <kvs/IdleEventListener>
#include <kvs/PaintEventListener>


namespace kvs
{

namespace opencabin
{

class Application;

/*===========================================================================*/
/**
 *  @brief  OpenCABIN screen class.
 */
/*===========================================================================*/
class Screen : public kvs::ScreenBase
{
    kvsClassName( kvs::opencabin::Screen );

private:

    typedef kvs::ScreenBase BaseClass;
    typedef void (Screen::*PaintEventFunction)( void );

protected:

    PaintEventFunction m_paint_event_func; ///< paint event function
    kvs::EventHandler* m_initialize_event_handler; ///< initialize event handler
    kvs::EventHandler* m_idle_event_handler; ///< idle event handler

public:

    Screen( kvs::opencabin::Application* application = 0 );

    virtual ~Screen( void );

public:

    std::string name( void ) const;

    int show( void );

public:

    virtual void redraw( void );

public:

    virtual void initializeEvent( void );

    virtual void idleEvent( void );

    virtual void paintEvent( void );

public:

    void setPaintEvent( kvs::PaintEventListener* event );

    void addInitializeEvent( kvs::InitializeEventListener* event );

    void addIdleEvent( kvs::IdleEventListener* event );

    void addPaintEvent( kvs::PaintEventListener* event );

protected:

    void default_paint_event( void );

private:

    /* Users cannot used the following methods because the screen geometory information
     * are specified via the OpenCABIN configuration file.
     */

    void setPosition( const int x, const int y );

    void setSize( const int width, const int height );

    void setGeometry( const int x, const int y, const int width, const int height );

public:

    void initializeTrackpad( void );

    void updateTrackpad( void );

protected:

    /* Ingored methods.
     */

    virtual void createWindow( void ) {};

    virtual void showFullScreen( void ) {};

    virtual void showNormal( void ) {};

    virtual void popUp( void ) {};

    virtual void pushDown( void ) {};

    virtual void hide( void ) {};

    virtual void showWindow( void ) {};
};

} // end of namespace opencabin

} // end of namespace kvs

#endif // KVS__OPENCABIN__SCREEN_H_INCLUDE
