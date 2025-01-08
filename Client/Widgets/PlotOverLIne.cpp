#include "PlotOverLine.h"
#include "ui_PlotOverLine.h"

PlotOverLine::PlotOverLine(QWidget *parent, Connect* Connect)
    : QDockWidget(parent)
    , ui(new Ui::PlotOverLine)
    , m_connect( Connect )
{
    ui->setupUi(this);

    connect( ui->resetPushButton, &QPushButton::clicked, this, &PlotOverLine::onResetButtonClicked );
    connect( ui->resolutionSpinBox, &QSpinBox::valueChanged, this, &PlotOverLine::onResolutionValueChanged );

    connect( ui->startPointXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onStartPointAxisValueChanged );
    connect( ui->startPointYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onStartPointAxisValueChanged );
    connect( ui->startPointZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onStartPointAxisValueChanged );

    connect( ui->endPointXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onEndPointAxisValueChanged );
    connect( ui->endPointYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onEndPointAxisValueChanged );
    connect( ui->endPointZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::onEndPointAxisValueChanged );

    connect( ui->applyPushButton, &QPushButton::clicked, this, &PlotOverLine::onApplyButtonClicked );

    initData();
    setPlotData( xAxis, mask, values );
}

PlotOverLine::~PlotOverLine()
{
    delete ui;
}

