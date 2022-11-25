#include "renderoptionpanel.h"
#include "ui_renderoptionpanel.h"

#include <QGlue/typedSignalConnect.h>
#include <Client/v3defines.h>

float RenderoptionPanel::resolution_width_level  = PBVR_RESOLUTION_WIDTH_LEVEL;
float RenderoptionPanel::resolution_height_level = PBVR_RESOLUTION_HEIGHT_LEVEL;

float RenderoptionPanel::data_size_limit = PBVR_PARTICLE_DATA_SIZE_LIMIT;
float RenderoptionPanel::pdensitylevel   = PBVR_PARTICLE_DENSITY;
int   RenderoptionPanel::plimitlevel     = PBVR_PARTICLE_LIMIT;

RenderoptionPanel::RenderoptionPanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::RenderoptionPanel)
{
    ui->setupUi(this);

    connect_T(ui->particleDensity,  QDoubleSpinBox, valueChanged,double, &RenderoptionPanel::onParticleDensityChanged);
    connect_T(ui->particleLimit,    QSpinBox,       valueChanged,   int, &RenderoptionPanel::onParticleLimitChanged);
    connect_T(ui->resolutionHeight, QSpinBox,       valueChanged,   int, &RenderoptionPanel::onReslutionHeightChanged);
    connect_T(ui->resolutionWidth,  QSpinBox,       valueChanged,   int, &RenderoptionPanel::onResolutionWidthChanged);
    connect_T(ui->dataSizeLimit,    QSpinBox,       valueChanged,   int, &RenderoptionPanel::onDataSizeLimitChanged);
    connect_T(ui->applyButton,      QPushButton,    clicked,       bool, &RenderoptionPanel::onApplyButtonClicked)

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
    this->ui->resolutionHeight->setValue(resolution_height_level);
    this->ui->resolutionWidth->setValue(resolution_width_level);
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
}

/**
 * @brief RenderoptionPanel::on_particleLimit_valueChanged
 * @param val
 */
void RenderoptionPanel::onParticleLimitChanged(int val)
{
    plimitlevel=val;
}

/**
 * @brief RenderoptionPanel::on_resolutionHeight_valueChanged
 * @param val
 */
void RenderoptionPanel::onReslutionHeightChanged(int val)
{
    resolution_height_level=val;
}

/**
 * @brief RenderoptionPanel::on_resolutionWidth_valueChanged
 * @param val
 */
void RenderoptionPanel::onResolutionWidthChanged(int val)
{
    resolution_width_level=val;
}

/**
 * @brief RenderoptionPanel::onDataSizeLimitChanged
 * @param val
 */
void RenderoptionPanel::onDataSizeLimitChanged(int val)
{
    data_size_limit=(float)val;
}


/**
 * @brief RenderoptionPanel::on_applyButton_clicked
 */
void RenderoptionPanel::onApplyButtonClicked()
{
    extCommand->CallBackApply(0);
}


