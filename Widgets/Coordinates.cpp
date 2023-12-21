#include "Coordinates.h"
#include "ui_Coordinates.h"

Coordinates::Coordinates(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::Coordinates)
{
    ui->setupUi(this);
}

Coordinates::~Coordinates()
{
    delete ui;
}
