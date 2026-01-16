#include "PlotOverLineEditor.h"
#include "ui_PlotOverLineEditor.h"

PlotOverLineEditor::PlotOverLineEditor( kvs::qt::jaea::Screen* screen, WebSocketPair* websockets, QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::PlotOverLineEditor )
    , m_web_sockets( websockets )
    , m_screen( screen )
    , m_q_custom_plot( new QCustomPlot( this ) )
    , m_is_operator( false )
{
    ui->setupUi( this );
    ui->customPlotArea->addWidget( m_q_custom_plot );

    // kvs::Vec3 initializeCoords( 0.0f, 0.0f, 0.0f );
    const kvs::Xform initializeXform = kvs::Xform( kvs::Mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        ) );

    m_start_point_object = new kvs::PointObject( kvs::ValueArray<kvs::Real32>{ 0.0f, 0.0f, 0.0f } );
    m_start_point_object->setXform( initializeXform );
    m_start_point_object->setColor( kvs::RGBColor::Red() );
    m_start_point_object->setSize( 20 );
    kvs::StochasticPointRenderer* startPointRenderer = new kvs::StochasticPointRenderer();
    m_screen->registerObject( m_start_point_object, startPointRenderer );

    m_end_point_object = new kvs::PointObject( kvs::ValueArray<kvs::Real32>{ 0.0f, 0.0f, 0.0f } );
    m_end_point_object->setXform( initializeXform );
    m_end_point_object->setColor( kvs::RGBColor::Blue() );
    m_end_point_object->setSize( 20 );
    kvs::StochasticPointRenderer* endPointRenderer = new kvs::StochasticPointRenderer();
    m_screen->registerObject( m_end_point_object, endPointRenderer );

    const kvs::ValueArray<kvs::Real32> coords( InitialCoords, N * 3 );
    const kvs::ValueArray<kvs::UInt8> colors( InitialColors, N * 3 );
    const kvs::ValueArray<kvs::UInt32> connections( InitialConnections, L * 2 );

    m_plot_over_line_object = new kvs::LineObject();
    m_plot_over_line_object->setXform( initializeXform );
    m_plot_over_line_object->setCoords( coords );
    m_plot_over_line_object->setColors( colors );
    m_plot_over_line_object->setConnections( connections );
    m_plot_over_line_object->setSize( 10 );
    m_plot_over_line_object->setLineType( kvs::LineObject::Segment );
    m_plot_over_line_object->setColorType( kvs::LineObject::VertexColor );
    m_plot_over_line_object->setMinMaxObjectCoords( m_start_point_object->minObjectCoord(), m_start_point_object->maxObjectCoord() );
    m_plot_over_line_object->setMinMaxExternalCoords( m_start_point_object->minExternalCoord(), m_start_point_object->maxExternalCoord() );

    kvs::StochasticLineRenderer* renderer = new kvs::StochasticLineRenderer();
    m_plot_over_line_object_id = m_screen->registerObject( m_plot_over_line_object, renderer );

#ifdef OPENXR_SCREEN
    m_screen->openxrInteractor()->setStartPoint( m_start_point_object );
    m_screen->openxrInteractor()->setEndPoint( m_end_point_object );
#else
    m_start_point_object->hide();
    m_end_point_object->hide();
