#include "RenderOptions.h"
#include "ui_RenderOptions.h"

RenderOptions::RenderOptions(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::RenderOptions),
    m_client_message( nullptr )
{
    ui->setupUi(this);
    connect( ui->applyPBtn, &QPushButton::clicked, this, &RenderOptions::onApplyButtonClicked );
}

RenderOptions::~RenderOptions()
{
    delete ui;
}

void RenderOptions::onApplyButtonClicked()
{
    m_client_message->m_particle_density = ui->densityDSBox->value();
    m_client_message->m_particle_limit = ui->limitSBox->value();
    m_client_message->m_particle_data_size_limit = ui->dataSizeLimitSBox->value();
}
