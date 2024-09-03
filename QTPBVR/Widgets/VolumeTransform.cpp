#include "VolumeTransform.h"
#include "ui_VolumeTransform.h"
#include "App/pbvrgui.h"
#include <kvs/ObjectManager>

VolumeTransform::VolumeTransform(QWidget *parent, PBVRGUI *pbvr_gui) :
    QDockWidget(parent),
    ui(new Ui::VolumeTransform),
    m_pbvr_gui( pbvr_gui )
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
    m_pbvr_gui->screen()->reset();
    kvs::Vector3f translate(
        ui->translationXAxisDSBox->value(),
        ui->translationYAxisDSBox->value(),
        ui->translationZAxisDSBox->value() );
    m_pbvr_gui->screen()->scene()->objectManager()->translate( translate );

    kvs::Vector3f scaling(
        ui->scaleDSBox->value(),
        ui->scaleDSBox->value(),
        ui->scaleDSBox->value() );
    m_pbvr_gui->screen()->scene()->objectManager()->scale( scaling );

    m_pbvr_gui->screen()->scene()->objectManager()->rotate( kvs::RPYRotationMatrix33<float>(
        ui->rotationXAxisDSBox->value(),
        ui->rotationYAxisDSBox->value(),
        ui->rotationZAxisDSBox->value()) );

    m_pbvr_gui->screen()->update();
}
