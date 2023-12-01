#ifndef PBVR__KVS__VISCLIENT__PBVR_COORDIANTE_PANEL_UI_H_INCLUDE
#define PBVR__KVS__VISCLIENT__PBVR_COORDIANTE_PANEL_UI_H_INCLUDE

#include <string>
#include <vector>
#include "Label.h"
//#include "../Communication/ParticleTransferProtocol.h"

#include "GL/glui.h"

namespace kvs
{
namespace visclient
{

class CoordinatepanelUI
{

protected:

    kvs::ScreenBase*  m_screen; ///< pointer to screen
    GLUI_EditText* m_edit_text_x_coordinate_expression;	// X座標の式
    GLUI_EditText* m_edit_text_y_coordinate_expression;	// Y座標の式
    GLUI_EditText* m_edit_text_z_coordinate_expression;	// Z座標の式
    char m_string_x_coordinate_expression[300];	    // X座標式の文字列
    char m_string_y_coordinate_expression[300];	    // Y座標式の文字列
    char m_string_z_coordinate_expression[300];	    // Z座標式の文字列
    GLUI_Button* m_buttom_rescale;	// リスケール
    GLUI_Button* m_buttom_close;		// クローズボタン
    bool m_reset_flag;             // Viewの正規化フラグ
    bool m_coordinate;    // 座標式に変更があった場合
    bool m_coordinate_for_store_particle; // 座標式に変更があった場合(粒子保存機能用)
    bool m_show_ui;               // Coordfinate Panelの表示／非表示

public:

    CoordinatepanelUI();
    virtual ~CoordinatepanelUI();

    GLUI* m_glui_coordinate_main;
    void createPanel( kvs::ScreenBase* parent = 0 );
    bool getShowUI();
    bool getResetViewFlag();
    bool getChangeCoordFlag();
    bool getChangeCoordFlagForStoreParticle();
    void resetResetViewFlag();
    void resetChangeFlagForStoreParticle();
    void changeShowUI();
    void setXCoordinateExpression();
    void setYCoordinateExpression();
    void setZCoordinateExpression();
    void setUISynthesizer();
    void reScale();
    void colsePanel();

    // サーバーに送信するメッセージを設定するメソッド
    void setParam( kvs::visclient::VisualizationParameter* param );
};

} // namespace visclient
} // namespace kvs

#endif // __KVS_VISCLIENT_PBVR_COORDIANTE_PANEL_UI_IH__
