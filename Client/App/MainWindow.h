#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <kvs/qt/Application>
#include "Screen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>
#include <kvs/Label>

#include "WebSocketPair.h"
#include "VizMode.h"

#include "ColorMapSelectorToolBar.h"
#include "PlayBackControlToolBar.h"
#include "TimeStepControlToolBar.h"
#include "TotalParticlesToolBar.h"

#include "AnimationControl.h"
#include "Communication.h"
#include "GlyphEditor.h"
#include "ObjectEditor.h"
#include "PlotOverLineEditor.h"
#include "PointSizeControl.h"
#include "Preference.h"
#include "RepetitionLevelControl.h"
#include "ShadingControl.h"
#include "TransferFunctionEditorWIP.h"
#include "VolumeTransform.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow( kvs::qt::Application& app, QWidget *parent = nullptr );
    ~MainWindow();

public slots:
    void onUpdateStatusBarMessage( const QString& message );

signals:
    void readyScreen();
    void updateInitialRepetitionLevel();
    // FIXME:KPI
    void load( const QString& filePath );
    void save( const QString& filePath );

private:
    static constexpr int DefaultRepetitionLevel = 4;
    static constexpr int DefaultScreenSize      = 620;

    Ui::MainWindow *ui;

    kvs::qt::jaea::Screen* m_screen                             = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor            = nullptr;
    kvs::ColorMapBar* m_color_map_bar                           = nullptr;
    kvs::OrientationAxis* m_orientation_axis                    = nullptr;
    kvs::Label* m_fps_label                                     = nullptr;
    kvs::Label* m_time_step_label                               = nullptr;

    // FIXME:VRで初期位置でkvs::glsl::ParticleBasedRendererを使用している場合、表示されない不具合の対策用です(参考URL:https://github.com/CCSEPBVR/CS-IS-PBVR/blob/develop/Client/Widgets/RepetitionLevelControl.cpp)
    kvs::Xform m_initialize_camera_xform;

    WebSocketPair* m_web_sockets                                = nullptr;
    Viz::Mode* m_viz_mode                                       = nullptr;

#ifdef OPENXR_SCREEN
    VRHandControllerListener* m_vr_listener;
#endif

    // FIXME:KPI
    QAction* m_load_action = nullptr;
    QAction* m_save_action = nullptr;

    // ToolBar
    ColorMapSelectorToolBar* m_color_map_bar_selector_tool_bar = nullptr; // NOTE:KPI関係有
    PlayBackControlToolBar* m_play_back_control_tool_bar       = nullptr; // NOTE:通信関係有
    TimeStepControlToolBar* m_time_step_control_tool_bar       = nullptr; // NOTE:通信関係有 KPI関係有
    TotalParticlesToolBar* m_total_particles_tool_bar          = nullptr;

    // Widget
    QAction* m_animation_control_action                        = nullptr;
    AnimationControl* m_animation_control                      = nullptr; // NOTE:KPI関係有

    QAction* m_communication_action                            = nullptr;
    Communication* m_communication                             = nullptr; // NOTE:通信関係有 KPI関係有

    QAction* m_glyph_editor_action                             = nullptr;
    GlyphEditor* m_glyph_editor                                = nullptr; // NOTE:通信関係有 KPI関係有

    QAction* m_object_editor_action                            = nullptr;
    ObjectEditor* m_object_editor                              = nullptr; // NOTE:通信関係有 KPI関係有

    QAction* m_plot_over_line_editor_action                    = nullptr;
    PlotOverLineEditor* m_plot_over_line_editor                = nullptr; // NOTE:通信関係有 KPI関係有

    QAction* m_point_size_control_action                       = nullptr;
    PointSizeControl* m_point_size_control                     = nullptr; // NOTE:KPI関係有

    QAction* m_preference_action                               = nullptr;
    Preference* m_preference                                   = nullptr;

    QAction* m_repetition_level_control_action                 = nullptr;
    RepetitionLevelControl* m_repetition_level_control         = nullptr; // NOTE:KPI関係有

    QAction* m_shading_control_action                          = nullptr;
    ShadingControl* m_shading_control                          = nullptr; // NOTE:KPI関係有

    QAction* m_transfer_function_editor_action                 = nullptr;
    TransferFunctionEditorWIP* m_transfer_function_editor      = nullptr; // NOTE:通信関係有 KPI関係有

    QAction* m_volume_transform_action                         = nullptr;
    VolumeTransform* m_volume_transform                        = nullptr; // NOTE:KPI関係有

    void toolBarInitialize();
    void animationControlInitialize();
    void communicationInitialize();
    void glyphEditorInitialize();
    void objectEditorInitialize();
    void plotOverLineEditorInitialize();
    void pointSizeControlInitialize();
    void preferenceInitialize();
    void repetitionLevelControlInitialize();
    void shadingControlInitialize();
    void transferFunctionEditorInitialize();
    void volumeTransformInitialize();
    void initializeAfterShow();

private slots:
    void onAnimationControl()       { m_animation_control->show();        }
    void onCommunication()          { m_communication->show();            }
    void onGlyphEditor()            { m_glyph_editor->show();             }
    void onObjectEditor()           { m_object_editor->show();            }
    void onPlotOverLineEditor()     { m_plot_over_line_editor->show();    }
    void onPointSizeControl()       { m_point_size_control->show();       }
    void onPreference()             { m_preference->show();               }
    void onRepetitionLevelControl() { m_repetition_level_control->show(); }
    void onShadingControl()         { m_shading_control->show();          }
    void onTransferFunctionEditor() { m_transfer_function_editor->show(); }
    void onVolumeTransform()        { m_volume_transform->show();         }

    void onUpdateServerState( bool serverState ); // true:接続中

    void onLoad();
    void onSave();
};

#endif // MAINWINDOW_H