void PlotOverLine::onResetButtonClicked()
{
    ui->customPlot->xAxis->setRange(m_x_min, m_x_max); // x 軸の範囲
    ui->customPlot->yAxis->setRange(m_y_min, m_y_max); // y 軸の範囲
    ui->customPlot->replot();
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

void PlotOverLine::setPlotData( kvs::ValueArray<float> xAxis, kvs::ValueArray<bool> mask, kvs::ValueArray<float> values )
{
    // 初期化: 非常に大きい値/小さい値を設定
    m_x_min = std::numeric_limits<double>::max();
    m_x_max = std::numeric_limits<double>::lowest();
    m_y_min = std::numeric_limits<double>::max();
    m_y_max = std::numeric_limits<double>::lowest();

    QVector<double> x( xAxis.size() ), y( values.size() );
    for( int i = 0; i < x.size(); i++ )
    {
        x[i] = xAxis.at(i);
        if (mask.at(i) == true)
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
    for( int i = 0; i < x.size(); i++ )
    {
        if (mask.at(i) == true)
        {
            y[i] = values.at(i);
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
    ui->customPlot->graph(0)->setData(x, y); // データをセット

    // グラフの軸を設定
    ui->customPlot->xAxis->setLabel("xAxis");
    ui->customPlot->yAxis->setLabel("Values");
    std::cout << m_x_min << "," << m_x_max << std::endl;
    std::cout << m_y_min << "," << m_y_max << std::endl;
    ui->customPlot->xAxis->setRange(m_x_min, m_x_max); // x 軸の範囲
    ui->customPlot->yAxis->setRange(m_y_min, m_y_max); // y 軸の範囲
    ui->customPlot->xAxis->ticker()->setTickCount(10);
    ui->customPlot->yAxis->ticker()->setTickCount(10);

    // 拡大縮小、ドラッグを有効にする
    ui->customPlot->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    // グラフを描画
    ui->customPlot->replot();
}

void PlotOverLine::initData()
{
    xAxis.allocate( 256 ); mask.allocate( 256 ); values.allocate( 256 );
    xAxis.at(0) = 0; mask.at(0) = 0; values.at(0) = 0;
    xAxis.at(1) = 0.0254902; mask.at(1) = 0; values.at(1) = 0;
    xAxis.at(2) = 0.0509804; mask.at(2) = 0; values.at(2) = 0;
    xAxis.at(3) = 0.0764706; mask.at(3) = 0; values.at(3) = 0;
    xAxis.at(4) = 0.101961; mask.at(4) = 0; values.at(4) = 0;
    xAxis.at(5) = 0.127451; mask.at(5) = 0; values.at(5) = 0;
    xAxis.at(6) = 0.152941; mask.at(6) = 0; values.at(6) = 0;
    xAxis.at(7) = 0.178431; mask.at(7) = 0; values.at(7) = 0;
    xAxis.at(8) = 0.203922; mask.at(8) = 0; values.at(8) = 0;
    xAxis.at(9) = 0.229412; mask.at(9) = 0; values.at(9) = 0;
    xAxis.at(10) = 0.254902; mask.at(10) = 0; values.at(10) = 0;
    xAxis.at(11) = 0.280392; mask.at(11) = 0; values.at(11) = 0;
    xAxis.at(12) = 0.305882; mask.at(12) = 0; values.at(12) = 0;
    xAxis.at(13) = 0.331373; mask.at(13) = 0; values.at(13) = 0;
    xAxis.at(14) = 0.356863; mask.at(14) = 0; values.at(14) = 0;
    xAxis.at(15) = 0.382353; mask.at(15) = 0; values.at(15) = 0;
    xAxis.at(16) = 0.407843; mask.at(16) = 1; values.at(16) = 0.4;
    xAxis.at(17) = 0.433333; mask.at(17) = 1; values.at(17) = 0.4;
    xAxis.at(18) = 0.458824; mask.at(18) = 1; values.at(18) = 0.4;
    xAxis.at(19) = 0.484314; mask.at(19) = 1; values.at(19) = 0.4;
    xAxis.at(20) = 0.509804; mask.at(20) = 1; values.at(20) = 0.4;
    xAxis.at(21) = 0.535294; mask.at(21) = 1; values.at(21) = 0.4;
    xAxis.at(22) = 0.560784; mask.at(22) = 1; values.at(22) = 0.4;
    xAxis.at(23) = 0.586275; mask.at(23) = 1; values.at(23) = 0.4;
    xAxis.at(24) = 0.611765; mask.at(24) = 1; values.at(24) = 0.4;
    xAxis.at(25) = 0.637255; mask.at(25) = 1; values.at(25) = 0.4;
    xAxis.at(26) = 0.662745; mask.at(26) = 1; values.at(26) = 0.4;
    xAxis.at(27) = 0.688236; mask.at(27) = 1; values.at(27) = 0.4;
    xAxis.at(28) = 0.713726; mask.at(28) = 1; values.at(28) = 0.4;
    xAxis.at(29) = 0.739216; mask.at(29) = 1; values.at(29) = 0.4;
    xAxis.at(30) = 0.764706; mask.at(30) = 1; values.at(30) = 0.4;
    xAxis.at(31) = 0.790196; mask.at(31) = 1; values.at(31) = 0.4;
    xAxis.at(32) = 0.815687; mask.at(32) = 1; values.at(32) = 0.4;
    xAxis.at(33) = 0.841177; mask.at(33) = 1; values.at(33) = 0.4;
    xAxis.at(34) = 0.866667; mask.at(34) = 1; values.at(34) = 0.4;
    xAxis.at(35) = 0.892157; mask.at(35) = 1; values.at(35) = 0.4;
    xAxis.at(36) = 0.917648; mask.at(36) = 1; values.at(36) = 0.4;
    xAxis.at(37) = 0.943138; mask.at(37) = 1; values.at(37) = 0.4;
    xAxis.at(38) = 0.968628; mask.at(38) = 1; values.at(38) = 0.4;
    xAxis.at(39) = 0.994118; mask.at(39) = 1; values.at(39) = 0.4;
    xAxis.at(40) = 1.01961; mask.at(40) = 1; values.at(40) = 0.387356;
    xAxis.at(41) = 1.0451; mask.at(41) = 1; values.at(41) = 0.374418;
    xAxis.at(42) = 1.07059; mask.at(42) = 1; values.at(42) = 0.361479;
    xAxis.at(43) = 1.09608; mask.at(43) = 1; values.at(43) = 0.348541;
    xAxis.at(44) = 1.12157; mask.at(44) = 1; values.at(44) = 0.336495;
    xAxis.at(45) = 1.14706; mask.at(45) = 1; values.at(45) = 0.330428;
    xAxis.at(46) = 1.17255; mask.at(46) = 1; values.at(46) = 0.324361;
    xAxis.at(47) = 1.19804; mask.at(47) = 1; values.at(47) = 0.318294;
    xAxis.at(48) = 1.22353; mask.at(48) = 1; values.at(48) = 0.312226;
    xAxis.at(49) = 1.24902; mask.at(49) = 1; values.at(49) = 0.306159;
    xAxis.at(50) = 1.27451; mask.at(50) = 1; values.at(50) = 0.300092;
    xAxis.at(51) = 1.3; mask.at(51) = 1; values.at(51) = 0.294025;
    xAxis.at(52) = 1.32549; mask.at(52) = 1; values.at(52) = 0.281975;
    xAxis.at(53) = 1.35098; mask.at(53) = 1; values.at(53) = 0.272457;
    xAxis.at(54) = 1.37647; mask.at(54) = 1; values.at(54) = 0.285732;
    xAxis.at(55) = 1.40196; mask.at(55) = 1; values.at(55) = 0.299378;
    xAxis.at(56) = 1.42745; mask.at(56) = 1; values.at(56) = 0.313023;
    xAxis.at(57) = 1.45294; mask.at(57) = 1; values.at(57) = 0.326668;
    xAxis.at(58) = 1.47843; mask.at(58) = 1; values.at(58) = 0.340313;
    xAxis.at(59) = 1.50392; mask.at(59) = 1; values.at(59) = 0.353959;
    xAxis.at(60) = 1.52941; mask.at(60) = 1; values.at(60) = 0.367604;
    xAxis.at(61) = 1.5549; mask.at(61) = 1; values.at(61) = 0.381163;
    xAxis.at(62) = 1.58039; mask.at(62) = 1; values.at(62) = 0.39456;
    xAxis.at(63) = 1.60588; mask.at(63) = 1; values.at(63) = 0.407956;
    xAxis.at(64) = 1.63137; mask.at(64) = 1; values.at(64) = 0.421352;
    xAxis.at(65) = 1.65686; mask.at(65) = 1; values.at(65) = 0.434749;
    xAxis.at(66) = 1.68235; mask.at(66) = 1; values.at(66) = 0.444516;
    xAxis.at(67) = 1.70784; mask.at(67) = 1; values.at(67) = 0.476997;
    xAxis.at(68) = 1.73333; mask.at(68) = 1; values.at(68) = 0.518714;
    xAxis.at(69) = 1.75882; mask.at(69) = 1; values.at(69) = 0.559797;
    xAxis.at(70) = 1.78431; mask.at(70) = 1; values.at(70) = 0.60088;
    xAxis.at(71) = 1.8098; mask.at(71) = 1; values.at(71) = 0.641963;
    xAxis.at(72) = 1.83529; mask.at(72) = 1; values.at(72) = 0.683046;
    xAxis.at(73) = 1.86078; mask.at(73) = 1; values.at(73) = 0.720045;
    xAxis.at(74) = 1.88627; mask.at(74) = 1; values.at(74) = 0.755047;
    xAxis.at(75) = 1.91176; mask.at(75) = 1; values.at(75) = 0.790048;
    xAxis.at(76) = 1.93725; mask.at(76) = 1; values.at(76) = 0.82505;
    xAxis.at(77) = 1.96274; mask.at(77) = 1; values.at(77) = 0.860052;
    xAxis.at(78) = 1.98823; mask.at(78) = 0; values.at(78) = 0;
    xAxis.at(79) = 2.01372; mask.at(79) = 0; values.at(79) = 0;
    xAxis.at(80) = 2.03922; mask.at(80) = 0; values.at(80) = 0;
    xAxis.at(81) = 2.06471; mask.at(81) = 0; values.at(81) = 0;
    xAxis.at(82) = 2.0902; mask.at(82) = 0; values.at(82) = 0;
    xAxis.at(83) = 2.11569; mask.at(83) = 0; values.at(83) = 0;
    xAxis.at(84) = 2.14118; mask.at(84) = 0; values.at(84) = 0;
    xAxis.at(85) = 2.16667; mask.at(85) = 0; values.at(85) = 0;
    xAxis.at(86) = 2.19216; mask.at(86) = 0; values.at(86) = 0;
    xAxis.at(87) = 2.21765; mask.at(87) = 0; values.at(87) = 0;
    xAxis.at(88) = 2.24314; mask.at(88) = 0; values.at(88) = 0;
    xAxis.at(89) = 2.26863; mask.at(89) = 0; values.at(89) = 0;
    xAxis.at(90) = 2.29412; mask.at(90) = 0; values.at(90) = 0;
    xAxis.at(91) = 2.31961; mask.at(91) = 0; values.at(91) = 0;
    xAxis.at(92) = 2.3451; mask.at(92) = 0; values.at(92) = 0;
    xAxis.at(93) = 2.37059; mask.at(93) = 0; values.at(93) = 0;
    xAxis.at(94) = 2.39608; mask.at(94) = 0; values.at(94) = 0;
    xAxis.at(95) = 2.42157; mask.at(95) = 0; values.at(95) = 0;
    xAxis.at(96) = 2.44706; mask.at(96) = 0; values.at(96) = 0;
    xAxis.at(97) = 2.47255; mask.at(97) = 0; values.at(97) = 0;
    xAxis.at(98) = 2.49804; mask.at(98) = 0; values.at(98) = 0;
    xAxis.at(99) = 2.52353; mask.at(99) = 0; values.at(99) = 0;
    xAxis.at(100) = 2.54902; mask.at(100) = 0; values.at(100) = 0;
    xAxis.at(101) = 2.57451; mask.at(101) = 0; values.at(101) = 0;
    xAxis.at(102) = 2.6; mask.at(102) = 0; values.at(102) = 0;
    xAxis.at(103) = 2.62549; mask.at(103) = 0; values.at(103) = 0;
    xAxis.at(104) = 2.65098; mask.at(104) = 0; values.at(104) = 0;
    xAxis.at(105) = 2.67647; mask.at(105) = 0; values.at(105) = 0;
    xAxis.at(106) = 2.70196; mask.at(106) = 0; values.at(106) = 0;
    xAxis.at(107) = 2.72745; mask.at(107) = 0; values.at(107) = 0;
    xAxis.at(108) = 2.75294; mask.at(108) = 0; values.at(108) = 0;
    xAxis.at(109) = 2.77843; mask.at(109) = 0; values.at(109) = 0;
    xAxis.at(110) = 2.80392; mask.at(110) = 0; values.at(110) = 0;
    xAxis.at(111) = 2.82941; mask.at(111) = 0; values.at(111) = 0;
    xAxis.at(112) = 2.8549; mask.at(112) = 0; values.at(112) = 0;
    xAxis.at(113) = 2.88039; mask.at(113) = 0; values.at(113) = 0;
    xAxis.at(114) = 2.90588; mask.at(114) = 0; values.at(114) = 0;
    xAxis.at(115) = 2.93138; mask.at(115) = 0; values.at(115) = 0;
    xAxis.at(116) = 2.95687; mask.at(116) = 0; values.at(116) = 0;
    xAxis.at(117) = 2.98236; mask.at(117) = 0; values.at(117) = 0;
    xAxis.at(118) = 3.00785; mask.at(118) = 0; values.at(118) = 0;
    xAxis.at(119) = 3.03334; mask.at(119) = 0; values.at(119) = 0;
    xAxis.at(120) = 3.05883; mask.at(120) = 0; values.at(120) = 0;
    xAxis.at(121) = 3.08432; mask.at(121) = 0; values.at(121) = 0;
    xAxis.at(122) = 3.10981; mask.at(122) = 0; values.at(122) = 0;
    xAxis.at(123) = 3.1353; mask.at(123) = 0; values.at(123) = 0;
    xAxis.at(124) = 3.16079; mask.at(124) = 0; values.at(124) = 0;
    xAxis.at(125) = 3.18628; mask.at(125) = 0; values.at(125) = 0;
    xAxis.at(126) = 3.21177; mask.at(126) = 0; values.at(126) = 0;
    xAxis.at(127) = 3.23726; mask.at(127) = 0; values.at(127) = 0;
    xAxis.at(128) = 3.26275; mask.at(128) = 0; values.at(128) = 0;
    xAxis.at(129) = 3.28824; mask.at(129) = 0; values.at(129) = 0;
    xAxis.at(130) = 3.31373; mask.at(130) = 0; values.at(130) = 0;
    xAxis.at(131) = 3.33922; mask.at(131) = 0; values.at(131) = 0;
    xAxis.at(132) = 3.36471; mask.at(132) = 0; values.at(132) = 0;
    xAxis.at(133) = 3.3902; mask.at(133) = 0; values.at(133) = 0;
    xAxis.at(134) = 3.41569; mask.at(134) = 0; values.at(134) = 0;
    xAxis.at(135) = 3.44118; mask.at(135) = 0; values.at(135) = 0;
    xAxis.at(136) = 3.46667; mask.at(136) = 0; values.at(136) = 0;
    xAxis.at(137) = 3.49216; mask.at(137) = 0; values.at(137) = 0;
    xAxis.at(138) = 3.51765; mask.at(138) = 0; values.at(138) = 0;
    xAxis.at(139) = 3.54314; mask.at(139) = 0; values.at(139) = 0;
    xAxis.at(140) = 3.56863; mask.at(140) = 1; values.at(140) = 0.784549;
    xAxis.at(141) = 3.59412; mask.at(141) = 1; values.at(141) = 0.731896;
    xAxis.at(142) = 3.61961; mask.at(142) = 1; values.at(142) = 0.679243;
    xAxis.at(143) = 3.6451; mask.at(143) = 1; values.at(143) = 0.626589;
    xAxis.at(144) = 3.67059; mask.at(144) = 1; values.at(144) = 0.587754;
    xAxis.at(145) = 3.69608; mask.at(145) = 1; values.at(145) = 0.634245;
    xAxis.at(146) = 3.72157; mask.at(146) = 1; values.at(146) = 0.663574;
    xAxis.at(147) = 3.74706; mask.at(147) = 1; values.at(147) = 0.628443;
    xAxis.at(148) = 3.77255; mask.at(148) = 1; values.at(148) = 0.597453;
    xAxis.at(149) = 3.79804; mask.at(149) = 1; values.at(149) = 0.573896;
    xAxis.at(150) = 3.82353; mask.at(150) = 1; values.at(150) = 0.550339;
    xAxis.at(151) = 3.84903; mask.at(151) = 1; values.at(151) = 0.526782;
    xAxis.at(152) = 3.87452; mask.at(152) = 1; values.at(152) = 0.503225;
    xAxis.at(153) = 3.90001; mask.at(153) = 1; values.at(153) = 0.479667;
    xAxis.at(154) = 3.9255; mask.at(154) = 1; values.at(154) = 0.45611;
    xAxis.at(155) = 3.95099; mask.at(155) = 1; values.at(155) = 0.422824;
    xAxis.at(156) = 3.97648; mask.at(156) = 1; values.at(156) = 0.386174;
    xAxis.at(157) = 4.00197; mask.at(157) = 1; values.at(157) = 0.375627;
    xAxis.at(158) = 4.02746; mask.at(158) = 1; values.at(158) = 0.395747;
    xAxis.at(159) = 4.05295; mask.at(159) = 1; values.at(159) = 0.415085;
    xAxis.at(160) = 4.07844; mask.at(160) = 1; values.at(160) = 0.418869;
    xAxis.at(161) = 4.10393; mask.at(161) = 1; values.at(161) = 0.422652;
    xAxis.at(162) = 4.12942; mask.at(162) = 1; values.at(162) = 0.426436;
    xAxis.at(163) = 4.15491; mask.at(163) = 1; values.at(163) = 0.429352;
    xAxis.at(164) = 4.1804; mask.at(164) = 1; values.at(164) = 0.43064;
    xAxis.at(165) = 4.20589; mask.at(165) = 1; values.at(165) = 0.431928;
    xAxis.at(166) = 4.23138; mask.at(166) = 1; values.at(166) = 0.433216;
    xAxis.at(167) = 4.25687; mask.at(167) = 1; values.at(167) = 0.434504;
    xAxis.at(168) = 4.28236; mask.at(168) = 1; values.at(168) = 0.435834;
    xAxis.at(169) = 4.30785; mask.at(169) = 1; values.at(169) = 0.437178;
    xAxis.at(170) = 4.33334; mask.at(170) = 1; values.at(170) = 0.438522;
    xAxis.at(171) = 4.35883; mask.at(171) = 1; values.at(171) = 0.439866;
    xAxis.at(172) = 4.38432; mask.at(172) = 1; values.at(172) = 0.44121;
    xAxis.at(173) = 4.40981; mask.at(173) = 1; values.at(173) = 0.442555;
    xAxis.at(174) = 4.4353; mask.at(174) = 1; values.at(174) = 0.443899;
    xAxis.at(175) = 4.46079; mask.at(175) = 1; values.at(175) = 0.445242;
    xAxis.at(176) = 4.48628; mask.at(176) = 1; values.at(176) = 0.447026;
    xAxis.at(177) = 4.51177; mask.at(177) = 1; values.at(177) = 0.450787;
    xAxis.at(178) = 4.53726; mask.at(178) = 1; values.at(178) = 0.454548;
    xAxis.at(179) = 4.56275; mask.at(179) = 1; values.at(179) = 0.458309;
    xAxis.at(180) = 4.58824; mask.at(180) = 1; values.at(180) = 0.46207;
    xAxis.at(181) = 4.61373; mask.at(181) = 1; values.at(181) = 0.465832;
    xAxis.at(182) = 4.63922; mask.at(182) = 1; values.at(182) = 0.469592;
    xAxis.at(183) = 4.66471; mask.at(183) = 0; values.at(183) = 0;
    xAxis.at(184) = 4.6902; mask.at(184) = 0; values.at(184) = 0;
    xAxis.at(185) = 4.71569; mask.at(185) = 0; values.at(185) = 0;
    xAxis.at(186) = 4.74119; mask.at(186) = 0; values.at(186) = 0;
    xAxis.at(187) = 4.76668; mask.at(187) = 0; values.at(187) = 0;
    xAxis.at(188) = 4.79217; mask.at(188) = 0; values.at(188) = 0;
    xAxis.at(189) = 4.81766; mask.at(189) = 0; values.at(189) = 0;
    xAxis.at(190) = 4.84315; mask.at(190) = 0; values.at(190) = 0;
    xAxis.at(191) = 4.86864; mask.at(191) = 0; values.at(191) = 0;
    xAxis.at(192) = 4.89413; mask.at(192) = 0; values.at(192) = 0;
    xAxis.at(193) = 4.91962; mask.at(193) = 0; values.at(193) = 0;
    xAxis.at(194) = 4.94511; mask.at(194) = 0; values.at(194) = 0;
    xAxis.at(195) = 4.9706; mask.at(195) = 0; values.at(195) = 0;
    xAxis.at(196) = 4.99609; mask.at(196) = 0; values.at(196) = 0;
    xAxis.at(197) = 5.02158; mask.at(197) = 0; values.at(197) = 0;
    xAxis.at(198) = 5.04707; mask.at(198) = 0; values.at(198) = 0;
    xAxis.at(199) = 5.07256; mask.at(199) = 0; values.at(199) = 0;
    xAxis.at(200) = 5.09805; mask.at(200) = 0; values.at(200) = 0;
    xAxis.at(201) = 5.12354; mask.at(201) = 0; values.at(201) = 0;
    xAxis.at(202) = 5.14903; mask.at(202) = 0; values.at(202) = 0;
    xAxis.at(203) = 5.17452; mask.at(203) = 0; values.at(203) = 0;
    xAxis.at(204) = 5.20001; mask.at(204) = 0; values.at(204) = 0;
    xAxis.at(205) = 5.2255; mask.at(205) = 0; values.at(205) = 0;
    xAxis.at(206) = 5.25099; mask.at(206) = 0; values.at(206) = 0;
    xAxis.at(207) = 5.27648; mask.at(207) = 0; values.at(207) = 0;
    xAxis.at(208) = 5.30197; mask.at(208) = 0; values.at(208) = 0;
    xAxis.at(209) = 5.32746; mask.at(209) = 0; values.at(209) = 0;
    xAxis.at(210) = 5.35295; mask.at(210) = 0; values.at(210) = 0;
    xAxis.at(211) = 5.37844; mask.at(211) = 0; values.at(211) = 0;
    xAxis.at(212) = 5.40393; mask.at(212) = 0; values.at(212) = 0;
    xAxis.at(213) = 5.42942; mask.at(213) = 0; values.at(213) = 0;
    xAxis.at(214) = 5.45491; mask.at(214) = 0; values.at(214) = 0;
    xAxis.at(215) = 5.4804; mask.at(215) = 0; values.at(215) = 0;
    xAxis.at(216) = 5.50589; mask.at(216) = 0; values.at(216) = 0;
    xAxis.at(217) = 5.53138; mask.at(217) = 0; values.at(217) = 0;
    xAxis.at(218) = 5.55687; mask.at(218) = 0; values.at(218) = 0;
    xAxis.at(219) = 5.58236; mask.at(219) = 0; values.at(219) = 0;
    xAxis.at(220) = 5.60785; mask.at(220) = 0; values.at(220) = 0;
    xAxis.at(221) = 5.63335; mask.at(221) = 0; values.at(221) = 0;
    xAxis.at(222) = 5.65884; mask.at(222) = 0; values.at(222) = 0;
    xAxis.at(223) = 5.68433; mask.at(223) = 0; values.at(223) = 0;
    xAxis.at(224) = 5.70982; mask.at(224) = 0; values.at(224) = 0;
    xAxis.at(225) = 5.73531; mask.at(225) = 0; values.at(225) = 0;
    xAxis.at(226) = 5.7608; mask.at(226) = 0; values.at(226) = 0;
    xAxis.at(227) = 5.78629; mask.at(227) = 0; values.at(227) = 0;
    xAxis.at(228) = 5.81178; mask.at(228) = 0; values.at(228) = 0;
    xAxis.at(229) = 5.83727; mask.at(229) = 0; values.at(229) = 0;
    xAxis.at(230) = 5.86276; mask.at(230) = 0; values.at(230) = 0;
    xAxis.at(231) = 5.88825; mask.at(231) = 0; values.at(231) = 0;
    xAxis.at(232) = 5.91374; mask.at(232) = 0; values.at(232) = 0;
    xAxis.at(233) = 5.93923; mask.at(233) = 0; values.at(233) = 0;
    xAxis.at(234) = 5.96472; mask.at(234) = 0; values.at(234) = 0;
    xAxis.at(235) = 5.99021; mask.at(235) = 0; values.at(235) = 0;
    xAxis.at(236) = 6.0157; mask.at(236) = 0; values.at(236) = 0;
    xAxis.at(237) = 6.04119; mask.at(237) = 0; values.at(237) = 0;
    xAxis.at(238) = 6.06668; mask.at(238) = 0; values.at(238) = 0;
    xAxis.at(239) = 6.09217; mask.at(239) = 0; values.at(239) = 0;
    xAxis.at(240) = 6.11766; mask.at(240) = 0; values.at(240) = 0;
    xAxis.at(241) = 6.14315; mask.at(241) = 0; values.at(241) = 0;
    xAxis.at(242) = 6.16864; mask.at(242) = 0; values.at(242) = 0;
    xAxis.at(243) = 6.19413; mask.at(243) = 0; values.at(243) = 0;
    xAxis.at(244) = 6.21962; mask.at(244) = 0; values.at(244) = 0;
    xAxis.at(245) = 6.24511; mask.at(245) = 0; values.at(245) = 0;
    xAxis.at(246) = 6.2706; mask.at(246) = 0; values.at(246) = 0;
    xAxis.at(247) = 6.29609; mask.at(247) = 0; values.at(247) = 0;
    xAxis.at(248) = 6.32158; mask.at(248) = 0; values.at(248) = 0;
    xAxis.at(249) = 6.34707; mask.at(249) = 0; values.at(249) = 0;
    xAxis.at(250) = 6.37256; mask.at(250) = 0; values.at(250) = 0;
    xAxis.at(251) = 6.39805; mask.at(251) = 0; values.at(251) = 0;
    xAxis.at(252) = 6.42354; mask.at(252) = 0; values.at(252) = 0;
    xAxis.at(253) = 6.44903; mask.at(253) = 0; values.at(253) = 0;
    xAxis.at(254) = 6.47452; mask.at(254) = 0; values.at(254) = 0;
    xAxis.at(255) = 6.50001; mask.at(255) = 0; values.at(255) = 0;
}
