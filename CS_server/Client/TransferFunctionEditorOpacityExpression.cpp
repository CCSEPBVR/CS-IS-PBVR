/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorOpacityExpression.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorOpacityExpression.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TransferFunctionEditorOpacityExpression.h"
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

#include "function.h"
#include "function_op.h"
#include "function_parser.h"

namespace
{
kvs::visclient::TransferFunctionEditorOpacityExpression* ptfe;

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


TransferFunctionEditorOpacityExpression::TransferFunctionEditorOpacityExpression( kvs::visclient::TransferFunctionEditorMain* parent ):
    m_parent( parent ),
    m_screen( parent ),
    m_opacity_map_palette( NULL ),
    m_min_value( 0.0f ),
    m_max_value( 0.0f ),
    m_resolution( 256 ),
    m_visible( false )
{
    const std::string title = "Opacity Map Editor (expression)";
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

    m_opacity_map_palette = new kvs::glut::OpacityMapPalette( this );
    m_opacity_map_palette->setCaption( "Opacity" );
    m_opacity_map_palette->setOpacityMap( m_initial_transfer_function.opacityMap() );
//  m_opacity_map_palette->setX( m_color_map_palette->x0() );
//  m_opacity_map_palette->setY( m_color_map_palette->y1() - m_color_map_palette->margin() );
    m_opacity_map_palette->setHeight( 100 );
    m_opacity_map_palette->setEventType(
        kvs::EventBase::PaintEvent |
        kvs::EventBase::ResizeEvent );
    m_opacity_map_palette->show();

}

TransferFunctionEditorOpacityExpression::~TransferFunctionEditorOpacityExpression()
{
    if ( m_opacity_map_palette ) delete m_opacity_map_palette;
}

void TransferFunctionEditorOpacityExpression::setResolution( const size_t resolution )
{
    m_resolution = resolution;
}

int TransferFunctionEditorOpacityExpression::show()
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

    m_glui_edit_tfa = m_glui_text_win->add_edittext( "O", GLUI_EDITTEXT_TEXT, NULL, id, ::CallBackUpdate );
    m_glui_edit_tfa->set_alignment( GLUI_ALIGN_RIGHT );
    m_glui_edit_tfa->set_w( 300 );

    m_glui_edit_tfa->set_text( "x" );
    this->update();

    return id;
}

void TransferFunctionEditorOpacityExpression::hide()
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

kvs::ScreenBase* TransferFunctionEditorOpacityExpression::screen()
{
    return m_screen;
}

const kvs::glut::OpacityMapPalette* TransferFunctionEditorOpacityExpression::opacityMapPalette() const
{
    return m_opacity_map_palette;
}

const kvs::OpacityMap TransferFunctionEditorOpacityExpression::opacityMap() const
{
    return m_opacity_map_palette->opacityMap();
}

const kvs::TransferFunction TransferFunctionEditorOpacityExpression::m_transfer_function() const
{
//    const kvs::Real32 min_value = m_initial_transfer_function.colorMap().minValue();
//    const kvs::Real32 max_value = m_initial_transfer_function.colorMap().maxValue();
//  kvs::TransferFunction m_transfer_function( this->colorMap(), this->opacityMap() );
    kvs::TransferFunction m_transfer_function( this->opacityMap() );
//    m_transfer_function.setRange( min_value, max_value );
    m_transfer_function.setRange( m_min_value, m_max_value );

    return m_transfer_function;
}

void TransferFunctionEditorOpacityExpression::setTransferFunction( const kvs::TransferFunction& transfer_function )
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

    m_opacity_map_palette->setOpacityMap( opacity_map );

}

void TransferFunctionEditorOpacityExpression::equation( std::string* oe )
{
    *oe = m_glui_edit_tfa->get_text();
}

void TransferFunctionEditorOpacityExpression::setEquation( const std::string& oe )
{
    m_glui_edit_tfa->set_text( oe.c_str() );

    this->setOpacityMapEquation( oe );
}

void TransferFunctionEditorOpacityExpression::setOpacityMapEquation( const std::string& oe )
{
    const float min_value = 0.0;
    const float max_value = 1.0;

    std::string ofe = oe;

    FuncParser::Variables vars;
    FuncParser::Variable var_x;
    FuncParser::Function of;

    var_x.tag( "x" );
    vars.push_back( var_x );

    FuncParser::FunctionParser of_parse( ofe, ofe.size() + 1 );
    FuncParser::FunctionParser::Error err_o = of_parse.express( of, vars );

    if ( err_o != FuncParser::FunctionParser::ERR_NONE )
    {
        std::cerr << "Expression Error. " << std::endl;
        return;
    }

    kvs::OpacityMap omap( m_resolution, min_value, max_value );

    const float stride = ( max_value - min_value ) / ( m_resolution - 1 );
    float x = min_value;
    for ( size_t i = 0; i < m_resolution; ++i, x += stride )
    {
        float opacity;

        var_x = x;
        opacity = of.eval();
        opacity = ( opacity > 1.0 ) ? 1.0 : ( opacity < 0 ) ? 0 : opacity;

        omap.addPoint( x, opacity );
    }
    omap.create();

    m_opacity_map_palette->setOpacityMap( omap );
    this->redraw();
}
void TransferFunctionEditorOpacityExpression::update()
{
    std::string opacity = m_glui_edit_tfa->get_text();

    setOpacityMapEquation( opacity );
}

void TransferFunctionEditorOpacityExpression::save()
{
    m_parent->saveTransferFunctionEditorOpacityExpression();
}

void TransferFunctionEditorOpacityExpression::cancel()
{
    this->hide();
}

} // end of namespace visclient

} // end of namespace kvs
