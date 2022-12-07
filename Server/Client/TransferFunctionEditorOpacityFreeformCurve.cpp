/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorOpacityFreeformCurve.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorOpacityFreeformCurve.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TransferFunctionEditorOpacityFreeformCurve.h"
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

class ResetButton : public kvs::glut::PushButton
{
    kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* m_editor;

public:

    ResetButton( kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* editor ):
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
    kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* m_editor;

public:

    CancelButton( kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* editor ):
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
    kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* m_editor;

public:

    SaveButton( kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* editor ):
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
    kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* m_editor;

public:

    UndoButton( kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* editor ):
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
    kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* m_editor;

public:

    RedoButton( kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* editor ):
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


TransferFunctionEditorOpacityFreeformCurve::TransferFunctionEditorOpacityFreeformCurve( kvs::visclient::TransferFunctionEditorMain* parent ):
    m_parent( parent ),
    m_screen( parent ),
    m_opacity_map_palette( NULL ),
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
    const std::string title = "Opacity Map Editor (freeform curve)";
    const int x = ( parent != 0 ) ? parent->x() + parent->width() + 5 : 0;
    const int y = ( parent != 0 ) ? parent->y() : 0;
    const int width = 350;
    const int height = 180;
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

    m_opacity_map_palette = new kvs::glut::OpacityMapPalette( this );
    m_opacity_map_palette->setCaption( "Opacity" );
    m_opacity_map_palette->setOpacityMap( m_initial_transfer_function.opacityMap() );
//  m_opacity_map_palette->setX( m_color_map_palette->x0() );
//  m_opacity_map_palette->setY( m_color_map_palette->y1() - m_color_map_palette->margin() );
    m_opacity_map_palette->setHeight( 100 );
    m_opacity_map_palette->show();

    const size_t button_margin = 5;
    const size_t button_width = ( width - 2 * margin - button_margin ) / 2;

    m_reset_button = new ::ResetButton( this );
    m_reset_button->setCaption( "Reset" );
    m_reset_button->setX( m_opacity_map_palette->x0() + m_opacity_map_palette->margin() );
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

    m_cancel_button = new ::CancelButton( this );
    m_cancel_button->setCaption( "Cancel" );
    m_cancel_button->setX( m_save_button->x1() + button_margin );
    m_cancel_button->setY( m_save_button->y0() );
    m_cancel_button->setWidth( ( width - margin ) / 2 - m_opacity_map_palette->margin() );
    m_cancel_button->show();
}

TransferFunctionEditorOpacityFreeformCurve::~TransferFunctionEditorOpacityFreeformCurve()
{
    if ( m_stack_event ) delete m_stack_event;
    if ( m_opacity_map_palette ) delete m_opacity_map_palette;
    if ( m_reset_button ) delete m_reset_button;
    if ( m_undo_button ) delete m_undo_button;
    if ( m_redo_button ) delete m_redo_button;
    if ( m_cancel_button ) delete m_cancel_button;
    if ( m_save_button ) delete m_save_button;
}

void TransferFunctionEditorOpacityFreeformCurve::setResolution( const size_t resolution )
{
    m_resolution = resolution;
    setTransferFunction( kvs::TransferFunction( resolution ) );
}

int TransferFunctionEditorOpacityFreeformCurve::show()
{
    if ( m_visible ) return m_id;
    m_visible = true;

    int id = SuperClass::show();
    this->redraw();
    return id;
}

void TransferFunctionEditorOpacityFreeformCurve::hide()
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

kvs::ScreenBase* TransferFunctionEditorOpacityFreeformCurve::screen()
{
    return m_screen;
}

const kvs::glut::OpacityMapPalette* TransferFunctionEditorOpacityFreeformCurve::opacityMapPalette() const
{
    return m_opacity_map_palette;
}

const kvs::OpacityMap TransferFunctionEditorOpacityFreeformCurve::opacityMap() const
{
    return m_opacity_map_palette->opacityMap();
}

const kvs::TransferFunction TransferFunctionEditorOpacityFreeformCurve::m_transfer_function() const
{
//    const kvs::Real32 min_value = m_initial_transfer_function.colorMap().minValue();
//    const kvs::Real32 max_value = m_initial_transfer_function.colorMap().maxValue();
//  kvs::TransferFunction m_transfer_function( this->colorMap(), this->opacityMap() );
//    m_transfer_function.setRange( min_value, max_value );
    kvs::TransferFunction m_transfer_function( this->opacityMap() );
    m_transfer_function.setRange( m_min_value, m_max_value );

    return m_transfer_function;
}

