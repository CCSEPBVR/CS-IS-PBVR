/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorMain.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorMain.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
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

#include <climits>
#include <fstream>
#include <algorithm>
#include "ParameterFile.h"

//#define PBVR_NO_HISTOGRAM

extern void CallBackTransferFunctionEditorPanelShow( const int i );

namespace
{

class ResetButton : public kvs::glut::PushButton
{
    kvs::visclient::TransferFunctionEditorMain* m_editor;

public:

    ResetButton( kvs::visclient::TransferFunctionEditorMain* editor ):
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released()
    {
        m_editor->reset();
    }
};

class m_applyButton : public kvs::glut::PushButton
{
    kvs::visclient::TransferFunctionEditorMain* m_editor;

public:

    m_applyButton( kvs::visclient::TransferFunctionEditorMain* editor ):
        kvs::glut::PushButton( editor ),
        m_editor( editor )
    {
    }

    void released()
    {
        m_editor->apply();
    }
};

} // end of namespace

namespace
{
kvs::visclient::TransferFunctionEditorMain* ptfe;

void CallBackResolution( const int id )
{
    ptfe->resolution();
}

void CallBackFunctionVariable( const int id )
{
    ptfe->functionVariable();
}

void CallBackImportFile( const int id )
{
    ptfe->importFile();
}

void CallBackExportFile( const int id )
{
    ptfe->exportFile();
}

void CallBackShowTransferFunctionEditorColorFreeformCurve( const int id )
{
    ptfe->showTransferFunctionEditorColorFreeformCurve();
}

void CallBackShowTransferFunctionEditorColorExpression( const int id )
{
    ptfe->showTransferFunctionEditorColorExpression();
}

void CallBackShowTransferFunctionEditorColorControlPoints( const int id )
{
    ptfe->showTransferFunctionEditorColorControlPoints();
}

void CallBackShowTransferFunctionEditorColorSelectColormap( const int id )
{
    ptfe->showTransferFunctionEditorColorSelectColormap();
}

void CallBackShowTransferFunctionEditorOpacityFreeformCurve( const int id )
{
    ptfe->showTransferFunctionEditorOpacityFreeformCurve();
}

void CallBackShowTransferFunctionEditorOpacityExpression( const int id )
{
    ptfe->showTransferFunctionEditorOpacityExpression();
}

void CallBackShowTransferFunctionEditorOpacityControlPoints( const int id )
{
    ptfe->showTransferFunctionEditorOpacityControlPoints();
}


/**
 * 色関数定義ダイアログを表示する
 * @param id		id
 */
void CallBackShowTransferFunctionEditorColorFunction( const int id )
{
    ptfe->showTransferFunctionEditorColorFunction();
}

/**
 * 不透明度関数定義ダイアログを表示する
 * @param id		id
 */
void CallBackShowTransferFunctionEditorOpacityFunction( const int id )
{
    ptfe->showTransferFunctionEditorOpacityFunction();
}

/**
 * 色関数の表示を切り替える
 * @param id
 */
void CallBackColorFunction( const int id )
{
    ptfe->setColorTransferFunction();
}


/**
 * 不透明度関数の表示を切り替える
 * @param id
 */
void CallBackOpacityFunction( const int id )
{
    ptfe->setOpacityTransferFunction();
}

/**
 * 伝達関数数を変更する
 * @param id
 */
void CallBackNumberTransferFunctions( const int id )
{
    ptfe->setNumberTransferFunctions();
}


} // end of namespace

namespace kvs
{

namespace visclient
{


TransferFunctionEditorMain::TransferFunctionEditorMain( Command* command, kvs::ScreenBase* parent ):
    m_command( command ),
    m_screen( parent ),
    m_color_map_palette( NULL ),
    m_opacity_map_palette( NULL ),
    m_color_histogram( NULL ),
    m_opacity_histogram( NULL ),
    m_reset_button( NULL ),
    m_apply_button( NULL ),
    m_min_value( 0.0f ),
    m_max_value( 1.0f )
    //,m_current_transfer_function( 0 )
{
    const std::string title = "Transfer Function Editor";
    const int x = ( parent != 0 ) ? parent->x() + parent->width() + 5 : 0;
    const int y = ( parent != 0 ) ? parent->y() : 0;
    const int width = 700;
#ifndef PBVR_NO_HISTOGRAM
    const int height = 660;
#else
    const int height = 550;
#endif
    const int margin = 10;
    const kvs::RGBColor color( 200, 200, 200 );
    const int MAPPALETTE_POS = 330;

    SuperClass::background()->setColor( color );
    SuperClass::setTitle( title );
    SuperClass::setPosition( x, y );
    SuperClass::setSize( width, height );

    m_command->m_extend_transfer_function_editor = this;;

    m_color_map_palette = new kvs::glut::ColorMapPalette( this );
    m_color_map_palette->setCaption( "Color" );
    m_color_map_palette->setColorMap( m_doc.m_color_transfer_function[0].colorMap() );
//  m_color_map_palette->setX( m_color_palette->x0() );
//  m_color_map_palette->setY( m_color_palette->y1() - m_color_palette->margin() );
    m_color_map_palette->setY( MAPPALETTE_POS );
    m_color_map_palette->setHeight( 100 );
    m_color_map_palette->setEventType(
    kvs::EventBase::PaintEvent |
    kvs::EventBase::ResizeEvent );
    m_color_map_palette->show();

    m_opacity_map_palette = new kvs::glut::OpacityMapPalette( this );
    m_opacity_map_palette->setCaption( "Opacity" );
    m_opacity_map_palette->setOpacityMap( m_doc.m_opacity_transfer_function[0].opacityMap() );
    m_opacity_map_palette->setX( m_color_map_palette->x1() );
    m_opacity_map_palette->setY( m_color_map_palette->y0() );
    m_opacity_map_palette->setHeight( 100 );
    m_opacity_map_palette->setEventType(
        kvs::EventBase::PaintEvent |
        kvs::EventBase::ResizeEvent );
    m_opacity_map_palette->show();

#ifndef PBVR_NO_HISTOGRAM
    m_color_histogram = new kvs::visclient::Histogram( this );
    m_color_histogram->setCaption( "Histogram" );
    m_color_histogram->setX( m_color_map_palette->x0() );
    m_color_histogram->setY( m_color_map_palette->y1() - m_opacity_map_palette->margin() );
    m_color_histogram->setHeight( 100 );
    m_color_histogram->setGraphColor( kvs::RGBAColor( 100, 100, 100, 1.0f ) );

    m_opacity_histogram = new kvs::visclient::Histogram( this );
    m_opacity_histogram->setCaption( "Histogram" );
    m_opacity_histogram->setX( m_color_map_palette->x1() );
    m_opacity_histogram->setY( m_color_histogram->y0() );
    m_opacity_histogram->setHeight( 100 );
    m_opacity_histogram->setGraphColor( kvs::RGBAColor( 100, 100, 100, 1.0f ) );

    std::vector<size_t> table( 1024 );
    m_color_histogram->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() ) );
    m_opacity_histogram->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() ) );

    m_color_histogram->show();
    m_opacity_histogram->show();
#endif

    const size_t button_margin = 5;
    const size_t button_width = ( width - 2 * margin - button_margin ) / 2;

    m_reset_button = new ::ResetButton( this );
    m_reset_button->setCaption( "Reset" );
#ifndef PBVR_NO_HISTOGRAM
    m_reset_button->setX( m_color_histogram->x0() + m_color_histogram->margin() );
    m_reset_button->setY( m_color_histogram->y1() + m_color_histogram->margin() );
#else
    m_reset_button->setX( m_color_map_palette->x0() + m_color_map_palette->margin() );
    m_reset_button->setY( m_color_map_palette->y1() + m_color_map_palette->margin() );
