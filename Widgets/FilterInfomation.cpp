#include "FilterInfomation.h"
#include "ui_FilterInfomation.h"

FilterInfomation::FilterInfomation(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::FilterInfomation)
{
    ui->setupUi(this);
}

FilterInfomation::~FilterInfomation()
{
    delete ui;
}
