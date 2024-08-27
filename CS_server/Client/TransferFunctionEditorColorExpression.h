/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorColorExpression.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorColorExpression.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_COLOR_EXPRESSION_H_INCLUDE
#define PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_COLOR_EXPRESSION_H_INCLUDE

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
class TransferFunctionEditorColorExpression : public kvs::glut::Screen
{
    kvsClassName( kvs::visclient::TransferFunctionEditorColorExpression );

public:

    typedef kvs::glut::Screen SuperClass;

protected:

    kvs::ScreenBase*              m_screen;              ///< pointer to screen
    kvs::glut::ColorMapPalette*   m_color_map_palette;   ///< color map palette
    kvs::glut::Histogram*         m_histogram;           ///< histogram
    kvs::TransferFunction m_initial_transfer_function;   ///< initial transfer function
    kvs::Real32           m_min_value; ///< min value
    kvs::Real32           m_max_value; ///< max value

    kvs::visclient::TransferFunctionEditorMain* m_parent;

    GLUI* m_glui_text_win;
    GLUI* m_glui_buttom_win;
    GLUI_EditText* m_glui_edit_red;
    GLUI_EditText* m_glui_edit_green;
    GLUI_EditText* m_glui_edit_blue;

    bool m_visible;
    size_t m_resolution;

public:

    TransferFunctionEditorColorExpression( kvs::visclient::TransferFunctionEditorMain* parent = 0 );

    virtual ~TransferFunctionEditorColorExpression();

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

    void equation( std::string* re, std::string* ge, std::string* be );

    void setEquation( const std::string& re, const std::string& ge, const std::string& be );

    void setColorMapEquation( const std::string& re, const std::string& ge, const std::string& be );

public:

    virtual void update();

    virtual void save();

    virtual void cancel();
};

} // end of namespace visclient

} // end of namespace kvs

#endif // PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_COLOR_EXPRESSION_H_INCLUDE
