#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <kvs/qt/Application>
#include "Screen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>
#include <kvs/Label>

#include "ColorMapSelectorToolBar.h"
#include "PlayBackControlToolBar.h"
#include "TimeStepControlToolBar.h"
#include "TotalParticlesToolBar.h"

#include "AnimationControl.h"
#include "Connect.h"
#include "GlyphEditor.h"
#include "ObjectEditor.h"
#include "PlotOverLine.h"
#include "PointSizeControl.h"
#include "Preference.h"
#include "RepetitionLevelControl.h"
#include "ShadingControl.h"
#include "TransferFunctionEditor.h"
#include "VolumeTransform.h"

#ifdef OPENXR_SCREEN
#include "VRHandControllerListener.h"
#endif

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( kvs::qt::Application& app,
                QWidget *parent = nullptr );
    ~MainWindow();

private:
    void initialize();

private:
    Ui::MainWindow *ui;
    kvs::qt::jaea::Screen* m_screen                             = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor            = nullptr;
    kvs::ColorMapBar* m_color_map_bar                           = nullptr;
    kvs::OrientationAxis* m_orientation_axis                    = nullptr;
    kvs::Label* m_fps_label                                     = nullptr;
    kvs::Label* m_time_step_label                               = nullptr;
#ifdef OPENXR_SCREEN
    VRHandControllerListener* m_vr_listener;
#endif

private: // ツールバー
    ColorMapSelectorToolBar* m_color_map_bar_selector_tool_bar  = nullptr;
    PlayBackControlToolBar* m_play_back_control_tool_bar        = nullptr;
    TimeStepControlToolBar* m_time_step_control_tool_bar        = nullptr;
    TotalParticlesToolBar* m_total_particle_tool_bar            = nullptr;

private: // ウィジェット
    AnimationControl* m_animation_control                       = nullptr;  // Dock
    Connect* m_connect                                          = nullptr;  // Dialog
    GlyphEditor* m_glyph_editor                                 = nullptr;  // Dialog
    ObjectEditor* m_object_editor                               = nullptr;  // Dock
    PlotOverLine* m_plot_over_line                              = nullptr;  // Dock
    PointSizeControl* m_point_size_control                      = nullptr;  // Dock
    Preference* m_preference                                    = nullptr;  // Dialog
    RepetitionLevelControl* m_repetition_level_control          = nullptr;  // Dock
    ShadingControl* m_shading_control                           = nullptr;  // Dock
    TransferFunctionEditor* m_transfer_function_editor          = nullptr;  // Dialog
    VolumeTransform* m_volume_transform                         = nullptr;  // Dock

private slots:
    void onAnimationControl()       { m_animation_control->show(); }
    void onConnect()                { m_connect->show(); }
    void onGlyphEditor()            { m_glyph_editor->show(); }
    void onObjectEditor()           { m_object_editor->show(); }
    void onPlotOverLine()           { m_plot_over_line->show(); }
    void onPointSizeControl()       { m_point_size_control->show(); }
    void onPreference()             { m_preference->show(); }
    void onRepetitionLevelControl() { m_repetition_level_control->show(); }
    void onShadingControl()         { m_shading_control->show(); }
    void onTransferFunctionEditor() { m_transfer_function_editor->show(); }
    void onVolumeTransform()        { m_volume_transform->show(); }
};
#endif // MAINWINDOW_H
