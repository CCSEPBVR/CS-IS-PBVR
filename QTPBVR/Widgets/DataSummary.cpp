#include "DataSummary.h"
#include "ui_DataSummary.h"

DataSummary::DataSummary(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataSummary)
{
    ui->setupUi(this);
}

DataSummary::~DataSummary()
{
    delete ui;
}

void DataSummary::setTotalParticles( int totalParticles )
{
    ui->totalParticlesLEdit->setText( QString::number( totalParticles ) );
}
