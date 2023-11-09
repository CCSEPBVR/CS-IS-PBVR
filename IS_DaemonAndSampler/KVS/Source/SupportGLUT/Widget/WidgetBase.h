/*****************************************************************************/
/**
 *  @file   WidgetBase.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WidgetBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLUT__WIDGET_BASE_H_INCLUDE
#define KVS__GLUT__WIDGET_BASE_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/RGBAColor>
#include <kvs/EventListener>
#include <kvs/ScreenBase>
#include <kvs/glut/Rectangle>


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Base class of the widget.
 */
/*===========================================================================*/
class WidgetBase : public kvs::glut::Rectangle, public kvs::EventListener
{
    // Class name.
    kvsClassName( kvs::glut::WidgetBase );

public:

    WidgetBase( kvs::ScreenBase* screen );

    virtual ~WidgetBase( void );

    enum FontType
    {
      BITMAP_8_BY_13,
      BITMAP_9_BY_15,
      BITMAP_TIMES_ROMAN_10,
      BITMAP_TIMES_ROMAN_24,
      BITMAP_HELVETICA_10,
      BITMAP_HELVETICA_12,
      BITMAP_HELVETICA_18
    };

protected:

    kvs::ScreenBase*   m_screen; ///< pointer to the screen
    int                m_margin; ///< margin
    bool               m_is_shown; ///< check flag whether the widget is shown or not
    kvs::RGBColor      m_text_color; ///< text color
    kvs::RGBAColor     m_background_color; ///< background color
    kvs::RGBAColor     m_background_border_color; ///< background border color
    float              m_background_border_width; ///< background border width
    int                m_character_width; ///< character width
    int                m_character_height; ///< character height
    FontType           m_font;

private:

    WidgetBase( void ){}

protected:

    kvs::ScreenBase* screen( void );

public:

    const int margin( void ) const;

    const bool isShown( void ) const;

    const kvs::RGBColor& textColor( void ) const;

    const kvs::RGBAColor& backgroundColor( void ) const;

    const kvs::RGBAColor& backgroundBorderColor( void ) const;

    const float backgroundBorderWidth( void ) const;

    const int characterWidth( void ) const;

    const int characterHeight( void ) const;

public:

    void setMargin( const int margin );

    void setTextColor( const kvs::RGBColor& text_color );

    void setBackgroundColor( const kvs::RGBAColor& background_color );

    void setBackgroundOpacity( const float opacity );

    void setBackgroundBorderColor( const kvs::RGBAColor& border_color );

    void setBackgroundBorderOpacity( const float opacity );

    void setBackgroundBorderWidth( const float border_width );

    void setCharacterType( FontType font );

public:

    void show( void );

    void hide( void );

protected:

    virtual void begin_draw( void );

    virtual void end_draw( void );

    virtual void draw_background( void );

    virtual void draw_text( const int x, const int y, const std::string& text );

    virtual void swap_color( kvs::RGBColor& color1, kvs::RGBColor& color2 );

    virtual kvs::RGBColor get_darkened_color( const kvs::RGBColor& color, const float darkness );

    virtual int get_fitted_width( void );

    virtual int get_fitted_height( void );
};

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__WIDGET_BASE_H_INCLUDE
