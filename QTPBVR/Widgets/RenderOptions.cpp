#include "RenderOptions.h"
#include "ui_RenderOptions.h"

RenderOptions::RenderOptions(QWidget *parent, MergePanel* merge, Connect* connect_panel) :
    QDockWidget(parent),
    ui(new Ui::RenderOptions),
    m_merge( merge ),
    m_connect( connect_panel )
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
    m_connect->getClientMessage()->m_particle_density = ui->densityDSBox->value();
    m_connect->getClientMessage()->m_particle_limit = ui->limitSBox->value();
    m_connect->getClientMessage()->m_particle_data_size_limit = ui->dataSizeLimitSBox->value();
    m_merge->setIsParticleGenerationNeeded( true );
}
