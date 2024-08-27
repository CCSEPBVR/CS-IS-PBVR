#include <algorithm>
#include <fstream>
#include <kvs/Directory>
#include <kvs/File>
#include <kvs/PointImporter>
#include <kvs/PointExporter>
#include <kvs/KVSMLObjectPoint>
#include "LegendpanelUI.h"
#include "KVSMLObjectPointWriter.h"
#include "Label.h"

namespace
{
kvs::visclient::LegendpanelUI* plegend_ui;

void CallBackLegendDisplayValue( const int id )
{
//    plegend_ui->setDisplay();
}
void CallBackBarDisplayValue( const int id )
{
//    plegend_ui->setOrientation();
}
void CallBackCaptionString( const int id )
{
//    plegend_ui->setCaption();
}
void CallBackIntervalsNumber( const int id )
{
//    plegend_ui->setDivision();
}
void CallBackIntervalsColor( const int id )
{
    plegend_ui->showLegendDivisionColor();
}
void CallBackIntervalsThickness( const int id )
{
//    plegend_ui->setDivisionThickness();
}
void CallBackFramelinesColor( const int id )
{
    plegend_ui->showLegendFrameColor();
}
void CallBackFramelinesThickness( const int id )
{
//    plegend_ui->setFranelineThickness();
}
void CallBackSelectedTransferFunction( const int id )
{
//    plegend_ui->selectTransferFunction();
}

void CallBackApply( const int id )
{
    plegend_ui->setDisplay();
    plegend_ui->setOrientation();
    plegend_ui->setCaption();
    plegend_ui->setDivision();
    plegend_ui->setDivisionThickness();
    plegend_ui->setDivisionColor();
    plegend_ui->setFranelineThickness();
    plegend_ui->setFranelineColor();
    plegend_ui->selectTransferFunction();
}

void CallBackClose( const int id )
{
    plegend_ui->colsePanel();
}
} // end of namespace

