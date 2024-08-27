#include <algorithm>
#include <kvs/Directory>
#include <kvs/File>
#include <kvs/PointImporter>
#include <kvs/PointExporter>
#include <kvs/KVSMLObjectPoint>
#include "CoordinatepanelUI.h"

// 正規化用
extern float* PVBRmaxcoords;
extern float* PVBRmincoords;
// パラメータ保存用
extern std::string param_x_synthesis;
extern std::string param_y_synthesis;
extern std::string param_z_synthesis;

namespace
{
kvs::visclient::CoordinatepanelUI* plegend_ui;


void CallBackSetXSynthesis( const int id )
{
//  plegend_ui->setXCoordinateExpression();
}
void CallBackSetYSynthesis( const int id )
{
//  plegend_ui->setYCoordinateExpression();
}
void CallBackSetZSynthesis( const int id )
{
//  plegend_ui->setZCoordinateExpression();
}
void CallBackReScale( const int id )
{
    plegend_ui->reScale();
}
void CallBackApply( const int id )
{
    plegend_ui->setXCoordinateExpression();
    plegend_ui->setYCoordinateExpression();
    plegend_ui->setZCoordinateExpression();
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

CoordinatepanelUI::CoordinatepanelUI()
{
}

CoordinatepanelUI::~CoordinatepanelUI()
{
}

void CoordinatepanelUI::createPanel( kvs::ScreenBase* parent )
{
    m_screen = parent;
    plegend_ui = this;
    m_glui_coordinate_main = GLUI_Master.create_glui( "Coordinate panel", 0, 900, 100 );


    if ( param_x_synthesis.empty() )
    {
        strcpy( m_string_x_coordinate_expression, "" );
    }
    else
    {
        strcpy( m_string_x_coordinate_expression, param_x_synthesis.c_str() );
    }
    if ( param_y_synthesis.empty() )
    {
        strcpy( m_string_y_coordinate_expression, "" );
    }
    else
    {
        strcpy( m_string_y_coordinate_expression, param_y_synthesis.c_str() );
    }
    if ( param_z_synthesis.empty() )
    {
        strcpy( m_string_z_coordinate_expression, "" );
    }
    else
    {
        strcpy( m_string_z_coordinate_expression, param_z_synthesis.c_str() );
    }
    m_edit_text_x_coordinate_expression = m_glui_coordinate_main->add_edittext( "Coordinate 1", GLUI_EDITTEXT_TEXT, m_string_x_coordinate_expression, -1, CallBackSetXSynthesis );
    m_edit_text_x_coordinate_expression->set_w( 500 );
    m_edit_text_y_coordinate_expression = m_glui_coordinate_main->add_edittext( "Coordinate 2", GLUI_EDITTEXT_TEXT, m_string_y_coordinate_expression, -1, CallBackSetYSynthesis );
    m_edit_text_y_coordinate_expression->set_w( 500 );
    m_edit_text_z_coordinate_expression = m_glui_coordinate_main->add_edittext( "Coordinate 3", GLUI_EDITTEXT_TEXT, m_string_z_coordinate_expression, -1, CallBackSetZSynthesis );
    m_edit_text_z_coordinate_expression->set_w( 500 );

    m_glui_coordinate_main->add_statictext( "" );
    m_buttom_rescale = m_glui_coordinate_main->add_button( "Apply", -1, CallBackApply );

    m_glui_coordinate_main->add_statictext( "" );
    m_buttom_rescale = m_glui_coordinate_main->add_button( "ReScale", -1, CallBackReScale );

    m_glui_coordinate_main->add_statictext( "" );
    m_glui_coordinate_main->add_button( "Close", 0, CallBackClose );
    m_glui_coordinate_main->add_statictext( "" );

    // Set init Parameter

    m_glui_coordinate_main->hide();
    m_show_ui = false;
    m_coordinate = false;
    m_coordinate_for_store_particle = false;
}

bool CoordinatepanelUI::getShowUI()
{
    return m_show_ui;
}

void CoordinatepanelUI::changeShowUI()
{
    m_show_ui = !m_show_ui;
}

void CoordinatepanelUI::setXCoordinateExpression()
{
	// 必要であれば式のエラーチェックをここで行う
    m_coordinate = true;
    m_coordinate_for_store_particle = true;
    param_x_synthesis = std::string( m_string_x_coordinate_expression );
}

void CoordinatepanelUI::setYCoordinateExpression()
{
	// 必要であれば式のエラーチェックをここで行う
    m_coordinate = true;
    m_coordinate_for_store_particle = true;
    param_y_synthesis = std::string( m_string_y_coordinate_expression );
}

void CoordinatepanelUI::setZCoordinateExpression()
{
	// 必要であれば式のエラーチェックをここで行う
    m_coordinate = true;
    m_coordinate_for_store_particle = true;
    param_z_synthesis = std::string( m_string_z_coordinate_expression );
}

bool CoordinatepanelUI::getChangeCoordFlag()
{
    return m_coordinate;
}

bool CoordinatepanelUI::getChangeCoordFlagForStoreParticle()
{
    return m_coordinate_for_store_particle;
}

void CoordinatepanelUI::reScale()
{
    m_reset_flag = true;
}

bool CoordinatepanelUI::getResetViewFlag()
{
    return m_reset_flag;
}

void CoordinatepanelUI::resetResetViewFlag()
{
    m_reset_flag = false;
    m_coordinate = false;
}

void CoordinatepanelUI::resetChangeFlagForStoreParticle()
{
    m_coordinate_for_store_particle = false;
}

void CoordinatepanelUI::setParam( kvs::visclient::VisualizationParameter* param )
{
    param->m_x_synthesis = std::string( param_x_synthesis );
    param->m_y_synthesis = std::string( param_y_synthesis );
    param->m_z_synthesis = std::string( param_z_synthesis );
}


void CoordinatepanelUI::setUISynthesizer()
{
    if ( !param_x_synthesis.empty() )
    {
        // 読み込んだパラメータと現在のパラメータが違う場合だけ設定
        if ( !( m_edit_text_x_coordinate_expression->get_text() == param_x_synthesis ) )
        {
            m_edit_text_x_coordinate_expression->set_text( param_x_synthesis );
            m_coordinate = true;
        }
    }
    if ( !param_y_synthesis.empty() )
    {
        // 読み込んだパラメータと現在のパラメータが違う場合だけ設定
        if ( !( m_edit_text_y_coordinate_expression->get_text() == param_y_synthesis ) )
        {
            m_edit_text_y_coordinate_expression->set_text( param_y_synthesis );
        }
    }
    if ( !param_z_synthesis.empty() )
    {
        // 読み込んだパラメータと現在のパラメータが違う場合だけ設定
        if ( !( m_edit_text_z_coordinate_expression->get_text() == param_z_synthesis ) )
        {
            m_edit_text_z_coordinate_expression->set_text( param_z_synthesis );
        }
    }
}

void CoordinatepanelUI::colsePanel()
{
    m_show_ui = false;
    m_glui_coordinate_main->hide();
}


} // namespace visclient
} // namespace kvs