#endif

    calculateInitialTranslation();

    connect( ui->resetPlotViewPushButton        , &QPushButton::clicked         , this, &PlotOverLineEditor::onResetPlotView ) ;
    connect( ui->plotOverLineGroupBox           , &QGroupBox::toggled           , this, &PlotOverLineEditor::onPlotOverLineGroupBoxCheckBox );

    connect( ui->startTranslationXDoubleSpinBox , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onStartTranslationValueChanged );
    connect( ui->startTranslationYDoubleSpinBox , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onStartTranslationValueChanged );
    connect( ui->startTranslationZDoubleSpinBox , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onStartTranslationValueChanged );
    connect( ui->startCoordsXDoubleSpinBox      , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onStartCoordsValueChanged );
    connect( ui->startCoordsYDoubleSpinBox      , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onStartCoordsValueChanged );
    connect( ui->startCoordsZDoubleSpinBox      , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onStartCoordsValueChanged );

    connect( ui->endTranslationXDoubleSpinBox   , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onEndTranslationValueChanged );
    connect( ui->endTranslationYDoubleSpinBox   , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onEndTranslationValueChanged );
    connect( ui->endTranslationZDoubleSpinBox   , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onEndTranslationValueChanged );
    connect( ui->endCoordsXDoubleSpinBox        , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onEndCoordsValueChanged );
    connect( ui->endCoordsYDoubleSpinBox        , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onEndCoordsValueChanged );
    connect( ui->endCoordsZDoubleSpinBox        , &QDoubleSpinBox::valueChanged , this, &PlotOverLineEditor::onEndCoordsValueChanged );

    connect( ui->createLinePushButton           , &QPushButton::clicked         , this, &PlotOverLineEditor::onCreateLine );
    connect( ui->applyPushButton                , &QPushButton::clicked         , this, &PlotOverLineEditor::onApply );
}

PlotOverLineEditor::~PlotOverLineEditor()
{
    if( m_q_custom_plot ) m_q_custom_plot->setParent( nullptr );  // ui 破棄前に親を外す
    delete ui;
}

void PlotOverLineEditor::onUpdateServerState( bool serverState )
{

}

void PlotOverLineEditor::onOperatorStateUpdate( bool operatorState )
{
    m_is_operator = operatorState;

    ui->plotOverLineGroupBox            ->setEnabled( m_is_operator );
    ui->resolutionSpinBox               ->setEnabled( m_is_operator );
    ui->targetComboBox                  ->setEnabled( m_is_operator );

    ui->startTranslationXDoubleSpinBox  ->setEnabled( m_is_operator );
    ui->startTranslationYDoubleSpinBox  ->setEnabled( m_is_operator );
    ui->startTranslationZDoubleSpinBox  ->setEnabled( m_is_operator );
    ui->startCoordsXDoubleSpinBox       ->setEnabled( m_is_operator );
    ui->startCoordsYDoubleSpinBox       ->setEnabled( m_is_operator );
    ui->startCoordsZDoubleSpinBox       ->setEnabled( m_is_operator );

    ui->endTranslationXDoubleSpinBox    ->setEnabled( m_is_operator );
    ui->endTranslationYDoubleSpinBox    ->setEnabled( m_is_operator );
    ui->endTranslationZDoubleSpinBox    ->setEnabled( m_is_operator );
    ui->endCoordsXDoubleSpinBox         ->setEnabled( m_is_operator );
    ui->endCoordsYDoubleSpinBox         ->setEnabled( m_is_operator );
    ui->endCoordsZDoubleSpinBox         ->setEnabled( m_is_operator );
}

void PlotOverLineEditor::onReset()
{
    onUpdateNumberOfVector( 0 );
    ui->plotOverLineGroupBox->setChecked( false );
    ui->resolutionSpinBox->setValue( 256 );
    ui->targetComboBox->setCurrentIndex( 0 );

    ui->startTranslationXDoubleSpinBox  ->setValue( 0.0 );
    ui->startTranslationYDoubleSpinBox  ->setValue( 0.0 );
    ui->startTranslationZDoubleSpinBox  ->setValue( 0.0 );
    ui->startCoordsXDoubleSpinBox       ->setValue( 0.0 );
    ui->startCoordsYDoubleSpinBox       ->setValue( 0.0 );
    ui->startCoordsZDoubleSpinBox       ->setValue( 0.0 );

    ui->endTranslationXDoubleSpinBox    ->setValue( 0.0 );
    ui->endTranslationYDoubleSpinBox    ->setValue( 0.0 );
    ui->endTranslationZDoubleSpinBox    ->setValue( 0.0 );
    ui->endCoordsXDoubleSpinBox         ->setValue( 0.0 );
    ui->endCoordsYDoubleSpinBox         ->setValue( 0.0 );
    ui->endCoordsZDoubleSpinBox         ->setValue( 0.0 );
}

