/*****************************************************************************/
/**
 *  @file   TransferFunctionEditor.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditor.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TransferFunctionEditor.h"
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


namespace { kvs::glut::old::TransferFunctionEditor* current; }

namespace
{

/*===========================================================================*/
/**
 *  @brief  Draws a rectangle as lines.
 *  @param  widget [in] pointer to a widget
 *  @param  width [in] line width
 *  @param  color [in] line color
 */
/*===========================================================================*/
void DrawRectangle(
    const kvs::glut::old::TransferFunctionEditor::Widget* widget,
    const float width,
    const kvs::RGBColor& color )
{
    glLineWidth( width );
    glBegin( GL_LINE_LOOP );
    {
        glColor3ub( color.r(), color.g(), color.b() );
        glVertex2i( widget->x1(), widget->y1() );
        glVertex2i( widget->x0(), widget->y1() );
        glVertex2i( widget->x0(), widget->y0() );
        glVertex2i( widget->x1(), widget->y0() );
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Interpolates color values.
 *  @param  ratio [in] ratio [0-1]
 *  @param  c0 [in] color value 0
 *  @param  c1 [in] color value 1
 *  @return interpolated color value
 */
/*===========================================================================*/
const kvs::RGBColor GetInterpolatedColor( const float ratio, const kvs::RGBColor& c0, const kvs::RGBColor& c1 )
{
    const kvs::UInt8 r = static_cast<kvs::UInt8>( ratio * c0.r() + ( 1.0f - ratio ) * c1.r() + 0.5f );
    const kvs::UInt8 g = static_cast<kvs::UInt8>( ratio * c0.g() + ( 1.0f - ratio ) * c1.g() + 0.5f );
    const kvs::UInt8 b = static_cast<kvs::UInt8>( ratio * c0.b() + ( 1.0f - ratio ) * c1.b() + 0.5f );

    return( kvs::RGBColor( r, g, b ) );
}

} // end of namespace



namespace
{

class ResetButton : public kvs::glut::PushButton
{
    kvs::glut::TransferFunctionEditor* m_editor;

public:

    ResetButton( kvs::glut::TransferFunctionEditor* editor ):
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released( void )
    {
        m_editor->reset();
    }
};

class ApplyButton : public kvs::glut::PushButton
{
    kvs::glut::TransferFunctionEditor* m_editor;

public:

    ApplyButton( kvs::glut::TransferFunctionEditor* editor ):
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released( void )
    {
        m_editor->apply();
    }
};

class SaveButton : public kvs::glut::PushButton
{
    kvs::glut::TransferFunctionEditor* m_editor;

public:

    SaveButton( kvs::glut::TransferFunctionEditor* editor ):
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released( void )
    {
        m_editor->save();
    }
};

class UndoButton : public kvs::glut::PushButton
{
    kvs::glut::TransferFunctionEditor* m_editor;

public:

    UndoButton( kvs::glut::TransferFunctionEditor* editor ):
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released( void )
    {
        m_editor->undo();
    }
};

class RedoButton : public kvs::glut::PushButton
{
    kvs::glut::TransferFunctionEditor* m_editor;

public:

    RedoButton( kvs::glut::TransferFunctionEditor* editor ):
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released( void )
    {
        m_editor->redo();
    }
};

} // end of namespace


namespace kvs
{

namespace glut
{


TransferFunctionEditor::TransferFunctionEditor( kvs::ScreenBase* parent ):
    m_screen( parent ),
    m_color_palette( NULL ),
    m_color_map_palette( NULL ),
    m_opacity_map_palette( NULL ),
    m_histogram( NULL ),
    m_reset_button( NULL ),
    m_undo_button( NULL ),
    m_redo_button( NULL ),
    m_save_button( NULL ),
    m_apply_button( NULL ),
    m_min_value( 0.0f ),
    m_max_value( 0.0f )
{
    const std::string title = "Transfer Function Editor";
    const int x = ( parent != 0 ) ? parent->x() + parent->width() + 5 : 0;
    const int y = ( parent != 0 ) ? parent->y() : 0;
    const int width = 350;
//    const int height = 512;
    const int margin = 10;
    const int height = 512 - 100 + margin;
    const kvs::RGBColor color( 200, 200, 200 );

    SuperClass::background()->setColor( color );
    SuperClass::setTitle( title );
    SuperClass::setPosition( x, y );
    SuperClass::setSize( width, height );

    const size_t resolution = 256;
    m_initial_transfer_function.create( resolution );

    m_undo_stack.push_front( m_initial_transfer_function );

    m_stack_event = new StackEvent( this );
    SuperClass::addMouseReleaseEvent( m_stack_event );

    m_max_stack_size = 10;

    m_color_palette = new kvs::glut::ColorPalette( this );
    m_color_palette->setCaption( "Color palette" );
    m_color_palette->show();

    m_color_map_palette = new kvs::glut::ColorMapPalette( this );
    m_color_map_palette->setCaption( "Color" );
    m_color_map_palette->setColorMap( m_initial_transfer_function.colorMap() );
    m_color_map_palette->setX( m_color_palette->x0() );
    m_color_map_palette->setY( m_color_palette->y1() - m_color_palette->margin() );
    m_color_map_palette->attachColorPalette( m_color_palette );
    m_color_map_palette->show();

    m_opacity_map_palette = new kvs::glut::OpacityMapPalette( this );
    m_opacity_map_palette->setCaption( "Opacity" );
    m_opacity_map_palette->setOpacityMap( m_initial_transfer_function.opacityMap() );
    m_opacity_map_palette->setX( m_color_map_palette->x0() );
    m_opacity_map_palette->setY( m_color_map_palette->y1() - m_color_map_palette->margin() );
    m_opacity_map_palette->setHeight( 100 );
    m_opacity_map_palette->show();

    m_histogram = new kvs::glut::Histogram( this );
    m_histogram->setCaption( "Histogram" );
    m_histogram->setX( m_opacity_map_palette->x0() );
    m_histogram->setY( m_opacity_map_palette->y1() - m_opacity_map_palette->margin() );
    m_histogram->setHeight( 100 );
    m_histogram->setGraphColor( kvs::RGBAColor( 100, 100, 100, 1.0f ) );
//    m_histogram->show();

    const size_t button_margin = 5;
    const size_t button_width = ( width - 2 * margin - button_margin ) / 2;

    m_reset_button = new ::ResetButton( this );
    m_reset_button->setCaption( "Reset" );
//    m_reset_button->setX( m_histogram->x0() + m_histogram->margin() );
//    m_reset_button->setY( m_histogram->y1() );
    m_reset_button->setX( m_opacity_map_palette->x0() + m_histogram->margin() );
    m_reset_button->setY( m_opacity_map_palette->y1() );
    m_reset_button->setWidth( button_width );
    m_reset_button->show();

    m_undo_button = new ::UndoButton( this );
    m_undo_button->setCaption( "Undo" );
    m_undo_button->setX( m_reset_button->x1() + button_margin );
    m_undo_button->setY( m_reset_button->y() );
    m_undo_button->setWidth( ( button_width - button_margin ) / 2 );
    m_undo_button->show();

    m_redo_button = new ::RedoButton( this );
    m_redo_button->setCaption( "Redo" );
    m_redo_button->setX( m_undo_button->x1() + button_margin );
    m_redo_button->setY( m_undo_button->y() );
    m_redo_button->setWidth( ( button_width - button_margin ) / 2 );
    m_redo_button->show();

    m_save_button = new ::SaveButton( this );
    m_save_button->setCaption( "Save" );
    m_save_button->setX( m_reset_button->x0() );
    m_save_button->setY( m_reset_button->y1() + button_margin );
    m_save_button->setWidth( button_width );
    m_save_button->show();

    m_apply_button = new ::ApplyButton( this );
    m_apply_button->setCaption( "Apply" );
    m_apply_button->setX( m_save_button->x1() + button_margin );
    m_apply_button->setY( m_save_button->y0() );
    m_apply_button->setWidth( ( width -margin ) / 2 - m_opacity_map_palette->margin() );
    m_apply_button->show();
}

TransferFunctionEditor::~TransferFunctionEditor( void )
{
    if ( m_stack_event ) delete m_stack_event;
    if ( m_color_palette ) delete m_color_palette;
    if ( m_color_map_palette ) delete m_color_map_palette;
    if ( m_opacity_map_palette ) delete m_opacity_map_palette;
    if ( m_histogram ) delete m_histogram;
    if ( m_reset_button ) delete m_reset_button;
    if ( m_undo_button ) delete m_undo_button;
    if ( m_redo_button ) delete m_redo_button;
    if ( m_apply_button ) delete m_apply_button;
    if ( m_save_button ) delete m_save_button;
}

kvs::ScreenBase* TransferFunctionEditor::screen( void )
{
    return( m_screen );
}

const kvs::glut::ColorPalette* TransferFunctionEditor::colorPalette( void ) const
{
    return( m_color_palette );
}

const kvs::glut::ColorMapPalette* TransferFunctionEditor::colorMapPalette( void ) const
{
    return( m_color_map_palette );
}

const kvs::glut::OpacityMapPalette* TransferFunctionEditor::opacityMapPalette( void ) const
{
    return( m_opacity_map_palette );
}

const kvs::ColorMap TransferFunctionEditor::colorMap( void ) const
{
    return( m_color_map_palette->colorMap() );
}

const kvs::OpacityMap TransferFunctionEditor::opacityMap( void ) const
{
    return( m_opacity_map_palette->opacityMap() );
}

const kvs::TransferFunction TransferFunctionEditor::transferFunction( void ) const
{
//    const kvs::Real32 min_value = m_initial_transfer_function.colorMap().minValue();
//    const kvs::Real32 max_value = m_initial_transfer_function.colorMap().maxValue();
    kvs::TransferFunction transfer_function( this->colorMap(), this->opacityMap() );
//    transfer_function.setRange( min_value, max_value );
    transfer_function.setRange( m_min_value, m_max_value );

    return( transfer_function );
}

const size_t TransferFunctionEditor::undoStackSize( void ) const
{
    return( m_undo_stack.size() );
}

const size_t TransferFunctionEditor::redoStackSize( void ) const
{
    return( m_redo_stack.size() );
}

const size_t TransferFunctionEditor::maxStackSize( void ) const
{
    return( m_max_stack_size );
}

void TransferFunctionEditor::setTransferFunction( const kvs::TransferFunction& transfer_function )
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
    m_opacity_map_palette->setOpacityMap( opacity_map );

    m_undo_stack.clear();
    m_undo_stack.push_front( m_initial_transfer_function );
}

void TransferFunctionEditor::setVolumeObject( const kvs::VolumeObjectBase* object )
{
    if ( !m_initial_transfer_function.hasRange() )
    {
        m_min_value = object->minValue();
        m_max_value = object->maxValue();
    }

    m_histogram->create( object );
}

void TransferFunctionEditor::setMaxStackSize( const size_t stack_size )
{
    m_max_stack_size = stack_size;
}

void TransferFunctionEditor::reset( void )
{
    m_color_map_palette->setColorMap( m_initial_transfer_function.colorMap() );
    m_opacity_map_palette->setOpacityMap( m_initial_transfer_function.opacityMap() );

    this->redraw();
}

void TransferFunctionEditor::apply( void )
{
}

void TransferFunctionEditor::save( void )
{
    const std::string date = kvs::Date().toString("");
    const std::string time = kvs::Time().toString("");
    const std::string filename = "tfunc_" + date + "_" + time + ".kvsml";

    kvs::TransferFunction transfer_function = this->transferFunction();
    transfer_function.write( filename );
}

void TransferFunctionEditor::undo( void )
{
    if ( m_undo_stack.size() > 1 )
    {
        if ( m_redo_stack.size() > m_max_stack_size ) m_redo_stack.pop_back();
        m_redo_stack.push_front( m_undo_stack.front() );
        m_undo_stack.pop_front();

        kvs::TransferFunction& transfer_function = m_undo_stack.front();
        m_color_map_palette->setColorMap( transfer_function.colorMap() );
        m_opacity_map_palette->setOpacityMap( transfer_function.opacityMap() );

        this->redraw();
    }
}

void TransferFunctionEditor::redo( void )
{
    if ( m_redo_stack.size() > 1 )
    {
        kvs::TransferFunction& transfer_function = m_redo_stack.front();
        m_color_map_palette->setColorMap( transfer_function.colorMap() );
        m_opacity_map_palette->setOpacityMap( transfer_function.opacityMap() );

        if ( m_undo_stack.size() > m_max_stack_size ) m_undo_stack.pop_back();
        m_undo_stack.push_front( m_redo_stack.front() );
        m_redo_stack.pop_front();

        this->redraw();
    }
}

TransferFunctionEditor::StackEvent::StackEvent(
    kvs::glut::TransferFunctionEditor* editor ):
    m_editor( editor )
{
}

void TransferFunctionEditor::StackEvent::update( kvs::MouseEvent* event )
{
    if ( m_editor->opacityMapPalette()->palette().isActive() ||
         m_editor->colorMapPalette()->palette().isActive() )
    {
        if ( m_editor->m_undo_stack.size() > m_editor->m_max_stack_size ) m_editor->m_undo_stack.pop_back();
        m_editor->m_undo_stack.push_front( m_editor->transferFunction() );
        m_editor->redraw();
    }
}



namespace old
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new TransferFunctionEditor::Mouse class.
 */
/*===========================================================================*/
TransferFunctionEditor::Mouse::Mouse( void )
{
    m_previous = kvs::Vector2i( 0, 0 );
    m_current = kvs::Vector2i( 0, 0 );
}

/*===========================================================================*/
/**
 *  @brief  Returns the previous mouse position.
 *  @return previous mouse position
 */
/*===========================================================================*/
const kvs::Vector2i TransferFunctionEditor::Mouse::previous( void ) const
{
    return( m_previous );
}

/*===========================================================================*/
/**
 *  @brief  Returns the current mouse position.
 *  @return current mouse position
 */
/*===========================================================================*/
const kvs::Vector2i TransferFunctionEditor::Mouse::current( void ) const
{
    return( m_current );
}

/*===========================================================================*/
/**
 *  @brief  Saves the mouse press position.
 *  @param  x [in] x value in the window coordinate
 *  @param  y [in] y value in the window coordinate
 */
/*===========================================================================*/
void TransferFunctionEditor::Mouse::press( const int x, const int y )
{
    m_previous = kvs::Vector2i( x, y );
    m_current = kvs::Vector2i( x, y );
}

/*===========================================================================*/
/**
 *  @brief  Saves the mouse move position.
 *  @param  x [in] x value in the window coordinate
 *  @param  y [in] y value in the window coordinate
 */
/*===========================================================================*/
void TransferFunctionEditor::Mouse::move( const int x, const int y )
{
    m_current = kvs::Vector2i( x, y );
}

/*===========================================================================*/
/**
 *  @brief  Update the mouse position.
 */
/*===========================================================================*/
void TransferFunctionEditor::Mouse::stop( void )
{
    m_previous = m_current;
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new TransferFunctionEditor::Widget class.
 */
/*===========================================================================*/
TransferFunctionEditor::Widget::Widget( void ):
    m_x( 0 ),
    m_y( 0 ),
    m_width( 0 ),
    m_height( 0 ),
    m_is_active( false )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new TransferFunctionEditor::Widget class.
 *  @param  x [in] x value of widget position in the window coordinate
 *  @param  x [in] y value of widget position in the window coordinate
 *  @param  width [in] widget width
 *  @param  height [in] widget height
 *  @param  active [in] active flag
 */
/*===========================================================================*/
TransferFunctionEditor::Widget::Widget(
    const int x,
    const int y,
    const int width,
    const int height,
    const bool active ):
    m_x( x ),
    m_y( y ),
    m_width( width ),
    m_height( height ),
    m_is_active( active )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the x value of the widget position.
 *  @return x value of the widget position
 */
/*===========================================================================*/
const int TransferFunctionEditor::Widget::x( void ) const
{
    return( m_x );
}

/*===========================================================================*/
/**
 *  @brief  Returns the y value of the widget position.
 *  @return y value of the widget position
 */
/*===========================================================================*/
const int TransferFunctionEditor::Widget::y( void ) const
{
    return( m_y );
}

/*===========================================================================*/
/**
 *  @brief  Returns the widget width.
 *  @return widget width
 */
/*===========================================================================*/
const int TransferFunctionEditor::Widget::width( void ) const
{
    return( m_width );
}

/*===========================================================================*/
/**
 *  @brief  Returns the widget height.
 *  @return widget height
 */
/*===========================================================================*/
const int TransferFunctionEditor::Widget::height( void ) const
{
    return( m_height );
}

/*===========================================================================*/
/**
 *  @brief  Returns the x value of the bottom-left of the widget.
 *  @return x value of the bottom-left
 */
/*===========================================================================*/
const int TransferFunctionEditor::Widget::x0( void ) const
{
    return( m_x );
}

/*===========================================================================*/
/**
 *  @brief  Returns the y value of the bottom-left of the widget.
 *  @return y value of the bottom-left
 */
/*===========================================================================*/
const int TransferFunctionEditor::Widget::y0( void ) const
{
    return( m_y );
}

/*===========================================================================*/
/**
 *  @brief  Returns the x value of the top-right of the widget.
 *  @return x value of the top-right
 */
/*===========================================================================*/
const int TransferFunctionEditor::Widget::x1( void ) const
{
    return( m_x + m_width );
}

/*===========================================================================*/
/**
 *  @brief  Returns the y value of the top-right of the widget.
 *  @return y value of the top-right
 */
/*===========================================================================*/
const int TransferFunctionEditor::Widget::y1( void ) const
{
    return( m_y + m_height );
}

/*===========================================================================*/
/**
 *  @brief  Checks whether the widget is active or not.
 *  @return true, if the widget is active
 */
/*===========================================================================*/
const bool TransferFunctionEditor::Widget::isActive( void ) const
{
    return( m_is_active );
}

/*===========================================================================*/
/**
 *  @brief  Sets x value of the widget position.
 *  @param  x [in] x value of the widget position
 */
/*===========================================================================*/
void TransferFunctionEditor::Widget::setX( const int x )
{
    m_x = x;
}

/*===========================================================================*/
/**
 *  @brief  Sets y value of the widget position.
 *  @param  y [in] y value of the widget position
 */
/*===========================================================================*/
void TransferFunctionEditor::Widget::setY( const int y )
{
    m_y = y;
}

/*===========================================================================*/
/**
 *  @brief  Sets widget width.
 *  @param  width [in] widget width
 */
/*===========================================================================*/
void TransferFunctionEditor::Widget::setWidth( const int width )
{
    m_width = width;
}

/*===========================================================================*/
/**
 *  @brief  Sets widget height.
 *  @param  height [in] widget height
 */
/*===========================================================================*/
void TransferFunctionEditor::Widget::setHeight( const int height )
{
    m_height = height;
}

/*===========================================================================*/
/**
 *  @brief  Activates the widget.
 */
/*===========================================================================*/
void TransferFunctionEditor::Widget::activate( void )
{
    m_is_active = true;
}

/*===========================================================================*/
/**
 *  @brief  Deactivates the widget.
 */
/*===========================================================================*/
void TransferFunctionEditor::Widget::deactivate( void )
{
    m_is_active = false;
}

/*===========================================================================*/
/**
 *  @brief  Check whether the given point is inside the widget or not.
 *  @param  x [in] x position
 *  @param  y [in] y position
 *  @param  proper [in] whether the edges are checked or not
 *  @return true if the given point is inside the width
 */
/*===========================================================================*/
const bool TransferFunctionEditor::Widget::contains(
    const int x,
    const int y,
    const bool proper )
{
    const int x0 = m_x;
    const int x1 = m_x + m_width;
    const int y0 = m_y;
    const int y1 = m_y + m_height;

    if ( proper )
    {
        return( ( x0 < x ) && ( x < x1 ) && ( y0 < y ) && ( y < y1 ) );
    }
    else
    {
        return( ( x0 <= x ) && ( x <= x1 ) && ( y0 <= y ) && ( y <= y1 ) );
    }
}

/*===========================================================================*/
/**
 *  @brief  Initialize function for the transfer function editor.
 */
/*===========================================================================*/
void initialize_function( void )
{
    // Setup a texture for the color map.
    if ( !glIsTexture( ::current->m_color_map_texture.id() ) )
    {
        ::current->initialize_color_map_texture();
    }

    // Setup a texture for the opacity map.
    if ( !glIsTexture( ::current->m_opacity_map_texture.id() ) )
    {
        ::current->initialize_opacity_map_texture();
    }

    // Setup a texture for the cheackerboard under the opacity map.
    if ( !glIsTexture( ::current->m_checkerboard_texture.id() ) )
    {
        ::current->initialize_checkerboard_texture();
    }
}

/*===========================================================================*/
/**
 *  @brief  Paint event for the transfer function editor.
 */
/*===========================================================================*/
void paint_event( void )
{
    const float r = static_cast<float>( ::current->m_background_color.r() ) / 255.0f;
    const float g = static_cast<float>( ::current->m_background_color.g() ) / 255.0f;
    const float b = static_cast<float>( ::current->m_background_color.b() ) / 255.0f;
    glClearColor( r, g, b, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushAttrib( GL_ALL_ATTRIB_BITS );

    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    {
        glLoadIdentity();

        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        {
            glLoadIdentity();
            glOrtho( 0, ::current->m_width, 0, ::current->m_height, -1, 1 );

            ::current->draw_color_map_box();
            ::current->draw_opacity_map_box();

            ::current->draw_SV_palette();
            ::current->draw_H_palette();
            ::current->draw_selected_color_box();

            ::current->draw_selected_color_value();
            ::current->draw_color_map_value();

            ::current->draw_H_cursor();
            ::current->draw_S_cursor();
            ::current->draw_V_cursor();
            ::current->draw_scalar_cursor();
        }
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
    }
    glPopMatrix();

    glPopAttrib();

    glFlush();
    glutSwapBuffers();
}

/*===========================================================================*/
/**
 *  @brief  Resize event for the transfer function editor.
 *  @param  width [in] window width
 *  @param  height [in] window height
 */
/*===========================================================================*/
void resize_event( int width, int height )
{
    kvs::IgnoreUnusedVariable( width );
    kvs::IgnoreUnusedVariable( height );

    // Fix window size.
    glutReshapeWindow( ::current->m_width, ::current->m_height );

    glFlush();
    glutPostRedisplay();
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event for the transfer function editor.
 *  @param  button [in] button ID
 *  @param  state [in] state
 *  @param  x [in] x coordinate of the mouse cursor position
 *  @param  y [in] y coordinate of the mouse cursor position
 */
/*===========================================================================*/
void mouse_press_event( int button, int state, int x, int y )
{
    // Change to left-bottom origin.
    y = ::current->m_height - y;
    ::current->m_mouse->press( x, y );

    if ( kvs::glut::KVSMouseButton::Button( button ) == kvs::MouseButton::Left )
    {
        switch ( kvs::glut::KVSMouseButton::State( state ) )
        {
        case kvs::MouseButton::Down:
        {
            if ( ::current->m_SV_palette->contains( x, y, true ) )
            {
                ::current->m_SV_palette->activate();
                ::current->press_SV_palette();
            }

            if ( ::current->m_H_palette->contains( x, y, true ) )
            {
                ::current->m_H_palette->activate();
                ::current->press_H_palette();
            }

            if ( ::current->m_color_map_box->contains( x, y, true ) )
            {
                ::current->m_color_map_box->activate();
                ::current->press_color_map_box();
            }

            if ( ::current->m_opacity_map_box->contains( x, y, true ) )
            {
                ::current->m_opacity_map_box->activate();
                ::current->press_opacity_map_box();
            }

            break;
        }
        case kvs::MouseButton::Up:
        {
            ::current->m_SV_palette->deactivate();
            ::current->m_H_palette->deactivate();
            ::current->m_color_map_box->deactivate();
            ::current->m_opacity_map_box->deactivate();
            break;
        }
        default: break;
        }
    }

    glutPostRedisplay();
}

/*===========================================================================*/
/**
 *  @brief  Mouse move event for the transfer function editor.
 *  @param  x [in] x coordinate of the mouse cursor
 *  @param  y [in] y coordinate of the mouse cursor
 */
/*===========================================================================*/
void mouse_move_event( int x, int y )
{
    // Change to left-bottom origin.
    y = ::current->m_height - y;
    ::current->m_mouse->move( x, y );

    if ( ::current->m_SV_palette->isActive() )
    {
        ::current->move_SV_palette();
    }

    if ( ::current->m_H_palette->isActive() )
    {
        ::current->move_H_palette();
    }

    if ( ::current->m_color_map_box->isActive() )
    {
        ::current->move_color_map_box();
    }

    if ( ::current->m_opacity_map_box->isActive() )
    {
        ::current->move_opacity_map_box();
    }

    ::current->m_mouse->stop();

    glutPostRedisplay();
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new TransferFunctionEditor class.
 */
/*===========================================================================*/
TransferFunctionEditor::TransferFunctionEditor( void )
{
    m_id = 0;
    m_x = 0;
    m_y = 0;
    m_width = 350;
    m_height = 400;
    m_title = "TransferFunctionEditor";
    m_background_color = kvs::RGBColor( 200, 200, 200 );
    m_mouse = new TransferFunctionEditor::Mouse();
    m_SV_palette = new TransferFunctionEditor::Widget();
    m_H_palette = new TransferFunctionEditor::Widget();
    m_selected_color_box = new TransferFunctionEditor::Widget();
    m_color_map_box = new TransferFunctionEditor::Widget();
    m_opacity_map_box = new TransferFunctionEditor::Widget();

    this->lay_out_widgets();
}

/*===========================================================================*/
/**
 *  @brief  Destructs the TransferFunctionEditor class.
 */
/*===========================================================================*/
TransferFunctionEditor::~TransferFunctionEditor( void )
{
    delete m_mouse;
    delete m_SV_palette;
    delete m_H_palette;
    delete m_selected_color_box;
    delete m_color_map_box;
    delete m_opacity_map_box;
}

/*===========================================================================*/
/**
 *  @brief  Sets a window position.
 *  @param  x [in] x coordinate value
 *  @param  y [in] y coordinate value
 */
/*===========================================================================*/
void TransferFunctionEditor::setPosition( const size_t x, const size_t y )
{
    m_x = x;
    m_y = y;
}

/*===========================================================================*/
/**
 *  @brief  Sets a background color.
 *  @param  color [in] background color
 */
/*===========================================================================*/
void TransferFunctionEditor::setBackgroundColor( const kvs::RGBColor color )
{
    m_background_color = color;
}

/*===========================================================================*/
/**
 *  @brief  Hides the window.
 */
/*===========================================================================*/
void TransferFunctionEditor::hideWindow( void )
{
    glutSetWindow( m_id );
    glutHideWindow();
}

/*===========================================================================*/
/**
 *  @brief  Shows the window.
 */
/*===========================================================================*/
void TransferFunctionEditor::showWindow( void )
{
    glutSetWindow( m_id );
    glutShowWindow();
}

/*===========================================================================*/
/**
 *  @brief  Creates a transfer function editor.
 *  @param  show [in] check flag whether the window is shown or hidden
 */
/*===========================================================================*/
void TransferFunctionEditor::create( const bool show )
{
    // Initialize glut parameters.
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

    glutInitWindowPosition( m_x, m_y );
    glutInitWindowSize( m_width, m_height );

    // Create the window.
    m_id = glutCreateWindow( m_title.c_str() );

    ::current = this;

    // Set the call-back functions
    initialize_function();
    glutMouseFunc( mouse_press_event );
    glutMotionFunc( mouse_move_event );
    glutDisplayFunc( paint_event );
    glutReshapeFunc( resize_event );

    if( !show )
    {
        hideWindow();
    }
}

/*===========================================================================*/
/**
 *  @brief  Lay out the widgets.
 */
/*===========================================================================*/
void TransferFunctionEditor::lay_out_widgets( void )
{
    /*  Widget layout:
     *
     *   ------------------------------------
     *  | [SV palette label]                 |
     *  |  ----------------   --   --------  |
     *  | | SV             | |H | |selected| |
     *  | | palette        | |  | |color   | |
     *  | |                | |pa| |box     | |
     *  | |                | |le|  --------  |
     *  | |                | |tt| [selected] |
     *  | |                | |e | [col.val.] |
     *  |  ----------------   --             |
     *  | [col. map label]                   |
     *  |  --------------------------------  |
     *  | | color map box                  | |
     *  | |                                | |
     *  | |                                | |
     *  |  --------------------------------  |
     *  |  --------------------------------  |
     *  | | opacity map box                | |
     *  | |                                | |
     *  | |                                | |
     *  |  --------------------------------  |
     *  |                     [col.map val.] |
     *   ------------------------------------
     *
     *  origin: bottom-left
     */

    // Opacity map box.
    m_opacity_map_box->setX( 10 );
    m_opacity_map_box->setY( 30 );
    m_opacity_map_box->setWidth( 330 );
    m_opacity_map_box->setHeight( 80 );

    // Color map box.
    m_color_map_box->setX( 10 );
    m_color_map_box->setY( m_opacity_map_box->y() + m_opacity_map_box->height() + 5 );
    m_color_map_box->setWidth( 330 );
    m_color_map_box->setHeight( 80 );

    // SV (value and saturation) palette.
    m_SV_palette->setX( 10 );
    m_SV_palette->setY( m_color_map_box->y() + m_color_map_box->height() + 35 );
    m_SV_palette->setWidth( 250 );
    m_SV_palette->setHeight( 150 );

    // H (hue) palette.
    m_H_palette->setX( m_SV_palette->x() + m_SV_palette->width() + 10 );
    m_H_palette->setY( m_SV_palette->y() );
    m_H_palette->setWidth( 10 ); // fixed.
    m_H_palette->setHeight( 150 );

    // Selected color.
    m_selected_color_box->setX( m_H_palette->x() + m_H_palette->width() + 10 );
    m_selected_color_box->setY( m_H_palette->y() + m_H_palette->height() - 50 );
    m_selected_color_box->setWidth( 50 );
    m_selected_color_box->setHeight( 50 );

    // Cursors.
    m_H_cursor = m_H_palette->y0();
    m_S_cursor = m_SV_palette->x0() + m_SV_palette->width() / 2;
    m_V_cursor = m_SV_palette->y0() + m_SV_palette->height() / 2;
    m_scalar_cursor = m_opacity_map_box->x();

    // Labels.
    m_SV_palette_label.setPosition( m_SV_palette->x(), m_SV_palette->y() + m_SV_palette->height() + 5 );
    m_color_map_label.setPosition( m_color_map_box->x(), m_color_map_box->y() + m_color_map_box->height() + 5 );

    // Values.
    m_selected_color_value.setPosition( m_selected_color_box->x(), m_selected_color_box->y() - 20 );
    m_color_map_value.setPosition( m_opacity_map_box->x() + 135, m_opacity_map_box->y() - 20 );
}

/*===========================================================================*/
/**
 *  @brief  Draws SV (saturation and value) palette.
 */
/*===========================================================================*/
void TransferFunctionEditor::draw_SV_palette( void )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // SV palette geometry.
    const int x0 = m_SV_palette->x0();
    const int x1 = m_SV_palette->x1();
    const int y0 = m_SV_palette->y0();
    const int y1 = m_SV_palette->y1();

    // Colors on the corners.
    const float h = static_cast<float>( m_H_cursor - m_H_palette->y0()  ) / m_H_palette->height();
    const kvs::RGBColor c0( kvs::HSVColor( h, 1, 1 ) ); // top-right
    const kvs::RGBColor c1( kvs::HSVColor( h, 0, 1 ) ); // top-left
    const kvs::RGBColor c2( kvs::HSVColor( h, 0, 0 ) ); // bottom-left
    const kvs::RGBColor c3( kvs::HSVColor( h, 1, 0 ) ); // bottom-right

    // Draw SV palette label.
    m_SV_palette_label.setText("Brightness palette");
    m_SV_palette_label.draw();

    // Draw SV palette.
    glBegin( GL_QUADS );
    glColor3ub( c0.r(), c0.g(), c0.b() ); glVertex2i( x1, y1 );
    glColor3ub( c1.r(), c1.g(), c1.b() ); glVertex2i( x0, y1 );
    glColor3ub( c2.r(), c2.g(), c2.b() ); glVertex2i( x0, y0 );
    glColor3ub( c3.r(), c3.g(), c3.b() ); glVertex2i( x1, y0 );
    glEnd();

    // Draw the cross lines.
    const int margin = 4;
    const float color = 1.0f;
    glLineWidth( 1 );
    glBegin( GL_LINE_LOOP );
    glColor3f( color, color, color );
    glVertex2i( m_S_cursor + margin, m_V_cursor + margin );
    glVertex2i( m_S_cursor - margin, m_V_cursor + margin );
    glVertex2i( m_S_cursor - margin, m_V_cursor - margin );
    glVertex2i( m_S_cursor + margin, m_V_cursor - margin );
    glEnd();

    glLineWidth( 1 );
    glBegin( GL_LINES );
    glColor3f( color, color, color );
    // Horizontal lines.
    glVertex2i( x0, m_V_cursor );
    glVertex2i( m_S_cursor - margin, m_V_cursor );
    glVertex2i( m_S_cursor + margin, m_V_cursor );
    glVertex2i( x1, m_V_cursor );
    // Vertical lines.
    glVertex2i( m_S_cursor, y0 );
    glVertex2i( m_S_cursor, m_V_cursor - margin );
    glVertex2i( m_S_cursor, m_V_cursor + margin );
    glVertex2i( m_S_cursor, y1 );
    glEnd();

    // Draw border.
    ::DrawRectangle( m_SV_palette, 1, kvs::RGBColor( 0, 0, 0 ) );

    glPopAttrib();
}

/*===========================================================================*/
/**
 *  @brief  Draws H (hue) palette.
 */
/*===========================================================================*/
void TransferFunctionEditor::draw_H_palette( void )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // H palette geometry.
    const int x0 = m_H_palette->x0();
    const int y0 = m_H_palette->y0();
    const int y1 = m_H_palette->y1();

    const int width = m_H_palette->width();
    const int stride = ( y1 - y0 ) / 6;
    const int dx = width / 2;

    // Draw H palette.
    glLineWidth( width );
    glBegin( GL_LINE_STRIP );
    glColor3ub( 255,   0,   0 ); glVertex2i( x0 + dx, y1 );
    glColor3ub( 255,   0, 255 ); glVertex2i( x0 + dx, y0 + stride * 5 );
    glColor3ub(   0,   0, 255 ); glVertex2i( x0 + dx, y0 + stride * 4 );
    glColor3ub(   0, 255, 255 ); glVertex2i( x0 + dx, y0 + stride * 3 );
    glColor3ub(   0, 255,   0 ); glVertex2i( x0 + dx, y0 + stride * 2 );
    glColor3ub( 255, 255,   0 ); glVertex2i( x0 + dx, y0 + stride * 1 );
    glColor3ub( 255,   0,   0 ); glVertex2i( x0 + dx, y0 );
    glEnd();

    // Draw border.
    ::DrawRectangle( m_H_palette, 1, kvs::RGBColor( 0, 0, 0 ) );

    glPopAttrib();
}

/*===========================================================================*/
/**
 *  @brief  Draws selected color box.
 */
/*===========================================================================*/
void TransferFunctionEditor::draw_selected_color_box( void )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // selected color box geometry
    const int x0 = m_selected_color_box->x0();
    const int x1 = m_selected_color_box->x1();
    const int y0 = m_selected_color_box->y0();
    const int y1 = m_selected_color_box->y1();

    const kvs::RGBColor selected_color = this->get_selected_color();

    glBegin( GL_QUADS );
    glColor3ub( selected_color.r(), selected_color.g(), selected_color.b() );
    glVertex2i( x1, y1 );
    glVertex2i( x0, y1 );
    glVertex2i( x0, y0 );
    glVertex2i( x1, y0 );
    glEnd();

    // Draw border.
    ::DrawRectangle( m_selected_color_box, 1, kvs::RGBColor( 0, 0, 0 ) );

    glPopAttrib();
}

/*===========================================================================*/
/**
 *  @brief  Draws color map box.
 */
/*===========================================================================*/
void TransferFunctionEditor::draw_color_map_box( void )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // Color map box geometry.
    const int x0 = m_color_map_box->x0();
    const int x1 = m_color_map_box->x1();
    const int y0 = m_color_map_box->y0();
    const int y1 = m_color_map_box->y1();

    m_color_map_label.setText("Color/Opacity map");
    m_color_map_label.draw();

    glDisable( GL_BLEND );
    glEnable( GL_TEXTURE_1D );
    glDisable( GL_TEXTURE_2D );

    // Draw color map texture.
    m_color_map_texture.bind();
    glBegin( GL_QUADS );
    glTexCoord2f( 0.0f, 0.0f ); glVertex2i( x0, y0 );
    glTexCoord2f( 1.0f, 0.0f ); glVertex2i( x1, y0 );
    glTexCoord2f( 1.0f, 1.0f ); glVertex2i( x1, y1 );
    glTexCoord2f( 0.0f, 1.0f ); glVertex2i( x0, y1 );
    glEnd();
    m_color_map_texture.unbind();

    // Draw border.
    ::DrawRectangle( m_color_map_box, 1, kvs::RGBColor( 0, 0, 0 ) );

    glPopAttrib();
}

/*===========================================================================*/
/**
 *  @brief  Draws opacity map box.
 */
/*===========================================================================*/
void TransferFunctionEditor::draw_opacity_map_box( void )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // Color map box geometry.
    const int x0 = m_opacity_map_box->x0();
    const int x1 = m_opacity_map_box->x1();
    const int y0 = m_opacity_map_box->y0();
    const int y1 = m_opacity_map_box->y1();

    glPushAttrib( GL_ALL_ATTRIB_BITS );

    glDisable( GL_TEXTURE_1D );
    glEnable( GL_TEXTURE_2D );

    // Cheacker board.
    m_checkerboard_texture.bind();
    glBegin( GL_QUADS );
    const float s = 0.2f;
    const float w = ( m_width / 32.0f );
    const float h = ( m_height / 32.0f ) * s;
    glTexCoord2f( 0.0f, 0.0f ); glVertex2i( x0, y0 );
    glTexCoord2f( w,    0.0f ); glVertex2i( x1, y0 );
    glTexCoord2f( w,    h    ); glVertex2i( x1, y1 );
    glTexCoord2f( 0.0f, h    ); glVertex2i( x0, y1 );
    glEnd();
    m_checkerboard_texture.unbind();

    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_1D );
    glDisable( GL_TEXTURE_2D );

    // Opacity map.
    glBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_ALPHA );
    m_opacity_map_texture.bind();
    glBegin( GL_QUADS );
    glTexCoord2f( 0.0f, 0.0f ); glVertex2i( x0, y0 );
    glTexCoord2f( 1.0f, 0.0f ); glVertex2i( x1, y0 );
    glTexCoord2f( 1.0f, 1.0f ); glVertex2i( x1, y1 );
    glTexCoord2f( 0.0f, 1.0f ); glVertex2i( x0, y1 );
    glEnd();
    m_opacity_map_texture.unbind();

    // Color map.
    glBlendFunc( GL_SRC_ALPHA_SATURATE, GL_ONE );
    m_color_map_texture.bind();
    glBegin( GL_QUADS );
    glTexCoord2f( 0.0f, 0.0f ); glVertex2i( x0, y0 );
    glTexCoord2f( 1.0f, 0.0f ); glVertex2i( x1, y0 );
    glTexCoord2f( 1.0f, 1.0f ); glVertex2i( x1, y1 );
    glTexCoord2f( 0.0f, 1.0f ); glVertex2i( x0, y1 );
    glEnd();
    m_color_map_texture.unbind();

    glPopAttrib();

    // Draw lines.
    const int width = m_opacity_map_box->width();
    const int height = m_opacity_map_box->height();
    const int resolution = BaseClass::resolution();
    const float stride_x = static_cast<float>( width ) / ( resolution - 1 );
    const kvs::Real32* data = BaseClass::opacityMap().table().pointer();
    const kvs::Vector2f range_min( static_cast<float>(x0), static_cast<float>(y0) );
    const kvs::Vector2f range_max( static_cast<float>(x1), static_cast<float>(y1) );
    glLineWidth( 1 );
    glColor3ub( 0, 0, 0 );
    glBegin( GL_LINE_STRIP );
    for( int i = 0; i < resolution; i++ )
    {
        const float x = kvs::Math::Clamp( x0 + i * stride_x,     range_min.x(), range_max.x() );
        const float y = kvs::Math::Clamp( y0 + height * data[i], range_min.y(), range_max.y() );
        glVertex2f( x, y );
    }
    glEnd();

    // Draw border.
    ::DrawRectangle( m_opacity_map_box, 1, kvs::RGBColor( 0, 0, 0 ) );

    glPopAttrib();
}

