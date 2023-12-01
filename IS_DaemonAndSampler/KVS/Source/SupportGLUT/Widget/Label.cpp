/*****************************************************************************/
/**
 *  @file   Label.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Label.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Label.h"
#include <kvs/OpenGL>
#include <cstdio>
#include <cstdarg>
#include <kvs/IgnoreUnusedVariable>

// Default parameters.
namespace { namespace Default
{
const size_t CharacterWidth = 8;
const size_t CharacterHeight = 12;
const size_t MaxLineLength = 255;
} }


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Label class.
 *  @param  screen [in] pointer to the parent screen
 */
/*===========================================================================*/
Label::Label( kvs::ScreenBase* screen ):
    kvs::glut::WidgetBase( screen )
{
    BaseClass::setEventType(
        kvs::EventBase::PaintEvent |
        kvs::EventBase::ResizeEvent );
}

/*==========================================================================*/
/**
 *  @brief Sets text string.
 *  @param text [in] text string
 */
/*==========================================================================*/
void Label::setText( const char* text, ... )
{
    m_text.clear();

    char buffer[ ::Default::MaxLineLength ];

    va_list args;
    va_start( args, text );
    vsprintf( buffer, text, args );
    va_end( args );

    m_text.push_back( std::string( buffer ) );
}

/*==========================================================================*/
/**
 *  @brief Add the text string.
 *  @param text [in] text string
 */
/*==========================================================================*/
void Label::addText( const char* text, ... )
{
    char buffer[ ::Default::MaxLineLength ];

    va_list args;
    va_start( args, text );
    vsprintf( buffer, text, args );
    va_end( args );

    m_text.push_back( std::string( buffer ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the fitted width.
 *  @return fitted width
 */
/*===========================================================================*/
int Label::get_fitted_width( void )
{
    size_t width = 0;

    for ( size_t i = 0; i < m_text.size(); i++ )
    {
        width = kvs::Math::Max( width, m_text[i].size() );
    }

    return( width * ::Default::CharacterWidth + BaseClass::margin() * 2 );
}

/*===========================================================================*/
/**
 *  @brief  Returns the fitted height.
 *  @return fitted height
 */
/*===========================================================================*/
int Label::get_fitted_height( void )
{
    return( m_text.size() * ::Default::CharacterHeight + BaseClass::margin() * 2 );
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
void Label::paintEvent( void )
{
    this->screenUpdated();

    if ( !BaseClass::isShown() ) return;

    BaseClass::begin_draw();
    BaseClass::draw_background();

    const int x = BaseClass::x() + BaseClass::margin();
    const int y = BaseClass::y() + BaseClass::margin();
    for ( size_t line = 0; line < m_text.size(); line++ )
    {
        BaseClass::draw_text( x, y + ::Default::CharacterHeight * ( line + 1 ), m_text[line] );
    }

    BaseClass::end_draw();
}

/*===========================================================================*/
/**
 *  @brief  Resize event.
 *  @param  width [in] resized screen width
 *  @param  height [in] resized screen height
 */
/*===========================================================================*/
void Label::resizeEvent( int width, int height )
{
    kvs::IgnoreUnusedVariable( width );
    kvs::IgnoreUnusedVariable( height );

    this->screenResized();
}

} // end of namespace glut

} // end of namespace kvs
