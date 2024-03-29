﻿#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08
#include <QOpenGLContext>

#include "coordinatepanel.h"
#include "ui_coordinatepanel.h"
#include "QGlue/renderarea.h"
#include "Client/LogManager.h"

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
#ifdef COMM_MODE_CS
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

    if(
            (param_x_synthesis.empty() || param_x_synthesis == "X" || param_x_synthesis == "x") &&
            (param_y_synthesis.empty() || param_y_synthesis == "Y" || param_y_synthesis == "y") &&
            (param_z_synthesis.empty() || param_z_synthesis == "Z" || param_x_synthesis == "z"))
    {
        m_use_particle_side_coords = false;
    }else{
        m_use_particle_side_coords = true;
    }

    m_coordinate = true;
    m_coordinate_for_store_particle = true;
    extCommand->m_screen->update();

    if( m_isRec )
    {
        LogManager lg;
        lg.Write("coordinatepanel.cpp",__func__,"XYZCoordinateExpression",param_x_synthesis + "," + param_y_synthesis + "," + param_z_synthesis);
    }
}

void CoordinatePanel::onResetScaleButton()
{
    this->reScale();
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("coordinatepanel.cpp",__func__,"rescaleButton","clicked");
    }
}

void CoordinatePanel::logPlay(std::string funcName,std::string str[])
{
    if(funcName.compare("onApplyButton") == 0)
    {
        ui->xCoordinateExpression->setText(QString::fromStdString(str[0]));
        ui->yCoordinateExpression->setText(QString::fromStdString(str[1]));
        ui->zCoordinateExpression->setText(QString::fromStdString(str[2]));
        this->onApplyButton();
    }

    if(funcName.compare("onResetScaleButton") == 0)
    {
        this->onResetScaleButton();
    }
}
