#include "Coordinates.h"
#include "ui_Coordinates.h"

#include "FunctionParser/ExpressionTokenizer.h"
#include "FunctionParser/ExpressionConverter.h"

Coordinates::Coordinates(QWidget *parent,MergePanel *merge) :
    QDialog(parent),
    ui(new Ui::Coordinates),
    m_merge( merge ),
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
#ifdef Q_OS_WIN
    m_client_message->m_x_synthesis = ui->coordinateXLEdit->text().toLocal8Bit().constData();
    m_client_message->m_y_synthesis = ui->coordinateYLEdit->text().toLocal8Bit().constData();
    m_client_message->m_z_synthesis = ui->coordinateZLEdit->text().toLocal8Bit().constData();
#else
    m_client_message->m_x_synthesis = ui->coordinateXLEdit->text().toStdString();
    m_client_message->m_y_synthesis = ui->coordinateYLEdit->text().toStdString();
    m_client_message->m_z_synthesis = ui->coordinateZLEdit->text().toStdString();
#endif

    m_merge->setIsParticleGenerationNeeded( true );
}
