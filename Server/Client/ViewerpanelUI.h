#ifndef PBVR__KVS__VISCLIENT__PBVR_VIEWER_PANEL_UI_H_INCLUDE
#define PBVR__KVS__VISCLIENT__PBVR_VIEWER_PANEL_UI_H_INCLUDE

#include <string>
#include <vector>
#include "Label.h"

#include "GL/glui.h"

#include "ViewerBackgroundColor.h"
#include "kvs/glut/OrientationAxis"

namespace kvs
{
namespace visclient
{

class ViewerpanelUI
{

protected:

    kvs::ScreenBase*  m_screen; ///< pointer to screen
    GLUI_Listbox* m_list_font_size;  // Font Size
    GLUI_Button* m_button_back_ground_color;	// Background Color
    int m_select_item;
    kvs::visclient::StepLabel* m_step_label;
    kvs::visclient::FPSLabel* m_fps_label;
    kvs::visclient::LegendBar* m_legend_bar;
    kvs::glut::OrientationAxis* m_orientation_axis;
    bool m_show_ui;              // Viewer Paramter Panelの表示／非表示

public:

    ViewerpanelUI();
    virtual ~ViewerpanelUI();

    GLUI* m_glui_viewer_main;
    void createPanel( kvs::visclient::StepLabel* StepLabel, kvs::visclient::FPSLabel* FPSLabel,
                      kvs::visclient::LegendBar* LegendBer, kvs::glut::OrientationAxis* orientation_axis, kvs::ScreenBase* parent = 0 );
    bool getShowUI();
    void changeShowUI();
    void applyFontType();
    void setFontSize();
    void setBackgroundColor();
    void applyBackgroundColor();
    void colsePanel();
    void showBackgroundColor();

    kvs::visclient::ViewerBackgroundColor* m_bgc;
};

} // namespace visclient
} // namespace kvs

#endif // PBVR__KVS__VISCLIENT__PBVR_VIEWER_PANEL_UI_H_INCLUDE
