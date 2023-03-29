//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08
#include <GL/glew.h>
#include <QOpenGLContext>

#include "transformpanel.h"
#include "ui_transformpanel.h"

#include <QGlue/typedSignalConnect.h>
#include <QGlue/renderarea.h>

TransformParam TransformPanel::param;
TransformPanel* TransformPanel::instance = 0;

TransformPanel::TransformPanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::TransformPanel)
{
    if (instance!=0){
        qWarning("TransformPanel:: allows only one instance");
        delete instance;
        exit(1);
    }
    instance=this;
    ui->setupUi(this);
    connect_T(ui->xAxisTranslation,QDoubleSpinBox,valueChanged,double,
              &TransformPanel::onTranslationXchanged);
    connect_T(ui->yAxisTranslation,QDoubleSpinBox,valueChanged,double,
              &TransformPanel::onTranslationYchanged);// MODIFIED BY)T.Osaki 2019.12.18
    connect_T(ui->zAxisTranslation,QDoubleSpinBox,valueChanged,double,
              &TransformPanel::onTranslationZchanged);
    connect_T(ui->xAxisRotation,QDoubleSpinBox,valueChanged,double,
              &TransformPanel::onXaxisRotationChanged);
    connect_T(ui->yAxisRotation,QDoubleSpinBox,valueChanged,double,
              &TransformPanel::onYaxisRotationChanged);
    connect_T(ui->zAxisRotation,QDoubleSpinBox,valueChanged,double,
              &TransformPanel::onZaxisRotationChanged);
    connect_T(ui->uniformScale,QDoubleSpinBox,valueChanged,double,
              &TransformPanel::onUniformScaleChanged);
    connect(ui->applyButton,&QPushButton::clicked,
            this, &TransformPanel::onApplyButtonClicked);

}

TransformPanel::~TransformPanel()
{
    delete ui;
}

/**
 * @brief TransformPanel::setRotationAxisX
 * @param c
 */
void TransformPanel::setRotationAxisX( char* c )
{
    param.rotationAxisX =atof(c);
}
/**
 * @brief TransformPanel::setRotationAxisY
 * @param c
 */
void TransformPanel::setRotationAxisY( char* c )
{
    param.rotationAxisY =atof(c);
    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
    QApplication::postEvent(instance,updateEvent);
}
/**
 * @brief TransformPanel::setRotationAxisZ
 * @param c
 */
void TransformPanel::setRotationAxisZ( char* c )
{
    param.rotationAxisZ =atof(c);
    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
    QApplication::postEvent(instance,updateEvent);
}
/**
 * @brief TransformPanel::setTranslationX
 * @param c
 */
void TransformPanel::setTranslationX( char* c)
{
    param.translateX =atof(c);
    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
    QApplication::postEvent(instance,updateEvent);
}
/**
 * @brief TransformPanel::setTranslationY
 * @param c
 */
void TransformPanel::setTranslationY( char* c )
{
    param.translateY =atof(c);
    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
    QApplication::postEvent(instance,updateEvent);
}
/**
 * @brief TransformPanel::setTranslationZ
 * @param c
 */
void TransformPanel::setTranslationZ( char* c)
{
    param.translateZ =atof(c);
    QEvent* updateEvent= new QEvent((QEvent::Type)QEvent::User);
    QApplication::postEvent(instance,updateEvent);
}
/**
 * @brief TransformPanel::setScaling
 * @param c
 */
void TransformPanel::setScaling( char* c )
{
    param.scalingLevel =atof(c);
}

/**
 * @brief TransformPanel::onXaxisTranslationChanged
 * @param val
 */
void TransformPanel::onTranslationXchanged(double val)
{
    param.translateX=val;
}

/**
 * @brief TransformPanel::onYaxisTranslationChanged
 * @param val
 */
void TransformPanel::onTranslationYchanged(double val)
{
    param.translateY=val;
}
/**
 * @brief TransformPanel::onZaxisTranslationChanged
 * @param val
 */
void TransformPanel::onTranslationZchanged(double val)
{
    param.translateZ=val;
}
/**
 * @brief TransformPanel::onXaxisRotationChanged
 * @param val
 */
void TransformPanel::onXaxisRotationChanged(double val)
{
    // MODIFIED BY)T.Osaki 2019.12.18
    param.rotationAxisX=val;
}
/**
 * @brief TransformPanel::onYaxisRotationChanged
 * @param val
 */
void TransformPanel::onYaxisRotationChanged(double val)
{
    // MODIFIED BY)T.Osaki 2019.12.18
    param.rotationAxisY=val;
}
/**
 * @brief TransformPanel::onZaxisRotationChanged
 * @param val
 */
void TransformPanel::onZaxisRotationChanged(double val)
{
    // MODIFIED BY)T.Osaki 2019.12.18
    param.rotationAxisZ=val;
}
/**
 * @brief TransformPanel::onUniformScaleChanged
 * @param val
 */
void TransformPanel::onUniformScaleChanged(double val)
{
    param.scalingLevel=val;
}
/**
 * @brief TransformPanel::onApplyButtonClicked
 */
void TransformPanel::onApplyButtonClicked()
{
//    // 20210106 yodo append
//    std::cout << "TransformPanel::onApplyButtonClicked() calls extCommand->CAllBackApply()" << std::endl;


    extCommand->CallBackApply(0);
    extCommand->m_screen->update();
}

/**
 * @brief TransformPanel::customEvent
 *        Process filter info update events
 * @param event
 */
void TransformPanel::customEvent(QEvent * event)
{
    ui->xAxisRotation->setValue(param.rotationAxisX);
    ui->yAxisRotation->setValue(param.rotationAxisY);
    ui->zAxisRotation->setValue(param.rotationAxisZ);
    ui->xAxisTranslation->setValue(param.translateX);
    ui->xAxisTranslation->setValue(param.translateX);
    ui->yAxisTranslation->setValue(param.translateY);
    ui->zAxisTranslation->setValue(param.translateZ);
    ui->uniformScale->setValue(param.scalingLevel);
}
