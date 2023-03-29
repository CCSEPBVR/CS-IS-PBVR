//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08
#include <GL/glew.h>
#include <QOpenGLContext>

#include "timecontrolpanel.h"
#include "ui_timecontrolpanel.h"

#include "Client/VisualizationParameter.h"
#include "QGlue/extCommand.h"
#include "QGlue/renderarea.h"

#define STOP_LABEL "◼ Stop"
#define START_LABEL "► Start"


int TimecontrolPanel::g_curStep=-1;
int TimecontrolPanel::lavel_time_step=-1;

//Static access instance needed for thread safety
TimecontrolPanel* TimecontrolPanel::instance = 0;

TimecontrolPanel::TimecontrolPanel(QWidget *parent ) :
    QDockWidget(parent),
    ui(new Ui::TimecontrolPanel)
{
    if (instance!=0){
        qWarning("TimecontrolPanel:: allows only one instance");
        delete instance;
    }
    instance=this;
    ui->setupUi(this);
    this->hide();
    ui->stop_button->setText(START_LABEL);
    ui->stop_button->setChecked(false);

#ifdef IS_MODE
    ui->m_progress->hide();
    ui->LBL_Progress->hide();
    connect(ui->lastStepCHK,&QCheckBox::toggled,
            this, &TimecontrolPanel::onLastStepToggled);
    if (ui->lastStepCHK->isChecked()){
        ui->sliderControl->setDisabled(true);
    }
#else
    ui->lastStepCHK->hide();
#endif
    connect(ui->stop_button,&QPushButton::clicked,
            this, &TimecontrolPanel::onStopButtonClicked);
    this->adjustSize();
    ui->dockWidgetContents->layout()->setSizeConstraint(QLayout::SetDefaultConstraint);
}

TimecontrolPanel::~TimecontrolPanel()
{
    delete ui;
}

/**
 * @brief TimecontrolPanel::updateCmd2UI, updates UI from command structure
 * @param command
 */
void TimecontrolPanel::updateCmd2UI(ExtCommand *command)
{
    ui->m_progress->setValue(0);
    ui->sliderControl->setValue(0);


#ifdef CS_MODE
    setServerRange(command->m_parameter.m_min_time_step,
                   command->m_parameter.m_max_time_step);
    this->onLastStepToggled(false); // Ensure rest of UI syncs up with initial check state.
#else
    this->onLastStepToggled(ui->lastStepCHK->isChecked()); // Ensure rest of UI syncs up with initial check state.
#endif
}

/**
 * @brief TimecontrolPanel::changeShowHide, toggles visibility of ui
 */
void TimecontrolPanel::changeShowHide()
{
    m_is_shown = !m_is_shown;
    ui->m_progress->setVisible(m_is_shown);
    //    ui->m_time_slider->setVisible(m_is_shown);
    //    ui->m_min_time_slider->setVisible(m_is_shown);
    //    ui->m_max_time_slider->setVisible(m_is_shown);
}

/**
 * @brief TimecontrolPanel::minValue
 * @return selected min time step
 */
int TimecontrolPanel::minValue()
{
    qInfo("TimecontrolPanel::minValue %d",ui->sliderControl->min );
    return ui->sliderControl->min;
}
/**
 * @brief TimecontrolPanel::maxValue
 * @return selected max time step
 */
int TimecontrolPanel::maxValue(){
    qInfo("TimecontrolPanel::maxValue %d",ui->sliderControl->max );
    return ui->sliderControl->max;
}
/**
 * @brief TimecontrolPanel::stepValue
 * @return selected time step
 */
int TimecontrolPanel::stepValue(){
    qInfo("TimecontrolPanel::stepValue %d",ui->sliderControl->curTime );
    return ui->sliderControl->curTime;
}

void TimecontrolPanel::setProgress(int progress){
    ui->m_progress->setValue(progress);
}
void TimecontrolPanel::setStepValue(int value){
#ifdef IS_MODE
    if (extCommand->lastStepCheckBoxState){
        value=extCommand->param.maxServerTimeStep;
    }
#endif
    ui->sliderControl->setValue(value);
    qInfo(" TC 121 SETTING TSTEP: %d -> %d",extCommand->m_parameter.m_time_step ,value);
    extCommand->m_parameter.m_time_step=value;
}

