/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorOpacityControlPoints.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorOpacityControlPoints.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_OPACITY_CONTROL_POINTS_H_INCLUDE
#define PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_OPACITY_CONTROL_POINTS_H_INCLUDE

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
#include <kvs/glut/OpacityMapPalette>

#include "GL/glui.h"

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
class TransferFunctionEditorOpacityControlPoints : public kvs::glut::Screen
{
    kvsClassName( kvs::visclient::TransferFunctionEditorOpacityControlPoints );

public:

    typedef kvs::glut::Screen SuperClass;

protected:

    bool m_visible;
    size_t m_resolution;

    kvs::ScreenBase*              m_screen;              ///< pointer to screen
    kvs::glut::OpacityMapPalette* m_opacity_map_palette; ///< opacity map palette
    kvs::TransferFunction m_initial_transfer_function;   ///< initial transfer function
    kvs::Real32           m_min_value; ///< min value
    kvs::Real32           m_max_value; ///< max value

    kvs::visclient::TransferFunctionEditorMain* m_parent;

    GLUI* m_glui_text_win;
    GLUI* m_glui_buttom_win;
    GLUI_EditText* m_glui_edit_c[10];
    GLUI_EditText* m_glui_edit_o[10];

public:

    TransferFunctionEditorOpacityControlPoints( kvs::visclient::TransferFunctionEditorMain* parent = 0 );

    virtual ~TransferFunctionEditorOpacityControlPoints();

public:

    void setResolution( const size_t resolution );

public:

    kvs::ScreenBase* screen();

    const kvs::glut::OpacityMapPalette* opacityMapPalette() const;

    const kvs::OpacityMap opacityMap() const;

    const kvs::TransferFunction m_transfer_function() const;

public:

    int show();

    virtual void hide();

public:

    void setTransferFunction( const kvs::TransferFunction& transfer_function );

public:

    void setControlPointValue( const size_t n,  const float cp, const float opacity );

public:

    virtual void update();

    virtual void save();

    virtual void cancel();
};

} // end of namespace visclient

} // end of namespace kvs

#endif // PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_OPACITY_CONTROL_POINTS_H_INCLUDE
