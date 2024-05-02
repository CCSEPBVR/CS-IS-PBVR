#include "Coordinates.h"
#include "ui_Coordinates.h"

#include "FunctionParser/ExpressionTokenizer.h"
#include "FunctionParser/ExpressionConverter.h"

Coordinates::Coordinates(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Coordinates),
    m_client_message( nullptr )
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
    m_client_message->m_x_synthesis = ui->coordinateXLEdit->text().toStdString();
    m_client_message->m_y_synthesis = ui->coordinateYLEdit->text().toStdString();
    m_client_message->m_z_synthesis = ui->coordinateZLEdit->text().toStdString();
    m_merge->setIsParticleGenerationNeeded( true );
}
