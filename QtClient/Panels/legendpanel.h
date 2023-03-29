#ifndef LEGENDPANEL_H
#define LEGENDPANEL_H

#include <kvs/OpenGL>

#include <QDockWidget>

#include "QGlue/legendbar.h"
#include "coloreditdialog.h"
#include "QGlue/extCommand.h"

namespace Ui {
class LegendPanel;
}

/**
 * @brief
 *
 */
class LegendPanel : public QDockWidget
{

public:

    Qt::DockWidgetArea default_area=Qt::RightDockWidgetArea;

public:

    explicit LegendPanel(QWidget *parent = 0);

    void bindLegendBar(QGlue::LegendBar* lbar);
    int importFile( const char* fname );
    void exportFile( const std::string& fname );
    void set2UI();
    ~LegendPanel();

private:
    //        void createPanel( QGlue::LegendBar* legend, kvs::ScreenBase* parent );
    void setColorMapFunction(const kvs::visclient::Command* command);
    void populateColormapFunctions(const Command* command);
    //        bool getShowUI();
    //        void changeShowUI();
    void selectTransferFunction();
    void setDisplay();
    void setDivisionColor();
    void setFranelineColor();
    void setCaption();
    //    void colsePanel();
    void showLegendFrameColor();
    void showLegendDivisionColor();

private:

    void setOrientation();
    void setDivision();
    void setDivisionThickness();
    void setFranelineThickness();

private:     // Event handlers
    void onDisplayLegendChanged(int val);
    void onColormapFunctionIndexChanged(int index);
    void onHorizontalToggled(bool checked);
    void onCaptionEdited();
    void onDivIntervalChanged(int arg1);
    void onDivThicknessChanged(int val);
    void onDivColorChanged();
    void onFrameThicknessChanged(int val);
    void onFrameColorBtnClicked();
    void onSetBtnClicked();

private:
    Ui::LegendPanel *ui;

    std::string m_caption;
//    kvs::ScreenBase*  m_screen; ///< pointer to screen
    int m_legend_display_value[1];	// Legendのon/off
    int m_selected_color_map;    // 選択されたカラーマップ
    char m_string_caption[300]={0};	// キャプションの文字列
    int m_bar_val=0;				// バーの向き
    int m_intervals_division=5;
    kvs::RGBColor m_divisionision_rgb_color;
    int m_intervals_thickness=1;
    kvs::RGBColor m_franeline_rgb_color;
    int m_frameline_thickness=1;
    bool m_show_ui;              // Legendの表示／非表示
    QGlue::LegendBar* m_legend;
    ColorEditDialog* m_legend_frame_color;
    ColorEditDialog* m_legend_division_color;

};

#endif // LEGENDPANEL_H
