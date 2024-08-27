/*****************************************************************************/
/**
 *  @file   Trackpad.cpp
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
#include "Trackpad.h"
#include <kvs/Math>
#include <kvs/MouseMoveEventListener>
#include <kvs/opencabin/Application>
#include <kvs/opencabin/Configuration>
#include <kvs/glut/GLUT>


namespace kvs
{

namespace opencabin
{

Trackpad::Trackpad( kvs::opencabin::Application* application )
{
    if ( application ) application->attach_trackpad( this );

    if ( kvs::opencabin::Application::IsMaster() )
    {
        const kvs::Vector2i resolution = kvs::opencabin::Configuration::KVSApplication::DisplayResolution();
        const int min = kvs::Math::Min( resolution.x(), resolution.y() );
        const int base = 256;
        const int width = base * resolution.x() / min;
        const int height = base * resolution.y() / min;
        const std::string title( "Trackpad" );
        const kvs::RGBColor color( 200, 200, 200 );

        const float scale_width = static_cast<float>(width) / resolution.x();
        const float scale_height = static_cast<float>(height) / resolution.y();
        m_screen_scale = kvs::Vector2f( scale_width, scale_height );

        BaseClass::setTitle( title );
        BaseClass::setSize( width, height );
//        BaseClass::setSize( resolution.x(), resolution.y() );
        BaseClass::background()->setColor( color );

        BaseClass::m_mouse_press_event_func = (void (kvs::glut::Screen::*)(kvs::MouseEvent*))(&Trackpad::default_mouse_press_event);
        BaseClass::m_mouse_move_event_func = (void (kvs::glut::Screen::*)(kvs::MouseEvent*))(&Trackpad::default_mouse_move_event);
        BaseClass::m_mouse_release_event_func = (void (kvs::glut::Screen::*)(kvs::MouseEvent*))(&Trackpad::default_mouse_release_event);
    }
}

int Trackpad::show( void )
{
    if ( kvs::opencabin::Application::IsMaster() )
    {
        return( BaseClass::show() );
    }

    return( 0 );
}

/*==========================================================================*/
/**
 *  @brief  Updates the xform of the control target.
 */
/*==========================================================================*/
void Trackpad::updateXform( void )
{
    if ( kvs::opencabin::Application::IsMaster() )
    {
        float* scaling = kvs::opencabin::Application::GetTrackpadScaling();
        scaling[0] = m_mouse->scaling()[0];
        scaling[1] = m_mouse->scaling()[1];
        scaling[2] = m_mouse->scaling()[2];

        float* translation = kvs::opencabin::Application::GetTrackpadTranslation();
        translation[0] = m_mouse->translation()[0];
        translation[1] = m_mouse->translation()[1];
        translation[2] = m_mouse->translation()[2];

        float* rotation = kvs::opencabin::Application::GetTrackpadRotation();
        rotation[0] = m_mouse->rotation()[0];
        rotation[1] = m_mouse->rotation()[1];
        rotation[2] = m_mouse->rotation()[2];
        rotation[3] = m_mouse->rotation()[0];

        BaseClass::updateXform();
    }
}

void Trackpad::default_mouse_press_event( kvs::MouseEvent* event )
{
//    const int x = kvs::Math::Round( event->x() / m_screen_scale.x() );
//    const int y = kvs::Math::Round( event->y() / m_screen_scale.y() );
//    event->setPosition( x, y );

    BaseClass::default_mouse_press_event( event );
}

void Trackpad::default_mouse_move_event( kvs::MouseEvent* event )
{
//    const int x = kvs::Math::Round( event->x() / m_screen_scale.x() );
//    const int y = kvs::Math::Round( event->y() / m_screen_scale.y() );
//    event->setPosition( x, y );

    BaseClass::default_mouse_move_event( event );
}

void Trackpad::default_mouse_release_event( kvs::MouseEvent* event )
{
//    const int x = kvs::Math::Round( event->x() / m_screen_scale.x() );
//    const int y = kvs::Math::Round( event->y() / m_screen_scale.y() );
//    event->setPosition( x, y );

    BaseClass::default_mouse_release_event( event );
}

} // end of namespace opencabin

} // end of namespace kvs
