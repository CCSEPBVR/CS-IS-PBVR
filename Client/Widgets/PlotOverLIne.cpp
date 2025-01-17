#include "PlotOverLine.h"
#include "ui_PlotOverLine.h"

#include "App/pbvrgui.h"
#include <kvs/StochasticLineRenderer>

PlotOverLine::PlotOverLine(QWidget *parent, PBVRGUI *pbvr_gui, Connect* Connect)
    : QDockWidget(parent)
    , ui(new Ui::PlotOverLine)
    , m_pbvr_gui( pbvr_gui )
    , m_connect( Connect ),
    m_vector_list( new QStringList() )
{
    ui->setupUi(this);

    connect( ui->resetPushButton, &QPushButton::clicked, this, &PlotOverLine::onResetButtonClicked );
    connect( ui->plotGroupBox, &QGroupBox::clicked, this, &PlotOverLine::updateXYZDoubleSpinBox );
    connect( ui->startPointXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::updateXYZDoubleSpinBox );
    connect( ui->startPointYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::updateXYZDoubleSpinBox );
    connect( ui->startPointZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::updateXYZDoubleSpinBox );
    connect( ui->endPointXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::updateXYZDoubleSpinBox );
    connect( ui->endPointYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::updateXYZDoubleSpinBox );
    connect( ui->endPointZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::updateXYZDoubleSpinBox );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &PlotOverLine::onApplyButtonClicked );

    initData(); //for debug
    // setPlotData( m_x_axis, m_mask, m_line_value ); //for debug
}

PlotOverLine::~PlotOverLine()
{
    delete ui;
}

void PlotOverLine::updateNumberOfVector( jpv::ParticleTransferServerMessage& server_message )
{
    const int numberOfVector = server_message.m_number_ingredients;

    m_vector_list->clear();
    for( int i = 1; i <= numberOfVector; i++ )
    {
        m_vector_list->append(QString("q%1").arg(i));
    }
    ui->comboBox->addItems( *m_vector_list );
    ui->comboBox->setCurrentIndex( 0 );
}

void PlotOverLine::onResetButtonClicked()
{
    ui->customPlot->xAxis->setRange(m_x_min, m_x_max); // x 軸の範囲
    ui->customPlot->yAxis->setRange(m_y_min, m_y_max); // y 軸の範囲
    ui->customPlot->replot();
}

void PlotOverLine::updateXYZDoubleSpinBox()
{
    kvs::ValueArray<kvs::Real32> pointer_coords =
        {
            static_cast<kvs::Real32>(ui->startPointXDoubleSpinBox->value()), static_cast<kvs::Real32>(ui->startPointYDoubleSpinBox->value()), static_cast<kvs::Real32>(ui->startPointZDoubleSpinBox->value()),
            static_cast<kvs::Real32>(ui->endPointXDoubleSpinBox->value()), static_cast<kvs::Real32>(ui->endPointYDoubleSpinBox->value()), static_cast<kvs::Real32>(ui->endPointZDoubleSpinBox->value())
        };

    kvs::Xform before_object_manager_xform = m_pbvr_gui->screen()->scene()->objectManager()->xform();
    if( m_pointer == nullptr )
    {
        m_pointer = new kvs::LineObject( pointer_coords );
        m_pointer->setXform( before_object_manager_xform );
        m_pointer->setLineTypeToStrip();
        m_pointer->setColorTypeToLine();
        m_pointer->setColor( kvs::RGBColor::Green() );
        m_pointer->setSize( 2.0f );
        m_pointer->updateMinMaxCoords();
        kvs::StochasticLineRenderer* renderer = new kvs::StochasticLineRenderer();
        m_pointer_id = m_pbvr_gui->screen()->registerObject( m_pointer, renderer );
    }
    else
    {
        m_pointer = new kvs::LineObject( pointer_coords );
        m_pointer->setXform( before_object_manager_xform );
        m_pointer->setLineTypeToStrip();
        m_pointer->setColorTypeToLine();
        m_pointer->setColor( kvs::RGBColor::Green() );
        m_pointer->setSize( 2.0f );
        m_pointer->updateMinMaxCoords();
        m_pbvr_gui->screen()->scene()->replaceObject( m_pointer_id.first, m_pointer );

        if( m_pointer->isVisible() )
        {
            if ( ui->plotGroupBox->isChecked() )
            {
                m_pointer->show();
            }
            else
            {
                m_pointer->hide();
            }
        }
    }
    m_pbvr_gui->screen()->update();
}

