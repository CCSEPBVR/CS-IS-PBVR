#include "PlotOverLine.h"
#include "ui_PlotOverLine.h"

PlotOverLine::PlotOverLine( kvs::qt::jaea::Screen* screen, QWidget *parent )
    : QDockWidget(parent)
    , ui(new Ui::PlotOverLine)
    , m_screen( screen )
    , m_model( new QStandardItemModel( this ) )
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
    // setMaximumSize( maximumWidth(), 250 );

    kvs::Vec3 initialCoord( 0.0f, 0.0f, 0.0f );
    kvs::Xform initXform = kvs::Xform( kvs::Mat4( 1, 0, 0, 0,
                                                  0, 1, 0, 0,
                                                  0, 0, 1, 0,
                                                  0, 0, 0, 1 ) );

    m_start_point = new kvs::PointObject( kvs::ValueArray<kvs::Real32>{ initialCoord.x(), initialCoord.y(), initialCoord.z() } );
    m_start_point->setXform( initXform );
    m_start_point->setColor( kvs::RGBColor::Red() );
    m_start_point->setSize( 10 );
    kvs::StochasticPointRenderer* startPointRenderer = new kvs::StochasticPointRenderer();
    m_screen->registerObject( m_start_point, startPointRenderer );

    m_end_point = new kvs::PointObject( kvs::ValueArray<kvs::Real32>{ initialCoord.x(), initialCoord.y(), initialCoord.z() } );
    m_end_point->setXform( initXform );
    m_end_point->setColor( kvs::RGBColor::Blue() );
    m_end_point->setSize( 10 );
    kvs::StochasticPointRenderer* endPointRenderer = new kvs::StochasticPointRenderer();
    m_screen->registerObject( m_end_point, endPointRenderer );

#ifdef OPENXR_SCREEN
    m_screen->openxrInteractor()->setStartPoint( m_start_point );
    m_screen->openxrInteractor()->setEndPoint( m_end_point );
#else
    m_start_point->hide();
    m_end_point->hide();
#endif

    updateCalTrans();

    connect( ui->resetPushButton, &QPushButton::clicked, this, &PlotOverLine::onReset );

    connect( ui->plotOverLineGroupBox, &QGroupBox::clicked, this, &PlotOverLine::onEnable );

    connect( ui->startTranslationXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::startTranslationUpdateXYZ );
    connect( ui->startTranslationYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::startTranslationUpdateXYZ );
    connect( ui->startTranslationZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::startTranslationUpdateXYZ );
    connect( ui->startCoordXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::startCoordUpdateXYZ );
    connect( ui->startCoordYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::startCoordUpdateXYZ );
    connect( ui->startCoordZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::startCoordUpdateXYZ );

    connect( ui->endTranslationXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::endTranslationUpdateXYZ );
    connect( ui->endTranslationYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::endTranslationUpdateXYZ );
    connect( ui->endTranslationZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::endTranslationUpdateXYZ );
    connect( ui->endCoordXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::endCoordUpdateXYZ );
    connect( ui->endCoordYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::endCoordUpdateXYZ );
    connect( ui->endCoordZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLine::endCoordUpdateXYZ );

    connect( ui->createLinePushButton, &QPushButton::clicked, this, &PlotOverLine::onApply );
}

