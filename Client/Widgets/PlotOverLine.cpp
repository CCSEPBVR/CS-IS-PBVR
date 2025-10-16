#include "PlotOverLine.h"
#include "ui_PlotOverLine.h"

PlotOverLine::PlotOverLine( QWidget *parent )
    : QDockWidget(parent)
    , ui(new Ui::PlotOverLine)
{
    initialize();
}

PlotOverLine::~PlotOverLine()
{
    delete ui;
}

void PlotOverLine::initialize()
{
    ui->setupUi(this);
}
