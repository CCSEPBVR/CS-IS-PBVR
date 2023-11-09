/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorColorControlPoints.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorColorControlPoints.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TransferFunctionEditorColorControlPoints.h"
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

#define PBVR_NO_HISTOGRAM

namespace
{
kvs::visclient::TransferFunctionEditorColorControlPoints* ptfe;

void CallBackUpdate( const int id )
{
    ptfe->update();
}

void CallBackSave( const int id )
{
    ptfe->save();
}

void CallBackCancel( const int id )
{
    ptfe->cancel();
}

} // end of namespace

namespace kvs
{

namespace visclient
{


TransferFunctionEditorColorControlPoints::TransferFunctionEditorColorControlPoints( kvs::visclient::TransferFunctionEditorMain* parent ):
    m_parent( parent ),
    m_screen( parent ),
    m_color_map_palette( NULL ),
    m_histogram( NULL ),
    m_min_value( 0.0f ),
    m_max_value( 0.0f ),
    m_resolution( 256 ),
    m_visible( false )
{
    const std::string title = "Color Map Editor (control points)";
    const int x = ( parent != 0 ) ? parent->x() + parent->width() + 5 : 0;
    const int y = ( parent != 0 ) ? parent->y() : 0;
//  const int width = 350;
    const int width = 470;
#ifndef PBVR_NO_HISTOGRAM
    const int height = 450;
#else
    const int height = 380;
#endif
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

#ifndef PBVR_NO_HISTOGRAM
    m_histogram = new kvs::glut::Histogram( this );
    m_histogram->setCaption( "Histogram" );
    m_histogram->setX( m_color_map_palette->x0() );
    m_histogram->setY( m_color_map_palette->y1() - m_color_map_palette->margin() );
    m_histogram->setHeight( 100 );
    m_histogram->setGraphColor( kvs::RGBAColor( 100, 100, 100, 1.0f ) );
//  m_histogram->show();
#endif

}

TransferFunctionEditorColorControlPoints::~TransferFunctionEditorColorControlPoints()
{
    if ( m_color_map_palette ) delete m_color_map_palette;
    if ( m_histogram ) delete m_histogram;
}

int TransferFunctionEditorColorControlPoints::show()
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

    m_glui_text_win->add_statictext( "Control Point" );
    for ( int n = 0; n < 10; n++ )
    {
        std::stringstream ss;
        ss << "CP" << ( n + 1 ) << ")";
        m_glui_edit_c[n] = m_glui_text_win->add_edittext( ss.str().c_str(), GLUI_EDITTEXT_TEXT, NULL, n, CallBackUpdate );
//      m_glui_edit_c[n]->set_alignment(GLUI_ALIGN_LEFT);
        m_glui_edit_c[n]->set_w( 10 );
    }
    m_glui_text_win->add_column( false );
    m_glui_text_win->add_statictext( "Red" );
    for ( int n = 0; n < 10; n++ )
    {
        std::stringstream ss;
        ss << "R" << ( n + 1 ) << ")";
        m_glui_edit_r[n] = m_glui_text_win->add_edittext( ss.str().c_str(), GLUI_EDITTEXT_FLOAT, NULL, n, CallBackUpdate );
//      m_glui_edit_r[n]->set_alignment(GLUI_ALIGN_LEFT);
        m_glui_edit_r[n]->set_w( 10 );
    }
    m_glui_text_win->add_column( false );
    m_glui_text_win->add_statictext( "Green" );
    for ( int n = 0; n < 10; n++ )
    {
        std::stringstream ss;
        ss << "G" << ( n + 1 ) << ")";
        m_glui_edit_g[n] = m_glui_text_win->add_edittext( ss.str().c_str(), GLUI_EDITTEXT_FLOAT, NULL, n, CallBackUpdate );
//      m_glui_edit_g[n]->set_alignment(GLUI_ALIGN_LFET);
        m_glui_edit_g[n]->set_w( 10 );
    }
    m_glui_text_win->add_column( false );
    m_glui_text_win->add_statictext( "Blue" );
    for ( int n = 0; n < 10; n++ )
    {
        std::stringstream ss;
        ss << "B" << ( n + 1 ) << ")";
        m_glui_edit_b[n] = m_glui_text_win->add_edittext( ss.str().c_str(), GLUI_EDITTEXT_FLOAT, NULL, n, CallBackUpdate );
//      m_glui_edit_b[n]->set_alignment(GLUI_ALIGN_LEFT);
        m_glui_edit_b[n]->set_w( 10 );
    }

