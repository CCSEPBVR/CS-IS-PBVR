#ifndef PBVR__KVS__VISCLIENT__LEGEND_PANEL_UI_H_INCLUDE
#define PBVR__KVS__VISCLIENT__LEGEND_PANEL_UI_H_INCLUDE

#include <string>
#include <vector>
#include "Label.h"

#include "GL/glui.h"

#include "LegendFrameColor.h"
#include "LegendDivisionColor.h"

namespace kvs
{
namespace visclient
{

class LegendpanelUI
{

private:

    kvs::ScreenBase*  m_screen; ///< pointer to screen
    GLUI_Checkbox* m_checkbox_legend;	// Legendのon/off
    int m_legend_display_value[1];	// Legendのon/off
    int m_selected_color_map;    // 選択されたカラーマップ
    GLUI_EditText* m_text_caption;	// キャプションの文字列
    char m_string_caption[300];	// キャプションの文字列
    GLUI_RadioGroup* m_radio_group_bar;	// バーの向き
    int m_bar_val;				// バーの向き
    GLUI_Panel* m_panel_division;	// 目盛り
    GLUI_EditText* m_text_intervals_number;		// 目盛りの数
    int m_intervals_division;
    GLUI_Button* m_buttom_intervals_color;		// 目盛りの色
    kvs::RGBColor m_divisionision_rgb_color;
    GLUI_EditText* m_text_intervals_thickness;	// 目盛りの太さ
    int m_intervals_thickness;
    GLUI_Panel* m_panel_frameline;	// 枠線
    GLUI_Button* m_buttom_framelines_color;		// 枠線の色
    kvs::RGBColor m_franeline_rgb_color;
    GLUI_EditText* m_text_frameline_thickness;	// 枠線の太さ
    int m_frameline_thickness;
    // GLUI_Listbox* m_list_color_map;	// 伝達関数の指定		delete by @hira at 2016/12/01 : Spinnerに変更
    GLUI_Spinner *m_spinner_color_map;	// 伝達関数の指定
    bool m_show_ui;              // Legendの表示／非表示
    kvs::visclient::LegendBar* m_legend;
    kvs::visclient::LegendFrameColor* m_legend_frame_color;
    kvs::visclient::LegendDivisionColor* m_legend_division_color;

public:

    LegendpanelUI();
    virtual ~LegendpanelUI();

    GLUI* m_glui_legend_main;
    void createPanel( kvs::visclient::LegendBar* legend, kvs::ScreenBase* parent = 0 );
    bool getShowUI();
    void changeShowUI();
    void selectTransferFunction();
    void setDisplay();
    void setOrientation();
    void setDivision();
    void setDivisionThickness();
    void setDivisionColor();
    void setFranelineThickness();
    void setFranelineColor();
    void setCaption();
    void set2UI();
    void colsePanel();
    void showLegendFrameColor();
    void showLegendDivisionColor();
    void exportFile( const std::string& fname );    // パラメータ出力
    int inportFile( const char* fname );    // パラメータ入力

private:
    void setColorMapFunction(const Command* command);

};

} // namespace visclient
} // namespace kvs

#endif // __KVS_VISCLIENT_PBVR_LEGEND_PANEL_UI_IH__
