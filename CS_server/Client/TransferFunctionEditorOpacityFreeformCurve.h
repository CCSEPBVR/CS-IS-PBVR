/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorOpacityFreeformCurve.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorOpacityFreeformCurve.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_OPACITY_FREEFORM_CURVE_H_INCLUDE
#define PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_OPACITY_FREEFORM_CURVE_H_INCLUDE

#include <string>
#include <kvs/ClassName>
#include <kvs/OpenGL>
#include <kvs/RGBColor>
#include <kvs/RGBAColor>
#include <kvs/Vector2>
#include <kvs/TransferFunction>
#include <kvs/Texture1D>
#include <kvs/Texture2D>
#include <kvs/MouseReleaseEventListener>
#include <kvs/glut/Screen>
#include <kvs/glut/PushButton>
#include <kvs/glut/Text>
#include <kvs/glut/ColorPalette>
#include <kvs/glut/ColorMapPalette>
#include <kvs/glut/OpacityMapPalette>
#include <kvs/glut/Histogram>


namespace kvs
{

namespace visclient
{

class TransferFunctionEditorMain;

/*===========================================================================*/
/**
 *  @brief  Transfer function editor class.
 */
/*===========================================================================*/
class TransferFunctionEditorOpacityFreeformCurve : public kvs::glut::Screen
{
    kvsClassName( kvs::visclient::TransferFunctionEditorOpacityFreeformCurve );

public:

    typedef kvs::glut::Screen SuperClass;

private:

    class StackEvent;

    std::deque<kvs::TransferFunction> m_undo_stack;     ///< undo stack
    std::deque<kvs::TransferFunction> m_redo_stack;     ///< redo stack
    StackEvent*                       m_stack_event;    ///< stack event
    size_t                            m_max_stack_size; ///< max. stack size

protected:

    bool m_visible;
    size_t m_resolution;

    kvs::ScreenBase*              m_screen;              ///< pointer to screen
    kvs::glut::OpacityMapPalette* m_opacity_map_palette; ///< opacity map palette
    kvs::glut::PushButton*        m_reset_button;        ///< reset button
    kvs::glut::PushButton*        m_undo_button;         ///< undo button
    kvs::glut::PushButton*        m_redo_button;         ///< redo button
    kvs::glut::PushButton*        m_save_button;         ///< save button
    kvs::glut::PushButton*        m_cancel_button;       ///< cancel button
    kvs::TransferFunction m_initial_transfer_function;   ///< initial transfer function
    kvs::Real32           m_min_value; ///< min value
    kvs::Real32           m_max_value; ///< max value

    kvs::visclient::TransferFunctionEditorMain* m_parent;

public:

    TransferFunctionEditorOpacityFreeformCurve( kvs::visclient::TransferFunctionEditorMain* parent = 0 );

    virtual ~TransferFunctionEditorOpacityFreeformCurve();

public:

    void setResolution( const size_t resolution );

public:

    int show();

    virtual void hide();

public:

    kvs::ScreenBase* screen();

    const kvs::glut::OpacityMapPalette* opacityMapPalette() const;

    const kvs::OpacityMap opacityMap() const;

    const kvs::TransferFunction m_transfer_function() const;

    const size_t undoStackSize() const;

    const size_t redoStackSize() const;

    const size_t maxStackSize() const;

public:

    void setTransferFunction( const kvs::TransferFunction& m_transfer_function );

    void setVolumeObject( const kvs::VolumeObjectBase& object );

    void setMaxStackSize( const size_t stack_size );

public:

    virtual void reset();

    virtual void undo();

    virtual void redo();

    virtual void save();

    virtual void cancel();
};

/*===========================================================================*/
/**
 *  @brief  Stack event for undo/redo.
 */
/*===========================================================================*/
class TransferFunctionEditorOpacityFreeformCurve::StackEvent : public kvs::MouseReleaseEventListener
{
    kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* m_editor;

public:

    StackEvent( kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* editor );

    void update( kvs::MouseEvent* event );
};

} // end of namespace visclient

} // end of namespace kvs

#endif // PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_OPACITY_FREEFORM_CURVE_H_INCLUDE