void PlotOverLine::onApplyButtonClicked()
{
    int32_t resolution = static_cast<int32_t>( ui->resolutionSpinBox->value() );

    float plotStartPoint[3] =
        {
            static_cast<float>( ui->startPointXDoubleSpinBox->value() ),
            static_cast<float>( ui->startPointYDoubleSpinBox->value() ),
            static_cast<float>( ui->startPointZDoubleSpinBox->value() )
        };

    float plotEndPoint[3] =
        {
            static_cast<float>( ui->endPointXDoubleSpinBox->value() ),
            static_cast<float>( ui->endPointYDoubleSpinBox->value() ),
            static_cast<float>( ui->endPointZDoubleSpinBox->value() )
        };

    m_connect->getClientMessage()->m_sampling_size = resolution;
    m_connect->getClientMessage()->m_start_point[0] = plotStartPoint[0];
    m_connect->getClientMessage()->m_start_point[1] = plotStartPoint[1];
    m_connect->getClientMessage()->m_start_point[2] = plotStartPoint[2];
    m_connect->getClientMessage()->m_end_point[0] = plotEndPoint[0];
    m_connect->getClientMessage()->m_end_point[1] = plotEndPoint[1];
    m_connect->getClientMessage()->m_end_point[2] = plotEndPoint[2];
    m_connect->getClientMessage()->m_plot_variable = ui->comboBox->currentText().toStdString();
}

void PlotOverLine::setPlotData(std::vector<float> xAxis, std::vector<bool> mask, std::vector<float> values)
{
    // メインスレッドで実行する必要がある場合
    if (QApplication::instance()->thread() != QThread::currentThread())
    {
        QMetaObject::invokeMethod(this, [=]() { setPlotData(xAxis, mask, values); }, Qt::QueuedConnection);
        return;
    }


    // 初期化：極端に大きい/小さい値を設定
    m_x_min = std::numeric_limits<double>::max();
    m_x_max = std::numeric_limits<double>::lowest();
    m_y_min = std::numeric_limits<double>::max();
    m_y_max = std::numeric_limits<double>::lowest();

    QVector<double> x(xAxis.size()), y(values.size());
    for (size_t i = 0; i < x.size(); i++)
    {
        x[i] = xAxis[i];
        if (mask[i])
        {
            if (x[i] < m_x_min)
            {
                m_x_min = x[i];
            }
            if (x[i] > m_x_max)
            {
                m_x_max = x[i];
            }
        }
    }
    for (size_t i = 0; i < x.size(); i++)
    {
        if (mask[i])
        {
            y[i] = values[i];
            if (y[i] < m_y_min)
            {
                m_y_min = y[i];
            }
            if (y[i] > m_y_max)
            {
                m_y_max = y[i];
            }
        }
        else
        {
            y[i] = std::numeric_limits<double>::quiet_NaN();
        }
    }

    // グラフにデータを追加
    ui->customPlot->addGraph(); // 新しいグラフを追加
    ui->customPlot->graph(0)->setData(x, y); // データを設定

    // 軸ラベルを設定
    ui->customPlot->xAxis->setLabel("xAxis");
    ui->customPlot->yAxis->setLabel("Values");

    // 軸の範囲を設定
    ui->customPlot->xAxis->setRange(m_x_min, m_x_max); // x軸範囲
    ui->customPlot->yAxis->setRange(m_y_min, m_y_max); // y軸範囲
    ui->customPlot->xAxis->ticker()->setTickCount(10);
    ui->customPlot->yAxis->ticker()->setTickCount(10);

    // ズームとドラッグを有効化
    ui->customPlot->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    // グラフを再描画
    ui->customPlot->replot();
}


