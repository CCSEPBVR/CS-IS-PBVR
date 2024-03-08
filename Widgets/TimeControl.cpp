#include "TimeControl.h"
#include "ui_TimeControl.h"

#include "Widgets/MergePanel.h"

#define STOP_LABEL "◼ Stop"
#define START_LABEL "► Start"

TimeControl::TimeControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeControl),
    m_current_time_step( 0 ),
    m_next_time_step( 0 ),
    m_limit_min_time_step( INT_MAX ),
    m_limit_max_time_step( INT_MIN ),
    m_is_loop( false ),
    m_merge( nullptr )
{
    ui->setupUi(this);
    ui->timeStepSBox->setWrapping( true );
    connect(ui->timeStepSBox, &QSpinBox::valueChanged, this, &TimeControl::onNextTimeStepChanged);
    connect(ui->limitMinTimeStepSBox, &QSpinBox::valueChanged, this, &TimeControl::onLimitMinTimeStepChanged);
    connect(ui->limitMaxTimeStepSBox, &QSpinBox::valueChanged, this, &TimeControl::onLimitMaxTimeStepChanged);
    connect(ui->playTogglePBtn, &QPushButton::clicked, this, &TimeControl::onPlayToggleButtonClicked );
    connect( &m_timer, &QTimer::timeout, this, &TimeControl::onTimerStart );
}

TimeControl::~TimeControl()
{
    delete ui;
}

void TimeControl::onNextTimeStepChanged()
{
    m_next_time_step = ui->timeStepSBox->value();
}

void TimeControl::updateTimeStepMinMax(int min, int max, bool isSingleOBject) {
    if (min != INT_MAX && max != INT_MIN) {
        // 計算されたmin maxを設定
        ui->limitMinTimeStepSBox->setRange(min, max);
        ui->limitMaxTimeStepSBox->setRange(min, max);

        // limitMinTimeStepSBoxの値がminより大きい場合は、最小値に設定
        if (ui->limitMinTimeStepSBox->value() > min) {
            // limitMinTimeStepSBoxの値がlimitMinTimeStepSBoxの範囲内であれば変更しない
            if (ui->limitMinTimeStepSBox->value() < ui->limitMinTimeStepSBox->minimum() ||
                ui->limitMinTimeStepSBox->value() > ui->limitMinTimeStepSBox->maximum()) {
                ui->limitMinTimeStepSBox->setValue(ui->limitMinTimeStepSBox->minimum());
            }
        }

        // limitMaxTimeStepSBoxの値がmaxより小さい場合は、最大値に設定
        if (ui->limitMaxTimeStepSBox->value() < max) {
            // limitMaxTimeStepSBoxの値がlimitMaxTimeStepSBoxの範囲内であれば変更しない
            if (ui->limitMaxTimeStepSBox->value() < ui->limitMaxTimeStepSBox->minimum() ||
                ui->limitMaxTimeStepSBox->value() > ui->limitMaxTimeStepSBox->maximum() ||
                isSingleOBject) {
                ui->limitMaxTimeStepSBox->setValue(ui->limitMaxTimeStepSBox->maximum());
            }
        }

        // limitの値をspinBoxに設定
        ui->timeStepSBox->setRange(ui->limitMinTimeStepSBox->value(), ui->limitMaxTimeStepSBox->value());
    }
    else
    {
        //オブジェクトがない時
        ui->limitMinTimeStepSBox->setRange(0,0);
        ui->limitMaxTimeStepSBox->setRange(0,0);
        ui->timeStepSBox->setRange(0,0);
    }
}

void TimeControl::onLimitMinTimeStepChanged()
{
    ui->limitMaxTimeStepSBox->setMinimum(ui->limitMinTimeStepSBox->value());
    ui->timeStepSBox->setMinimum(ui->limitMinTimeStepSBox->value());
}

void TimeControl::onLimitMaxTimeStepChanged()
{
    ui->limitMinTimeStepSBox->setMaximum(ui->limitMaxTimeStepSBox->value());
    ui->timeStepSBox->setMaximum(ui->limitMaxTimeStepSBox->value());
}

void TimeControl::setCurrentTimeStep(int current)
{
    m_current_time_step = current;
    ui->lineEdit->setText(QString::number(m_current_time_step));
}

void TimeControl::onPlayToggleButtonClicked()
{
    if( ui->playTogglePBtn->text() == STOP_LABEL )
    {
        ui->timeStepSBox->setEnabled( true );
        m_timer.stop();
        ui->playTogglePBtn->setText(START_LABEL);
    }
    else
    {
        ui->timeStepSBox->setEnabled( false );
        m_timer.start(70);
        ui->playTogglePBtn->setText(STOP_LABEL);
    }
}

void TimeControl::onTimerStart()
{
    ui->timeStepSBox->setValue(ui->timeStepSBox->value() + 1);
    m_merge->mergeObjects();
}
