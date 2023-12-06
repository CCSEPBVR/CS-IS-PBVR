#include "TimeControl.h"
#include "ui_TimeControl.h"

TimeControl::TimeControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeControl)
{
    ui->setupUi(this);
    ui->spinBox->setWrapping( true );
    ui->spinBox->setRange(0,0);
    ui->spinBox_2->setRange(0,0);
    ui->spinBox_3->setRange(0,0);
    connect(ui->spinBox, &QSpinBox::valueChanged, this, &TimeControl::onFutureTimeStepChanged);
}

TimeControl::~TimeControl()
{
    delete ui;
}

void TimeControl::onFutureTimeStepChanged()
{
    m_future_time_step = ui->spinBox->value();
//    qInfo() << "OH";
}

void TimeControl::updateTimeStepMinMax( int min, int max )
{
    if( min != INT_MAX && max != INT_MIN )
    {
        ui->spinBox->setRange(min, max);

        ui->spinBox_2->setRange(min, max);
        ui->spinBox_2->setValue( ui->spinBox_2->minimum() );

        ui->spinBox_3->setRange(min, max);
        ui->spinBox_3->setValue( ui->spinBox_3->maximum());
    }
}
