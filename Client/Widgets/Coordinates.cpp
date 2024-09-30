#include "Coordinates.h"
#include "ui_Coordinates.h"

Coordinates::Coordinates(QWidget *parent, MergePanel* merge, Connect* connect_panel) :
    QDialog(parent),
    ui(new Ui::Coordinates),
    m_merge( merge ),
    m_connect( connect_panel )
{
    ui->setupUi(this);
    connect( ui->applyPBtn, &QPushButton::clicked, this, &Coordinates::onApplyButtonClicked );
}

Coordinates::~Coordinates()
{
    delete ui;
}

void Coordinates::onApplyButtonClicked()
{
#ifdef Q_OS_WIN
    m_connect->getClientMessage()->m_x_synthesis = ui->coordinateXLEdit->text().toLocal8Bit().constData();
    m_connect->getClientMessage()->m_y_synthesis = ui->coordinateYLEdit->text().toLocal8Bit().constData();
    m_connect->getClientMessage()->m_z_synthesis = ui->coordinateZLEdit->text().toLocal8Bit().constData();
#else
    m_connect->getClientMessage()->m_x_synthesis = ui->coordinateXLEdit->text().toStdString();
    m_connect->getClientMessage()->m_y_synthesis = ui->coordinateYLEdit->text().toStdString();
    m_connect->getClientMessage()->m_z_synthesis = ui->coordinateZLEdit->text().toStdString();
#endif
    m_merge->setIsParticleGenerationNeeded( true );
}
