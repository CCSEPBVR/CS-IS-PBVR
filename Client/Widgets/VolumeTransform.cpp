#include "VolumeTransform.h"
#include "ui_VolumeTransform.h"

VolumeTransform::VolumeTransform( kvs::qt::jaea::Screen* screen, QWidget *parent )
    : QDockWidget(parent)
    , ui(new Ui::VolumeTransform)
    , m_screen( screen )
{    
    initialize();
}

VolumeTransform::~VolumeTransform()
{
    delete ui;
}

void VolumeTransform::initialize()
{
    ui->setupUi(this);
    connect( ui->applyPushButton, &QPushButton::clicked, this, &VolumeTransform::onApply );
}

void VolumeTransform::onApply()
{
    m_screen->reset();
    emit updatePointsTranslation();

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

void VolumeTransform::loadParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void VolumeTransform::saveParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}
