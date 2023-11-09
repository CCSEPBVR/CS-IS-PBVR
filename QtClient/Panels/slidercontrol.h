#ifndef SLIDERCONTROL_H
#define SLIDERCONTROL_H
#include <Panels/timecontrolpanel.h>
#include <QWidget>
#include <QSlider>

#include "slidersynchronizer.h"

namespace Ui {
class SliderControl;
}

class SliderControl : public QWidget, public SliderSynchronizationClient
{
    Q_OBJECT

public:
    explicit SliderControl(QWidget *parent = 0);
    ~SliderControl();

    TimecontrolPanel* tcp;
    void SetResolution(int res);
    void SetLimit(int min, int max);
    void setRange(int min, int max, bool blockSignalsFlag = false);
    void setValue(int value, bool blockSignalsFlag = false);
    void setMinValue(int value, bool blockSignalsFlag = false);
    void setMaxValue(int value, bool blockSignalsFlag = false);
    int getValue();
    void snapSliderToTick(QSlider* slider);
    void snapAllSlidersToTick();
    void blockAllSliderSignals(bool flag);
    void limitSliderToMax(QSlider* slider, int max);
    void limitSliderToMin(QSlider* slider, int min);
    void updateSliders (int min, int max, int curTime, int minTime, int maxTime);

public:
    static const int stepSize=100;
    static const int tickInterval=1;

    // min/max range of sliders
    int range_min;
    int range_max;

    // current value of sliders
    int min, max, curTime;
private slots:

    void on_timeSlider_sliderMoved(int position);

    void on_timeSlider_valueChanged(int value);

    void on_maxSlider_sliderMoved(int position);

    void on_maxSlider_valueChanged(int value);

    void on_minSlider_sliderMoved(int position);

    void on_minSlider_valueChanged(int value);

private:
    Ui::SliderControl *ui;
};

#endif // SLIDERCONTROL_H
