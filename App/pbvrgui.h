#ifndef PBVRGUI_H
#define PBVRGUI_H

#include "ExtendedKVS/Screen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>
#include <kvs/Label>
#include <QMainWindow>

#include "Widgets/Preference.h"
#include "Widgets/Merge.h"
#include "Widgets/VolumeTransform.h"
#include "Widgets/Connect.h"
#include "Widgets/TimeControl.h"
#include "Widgets/AnimationControls.h"
#include "Widgets/DataProperties.h"
#include "Widgets/DataSummary.h"
#include "Widgets/RenderOptions.h"
#include "Widgets/RepetitionLevelControl.h"

namespace Ui {
class PBVRGUI;
}

class PBVRGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit PBVRGUI(kvs::qt::Application& app, QWidget *parent = nullptr);
    void initialize();
    Preference& preference() { return this->m_preference; };
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
    Merge           m_merge;
    VolumeTransform m_volumeTransform;
    Connect         m_connect;
    TimeControl     m_timeControl;
    AnimationControls m_animation_controls;
    DataProperties m_data_properties;
    DataSummary m_data_summary;
    RenderOptions m_render_options;
    RepetitionLevelControl m_repetition_level_control;

private:
    void onPreference()      { m_preference.show(); }
    void onMerge()           { m_merge.show(); }
    void onVolumeTransform() { m_volumeTransform.show(); }
    void onConnect()         { m_connect.show(); }
    void onAnimationControl() { m_animation_controls.show(); }
    void onFilterInfomation() { m_data_properties.show(); }
    void onRenderOptions() { m_render_options.show(); }
    void onRepetitionLevelControl() { m_repetition_level_control.show(); }

    void keyPressEvent( QKeyEvent *event );
};

#endif // PBVRGUI_H