#endif
    m_reset_button->setWidth( button_width );
    m_reset_button->show();

    m_apply_button = new ::m_applyButton( this );
    m_apply_button->setCaption( "Apply" );
    m_apply_button->setX( m_reset_button->x1() + button_margin );
    m_apply_button->setY( m_reset_button->y0() );
    m_apply_button->setWidth( ( width - margin ) / 2 - m_opacity_map_palette->margin() );
    m_apply_button->show();

    m_transfer_function_editor_color_freeform_curve = new kvs::visclient::TransferFunctionEditorColorFreeformCurve( this );
    m_transfer_function_editor_color_expression = new kvs::visclient::TransferFunctionEditorColorExpression( this );
    m_transfer_function_editor_color_control_points = new kvs::visclient::TransferFunctionEditorColorControlPoints( this );
    m_transfer_function_editor_color_select_color_map = new kvs::visclient::TransferFunctionEditorColorSelectColormap( this );
    m_transfer_function_editor_opacity_freeform_curve = new kvs::visclient::TransferFunctionEditorOpacityFreeformCurve( this );
    m_transfer_function_editor_opacity_expression = new kvs::visclient::TransferFunctionEditorOpacityExpression( this );
    m_transfer_function_editor_opacity_control_points = new kvs::visclient::TransferFunctionEditorOpacityControlPoints( this );
    m_transfer_function_editor_color_function = new kvs::visclient::TransferFunctionEditorColorFunction( this );

    // add by @hira at 2017/07/30
    this->m_doc_initial = this->m_doc;
}

TransferFunctionEditorMain::~TransferFunctionEditorMain()
{
    if ( m_color_map_palette ) delete m_color_map_palette;
    if ( m_opacity_map_palette ) delete m_opacity_map_palette;
    if ( m_color_histogram ) delete m_color_histogram;
    if ( m_opacity_histogram ) delete m_opacity_histogram;
    if ( m_reset_button ) delete m_reset_button;
    if ( m_apply_button ) delete m_apply_button;
}

int TransferFunctionEditorMain::show()
{
    return this->show(1, 1);
}

int TransferFunctionEditorMain::show(const int n_color, const int n_opacity)
{
    int id = SuperClass::show();
    glutSetWindow( m_id );
    ::ptfe = this;
    const float  SPINNER_SPEED = 0.0;

    m_glui_common = GLUI_Master.create_glui_subwindow( m_id, GLUI_SUBWINDOW_TOP );
    m_glui_transfer_function_synthesizer = GLUI_Master.create_glui_subwindow( m_id, GLUI_SUBWINDOW_TOP );
    m_glui_transfer_function_editor_range = GLUI_Master.create_glui_subwindow( m_id, GLUI_SUBWINDOW_TOP );
    m_glui_transfer_function_control = GLUI_Master.create_glui_subwindow( m_id, GLUI_SUBWINDOW_BOTTOM );
    m_glui_transfer_function_file = GLUI_Master.create_glui_subwindow( m_id, GLUI_SUBWINDOW_BOTTOM );

    m_glui_common->set_main_gfx_window( m_id );
    m_glui_transfer_function_synthesizer->set_main_gfx_window( m_id );
    m_glui_transfer_function_editor_range->set_main_gfx_window( m_id );
    m_glui_transfer_function_control->set_main_gfx_window( m_id );
    m_glui_transfer_function_file->set_main_gfx_window( m_id );

    {
        GLUI_Panel* panel_resolution = m_glui_common->add_panel( "", GLUI_PANEL_NONE );
        panel_resolution->set_alignment( GLUI_ALIGN_LEFT );
        m_glui_resolution = m_glui_common->add_edittext_to_panel(panel_resolution, "Transfer Function RESOLUTION", GLUI_EDITTEXT_INT, NULL, -1, CallBackResolution );
        m_glui_resolution->set_w( 250 );

        m_glui_common->add_column_to_panel(panel_resolution, false);
        m_glui_num_transfer = m_glui_common->add_spinner_to_panel(panel_resolution, "Number Transfer Functions", GLUI_EDITTEXT_INT, NULL, -1, CallBackNumberTransferFunctions );
        m_glui_num_transfer->set_w( 250 );
        m_glui_num_transfer->set_int_limits(1, 99);
        // >> start : add by @hira at 2017/07/30
        m_glui_num_transfer->set_speed(SPINNER_SPEED);
        // << end : add by @hira at 2017/07/30
    }
    {
        m_glui_color_function_synth   = m_glui_transfer_function_synthesizer->add_edittext("Color Function SYNTHESIZER:", GLUI_EDITTEXT_TEXT, NULL, -1, CallBackFunctionVariable );
        m_glui_color_function_synth->set_w(340);

        m_glui_transfer_function_synthesizer->add_column( false );
        m_glui_opacity_function_synth   = m_glui_transfer_function_synthesizer->add_edittext("Opacity Function SYNTHESIZER:", GLUI_EDITTEXT_TEXT, NULL, -1, CallBackFunctionVariable );
        m_glui_opacity_function_synth->set_w(340);
    }

    {
        m_glui_transfer_function_editor_range->add_statictext("Transfer Function Color Map:" );
        GLUI_Button* cfunc = m_glui_transfer_function_editor_range->add_button("Color Function Editor", id, CallBackShowTransferFunctionEditorColorFunction );
        cfunc->set_w( 340 );
    }
    {
        GLUI_Panel* panel_function = m_glui_transfer_function_editor_range->add_panel( "", GLUI_PANEL_NONE );
        panel_function->set_alignment( GLUI_ALIGN_LEFT );
        // panel_function->y_off_bot = -10;
        m_glui_colormap_function = m_glui_transfer_function_editor_range->add_spinner_to_panel(panel_function, "Color Map Function : C", GLUI_EDITTEXT_INT, NULL, -1, CallBackColorFunction);
        m_glui_colormap_function->set_w( 80 );
        // >> start : add by @hira at 2017/07/30
        m_glui_colormap_function->set_speed(SPINNER_SPEED);
        // << end : add by @hira at 2017/07/30
        m_glui_transfer_function_editor_range->add_column_to_panel(panel_function, false);
        m_glui_transfer_function_var_color   = m_glui_transfer_function_editor_range->add_edittext_to_panel(panel_function, "= f( ", GLUI_EDITTEXT_TEXT, NULL, -1, CallBackFunctionVariable );
        m_glui_transfer_function_var_color->set_w(100);
        m_glui_transfer_function_var_color->text_x_offset = 0;
        m_glui_transfer_function_editor_range->add_column_to_panel(panel_function, false);
        GLUI_StaticText *label = m_glui_transfer_function_editor_range->add_statictext_to_panel(panel_function, ")");
        label->set_w(5);
        label->y_off_top = 10;
    }
    {
        GLUI_Panel* panel_range = m_glui_transfer_function_editor_range->add_panel( "", GLUI_PANEL_NONE );
        panel_range->set_alignment( GLUI_ALIGN_LEFT );
        panel_range->y_off_bot = 0;
        m_glui_transfer_function_min_color = m_glui_transfer_function_editor_range->add_edittext_to_panel(panel_range, "Range Min.", GLUI_EDITTEXT_FLOAT, NULL, -1, CallBackFunctionVariable );
        m_glui_transfer_function_max_color = m_glui_transfer_function_editor_range->add_edittext_to_panel(panel_range, "Range Max.", GLUI_EDITTEXT_FLOAT, NULL, -1, CallBackFunctionVariable );
        m_glui_transfer_function_min_color->set_w( 140 );
        m_glui_transfer_function_max_color->set_w( 140 );

        m_glui_range_min_color = m_glui_transfer_function_editor_range->add_statictext_to_panel(panel_range, "Server side range min: xxx" );
        m_glui_range_max_color = m_glui_transfer_function_editor_range->add_statictext_to_panel(panel_range, "Server side range max: xxx" );
    }

    GLUI_Button* c1 = m_glui_transfer_function_editor_range->add_button( "Color Map Editor (freeform curve)", id, CallBackShowTransferFunctionEditorColorFreeformCurve );
    GLUI_Button* c2 = m_glui_transfer_function_editor_range->add_button( "Color Map Editor (expression)", id, CallBackShowTransferFunctionEditorColorExpression );
    GLUI_Button* c3 = m_glui_transfer_function_editor_range->add_button( "Color Map Editor (control points)", id, CallBackShowTransferFunctionEditorColorControlPoints );
    GLUI_Button* c4 = m_glui_transfer_function_editor_range->add_button( "Color Map Editor (select colormap)", id, CallBackShowTransferFunctionEditorColorSelectColormap );

    c1->set_w( 340 );
    c2->set_w( 340 );
    c3->set_w( 340 );
    c4->set_w( 340 );

    m_glui_transfer_function_editor_range->add_column( false );
    {
        m_glui_transfer_function_editor_range->add_statictext( "Transfer Function Opacity Map:" );
        GLUI_Button* ofunc = m_glui_transfer_function_editor_range->add_button( "Opacity Function Editor", id, CallBackShowTransferFunctionEditorOpacityFunction );
        ofunc->set_w( 340 );
    }
    {
        GLUI_Panel* panel_function = m_glui_transfer_function_editor_range->add_panel( "", GLUI_PANEL_NONE );
        panel_function->set_alignment( GLUI_ALIGN_LEFT );
        // panel_function->y_off_bot = -10;
        m_glui_opacitymap_function = m_glui_transfer_function_editor_range->add_spinner_to_panel(panel_function, "Opacity Map Function : O", GLUI_EDITTEXT_INT, NULL, -1, CallBackOpacityFunction);
        m_glui_opacitymap_function->set_w( 80 );
        // >> start : add by @hira at 2017/07/30
        m_glui_opacitymap_function->set_speed(SPINNER_SPEED);
        // << end : add by @hira at 2017/07/30

        m_glui_transfer_function_editor_range->add_column_to_panel(panel_function, false);
        m_glui_transfer_function_var_opacity   = m_glui_transfer_function_synthesizer->add_edittext_to_panel(panel_function, "= f( ", GLUI_EDITTEXT_TEXT, NULL, -1, CallBackFunctionVariable );
        m_glui_transfer_function_var_opacity->set_w(100);
        m_glui_transfer_function_var_opacity->text_x_offset = 0;
        m_glui_transfer_function_editor_range->add_column_to_panel(panel_function, false);
        GLUI_StaticText *label = m_glui_transfer_function_editor_range->add_statictext_to_panel(panel_function, ")");
        label->set_w(5);
        label->y_off_top = 10;
    }
    {
        GLUI_Panel* panel_range = m_glui_transfer_function_editor_range->add_panel( "", GLUI_PANEL_NONE );
        panel_range->set_alignment( GLUI_ALIGN_LEFT );
        panel_range->y_off_bot = 0;
        m_glui_transfer_function_min_opacity = m_glui_transfer_function_editor_range->add_edittext_to_panel(panel_range, "Range Min.", GLUI_EDITTEXT_FLOAT, NULL, -1, CallBackFunctionVariable );
        m_glui_transfer_function_max_opacity = m_glui_transfer_function_editor_range->add_edittext_to_panel(panel_range, "Range Max.", GLUI_EDITTEXT_FLOAT, NULL, -1, CallBackFunctionVariable );
        m_glui_transfer_function_min_opacity->set_w( 140 );
        m_glui_transfer_function_max_opacity->set_w( 140 );

        m_glui_range_min_opacity = m_glui_transfer_function_editor_range->add_statictext_to_panel(panel_range, "Server side range min: xxx" );
        m_glui_range_max_opacity = m_glui_transfer_function_editor_range->add_statictext_to_panel(panel_range, "Server side range max: xxx" );
    }

    GLUI_Button* o1 = m_glui_transfer_function_editor_range->add_button( "Opacity Map Editor (freeform curve)", id, CallBackShowTransferFunctionEditorOpacityFreeformCurve );
    GLUI_Button* o2 = m_glui_transfer_function_editor_range->add_button( "Opacity Map Editor (expression)", id, CallBackShowTransferFunctionEditorOpacityExpression );
    GLUI_Button* o3 = m_glui_transfer_function_editor_range->add_button( "Opacity Map Editor (control points)", id, CallBackShowTransferFunctionEditorOpacityControlPoints );
    o1->set_w( 340 );
    o2->set_w( 340 );
    o3->set_w( 340 );

    m_glui_transfer_function_path = m_glui_transfer_function_file->add_edittext( "File Path" );
    m_glui_transfer_function_file->add_column( false );
    /*
    GLUI_Button* b1 = m_glui_transfer_function_file->add_button("Browse...", 0);
    */
    m_glui_transfer_function_path->set_w( 500 );
//    b1->set_w(150);

    GLUI_Button* b2 = m_glui_transfer_function_control->add_button( "Export File", 0, CallBackExportFile );
    // Add Close Button 2016.2.14
    GLUI_Button* b4 = m_glui_transfer_function_control->add_button( "Close", 1, CallBackTransferFunctionEditorPanelShow );

    m_glui_transfer_function_control->add_column( false );
    GLUI_Button* b3 = m_glui_transfer_function_control->add_button( "Import File", 0, CallBackImportFile );
    b2->set_w( 150 );
    b3->set_w( 150 );
    b4->set_w( 150 );

    this->displayTransferFunction(n_color, n_opacity);

    m_show_transfer_function_editor = true;
    return id;
}