void PlotOverLineEditor::onUpdateNumberOfVector( const int numberOfVector )
{
    m_vector_list.clear();
    for( int i = 0; i < numberOfVector; ++i )
    {
        const QString name = QString( "q%1" ).arg( i + 1 );
        m_vector_list << name;
    }
    ui->targetComboBox->clear();
    ui->targetComboBox->addItems( m_vector_list );
}

// NOTE:フォーカス対象のオブジェクトが変更された場合に呼び出される。
void PlotOverLineEditor::onUpdateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords )
{
    if( m_start_point_object && m_end_point_object )
    {
        m_start_point_object->setMinMaxObjectCoords( resultMinObjectCoords, resultMaxObjectCoords );
        m_start_point_object->setMinMaxExternalCoords( resultMinObjectCoords, resultMaxObjectCoords );
        m_end_point_object->setMinMaxObjectCoords( resultMinObjectCoords, resultMaxObjectCoords );
        m_end_point_object->setMinMaxExternalCoords( resultMinObjectCoords, resultMaxObjectCoords );

        m_plot_over_line_object->setMinMaxObjectCoords( resultMinObjectCoords, resultMaxObjectCoords );
        m_plot_over_line_object->setMinMaxExternalCoords( resultMinObjectCoords, resultMaxObjectCoords );

        m_screen->scene()->objectManager()->updateMinMaxCoords();
        m_screen->scene()->objectManager()->updateExternalCoords();
        m_screen->update();
    }
}

// NOTE:フォーカス対象のオブジェクトが変更された場合に呼び出される。
void PlotOverLineEditor::onUpdateTranslation()
{
    onStartCoordsValueChanged();
    onEndCoordsValueChanged();
}