const size_t TransferFunctionEditorOpacityFreeformCurve::undoStackSize() const
{
    return m_undo_stack.size();
}

const size_t TransferFunctionEditorOpacityFreeformCurve::redoStackSize() const
{
    return m_redo_stack.size();
}

const size_t TransferFunctionEditorOpacityFreeformCurve::maxStackSize() const
{
    return m_max_stack_size;
}

void TransferFunctionEditorOpacityFreeformCurve::setTransferFunction( const kvs::TransferFunction& m_transfer_function )
{
    const kvs::OpacityMap& omap = m_transfer_function.opacityMap();

    // Deep copy for the initial transfer function.
    kvs::OpacityMap::Table opacity_map_table( omap.table().pointer(), omap.table().size() );
//    kvs::ColorMap color_map( color_map_table, cmap.minValue(), cmap.maxValue() );
//    kvs::OpacityMap opacity_map( opacity_map_table, omap.minValue(), omap.maxValue() );
    kvs::OpacityMap opacity_map( opacity_map_table );
    m_initial_transfer_function.setOpacityMap( opacity_map );

    if ( m_transfer_function.hasRange() )
    {
        m_min_value = m_transfer_function.opacityMap().minValue();
        m_max_value = m_transfer_function.opacityMap().maxValue();
        m_initial_transfer_function.setRange( m_min_value, m_max_value );
    }

    m_opacity_map_palette->setOpacityMap( opacity_map );

    m_undo_stack.clear();
    m_undo_stack.push_front( m_initial_transfer_function );
}

void TransferFunctionEditorOpacityFreeformCurve::setVolumeObject( const kvs::VolumeObjectBase& object )
{
    if ( !m_initial_transfer_function.hasRange() )
    {
        m_min_value = object.minValue();
        m_max_value = object.maxValue();
    }

//  m_histogram->create( &object );
}

void TransferFunctionEditorOpacityFreeformCurve::setMaxStackSize( const size_t stack_size )
{
    m_max_stack_size = stack_size;
}

void TransferFunctionEditorOpacityFreeformCurve::reset()
{
    m_opacity_map_palette->setOpacityMap( m_initial_transfer_function.opacityMap() );

    this->redraw();
}

void TransferFunctionEditorOpacityFreeformCurve::cancel()
{
    this->hide();
}

void TransferFunctionEditorOpacityFreeformCurve::save()
{
    m_parent->saveTransferFunctionEditorOpacityFreeformCurve();
}

void TransferFunctionEditorOpacityFreeformCurve::undo()
{
    if ( m_undo_stack.size() > 1 )
    {
        if ( m_redo_stack.size() > m_max_stack_size ) m_redo_stack.pop_back();
        m_redo_stack.push_front( m_undo_stack.front() );
        m_undo_stack.pop_front();

        kvs::TransferFunction& m_transfer_function = m_undo_stack.front();
        m_opacity_map_palette->setOpacityMap( m_transfer_function.opacityMap() );

        this->redraw();
    }
}

void TransferFunctionEditorOpacityFreeformCurve::redo()
{
    if ( m_redo_stack.size() > 1 )
    {
        kvs::TransferFunction& m_transfer_function = m_redo_stack.front();
        m_opacity_map_palette->setOpacityMap( m_transfer_function.opacityMap() );

        if ( m_undo_stack.size() > m_max_stack_size ) m_undo_stack.pop_back();
        m_undo_stack.push_front( m_redo_stack.front() );
        m_redo_stack.pop_front();

        this->redraw();
    }
}

TransferFunctionEditorOpacityFreeformCurve::StackEvent::StackEvent(
    kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* editor ):
    m_editor( editor )
{
}

void TransferFunctionEditorOpacityFreeformCurve::StackEvent::update( kvs::MouseEvent* event )
{
    if ( m_editor->opacityMapPalette()->palette().isActive() )
    {
        if ( m_editor->m_undo_stack.size() > m_editor->m_max_stack_size ) m_editor->m_undo_stack.pop_back();
        m_editor->m_undo_stack.push_front( m_editor->m_transfer_function() );
        m_editor->redraw();
    }
}

} // end of namespace visclient

} // end of namespace kvs