    return id;
}

void TransferFunctionEditorColorControlPoints::hide()
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

void TransferFunctionEditorColorControlPoints::setResolution( const size_t resolution )
{
    m_resolution = resolution;
}

kvs::ScreenBase* TransferFunctionEditorColorControlPoints::screen()
{
    return m_screen;
}

const kvs::glut::ColorMapPalette* TransferFunctionEditorColorControlPoints::colorMapPalette() const
{
    return m_color_map_palette;
}

const kvs::ColorMap TransferFunctionEditorColorControlPoints::colorMap() const
{
    return m_color_map_palette->colorMap();
}

const kvs::TransferFunction TransferFunctionEditorColorControlPoints::m_transfer_function() const
{
//    const kvs::Real32 min_value = m_initial_transfer_function.colorMap().minValue();
//    const kvs::Real32 max_value = m_initial_transfer_function.colorMap().maxValue();
//  kvs::TransferFunction m_transfer_function( this->colorMap(), this->opacityMap() );
    kvs::TransferFunction m_transfer_function( this->colorMap() );
//    m_transfer_function.setRange( min_value, max_value );
    m_transfer_function.setRange( m_min_value, m_max_value );

    return m_transfer_function;
}

void TransferFunctionEditorColorControlPoints::setTransferFunction( const kvs::TransferFunction& transfer_function )
{
    const kvs::ColorMap& cmap = transfer_function.colorMap();
    const kvs::OpacityMap& omap = transfer_function.opacityMap();

    // Deep copy for the initial transfer function.
    kvs::ColorMap::Table color_map_table( cmap.table().pointer(), cmap.table().size() );
    kvs::OpacityMap::Table opacity_map_table( omap.table().pointer(), omap.table().size() );
//    kvs::ColorMap color_map( color_map_table, cmap.minValue(), cmap.maxValue() );
//    kvs::OpacityMap opacity_map( opacity_map_table, omap.minValue(), omap.maxValue() );
    kvs::ColorMap color_map( color_map_table );
    kvs::OpacityMap opacity_map( opacity_map_table );
    m_initial_transfer_function.setColorMap( color_map );
    m_initial_transfer_function.setOpacityMap( opacity_map );

    if ( transfer_function.hasRange() )
    {
        m_min_value = transfer_function.colorMap().minValue();
        m_max_value = transfer_function.colorMap().maxValue();
        m_initial_transfer_function.setRange( m_min_value, m_max_value );
    }

    m_color_map_palette->setColorMap( color_map );
}

void TransferFunctionEditorColorControlPoints::setVolumeObject( const kvs::VolumeObjectBase& object )
{
    if ( !m_initial_transfer_function.hasRange() )
    {
        m_min_value = object.minValue();
        m_max_value = object.maxValue();
    }

    m_histogram->create( &object );
}

//kawamura
void TransferFunctionEditorColorControlPoints::setControlPointValue( const size_t n,  const float cp, const float r, const float g, const float b )
{
    std::stringstream ss;
    ss << cp;
    if ( m_glui_edit_c[n] ) m_glui_edit_c[n]->set_text( ss.str() );
    if ( m_glui_edit_r[n] ) m_glui_edit_r[n]->set_float_val( r );
    if ( m_glui_edit_g[n] ) m_glui_edit_g[n]->set_float_val( g );
    if ( m_glui_edit_b[n] ) m_glui_edit_b[n]->set_float_val( b );
}

void TransferFunctionEditorColorControlPoints::update()
{
    const float max_value = 1.0;
    const float min_value = 0.0;

    kvs::ColorMap cmap( m_resolution, min_value, max_value );

    //kawamura
    for ( int n = 0; n < 10; n++ )
    {
        std::string cpt = m_glui_edit_c[n]->get_text();
        if ( cpt != "" )
        {
            float x = std::atof( cpt.c_str() );
            float r = m_glui_edit_r[n]->get_float_val();
            float g = m_glui_edit_g[n]->get_float_val();
            float b = m_glui_edit_b[n]->get_float_val();

            r *= 255.0;
            g *= 255.0;
            b *= 255.0;

            kvs::RGBColor color( ( int )r, ( int )g, ( int )b );
            cmap.addPoint( x, color );
        }
    }
    cmap.create();

    m_color_map_palette->setColorMap( cmap );
    this->redraw();
}

void TransferFunctionEditorColorControlPoints::save()
{
    m_parent->saveTransferFunctionEditorColorControlPoints();
}

void TransferFunctionEditorColorControlPoints::cancel()
{
    this->hide();
}

} // end of namespace visclient

} // end of namespace kvs