void TimecontrolPanel::setServerRange(int min, int max)
{
    qInfo("########### Set server range %d %d",min,max);
//    extCommand->param.maxTimeStep=max;
//    extCommand->param.minTimeStep=min;
    ui->sliderControl->setRange(min,max);
        extCommand->m_parameter.m_max_time_step=ui->sliderControl->max;
        extCommand->m_parameter.m_min_time_step=ui->sliderControl->min;
}
#include <QMutex>
QMutex mutex;
void TimecontrolPanel::requestUpdate(  kvs::visclient::VisualizationParameter* param,
                                       kvs::visclient::ReceivedMessage* result)
{
     std::cout<<"@####TimecontrolPanel::requestUpdate starts"<<std::endl;
     mutex.lock();
    instance->param_maxTimeStep=param->m_max_server_time_step;
    instance->param_minTimeStep=param->m_min_server_time_step;
    instance->result_maxMergedTimeStep=result->m_max_merged_time_step;
    instance->result_minMergedTimeStep=result->m_min_merged_time_step;
    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
    QApplication::postEvent(instance,updateEvent);
    mutex.lock();

    param->m_max_time_step=instance->maxValue();
    param->m_min_time_step=instance->minValue();
    std::cout<<"@####TimecontrolPanel::requestUpdate ends"<<std::endl;
    mutex.unlock();
}

void TimecontrolPanel::customEvent(QEvent * event)
{
        std::cout<<"@@####TimecontrolPanel::on_updateRequest"<<std::endl;
#ifdef CS_MODE
    setServerRange(result_minMergedTimeStep,result_maxMergedTimeStep);
#else
    setServerRange(result_minMergedTimeStep,result_maxMergedTimeStep);
    if (extCommand->lastStepCheckBoxState){
        setStepValue(param_maxTimeStep);
    }
    else {
        setStepValue(extCommand->param.timeStep);
    }
#endif
    std::cout<<"@@####TimecontrolPanel::on_updateRequest ends"<<std::endl;

    mutex.unlock();
}

void TimecontrolPanel::toggleStop(bool val){
    ui->stop_button->setChecked(val);
}

void TimecontrolPanel::toggleActive()
{
    bool checked=ui->stop_button->isChecked();
    if ( g_curStep >= 0 )
    {
        ui->stop_button->setText(checked?STOP_LABEL:START_LABEL);
#ifdef CS_MODE
        extCommand->m_is_under_animation = checked;
#endif
        extCommand->m_is_under_communication = checked;
    }
//    extCommand->m_glut_timer->startSingleShot();
    extCommand->m_screen->toggleTimer(checked);
}

void TimecontrolPanel::omMaxTimeSliderMoved(int position)
{
    qInfo(" TC 179 SETTING MAXTIMESTEP: %d -> %d", extCommand->m_parameter.m_max_time_step ,position);
    extCommand->m_parameter.m_max_time_step = position;
    extCommand->m_glut_timer->startSingleShot();
}

void TimecontrolPanel::onMinTimeSliderMoved(int position)
{
    extCommand->m_parameter.m_min_time_step = position;
    extCommand->m_glut_timer->startSingleShot();
}

void TimecontrolPanel::onTimeSliderMoved(int position)
{
    qInfo("TimecontrolPanel::onTimeSliderMoved %d", position);
    g_curStep=position;
    qInfo(" TE 198 SETTING TSTEP: %d -> %d",extCommand->m_parameter.m_time_step ,g_curStep);
    extCommand->m_parameter.m_time_step= g_curStep;
#ifdef IN_SITU
    extCommand->m_glut_timer->startSingleShot();
#endif
//    DEL BY)T.Osaki 2020.03.05 -> 2020.03.09 Abort
    extCommand->m_glut_timer->startSingleShot();
}

void TimecontrolPanel::onStopButtonClicked(bool checked)
{
    if (blockEventHandling) return;
    blockEventHandling=true;
    qInfo("TimecontrolPanel::onStopButtonClicked");
    this->toggleActive();
    blockEventHandling=false;
}

void TimecontrolPanel::onLastStepToggled(bool checked)
{
    if (blockEventHandling) return;
    blockEventHandling=true;
    extCommand->lastStepCheckBoxState=checked;
    ui->sliderControl->setDisabled(checked);
//    MOD BY)T.Osaki 2020.03.05
//    setStepValue(extCommand->m_parameter.m_max_server_time_step);
    setStepValue(extCommand->m_parameter.m_min_server_time_step);
    blockEventHandling=false;
    extCommand->m_glut_timer->startSingleShot();
}
