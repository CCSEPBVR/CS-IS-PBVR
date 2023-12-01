/*
 * NormalVectorSelectionUI.cpp
 *
 *  Created on: 2017/01/13
 *      Author: hira
 */

#include "NormalVectorSelectionUI.h"

namespace kvs {
namespace visclient {

NormalVectorSelectionUI::NormalVectorSelectionUI() {
    // TODO 自動生成されたコンストラクター・スタブ

}

NormalVectorSelectionUI::~NormalVectorSelectionUI() {
    // TODO Auto-generated destructor stub
}

void NormalVectorSelectionUI::createPanel( const int client_server_mode )
{

    m_glui_normal_main = GLUI_Master.create_glui( "Normal Vector panel", 0, 800, 100 );
    m_normal_panel = m_glui_normal_main->add_panel( "Select Normal Vector", GLUI_PANEL_EMBOSSED );

    m_select_normal = m_glui_normal_main->add_radiogroup_to_panel( m_normal_panel );
    m_glui_normal_main->add_radiobutton_to_group( m_select_normal, "Grain Size" );
    m_glui_normal_main->add_radiobutton_to_group( m_select_normal, "Local Coordinates" );
    m_glui_normal_main->add_radiobutton_to_group( m_select_normal, "Global Coordinates      " );

    m_button_panel = m_glui_normal_main->add_panel( "", GLUI_PANEL_NONE );

    m_btn_apply = m_glui_normal_main->add_button_to_panel(m_button_panel, "Apply", -1 );
    m_glui_normal_main->add_column_to_panel(m_button_panel, false);
    m_btn_cancel = m_glui_normal_main->add_button_to_panel(m_button_panel, "Cancel", -1 );
    m_glui_normal_main->hide();

    return;
}


bool NormalVectorSelectionUI::isShowNormalVectorUI()
{
    return m_isshow_normalvector;
}

void NormalVectorSelectionUI::changeShowNormalVectorUI()
{
    m_isshow_normalvector = ! m_isshow_normalvector;
}

int NormalVectorSelectionUI::show()
{
    m_glui_normal_main->show();
    m_isshow_normalvector = true;

    return 0;
}

void NormalVectorSelectionUI::hide()
{
    m_glui_normal_main->hide();
    m_isshow_normalvector = false;
}


} /* namespace visclient */
} /* namespace kvs */
