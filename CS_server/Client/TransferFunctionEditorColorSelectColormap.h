/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorColorSelectColormap.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorColorSelectColormap.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_COLOR_SELECT_COLORMAP_H_INCLUDE
#define PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_COLOR_SELECT_COLORMAP_H_INCLUDE

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

#include "ColorMapLibrary.h"

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
class TransferFunctionEditorColorSelectColormap : public kvs::glut::Screen
{
    kvsClassName( kvs::visclient::TransferFunctionEditorColorSelectColormap );

public:

    typedef kvs::glut::Screen SuperClass;

protected:

    bool m_visible;
    size_t m_resolution;

    kvs::ScreenBase*              m_screen;              ///< pointer to screen
    kvs::glut::ColorMapPalette*   m_color_map_palette;   ///< color map palette
    kvs::TransferFunction m_initial_transfer_function;   ///< initial transfer function
    kvs::Real32           m_min_value; ///< min value
    kvs::Real32           m_max_value; ///< max value

    kvs::visclient::TransferFunctionEditorMain* m_parent;

    GLUI* m_glui_text_win;
    GLUI* m_glui_buttom_win;
    GLUI_Listbox* m_glui_color;

    ColorMapLibrary* m_color_map_library;

public:

    TransferFunctionEditorColorSelectColormap( kvs::visclient::TransferFunctionEditorMain* parent = 0 );

    virtual ~TransferFunctionEditorColorSelectColormap();

public:

    void setResolution( const size_t resolution );

public:

    int show();

    virtual void hide();

public:

    kvs::ScreenBase* screen();

    const kvs::glut::ColorMapPalette* colorMapPalette() const;

    const kvs::ColorMap colorMap() const;

    const kvs::TransferFunction m_transfer_function() const;

public:

    void setColorMapName( const std::string& name );

    std::string colorMapName() const;

public:

    virtual void save();

    virtual void cancel();

    virtual void color();
};

} // end of namespace visclient

} // end of namespace kvs

#endif // PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_COLOR_SELECT_COLORMAP_H_INCLUDE
