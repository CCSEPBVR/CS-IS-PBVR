/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorMain.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorMain.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_MAIN_H_INCLUDE
#define PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_MAIN_H_INCLUDE

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
#include "Histogram.h"

#include "GL/glui.h"

#include "TransferFunctionEditorColorFreeformCurve.h"
#include "TransferFunctionEditorColorExpression.h"
#include "TransferFunctionEditorColorControlPoints.h"
#include "TransferFunctionEditorColorSelectColormap.h"
#include "TransferFunctionEditorOpacityFreeformCurve.h"
#include "TransferFunctionEditorOpacityExpression.h"
#include "TransferFunctionEditorOpacityControlPoints.h"
#include "TransferFunctionEditorColorFunction.h"
#include "ExtendedTransferFunctionMessage.h"
#include "Command.h"

namespace kvs
{

namespace visclient
{

/*===========================================================================*/
/**
 *  @brief  Transfer function editor class.
 */
/*===========================================================================*/
class TransferFunctionEditorMain : public kvs::glut::Screen
{
    kvsClassName( kvs::visclient::TransferFunctionEditorMain );

public:

    typedef kvs::glut::Screen SuperClass;

    typedef std::map<int, kvs::TransferFunction> TransferFunctionSet;

protected:

    Command* m_command;

    ExtendedTransferFunctionMessage m_doc;
    ExtendedTransferFunctionMessage m_doc_initial;
    // int m_current_transfer_function;

    kvs::ScreenBase*              m_screen;              ///< pointer to screen
    kvs::glut::ColorMapPalette*   m_color_map_palette;   ///< color map palette
    kvs::glut::OpacityMapPalette* m_opacity_map_palette; ///< opacity map palette
    kvs::visclient::Histogram*    m_color_histogram;     ///< color histogram
    kvs::visclient::Histogram*    m_opacity_histogram;   ///< opacity histogram
    kvs::glut::PushButton*        m_reset_button;        ///< reset button
    kvs::glut::PushButton*        m_apply_button;        ///< apply button
    kvs::Real32           m_min_value; ///< min value
    kvs::Real32           m_max_value; ///< max value

    kvs::visclient::TransferFunctionEditorColorFreeformCurve* m_transfer_function_editor_color_freeform_curve;
    kvs::visclient::TransferFunctionEditorColorExpression* m_transfer_function_editor_color_expression;
    kvs::visclient::TransferFunctionEditorColorControlPoints* m_transfer_function_editor_color_control_points;
    kvs::visclient::TransferFunctionEditorColorSelectColormap* m_transfer_function_editor_color_select_color_map;
    kvs::visclient::TransferFunctionEditorOpacityFreeformCurve* m_transfer_function_editor_opacity_freeform_curve;
    kvs::visclient::TransferFunctionEditorOpacityExpression* m_transfer_function_editor_opacity_expression;
    kvs::visclient::TransferFunctionEditorOpacityControlPoints* m_transfer_function_editor_opacity_control_points;
    kvs::visclient::TransferFunctionEditorColorFunction* m_transfer_function_editor_color_function;

    GLUI* m_glui_common;
    GLUI* m_glui_transfer_function_editor_range;
    GLUI* m_glui_transfer_function_file;
    GLUI* m_glui_transfer_function_control;

    GLUI_EditText* m_glui_resolution;
    GLUI_EditText* m_glui_transfer_function_synth;
    // GLUI_Listbox*  m_glui_transfer_function_name;
    GLUI_EditText* m_glui_transfer_function_var_color;
    GLUI_EditText* m_glui_transfer_function_min_color;
    GLUI_EditText* m_glui_transfer_function_max_color;
    GLUI_EditText* m_glui_transfer_function_var_opacity;
    GLUI_EditText* m_glui_transfer_function_min_opacity;
    GLUI_EditText* m_glui_transfer_function_max_opacity;
    GLUI_EditText* m_glui_transfer_function_path;
    GLUI_StaticText* m_glui_range_min_color;
    GLUI_StaticText* m_glui_range_max_color;
    GLUI_StaticText* m_glui_range_min_opacity;
    GLUI_StaticText* m_glui_range_max_opacity;

