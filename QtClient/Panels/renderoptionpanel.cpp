#include "renderoptionpanel.h"
#include "ui_renderoptionpanel.h"

#include <QGlue/typedSignalConnect.h>
#include <Client/v3defines.h>
#include <QScreen>

#include <QGlue/boundingbox.h>
#include "Client/LogManager.h"

float RenderoptionPanel::resolution_width_level  = PBVR_RESOLUTION_WIDTH_LEVEL;
float RenderoptionPanel::resolution_height_level = PBVR_RESOLUTION_HEIGHT_LEVEL;

float RenderoptionPanel::data_size_limit = PBVR_PARTICLE_DATA_SIZE_LIMIT;
float RenderoptionPanel::pdensitylevel   = PBVR_PARTICLE_DENSITY;
int   RenderoptionPanel::plimitlevel     = PBVR_PARTICLE_LIMIT;

//bool RenderoptionPanel::isEnabledBoundingBox = INITIAL_STATE_BOUNDING_BOX;


RenderoptionPanel::RenderoptionPanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::RenderoptionPanel)
{
    ui->setupUi(this);
    //pixelRatio = QPaintDevice::devicePixelRatioF();
#ifdef IS_MODE
    pixelRatio = parent->screen()->devicePixelRatio();
#endif
    connect_T(ui->particleDensity,  QDoubleSpinBox, valueChanged,double, &RenderoptionPanel::onParticleDensityChanged);
    connect_T(ui->particleLimit,    QSpinBox,       valueChanged,   int, &RenderoptionPanel::onParticleLimitChanged);
    connect_T(ui->resolutionHeight, QSpinBox,       valueChanged,   int, &RenderoptionPanel::onReslutionHeightChanged);
    connect_T(ui->resolutionWidth,  QSpinBox,       valueChanged,   int, &RenderoptionPanel::onResolutionWidthChanged);
    connect_T(ui->dataSizeLimit,    QSpinBox,       valueChanged,   int, &RenderoptionPanel::onDataSizeLimitChanged);
//    connect_T(ui->isEnabledBoundingBox, QCheckBox, stateChanged, int, &RenderoptionPanel::onIsEnabledBoundingBoxChanged);
    connect_T(ui->applyButton,      QPushButton,    clicked,       bool, &RenderoptionPanel::onApplyButtonClicked)

//    BoundingBoxSynchronizer::instance().addClient(this);
}

RenderoptionPanel::~RenderoptionPanel()
{
    delete ui;
}


/**
 * @brief RenderoptionPanel::updateCommandInfo
 */
void RenderoptionPanel::updateCmd2UI()
{
#ifdef CS_MODE
    this->ui->resolutionHeight->setValue(resolution_height_level);
    this->ui->resolutionWidth->setValue(resolution_width_level);
#endif
#ifdef IS_MODE
    this->ui->resolutionHeight->setValue(resolution_height_level / pixelRatio);
    this->ui->resolutionWidth->setValue(resolution_width_level / pixelRatio);
#endif
    this->ui->particleDensity->setValue(extCommand->m_parameter.m_particle_density);
    this->ui->particleLimit->setValue(extCommand->m_parameter.m_particle_limit);
    this->ui->dataSizeLimit->setValue(extCommand->m_parameter.particle_data_size_limit);
}

/**
 * @brief RenderoptionPanel::on_particleDensity_valueChanged
 * @param val
 */
void RenderoptionPanel::onParticleDensityChanged(double val)
{
    pdensitylevel=val;
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("renderoptionpanel.cpp",__func__,"particleDensityLevel",std::to_string(pdensitylevel));
    }
}

/**
 * @brief RenderoptionPanel::on_particleLimit_valueChanged
 * @param val
 */
void RenderoptionPanel::onParticleLimitChanged(int val)
{
    plimitlevel=val;
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("renderoptionpanel.cpp",__func__,"particleLimit",std::to_string(plimitlevel));
    }
}

/**
 * @brief RenderoptionPanel::on_resolutionHeight_valueChanged
 * @param val
 */
void RenderoptionPanel::onReslutionHeightChanged(int val)
{
    resolution_height_level=val;
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("renderoptionpanel.cpp",__func__,"resolutionHeightLevel",std::to_string(resolution_height_level));
    }
}

/**
 * @brief RenderoptionPanel::on_resolutionWidth_valueChanged
 * @param val
 */
void RenderoptionPanel::onResolutionWidthChanged(int val)
{
    resolution_width_level=val;
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("renderoptionpanel.cpp",__func__,"resolutionWidthLevel",std::to_string(resolution_width_level));
    }
}

/**
 * @brief RenderoptionPanel::onDataSizeLimitChanged
 * @param val
 */
void RenderoptionPanel::onDataSizeLimitChanged(int val)
{
    data_size_limit=(float)val;
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("renderoptionpanel.cpp",__func__,"dataSizeLimit",std::to_string(data_size_limit));
    }
}

/**
 * @brief RenderoptionPanel::onIsEnabledBoundingBoxChanged
 */
//void RenderoptionPanel::onIsEnabledBoundingBoxChanged(bool val) {
//    isEnabledBoundingBox = val;
//    BoundingBoxSynchronizer::instance().synchronize(this, isEnabledBoundingBox);
////    if( m_isRec )
////    {
////        LogManager lg;
////        lg.Write("renderoptionpanel.cpp",__func__,"EnabledBoundBoxChanged",std::to_string(data_size_limit));
////    }
//}

/**
 * @brief RenderoptionPanel::on_applyButton_clicked
 */
void RenderoptionPanel::onApplyButtonClicked()
{
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("renderoptionpanel.cpp",__func__,"applyButton","clicked");
    }
    extCommand->CallBackApply(0);
}

//void RenderoptionPanel::updateBoundingBoxVisibility(bool isVisible) {
//   isEnabledBoundingBox = isVisible;
//    this->ui->isEnabledBoundingBox->setChecked(isEnabledBoundingBox);
//}

void RenderoptionPanel::logPlay(std::string funcName, std::string str[])
{

    if(funcName.compare("onParticleDensityChanged") == 0)
    {
        ui->particleDensity->setValue(std::stod(str[0]));
    }

    if(funcName.compare("onParticleLimitChanged") == 0)
    {
        ui->particleLimit->setValue(std::stoi(str[0]));
    }

    if(funcName.compare("onReslutionHeightChanged") == 0)
    {
        ui->resolutionHeight->setValue(std::stoi(str[0]));
    }

    if(funcName.compare("onResolutionWidthChanged") == 0)
    {
        ui->resolutionWidth->setValue(std::stoi(str[0]));
    }

    if(funcName.compare("onDataSizeLimitChanged") == 0)
    {
        std::cout << __LINE__ << ":" << str[0] << std::endl;
        ui->dataSizeLimit->setValue(std::stoi(str[0]));
    }

    if(funcName.compare("onApplyButtonClicked") == 0)
    {
        this->onApplyButtonClicked();
    }

}