void PlotOverLine::updateTranslationXYZ( kvs::PointObject* point,
                                        QDoubleSpinBox* xTransSpin, QDoubleSpinBox* yTransSpin, QDoubleSpinBox* zTransSpin,
                                        QDoubleSpinBox* xCoordSpin, QDoubleSpinBox* yCoordSpin, QDoubleSpinBox* zCoordSpin,
                                        const kvs::Vec3& initialTranslation )
{
    if( point != nullptr )
    {
        kvs::Xform currentObjectManagerXform = m_screen->scene()->objectManager()->xform(); // 現在のオブジェクトマネージャーのTranslation, Scaling, Rotationを取得する。
        float scalingFactor = 1 / ( currentObjectManagerXform.inverse() * m_screen->scene()->object( m_screen->scene()->numberOfObjects() - 1 )->xform() ).scaling().x();

        float finalX = ( xTransSpin->value() * scalingFactor ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][0] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][0] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][0] )
                            ) * scalingFactor
                        );

        float finalY = ( yTransSpin->value() * scalingFactor ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][1] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][1] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][1] )
                            ) * scalingFactor
                        );


        float finalZ = ( zTransSpin->value() * scalingFactor ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][2] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][2] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][2] )
                            ) * scalingFactor
                        );

        kvs::Xform rotation = kvs::Xform::Rotation( point->xform().rotation() );
        kvs::Xform scaling = kvs::Xform::Scaling( point->xform().scaling() );
        kvs::Xform translation = kvs::Xform::Translation( kvs::Vec3( finalX, finalY, finalZ ) );

        kvs::Xform newXform = rotation * scaling * translation;

        point->setXform( newXform );

        double startXTranslation = initialTranslation.x() - ( ( currentObjectManagerXform.inverse() * point->xform() ).translation().x() ) - currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x();
        double startYTranslation = initialTranslation.y() - ( ( currentObjectManagerXform.inverse() * point->xform() ).translation().y() ) - currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y();
        double startZTranslation = initialTranslation.z() - ( ( currentObjectManagerXform.inverse() * point->xform() ).translation().z() ) - currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z();

        double startXCoordinateMovement = ( startXTranslation * scalingFactor * -1 ) -
                                          ( currentObjectManagerXform.translation().x() * scalingFactor * currentObjectManagerXform.inverse().scaling().x() );
        double startYCoordinateMovement = ( startYTranslation * scalingFactor * -1 ) -
                                          ( currentObjectManagerXform.translation().y() * scalingFactor * currentObjectManagerXform.inverse().scaling().y() );
        double startZCoordinateMovement = ( startZTranslation * scalingFactor * -1 ) -
                                          ( currentObjectManagerXform.translation().z() * scalingFactor * currentObjectManagerXform.inverse().scaling().z() );

        { // ObjectEditorとの連携時に必要
            startXCoordinateMovement += point->externalCenter().x();
            startYCoordinateMovement += point->externalCenter().y();
            startZCoordinateMovement += point->externalCenter().z();
        }

        xCoordSpin->blockSignals( true );
        yCoordSpin->blockSignals( true );
        zCoordSpin->blockSignals( true );
        xCoordSpin->setValue( startXCoordinateMovement );
        yCoordSpin->setValue( startYCoordinateMovement );
        zCoordSpin->setValue( startZCoordinateMovement );
        xCoordSpin->blockSignals( false );
        yCoordSpin->blockSignals( false );
        zCoordSpin->blockSignals( false );

        m_screen->update();
    }
}

