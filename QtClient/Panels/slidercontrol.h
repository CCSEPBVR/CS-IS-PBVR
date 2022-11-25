#ifndef SLIDERCONTROL_H
#define SLIDERCONTROL_H
#include <timecontrolpanel.h>
#include <QWidget>
#include <QSlider>
namespace Ui {
class SliderControl;
}

class SliderControl : public QWidget
{
    Q_OBJECT

public:
    explicit SliderControl(QWidget *parent = 0);
    ~SliderControl();

    TimecontrolPanel* tcp;
    void SetResolution(int res);
    void SetLimit(int min, int max);
    void setRange(int min, int max);
    void setValue(int value);
    int getValue();
    void snapSliderToTick(QSlider* slider);
    void snapAllSlidersToTick();
    void blockAllSliderSignals(bool flag);
    void limitSliderToMax(QSlider* slider, int max);
    void limitSliderToMin(QSlider* slider, int min);
public:
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