namespace kvs
{
namespace visclient
{

LegendpanelUI::LegendpanelUI()
{
    m_legend_display_value[0] = 0;
    m_divisionision_rgb_color.set( 0, 0, 0 );
    m_franeline_rgb_color.set( 0, 0, 0 );
}

LegendpanelUI::~LegendpanelUI()
{
}

void LegendpanelUI::createPanel( kvs::visclient::LegendBar* legend, kvs::ScreenBase* parent )
{
    const float  SPINNER_SPEED = 0.0;
    m_screen = parent;
    plegend_ui = this;
    m_legend = legend;
    m_glui_legend_main = GLUI_Master.create_glui( "Legend panel", 0, 900, 100 );

    m_checkbox_legend = m_glui_legend_main->add_checkbox( "Display Legend", m_legend_display_value, 0, CallBackLegendDisplayValue );
    m_spinner_color_map = m_glui_legend_main->add_spinner("Color Map Function:C", GLUI_EDITTEXT_INT, NULL, -1, CallBackSelectedTransferFunction);
    // >> start : add by @hira at 2017/07/30
    m_spinner_color_map->set_speed(SPINNER_SPEED);
    // << end : add by @hira at 2017/07/30

    this->setColorMapFunction(m_legend->getCommand());

    m_radio_group_bar = m_glui_legend_main->add_radiogroup( NULL, -1, CallBackBarDisplayValue );
    m_glui_legend_main->add_radiobutton_to_group( m_radio_group_bar, "vertical" );
    m_glui_legend_main->add_radiobutton_to_group( m_radio_group_bar, "horizontal" );

    strcpy( m_string_caption, "" );
    m_text_caption = m_glui_legend_main->add_edittext( "Caption", GLUI_EDITTEXT_TEXT, m_string_caption, -1, CallBackCaptionString );
    m_text_caption->set_w( 200 );

    m_panel_division = m_glui_legend_main->add_panel( "Division", GLUI_PANEL_EMBOSSED );
    m_text_intervals_number = m_glui_legend_main->add_edittext_to_panel( m_panel_division, "Intervals", GLUI_EDITTEXT_INT, NULL, -1, CallBackIntervalsNumber );
    m_text_intervals_number->set_int_val( 5 );
    m_buttom_intervals_color = m_glui_legend_main->add_button_to_panel( m_panel_division, "Edit Color", -1, CallBackIntervalsColor );
    m_text_intervals_thickness = m_glui_legend_main->add_edittext_to_panel( m_panel_division, "Thickness", GLUI_EDITTEXT_INT, NULL, -1, CallBackIntervalsThickness );
    m_text_intervals_thickness->set_int_val( 1 );

    m_panel_frameline = m_glui_legend_main->add_panel( "Frameline", GLUI_PANEL_EMBOSSED );
    m_buttom_framelines_color = m_glui_legend_main->add_button_to_panel( m_panel_frameline, "Edit Color", -1, CallBackFramelinesColor );
    m_text_frameline_thickness = m_glui_legend_main->add_edittext_to_panel( m_panel_frameline, "Thickness", GLUI_EDITTEXT_INT, NULL, -1, CallBackFramelinesThickness );
    m_text_frameline_thickness->set_int_val( 1 );

    m_glui_legend_main->add_statictext( "" );
    m_glui_legend_main->add_button( "Apply", 0, CallBackApply );
    m_glui_legend_main->add_statictext( "" );
    m_glui_legend_main->add_button( "Close", 0, CallBackClose );
    m_glui_legend_main->add_statictext( "" );

    // Set init Parameter
    set2UI();
    m_glui_legend_main->hide();
    m_show_ui = false;

    m_legend_frame_color = new kvs::visclient::LegendFrameColor( this, m_screen );
    m_legend_division_color = new kvs::visclient::LegendDivisionColor( this, m_screen );
}

bool LegendpanelUI::getShowUI()
{
    return m_show_ui;
}

void LegendpanelUI::changeShowUI()
{
    m_show_ui = !m_show_ui;
    // add by @hira at 2016/12/01
    this->setColorMapFunction(m_legend->getCommand());
}

void LegendpanelUI::selectTransferFunction()
{
    m_legend->screenResizedAfterSelectTransferFunction( m_spinner_color_map->get_int_val() );
}

void LegendpanelUI::setDisplay()
{
    if ( m_legend_display_value[0] == 0 )
    {
        m_legend->hide();
    }
    else
    {
        m_legend->show();
    }
}

void LegendpanelUI::setOrientation()
{
    if ( m_radio_group_bar->get_int_val() == 0 )
    {
        //                m_legend->setOrientation( LegendBar::OrientationType::Vertical );
        m_legend->setOrientation( LegendBar::Vertical );
    }
    else
    {
        //                m_legend->setOrientation( LegendBar::OrientationType::Horizontal );
        m_legend->setOrientation( LegendBar::Horizontal );
    }
    m_legend->screenResized();
}

void LegendpanelUI::setDivision()
{
    int val = m_text_intervals_number->get_int_val();
    if ( val < 1 )
    {
        m_text_intervals_number->set_int_val( 1 );
    }
    m_legend->setNumberOfDivisions( val );
}

void LegendpanelUI::setDivisionThickness()
{
    int val = m_text_intervals_thickness->get_int_val();
    if ( val < 0 )
    {
        m_text_intervals_thickness->set_int_val( 0 );
    }
    m_legend->setDivisionLineWidth( val );
}

void LegendpanelUI::setDivisionColor()
{
    m_divisionision_rgb_color = m_legend_division_color->color();
    m_legend->setDivisionLineColor( m_divisionision_rgb_color );
}

void LegendpanelUI::setFranelineThickness()
{
    int val = m_text_frameline_thickness->get_int_val();
    if ( val < 0 )
    {
        m_text_frameline_thickness->set_int_val( 0 );
    }
    m_legend->setBorderWidth( val );
}

void LegendpanelUI::setFranelineColor()
{
    m_franeline_rgb_color = m_legend_frame_color->color();
    m_legend->setBorderColor( m_franeline_rgb_color );
}

void LegendpanelUI::setCaption()
{
    const std::string strval = std::string( m_string_caption );
    m_legend->setCaption( strval );
}

void LegendpanelUI::colsePanel()
{
    m_show_ui = false;
    m_glui_legend_main->hide();
}

void LegendpanelUI::showLegendFrameColor()
{
    m_legend_frame_color->show();
    m_legend_frame_color->redraw();
}

void LegendpanelUI::showLegendDivisionColor()
{
    m_legend_division_color->show();
    m_legend_division_color->redraw();
}

void LegendpanelUI::exportFile( const std::string& fname )
{
    std::ofstream ofs;
    ofs.open( fname.c_str(), std::ofstream::app );
    if ( ofs.fail() )
    {
        std::cerr << "Error: open file " << fname << std::endl;
        return;
    }
    int n_selected_color = m_spinner_color_map->get_int_val();
    char color_name[8] = {0x00};
    sprintf(color_name, "C%d", n_selected_color);

    ofs << "LEGEND_DISPLAY = " << m_legend_display_value[0] << std::endl;
    // modify by @hira at 2016/12/01 : LEGEND_FUNCTIONを数値から色関数名に変更する
    // ofs << "LEGEND_FUNCTION = " << m_list_color_map->get_int_val() << std::endl;
    ofs << "LEGEND_FUNCTION = " << std::string(color_name) << std::endl;
    ofs << "LEGEND_POSITION = " << m_radio_group_bar->get_int_val() << std::endl;
    ofs << "LEGEND_CAPTION = " << m_string_caption << std::endl;
    ofs << "LEGEND_INTERVALS = " << m_text_intervals_number->get_int_val() << std::endl;
    ofs << "LEGEND_INTERVAL_THICKNESS = " << m_text_intervals_thickness->get_int_val() << std::endl;
    ofs << "LEGEND_INTERVAL_COLOR_R = " << ( unsigned int )m_divisionision_rgb_color.r() << std::endl;
    ofs << "LEGEND_INTERVAL_COLOR_G = " << ( unsigned int )m_divisionision_rgb_color.g() << std::endl;
    ofs << "LEGEND_INTERVAL_COLOR_B = " << ( unsigned int )m_divisionision_rgb_color.b() << std::endl;
    ofs << "LEGEND_FRAMELINE_THICKNESS = " << m_text_frameline_thickness->get_int_val() << std::endl;
    ofs << "LEGEND_FRAMELINE_COLOR_R = " << ( unsigned int )m_franeline_rgb_color.r() << std::endl;
    ofs << "LEGEND_FRAMELINE_COLOR_G = " << ( unsigned int )m_franeline_rgb_color.g() << std::endl;
    ofs << "LEGEND_FRAMELINE_COLOR_B = " << ( unsigned int )m_franeline_rgb_color.b() << std::endl;

    ofs.close();
}

int LegendpanelUI::inportFile( const char* fname )
{
    char s[256], *p;
    char k[100][256];
    char v[100][256];
    int cnt = 0;

    FILE* fp;
    if ( ( fp = fopen( fname, "r" ) ) == NULL )
    {
        return -1;
    }
    cnt = 0;
    while ( fgets( s, 256, fp ) != NULL )
    {
        //printf("%s", s);

        if ( strncmp( s, "LEGEND", 6 ) != 0 ) // LEGENDで始まらないキーワードはpassする
        {
            continue;
        }
        p = strstr( s, " = " );
        if ( p == 0 )
        {
            continue;
        }
        *p = '\0';
        strcpy( k[cnt], s );
        strcpy( v[cnt], p + 3 );

        p = strrchr( v[cnt], '\r' );
        if ( p != 0 )  *p = '\0';
        p = strrchr( v[cnt], '\n' );
        if ( p != 0 )  *p = '\0';

        printf( " ==> %s=%s\n", k[cnt], v[cnt] );
        cnt++;
    }
    fclose( fp );

    for ( int i = 0; i < cnt; i++ )
    {
        if ( strcmp( k[i], "LEGEND_DISPLAY" ) == 0 )
        {
            int tmp = atoi( v[i] );
            m_legend_display_value[0] = tmp;
        }
        if ( strcmp( k[i], "LEGEND_FUNCTION" ) == 0 )
        {
            // m_selected_color_map = atoi( v[i] );
            m_selected_color_map = atoi( v[i]+1 );
        }
        if ( strcmp( k[i], "LEGEND_POSITION" ) == 0 )
        {
            m_bar_val = atoi( v[i] );
        }
        if ( strcmp( k[i], "LEGEND_CAPTION" ) == 0 )
        {
            strcpy( m_string_caption, v[i] );
        }
        if ( strcmp( k[i], "LEGEND_INTERVALS" ) == 0 )
        {
            m_intervals_division = atoi( v[i] );
        }
        if ( strcmp( k[i], "LEGEND_INTERVAL_THICKNESS" ) == 0 )
        {
            m_intervals_thickness = atoi( v[i] );
        }
        if ( strcmp( k[i], "LEGEND_INTERVAL_COLOR_R" ) == 0 )
        {
            kvs::UInt8 r = atoi( v[i] );
            kvs::RGBColor tmp( r, m_divisionision_rgb_color.g(), m_divisionision_rgb_color.b() );
            m_divisionision_rgb_color = tmp;
        }
        if ( strcmp( k[i], "LEGEND_INTERVAL_COLOR_G" ) == 0 )
        {
            kvs::UInt8 g = atoi( v[i] );
            kvs::RGBColor tmp( m_divisionision_rgb_color.r(), g, m_divisionision_rgb_color.b() );
            m_divisionision_rgb_color = tmp;
        }
        if ( strcmp( k[i], "LEGEND_INTERVAL_COLOR_B" ) == 0 )
        {
            kvs::UInt8 b = atoi( v[i] );
            kvs::RGBColor tmp( m_divisionision_rgb_color.r(), m_divisionision_rgb_color.g(), b );
            m_divisionision_rgb_color = tmp;
        }
        if ( strcmp( k[i], "LEGEND_FRAMELINE_THICKNESS" ) == 0 )
        {
            m_frameline_thickness = atoi( v[i] );
        }
        if ( strcmp( k[i], "LEGEND_FRAMELINE_COLOR_R" ) == 0 )
        {
            kvs::UInt8 r = atoi( v[i] );
            kvs::RGBColor tmp( r, m_franeline_rgb_color.g(), m_franeline_rgb_color.b() );
            m_franeline_rgb_color = tmp;
        }
        if ( strcmp( k[i], "LEGEND_FRAMELINE_COLOR_G" ) == 0 )
        {
            kvs::UInt8 g = atoi( v[i] );
            kvs::RGBColor tmp( m_franeline_rgb_color.r(), g, m_franeline_rgb_color.b() );
            m_franeline_rgb_color = tmp;
        }
        if ( strcmp( k[i], "LEGEND_FRAMELINE_COLOR_B" ) == 0 )
        {
            kvs::UInt8 b = atoi( v[i] );
            kvs::RGBColor tmp( m_franeline_rgb_color.r(), m_franeline_rgb_color.g(), b );
            m_franeline_rgb_color = tmp;
        }
    }

    return 0;
}

void LegendpanelUI::set2UI()
{
    // modify by @hira at 2016/12/01
    // m_list_color_map->set_int_val( m_selected_color_map );
    if (m_legend != NULL) {
        this->setColorMapFunction(m_legend->getCommand());
    }
    m_spinner_color_map->set_int_val( m_selected_color_map );
    m_radio_group_bar->set_int_val( m_bar_val );
    m_text_caption->set_text( m_string_caption );
    m_text_intervals_number->set_int_val( m_intervals_division );
    m_text_intervals_thickness->set_int_val( m_intervals_thickness );
    m_text_frameline_thickness->set_int_val( m_frameline_thickness );
    GLUI_Master.sync_live_all();
    if ( m_legend )
    {
        selectTransferFunction();
        setDisplay();
        setOrientation();
        setDivision();
        setDivisionThickness();
        setFranelineThickness();
        setCaption();
        m_legend->setDivisionLineColor( m_divisionision_rgb_color );
        m_legend->setBorderColor( m_franeline_rgb_color );
    }
}

void LegendpanelUI::setColorMapFunction(const Command* command)
{
    if (command == NULL) return;
    const kvs::visclient::ExtendedTransferFunctionMessage *m_doc =
            &command->m_parameter.m_parameter_extend_transfer_function;

    const std::vector<NamedTransferFunctionParameter> m_color_transfer_function =
            m_doc->m_color_transfer_function;

    int max_color = 0;
    std::vector<NamedTransferFunctionParameter>::const_iterator itr;
    for (itr=m_color_transfer_function.begin(); itr!=m_color_transfer_function.end(); itr++) {
        const char *name = itr->m_name.c_str();
        int n_color = atoi(name+1);
        if (max_color < n_color) max_color = n_color;
    }
    this->m_spinner_color_map->set_int_limits(1, max_color);

}

} // namespace visclient
} // namespace kvs