void PlotOverLine::updateCoordinateXYZ( kvs::PointObject* point,
                                       QDoubleSpinBox* xTransSpin, QDoubleSpinBox* yTransSpin, QDoubleSpinBox* zTransSpin,
                                       QDoubleSpinBox* xCoordSpin, QDoubleSpinBox* yCoordSpin, QDoubleSpinBox* zCoordSpin,
                                       const kvs::Vec3& initialTranslation )
{
    if( point != nullptr )
    {
        kvs::Xform currentObjectManagerXform = m_screen->scene()->objectManager()->xform(); // 現在のオブジェクトマネージャーのTranslation, Scaling, Rotationを取得する。
        float scalingFactor = 1 / ( currentObjectManagerXform.inverse() * point->xform() ).scaling().x();
        float finalX = ( xCoordSpin->value() - point->externalCenter().x() ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][0] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][0] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][0] )
                            ) * scalingFactor
                        ) +
                       ( initialTranslation.x() * scalingFactor );

        float finalY = ( yCoordSpin->value() - point->externalCenter().y() ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][1] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][1] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][1] )
                            ) * scalingFactor
                        ) +
                       ( initialTranslation.y() * scalingFactor );

        float finalZ = ( zCoordSpin->value() - point->externalCenter().z() ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][2] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][2] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][2] )
                            ) * scalingFactor
                        ) +
                       ( initialTranslation.z() * scalingFactor );

        kvs::Xform rotation = kvs::Xform::Rotation( point->xform().rotation() );
        kvs::Xform scaling = kvs::Xform::Scaling( point->xform().scaling() );
        kvs::Xform translation = kvs::Xform::Translation( kvs::Vec3( finalX, finalY, finalZ) );

        kvs::Xform newXform = rotation * scaling * translation;
        point->setXform( newXform );

        kvs::Mat3 inv_rotation = currentObjectManagerXform.rotation().transposed();
        kvs::Vec3 translation1 = newXform.translation();
        kvs::Vec3 corrected_position = inv_rotation * translation1;

        float xTranslationMovement = corrected_position.x() * currentObjectManagerXform.inverse().scaling().x()
                                     + currentObjectManagerXform.inverse().translation().x();

        float yTranslationMovement = corrected_position.y() * currentObjectManagerXform.inverse().scaling().y()
                                     + currentObjectManagerXform.inverse().translation().y();

        float zTranslationMovement = corrected_position.z() * currentObjectManagerXform.inverse().scaling().z()
                                     + currentObjectManagerXform.inverse().translation().z();

        xTransSpin->blockSignals( true );
        yTransSpin->blockSignals( true );
        zTransSpin->blockSignals( true );

        xTransSpin->setValue( xTranslationMovement );
        yTransSpin->setValue( yTranslationMovement );
        zTransSpin->setValue( zTranslationMovement );

        xTransSpin->blockSignals( false );
        yTransSpin->blockSignals( false );
        zTransSpin->blockSignals( false );

        m_screen->update();
    }
}

void PlotOverLine::focus( kvs::Vec3 min, kvs::Vec3 max )
{
    if( m_start_point != nullptr && m_end_point != nullptr )
    {
        m_start_point->setMinMaxObjectCoords( min, max );
        m_start_point->setMinMaxExternalCoords( min, max );
        m_end_point->setMinMaxObjectCoords( min, max );
        m_end_point->setMinMaxExternalCoords( min, max );
    }

    if( m_plot_over_line != nullptr )
    {
        m_plot_over_line->setMinMaxObjectCoords( min, max );
        m_plot_over_line->setMinMaxExternalCoords( min, max );
    }

    m_screen->scene()->objectManager()->updateMinMaxCoords();
    m_screen->scene()->objectManager()->updateExternalCoords();

    m_screen->update();
}

void PlotOverLine::updateFocus( kvs::Vec3 min, kvs::Vec3 max )
{    
    focus( min, max );
}

void PlotOverLine::updatePointsTranslation()
{
    updateCoordinateXYZ(
        m_start_point,
        ui->startTranslationXDoubleSpinBox,
        ui->startTranslationYDoubleSpinBox,
        ui->startTranslationZDoubleSpinBox,
        ui->startCoordXDoubleSpinBox,
        ui->startCoordYDoubleSpinBox,
        ui->startCoordZDoubleSpinBox,
        m_start_point_initial_translation
        );

    updateCoordinateXYZ(
        m_end_point,
        ui->endTranslationXDoubleSpinBox,
        ui->endTranslationYDoubleSpinBox,
        ui->endTranslationZDoubleSpinBox,
        ui->endCoordXDoubleSpinBox,
        ui->endCoordYDoubleSpinBox,
        ui->endCoordZDoubleSpinBox,
        m_end_point_initial_translation
        );
}