void PlotOverLineEditor::onSetPlotData( std::vector<float> xAxis, std::vector<bool> mask, std::vector<float> values )
{
    // メインスレッドで実行する必要がある場合
    if( QApplication::instance()->thread() != QThread::currentThread() )
    {
        QMetaObject::invokeMethod( this, [=]() { onSetPlotData(xAxis, mask, values); }, Qt::QueuedConnection );
        return;
    }

    // 初期化：極端に大きい/小さい値を設定
    m_x_min = std::numeric_limits<double>::max();
    m_x_max = std::numeric_limits<double>::lowest();
    m_y_min = std::numeric_limits<double>::max();
    m_y_max = std::numeric_limits<double>::lowest();

    QVector<double> x( xAxis.size() ), y(values.size() );
    for( size_t i = 0; i < x.size(); i++ )
    {
        x[i] = xAxis[i];
        if( mask[i] )
        {
            if( x[i] < m_x_min )
            {
                m_x_min = x[i];
            }
            if( x[i] > m_x_max )
            {
                m_x_max = x[i];
            }
        }
    }
    for( size_t i = 0; i < x.size(); i++ )
    {
        if( mask[i] )
        {
            y[i] = values[i];
            if( y[i] < m_y_min )
            {
                m_y_min = y[i];
            }
            if( y[i] > m_y_max )
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
    m_q_custom_plot->addGraph(); // 新しいグラフを追加
    m_q_custom_plot->graph( 0 )->setData( x, y ); // データを設定

    // 軸ラベルを設定
    m_q_custom_plot->xAxis->setLabel( "xAxis" );
    m_q_custom_plot->yAxis->setLabel( "Values" );

    // 軸の範囲を設定
    m_q_custom_plot->xAxis->setRange( m_x_min, m_x_max ); // x軸範囲
    m_q_custom_plot->yAxis->setRange( m_y_min, m_y_max ); // y軸範囲
    m_q_custom_plot->xAxis->ticker()->setTickCount( 10 );
    m_q_custom_plot->yAxis->ticker()->setTickCount( 10 );

    // ズームとドラッグを有効化
    m_q_custom_plot->setInteractions( QCP::iRangeZoom | QCP::iRangeDrag );

    // グラフを再描画
    m_q_custom_plot->replot();
}

void PlotOverLineEditor::onReceivePlotOverLineParameter( const QJsonObject& plotOverLineParameter )
{
    // Enable
    ui->plotOverLineGroupBox->setChecked( plotOverLineParameter.value( QString::fromUtf8( Protocol::Key::Enable ) ).toBool() );
    // Resolution
    ui->resolutionSpinBox->setValue( plotOverLineParameter.value( QString::fromUtf8( Protocol::Key::Resolution ) ).toInt() );
    // Resolution
    ui->targetComboBox->setCurrentIndex( plotOverLineParameter.value( QString::fromUtf8( Protocol::Key::Target ) ).toInt() );

    // Start Coords
    const QJsonArray start = plotOverLineParameter.value( QString::fromUtf8( Protocol::Key::StartCoords ) ).toArray();
    if( start.size() == 3 )
    {
        ui->startCoordsXDoubleSpinBox->setValue( start.at( 0 ) .toDouble() ) ;
        ui->startCoordsYDoubleSpinBox->setValue( start.at( 1 ) .toDouble() ) ;
        ui->startCoordsZDoubleSpinBox->setValue( start.at( 2 ) .toDouble() ) ;
    }

    const QJsonArray end = plotOverLineParameter.value( QString::fromUtf8( Protocol::Key::EndCoords ) ).toArray();
    if( end.size() == 3 )
    {
        ui->endCoordsXDoubleSpinBox->setValue( end.at( 0 ) .toDouble() ) ;
        ui->endCoordsYDoubleSpinBox->setValue( end.at( 1 ) .toDouble() ) ;
        ui->endCoordsZDoubleSpinBox->setValue( end.at( 2 ) .toDouble() ) ;
    }
    onCreateLine();
}

void PlotOverLineEditor::onReceiveRequestDataAtPlotOverLineParameter( const QJsonObject& dataArray )
{
    const auto keyValue = QString::fromUtf8( Protocol::Key::ValueOnLine );
    const auto keyXAxis = QString::fromUtf8( Protocol::Key::XAxis );
    const auto keyMask  = QString::fromUtf8( Protocol::Key::Mask );

    const QJsonArray valuesArr = dataArray.value( keyValue ).toArray();
    const QJsonArray xAxisArr  = dataArray.value( keyXAxis ).toArray();
    const QJsonArray maskArr   = dataArray.value( keyMask ).toArray();

    std::vector<float> values_on_line;
    values_on_line.reserve( valuesArr.size() );
    for( const auto& v : valuesArr )
    {
        values_on_line.push_back( static_cast<float>( v.toDouble() ) );
    }

    std::vector<float> x_axis;
    x_axis.reserve( xAxisArr.size() );
    for( const auto& x : xAxisArr )
    {
        x_axis.push_back( static_cast<float>( x.toDouble() ) );
    }

    // ★ onSetPlotData が vector<bool> を要求しているので合わせる
    std::vector<bool> mask;
    mask.reserve( maskArr.size() );
    for( const auto& m : maskArr )
    {
        mask.push_back( m.toInt() != 0 );
    }

    onSetPlotData( x_axis, mask, values_on_line );
}

void PlotOverLineEditor::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void PlotOverLineEditor::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void PlotOverLineEditor::calculateInitialTranslation()
{
#ifdef OPENXR_SCREEN
    // const kvs::Vec3 initialCoords( 0.0f, 0.0f, 0.0f );
    const kvs::Xform initializeXform = kvs::Xform( kvs::Mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        ) );
    m_start_point_object->setXform( initializeXform );
#endif
    kvs::Vec3 tmp = ( m_screen->scene()->objectManager()->xform().inverse() * m_start_point_object->xform() ).translation();
    m_start_point_initial_translation   = kvs::Vec3( tmp.x(), tmp.y(), tmp.z() );
    m_end_point_initial_translation     = kvs::Vec3( tmp.x(), tmp.y(), tmp.z() );
#ifdef OPENXR_SCREEN
    m_screen->openxrInteractor()->setStartInitialTranslation( m_start_point_initial_translation );
    m_screen->openxrInteractor()->setEndInitialTranslation( m_end_point_initial_translation );
#endif
    ui->startTranslationXDoubleSpinBox  ->setValue( m_start_point_initial_translation.x() );
    ui->startTranslationYDoubleSpinBox  ->setValue( m_start_point_initial_translation.y() );
    ui->startTranslationZDoubleSpinBox  ->setValue( m_start_point_initial_translation.z() );
    ui->endTranslationXDoubleSpinBox    ->setValue( m_end_point_initial_translation.x() );
    ui->endTranslationYDoubleSpinBox    ->setValue( m_end_point_initial_translation.y() );
    ui->endTranslationZDoubleSpinBox    ->setValue( m_end_point_initial_translation.z() );
}

void PlotOverLineEditor::updateTranslationSpinBox( kvs::PointObject* point,
                                                     QDoubleSpinBox* xTranslationSpinBox  , QDoubleSpinBox* yTranslationSpinBox   , QDoubleSpinBox* zTranslationSpinBox,
                                                     QDoubleSpinBox* xCoordsSpinBox       , QDoubleSpinBox* yCoordsSpinBox        , QDoubleSpinBox* zCoordsSpinBox,
                                                     const kvs::Vec3& initialTranslation )
{
    if( point != nullptr )
    {
        kvs::Xform currentObjectManagerXform = m_screen->scene()->objectManager()->xform(); // 現在のオブジェクトマネージャーのTranslation, Scaling, Rotationを取得する。
        float scalingFactor = 1 / ( currentObjectManagerXform.inverse() * m_screen->scene()->object( m_screen->scene()->numberOfObjects() - 1 )->xform() ).scaling().x();

        float finalX = ( xTranslationSpinBox->value() * scalingFactor ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][0] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][0] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][0] )
                            ) * scalingFactor
                        );

        float finalY = ( yTranslationSpinBox->value() * scalingFactor ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][1] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][1] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][1] )
                            ) * scalingFactor
                        );


        float finalZ = ( zTranslationSpinBox->value() * scalingFactor ) +
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

        xCoordsSpinBox->blockSignals( true );
        yCoordsSpinBox->blockSignals( true );
        zCoordsSpinBox->blockSignals( true );

        xCoordsSpinBox->setValue( startXCoordinateMovement );
        yCoordsSpinBox->setValue( startYCoordinateMovement );
        zCoordsSpinBox->setValue( startZCoordinateMovement );

        xCoordsSpinBox->blockSignals( false );
        yCoordsSpinBox->blockSignals( false );
        zCoordsSpinBox->blockSignals( false );

        m_screen->update();
    }
}

