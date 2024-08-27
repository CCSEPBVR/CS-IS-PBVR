#include <algorithm>
#include <kvs/Directory>
#include <kvs/File>
#include <kvs/PointImporter>
#include <kvs/PointExporter>
#include <kvs/KVSMLObjectPoint>
#include "LegendpanelUI.h"
#include "KVSMLObjectPointWriter.h"
#include "Label.h"
#include "ViewerpanelUI.h"
#include "ViewerBackgroundColor.h"

// パラメータ保存用
extern std::string param_fonttype;
extern kvs::RGBColor crgb;

namespace
{
kvs::visclient::ViewerpanelUI* pm_viewer_ui;

void CallBackFontSize( const int id )
{
    pm_viewer_ui->setFontSize();
}
void CallBackBackgroundColor( const int id )
{
    pm_viewer_ui->showBackgroundColor();
}
void CallBackClose( const int id )
{
    pm_viewer_ui->colsePanel();
}
} // end of namespace

namespace kvs
{
namespace visclient
{

ViewerpanelUI::ViewerpanelUI()
{
}

ViewerpanelUI::~ViewerpanelUI()
{
}

void ViewerpanelUI::createPanel( kvs::visclient::StepLabel* StepLabel, kvs::visclient::FPSLabel* FPSLabel,
                                 kvs::visclient::LegendBar* LegendBer, kvs::glut::OrientationAxis* orientation_axis, kvs::ScreenBase* parent )
{
    m_step_label = StepLabel;
    m_fps_label = FPSLabel;
    m_legend_bar = LegendBer;
    m_orientation_axis = orientation_axis;
    m_screen = parent;

    pm_viewer_ui = this;
    m_glui_viewer_main = GLUI_Master.create_glui( "Viewer Control panel", 0, 900, 100 );

    m_button_back_ground_color = m_glui_viewer_main->add_button( "Background Color", -1, CallBackBackgroundColor );
    m_glui_viewer_main->add_statictext( "" );

    m_list_font_size = m_glui_viewer_main->add_listbox( "Font Size", &m_select_item, -1, CallBackFontSize );
    m_list_font_size->add_item( 0, "8x13" );
    m_list_font_size->add_item( 1, "9x15" );
    m_list_font_size->add_item( 2, "TIMES_ROMAN_10" );
    m_list_font_size->add_item( 3, "TIMES_ROMAN_24" );
    m_list_font_size->add_item( 4, "HELVETICA_10" );
    m_list_font_size->add_item( 5, "HELVETICA_12" );
    m_list_font_size->add_item( 6, "HELVETICA_18" );

    m_glui_viewer_main->add_statictext( "" );
    m_glui_viewer_main->add_button( "Close", 0, CallBackClose );
    m_glui_viewer_main->add_statictext( "" );

    this->applyFontType();

    m_glui_viewer_main->hide();
    m_show_ui = false;

    m_bgc = new kvs::visclient::ViewerBackgroundColor( this, m_screen );
}

bool ViewerpanelUI::getShowUI()
{
    return m_show_ui;
}

void ViewerpanelUI::changeShowUI()
{
    m_show_ui = !m_show_ui;
}

void ViewerpanelUI::applyFontType()
{
    // Set init Parameter
    if ( param_fonttype.compare( "8x13" ) == 0 )
    {
        m_select_item = 0;
    }
    else if ( param_fonttype.compare( "9x15" ) == 0 )
    {
        m_select_item = 1;
    }
    else if ( param_fonttype.compare( "TIMES_ROMAN_10" ) == 0 )
    {
        m_select_item = 2;
    }
    else if ( param_fonttype.compare( "TIMES_ROMAN_24" ) == 0 )
    {
        m_select_item = 3;
    }
    else if ( param_fonttype.compare( "HELVETICA_10" ) == 0 )
    {
        m_select_item = 4;
    }
    else if ( param_fonttype.compare( "HELVETICA_12" ) == 0 )
    {
        m_select_item = 5;
    }
    else if ( param_fonttype.compare( "HELVETICA_18" ) == 0 )
    {
        m_select_item = 6;
    }
    else
    {
        m_select_item = 0;
    }
    this->setFontSize();
}

void ViewerpanelUI::setFontSize()
{
    switch ( m_select_item )
    {
    case 0:
        m_fps_label->setFontType ( kvs::glut::LegendBar::BITMAP_8_BY_13 );
        m_step_label->setFontType( kvs::glut::LegendBar::BITMAP_8_BY_13 );
        m_legend_bar->setFontType( kvs::glut::LegendBar::BITMAP_8_BY_13 );
        m_orientation_axis->setCharacterType( kvs::glut::OrientationAxis::BITMAP_8_BY_13 );
        param_fonttype = "8x13";
        break;
    case 1:
        m_fps_label->setFontType ( kvs::glut::LegendBar::BITMAP_9_BY_15 );
        m_step_label->setFontType( kvs::glut::LegendBar::BITMAP_9_BY_15 );
        m_legend_bar->setFontType( kvs::glut::LegendBar::BITMAP_9_BY_15 );
        m_orientation_axis->setCharacterType( kvs::glut::OrientationAxis::BITMAP_9_BY_15 );
        param_fonttype = "9x15";
        break;
    case 2:
        m_fps_label->setFontType ( kvs::glut::LegendBar::BITMAP_TIMES_ROMAN_10 );
        m_step_label->setFontType( kvs::glut::LegendBar::BITMAP_TIMES_ROMAN_10 );
        m_legend_bar->setFontType( kvs::glut::LegendBar::BITMAP_TIMES_ROMAN_10 );
        m_orientation_axis->setCharacterType( kvs::glut::OrientationAxis::BITMAP_TIMES_ROMAN_10 );
        param_fonttype = "TIMES_ROMAN_10";
        break;
    case 3:
        m_fps_label->setFontType ( kvs::glut::LegendBar::BITMAP_TIMES_ROMAN_24 );
        m_step_label->setFontType( kvs::glut::LegendBar::BITMAP_TIMES_ROMAN_24 );
        m_legend_bar->setFontType( kvs::glut::LegendBar::BITMAP_TIMES_ROMAN_24 );
        m_orientation_axis->setCharacterType( kvs::glut::OrientationAxis::BITMAP_TIMES_ROMAN_24 );
        param_fonttype = "TIMES_ROMAN_24";
        break;
    case 4:
        m_fps_label->setFontType ( kvs::glut::LegendBar::BITMAP_HELVETICA_10 );
        m_step_label->setFontType( kvs::glut::LegendBar::BITMAP_HELVETICA_10 );
        m_legend_bar->setFontType( kvs::glut::LegendBar::BITMAP_HELVETICA_10 );
        m_orientation_axis->setCharacterType( kvs::glut::OrientationAxis::BITMAP_HELVETICA_10 );
        param_fonttype = "HELVETICA_10";
        break;
    case 5:
        m_fps_label->setFontType ( kvs::glut::LegendBar::BITMAP_HELVETICA_12 );
        m_step_label->setFontType( kvs::glut::LegendBar::BITMAP_HELVETICA_12 );
        m_legend_bar->setFontType( kvs::glut::LegendBar::BITMAP_HELVETICA_12 );
        m_orientation_axis->setCharacterType( kvs::glut::OrientationAxis::BITMAP_HELVETICA_12 );
        param_fonttype = "HELVETICA_12";
        break;
    case 6:
        m_fps_label->setFontType ( kvs::glut::LegendBar::BITMAP_HELVETICA_18 );
        m_step_label->setFontType( kvs::glut::LegendBar::BITMAP_HELVETICA_18 );
        m_legend_bar->setFontType( kvs::glut::LegendBar::BITMAP_HELVETICA_18 );
        m_orientation_axis->setCharacterType( kvs::glut::OrientationAxis::BITMAP_HELVETICA_18 );
        param_fonttype = "HELVETICA_18";
        break;
    default:
        break;
    }
}

void ViewerpanelUI::setBackgroundColor()
{
    crgb = m_bgc->color();
    m_screen->background()->setColor( crgb );
}

void ViewerpanelUI::applyBackgroundColor()
{
    m_screen->background()->setColor( crgb );
}

void ViewerpanelUI::colsePanel()
{
    m_show_ui = false;
    m_glui_viewer_main->hide();
}

void ViewerpanelUI::showBackgroundColor()
{
    m_bgc->show();
    m_bgc->redraw();
}

} // namespace visclient
} // namespace kvs
