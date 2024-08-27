/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorColorSelectColormap.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorColorSelectColormap.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TransferFunctionEditorColorSelectColormap.h"
#include "TransferFunctionEditorMain.h"
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
kvs::visclient::TransferFunctionEditorColorSelectColormap* ptfe;

void CallBackSave( const int id )
{
    ptfe->save();
}

void CallBackCancel( const int id )
{
    ptfe->cancel();
}

void CallBackColor( const int id )
{
    ptfe->color();
}

} // end of namespace

namespace kvs
{

namespace visclient
{


TransferFunctionEditorColorSelectColormap::TransferFunctionEditorColorSelectColormap( kvs::visclient::TransferFunctionEditorMain* parent ):
    m_parent( parent ),
    m_screen( parent ),
    m_color_map_palette( NULL ),
    m_min_value( 0.0f ),
    m_max_value( 0.0f ),
    m_resolution( 256 ),
    m_visible( false )
{
    const std::string title = "Color Map Editor (select colormap)";
    const int x = ( parent != 0 ) ? parent->x() + parent->width() + 5 : 0;
    const int y = ( parent != 0 ) ? parent->y() : 0;
    const int width = 350;
    const int height = 160;
    const int margin = 10;
    const kvs::RGBColor color( 200, 200, 200 );

    SuperClass::background()->setColor( color );
    SuperClass::setTitle( title );
    SuperClass::setPosition( x, y );
    SuperClass::setSize( width, height );

    m_initial_transfer_function.create( m_resolution );

    m_color_map_palette = new kvs::glut::ColorMapPalette( this );
    m_color_map_palette->setCaption( "Color" );
//  m_color_map_palette->setX( 0 );
//  m_color_map_palette->setY( 100 );
    m_color_map_palette->setColorMap( m_initial_transfer_function.colorMap() );
    m_color_map_palette->setEventType(
        kvs::EventBase::PaintEvent |
        kvs::EventBase::ResizeEvent );
    m_color_map_palette->show();

    m_color_map_library = new ColorMapLibrary( m_resolution );
}

TransferFunctionEditorColorSelectColormap::~TransferFunctionEditorColorSelectColormap()
{
    if ( m_color_map_palette ) delete m_color_map_palette;
    if ( m_color_map_library ) delete m_color_map_library;
}

void TransferFunctionEditorColorSelectColormap::setResolution( const size_t resolution )
{
    m_resolution = resolution;

    if ( m_color_map_library )
    {
        delete m_color_map_library;
        m_color_map_library = new ColorMapLibrary( resolution );
    }
}

int TransferFunctionEditorColorSelectColormap::show()
{
    if ( m_visible ) return m_id;
    m_visible = true;

    int id = SuperClass::show();
    ::ptfe = this;

    m_glui_buttom_win = GLUI_Master.create_glui_subwindow( m_id, GLUI_SUBWINDOW_BOTTOM );
    m_glui_text_win   = GLUI_Master.create_glui_subwindow( m_id, GLUI_SUBWINDOW_BOTTOM );

    m_glui_buttom_win->set_main_gfx_window( m_id );
    m_glui_text_win->set_main_gfx_window( m_id );

    m_glui_buttom_win->add_button( "Save", id, ::CallBackSave );
    m_glui_buttom_win->add_column( false );
    m_glui_buttom_win->add_button( "Cancel", id, ::CallBackCancel );

    m_glui_color = m_glui_text_win->add_listbox( "Default Color ", NULL, 0, CallBackColor );
    m_glui_color->set_alignment( GLUI_ALIGN_RIGHT );
    m_glui_color->set_w( 220 );

    if ( !m_color_map_library->empty() )
    {
        int n = 0;
        for ( ColorMapLibrary::iterator i = m_color_map_library->begin();
                i != m_color_map_library->end(); i++ )
        {
            m_glui_color->add_item( n, i->c_str() );
            n++;
        }
        m_glui_color->set_int_val( 0 );
    }
    else
    {
        m_glui_color->add_item( -1, "" );
    }

    m_color_map_palette->setColorMap( m_color_map_library->getColorMapByIndex( 0 ) );
    this->redraw();

    return id;
}

void TransferFunctionEditorColorSelectColormap::hide()
{
    ::ptfe = NULL;
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

kvs::ScreenBase* TransferFunctionEditorColorSelectColormap::screen()
{
    return m_screen;
}

const kvs::glut::ColorMapPalette* TransferFunctionEditorColorSelectColormap::colorMapPalette() const
{
    return m_color_map_palette;
}

const kvs::ColorMap TransferFunctionEditorColorSelectColormap::colorMap() const
{
    return m_color_map_palette->colorMap();
}

const kvs::TransferFunction TransferFunctionEditorColorSelectColormap::m_transfer_function() const
{
//    const kvs::Real32 min_value = m_initial_transfer_function.colorMap().minValue();
//    const kvs::Real32 max_value = m_initial_transfer_function.colorMap().maxValue();
//  kvs::TransferFunction m_transfer_function( this->colorMap(), this->opacityMap() );
    kvs::TransferFunction m_transfer_function( this->colorMap() );
//    m_transfer_function.setRange( min_value, max_value );
    m_transfer_function.setRange( m_min_value, m_max_value );

    return m_transfer_function;
}

void TransferFunctionEditorColorSelectColormap::setColorMapName( const std::string& name )
{
    size_t n = m_color_map_library->getIndexByName( name );
    if ( n < m_color_map_library->size() )
    {
        m_glui_color->set_int_val( n );
        kvs::ColorMap color_map = m_color_map_library->getColorMapByName( name );
        m_color_map_palette->setColorMap( color_map );
    }
    this->redraw();
}

std::string TransferFunctionEditorColorSelectColormap::colorMapName() const
{
    return m_color_map_library->at( m_glui_color->get_int_val() );
}

void TransferFunctionEditorColorSelectColormap::save()
{
    m_parent->saveTransferFunctionEditorColorSelectColormap();
}

void TransferFunctionEditorColorSelectColormap::cancel()
{
    this->hide();
}

void TransferFunctionEditorColorSelectColormap::color()
{
    int val = m_glui_color->get_int_val();
    const kvs::ColorMap& color_map = m_color_map_library->getColorMapByIndex( val );

    m_color_map_palette->setColorMap( color_map );
    this->redraw();
}

} // end of namespace visclient

} // end of namespace kvs
