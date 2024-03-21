#include "ColorFunctionSelector.h"
#include "ui_ColorFunctionSelector.h"

ColorFunctionSelector::ColorFunctionSelector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorFunctionSelector)
{
    ui->setupUi(this);
}

ColorFunctionSelector::~ColorFunctionSelector()
{
    delete ui;
}
