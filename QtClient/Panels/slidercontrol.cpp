#include "slidercontrol.h"
#include "ui_slidercontrol.h"

// stepSize is the number of "intermediate" tics between two "integers"
const int stepSize=100;
const int tickInterval=1;
SliderControl::SliderControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SliderControl)
{
    ui->setupUi(this);
    tcp=(TimecontrolPanel*)parent;
#ifdef IS_MODE
    ui->maxSlider->hide();
    ui->minSlider->hide();
    ui->label_2->hide();
    ui->label_3->hide();
    ui->timeSlider->setTickPosition(QSlider::TicksBothSides);
#else
    ui->timeSlider->setTickPosition(QSlider::NoTicks);
#endif
    ui->minSlider->setTickInterval(stepSize);
    ui->minSlider->setPageStep(stepSize);
    ui->minSlider->setTracking(false);
    ui->maxSlider->setTickInterval(stepSize);
    ui->maxSlider->setPageStep(stepSize);
    ui->maxSlider->setTracking(false);
    ui->timeSlider->setTickInterval(tickInterval*stepSize);
    ui->timeSlider->setPageStep(stepSize);
    ui->timeSlider->setTracking(false);
    this->adjustSize();
    this->layout()->setSizeConstraint(QLayout::SetDefaultConstraint);
}

SliderControl::~SliderControl()
{
    delete ui;
}

/**
 * @brief SliderControl::blockAllSliderSignals, enable/disable slider events
 * @param flag, true to block, false to allow
 */
void SliderControl::blockAllSliderSignals(bool flag)
{
    ui->minSlider->blockSignals(flag);
    ui->maxSlider->blockSignals(flag);
    ui->timeSlider->blockSignals(flag);
}

/**
 * @brief SliderControl::setRange, sets the allowed slider range
 * @param min
 * @param max
 */
void SliderControl::setRange(int min, int max)
{
    qInfo("SliderControl::setRange %d %d", min,  max);
    ui->minVal->setText(QString::number(min));
    ui->maxVal->setText(QString::number(max));
    ui->maxSlider->setRange(min*stepSize,max*stepSize);
    ui->minSlider->setRange(min*stepSize,max*stepSize);
    ui->timeSlider->setRange(min*stepSize,max*stepSize);

    //DEL BY)T.Osaki 2020.03.06
//    ui->maxSlider->setValue(max*stepSize);
//    ui->minSlider->setValue(min*stepSize);
    //MOD BY)T.Osaki 2020.03.06

#ifdef IS_MODE
    this->min=min;
    this->max=max;
#else
    this->min = ui->minSlider->value()/100;
    this->max = ui->maxSlider->value()/100;
#endif
}

void SliderControl::setValue(int value)
{
    qInfo("SliderControl::setValue %d  ", value);
    curTime=value;
    ui->timeSlider->setValue(value*stepSize);
}

int SliderControl::getValue( ){
    return ui->timeSlider->value()/stepSize;
}
/**
 * @brief SliderControl::snapSliderToTick, snap slider to closest 'integer' tickmark
 * @param slider
 */
void SliderControl::snapSliderToTick(QSlider* slider)
{
    slider->blockSignals(true);
    float val=round((float)slider->value()/stepSize);
    slider->setValue(val*stepSize);
    slider->blockSignals(false);
}
/**
 * @brief SliderControl::snapAllSlidersToTick, snap all sliders to tickmarks
 */
void SliderControl::snapAllSlidersToTick()
{
    snapSliderToTick(ui->maxSlider);
    snapSliderToTick(ui->minSlider);
    snapSliderToTick(ui->timeSlider);
}
/**
 * @brief SliderControl::limitSliderToMax, limit slider to a max value which is inside the current range
 * @param slider
 * @param max
 */
void SliderControl::limitSliderToMax(QSlider* slider, int max)
{
    slider->blockSignals(true);
    if (slider->value() >max)
        slider->setValue(max);
    slider->blockSignals(false);
}
/**
 * @brief SliderControl::limitSliderToMin limit slider to a min value which is inside the current range
 * @param slider
 * @param min
 */
