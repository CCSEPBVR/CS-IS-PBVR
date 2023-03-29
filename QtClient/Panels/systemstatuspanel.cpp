//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08
#include <GL/glew.h>
#include <QOpenGLContext>

#include "systemstatuspanel.h"
#include "ui_systemstatuspanel.h"
#include <QDebug>

#include <QGlue/typedSignalConnect.h>
#include <QGlue/renderarea.h>
#include <QGlue/systeminfo.h>

int SystemstatusPanel::m_no_repsampling=0;
int SystemstatusPanel::interval_msec=0;
struct
{
    int32_t nparticles;
    std::string filterParamFileName;
} systemInfoParams;

SystemstatusPanel* SystemstatusPanel::instance = 0;

SystemstatusPanel::SystemstatusPanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::SystemstatusPanel),
    updateTimer(this)
{
    if (instance!=0){
        qWarning("SystemstatusPanel:: allows only one instance");
        delete instance;
        exit(1);
    }

    instance=this;
    ui->setupUi(this);

    connect(&updateTimer, &QTimer::timeout,
            this, &SystemstatusPanel::onUpdateTimer);
    connect_T(ui->samplingInterval,QSpinBox,valueChanged,int,
              &SystemstatusPanel::onSamplingIntervalChanged);
    connect(ui->disableSampling_chk, &QCheckBox::stateChanged,
            this, &SystemstatusPanel::onDisableSamplingToggled);
    connect(ui->setParam_btn, &QPushButton::clicked,
            this,&SystemstatusPanel::onSetParamClicked);

    updateTimer.start(3000);

#ifdef IS_MODE
    ui->parameterFilename->hide();
    ui->LBL_ParamFile->hide();
    ui->disableSampling_chk->hide();
    ui->LBL_disableSampling->hide();
    resize(sizeHint());
#endif
}

/**
 * @brief SystemstatusPanel::~SystemstatusPanel, destructor
 */
SystemstatusPanel::~SystemstatusPanel()
{
    delete ui;
}


/**
 * @brief SystemstatusPanel::updateMemoryInfo, update GPU memory
 */
void SystemstatusPanel::updateMemoryInfo()
{
    int c,t;
    size_t cx,tx,px;

    if (extCommand->m_screen){

        if (systemInfo::getGPUType(extCommand->m_screen->context())
                == systemInfo::GPU_UNKNOWN)
        {
            ui->gpu_usage_bar->setHidden(true);
            ui->LBL_GPUmemory->setHidden(true);
        }
        else {
            systemInfo::GetVideoMem(&c,&t);
            ui->gpu_usage_bar->setValue(c);
            ui->gpu_usage_bar->setMaximum(t);
        }

        systemInfo::getMemoryInfo(&cx,&tx,&px);
        QString format=  QString::number( cx )+"/"+QString::number( tx )+"MB";
        //("+QString::number( px )+"%)";
        ui->cpu_useage_bar->setFormat(format);
        ui->cpu_useage_bar->setValue(std::floor(px));
    }
    return;
}


/**
 * @brief SystemstatusPanel::requestUpdate thread safe update request
 */
void SystemstatusPanel::updateSystemStatus(int nparticles)
{
    systemInfoParams.nparticles=nparticles;
    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
    QApplication::postEvent(instance,updateEvent);
}

/**
 * @brief SystemstatusPanel::updateSystemStatus
 */
void SystemstatusPanel::updateSystemStatus()
{
#ifdef CS_MODE

    if(!extCommand->m_parameter.m_filter_parameter_filename.empty()){
    systemInfoParams.filterParamFileName=extCommand->m_parameter.m_filter_parameter_filename;
    }
    if(!extCommand->m_parameter.m_input_directory.empty()){
    systemInfoParams.filterParamFileName=extCommand->m_parameter.m_input_directory;
    }
#endif
    systemInfoParams.nparticles=extCommand->m_detailed_particles[0]->nvertices();
    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
    QApplication::postEvent(instance,updateEvent);
}

/**
 * @brief SystemstatusPanel::customEvent
 * @param event
 */
void SystemstatusPanel::customEvent(QEvent * event)
{

    static int send_cnt=0;
    static int recv_cnt=0;
    using namespace kvs::visclient;
    const ParticleServer::Status srvstat = extCommand->m_server.getStatus();

    if ( srvstat == ParticleServer::Sending ){
        recv_cnt=0;
        send_cnt++;
        if (send_cnt>20){
            send_cnt=0;
        }
    }
    else if ( srvstat == ParticleServer::Recieving ){
        send_cnt=0;
        recv_cnt++;
        if (recv_cnt>20){
            recv_cnt=0;
        }

    }
    else {
         send_cnt=0;
         recv_cnt=0;
    }
    //DEL BY)T.Osaki 2020.05.25
    //ui->ReceiveProgress->setValue(recv_cnt);
    //ui->SendProgress->setValue(send_cnt);
    if (systemInfoParams.nparticles > 0){
        ui->particleNumber->setText(QString::number(systemInfoParams.nparticles));
    }

//    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
//    QApplication::postEvent(instance,updateEvent);
#ifdef CS_MODE
    ui->parameterFilename->setText(QString::fromStdString(systemInfoParams.filterParamFileName));
#endif
    updateMemoryInfo();
}

/**
 * @brief SystemstatusPanel::on_updateTimer internal timer event handler
 */
void SystemstatusPanel::onUpdateTimer()
{
//    qInfo("onUpdateTimer");
    updateMemoryInfo();
}

/**
 * @brief SystemstatusPanel::on_samplingInterval_valueChanged, update sampling interval
 * @param arg1
 */
void SystemstatusPanel::onSamplingIntervalChanged(int arg1)
{
    interval_msec=arg1;
}

/**
 * @brief SystemstatusPanel::on_disableSampling_chk_stateChanged, enable/disable resampling
 * @param arg1
 */
void SystemstatusPanel::onDisableSamplingToggled(int arg1)
{
    m_no_repsampling = (arg1>0) ? 1:0;
}

/**
 * @brief SystemstatusPanel::on_setParam_btn_clicked , callback for apply button
 */
void SystemstatusPanel::onSetParamClicked()
{
//    // 20210106 yodo append
//    std::cout << "SystemstatusPanel::onSetParamClicked() calls extCommand->CAllBackApply()" << std::endl;

    extCommand->CallBackApply(0);
}

