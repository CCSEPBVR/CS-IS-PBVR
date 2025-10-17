#include "PointSizeControl.h"
#include "ui_PointSizeControl.h"

PointSizeControl::PointSizeControl( QWidget *parent )
    : QDockWidget(parent)
    , ui(new Ui::PointSizeControl)
{
    initialize();
}

PointSizeControl::~PointSizeControl()
{
    delete ui;
}

void PointSizeControl::initialize()
{
    ui->setupUi(this);
}
