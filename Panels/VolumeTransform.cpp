#include "VolumeTransform.h"
#include "ui_VolumeTransform.h"

#include <kvs/ObjectManager>

VolumeTransform::VolumeTransform(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::VolumeTransform)
{
    ui->setupUi(this);

    connect( ui->applyBtn, &QPushButton::clicked, this, &VolumeTransform::onApplyButtonClicked );
}

VolumeTransform::~VolumeTransform()
{
    delete ui;
}

void VolumeTransform::onApplyButtonClicked()
{
    m_screen->reset();
    kvs::Vector3f translate(
                ui->translationXAxisSB->value(),
                ui->translationYAxisSB->value(),
                ui->translationZAxisSB->value()
                );
    m_screen->scene()->objectManager()->translate( translate );

    kvs::Vector3f scaling(
                ui->scaleSB->value(),
                ui->scaleSB->value(),
                ui->scaleSB->value()
                );
    m_screen->scene()->objectManager()->scale( scaling );

    m_screen->scene()->objectManager()->rotate( kvs::RPYRotationMatrix33<float>(
                                                    ui->rotationXAxisSB->value(),
                                                    ui->rotationYAxisSB->value(),
                                                    ui->rotationZAxisSB->value()
                                                    ) );
    m_screen->update();
}
