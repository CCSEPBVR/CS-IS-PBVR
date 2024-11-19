/*****************************************************************************/
/**
 *  @file   ResizeEvent.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ResizeEvent.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__RESIZE_EVENT_H_INCLUDE
#define VIS_MODULE__RESIZE_EVENT_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/EventBase>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Resize event class.
 */
/*===========================================================================*/
class ResizeEvent : public vismodule::EventBase
{
    visModuleClassName( vismodule::ResizeEvent );

protected:

    int m_width;  ///< window width
    int m_height; ///< window height

public:

    ResizeEvent( void );

    ResizeEvent( const ResizeEvent& event );

    ResizeEvent( int width, int height );

    virtual ~ResizeEvent( void );

public:

    void setSize( int width, int height );

    const int width( void ) const;

    const int height( void ) const;

    virtual const int type( void ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__RESIZE_EVENT_H_INCLUDE
