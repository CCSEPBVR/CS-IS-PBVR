#include "RenderOptions.h"
#include "ui_RenderOptions.h"

RenderOptions::RenderOptions(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::RenderOptions)
{
    ui->setupUi(this);
}

RenderOptions::~RenderOptions()
{
    delete ui;
}
