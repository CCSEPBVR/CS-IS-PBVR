//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08
#include <GL/glew.h>
#include <QOpenGLContext>

#include "coordinatepanel.h"
#include "ui_coordinatepanel.h"
#include "QGlue/renderarea.h"

std::string CoordinatePanel::param_x_synthesis="";
std::string CoordinatePanel::param_y_synthesis="";
std::string CoordinatePanel::param_z_synthesis="";

CoordinatePanel::CoordinatePanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::CoordinatePanel)
{
    ui->setupUi(this);
    this->hide();
    connect (ui->apply_btn,&QPushButton::clicked,this,&CoordinatePanel::onApplyButton);
    connect (ui->reScale_btn,&QPushButton::clicked,this,&CoordinatePanel::onResetScaleButton);

}

CoordinatePanel::~CoordinatePanel()
{
    delete ui;
}

/**
 * @brief CoordinatePanel::setParam update  param from UI
 * @param param
 */
void CoordinatePanel::setParam( kvs::visclient::VisualizationParameter* param )
{
#ifdef CS_MODE
    param->m_x_synthesis = std::string( param_x_synthesis );
    param->m_y_synthesis = std::string( param_y_synthesis );
    param->m_z_synthesis = std::string( param_z_synthesis );
#endif
}

/**
 * @brief CoordinatePanel::resetChangeFlagForStoreParticle
 */
void CoordinatePanel::resetChangeFlagForStoreParticle()
{
    m_coordinate_for_store_particle = false;
}
/**
 * @brief CoordinatePanel::getChangeCoordFlag
 * @return
 */
bool CoordinatePanel::getChangeCoordFlag()
{
     return m_coordinate;
}
/**
 * @brief CoordinatePanel::getChangeCoordFlagForStoreParticle
 * @return
 */
bool CoordinatePanel::getChangeCoordFlagForStoreParticle(){
    return m_coordinate_for_store_particle;
}
/**
 * @brief CoordinatePanel::resetResetViewFlag
 */
void CoordinatePanel::resetResetViewFlag()
{
    m_reset_flag = false;
    m_coordinate = false;
}
/**
 * @brief CoordinatePanel::getResetViewFlag
 * @return
 */
bool CoordinatePanel::getResetViewFlag()
{
    return m_reset_flag;
}

/**
 * @brief CoordinatePanel::reScale trigger reset on rescale
 */
void CoordinatePanel::reScale()
{
    m_reset_flag = true;
}

//bool CoordinatePanel::getShowUI()
//{
//    return this->isVisible();
//}

//void CoordinatePanel::changeShowUI()
//{
//    this->setVisible(!this->isVisible());
//}
/**
 * @brief CoordinatePanel::setXCoordinateExpression   X coordinate expression setter
 */
void CoordinatePanel::setXCoordinateExpression()
{
    // 必要であれば式のエラーチェックをここで行う
    m_coordinate = true;
    m_coordinate_for_store_particle = true;
    param_x_synthesis =  this->ui->xCoordinateExpression->text().toStdString();
}
/**
 * @brief CoordinatePanel::setYCoordinateExpression   Y coordinate expression setter
 */
void CoordinatePanel::setYCoordinateExpression()
{
    // 必要であれば式のエラーチェックをここで行う
    m_coordinate = true;
    m_coordinate_for_store_particle = true;
    param_y_synthesis = this->ui->yCoordinateExpression->text().toStdString();
}

/**
 * @brief CoordinatePanel::setZCoordinateExpression Z coordinate expression setter
 */
void CoordinatePanel::setZCoordinateExpression()
{
    // 必要であれば式のエラーチェックをここで行う
    m_coordinate = true;
    m_coordinate_for_store_particle = true;
    param_z_synthesis =  this->ui->zCoordinateExpression->text().toStdString();
}

/**
 * @brief CoordinatePanel::setUISynthesizer, Set the UI Synthesizer strings
 */
void CoordinatePanel::setUISynthesizer()
{
    if ( !param_x_synthesis.empty() )
    {
        // 読み込んだパラメータと現在のパラメータが違う場合だけ設定
        if ( ! ( this->ui->xCoordinateExpression->text().toStdString() == param_x_synthesis ) )
        {

            ui->xCoordinateExpression->setText(QString::fromStdString(param_x_synthesis));
            m_coordinate = true;
        }
    }
    if ( !param_y_synthesis.empty() )
    {
        // 読み込んだパラメータと現在のパラメータが違う場合だけ設定
        if ( ! ( this->ui->yCoordinateExpression->text().toStdString() == param_y_synthesis ) )
        {

            ui->yCoordinateExpression->setText(QString::fromStdString(param_y_synthesis));
            m_coordinate = true;
        }
    }
    if ( !param_z_synthesis.empty() )
    {
        // 読み込んだパラメータと現在のパラメータが違う場合だけ設定
        if ( ! ( this->ui->zCoordinateExpression->text().toStdString() == param_z_synthesis ) )
        {

            ui->zCoordinateExpression->setText(QString::fromStdString(param_z_synthesis));
            m_coordinate = true;
        }
    }
}

/**
 * @brief CoordinatePanel::onApplyButton event handler
 */
void CoordinatePanel::onApplyButton()
{
    this->setXCoordinateExpression();
    this->setYCoordinateExpression();
    this->setZCoordinateExpression();
    m_coordinate = true;
    m_coordinate_for_store_particle = true;
    extCommand->m_screen->update();
}

void CoordinatePanel::onResetScaleButton()
{
    this->reScale();
}