void TransferFunctionEditorMain::hide()
{
    ::ptfe = NULL;

    glutSetWindow( m_id );
    int x = glutGet( ( GLenum )GLUT_WINDOW_X );
    int y = glutGet( ( GLenum )GLUT_WINDOW_Y );
#if defined(WIN32)
    SuperClass::setPosition( x, y );
#elif defined(__APPLE__)
    SuperClass::setPosition( x, y );
#else
    SuperClass::setPosition( x, y - 28 );
#endif
    SuperClass::hide();
    m_show_transfer_function_editor = false;
}

bool TransferFunctionEditorMain::getShowTransferFunctionEditor()
{
    return m_show_transfer_function_editor;
}

kvs::ScreenBase* TransferFunctionEditorMain::screen()
{
    return m_screen;
}

const kvs::glut::ColorMapPalette* TransferFunctionEditorMain::colorMapPalette() const
{
    return m_color_map_palette;
}

const kvs::glut::OpacityMapPalette* TransferFunctionEditorMain::opacityMapPalette() const
{
    return m_opacity_map_palette;
}

const kvs::ColorMap TransferFunctionEditorMain::colorMap() const
{
    return m_color_map_palette->colorMap();
}

const kvs::OpacityMap TransferFunctionEditorMain::opacityMap() const
{
    return m_opacity_map_palette->opacityMap();
}

const kvs::TransferFunction TransferFunctionEditorMain::transferFunction() const
{
    kvs::TransferFunction transfer_function( this->colorMap(), this->opacityMap() );
    transfer_function.setRange( m_min_value, m_max_value );

    return transfer_function;
}

void TransferFunctionEditorMain::setVolumeObject( const kvs::VolumeObjectBase& object )
{
    m_color_histogram->create( &object );
    m_opacity_histogram->create( &object );
}

kvs::visclient::ExtendedTransferFunctionMessage& TransferFunctionEditorMain::doc()
{
    return m_doc;
}

const kvs::visclient::ExtendedTransferFunctionMessage& TransferFunctionEditorMain::doc() const
{
    return m_doc;
}

