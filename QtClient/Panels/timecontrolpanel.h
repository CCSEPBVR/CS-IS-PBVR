#ifndef TIMECONTROLPANEL_H
#define TIMECONTROLPANEL_H

#include <QDockWidget>

#include "QGlue/extCommand.h"
#include <Client/VisualizationParameter.h>
//#include "slidersynchronizer.h"

namespace Ui {
class TimecontrolPanel;
}

class TimecontrolPanel : public QDockWidget/*, public SliderSynchronizationClient*/ {

public:

    explicit TimecontrolPanel(QWidget* parent);

    ~TimecontrolPanel();
    Qt::DockWidgetArea default_area = Qt::BottomDockWidgetArea;

    void updateCmd2UI(ExtCommand* command);
    void changeShowHide();
    int minValue();
    int maxValue();
    int stepValue();
    int maxValueMax();

    void toggleStop(bool val);
    void setProgress(int progress);
    void setStepValue(int value);
    void toggleActive();
    //    void doChange(kvs::visclient::PBVRParam* param);
    static void requestUpdate(kvs::visclient::VisualizationParameter* param,
                              kvs::visclient::ReceivedMessage* result);

    static int g_curStep;
    static int lavel_time_step;

public: // Slots & Event handlers
    void onTimeSliderMoved(int position);
    void onMinTimeSliderMoved(int position);
    void omMaxTimeSliderMoved(int position);

private:
    void setServerRange(int min, int max);
    void onStopButtonClicked(bool checked);
    void onLastStepToggled(bool checked);
    void customEvent(QEvent* event);

private:
    Ui::TimecontrolPanel* ui;
    static TimecontrolPanel* instance;
    bool m_is_shown = true;
    int result_minMergedTimeStep;
    int result_maxMergedTimeStep;
    int param_minTimeStep;
    int param_maxTimeStep;
    bool blockEventHandling=false;
};

#endif // TIMECONTROLPANEL_H
