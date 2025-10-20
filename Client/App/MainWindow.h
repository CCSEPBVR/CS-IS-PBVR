#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>

#include <kvs/qt/Application>
#include "Screen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>
#include <kvs/Label>

// ウィジェット群(A~Z)
// ABCDEFGHIJKLMNOPQRSTUVWXYZ
#include "AnimationControl.h"
#include "Communication.h"
#include "GlyphEditor.h"
#include "PlotOverLineEditor.h"
#include "PointSizeControl.h"
#include "RepetitionLevelControl.h"
#include "ShadingControl.h"
#include "TransferFunctionEditor.h"
#include "VolumeTransform.h"

// デバッグ
#include <kvs/HydrogenVolumeData>
#include <kvs/CellByCellMetropolisSampling>

#ifdef OPENXR_SCREEN
#include "VRHandControllerListener.h"
#endif

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( kvs::qt::Application& app, QWidget *parent = nullptr );
    ~MainWindow();

private:
    // QtUI関連
    Ui::MainWindow *ui;

    // KVS関連
    kvs::qt::jaea::Screen* m_screen                             = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor            = nullptr;
    kvs::ColorMapBar* m_color_map_bar                           = nullptr;
    kvs::OrientationAxis* m_orientation_axis                    = nullptr;
    kvs::Label* m_fps_label                                     = nullptr;
    kvs::Label* m_time_step_label                               = nullptr;

    // 通信関連
    QWebSocket* m_web_binary_socket                             = nullptr;
    QWebSocket* m_web_text_socket                               = nullptr;

    // ウィジェット群(A~Z)
    // ABCDEFGHIJKLMNOPQRSTUVWXYZ
    QAction* m_animation_control_action = nullptr;
    AnimationControl* m_animation_control = nullptr;

    QAction* m_communication_action = nullptr;
    Communication* m_communication = nullptr;                       // NOTE:通信関係有

    QAction* m_glyph_editor_action = nullptr;
    GlyphEditor* m_glyph_editor = nullptr;                          // NOTE:通信関係有

    QAction* m_plot_over_line_editor_action = nullptr;
    PlotOverLineEditor* m_plot_over_line_editor = nullptr;          // NOTE:通信関係有

    QAction* m_point_size_control_action = nullptr;
    PointSizeControl* m_point_size_control = nullptr;

    QAction* m_repetition_level_control_action = nullptr;
    RepetitionLevelControl* m_repetition_level_control = nullptr;

    QAction* m_shading_control_action = nullptr;
    ShadingControl* m_shading_control = nullptr;

    QAction* m_transfer_function_editor_action = nullptr;
    TransferFunctionEditor* m_transfer_function_editor = nullptr;   // NOTE:通信関係有

    QAction* m_volume_transform_action = nullptr;
    VolumeTransform* m_volume_transform = nullptr;

    // VR関連
#ifdef OPENXR_SCREEN
    VRHandControllerListener* m_vr_listener;
#endif

    // メソッド群
    void initialize();
    void animationControlInitialize();
    void communicationInitialize();
    void glyphEditorInitialize();
    void plotOverLineEditorInitialize();
    void pointSizeControlInitialize();
    void repetitionLevelControlInitialize();
    void shadingControlInitialize();
    void transferFunctionEditorInitialize();
    void volumeTransformInitialize();
    void initializeAfterShow();

private slots:
    void onAnimationControl() { m_animation_control->show(); }
    void onCommunication() { m_communication->show(); }
    void onGlyphEditor() { m_glyph_editor->show(); }
    void onPlotOverLineEditor() { m_plot_over_line_editor->show(); }
    void onPointSizeControl() { m_point_size_control->show(); }
    void onRepetitionLevelControl() { m_repetition_level_control->show(); }
    void onShadingControl() { m_shading_control->show(); }
    void onTransferFunctionEditor() { m_transfer_function_editor->show(); }
    void onVolumeTransform() { m_volume_transform->show(); }

    void onUpdateServerState( bool serverState ); // true:接続中

signals:
    void updateCurrentRepetitionLevel();
};
#endif // MAINWINDOW_H