void TransferFunctionEditorMain::exportFile( const std::string& fname, const bool addition )
{
    std::ofstream ofs;
    if ( addition )
    {
        ofs.open( fname.c_str(), std::ofstream::app );
    }
    else
    {
        ofs.open( fname.c_str(), std::ofstream::out );
    }
    if ( ofs.fail() )
    {
        std::cerr << "Error: open file " << fname << std::endl;
        return;
    }

    ofs << "TF_RESOLUTION=" << m_doc.m_extend_transfer_function_resolution << std::endl;
#if 0      // delete by @hira at 2016/12/01
    ofs << "TF_SYNTH=" << m_doc.m_extend_transfer_function_synthesis << std::endl;
    for ( size_t n = 0; n < m_doc.m_transfer_function.size(); n++ )
    {
        std::stringstream ss;
        // modify by @hira at 2016/12/01
        // ss << "TF_NAME" << n + 1 << "_";
        int name_number = m_doc.m_transfer_function[n].getNameNumber();
        ss << "TF_NAME" << name_number << "_";

        const std::string tag_base = ss.str();
        ofs << tag_base << "VAR_C=" << m_doc.m_transfer_function[n].m_color_variable << std::endl;
        ofs << tag_base << "MIN_C=" << m_doc.m_transfer_function[n].m_color_variable_min << std::endl;
        ofs << tag_base << "MAX_C=" << m_doc.m_transfer_function[n].m_color_variable_max << std::endl;
        ofs << tag_base << "VAR_O=" << m_doc.m_transfer_function[n].m_opacity_variable << std::endl;
        ofs << tag_base << "MIN_O=" << m_doc.m_transfer_function[n].m_opacity_variable_min << std::endl;
        ofs << tag_base << "MAX_O=" << m_doc.m_transfer_function[n].m_opacity_variable_max << std::endl;

        kvs::ColorMap::Table color_table = m_doc.m_transfer_function[n].colorMap().table();
        kvs::OpacityMap::Table opacity_table = m_doc.m_transfer_function[n].opacityMap().table();

        ofs << tag_base << "TABLE_C=";
        for ( size_t i = 0; i < color_table.size(); i++ )
        {
            ofs << static_cast<int>( color_table.at( i ) ) << ",";
        }
        ofs << std::endl;

        ofs << tag_base << "TABLE_O=";
        for ( size_t i = 0; i < opacity_table.size(); i++ )
        {
            ofs << opacity_table.at( i ) << ",";
        }
        ofs << std::endl;
    }
#endif

    ofs << "TF_NUMBER=" << m_doc.m_transfer_function_number << std::endl;
    ofs << "TF_SYNTH_C=" << m_doc.m_color_transfer_function_synthesis << std::endl;
    ofs << "TF_SYNTH_O=" << m_doc.m_opacity_transfer_function_synthesis << std::endl;

    for ( size_t n = 0; n < m_doc.m_color_transfer_function.size(); n++ ) {
        std::stringstream ss;
        int name_number = m_doc.m_color_transfer_function[n].getNameNumber();
        ss << "TF_NAME" << name_number << "_";

        const std::string tag_base = ss.str();
        ofs << tag_base << "C=" << m_doc.m_color_transfer_function[n].m_name << std::endl;
        ofs << tag_base << "VAR_C=" << m_doc.m_color_transfer_function[n].m_color_variable << std::endl;
        ofs << tag_base << "MIN_C=" << m_doc.m_color_transfer_function[n].m_color_variable_min << std::endl;
        ofs << tag_base << "MAX_C=" << m_doc.m_color_transfer_function[n].m_color_variable_max << std::endl;
        kvs::ColorMap::Table color_table = m_doc.m_color_transfer_function[n].colorMap().table();

        ofs << tag_base << "TABLE_C=";
        for ( size_t i = 0; i < color_table.size(); i++ )
        {
            ofs << static_cast<int>( color_table.at( i ) ) << ",";
        }
        ofs << std::endl;
    }

    for ( size_t n = 0; n < m_doc.m_opacity_transfer_function.size(); n++ ) {
        std::stringstream ss;
        int name_number = m_doc.m_opacity_transfer_function[n].getNameNumber();
        ss << "TF_NAME" << name_number << "_";

        const std::string tag_base = ss.str();
        ofs << tag_base << "O=" << m_doc.m_opacity_transfer_function[n].m_name << std::endl;
        ofs << tag_base << "VAR_O=" << m_doc.m_opacity_transfer_function[n].m_opacity_variable << std::endl;
        ofs << tag_base << "MIN_O=" << m_doc.m_opacity_transfer_function[n].m_opacity_variable_min << std::endl;
        ofs << tag_base << "MAX_O=" << m_doc.m_opacity_transfer_function[n].m_opacity_variable_max << std::endl;

        kvs::OpacityMap::Table opacity_table = m_doc.m_opacity_transfer_function[n].opacityMap().table();
        ofs << tag_base << "TABLE_O=";
        for ( size_t i = 0; i < opacity_table.size(); i++ )
        {
            ofs << opacity_table.at( i ) << ",";
        }
        ofs << std::endl;
    }
    ofs.close();

    std::cerr << "TransferFunction parameters are exported to " << fname << std::endl;
}

