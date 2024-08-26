#include "DataProperties.h"
#include "ui_DataProperties.h"

DataProperties::DataProperties(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DataProperties)
{
    ui->setupUi(this);
}

DataProperties::~DataProperties()
{
    delete ui;
}

void DataProperties::updateFilterInfomation( QString volumeDataFilePath, jpv::ParticleTransferServerMessage& reply )
{
    ui->filterParameterFileLEdit->setText( volumeDataFilePath );

    ui->vectorNumberLEdit->setText( QString::number( reply.m_number_ingredients ) );
    ui->allElementsLEdit->setText( QString::number( reply.m_number_elements ));
    ui->subVolumeLEdit->setText( QString::number( reply.m_number_volume_divide ) );
    ui->elementTypeLEdit->setText( QString::number( reply.m_element_type ) );
    ui->allNodesLEdit->setText( QString::number( reply.m_number_nodes ) );
    ui->stepNumberLEdit->setText( QString::number( reply.m_number_step ));
    ui->fileTypeLEdit->setText( QString::number(reply.m_file_type) );

    ui->xMinLEdit->setText( QString::number( reply.m_min_object_coord[0] ));
    ui->xMaxLEdit->setText( QString::number( reply.m_max_object_coord[0] ));
    ui->yMinLEdit->setText( QString::number( reply.m_min_object_coord[1] ));
    ui->yMaxLEdit->setText( QString::number( reply.m_max_object_coord[1] ));
    ui->zMinLEdit->setText( QString::number( reply.m_min_object_coord[2] ));
    ui->zMaxLEdit->setText( QString::number( reply.m_max_object_coord[2] ));
}