void PlotOverLineEditor::updateCoordsSpinBox( kvs::PointObject* point,
                                                QDoubleSpinBox* xTranslationSpinBox  , QDoubleSpinBox* yTranslationSpinBox   , QDoubleSpinBox* zTranslationSpinBox,
                                                QDoubleSpinBox* xCoordsSpinBox       , QDoubleSpinBox* yCoordsSpinBox        , QDoubleSpinBox* zCoordsSpinBox,
                                                const kvs::Vec3& initialTranslation )
{
    if( point != nullptr )
    {
        kvs::Xform currentObjectManagerXform = m_screen->scene()->objectManager()->xform(); // 現在のオブジェクトマネージャーのTranslation, Scaling, Rotationを取得する。
        float scalingFactor = 1 / ( currentObjectManagerXform.inverse() * point->xform() ).scaling().x();
        float finalX = ( xCoordsSpinBox->value() - point->externalCenter().x() ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][0] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][0] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][0] )
                            ) * scalingFactor
                        ) +
                       ( initialTranslation.x() * scalingFactor );

        float finalY = ( yCoordsSpinBox->value() - point->externalCenter().y() ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][1] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][1] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][1] )
                            ) * scalingFactor
                        ) +
                       ( initialTranslation.y() * scalingFactor );

        float finalZ = ( zCoordsSpinBox->value() - point->externalCenter().z() ) +
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

        xTranslationSpinBox->blockSignals( true );
        yTranslationSpinBox->blockSignals( true );
        zTranslationSpinBox->blockSignals( true );

        xTranslationSpinBox->setValue( xTranslationMovement );
        yTranslationSpinBox->setValue( yTranslationMovement );
        zTranslationSpinBox->setValue( zTranslationMovement );

        xTranslationSpinBox->blockSignals( false );
        yTranslationSpinBox->blockSignals( false );
        zTranslationSpinBox->blockSignals( false );

        m_screen->update();
    }
}

