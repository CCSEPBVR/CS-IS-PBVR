/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorColorFreeformCurve.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorColorFreeformCurve.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "LegendDivisionColor.h"
#include "LegendpanelUI.h"
#include <kvs/DebugNew>
#include <kvs/MouseEvent>
#include <kvs/Math>
#include <kvs/HSVColor>
#include <kvs/RGBColor>
#include <kvs/Date>
#include <kvs/Time>
#include <kvs/glut/GLUT>
#include <kvs/IgnoreUnusedVariable>
#include <SupportGLUT/Viewer/KVSKey.h>
#include <SupportGLUT/Viewer/KVSMouseButton.h>

namespace
{

class CancelButton : public kvs::glut::PushButton
{
    kvs::visclient::LegendDivisionColor* m_editor;

public:

    CancelButton( kvs::visclient::LegendDivisionColor* editor ) :
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released()
    {
        m_editor->cancel();
    }
};

class SaveButton : public kvs::glut::PushButton
{
    kvs::visclient::LegendDivisionColor* m_editor;

public:

    SaveButton( kvs::visclient::LegendDivisionColor* editor ) :
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released()
    {
        m_editor->save();
    }
};

} // end of namespace


namespace kvs
{

namespace visclient
{


LegendDivisionColor::LegendDivisionColor( kvs::visclient::LegendpanelUI* parent, kvs::ScreenBase* parent_screen ) :
    m_parent( parent ),
    m_screen( parent_screen ),
    m_color_palette( NULL ),
    m_save_button( NULL ),
    m_cancel_button( NULL ),
    m_visible( false )
{
    const std::string title = "Color Map Editor(Division)";
    const int x = 905;
    const int y = 105;
    const int width = 350;
    const int height = 220;
    const int margin = 10;
    const kvs::RGBColor color( 200, 200, 200 );

    SuperClass::background()->setColor( color );
    SuperClass::setTitle( title );
    SuperClass::setPosition( x, y );
    SuperClass::setSize( width, height );

    m_color_palette = new kvs::glut::ColorPalette( this );
    m_color_palette->setCaption( "Color palette(Division)" );
    m_color_palette->show();

    const size_t button_margin = 5;
    const size_t button_width = ( width - 2 * margin - button_margin ) / 2;

    m_cancel_button = new ::CancelButton( this );
    m_cancel_button->setCaption( "Close" );
    m_cancel_button->setX( ( m_color_palette->x1() - m_color_palette->x0() ) / 2 - button_width / 2 );
    m_cancel_button->setY( m_color_palette->y1() );
    m_cancel_button->setWidth( button_width );
    m_cancel_button->show();
}

LegendDivisionColor::~LegendDivisionColor()
{
    if ( m_color_palette ) delete m_color_palette;
    if ( m_cancel_button ) delete m_cancel_button;
    if ( m_save_button ) delete m_save_button;
}


int LegendDivisionColor::show()
{
    if ( m_visible ) return m_id;
    m_visible = true;

    int id = SuperClass::show();
    this->redraw();
    return id;
}

void LegendDivisionColor::hide()
{
    glutSetWindow( m_id );
    int x = glutGet( ( GLenum )GLUT_WINDOW_X );
    int y = glutGet( ( GLenum )GLUT_WINDOW_Y );
#ifdef WIN32
    SuperClass::setPosition( x, y - 22 );
#else
    SuperClass::setPosition( x - 4, y - 28 );
#endif

    SuperClass::hide();
    m_visible = false;
}

kvs::ScreenBase* LegendDivisionColor::screen()
{
    return m_screen;
}

const kvs::glut::ColorPalette* LegendDivisionColor::colorPalette() const
{
    return m_color_palette;
}

const kvs::RGBColor LegendDivisionColor::color() const
{
    return m_color_palette->color();

}

void LegendDivisionColor::cancel()
{
    this->hide();
}

void LegendDivisionColor::save()
{
//  m_parent->setDivisionColor();
}

} // end of namespace visclient

} // end of namespace kvs