/*===========================================================================*/
/**
 *  @brief  Draws H cursor.
 */
/*===========================================================================*/
void TransferFunctionEditor::draw_H_cursor( void )
{
    const int x0 = m_H_palette->x();

    glBegin( GL_TRIANGLES );
    glColor3ub( 50, 50, 50 ); glVertex2i( x0 - 2, m_H_cursor );
    glColor3ub( 50, 50, 50 ); glVertex2i( x0 - 8, m_H_cursor + 4 );
    glColor3ub( 50, 50, 50 ); glVertex2i( x0 - 8, m_H_cursor - 4 );
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Draws S cursor.
 */
/*===========================================================================*/
void TransferFunctionEditor::draw_S_cursor( void )
{
    const int y0 = m_SV_palette->y();

    glBegin( GL_TRIANGLES );
    glColor3ub( 50, 50, 50 ); glVertex2i( m_S_cursor,     y0 - 2 );
    glColor3ub( 50, 50, 50 ); glVertex2i( m_S_cursor + 4, y0 - 8 );
    glColor3ub( 50, 50, 50 ); glVertex2i( m_S_cursor - 4, y0 - 8 );
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Draws V cursor.
 */
/*===========================================================================*/
void TransferFunctionEditor::draw_V_cursor( void )
{
    const int x0 = m_SV_palette->x();

    glBegin( GL_TRIANGLES );
    glColor3ub( 50, 50, 50 ); glVertex2i( x0 - 2, m_V_cursor );
    glColor3ub( 50, 50, 50 ); glVertex2i( x0 - 8, m_V_cursor + 4 );
    glColor3ub( 50, 50, 50 ); glVertex2i( x0 - 8, m_V_cursor - 4 );
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Draws scalar cursor.
 */
/*===========================================================================*/
void TransferFunctionEditor::draw_scalar_cursor( void )
{
    const int y0 = m_opacity_map_box->y();

    glBegin( GL_TRIANGLES );
    glColor3ub( 50, 50, 50 ); glVertex2i( m_scalar_cursor,     y0 - 2 );
    glColor3ub( 50, 50, 50 ); glVertex2i( m_scalar_cursor + 4, y0 - 8 );
    glColor3ub( 50, 50, 50 ); glVertex2i( m_scalar_cursor - 4, y0 - 8 );
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Draws selected color value.
 */
/*===========================================================================*/
void TransferFunctionEditor::draw_selected_color_value( void )
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    const kvs::RGBColor selected_color = this->get_selected_color();
    const int r = static_cast<int>( selected_color.r() );
    const int g = static_cast<int>( selected_color.g() );
    const int b = static_cast<int>( selected_color.b() );
    const float h = static_cast<float>( m_H_cursor - m_SV_palette->y0() ) / m_SV_palette->height();
    const float s = static_cast<float>( m_S_cursor - m_SV_palette->x0() ) / m_SV_palette->width();
    const float v = static_cast<float>( m_V_cursor - m_SV_palette->y0() ) / m_SV_palette->height();

    m_selected_color_value.setText( "R: %3d", r );
    m_selected_color_value.addText( "G: %3d", g );
    m_selected_color_value.addText( "B: %3d", b );
    m_selected_color_value.addText( "" );
    m_selected_color_value.addText( "H: %3d", static_cast<int>( h * 255.0f + 0.5f ) );
    m_selected_color_value.addText( "S: %3d", static_cast<int>( s * 255.0f + 0.5f ) );
    m_selected_color_value.addText( "V: %3d", static_cast<int>( v * 255.0f + 0.5f ) );
    m_selected_color_value.draw();

    glPopAttrib();
}

/*===========================================================================*/
/**
 *  @brief  Draws color map value (RGBA).
 */
/*===========================================================================*/
void TransferFunctionEditor::draw_color_map_value( void )
{
    const float i = static_cast<float>( m_scalar_cursor - m_color_map_box->x0() ) / m_color_map_box->width();
    const size_t index = static_cast<size_t>( i * ( BaseClass::resolution() - 1.0f ) + 0.5f );

    const int r = static_cast<int>( BaseClass::colorMap().table().at( 3 * index ) );
    const int g = static_cast<int>( BaseClass::colorMap().table().at( 3 * index + 1 ) );
    const int b = static_cast<int>( BaseClass::colorMap().table().at( 3 * index + 2 ) );
    const float a = BaseClass::opacityMap().table().at( index );

    m_color_map_value.setText( "R:%-3d G:%-3d B:%-3d A:%0.2f", r, g, b, a );
    m_color_map_value.draw();
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event for the SV palette.
 */
/*===========================================================================*/
void TransferFunctionEditor::press_SV_palette( void )
{
    this->move_SV_palette();
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event for the H palette.
 */
/*===========================================================================*/
void TransferFunctionEditor::press_H_palette( void )
{
    this->move_H_palette();
}

/*===========================================================================*/
/**
 *  @brief  Mouse press event for the color map box.
 */
/*===========================================================================*/
void TransferFunctionEditor::press_color_map_box( void )
{
    // Color map box geometry.
    const int x0 = m_color_map_box->x0();
    const int x1 = m_color_map_box->x1();
    const int y0 = m_color_map_box->y0();
    const int y1 = m_color_map_box->y1();

    // Current mouse cursor position.
    const int x = m_mouse->current().x();
    const int y = m_mouse->current().y();

    // Update the scalar cursor position.
    m_scalar_cursor = x;

    const float resolution = static_cast<float>( BaseClass::resolution() );
    const float ratio = 1.0f - static_cast<float>( y1 - y ) / ( y1 - y0 );
    const int index = static_cast<int>( ( x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );

    // Update the color data.
    const kvs::RGBColor selected_color = this->get_selected_color();
    kvs::UInt8* data = const_cast<kvs::UInt8*>( BaseClass::colorMap().table().pointer() );
    kvs::UInt8* pdata = data;
    pdata = data + index * 3;
    pdata[0] = selected_color.r() * ratio + pdata[0] * ( 1 - ratio );
    pdata[1] = selected_color.g() * ratio + pdata[1] * ( 1 - ratio );
    pdata[2] = selected_color.b() * ratio + pdata[2] * ( 1 - ratio );

    const size_t width = BaseClass::colorMap().resolution();
    m_color_map_texture.bind();
    m_color_map_texture.download( width, data );
    m_color_map_texture.unbind();
}

/*===========================================================================*/
/**
 *  @brief  Mouse press opacity map box.
 */
/*===========================================================================*/
void TransferFunctionEditor::press_opacity_map_box( void )
{
    // Opacity map box geometry.
    const int x0 = m_opacity_map_box->x0();
    const int x1 = m_opacity_map_box->x1();
    const int y0 = m_opacity_map_box->y0();
    const int y1 = m_opacity_map_box->y1();

    // Current mouse cursor position.
    const int x = m_mouse->current().x();
    const int y = m_mouse->current().y();

    // Update the scalar cursor position.
    m_scalar_cursor = x;

    const float resolution = static_cast<float>( BaseClass::resolution() );
    const int index = static_cast<int>( ( x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const float opacity = static_cast<float>( y - y0 ) / ( y1 - y0 );

    // Update the color data.
    kvs::Real32* data = const_cast<kvs::Real32*>( BaseClass::opacityMap().table().pointer() );
    kvs::Real32* pdata = data;
    pdata = data + index;
    pdata[0] = opacity;

    const size_t width = BaseClass::opacityMap().resolution();
    m_opacity_map_texture.bind();
    m_opacity_map_texture.download( width, data );
    m_opacity_map_texture.unbind();
}

/*===========================================================================*/
/**
 *  @brief  Mouse move event for the SV palette.
 */
/*===========================================================================*/
void TransferFunctionEditor::move_SV_palette( void )
{
    const int x0 = m_SV_palette->x0();
    const int x1 = m_SV_palette->x1();
    m_S_cursor = kvs::Math::Clamp( m_mouse->current().x(), x0, x1 );

    const int y0 = m_SV_palette->y0();
    const int y1 = m_SV_palette->y1();
    m_V_cursor = kvs::Math::Clamp( m_mouse->current().y(), y0, y1 );
}

/*===========================================================================*/
/**
 *  @brief  Mouse move event for the H palette.
 */
/*===========================================================================*/
void TransferFunctionEditor::move_H_palette( void )
{
    const int y0 = m_H_palette->y0();
    const int y1 = m_H_palette->y1();
    m_H_cursor = kvs::Math::Clamp( m_mouse->current().y(), y0, y1 );
}

/*===========================================================================*/
/**
 *  @brief  Mouse move event for the color map box.
 */
/*===========================================================================*/
void TransferFunctionEditor::move_color_map_box( void )
{
    // Color map box geometry.
    const int x0 = m_color_map_box->x0();
    const int x1 = m_color_map_box->x1();
    const int y0 = m_color_map_box->y0();
    const int y1 = m_color_map_box->y1();

    // Current mouse cursor position.
    const int old_x = kvs::Math::Clamp( m_mouse->previous().x(), x0, x1 );
    const int old_y = kvs::Math::Clamp( m_mouse->previous().y(), y0, y1 );
    const int new_x = kvs::Math::Clamp( m_mouse->current().x(), x0,  x1 );
    const int new_y = kvs::Math::Clamp( m_mouse->current().y(), y0,  y1 );

    // Update the scalar cursor position.
    m_scalar_cursor = new_x;

    const float old_ratio = 1.0f - static_cast<float>( y1 - old_y ) / ( y1 - y0 );
    const float new_ratio = 1.0f - static_cast<float>( y1 - new_y ) / ( y1 - y0 );
    const float diff_ratio = new_ratio - old_ratio;

    const float resolution = static_cast<float>( BaseClass::resolution() );
    const int old_index = static_cast<int>( ( old_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const int new_index = static_cast<int>( ( new_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const float diff_index = static_cast<float>( new_index - old_index );

    const kvs::RGBColor selected_color = this->get_selected_color();
    const int begin_index = kvs::Math::Min( old_index, new_index );
    const int end_index = kvs::Math::Max( old_index, new_index );
    kvs::UInt8* data = const_cast<kvs::UInt8*>( BaseClass::colorMap().table().pointer() );
    kvs::UInt8* pdata = data + begin_index * 3;
    for ( int i = begin_index; i < end_index; i++, pdata += 3 )
    {
        const float ratio = ( i - old_index ) * diff_ratio / diff_index + old_ratio;
        pdata[0] = selected_color.r() * ratio + pdata[0] * ( 1 - ratio );
        pdata[1] = selected_color.g() * ratio + pdata[1] * ( 1 - ratio );
        pdata[2] = selected_color.b() * ratio + pdata[2] * ( 1 - ratio );
    }

    const size_t width = BaseClass::colorMap().resolution();
    m_color_map_texture.bind();
    m_color_map_texture.download( width, data );
    m_color_map_texture.unbind();
}

/*===========================================================================*/
/**
 *  @brief  Mouse move event for the opacity map box.
 */
/*===========================================================================*/
void TransferFunctionEditor::move_opacity_map_box( void )
{
    // Opacity map box geometry.
    const int x0 = m_opacity_map_box->x0();
    const int x1 = m_opacity_map_box->x1();
    const int y0 = m_opacity_map_box->y0();
    const int y1 = m_opacity_map_box->y1();

    // Current mouse cursor position.
    const int old_x = kvs::Math::Clamp( m_mouse->previous().x(), x0, x1 );
    const int old_y = kvs::Math::Clamp( m_mouse->previous().y(), y0, y1 );
    const int new_x = kvs::Math::Clamp( m_mouse->current().x(),  x0, x1 );
    const int new_y = kvs::Math::Clamp( m_mouse->current().y(),  y0, y1 );

    // Update the scalar cursor position.
    m_scalar_cursor = new_x;

    const float old_opacity = static_cast<float>( old_y - y0 ) / ( y1 - y0 );
    const float new_opacity = static_cast<float>( new_y - y0 ) / ( y1 - y0 );
    const float diff_opacity = new_opacity - old_opacity;

    const float resolution = static_cast<float>( BaseClass::resolution() );
    const int old_index = static_cast<int>( ( old_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const int new_index = static_cast<int>( ( new_x - x0 ) * resolution / ( x1 - x0 ) + 0.5f );
    const float diff_index = static_cast<float>( new_index - old_index );

    const int begin_index = kvs::Math::Min( old_index, new_index );
    const int end_index = kvs::Math::Max( old_index, new_index );
    kvs::Real32* data = const_cast<kvs::Real32*>( BaseClass::opacityMap().table().pointer() );
    kvs::Real32* pdata = data + begin_index;
    for ( int i = begin_index; i < end_index; i++ )
    {
        const float opacity = ( i - old_index ) * diff_opacity / diff_index + old_opacity;
        *(pdata++) = opacity;
    }

    const size_t width = BaseClass::opacityMap().resolution();
    m_opacity_map_texture.bind();
    m_opacity_map_texture.download( width, data );
    m_opacity_map_texture.unbind();
}

/*===========================================================================*/
/**
 *  @brief  Initializes a color map texture.
 */
/*===========================================================================*/
void TransferFunctionEditor::initialize_color_map_texture( void )
{
    const size_t nchannels  = 3; // rgb
    const size_t width = BaseClass::colorMap().resolution();
    const kvs::UInt8* data = BaseClass::colorMap().table().pointer();

    m_color_map_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_color_map_texture.setMinFilter( GL_LINEAR );
    m_color_map_texture.setMagFilter( GL_LINEAR );
    m_color_map_texture.create( width );
    m_color_map_texture.download( width, data );
}

/*===========================================================================*/
/**
 *  @brief  Initializes an opacity map texture.
 */
/*===========================================================================*/
void TransferFunctionEditor::initialize_opacity_map_texture( void )
{
    const size_t nchannels  = 1; // opacity
    const size_t width = BaseClass::opacityMap().resolution();
    const kvs::Real32* data = BaseClass::opacityMap().table().pointer();

    m_opacity_map_texture.setPixelFormat( nchannels, sizeof( kvs::Real32 ) );
    m_opacity_map_texture.setMinFilter( GL_LINEAR );
    m_opacity_map_texture.setMagFilter( GL_LINEAR );
    m_opacity_map_texture.create( width );
    m_opacity_map_texture.download( width, data );
}

/*===========================================================================*/
/**
 *  @brief  Initializes a checkerboard texture.
 */
/*===========================================================================*/
void TransferFunctionEditor::initialize_checkerboard_texture( void )
{
    const size_t nchannels = 3;
    const int width = 32;
    const int height = 32;

    GLubyte* data = new GLubyte [ width * height * nchannels ];
    if ( !data )
    {
        kvsMessageError("Cannot allocate for the checkerboard texture.");
        return;
    }

    GLubyte* pdata = data;
    const int c1 = 255; // checkerboard color (gray value) 1
    const int c2 = 230; // checkerboard color (gray value) 2
    for ( int i = 0; i < height; i++ )
    {
        for ( int j = 0; j < width; j++ )
        {
            int c = ((((i&0x8)==0)^((j&0x8)==0))) * c1;
            c = ( c == 0 ) ? c2 : c;
            *(pdata++) = static_cast<GLubyte>(c);
            *(pdata++) = static_cast<GLubyte>(c);
            *(pdata++) = static_cast<GLubyte>(c);
        }
    }

    m_checkerboard_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
    m_checkerboard_texture.setMinFilter( GL_NEAREST );
    m_checkerboard_texture.setMagFilter( GL_NEAREST );
    m_checkerboard_texture.setWrapS( GL_REPEAT );
    m_checkerboard_texture.setWrapT( GL_REPEAT );
    m_checkerboard_texture.create( width, height );
    m_checkerboard_texture.download( width, height, data );

    delete [] data;
}

/*===========================================================================*/
/**
 *  @brief  Returns the color value that is indicated by the HSV cursors.
 *  @return selected color value
 */
/*===========================================================================*/
const kvs::RGBColor TransferFunctionEditor::get_selected_color( void )
{
    const float h = static_cast<float>( m_H_cursor - m_H_palette->y0()  ) / m_H_palette->height();
    const kvs::RGBColor c0( kvs::HSVColor( h, 1, 1 ) ); // top-right
    const kvs::RGBColor c1( kvs::HSVColor( h, 0, 1 ) ); // top-left
    const kvs::RGBColor c2( kvs::HSVColor( h, 0, 0 ) ); // bottom-left
    const kvs::RGBColor c3( kvs::HSVColor( h, 1, 0 ) ); // bottom-right

    const float v = static_cast<float>( m_V_cursor - m_SV_palette->y0() ) / m_SV_palette->height();
    const kvs::RGBColor v0( ::GetInterpolatedColor( v, c1, c2 ) );
    const kvs::RGBColor v1( ::GetInterpolatedColor( v, c0, c3 ) );

    const float s = static_cast<float>( m_S_cursor - m_SV_palette->x0() ) / m_SV_palette->width();
    return( ::GetInterpolatedColor( s, v1, v0 ) );
}

} // end of namespace old

} // end of namespace glut

} // end of namespace kvs