void SliderControl::limitSliderToMin(QSlider* slider, int min)
{
    slider->blockSignals(true);
    if (slider->value() <min)
        slider->setValue(min);
    slider->blockSignals(false);
}


/**
 * @brief SliderControl::on_timeSlider_valueChanged
 *        Triggered when time slider move is completed
 * @param value new value (scaled by stepSize)
 */
void SliderControl::on_timeSlider_valueChanged(int value)
{
    qInfo("SliderControl::on_slider_valueChanged %d", value);
#ifdef IS_MODE
    limitSliderToMax(ui->timeSlider,ui->maxSlider->value()*(value/100));
#else
    limitSliderToMax(ui->timeSlider,ui->maxSlider->value() );
    limitSliderToMin(ui->timeSlider,ui->minSlider->value() );
#endif
    snapAllSlidersToTick();

    curTime=ui->timeSlider->value()/stepSize;
    ui->timeVal->setText(QString::number(curTime));
    tcp->onTimeSliderMoved(curTime);
}

/**
 * @brief SliderControl::on_timeSlider_sliderMoved
 *        Triggered when time slider moves
 * @param position new position (scaled by stepSize)
 */
void SliderControl::on_timeSlider_sliderMoved(int position)
{
    //    qInfo("SliderControl::on_slider_sliderMoved %d", position);
    int val=round((float)position/stepSize);
    ui->timeVal->setText(QString::number(val));
}

/**
 * @brief SliderControl::on_maxSlider_valueChanged
 *        Triggered when max slider move is completed
 * @param value new value (scaled by stepSize)
 */
void SliderControl::on_maxSlider_valueChanged(int value)
{
    qInfo("SliderControl::on_maxSlider_valueChanged %d" ,value);
//    limitSliderToMax(ui->minSlider , value-stepSize);
    limitSliderToMax(ui->minSlider , value);
    limitSliderToMax(ui->timeSlider, value);
//    limitSliderToMin(ui->maxSlider , ui->minSlider->minimum() + stepSize);
    limitSliderToMin(ui->maxSlider , ui->minSlider->minimum());
    snapAllSlidersToTick();
    max=ui->maxSlider->value()/stepSize;
    //ADD BY)T.Osaki 2020.03.06
    on_timeSlider_valueChanged(max);
}

/**
 * @brief SliderControl::on_maxSlider_sliderMoved
 *        Triggered when max slider moves
 * @param position new position (scaled by stepSize)
 */
void SliderControl::on_maxSlider_sliderMoved(int position)
{
//    limitSliderToMax(ui->minSlider,position-stepSize);
    limitSliderToMax(ui->minSlider,position);
    limitSliderToMax(ui->timeSlider,position);
}

/**
 * @brief SliderControl::on_minSlider_valueChanged
 *        Triggered when min slider move is completed
 * @param value new value (scaled by stepSize)
 */
void SliderControl::on_minSlider_valueChanged(int value)
{
    qInfo("SliderControl::on_minSlider_valueChanged %d", value);
//    limitSliderToMin(ui->maxSlider, value+stepSize);
    limitSliderToMin(ui->maxSlider, value);
    limitSliderToMin(ui->timeSlider,value);
//    limitSliderToMax(ui->minSlider,  ui->maxSlider->maximum() - stepSize);
    limitSliderToMax(ui->minSlider,  ui->maxSlider->maximum());
    snapAllSlidersToTick();
    min=ui->minSlider->value()/stepSize;
    //ADD BY)T.Osaki 2020.03.06
    on_timeSlider_valueChanged(min);
}

/**
 * @brief SliderControl::on_minSlider_sliderMoved
 *        Triggered when min slider moves
 * @param position new position (scaled by stepSize)
 */
void SliderControl::on_minSlider_sliderMoved(int position)
{
//    limitSliderToMin(ui->maxSlider, position+stepSize);
    limitSliderToMin(ui->maxSlider, position);
    limitSliderToMin(ui->timeSlider,position);
}
