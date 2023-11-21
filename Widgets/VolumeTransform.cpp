#include "VolumeTransform.h"
#include "ui_VolumeTransform.h"

#include <kvs/ObjectManager>

VolumeTransform::VolumeTransform(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::VolumeTransform)
{
    ui->setupUi(this);
    
    connect( ui->applyPBtn, &QPushButton::clicked, this, &VolumeTransform::onApplyButtonClicked );
}

VolumeTransform::~VolumeTransform()
{
    delete ui;
}

void VolumeTransform::onApplyButtonClicked()
{
    m_screen->reset();
    kvs::Vector3f translate(
        ui->translationXAxisDSBox->value(),
        ui->translationYAxisDSBox->value(),
        ui->translationZAxisDSBox->value() );
    m_screen->scene()->objectManager()->translate( translate );

    kvs::Vector3f scaling(
        ui->scaleDSBox->value(),
        ui->scaleDSBox->value(),
        ui->scaleDSBox->value() );
    m_screen->scene()->objectManager()->scale( scaling );

    m_screen->scene()->objectManager()->rotate( kvs::RPYRotationMatrix33<float>(
        ui->rotationXAxisDSBox->value(),
        ui->rotationYAxisDSBox->value(),
        ui->rotationZAxisDSBox->value()) );

    m_screen->update();
}
