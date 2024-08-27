/*
 * NormalVectorPanel.cpp
 *
 *  Created on: 2017/01/13
 *      Author: hira
 */

#include "NormalVectorPanel.h"

namespace
{
kvs::visclient::NormalVectorPanel* m_normalvector_panel;


void CallBackShowNormal( const int id )
{
    m_normalvector_panel->showNormal();
}

}

namespace kvs {
namespace visclient {

/**
 * コンストラクタ
 */
NormalVectorPanel::NormalVectorPanel() {

}

/**
 * デストラクタ
 */
NormalVectorPanel::~NormalVectorPanel() {

}

void NormalVectorPanel::createNormalVectorPanel(GLUI* glui)
{
    ::m_normalvector_panel = this;

    int width = 350;
    GLUI_Panel* obj_panel_normal = glui->add_panel( "Normal Vector", 1 );
    obj_panel_normal->set_alignment( GLUI_ALIGN_LEFT );

    // Grain size
    // Local coordinates
    // Global coordinates
    m_lbl_normalvector = glui->add_statictext_to_panel( obj_panel_normal, "Global coordinates" );
    m_lbl_normalvector->set_w(160);

    glui->add_column_to_panel(obj_panel_normal, false);
    m_btn_select = glui->add_button_to_panel( obj_panel_normal, "Select", -1, CallBackShowNormal);

    // 法線ベクトル選択ダイアログを開く
    createDialogNormalVectorSelection();
}


void NormalVectorPanel::showNormal()
{
    if ( m_dialogNormalVector->isShowNormalVectorUI() )
    {
        m_dialogNormalVector->hide();
    }
    else
    {
        m_dialogNormalVector->show();
    }
}

void NormalVectorPanel::createDialogNormalVectorSelection()
{
    m_dialogNormalVector = new NormalVectorSelectionUI();
    m_dialogNormalVector->createPanel(1);
}

} /* namespace visclient */
} /* namespace kvs */