void TransferFunctionEditorMain::importFile( const std::string& fname )
{
    ExtendedTransferFunctionMessage doc;
    ParameterFile param;
    bool stat;

    stat = param.loadIN( fname );

    if ( !stat ) return;

    doc.m_extend_transfer_function_resolution = param.getInt( "tf_resolution" );
    const size_t resolution = doc.m_extend_transfer_function_resolution;

#if 0      // delete by @hira at 2016/12/01
    doc.m_extend_transfer_function_synthesis = param.getString( "tf_synthesis" );

    doc.m_transfer_function.clear();
    std::string select_transname = "t1";
    int min_number = INT_MAX;
    int tfname_size = param.getTfnameSize();
    //for ( size_t n = 0; n < doc.m_transfer_function.size(); n++ )
    for ( size_t n = 0; n < tfname_size; n++ )
    {
        std::string prefix = param.getTfnamePrefix(n);
        int name_number = param.getTfnameNumber(n);

        std::stringstream ss;
        // ss << "TF_NAME" << n + 1 << "_";
        ss << prefix << "_";
        const std::string tag_base = ss.str();

        ss.str("");
        ss << "t" << name_number;
        const std::string name = ss.str();
        if (name_number < min_number) {
            select_transname = name;
            min_number = name_number;
        }

        // doc.m_transfer_function[n].setResolution( resolution );
        // doc.m_transfer_function[n].m_color_variable      = param.getString( tag_base + "VAR_C" );
        // doc.m_transfer_function[n].m_color_variable_min   = param.getFloat( tag_base + "MIN_C" );
        // doc.m_transfer_function[n].m_color_variable_max   = param.getFloat( tag_base + "MAX_C" );
        // doc.m_transfer_function[n].m_opacity_variable    = param.getString( tag_base + "VAR_O" );
        // doc.m_transfer_function[n].m_opacity_variable_min = param.getFloat( tag_base + "MIN_O" );
        // doc.m_transfer_function[n].m_opacity_variable_max = param.getFloat( tag_base + "MAX_O" );

        NamedTransferFunctionParameter trans;
        trans.setResolution( resolution );
        trans.m_name = name;;
        trans.m_color_variable      = param.getString( tag_base + "VAR_C" );
        trans.m_color_variable_min   = param.getFloat( tag_base + "MIN_C" );
        trans.m_color_variable_max   = param.getFloat( tag_base + "MAX_C" );
        trans.m_opacity_variable    = param.getString( tag_base + "VAR_O" );
        trans.m_opacity_variable_min = param.getFloat( tag_base + "MIN_O" );
        trans.m_opacity_variable_max = param.getFloat( tag_base + "MAX_O" );

        std::string s_color = param.getString( tag_base + "TABLE_C" );
        std::string s_opacity = param.getString( tag_base + "TABLE_O" );

        std::replace( s_color.begin(), s_color.end(), ',', ' ' );
        std::replace( s_opacity.begin(), s_opacity.end(), ',', ' ' );

        std::stringstream ss_color( s_color );
        std::stringstream ss_opacity( s_opacity );

        kvs::ColorMap::Table color_table( resolution * 3 );
        kvs::OpacityMap::Table opacity_table( resolution );

        for ( size_t i = 0; i < resolution; i++ )
        {
            for ( size_t c = 0; c < 3; c++ )
            {
                int color_e;
                ss_color >> color_e;
                color_table.at( i * 3 + c ) = color_e;
            }
        }

        for ( size_t i = 0; i < resolution; i++ )
        {
            float opacity;
            ss_opacity >> opacity;
            opacity_table.at( i ) = opacity;
        }

        kvs::ColorMap color_map( color_table );
        kvs::OpacityMap opacity_map( opacity_table );
        // kvs::TransferFunction tf( color_map, opacity_map );
        // kvs::TransferFunction& tt = doc.m_transfer_function[n];
        // tt = tf;
        trans.setColorMap(color_map);
        trans.setOpacityMap(opacity_map);
        doc.m_transfer_function.push_back(trans);
    }
#endif

    if (!param.hasParam("TF_SYNTH_C") || !param.hasParam("TF_SYNTH_O")) {
        std::cerr << "[Error] This import file is old format. file=" << fname << std::endl;
        return;
    }

    doc.m_transfer_function_number = param.getInt( "TF_NUMBER" );
    doc.m_color_transfer_function_synthesis = param.getString( "TF_SYNTH_C" );
    doc.m_opacity_transfer_function_synthesis = param.getString( "TF_SYNTH_O" );

    doc.m_color_transfer_function.clear();
    for ( size_t n = 0; n < doc.m_transfer_function_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
        const std::string tag_base = ss.str();
        std::string name = tag_base + "C";

        NamedTransferFunctionParameter trans;
        trans.setResolution( resolution );
        if (!param.hasParam(name)) {
            char c_name[8] = {0x00};
            //sprintf(c_name, "C%d", n + 1);
			sprintf(c_name, "C%zd", n + 1);
			trans.m_name = std::string(c_name);
            trans.m_color_variable      = "q1";
        }
        else {
            trans.m_name = param.getString( tag_base + "C" );
            trans.m_color_variable      = param.getString( tag_base + "VAR_C" );
            trans.m_color_variable_min   = param.getFloat( tag_base + "MIN_C" );
            trans.m_color_variable_max   = param.getFloat( tag_base + "MAX_C" );

            std::string s_color = param.getString( tag_base + "TABLE_C" );
            std::replace( s_color.begin(), s_color.end(), ',', ' ' );
            std::stringstream ss_color( s_color );
            kvs::ColorMap::Table color_table( resolution * 3 );
            for ( size_t i = 0; i < resolution; i++ )
            {
                for ( size_t c = 0; c < 3; c++ )
                {
                    int color_e;
                    ss_color >> color_e;
                    color_table.at( i * 3 + c ) = color_e;
                }
            }
            kvs::ColorMap color_map( color_table );
            trans.setColorMap(color_map);
        }
        if (doc.getColorTransferFunction(trans.m_name) == NULL) {
            doc.m_color_transfer_function.push_back(trans);
        }
    }

    doc.m_opacity_transfer_function.clear();
    for ( size_t n = 0; n < doc.m_transfer_function_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
        const std::string tag_base = ss.str();
        std::string name = tag_base + "O";

        NamedTransferFunctionParameter trans;
        trans.setResolution( resolution );
        if (!param.hasParam(name)) {
            char o_name[8] = {0x00};
            //sprintf(o_name, "O%d", n + 1);
			sprintf(o_name, "O%zd", n + 1);
			trans.m_name = std::string(o_name);
            trans.m_opacity_variable      = "q1";
        }
        else {
            trans.m_name = param.getString( tag_base + "O" );
            trans.m_opacity_variable    = param.getString( tag_base + "VAR_O" );
            trans.m_opacity_variable_min = param.getFloat( tag_base + "MIN_O" );
            trans.m_opacity_variable_max = param.getFloat( tag_base + "MAX_O" );

            std::string s_opacity = param.getString( tag_base + "TABLE_O" );
            std::replace( s_opacity.begin(), s_opacity.end(), ',', ' ' );
            std::stringstream ss_opacity( s_opacity );
            kvs::OpacityMap::Table opacity_table( resolution );
            for ( size_t i = 0; i < resolution; i++ )
            {
                float opacity;
                ss_opacity >> opacity;
                opacity_table.at( i ) = opacity;
            }
            kvs::OpacityMap opacity_map( opacity_table );
            trans.setOpacityMap(opacity_map);
        }

        if (doc.getOpacityTransferFunction(trans.m_name) == NULL) {
            doc.m_opacity_transfer_function.push_back(trans);
        }
    }

    m_doc_initial = doc;
    m_doc = doc;
    m_command->m_is_import_transfer_function_parameter = true;

    displayTransferFunction();
    this->redraw();
    this->apply();

    std::cerr << "TransferFunction parameters are imported from " << fname << std::endl;
}

void TransferFunctionEditorMain::importFileWithInit( const std::string& fname )
{
    this->importFile( fname );
}

void TransferFunctionEditorMain::resolution()
{
    size_t resolution = m_glui_resolution->get_int_val();
    m_doc.m_extend_transfer_function_resolution = resolution;

    for ( size_t n = 0; n < m_doc.m_color_transfer_function.size(); n++ ) {
        m_doc.m_color_transfer_function[n].setResolution( resolution );
    }

    for ( size_t n = 0; n < m_doc.m_opacity_transfer_function.size(); n++ ) {
        m_doc.m_opacity_transfer_function[n].setResolution( resolution );
    }

    this->displayTransferFunction();
    this->redraw();
}

void TransferFunctionEditorMain::functionVariable()
{
    m_command->m_is_import_transfer_function_parameter = true;

    // 1次伝達関数：色、不透明度
    m_doc.m_color_transfer_function_synthesis = m_glui_color_function_synth->get_text();
    m_doc.m_opacity_transfer_function_synthesis = m_glui_opacity_function_synth->get_text();

    // 色関数を設定する
    {
        int select_num = this->m_glui_colormap_function->get_int_val();
        char text[256] = {0x00};
        sprintf(text, "C%d", select_num);
        const char* variable = m_glui_transfer_function_var_color->get_text();
        // 既存かチェックする
        bool exists = (this->m_doc.getColorTransferFunction(text) != NULL);
        // 更新：存在しなければ追加
        m_doc.setColorTransferFunction(text, variable);

        // 既存の場合は最小値、最大値を設定する。（新規の場合は、[0]のコピーを作成済み
        if (exists) {
            // 最小値、最大値
            this->m_doc.setColorTransferRange(text,
                    m_glui_transfer_function_min_color->get_float_val(),
                    m_glui_transfer_function_max_color->get_float_val());
        }
    }

    // 不透明度関数を設定する
    {
        int select_num = this->m_glui_opacitymap_function->get_int_val();
        char text[256] = {0x00};
        sprintf(text, "O%d", select_num);
        const char* variable = m_glui_transfer_function_var_opacity->get_text();
        // 既存かチェックする
        bool exists = (this->m_doc.getOpacityTransferFunction(text) != NULL);
        // 更新：存在しなければ追加
        m_doc.setOpacityTransferFunction(text, variable);

        // 最小値、最大値
        if (exists) {
            this->m_doc.setOpacityTransferRange(text,
                    m_glui_transfer_function_min_opacity->get_float_val(),
                    m_glui_transfer_function_max_opacity->get_float_val());
        }
    }

    this->displayTransferFunction();
}

void TransferFunctionEditorMain::reset()
{
    // add by @hira at 2017/07/30
    //this->m_doc = this->m_doc_initial; // commented out by yoh 2017/08/03

    size_t resolution = m_doc.m_extend_transfer_function_resolution;

    for ( size_t n = 0; n < m_doc.m_color_transfer_function.size(); n++ ) {
        kvs::TransferFunction* ptf = &m_doc.m_color_transfer_function[n];
        *ptf = kvs::TransferFunction( resolution );
    }

    for ( size_t n = 0; n < m_doc.m_opacity_transfer_function.size(); n++ ) {
        kvs::TransferFunction* ptf = &m_doc.m_opacity_transfer_function[n];
        *ptf = kvs::TransferFunction( resolution );
    }

    this->displayTransferFunction();

    // add by @hira at 2017/07/30
    this->setColorTransferFunction();
    this->setOpacityTransferFunction();

    this->redraw();
}