void PlotOverLine::initData() //for debug
{

    m_connect->getClientMessage()->m_sampling_size = 256;
    m_connect->getClientMessage()->m_start_point[0] = -1.f;
    m_connect->getClientMessage()->m_start_point[1] = 0;
    m_connect->getClientMessage()->m_start_point[2] = 0;
    m_connect->getClientMessage()->m_end_point[0] = 1.f;
    m_connect->getClientMessage()->m_end_point[1] = 0;
    m_connect->getClientMessage()->m_end_point[2] = 0;
    m_connect->getClientMessage()->m_plot_variable = "q1";

}

// void PlotOverLine::initData() //for debug
// {
//     m_x_axis.clear();
//     m_mask.clear();
//     m_line_value.clear();

//     m_x_axis.push_back( 0 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.0254902 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.0509804 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.0764706 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.101961 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.127451 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.152941 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.178431 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.203922 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.229412 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.254902 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.280392 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.305882 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.331373 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.356863 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.382353 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 0.407843 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.433333 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.458824 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.484314 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.509804 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.535294 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.560784 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.586275 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.611765 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.637255 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.662745 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.688236 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.713726 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.739216 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.764706 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.790196 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.815687 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.841177 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.866667 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.892157 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.917648 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.943138 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.968628 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 0.994118 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.4 );
//     m_x_axis.push_back( 1.01961 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.387356 );
//     m_x_axis.push_back( 1.0451 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.374418 );
//     m_x_axis.push_back( 1.07059 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.361479 );
//     m_x_axis.push_back( 1.09608 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.348541 );
//     m_x_axis.push_back( 1.12157 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.336495 );
//     m_x_axis.push_back( 1.14706 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.330428 );
//     m_x_axis.push_back( 1.17255 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.324361 );
//     m_x_axis.push_back( 1.19804 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.318294 );
//     m_x_axis.push_back( 1.22353 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.312226 );
//     m_x_axis.push_back( 1.24902 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.306159 );
//     m_x_axis.push_back( 1.27451 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.300092 );
//     m_x_axis.push_back( 1.3 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.294025 );
//     m_x_axis.push_back( 1.32549 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.281975 );
//     m_x_axis.push_back( 1.35098 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.272457 );
//     m_x_axis.push_back( 1.37647 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.285732 );
//     m_x_axis.push_back( 1.40196 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.299378 );
//     m_x_axis.push_back( 1.42745 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.313023 );
//     m_x_axis.push_back( 1.45294 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.326668 );
//     m_x_axis.push_back( 1.47843 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.340313 );
//     m_x_axis.push_back( 1.50392 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.353959 );
//     m_x_axis.push_back( 1.52941 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.367604 );
//     m_x_axis.push_back( 1.5549 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.381163 );
//     m_x_axis.push_back( 1.58039 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.39456 );
//     m_x_axis.push_back( 1.60588 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.407956 );
//     m_x_axis.push_back( 1.63137 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.421352 );
//     m_x_axis.push_back( 1.65686 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.434749 );
//     m_x_axis.push_back( 1.68235 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.444516 );
//     m_x_axis.push_back( 1.70784 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.476997 );
//     m_x_axis.push_back( 1.73333 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.518714 );
//     m_x_axis.push_back( 1.75882 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.559797 );
//     m_x_axis.push_back( 1.78431 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.60088 );
//     m_x_axis.push_back( 1.8098 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.641963 );
//     m_x_axis.push_back( 1.83529 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.683046 );
//     m_x_axis.push_back( 1.86078 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.720045 );
//     m_x_axis.push_back( 1.88627 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.755047 );
//     m_x_axis.push_back( 1.91176 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.790048 );
//     m_x_axis.push_back( 1.93725 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.82505 );
//     m_x_axis.push_back( 1.96274 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.860052 );
//     m_x_axis.push_back( 1.98823 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.01372 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.03922 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.06471 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.0902 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.11569 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.14118 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.16667 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.19216 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.21765 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.24314 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.26863 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.29412 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.31961 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.3451 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.37059 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.39608 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.42157 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.44706 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.47255 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.49804 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.52353 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.54902 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.57451 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.6 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.62549 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.65098 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.67647 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.70196 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.72745 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.75294 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.77843 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.80392 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.82941 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.8549 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.88039 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.90588 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.93138 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.95687 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 2.98236 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.00785 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.03334 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.05883 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.08432 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.10981 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.1353 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.16079 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.18628 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.21177 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.23726 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.26275 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.28824 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.31373 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.33922 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.36471 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.3902 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.41569 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.44118 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.46667 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.49216 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.51765 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.54314 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 3.56863 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.784549 );
//     m_x_axis.push_back( 3.59412 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.731896 );
//     m_x_axis.push_back( 3.61961 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.679243 );
//     m_x_axis.push_back( 3.6451 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.626589 );
//     m_x_axis.push_back( 3.67059 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.587754 );
//     m_x_axis.push_back( 3.69608 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.634245 );
//     m_x_axis.push_back( 3.72157 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.663574 );
//     m_x_axis.push_back( 3.74706 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.628443 );
//     m_x_axis.push_back( 3.77255 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.597453 );
//     m_x_axis.push_back( 3.79804 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.573896 );
//     m_x_axis.push_back( 3.82353 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.550339 );
//     m_x_axis.push_back( 3.84903 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.526782 );
//     m_x_axis.push_back( 3.87452 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.503225 );
//     m_x_axis.push_back( 3.90001 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.479667 );
//     m_x_axis.push_back( 3.9255 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.45611 );
//     m_x_axis.push_back( 3.95099 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.422824 );
//     m_x_axis.push_back( 3.97648 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.386174 );
//     m_x_axis.push_back( 4.00197 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.375627 );
//     m_x_axis.push_back( 4.02746 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.395747 );
//     m_x_axis.push_back( 4.05295 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.415085 );
//     m_x_axis.push_back( 4.07844 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.418869 );
//     m_x_axis.push_back( 4.10393 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.422652 );
//     m_x_axis.push_back( 4.12942 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.426436 );
//     m_x_axis.push_back( 4.15491 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.429352 );
//     m_x_axis.push_back( 4.1804 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.43064 );
//     m_x_axis.push_back( 4.20589 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.431928 );
//     m_x_axis.push_back( 4.23138 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.433216 );
//     m_x_axis.push_back( 4.25687 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.434504 );
//     m_x_axis.push_back( 4.28236 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.435834 );
//     m_x_axis.push_back( 4.30785 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.437178 );
//     m_x_axis.push_back( 4.33334 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.438522 );
//     m_x_axis.push_back( 4.35883 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.439866 );
//     m_x_axis.push_back( 4.38432 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.44121 );
//     m_x_axis.push_back( 4.40981 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.442555 );
//     m_x_axis.push_back( 4.4353 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.443899 );
//     m_x_axis.push_back( 4.46079 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.445242 );
//     m_x_axis.push_back( 4.48628 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.447026 );
//     m_x_axis.push_back( 4.51177 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.450787 );
//     m_x_axis.push_back( 4.53726 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.454548 );
//     m_x_axis.push_back( 4.56275 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.458309 );
//     m_x_axis.push_back( 4.58824 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.46207 );
//     m_x_axis.push_back( 4.61373 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.465832 );
//     m_x_axis.push_back( 4.63922 ); m_mask.push_back( 1 ); m_line_value.push_back( 0.469592 );
//     m_x_axis.push_back( 4.66471 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.6902 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.71569 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.74119 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.76668 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.79217 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.81766 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.84315 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.86864 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.89413 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.91962 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.94511 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.9706 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 4.99609 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.02158 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.04707 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.07256 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.09805 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.12354 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.14903 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.17452 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.20001 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.2255 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.25099 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.27648 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.30197 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.32746 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.35295 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.37844 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.40393 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.42942 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.45491 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.4804 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.50589 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.53138 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.55687 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.58236 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.60785 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.63335 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.65884 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.68433 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.70982 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.73531 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.7608 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.78629 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.81178 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.83727 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.86276 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.88825 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.91374 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.93923 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.96472 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 5.99021 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.0157 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.04119 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.06668 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.09217 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.11766 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.14315 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.16864 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.19413 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.21962 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.24511 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.2706 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.29609 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.32158 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.34707 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.37256 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.39805 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.42354 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.44903 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );
//     m_x_axis.push_back( 6.47452 ); m_mask.push_back( 0 ); m_line_value.push_back( 0 );

//     std::cout << m_x_axis.size() << "," << m_mask.size() << "," << m_line_value.size() << std::endl;
// }
bool PlotOverLine::enable_flag()
{
    return ui->plotGroupBox->isChecked();
}