void PlotOverLineEditor::onResetPlotView()
{
    m_q_custom_plot->xAxis->setRange( m_x_min, m_x_max ); // x 軸の範囲
    m_q_custom_plot->yAxis->setRange( m_y_min, m_y_max ); // y 軸の範囲
    m_q_custom_plot->replot();
}

void PlotOverLineEditor::onPlotOverLineGroupBoxCheckBox()
{
    if( ui->plotOverLineGroupBox->isChecked() )
    {
        if( m_start_point_object && m_end_point_object )
        {
#ifndef OPENXR_SCREEN
            m_start_point_object->show();
            m_end_point_object->show();
#endif
            if( m_plot_over_line_object )
            {
                m_plot_over_line_object->show();
            }
            m_screen->update();
        }
    }
    else
    {
        if( m_start_point_object && m_end_point_object )
        {
#ifndef OPENXR_SCREEN
            m_start_point_object->hide();
            m_end_point_object->hide();
#endif
            if( m_plot_over_line_object )
            {
                m_plot_over_line_object->hide();
            }
            m_screen->update();
        }
    }
}

void PlotOverLineEditor::onStartTranslationValueChanged()
{
    updateTranslationSpinBox(
        m_start_point_object,
        ui->startTranslationXDoubleSpinBox,
        ui->startTranslationYDoubleSpinBox,
        ui->startTranslationZDoubleSpinBox,
        ui->startCoordsXDoubleSpinBox,
        ui->startCoordsYDoubleSpinBox,
        ui->startCoordsZDoubleSpinBox,
        m_start_point_initial_translation
        );
}

void PlotOverLineEditor::onStartCoordsValueChanged()
{
    updateCoordsSpinBox(
        m_start_point_object,
        ui->startTranslationXDoubleSpinBox,
        ui->startTranslationYDoubleSpinBox,
        ui->startTranslationZDoubleSpinBox,
        ui->startCoordsXDoubleSpinBox,
        ui->startCoordsYDoubleSpinBox,
        ui->startCoordsZDoubleSpinBox,
        m_start_point_initial_translation
        );
}

void PlotOverLineEditor::onEndTranslationValueChanged()
{
    updateTranslationSpinBox(
        m_end_point_object,
        ui->endTranslationXDoubleSpinBox,
        ui->endTranslationYDoubleSpinBox,
        ui->endTranslationZDoubleSpinBox,
        ui->endCoordsXDoubleSpinBox,
        ui->endCoordsYDoubleSpinBox,
        ui->endCoordsZDoubleSpinBox,
        m_end_point_initial_translation
        );
}

void PlotOverLineEditor::onEndCoordsValueChanged()
{
    updateCoordsSpinBox(
        m_end_point_object,
        ui->endTranslationXDoubleSpinBox,
        ui->endTranslationYDoubleSpinBox,
        ui->endTranslationZDoubleSpinBox,
        ui->endCoordsXDoubleSpinBox,
        ui->endCoordsYDoubleSpinBox,
        ui->endCoordsZDoubleSpinBox,
        m_end_point_initial_translation
        );
}