void TransferFunctionEditorMain::apply()
{
    this->m_command->m_parameter.m_parameter_extend_transfer_function = m_doc;

}

void TransferFunctionEditorMain::importFile()
{
    std::string file_name = m_glui_transfer_function_path->get_text();
    this->importFile( file_name );
}

void TransferFunctionEditorMain::exportFile()
{
    std::string file_name = m_glui_transfer_function_path->get_text();
    this->exportFile( file_name, false );
}

void TransferFunctionEditorMain::showTransferFunctionEditorColorFreeformCurve()
{
    m_transfer_function_editor_color_freeform_curve->show();
    m_transfer_function_editor_color_freeform_curve->setResolution( m_doc.m_extend_transfer_function_resolution );
    m_transfer_function_editor_color_freeform_curve->setTransferFunction( m_color_map_palette->colorMap() );
    m_transfer_function_editor_color_freeform_curve->redraw();
}

void TransferFunctionEditorMain::showTransferFunctionEditorColorExpression()
{
    m_transfer_function_editor_color_expression->show();
    m_transfer_function_editor_color_expression->setResolution( m_doc.m_extend_transfer_function_resolution );
    m_transfer_function_editor_color_expression->setEquation( "3*x-1.5", "1.4*sin(3.14*x)", "-3*x+1.5" );
    m_transfer_function_editor_color_expression->redraw();
}

void TransferFunctionEditorMain::showTransferFunctionEditorColorControlPoints()
{
    m_transfer_function_editor_color_control_points->show();
    m_transfer_function_editor_color_control_points->setResolution( m_doc.m_extend_transfer_function_resolution );
    m_transfer_function_editor_color_control_points->setControlPointValue( 0, 0.00,  0,  0, 1.0 );
    m_transfer_function_editor_color_control_points->setControlPointValue( 1, 0.25,  0, 1.0, 1.0 );
    m_transfer_function_editor_color_control_points->setControlPointValue( 2, 0.50,  0, 1.0,  0 );
    m_transfer_function_editor_color_control_points->setControlPointValue( 3, 0.75, 1.0, 1.0,  0 );
    m_transfer_function_editor_color_control_points->setControlPointValue( 4, 1.00, 1.0,  0,  0 );
    m_transfer_function_editor_color_control_points->update();
    m_transfer_function_editor_color_control_points->redraw();
}

void TransferFunctionEditorMain::showTransferFunctionEditorColorSelectColormap()
{
    m_transfer_function_editor_color_select_color_map->show();
    m_transfer_function_editor_color_select_color_map->setResolution( m_doc.m_extend_transfer_function_resolution );
    m_transfer_function_editor_color_select_color_map->setColorMapName( "Rainbow" );
    m_transfer_function_editor_color_select_color_map->redraw();
}

void TransferFunctionEditorMain::showTransferFunctionEditorOpacityFreeformCurve()
{
    m_transfer_function_editor_opacity_freeform_curve->show();
    m_transfer_function_editor_opacity_freeform_curve->setResolution( m_doc.m_extend_transfer_function_resolution );
    m_transfer_function_editor_opacity_freeform_curve->setTransferFunction( m_opacity_map_palette->opacityMap() );
    m_transfer_function_editor_opacity_freeform_curve->redraw();
}

void TransferFunctionEditorMain::showTransferFunctionEditorOpacityExpression()
{
    m_transfer_function_editor_opacity_expression->show();
    m_transfer_function_editor_opacity_expression->setResolution( m_doc.m_extend_transfer_function_resolution );
    m_transfer_function_editor_opacity_expression->setEquation( "x" );
    m_transfer_function_editor_opacity_expression->redraw();
}

void TransferFunctionEditorMain::showTransferFunctionEditorOpacityControlPoints()
{
    m_transfer_function_editor_opacity_control_points->show();
    m_transfer_function_editor_opacity_control_points->setResolution( m_doc.m_extend_transfer_function_resolution );
    m_transfer_function_editor_opacity_control_points->setControlPointValue( 0, 0.00, 0.0 );
    m_transfer_function_editor_opacity_control_points->setControlPointValue( 1, 1.00, 1.0 );
    m_transfer_function_editor_opacity_control_points->update();
    m_transfer_function_editor_opacity_control_points->redraw();
}

void TransferFunctionEditorMain::saveTransferFunctionEditorColorFreeformCurve()
{
    const kvs::ColorMap cmap = m_transfer_function_editor_color_freeform_curve->colorMap();
    this->m_color_map_palette->setColorMap( cmap );
    // delete by @hira at 2016/12/01
    // this->m_doc.m_transfer_function[m_current_transfer_function].setColorMap( cmap );

    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();
    // 色関数定義の取得
    NamedTransferFunctionParameter *trans_color = this->m_doc.getColorTransferFunction(n_select_color);
    if (trans_color == NULL) return;
    // Color Mapの設定
    trans_color->setColorMap(cmap);

//  this->redraw();
    this->hide();
    this->show(n_select_color, n_select_opacity);
}

void TransferFunctionEditorMain::saveTransferFunctionEditorColorExpression()
{
    const kvs::ColorMap cmap = m_transfer_function_editor_color_expression->colorMap();
    this->m_color_map_palette->setColorMap( cmap );
    // delete by @hira at 2016/12/01
    // this->m_doc.m_transfer_function[m_current_transfer_function].setColorMap( cmap );

    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();
    // 色関数定義の取得
    NamedTransferFunctionParameter *trans_color = this->m_doc.getColorTransferFunction(n_select_color);
    if (trans_color == NULL) return;
    // Color Mapの設定
    trans_color->setColorMap(cmap);

//  this->redraw();
    this->hide();
    this->show(n_select_color, n_select_opacity);
}

void TransferFunctionEditorMain::saveTransferFunctionEditorColorControlPoints()
{
    const kvs::ColorMap cmap = m_transfer_function_editor_color_control_points->colorMap();
    this->m_color_map_palette->setColorMap( cmap );
    // delete by @hira at 2016/12/01
    // this->m_doc.m_transfer_function[m_current_transfer_function].setColorMap( cmap );

    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();
    // 色関数定義の取得
    NamedTransferFunctionParameter *trans_color = this->m_doc.getColorTransferFunction(n_select_color);
    if (trans_color == NULL) return;
    // Color Mapの設定
    trans_color->setColorMap(cmap);

//  this->redraw();
    this->hide();
    this->show(n_select_color, n_select_opacity);
}

void TransferFunctionEditorMain::saveTransferFunctionEditorColorSelectColormap()
{
    const kvs::ColorMap cmap = m_transfer_function_editor_color_select_color_map->colorMap();
    this->m_color_map_palette->setColorMap( cmap );
    // delete by @hira at 2016/12/01
    // this->m_doc.m_transfer_function[m_current_transfer_function].setColorMap( cmap );

    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();
    // 色関数定義の取得
    NamedTransferFunctionParameter *trans_color = this->m_doc.getColorTransferFunction(n_select_color);
    if (trans_color == NULL) return;
    // Color Mapの設定
    trans_color->setColorMap(cmap);

//  this->redraw();
    this->hide();
    this->show(n_select_color, n_select_opacity);
}

