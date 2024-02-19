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
#include "TransferFunctionEditorColorFreeformCurve.h"
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

class ResetButton : public kvs::glut::PushButton
{
    kvs::visclient::TransferFunctionEditorColorFreeformCurve* m_editor;

public:

    ResetButton( kvs::visclient::TransferFunctionEditorColorFreeformCurve* editor ):
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released()
    {
        m_editor->reset();
    }
};

class CancelButton : public kvs::glut::PushButton
{
    kvs::visclient::TransferFunctionEditorColorFreeformCurve* m_editor;

public:

    CancelButton( kvs::visclient::TransferFunctionEditorColorFreeformCurve* editor ):
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
    kvs::visclient::TransferFunctionEditorColorFreeformCurve* m_editor;

public:

    SaveButton( kvs::visclient::TransferFunctionEditorColorFreeformCurve* editor ):
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released()
    {
        m_editor->save();
    }
};

class UndoButton : public kvs::glut::PushButton
{
    kvs::visclient::TransferFunctionEditorColorFreeformCurve* m_editor;

public:

    UndoButton( kvs::visclient::TransferFunctionEditorColorFreeformCurve* editor ):
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released()
    {
        m_editor->undo();
    }
};

class RedoButton : public kvs::glut::PushButton
{
    kvs::visclient::TransferFunctionEditorColorFreeformCurve* m_editor;

public:

    RedoButton( kvs::visclient::TransferFunctionEditorColorFreeformCurve* editor ):
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released()
    {
        m_editor->redo();
    }
};

} // end of namespace