void PlotOverLineEditor::onCreateLine()
{
    kvs::Real32 CoordArray[ N * 3 ] = // 座標定義
        {
            kvs::Real32( ui->startCoordsXDoubleSpinBox->value() )   , kvs::Real32(ui->startCoordsYDoubleSpinBox->value() )  , kvs::Real32(ui->startCoordsZDoubleSpinBox->value() ),
            kvs::Real32( ui->endCoordsXDoubleSpinBox->value() )     , kvs::Real32(ui->endCoordsYDoubleSpinBox->value() )    , kvs::Real32(ui->endCoordsZDoubleSpinBox->value() )
        };

    const kvs::ValueArray<kvs::Real32> coords( CoordArray, N * 3 );
    const kvs::ValueArray<kvs::UInt8> colors( InitialColors, N * 3 );
    const kvs::ValueArray<kvs::UInt32> connections( InitialConnections, L * 2 );
    if( m_start_point_object->minObjectCoord() != m_start_point_object->maxObjectCoord() )
    {
        if( m_plot_over_line_object ) // オブジェクトリプレイス
        {
            m_plot_over_line_object = new kvs::LineObject();
            m_plot_over_line_object->setXform( m_screen->scene()->objectManager()->xform( ) );
            m_plot_over_line_object->setCoords( coords );
            m_plot_over_line_object->setColors( colors );
            m_plot_over_line_object->setConnections( connections );
            m_plot_over_line_object->setSize( 10 );
            m_plot_over_line_object->setLineType( kvs::LineObject::Segment );
            m_plot_over_line_object->setColorType( kvs::LineObject::VertexColor );
            m_plot_over_line_object->setMinMaxObjectCoords( m_start_point_object->minObjectCoord(), m_start_point_object->maxObjectCoord() );
            m_plot_over_line_object->setMinMaxExternalCoords( m_start_point_object->minExternalCoord(), m_start_point_object->maxExternalCoord() );
            m_screen->scene()->replaceObject( m_plot_over_line_object_id.first, m_plot_over_line_object );
            if( !ui->plotOverLineGroupBox->isChecked() )
            {
                m_plot_over_line_object->hide();
            }
            m_screen->update();
        }
    }
}

void PlotOverLineEditor::onApply()
{
    if( !m_web_sockets->isConnected() )
    {
        qDebug() << "Not connected";
        return;
    }

    onCreateLine();

    QJsonObject plotOverLineParameter;
    plotOverLineParameter[QString::fromUtf8( Protocol::Key::Event )]          = "PlotOverLineParameter";
    // Enable
    plotOverLineParameter[QString::fromUtf8( Protocol::Key::Enable )]         = ui->plotOverLineGroupBox->isChecked();
    // Resolution
    plotOverLineParameter[QString::fromUtf8( Protocol::Key::Resolution )]     = ui->resolutionSpinBox->value();
    // Target
    plotOverLineParameter[QString::fromUtf8( Protocol::Key::Target )]         = ui->targetComboBox->currentIndex();

    // Start Coords
    QJsonArray startCoords;
    startCoords.append( ui->startCoordsXDoubleSpinBox->value() );
    startCoords.append( ui->startCoordsYDoubleSpinBox->value() );
    startCoords.append( ui->startCoordsZDoubleSpinBox->value() );
    plotOverLineParameter[QString::fromUtf8( Protocol::Key::StartCoords )]    = startCoords;

    // End Coords
    QJsonArray endCoords;
    endCoords.append( ui->endCoordsXDoubleSpinBox->value() );
    endCoords.append( ui->endCoordsYDoubleSpinBox->value() );
    endCoords.append( ui->endCoordsZDoubleSpinBox->value() );
    plotOverLineParameter[QString::fromUtf8( Protocol::Key::EndCoords )]    = endCoords;

    m_web_sockets->text()->sendTextMessage( QJsonDocument( plotOverLineParameter ).toJson(QJsonDocument::Compact ) );
}