void TransferFunctionEditorMain::saveTransferFunctionEditorOpacityFreeformCurve()
{
    const kvs::OpacityMap omap = m_transfer_function_editor_opacity_freeform_curve->opacityMap();
    this->m_opacity_map_palette->setOpacityMap( omap );
    // delete by @hira at 2016/12/01
    // this->m_doc.m_transfer_function[m_current_transfer_function].setOpacityMap( omap );

    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();
    // 不透明度関数定義の取得
    NamedTransferFunctionParameter *trans_opacity = this->m_doc.getOpacityTransferFunction(n_select_opacity);
    if (trans_opacity == NULL) return;
    // Opacity Mapの設定
    trans_opacity->setOpacityMap(omap);

//  this->redraw();
    this->hide();
    this->show(n_select_color, n_select_opacity);
}

void TransferFunctionEditorMain::saveTransferFunctionEditorOpacityExpression()
{
    const kvs::OpacityMap omap = m_transfer_function_editor_opacity_expression->opacityMap();
    this->m_opacity_map_palette->setOpacityMap( omap );
    // delete by @hira at 2016/12/01
    // this->m_doc.m_transfer_function[m_current_transfer_function].setOpacityMap( omap );

    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();
    // 不透明度関数定義の取得
    NamedTransferFunctionParameter *trans_opacity = this->m_doc.getOpacityTransferFunction(n_select_opacity);
    if (trans_opacity == NULL) return;
    // Opacity Mapの設定
    trans_opacity->setOpacityMap(omap);

//  this->redraw();
    this->hide();
    this->show(n_select_color, n_select_opacity);
}

void TransferFunctionEditorMain::saveTransferFunctionEditorOpacityControlPoints()
{
    const kvs::OpacityMap omap = m_transfer_function_editor_opacity_control_points->opacityMap();
    this->m_opacity_map_palette->setOpacityMap( omap );
    // delete by @hira at 2016/12/01
    // this->m_doc.m_transfer_function[m_current_transfer_function].setOpacityMap( omap );

    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();
    // 不透明度関数定義の取得
    NamedTransferFunctionParameter *trans_opacity = this->m_doc.getOpacityTransferFunction(n_select_opacity);
    if (trans_opacity == NULL) return;
    // Opacity Mapの設定
    trans_opacity->setOpacityMap(omap);

//  this->redraw();
    this->hide();
    this->show(n_select_color, n_select_opacity);
}


/**
 * 伝達関数の表示を行う。
 */
void TransferFunctionEditorMain::displayTransferFunction()
{
    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();

    this->displayTransferFunction(n_select_color, n_select_opacity);

    return;
}

/**
 * 伝達関数の表示を行う。
 * @param n_color		色関数定義番号（１〜）
 * @param n_opacity		不透明度関数定義番号（１〜）
 */
void TransferFunctionEditorMain::displayTransferFunction(const int n_color, const int n_opacity)
{
    int n_selected_color = n_color;
    int n_selected_opacity = n_opacity;
    if (n_color <= 0) n_selected_color = 1;
    if (n_opacity <= 0) n_selected_opacity = 1;

    char color_function_name[8] = {0x00};
    char opacity_function_name[8] = {0x00};
    sprintf(color_function_name, "C%d", n_selected_color);
    sprintf(opacity_function_name, "O%d", n_selected_opacity);

    const NamedTransferFunctionParameter *trans_color = this->m_doc.getColorTransferFunction(n_selected_color);
    const NamedTransferFunctionParameter *trans_opacity = this->m_doc.getOpacityTransferFunction(n_selected_opacity);

    // 初期表示
    this->clearTransferFunction();

    // 色、不透明度定義番号（1〜）
    this->m_glui_colormap_function->set_int_val(n_selected_color);
    this->m_glui_opacitymap_function->set_int_val(n_selected_opacity);

    if (trans_color != NULL) {
        kvs::ColorMap color_map = trans_color->colorMap();
        m_color_map_palette->setColorMap( color_map );
        const kvs::visclient::FrequencyTable* freq_table = m_command->m_result.findColorFrequencyTable(std::string(color_function_name));
        if ( freq_table != NULL ) {
            m_color_histogram->setTable( *freq_table );
        }

        m_glui_transfer_function_var_color->set_text( trans_color->m_color_variable );
        m_glui_transfer_function_min_color->set_float_val( trans_color->m_color_variable_min );
        m_glui_transfer_function_max_color->set_float_val( trans_color->m_color_variable_max );
    }

    if (trans_opacity != NULL) {
        kvs::OpacityMap opacity_map = trans_opacity->opacityMap();
        m_opacity_map_palette->setOpacityMap( opacity_map );
        const kvs::visclient::FrequencyTable* freq_table = m_command->m_result.findOpacityFrequencyTable(std::string(opacity_function_name));
        if ( freq_table != NULL ) {
            m_opacity_histogram->setTable( *freq_table );
        }

        m_glui_transfer_function_var_opacity->set_text( trans_opacity->m_opacity_variable );
        m_glui_transfer_function_min_opacity->set_float_val( trans_opacity->m_opacity_variable_min );
        m_glui_transfer_function_max_opacity->set_float_val( trans_opacity->m_opacity_variable_max );
    }

    m_glui_resolution->set_int_val( m_doc.m_extend_transfer_function_resolution );
    m_glui_num_transfer->set_int_val(m_doc.m_transfer_function_number);
    m_glui_color_function_synth->set_text( m_doc.m_color_transfer_function_synthesis );
    m_glui_opacity_function_synth->set_text( m_doc.m_opacity_transfer_function_synthesis );
    m_glui_colormap_function->set_int_limits(1, m_glui_num_transfer->get_int_val());
    m_glui_opacitymap_function->set_int_limits(1, m_glui_num_transfer->get_int_val());

    this->redraw();

    m_color_histogram->Redraw();
    m_opacity_histogram->Redraw();
}


void TransferFunctionEditorMain::updateRangeView()
{
    // this->updateRangeView( m_current_transfer_function );    delete b y@hira at 2016/12/01

    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();
    char color_function_name[8] = {0x00};
    char opacity_function_name[8] = {0x00};
    sprintf(color_function_name, "C%d", n_select_color);
    sprintf(opacity_function_name, "O%d", n_select_opacity);
    char tag_c[16] = {0x00};
    char tag_o[16] = {0x00};
    sprintf(tag_c, "%s_var_c", color_function_name);
    sprintf(tag_o, "%s_var_o", opacity_function_name);

    const NamedTransferFunctionParameter *trans_color = this->m_doc.getColorTransferFunction(n_select_color);
    const NamedTransferFunctionParameter *trans_opacity = this->m_doc.getOpacityTransferFunction(n_select_opacity);

    if (trans_color != NULL) {
        kvs::ColorMap color_map = trans_color->colorMap();
        m_color_map_palette->setColorMap( color_map );

        std::vector<size_t> table( 1024, 0);
        m_color_histogram->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() ) );
        const kvs::visclient::FrequencyTable* freq_table = m_command->m_result.findColorFrequencyTable(std::string(color_function_name));
        if ( freq_table != NULL ) {
            m_color_histogram->setTable( *freq_table );
        }
    }

    if (trans_opacity != NULL) {
        kvs::OpacityMap opacity_map = trans_opacity->opacityMap();
        m_opacity_map_palette->setOpacityMap( opacity_map );

        std::vector<size_t> table( 1024, 0);
        m_opacity_histogram->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() ) );
        const kvs::visclient::FrequencyTable* freq_table = m_command->m_result.findOpacityFrequencyTable(std::string(opacity_function_name));
        if ( freq_table != NULL ) {
            m_opacity_histogram->setTable( *freq_table );
        }
    }

    std::stringstream ss_min_c, ss_max_c;
    std::stringstream ss_min_o, ss_max_o;
    ss_min_c << "Server side range min: " << m_command->m_result.m_var_range.min( tag_c );
    ss_max_c << "Server side range max: " << m_command->m_result.m_var_range.max( tag_c );
    ss_min_o << "Server side range min: " << m_command->m_result.m_var_range.min( tag_o );
    ss_max_o << "Server side range max: " << m_command->m_result.m_var_range.max( tag_o );

    m_glui_range_min_color->set_text( ss_min_c.str().c_str() );
    m_glui_range_max_color->set_text( ss_max_c.str().c_str() );
    m_glui_range_min_opacity->set_text( ss_min_o.str().c_str() );
    m_glui_range_max_opacity->set_text( ss_max_o.str().c_str() );

    this->redraw();
    m_color_histogram->Redraw();
    m_opacity_histogram->Redraw();

    return;
}


