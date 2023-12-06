#include "TimeControl.h"
#include "ui_TimeControl.h"

TimeControl::TimeControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeControl)
{
    ui->setupUi(this);
}

TimeControl::~TimeControl()
{
    delete ui;
}
