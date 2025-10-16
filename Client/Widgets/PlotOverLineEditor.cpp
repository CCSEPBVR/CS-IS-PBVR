#include "PlotOverLineEditor.h"
#include "ui_PlotOverLineEditor.h"

PlotOverLineEditor::PlotOverLineEditor( QWidget *parent )
    : QDockWidget(parent)
    , ui(new Ui::PlotOverLineEditor)
    , m_q_custom_plot( new QCustomPlot( this ) )
{
    initialize();
}

PlotOverLineEditor::~PlotOverLineEditor()
{
    if( m_q_custom_plot ) m_q_custom_plot->setParent(nullptr);  // ui 破棄前に親を外す
    delete ui;
}

void PlotOverLineEditor::initialize()
{
    ui->setupUi(this);
    ui->customPlotArea->addWidget( m_q_custom_plot );
}