void TransferFunctionEditorMain::updateRangeEdit()
{
    // delete by @hira at 2016/12/01
    // this->updateRangeEdit( m_current_transfer_function );

    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();
    char color_function_name[8] = {0x00};
    char opacity_function_name[8] = {0x00};
    sprintf(color_function_name, "C%d", n_select_color);
    sprintf(opacity_function_name, "O%d", n_select_opacity);
    char tag_c[16] = {0x00};
    char tag_o[16] = {0x00};
    sprintf(tag_c, "%s_var_c", color_function_name);
    sprintf(tag_o, "%s_var_o", opacity_function_name);

    const float var_min_c = m_command->m_result.m_var_range.min( tag_c );
    const float var_max_c = m_command->m_result.m_var_range.max( tag_c );
    const float var_min_o = m_command->m_result.m_var_range.min( tag_o );
    const float var_max_o = m_command->m_result.m_var_range.max( tag_o );

    m_glui_transfer_function_min_color->set_float_val( var_min_c );
    m_glui_transfer_function_max_color->set_float_val( var_max_c );
    m_glui_transfer_function_min_opacity->set_float_val( var_min_o );
    m_glui_transfer_function_max_opacity->set_float_val( var_max_o );

    return;
}


/**
 * 色関数定義ダイアログを表示する。
 */
void TransferFunctionEditorMain::showTransferFunctionEditorColorFunction()
{
    if (m_transfer_function_editor_color_function->getFunctionType() != COLOR_FUNCTION_DIALOG) {
        m_transfer_function_editor_color_function->hide();
    }
    m_transfer_function_editor_color_function->show(
            COLOR_FUNCTION_DIALOG,
            &this->m_doc.m_color_transfer_function);
}


/**
 * 色関数定義を選択する。
 */
bool TransferFunctionEditorMain::selectTransferFunctionEditorColorFunction(const NamedTransferFunctionParameter *select_trans)
{
    if (select_trans == NULL) return false;
    int num = atoi(select_trans->m_name.substr(1).c_str());
    if (this->m_glui_num_transfer->get_int_val() < num) {
        return false;
    }
    this->m_glui_colormap_function->set_int_val(num);
    this->m_glui_transfer_function_var_color->set_text(select_trans->m_color_variable);

    // 再表示
    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();
    this->hide();
    this->show(n_select_color, n_select_opacity);

    return true;
}


/**
 * 不透明度関数定義ダイアログを表示する。
 */
void TransferFunctionEditorMain::showTransferFunctionEditorOpacityFunction()
{
    if (m_transfer_function_editor_color_function->getFunctionType() != OPACITY_FUNCTION_DIALOG) {
        m_transfer_function_editor_color_function->hide();
    }
    m_transfer_function_editor_color_function->show(
            OPACITY_FUNCTION_DIALOG,
            &this->m_doc.m_opacity_transfer_function);
}


/**
 * 不透明度関数定義を選択する。
 */
bool TransferFunctionEditorMain::selectTransferFunctionEditorOpacityFunction(const NamedTransferFunctionParameter *select_trans)
{
    if (select_trans == NULL) return false;
    int num = atoi(select_trans->m_name.substr(1).c_str());
    if (this->m_glui_num_transfer->get_int_val() < num) {
        return false;
    }
    this->m_glui_opacitymap_function->set_int_val(num);
    this->m_glui_transfer_function_var_opacity->set_text(select_trans->m_opacity_variable);

    // 再表示
    int n_select_color = this->m_glui_colormap_function->get_int_val();
    int n_select_opacity = this->m_glui_opacitymap_function->get_int_val();
    this->hide();
    this->show(n_select_color, n_select_opacity);

    return true;
}

void TransferFunctionEditorMain::setColorTransferFunction()
{
    /**
    this->m_glui_transfer_function_var_color->set_text("");
    this->m_glui_transfer_function_min_color->set_float_val(0.0);
    this->m_glui_transfer_function_max_color->set_float_val(1.0);

    int select_num = this->m_glui_colormap_function->get_int_val();
    const NamedTransferFunctionParameter *trans = this->m_doc.getColorTransferFunction(select_num);
    if (trans == NULL) return;

    this->m_glui_transfer_function_var_color->set_text(trans->m_color_variable);
    this->m_glui_transfer_function_min_color->set_float_val(trans->m_color_variable_min );
    this->m_glui_transfer_function_max_color->set_float_val(trans->m_color_variable_max );
    **/

    this->displayTransferFunction();

    return;
}



void TransferFunctionEditorMain::setOpacityTransferFunction()
{
    /**
    this->m_glui_transfer_function_var_opacity->set_text("");
    this->m_glui_transfer_function_min_opacity->set_float_val(0.0);
    this->m_glui_transfer_function_max_opacity->set_float_val(1.0);

    int select_num = this->m_glui_opacitymap_function->get_int_val();
    const NamedTransferFunctionParameter *trans = this->m_doc.getOpacityTransferFunction(select_num);
    if (trans == NULL) return;

    this->m_glui_transfer_function_var_opacity->set_text(trans->m_opacity_variable);
    this->m_glui_transfer_function_min_opacity->set_float_val(trans->m_opacity_variable_min );
    this->m_glui_transfer_function_max_opacity->set_float_val(trans->m_opacity_variable_max );
    **/

    this->displayTransferFunction();

    return;
}

void TransferFunctionEditorMain::setNumberTransferFunctions()
{
    int num_transfer_function = m_glui_num_transfer->get_int_val();
    m_doc.m_transfer_function_number = num_transfer_function;
    m_glui_colormap_function->set_int_limits(1, num_transfer_function);
    m_glui_opacitymap_function->set_int_limits(1, num_transfer_function);

    int n;
    int current_size;
    char name[8];
    // 色関数を追加、削除する。
    current_size = this->m_doc.m_color_transfer_function.size();
    if (current_size <= num_transfer_function) {
        // 追加を行う
        for (n=current_size+1; n<=num_transfer_function; n++) {
            sprintf(name, "C%d", n);
            this->m_doc.addColorTransferFunction(name, "q1");
        }
    }
    else {
        // 削除を行う
        for (n=num_transfer_function+1; n<=current_size; n++) {
            sprintf(name, "C%d", n);
            this->m_doc.removeColorTransferFunction(name);
        }
    }

    // 不透明度関数を追加、削除する。
    current_size = this->m_doc.m_opacity_transfer_function.size();
    if (current_size <= num_transfer_function) {
        // 追加を行う
        for (n=current_size+1; n<=num_transfer_function; n++) {
            sprintf(name, "O%d", n);
            this->m_doc.addOpacityTransferFunction(name, "q1");
        }
    }
    else {
        // 削除を行う
        for (n=num_transfer_function+1; n<=current_size; n++) {
            sprintf(name, "O%d", n);
            this->m_doc.removeOpacityTransferFunction(name);
        }
    }

    return;
}


void TransferFunctionEditorMain::clearTransferFunction()
{
    std::vector<size_t> table( 1024, 0);
    m_color_histogram->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() ) );
    m_opacity_histogram->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() ) );

    m_glui_transfer_function_var_color->set_text("" );
    m_glui_transfer_function_min_color->set_float_val(0.0);
    m_glui_transfer_function_max_color->set_float_val(1.0);
    m_glui_transfer_function_var_opacity->set_text("");
    m_glui_transfer_function_min_opacity->set_float_val(0.0);
    m_glui_transfer_function_max_opacity->set_float_val(1.0);

    return;
}


} // end of namespace visclient

} // end of namespace kvs
