/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorColorControlPoints.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorColorControlPoints.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_COLOR_CONTROL_POINTS_H_INCLUDE
#define PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_COLOR_CONTROL_POINTS_H_INCLUDE

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
#include <kvs/glut/ColorMapPalette>
#include <kvs/glut/Histogram>

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
class TransferFunctionEditorColorControlPoints : public kvs::glut::Screen
{
    kvsClassName( kvs::visclient::TransferFunctionEditorColorControlPoints );

public:

    typedef kvs::glut::Screen SuperClass;

protected:

    bool m_visible;
    size_t m_resolution;

    kvs::ScreenBase*              m_screen;              ///< pointer to screen
    kvs::glut::ColorMapPalette*   m_color_map_palette;   ///< color map palette
    kvs::glut::Histogram*         m_histogram;           ///< histogram
    kvs::TransferFunction m_initial_transfer_function;   ///< initial transfer function
    kvs::Real32           m_min_value; ///< min value
    kvs::Real32           m_max_value; ///< max value

    kvs::visclient::TransferFunctionEditorMain* m_parent;

    GLUI* m_glui_text_win;
    GLUI* m_glui_buttom_win;
    GLUI_EditText* m_glui_edit_c[10];
    GLUI_EditText* m_glui_edit_r[10];
    GLUI_EditText* m_glui_edit_g[10];
    GLUI_EditText* m_glui_edit_b[10];

public:

    TransferFunctionEditorColorControlPoints( kvs::visclient::TransferFunctionEditorMain* parent = 0 );

    virtual ~TransferFunctionEditorColorControlPoints();

public:

    int show();

    virtual void hide();

public:

    void setResolution( const size_t resolution );

public:

    kvs::ScreenBase* screen();

    const kvs::glut::ColorMapPalette* colorMapPalette() const;

    const kvs::ColorMap colorMap() const;

    const kvs::TransferFunction m_transfer_function() const;

public:

    void setTransferFunction( const kvs::TransferFunction& transfer_function );

    void setVolumeObject( const kvs::VolumeObjectBase& object );

public:

    //void setControlPointValue( size_t n,  float cp, kvs::UInt8 r, kvs::UInt8 g, kvs::UInt8 b );
    void setControlPointValue( const size_t n, const float cp, const float r, const float g, const float b );

public:

    virtual void update();

    virtual void save();

    virtual void cancel();
};

} // end of namespace visclient

} // end of namespace kvs

#endif // PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_COLOR_CONTROL_POINTS_H_INCLUDE
