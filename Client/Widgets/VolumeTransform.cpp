#include "VolumeTransform.h"
#include "ui_VolumeTransform.h"

VolumeTransform::VolumeTransform( kvs::qt::jaea::Screen* screen, QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::VolumeTransform )
    , m_screen( screen )
{
    ui->setupUi( this );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &VolumeTransform::onApply );
}

VolumeTransform::~VolumeTransform()
{
    delete ui;
}

void VolumeTransform::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void VolumeTransform::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void VolumeTransform::onApply()
{
    m_screen->reset();
    emit updateTranslation();

    m_screen->scene()->objectManager()->translate( kvs::Vec3f(
        ui->translationXAxisDoubleSpinBox->value(),
        ui->translationYAxisDoubleSpinBox->value(),
        ui->translationZAxisDoubleSpinBox->value()
        ) );

    m_screen->scene()->objectManager()->scale( kvs::Vec3f(
        ui->scaleDoubleSpinBox->value(),
        ui->scaleDoubleSpinBox->value(),
        ui->scaleDoubleSpinBox->value() ) );

    m_screen->scene()->objectManager()->rotate( kvs::RPYRotationMatrix33<float>(
        ui->rotationZAxisDoubleSpinBox->value(),
        ui->rotationYAxisDoubleSpinBox->value(),
        ui->rotationXAxisDoubleSpinBox->value()
        ) );

    m_screen->update();
}
