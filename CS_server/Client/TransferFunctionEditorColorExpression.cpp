/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorColorExpression.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorColorExpression.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TransferFunctionEditorColorExpression.h"
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

#define PBVR_NO_HISTOGRAM

namespace
{
kvs::visclient::TransferFunctionEditorColorExpression* ptfe;

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


TransferFunctionEditorColorExpression::TransferFunctionEditorColorExpression( kvs::visclient::TransferFunctionEditorMain* parent ):
    m_parent( parent ),
    m_screen( parent ),
    m_color_map_palette( NULL ),
    m_histogram( NULL ),
    m_min_value( 0.0f ),
    m_max_value( 0.0f ),
    m_resolution( 256 ),
    m_visible( false )
{
    const std::string title = "Color Map Editor (expression)";
    const int x = ( parent != 0 ) ? parent->x() + parent->width() + 5 : 0;
    const int y = ( parent != 0 ) ? parent->y() : 0;
    const int width = 350;
#ifndef PBVR_NO_HISTOGRAM
    const int height = 300;
#else
    const int height = 200;
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

TransferFunctionEditorColorExpression::~TransferFunctionEditorColorExpression()
{
    if ( m_color_map_palette ) delete m_color_map_palette;
    if ( m_histogram ) delete m_histogram;
}

int TransferFunctionEditorColorExpression::show()
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

    m_glui_edit_red = m_glui_text_win->add_edittext( "R", GLUI_EDITTEXT_TEXT, NULL, id, ::CallBackUpdate );
    m_glui_edit_red->set_alignment( GLUI_ALIGN_RIGHT );
    m_glui_edit_red->set_w( 300 );

    m_glui_edit_green = m_glui_text_win->add_edittext( "G", GLUI_EDITTEXT_TEXT, NULL, id, ::CallBackUpdate );
    m_glui_edit_green->set_alignment( GLUI_ALIGN_RIGHT );
    m_glui_edit_green->set_w( 300 );

    m_glui_edit_blue  = m_glui_text_win->add_edittext( "B", GLUI_EDITTEXT_TEXT, NULL, id, ::CallBackUpdate );
    m_glui_edit_blue->set_alignment( GLUI_ALIGN_RIGHT );
    m_glui_edit_blue->set_w( 300 );

    m_glui_edit_red->set_text( "0" );
    m_glui_edit_green->set_text( "0" );
    m_glui_edit_blue->set_text( "0" );
    this->update();

    return id;
}

void TransferFunctionEditorColorExpression::hide()
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

void TransferFunctionEditorColorExpression::setResolution( const size_t resolution )
{
    m_resolution = resolution;
}

kvs::ScreenBase* TransferFunctionEditorColorExpression::screen()
{
    return m_screen;
}

const kvs::glut::ColorMapPalette* TransferFunctionEditorColorExpression::colorMapPalette() const
{
    return m_color_map_palette;
}

const kvs::ColorMap TransferFunctionEditorColorExpression::colorMap() const
{
    return m_color_map_palette->colorMap();
}

const kvs::TransferFunction TransferFunctionEditorColorExpression::m_transfer_function() const
{
//    const kvs::Real32 min_value = m_initial_transfer_function.colorMap().minValue();
//    const kvs::Real32 max_value = m_initial_transfer_function.colorMap().maxValue();
//  kvs::TransferFunction m_transfer_function( this->colorMap(), this->opacityMap() );
    kvs::TransferFunction m_transfer_function( this->colorMap() );
//    m_transfer_function.setRange( min_value, max_value );
    m_transfer_function.setRange( m_min_value, m_max_value );

    return m_transfer_function;
}

void TransferFunctionEditorColorExpression::setTransferFunction( const kvs::TransferFunction& transfer_function )
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

void TransferFunctionEditorColorExpression::setVolumeObject( const kvs::VolumeObjectBase& object )
{
    if ( !m_initial_transfer_function.hasRange() )
    {
        m_min_value = object.minValue();
        m_max_value = object.maxValue();
    }

    m_histogram->create( &object );
}

void TransferFunctionEditorColorExpression::equation( std::string* re, std::string* ge, std::string* be )
{
    *re = m_glui_edit_red->get_text();
    *ge = m_glui_edit_green->get_text();
    *be = m_glui_edit_blue->get_text();
}

void TransferFunctionEditorColorExpression::setEquation( const std::string& re, const std::string& ge, const std::string& be )
{
    m_glui_edit_red->set_text( re.c_str() );
    m_glui_edit_green->set_text( ge.c_str() );
    m_glui_edit_blue->set_text( be.c_str() );

    this->setColorMapEquation( re, ge, be );
}

//kawamura
//set input color range to 0,1 -> 0,255
void TransferFunctionEditorColorExpression::setColorMapEquation( const std::string& re, const std::string& ge, const std::string& be )
{
    const float min_value = 0.0;
    const float max_value = 1.0;

    std::string rfe = re;
    std::string gfe = ge;
    std::string bfe = be;

    FuncParser::Variables vars;
    FuncParser::Variable var_x;
    FuncParser::Function rf, gf, bf;

    var_x.tag( "x" );
    vars.push_back( var_x );

    FuncParser::FunctionParser rf_parse( rfe, rfe.size() + 1 );
    FuncParser::FunctionParser gf_parse( gfe, gfe.size() + 1 );
    FuncParser::FunctionParser bf_parse( bfe, bfe.size() + 1 );
    FuncParser::FunctionParser::Error err_r = rf_parse.express( rf, vars );
    FuncParser::FunctionParser::Error err_g = gf_parse.express( gf, vars );
    FuncParser::FunctionParser::Error err_b = bf_parse.express( bf, vars );

    if ( ( err_r != FuncParser::FunctionParser::ERR_NONE ) ||
            ( err_g != FuncParser::FunctionParser::ERR_NONE ) ||
            ( err_b != FuncParser::FunctionParser::ERR_NONE ) )
    {
        std::cerr << "Expression Error. " << std::endl;
        return;
    }

    kvs::ColorMap cmap( m_resolution, min_value, max_value );

    const float stride = ( max_value - min_value ) / ( m_resolution - 1 );
    float x = min_value;
    for ( size_t i = 0; i < m_resolution; ++i, x += stride )
    {
        //int r,g,b;
        float r, g, b; //kawamura

        var_x = x;
        r = rf.eval();
        g = gf.eval();
        b = bf.eval();

        /*
        r = ( r > 255 )? 255: ( r < 0 )? 0: r;
        g = ( g > 255 )? 255: ( g < 0 )? 0: g;
        b = ( b > 255 )? 255: ( b < 0 )? 0: b;
        */
        //kawamura
        r = ( r > 1.0 ) ? 1.0 : ( r < 0 ) ? 0 : r;
        g = ( g > 1.0 ) ? 1.0 : ( g < 0 ) ? 0 : g;
        b = ( b > 1.0 ) ? 1.0 : ( b < 0 ) ? 0 : b;

        r *= 255;
        g *= 255;
        b *= 255;

        kvs::RGBColor color( ( int )r, ( int )g, ( int )b );
        cmap.addPoint( x, color );
    }
    cmap.create();

    m_color_map_palette->setColorMap( cmap );
    this->redraw();
}

void TransferFunctionEditorColorExpression::update()
{
    std::string red = m_glui_edit_red->get_text();
    std::string green = m_glui_edit_green->get_text();
    std::string blue = m_glui_edit_blue->get_text();

    setColorMapEquation( red, green, blue );
}

void TransferFunctionEditorColorExpression::save()
{
    m_parent->saveTransferFunctionEditorColorExpression();
}

void TransferFunctionEditorColorExpression::cancel()
{
    this->hide();
}

} // end of namespace visclient

} // end of namespace kvs
