#include "PlotOverLine.h"
#include "ui_PlotOverLine.h"
PlotOverLine::PlotOverLine(QWidget *parent, Connect* Connect)
    : QDockWidget(parent)
    , ui(new Ui::PlotOverLine)
    , m_connect( Connect )
{
    ui->setupUi(this);
    connect( ui->resolutionSpinBox, &QSpinBox::valueChanged, this, &PlotOverLine::onResolutionValueChanged );
    connect( ui->startPointXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onStartPointAxisValueChanged );
    connect( ui->startPointYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onStartPointAxisValueChanged );
    connect( ui->startPointZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onStartPointAxisValueChanged );
    connect( ui->endPointXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onEndPointAxisValueChanged );
    connect( ui->endPointYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onEndPointAxisValueChanged );
    connect( ui->endPointZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onEndPointAxisValueChanged );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &PlotOverLine::onApplyButtonClicked );
}
PlotOverLine::~PlotOverLine()
{
    delete ui;
}
void PlotOverLine::onResolutionValueChanged()
{
}
void PlotOverLine::onStartPointAxisValueChanged()
{
}
void PlotOverLine::onEndPointAxisValueChanged()
{
}
void PlotOverLine::onApplyButtonClicked()
{
}
