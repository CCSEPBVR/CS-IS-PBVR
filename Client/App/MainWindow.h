#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

#include <kvs/ColorMapBar>
#include <kvs/Label>
#include <kvs/OrientationAxis>
#include <kvs/StochasticRenderingCompositor>
#include <kvs/qt/Application>
#include "Screen.h" // <kvs/qt/Screen>

#include "VizMode.h"
#include "WebSocketPair.h"

#ifdef OPENXR_SCREEN
#include "VRHandControllerListener.h"
#endif

#include "ColorMapSelectorToolBar.h"
#include "PlayBackControlToolBar.h"
#include "TimeStepControlToolBar.h"
#include "TotalParticlesToolBar.h"

#include "AnimationControl.h"
#include "Communication.h"
#include "GlyphEditor.h"
#include "ObjectEditor.h"
#include "PlotOverLineEditor.h"
#include "PlotOverTimeEditor.h"
#include "PointSizeControl.h"
#include "Preference.h"
#include "RepetitionLevelControl.h"
#include "ShadingControl.h"
#include "EnsembleTransferFunctionEditor.h"
#include "TransferFunctionEditor.h"
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

signals:
    void screenInitialized();
    void updateInitialRepetitionLevel();
    void load( const QString& filePath ); // KPI
    void save( const QString& filePath ); // KPI

public slots:
    void onUpdateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords );
    void onUpdateStatusBarMessage( const QString& message );
    void onUpdateServerState( const bool serverState ); // true: 接続中 , false: 未接続

    // ObjectEditor
    void onUpdateNumberOfVector( const int numberOfVector );

private:
    static constexpr int k_default_repetition_level  = 4;
    static constexpr int k_default_screen_resolution = 620;

    Ui::MainWindow *ui;

    kvs::qt::jaea::Screen* m_screen                  = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor = nullptr;
    kvs::ColorMapBar* m_color_map_bar                = nullptr;
    kvs::OrientationAxis* m_orientation_axis         = nullptr;
    kvs::Label* m_fps_label                          = nullptr;
    kvs::Label* m_time_step_label                    = nullptr;
    kvs::PolygonObject* m_dummy_object               = nullptr;

    WebSocketPair* m_web_sockets = nullptr;
    Viz::Mode* m_viz_mode        = nullptr;

#ifdef OPENXR_SCREEN
    VRHandControllerListener* m_vr_listener;
#endif

    QAction* m_load_action = nullptr; // KPI
    QAction* m_save_action = nullptr; // KPI

    // ToolBar
    ColorMapSelectorToolBar* m_color_map_bar_selector_tool_bar = nullptr; // KPI
    PlayBackControlToolBar*  m_play_back_control_tool_bar      = nullptr; //     WebSocket Operator
    TimeStepControlToolBar*  m_time_step_control_tool_bar      = nullptr; // KPI WebSocket Operator
    TotalParticlesToolBar*   m_total_particles_tool_bar        = nullptr;

    // Action
    QAction* m_animation_control_action        = nullptr;
    QAction* m_communication_action            = nullptr;
    QAction* m_glyph_editor_action             = nullptr;
    QAction* m_object_editor_action            = nullptr;
    QAction* m_plot_over_line_editor_action    = nullptr;
    QAction* m_plot_over_time_editor_action    = nullptr;
    QAction* m_point_size_control_action       = nullptr;
    QAction* m_preference_action               = nullptr;
    QAction* m_repetition_level_control_action = nullptr;
    QAction* m_shading_control_action                   = nullptr;
    QAction* m_ensemble_transfer_function_editor_action = nullptr;
    QAction* m_transfer_function_editor_action          = nullptr;
    QAction* m_volume_transform_action                  = nullptr;

    // Widget
    AnimationControl* m_animation_control              = nullptr; // KPI
    Communication* m_communication                     = nullptr; // KPI WebSocket Operator
    GlyphEditor* m_glyph_editor                        = nullptr; // KPI WebSocket Operator
    ObjectEditor* m_object_editor                      = nullptr; // KPI WebSocket Operator
    PlotOverLineEditor* m_plot_over_line_editor        = nullptr; // KPI WebSocket Operator
    PlotOverTimeEditor* m_plot_over_time_editor        = nullptr; // KPI WebSocket Operator
    PointSizeControl* m_point_size_control             = nullptr; // KPI
    Preference* m_preference                           = nullptr;
    RepetitionLevelControl* m_repetition_level_control = nullptr; // KPI
    ShadingControl* m_shading_control                  = nullptr; // KPI
    EnsembleTransferFunctionEditor* m_ensemble_transfer_function_editor = nullptr;
    TransferFunctionEditor* m_transfer_function_editor = nullptr; // KPI WebSocket Operator
    VolumeTransform* m_volume_transform                = nullptr; // KPI

    // Initializer
    void initializeToolBar();
    void initializeMenuBar();

    void initializeAnimationControl();
    void initializeCommunication();
    void initializeGlyphEditor();
    void initializeObjectEditor();
    void initializePlotOverLineEditor();
    void initializePlotOverTimeEditor();
    void initializePointSizeControl();
    void initializePreference();
    void initializeRepetitionLevelControl();
    void initializeShadingControl();
    void initializeEnsembleTransferFunctionEditor();
    void initializeTransferFunctionEditor();
    void initializeVolumeTransform();

    void initializeShaderAndFonts();

private slots:
    void onAnimationControl()       { m_animation_control->show();        }
    void onCommunication()          { m_communication->show();            }
    void onGlyphEditor()            { m_glyph_editor->show();             }
    void onObjectEditor()           { m_object_editor->show();            }
    void onPlotOverLineEditor()     { m_plot_over_line_editor->show();    }
    void onPlotOverTimeEditor()     { m_plot_over_time_editor->show();    }
    void onPointSizeControl()       { m_point_size_control->show();       }
    void onPreference()             { m_preference->show();               }
    void onRepetitionLevelControl() { m_repetition_level_control->show(); }
    void onShadingControl()         { m_shading_control->show();          }
    void onEnsembleTransferFunctionEditor() { m_ensemble_transfer_function_editor->show(); }
    void onTransferFunctionEditor() { m_transfer_function_editor->show(); }
    void onVolumeTransform()        { m_volume_transform->show();         }
    void onUpdateIsEnsemble( bool isEnsemble );

    void onLoad(); // KPI
    void onSave(); // KPI
};

#endif // MAINWINDOW_H
