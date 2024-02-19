#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "filterinfopanel.h"
#include "ui_filterinfopanel.h"
#include <QDebug>


FilterinfoPanel* FilterinfoPanel::instance = 0;

// Filter information parameters
struct
{
    bool    updateUI=false;
    int32_t numNodes;
    int32_t numElements;
    int32_t elemType;
    int32_t fileType;
    int32_t numStep;
    int32_t numVolDiv;
    //    MOD BY)T.Osaki 2020.03.16
    int32_t numVector;
    float minObjectCoord[3];
    float maxObjectCoord[3];
} filterInfoParams;


/**
 * @brief FilterinfoPanel::FilterinfoPanel
 * @param parent
 */
FilterinfoPanel::FilterinfoPanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::FilterinfoPanel)
{
    if (instance!=0){
        qWarning("FilterinfoPanel:: allows only one instance");
        delete instance;
        exit(1);
    }

    instance=this;
    ui->setupUi(this);
    extCommand=extCommand;
    this->hide();
}

/**
 * @brief FilterinfoPanel::~FilterinfoPanel
 */
FilterinfoPanel::~FilterinfoPanel()
{
    delete ui;
}

/**
 * @brief FilterinfoPanel::updateFilterInfo
 *        Thread safe update, no arguments, based on global extCommand
 */
void FilterinfoPanel::updateFilterInfo()
{
#ifdef COMM_MODE_IS
    return;
#else
    filterInfoParams.numNodes   = extCommand->m_number_nodes;
    filterInfoParams.numElements= extCommand->m_number_elements;
    filterInfoParams.elemType   = extCommand->m_element_type;
    filterInfoParams.fileType   = extCommand->m_file_type;
    filterInfoParams.numStep    = extCommand->m_number_step;
    filterInfoParams.numVolDiv  = extCommand->m_number_volume_divide;
    //    MOD BY)T.Osaki 2020.03.16
    filterInfoParams.numVector  = extCommand->m_number_ingredients;

    filterInfoParams.minObjectCoord[0] = extCommand->m_min_object_coord[0];
    filterInfoParams.minObjectCoord[1] = extCommand->m_min_object_coord[1];
    filterInfoParams.minObjectCoord[2] = extCommand->m_min_object_coord[2];

    filterInfoParams.maxObjectCoord[0] = extCommand->m_max_object_coord[0];
    filterInfoParams.maxObjectCoord[1] = extCommand->m_max_object_coord[1];
    filterInfoParams.maxObjectCoord[2] = extCommand->m_max_object_coord[2];

    filterInfoParams.updateUI=true;
    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
    QApplication::postEvent(instance,updateEvent);
#endif
}

/**
 * @brief FilterinfoPanel::updateFilterInfo
 *        Thread safe update, based on jpv::ParticleTransferServerMessage
 * @param reply
 */
void FilterinfoPanel::updateFilterInfo(jpv::ParticleTransferServerMessage reply)
{
#ifdef COMM_MODE_IS
    return;
#else
    filterInfoParams.numNodes   = reply.m_number_nodes;
    filterInfoParams.numElements= reply.m_number_elements;
    filterInfoParams.elemType   = reply.m_element_type;
    filterInfoParams.fileType   = reply.m_file_type;
    filterInfoParams.numStep    = reply.m_number_step;
    filterInfoParams.numVolDiv  = reply.m_number_volume_divide;
    //    MOD BY)T.Osaki 2020.03.16
    filterInfoParams.numVector  = reply.m_number_ingredients;

    filterInfoParams.minObjectCoord[0] = reply.m_min_object_coord[0];
    filterInfoParams.minObjectCoord[1] = reply.m_min_object_coord[1];
    filterInfoParams.minObjectCoord[2] = reply.m_min_object_coord[2];

    filterInfoParams.maxObjectCoord[0] = reply.m_max_object_coord[0];
    filterInfoParams.maxObjectCoord[1] = reply.m_max_object_coord[1];
    filterInfoParams.maxObjectCoord[2] = reply.m_max_object_coord[2];

    filterInfoParams.updateUI=true;
    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
    QApplication::postEvent(instance,updateEvent);
#endif
}

/**
 * @brief FilterinfoPanel::updateFilterInfo
 *        Thread safe update, based on kvs::visclient::PBVRResult
 * @param result
 */
void FilterinfoPanel::updateFilterInfo(kvs::visclient::ReceivedMessage* result)
{
    filterInfoParams.numNodes   = result->numNodes;
    filterInfoParams.numElements= result->numElements;
    filterInfoParams.elemType   = result->elemType;
    filterInfoParams.fileType    = result->fileType;
    filterInfoParams.numStep    = result->numStep;
    filterInfoParams.numVolDiv    = result->numVolDiv;

    filterInfoParams.minObjectCoord[0] = result->minObjectCoord[0];
    filterInfoParams.minObjectCoord[1] = result->minObjectCoord[1];
    filterInfoParams.minObjectCoord[2] = result->minObjectCoord[2];

    filterInfoParams.maxObjectCoord[0] = result->maxObjectCoord[0];
    filterInfoParams.maxObjectCoord[1] = result->maxObjectCoord[1];
    filterInfoParams.maxObjectCoord[2] = result->maxObjectCoord[2];

    filterInfoParams.updateUI=true;
    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
    QApplication::postEvent(instance,updateEvent);
}


/**
 * @brief FilterinfoPanel::customEvent
 *        Process filter info update events
 * @param event
 */
void FilterinfoPanel::customEvent(QEvent * event)
{
    if (filterInfoParams.updateUI){
        ui->allNodes->setValue(   filterInfoParams.numNodes );
        ui->allElements->setValue( filterInfoParams.numElements );
        ui->elementType->setValue( filterInfoParams.elemType );
        ui->fileType->setValue(   filterInfoParams.fileType );
        ui->stepNumber->setValue( filterInfoParams.numStep );
        ui->subVolume->setValue(  filterInfoParams.numVolDiv );
        //    MOD BY)T.Osaki 2020.03.16
        ui->vectorNumber->setValue( filterInfoParams.numVector);

        ui->xAxisMin->setValue(   filterInfoParams.minObjectCoord[0] );
        ui->yAxisMin->setValue(   filterInfoParams.minObjectCoord[1] );
        ui->zAxisMin->setValue(   filterInfoParams.minObjectCoord[2] );

        ui->xAxisMax->setValue(   filterInfoParams.maxObjectCoord[0] );
        ui->yAxisMax->setValue(   filterInfoParams.maxObjectCoord[1] );
        ui->zAxisMax->setValue(   filterInfoParams.maxObjectCoord[2] );
        filterInfoParams.updateUI=false;
    }
}