    bool m_show_transfer_function_editor;

    // add by @hira at 2016/12/01
    GLUI* m_glui_transfer_function_synthesizer;
    GLUI_Spinner *m_glui_num_transfer;
    GLUI_EditText* m_glui_color_function_synth;
    GLUI_EditText* m_glui_opacity_function_synth;
    GLUI_Spinner *m_glui_colormap_function;
    GLUI_Spinner *m_glui_opacitymap_function;

    GLUI_Button *m_addtrans_button;
    GLUI_Button *m_deltrans_button;

public:

    TransferFunctionEditorMain( Command* command, kvs::ScreenBase* parent = 0 );

    virtual ~TransferFunctionEditorMain();

public:

    int show();

    virtual void hide();

    bool getShowTransferFunctionEditor();

public:

    kvs::ScreenBase* screen();

    const kvs::glut::ColorMapPalette* colorMapPalette() const;

    const kvs::glut::OpacityMapPalette* opacityMapPalette() const;

    const kvs::ColorMap colorMap() const;

    const kvs::OpacityMap opacityMap() const;

    const kvs::TransferFunction transferFunction() const;

    kvs::visclient::ExtendedTransferFunctionMessage& doc();

    const kvs::visclient::ExtendedTransferFunctionMessage& doc() const;

public:

    void setVolumeObject( const kvs::VolumeObjectBase& object );

    void exportFile( const std::string& fname, const bool addition = false );

    void importFile( const std::string& fname );
    void importFileWithInit( const std::string& fname );

public:

    virtual void resolution();

    // virtual void functionName();		delete by @hira at 2016/12/01

    virtual void functionVariable();

    virtual void reset();

    virtual void apply();

    virtual void importFile();

    virtual void exportFile();

    virtual void showTransferFunctionEditorColorFreeformCurve();

    virtual void showTransferFunctionEditorColorExpression();

    virtual void showTransferFunctionEditorColorControlPoints();

    virtual void showTransferFunctionEditorColorSelectColormap();

    virtual void showTransferFunctionEditorOpacityFreeformCurve();

    virtual void showTransferFunctionEditorOpacityExpression();

    virtual void showTransferFunctionEditorOpacityControlPoints();

    virtual void saveTransferFunctionEditorColorFreeformCurve();

    virtual void saveTransferFunctionEditorColorExpression();

    virtual void saveTransferFunctionEditorColorControlPoints();

    virtual void saveTransferFunctionEditorColorSelectColormap();

    virtual void saveTransferFunctionEditorOpacityFreeformCurve();

    virtual void saveTransferFunctionEditorOpacityExpression();

    virtual void saveTransferFunctionEditorOpacityControlPoints();

    // add by @hira at 2016/12/01
    virtual void showTransferFunctionEditorColorFunction();
    virtual bool selectTransferFunctionEditorColorFunction(const NamedTransferFunctionParameter *select_trans);
    virtual void showTransferFunctionEditorOpacityFunction();
    virtual bool selectTransferFunctionEditorOpacityFunction(const NamedTransferFunctionParameter *select_trans);
    virtual void setColorTransferFunction();
    virtual void setOpacityTransferFunction();
    virtual void setNumberTransferFunctions();

public:

    // modify by @hira at 2016/12/01
    // void displayTransferFunction( const int n );
    void displayTransferFunction();

    void updateRangeView();
    // void updateRangeView( const int n );		// delete by @hira at 2016/12/01
    void updateRangeEdit();
    // void updateRangeEdit( const int n );		// delete by @hira at 2016/12/01

private:
    void refreshGluiTransferFunctionName(const std::string &select_transname);
    void clearTransferFunction();
    void displayTransferFunction(const int n_color, const int n_opacity);
    int show(const int n_color, const int n_opacity);

};

} // end of namespace visclient

} // end of namespace kvs

#endif // PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_MAIN_H_INCLUDE