void PlotOverLine::updateNumberOfVector( const int numberOfVector )
{
    m_vector_list.clear();
    for( int i = 0; i < numberOfVector; i++ )
    {
        m_vector_list << QString( "q%1" ).arg( i + 1 );
    }

    ui->targetComboBox->clear();
    ui->targetComboBox->addItems( m_vector_list );
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

void PlotOverLine::onReset()
{
    ui->customPlot->xAxis->setRange(m_x_min, m_x_max); // x 軸の範囲
    ui->customPlot->yAxis->setRange(m_y_min, m_y_max); // y 軸の範囲
    ui->customPlot->replot();
}

void PlotOverLine::onEnable()
{
    if( m_start_point->isVisible() && m_end_point->isVisible() )
    {
        m_start_point->hide();
        m_end_point->hide();
        if( m_plot_over_line != nullptr )
        {
            m_plot_over_line->hide();
        }
    }
    else
    {
        m_start_point->show();
        m_end_point->show();
        if( m_plot_over_line != nullptr )
        {
            m_plot_over_line->show();
        }
    }
    m_screen->update();
}

void PlotOverLine::startTranslationUpdateXYZ()
{
    updateTranslationXYZ(
        m_start_point,
        ui->startTranslationXDoubleSpinBox,
        ui->startTranslationYDoubleSpinBox,
        ui->startTranslationZDoubleSpinBox,
        ui->startCoordXDoubleSpinBox,
        ui->startCoordYDoubleSpinBox,
        ui->startCoordZDoubleSpinBox,
        m_start_point_initial_translation
        );
}

void PlotOverLine::startCoordUpdateXYZ()
{
    updateCoordinateXYZ(
        m_start_point,
        ui->startTranslationXDoubleSpinBox,
        ui->startTranslationYDoubleSpinBox,
        ui->startTranslationZDoubleSpinBox,
        ui->startCoordXDoubleSpinBox,
        ui->startCoordYDoubleSpinBox,
        ui->startCoordZDoubleSpinBox,
        m_start_point_initial_translation
        );
}

void PlotOverLine::endTranslationUpdateXYZ()
{
    updateTranslationXYZ(
        m_end_point,
        ui->endTranslationXDoubleSpinBox,
        ui->endTranslationYDoubleSpinBox,
        ui->endTranslationZDoubleSpinBox,
        ui->endCoordXDoubleSpinBox,
        ui->endCoordYDoubleSpinBox,
        ui->endCoordZDoubleSpinBox,
        m_end_point_initial_translation
        );
}

void PlotOverLine::endCoordUpdateXYZ()
{
    updateCoordinateXYZ(
        m_end_point,
        ui->endTranslationXDoubleSpinBox,
        ui->endTranslationYDoubleSpinBox,
        ui->endTranslationZDoubleSpinBox,
        ui->endCoordXDoubleSpinBox,
        ui->endCoordYDoubleSpinBox,
        ui->endCoordZDoubleSpinBox,
        m_end_point_initial_translation
        );
}

void PlotOverLine::onApply()
{
    m_model->clear();

    if( ui->plotOverLineGroupBox->isChecked() )
    {

        // 点の数
        const size_t N = 2;
        // 線分の数
        const size_t L = 1;

        kvs::Real32 CoordArray[ N * 3 ] = {
            kvs::Real32( ui->startCoordXDoubleSpinBox->value()), kvs::Real32(ui->startCoordYDoubleSpinBox->value()), kvs::Real32(ui->startCoordZDoubleSpinBox->value() ),
            kvs::Real32( ui->endCoordXDoubleSpinBox->value()), kvs::Real32(ui->endCoordYDoubleSpinBox->value()), kvs::Real32(ui->endCoordZDoubleSpinBox->value() )
        };

        // カラー値配列の定義
        kvs::UInt8 ColorArray[ N * 3 ] = {
            0, 255, 0,
            0, 255, 0,
        };

        kvs::UInt32 ConnectionArray[ L * 2 ] = {
            0, 1,  // 点0と点1を接続
        };

        kvs::ValueArray<kvs::Real32> coords( CoordArray, N * 3 );
        kvs::ValueArray<kvs::UInt8> colors( ColorArray, N * 3 );
        kvs::ValueArray<kvs::UInt32> connections( ConnectionArray, L * 2 );

        if( m_start_point->minObjectCoord() != m_start_point->maxObjectCoord() )
        {
            if( m_plot_over_line == nullptr )
            {
                m_plot_over_line = new kvs::LineObject();
                m_plot_over_line->setXform( m_screen->scene()->objectManager()->xform( ) );
                m_plot_over_line->setCoords( coords );
                m_plot_over_line->setColors( colors );
                m_plot_over_line->setConnections( connections );
                m_plot_over_line->setSize( 10 );
                m_plot_over_line->setLineType( kvs::LineObject::Segment );
                m_plot_over_line->setColorType( kvs::LineObject::VertexColor );
                m_plot_over_line->setMinMaxObjectCoords( m_start_point->minObjectCoord(), m_start_point->maxObjectCoord() );
                m_plot_over_line->setMinMaxExternalCoords( m_start_point->minExternalCoord(), m_start_point->maxExternalCoord() );

                kvs::StochasticLineRenderer* renderer = new kvs::StochasticLineRenderer();
                m_line_object_id = m_screen->registerObject( m_plot_over_line, renderer );
            }
            else
            {
                m_plot_over_line = new kvs::LineObject( coords );
                m_plot_over_line->setXform( m_screen->scene()->objectManager()->xform() );
                m_plot_over_line->setLineTypeToStrip();
                m_plot_over_line->setColorTypeToLine();
                m_plot_over_line->setColor( kvs::RGBColor::Green() );
                m_plot_over_line->setSize( 10 );
                m_plot_over_line->setMinMaxObjectCoords( m_start_point->minObjectCoord(), m_start_point->maxObjectCoord() );
                m_plot_over_line->setMinMaxExternalCoords( m_start_point->minExternalCoord(), m_start_point->maxExternalCoord() );
                m_screen->scene()->replaceObject( m_line_object_id.first, m_plot_over_line );
            }
        }

        m_screen->update();
    }

    QVector3D start_point(
        ui->startCoordXDoubleSpinBox->value(),
        ui->startCoordYDoubleSpinBox->value(),
        ui->startCoordZDoubleSpinBox->value()
        );
    QVector3D end_point(
        ui->endCoordXDoubleSpinBox->value(),
        ui->endCoordYDoubleSpinBox->value(),
        ui->endCoordZDoubleSpinBox->value()
        );

    auto* rootItem = new QStandardItem( "PlotOverLineParams" );
    rootItem->setData( ui->plotOverLineGroupBox->isChecked() , PlotOverLineItem::IsEnable );
    rootItem->setData( ui->resolutionSpinBox->value(), PlotOverLineItem::Resolution );
    rootItem->setData( ui->targetComboBox->currentText(), PlotOverLineItem::Variable );
    rootItem->setData( QVariant::fromValue( start_point ), PlotOverLineItem::StartPoints );
    rootItem->setData( QVariant::fromValue( end_point ), PlotOverLineItem::EndPoints );

    m_model->appendRow( rootItem );

    emit updatePlotOverLineParameterClientMessage( m_model );
    emit requestReplaceServerPointObject();
}

void PlotOverLine::updateCalTrans()
{
#ifdef OPENXR_SCREEN
    kvs::Vec3 initialCoord( 0.0f, 0.0f, 0.0f );

    kvs::Xform initXform = kvs::Xform( kvs::Mat4( 1, 0, 0, 0,
                                                0, 1, 0, 0,
                                                0, 0, 1, 0,
                                                0, 0, 0, 1 ) );

    m_start_point->setXform( initXform );
#endif

    kvs::Vec3 tmp = ( m_screen->scene()->objectManager()->xform().inverse() * m_start_point->xform() ).translation();
    m_start_point_initial_translation = kvs::Vec3( tmp.x(), tmp.y(), tmp.z() );
    m_end_point_initial_translation = kvs::Vec3( tmp.x(), tmp.y(), tmp.z() );

#ifdef OPENXR_SCREEN
    m_screen->openxrInteractor()->setStartInitialTranslation( m_start_point_initial_translation );
    m_screen->openxrInteractor()->setEndInitialTranslation( m_end_point_initial_translation );
#endif

    ui->startTranslationXDoubleSpinBox->setValue( m_start_point_initial_translation.x() );
    ui->startTranslationYDoubleSpinBox->setValue( m_start_point_initial_translation.y() );
    ui->startTranslationZDoubleSpinBox->setValue( m_start_point_initial_translation.z() );

    ui->endTranslationXDoubleSpinBox->setValue( m_end_point_initial_translation.x() );
    ui->endTranslationYDoubleSpinBox->setValue( m_end_point_initial_translation.y() );
    ui->endTranslationZDoubleSpinBox->setValue( m_end_point_initial_translation.z() );
}

void PlotOverLine::drawPlotOverLine( kvs::Real32 CoordArray[ 2 * 3 ] )
{
    m_model->clear();

    // 繧ｫ繝ｩ繝ｼ蛟､驟榊・縺ｮ螳夂ｾｩ
    kvs::UInt8 ColorArray[ 2 * 3 ] = {
        0, 255, 0,
        0, 255, 0,
    };

    kvs::UInt32 ConnectionArray[ 1 * 2 ] = {
        0, 1,  // 轤ｹ0縺ｨ轤ｹ1繧呈磁邯・[m
    };

    kvs::ValueArray<kvs::Real32> coords( CoordArray, 2 * 3 );
    kvs::ValueArray<kvs::UInt8> colors( ColorArray, 2 * 3 );
    kvs::ValueArray<kvs::UInt32> connections( ConnectionArray, 1 * 2 );

    if( m_start_point->minObjectCoord() != m_start_point->maxObjectCoord() )
    {
        if( m_plot_over_line == nullptr )
        {
            m_plot_over_line = new kvs::LineObject();
            m_plot_over_line->setXform( m_screen->scene()->objectManager()->xform( ) );
            m_plot_over_line->setCoords( coords );
            m_plot_over_line->setColors( colors );
            m_plot_over_line->setConnections( connections );
            m_plot_over_line->setSize( 10 );
            m_plot_over_line->setLineType( kvs::LineObject::Segment );
            m_plot_over_line->setColorType( kvs::LineObject::VertexColor );
            m_plot_over_line->setMinMaxObjectCoords( m_start_point->minObjectCoord(), m_start_point->maxObjectCoord() );
            m_plot_over_line->setMinMaxExternalCoords( m_start_point->minExternalCoord(), m_start_point->maxExternalCoord() );

            kvs::StochasticLineRenderer* renderer = new kvs::StochasticLineRenderer();
            m_line_object_id = m_screen->registerObject( m_plot_over_line, renderer );
        }
        else
        {
            m_plot_over_line = new kvs::LineObject( coords );
            m_plot_over_line->setXform( m_screen->scene()->objectManager()->xform() );
            m_plot_over_line->setLineTypeToStrip();
            m_plot_over_line->setColorTypeToLine();
            m_plot_over_line->setColor( kvs::RGBColor::Green() );
            m_plot_over_line->setSize( 10 );
            m_plot_over_line->setMinMaxObjectCoords( m_start_point->minObjectCoord(), m_start_point->maxObjectCoord() );
            m_plot_over_line->setMinMaxExternalCoords( m_start_point->minExternalCoord(), m_start_point->maxExternalCoord() );
            m_screen->scene()->replaceObject( m_line_object_id.first, m_plot_over_line );
        }
    }
    // std::cout << CoordArray[0] << std::endl;
    m_screen->update();
    QVector3D start_point(
        coords.at( 0 ),
        coords.at( 1 ),
        coords.at( 2 )
        );
    QVector3D end_point(
        coords.at( 3 ),
        coords.at( 4 ),
        coords.at( 5 )
        );

    auto* rootItem = new QStandardItem( "PlotOverLineParams" );
    rootItem->setData( true , PlotOverLineItem::IsEnable );
    rootItem->setData( ui->resolutionSpinBox->value(), PlotOverLineItem::Resolution );
    rootItem->setData( ui->targetComboBox->currentText(), PlotOverLineItem::Variable );
    rootItem->setData( QVariant::fromValue( start_point ), PlotOverLineItem::StartPoints );
    rootItem->setData( QVariant::fromValue( end_point ), PlotOverLineItem::EndPoints );

    m_model->appendRow( rootItem );

    emit updatePlotOverLineParameterClientMessage( m_model );
    emit requestReplaceServerPointObject();
}

void PlotOverLine::showHidePlotOverLine()
{
    if( m_plot_over_line != nullptr )
    {
        if( m_plot_over_line->isVisible() )
        {
            m_plot_over_line->hide();
        }
        else
        {
            m_plot_over_line->show();
        }
    }
}
