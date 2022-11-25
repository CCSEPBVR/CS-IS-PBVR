#include "animationcontrols.h"
#include "ui_animationcontrols.h"
#include "Client/v3defines.h"
#include "QGlue/typedSignalConnect.h"

#include <iostream>
char buff_workdir[100]=".";
extern std::string ValueKeyFrameFile;

AnimationControls* AnimationControls::instance = 0;
AnimationControlParameter animation_control_parameter;

AnimationControls::AnimationControls(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::AnimationControls)
{
    if (instance!=0){
        qWarning("TimecontrolPanel:: allows only one instance");
        delete instance;
    }
    instance=this;
    ui->setupUi(this);
    ui->animationFile->setText(ValueKeyFrameFile.c_str());
    ui->imageFile->setText("PBVR_image");

    // Connect event handlers
    connect_T(ui->captureEnable,QComboBox,currentIndexChanged, int,
              &AnimationControls::onEnableCaptureChanged);
    connect(ui->imageFile,&QLineEdit::textEdited,
            this, &AnimationControls::onImageFileChanged);
    connect(ui->animationFile,&QLineEdit::textEdited,
            this, &AnimationControls::onAnimationFileChange);
    connect_T(ui->interpolation,QSpinBox,valueChanged, int,
              &AnimationControls::onInterpolationValueChanged);

}

AnimationControls::~AnimationControls()
{
    delete ui;
}

/**
 * @brief AnimationControls::getInterpolationValue
 * @return the current interpolation value
 */
int AnimationControls::getInterpolationValue()
{
    if (!instance) return 0;
    return  instance->ui->interpolation->text().toInt();
}
/**
 * @brief AnimationControls::getParameters
 * @return  the animation control parameter
 */
AnimationControlParameter AnimationControls::getParameters()
{
    return animation_control_parameter;
}

/**
 * @brief AnimationControls::getImageFileName
 * @param step
 * @param ext
 * @return  the image filename for a given step number and file extension.
 */
std::string AnimationControls::getImageFileName(std::string step, std::string ext)
{
    std::string basename="PBVR_Image";
    if (instance)
        basename=instance->ui->imageFile->text().toStdString();

    return basename + step +ext;
}

/**
 * @brief AnimationControls::RedrawAnimationControl
 *          static method for redrawing animation control panel
 * @param ValueNumKeyFrames
 */
void AnimationControls::RedrawAnimationControl(int ValueNumKeyFrames)
{
    qInfo(">>>>AnimationControls::RedrawAnimationControl()");
    if (!instance) return;
    Ui::AnimationControls* ui=instance->ui;

    if ( ValueNumKeyFrames <= 0 )
    {
        ui->totalKeyFrames->setText(QString::number(ValueNumKeyFrames));
        ui->totalAnimationFrames->setText(QString::number(0));
    }
    else
    {
        int ValueInterpolation = ui->interpolation->text().toInt();
        int frames=( ValueNumKeyFrames - 1 )*ValueInterpolation ;
        ui->totalKeyFrames->setText(QString::number(ValueNumKeyFrames));
        ui->totalAnimationFrames->setText(QString::number(frames));
    }
}

/**
 * @brief AnimationControls::onEnableCaptureChanged
 *        Event handler for enableCapture control.
 * @param index
 */
void AnimationControls::onEnableCaptureChanged(int index)
{
    bool capture= index==0?PBVR_OFF:PBVR_ON;
    animation_control_parameter.m_capture = capture;
    //m_capture = ONなら、LOAD ボタンを無効に
    if ( animation_control_parameter.m_capture == capture )
        animation_control_parameter.m_load = false;
}

/**
 * @brief AnimationControls::onImageFileChanged
 *        Event handler for image file selection control
 * @param arg1
 */
void AnimationControls::onImageFileChanged(const QString &arg1)
{
    animation_control_parameter.m_image_file  = std::string( buff_workdir );
    animation_control_parameter.m_image_file += '/';
    animation_control_parameter.m_image_file += arg1.toStdString();
}

/**
 * @brief AnimationControls::onAnimationFileChange
 *        Event handler for animation file selection control
 * @param arg1
 */
void AnimationControls::onAnimationFileChange(const QString &arg1)
{
    ValueKeyFrameFile=arg1.toStdString();
}

/**
 * @brief AnimationControls::onInterpolationValueChanged
 *        Event handler for the interpolationValue control
 * @param arg1
 */
void AnimationControls::onInterpolationValueChanged(int arg1)
{
    int ValueNumKeyFrames = ui->totalKeyFrames->text().toInt();
    RedrawAnimationControl(ValueNumKeyFrames);
}