namespace kvs
{

namespace visclient
{


TransferFunctionEditorColorFreeformCurve::TransferFunctionEditorColorFreeformCurve( kvs::visclient::TransferFunctionEditorMain* parent ):
    m_parent( parent ),
    m_screen( parent ),
    m_color_palette( NULL ),
    m_color_map_palette( NULL ),
    m_histogram( NULL ),
    m_reset_button( NULL ),
    m_undo_button( NULL ),
    m_redo_button( NULL ),
    m_save_button( NULL ),
    m_cancel_button( NULL ),
    m_min_value( 0.0f ),
    m_max_value( 0.0f ),
    m_resolution( 256 ),
    m_visible( false )
{
    const std::string title = "Color Map Editor (freeform curve)";
    const int x = ( parent != 0 ) ? parent->x() + parent->width() + 5 : 0;
    const int y = ( parent != 0 ) ? parent->y() : 0;
    const int width = 350;
#ifndef PBVR_NO_HISTOGRAM
    const int height = 420;
#else
    const int height = 330;
#endif
    const int margin = 10;
    const kvs::RGBColor color( 200, 200, 200 );

    SuperClass::background()->setColor( color );
    SuperClass::setTitle( title );
    SuperClass::setPosition( x, y );
    SuperClass::setSize( width, height );

    m_initial_transfer_function.create( m_resolution );

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

#ifndef PBVR_NO_HISTOGRAM
    m_histogram = new kvs::glut::Histogram( this );
    m_histogram->setCaption( "Histogram" );
    m_histogram->setX( m_color_map_palette->x0() );
    m_histogram->setY( m_color_map_palette->y1() - m_color_map_palette->margin() );
    m_histogram->setHeight( 100 );
    m_histogram->setGraphColor( kvs::RGBAColor( 100, 100, 100, 1.0f ) );
//  m_histogram->show();
#endif

    const size_t button_margin = 5;
    const size_t button_width = ( width - 2 * margin - button_margin ) / 2;

    m_reset_button = new ::ResetButton( this );
    m_reset_button->setCaption( "Reset" );
#ifndef PBVR_NO_HISTOGRAM
    m_reset_button->setX( m_histogram->x0() + m_histogram->margin() );
    m_reset_button->setY( m_histogram->y1() );
#else
    m_reset_button->setX( m_color_map_palette->x0() + m_color_map_palette->margin() );
    m_reset_button->setY( m_color_map_palette->y1() );
#endif
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

    m_cancel_button = new ::CancelButton( this );
    m_cancel_button->setCaption( "Cancel" );
    m_cancel_button->setX( m_save_button->x1() + button_margin );
    m_cancel_button->setY( m_save_button->y0() );
    m_cancel_button->setWidth( ( width - margin ) / 2 - m_color_map_palette->margin() );
    m_cancel_button->show();
}

TransferFunctionEditorColorFreeformCurve::~TransferFunctionEditorColorFreeformCurve()
{
    if ( m_stack_event ) delete m_stack_event;
    if ( m_color_palette ) delete m_color_palette;
    if ( m_color_map_palette ) delete m_color_map_palette;
    if ( m_histogram ) delete m_histogram;
    if ( m_reset_button ) delete m_reset_button;
    if ( m_undo_button ) delete m_undo_button;
    if ( m_redo_button ) delete m_redo_button;
    if ( m_cancel_button ) delete m_cancel_button;
    if ( m_save_button ) delete m_save_button;
}

void TransferFunctionEditorColorFreeformCurve::setResolution( const size_t resolution )
{
    m_resolution = resolution;
    kvs::ColorMap color_map = kvs::ColorMap( resolution );
    setTransferFunction( kvs::TransferFunction( color_map ) );
}

int TransferFunctionEditorColorFreeformCurve::show()
{
    if ( m_visible ) return m_id;
    m_visible = true;

    int id = SuperClass::show();
    this->redraw();
    return id;
}

void TransferFunctionEditorColorFreeformCurve::hide()
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

kvs::ScreenBase* TransferFunctionEditorColorFreeformCurve::screen()
{
    return m_screen;
}

const kvs::glut::ColorPalette* TransferFunctionEditorColorFreeformCurve::colorPalette() const
{
    return m_color_palette;
}

const kvs::glut::ColorMapPalette* TransferFunctionEditorColorFreeformCurve::colorMapPalette() const
{
    return m_color_map_palette;
}

const kvs::ColorMap TransferFunctionEditorColorFreeformCurve::colorMap() const
{
    return m_color_map_palette->colorMap();
}

const kvs::TransferFunction TransferFunctionEditorColorFreeformCurve::m_transfer_function() const
{
//    const kvs::Real32 min_value = m_initial_transfer_function.colorMap().minValue();
//    const kvs::Real32 max_value = m_initial_transfer_function.colorMap().maxValue();
//  kvs::TransferFunction m_transfer_function( this->colorMap(), this->opacityMap() );
//    m_transfer_function.setRange( min_value, max_value );
    kvs::TransferFunction m_transfer_function( this->colorMap() );
    m_transfer_function.setRange( m_min_value, m_max_value );

    return m_transfer_function;
}

const size_t TransferFunctionEditorColorFreeformCurve::undoStackSize() const
{
    return m_undo_stack.size();
}

const size_t TransferFunctionEditorColorFreeformCurve::redoStackSize() const
{
    return m_redo_stack.size();
}

const size_t TransferFunctionEditorColorFreeformCurve::maxStackSize() const
{
    return m_max_stack_size;
}

void TransferFunctionEditorColorFreeformCurve::setTransferFunction( const kvs::TransferFunction& transfer_function )
{
    const kvs::ColorMap& cmap = transfer_function.colorMap();
//  const kvs::OpacityMap& omap = transfer_function.opacityMap();

    // Deep copy for the initial transfer function.
    kvs::ColorMap::Table color_map_table( cmap.table().pointer(), cmap.table().size() );
//    kvs::ColorMap color_map( color_map_table, cmap.minValue(), cmap.maxValue() );
    kvs::ColorMap color_map( color_map_table );
    m_initial_transfer_function.setColorMap( color_map );

    if ( transfer_function.hasRange() )
    {
        m_min_value = transfer_function.colorMap().minValue();
        m_max_value = transfer_function.colorMap().maxValue();
        m_initial_transfer_function.setRange( m_min_value, m_max_value );
    }

    m_color_map_palette->setColorMap( color_map );

    m_undo_stack.clear();
    m_undo_stack.push_front( m_initial_transfer_function );
}

void TransferFunctionEditorColorFreeformCurve::setVolumeObject( const kvs::VolumeObjectBase& object )
{
    if ( !m_initial_transfer_function.hasRange() )
    {
        m_min_value = object.minValue();
        m_max_value = object.maxValue();
    }

    m_histogram->create( &object );
}

void TransferFunctionEditorColorFreeformCurve::setMaxStackSize( const size_t stack_size )
{
    m_max_stack_size = stack_size;
}

void TransferFunctionEditorColorFreeformCurve::reset()
{
    m_color_map_palette->setColorMap( m_initial_transfer_function.colorMap() );

    this->redraw();
}

void TransferFunctionEditorColorFreeformCurve::cancel()
{
    this->hide();
}

void TransferFunctionEditorColorFreeformCurve::save()
{
    m_parent->saveTransferFunctionEditorColorFreeformCurve();
}

void TransferFunctionEditorColorFreeformCurve::undo()
{
    if ( m_undo_stack.size() > 1 )
    {
        if ( m_redo_stack.size() > m_max_stack_size ) m_redo_stack.pop_back();
        m_redo_stack.push_front( m_undo_stack.front() );
        m_undo_stack.pop_front();

        kvs::TransferFunction& m_transfer_function = m_undo_stack.front();
        m_color_map_palette->setColorMap( m_transfer_function.colorMap() );

        this->redraw();
    }
}

void TransferFunctionEditorColorFreeformCurve::redo()
{
    if ( m_redo_stack.size() > 1 )
    {
        kvs::TransferFunction& m_transfer_function = m_redo_stack.front();
        m_color_map_palette->setColorMap( m_transfer_function.colorMap() );

        if ( m_undo_stack.size() > m_max_stack_size ) m_undo_stack.pop_back();
        m_undo_stack.push_front( m_redo_stack.front() );
        m_redo_stack.pop_front();

        this->redraw();
    }
}

TransferFunctionEditorColorFreeformCurve::StackEvent::StackEvent(
    kvs::visclient::TransferFunctionEditorColorFreeformCurve* editor ):
    m_editor( editor )
{
}

void TransferFunctionEditorColorFreeformCurve::StackEvent::update( kvs::MouseEvent* event )
{
    if ( m_editor->colorMapPalette()->palette().isActive() )
    {
        if ( m_editor->m_undo_stack.size() > m_editor->m_max_stack_size ) m_editor->m_undo_stack.pop_back();
        m_editor->m_undo_stack.push_front( m_editor->m_transfer_function() );
        m_editor->redraw();
    }
}

} // end of namespace visclient

} // end of namespace kvs
