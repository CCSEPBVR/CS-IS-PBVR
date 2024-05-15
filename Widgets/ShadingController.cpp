#include "ShadingController.h"
#include "ui_ShadingController.h"

ShadingController::ShadingController(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ShadingController)
{
    ui->setupUi(this);
}

ShadingController::~ShadingController()
{
    delete ui;
}
