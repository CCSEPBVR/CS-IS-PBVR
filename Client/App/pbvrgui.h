#ifndef PBVRGUI_H
#define PBVRGUI_H

#include "ExtendedKVS/Screen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>
#include <kvs/Label>
#include <QMainWindow>

#include "Widgets/Preference.h"
#include "Widgets/MergePanel.h"
#include "Widgets/VolumeTransform.h"
#include "Widgets/Connect.h"
#include "Widgets/AnimationControls.h"
#include "Widgets/DataProperties.h"
#include "Widgets/RenderOptions.h"
#include "Widgets/RepetitionLevelControl.h"
#include "Widgets/DisplayPointSizeControl.h"
#include "Widgets/ShadingController.h"
#include "Widgets/TransferFunctionEditor.h"
#include "Widgets/Coordinates.h"
#include "Widgets/ToolBars.h"
#include "Widgets/GlyphEditor.h"
#include "Widgets/PlotOverLine.h"

namespace Ui {
class PBVRGUI;
}

class PBVRGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit PBVRGUI(kvs::qt::Application& app, QWidget *parent = nullptr);
    kvs::qt::jaea::Screen* screen() const { return m_screen; }
    kvs::StochasticRenderingCompositor* compositor() const { return m_compositor; }
    kvs::ColorMapBar* colorMapBar() const { return m_color_map_bar; }
    kvs::OrientationAxis* orientationAxis() const { return m_orientation_axis; }
    kvs::Label* fpsLabel() const { return m_fps_label; }
    kvs::Label* timeStepLabel() const { return m_time_step_label; }
    void initializePanels();
    const kvs::Xform getInitializedCameraXform() { return m_initialize_camera_xform; }
    ~PBVRGUI();

private:
    Ui::PBVRGUI *ui;
    kvs::qt::jaea::Screen* m_screen;
    kvs::StochasticRenderingCompositor* m_compositor;
    kvs::ColorMapBar* m_color_map_bar;
    kvs::OrientationAxis* m_orientation_axis;
    kvs::Label* m_fps_label;
    kvs::Label* m_time_step_label;

    Preference      m_preference;
    MergePanel      m_merge;
    VolumeTransform m_volumeTransform;
    Connect         m_connect;
    AnimationControls m_animation_controls;
    DataProperties m_data_properties;
    RenderOptions m_render_options;
    RepetitionLevelControl m_repetition_level_control;
    DisplayPointSizeControl m_display_point_size_control;
    ShadingController m_shading_controller;
    TransferFunctionEditor m_transfer_function_editor;
    Coordinates m_coordinates;
    TimeControllerA m_time_controller_A;
    TimeControllerB m_time_controller_B;
    TotalParticles m_total_particles;
    ColorMapBarSelector m_color_map_bar_selector;
    GlyphEditor m_glyph_editor;
    PlotOverLine m_plot_over_line;

    const kvs::Xform m_initialize_camera_xform;

private:
    void onPreference()      { m_preference.show(); }
    void onMerge()           { m_merge.show(); }
    void onVolumeTransform() { m_volumeTransform.show(); }
    void onConnect()         { m_connect.show(); }
    void onAnimationControl() { m_animation_controls.show(); }
    void onFilterInfomation() { m_data_properties.show(); }
    void onRenderOptions() { m_render_options.show(); }
    void onRepetitionLevelControl() { m_repetition_level_control.show(); }
    void onDisplayPointSizeControl() { m_display_point_size_control.show(); }
    void onShadingControl() { m_shading_controller.show(); }
    void onTransferFunctionEditor() { m_transfer_function_editor.show(); }
    void onCoordinates() { m_coordinates.show(); }

    void keyPressEvent( QKeyEvent *event );
};

#endif // PBVRGUI_H
