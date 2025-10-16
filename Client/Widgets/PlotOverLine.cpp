#include "PlotOverLine.h"
#include "ui_PlotOverLine.h"

PlotOverLine::PlotOverLine( QWidget *parent )
    : QDockWidget(parent)
    , ui(new Ui::PlotOverLine)
    , m_q_custom_plot( new QCustomPlot( this ) )
{
    initialize();
}

PlotOverLine::~PlotOverLine()
{
    if( m_q_custom_plot ) m_q_custom_plot->setParent(nullptr);  // ui 破棄前に親を外す
    delete ui;
}

void PlotOverLine::initialize()
{
    ui->setupUi(this);
    ui->customPlotArea->addWidget( m_q_custom_plot );
}
