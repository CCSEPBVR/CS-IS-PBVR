#include "PlotOverTimeEditor.h"
#include "ui_PlotOverTimeEditor.h"

PlotOverTimeEditor::PlotOverTimeEditor( kvs::qt::jaea::Screen* screen, WebSocketPair* websockets, QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::PlotOverTimeEditor )
    , m_q_custom_plot( new QCustomPlot( this ) )
    , m_screen( screen )
    , m_web_sockets( websockets )
    , m_is_operator( false )
{
    ui->setupUi( this );
    ui->customPlotArea->addWidget( m_q_custom_plot );

    const kvs::Xform initializeXform = kvs::Xform( kvs::Mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        ) );

    // NOTE:予め始点終点、線を登録しておく
    m_point_object = new kvs::PointObject( kvs::ValueArray<kvs::Real32>{ 0.0f, 0.0f, 0.0f } );
    m_point_object->setXform( initializeXform );
    m_point_object->setColor( kvs::RGBColor::Yellow() );
    m_point_object->setSize( 30 );
    kvs::StochasticPointRenderer* tmpPointRenderer = new kvs::StochasticPointRenderer();
    m_screen->registerObject( m_point_object, tmpPointRenderer );

    m_point_object->hide();

    calculateInitialTranslation();

    connect( ui->resetPlotViewPushButton, &QPushButton::clicked, this, &PlotOverTimeEditor::onResetPlotView ) ;
    connect( ui->plotOverTimeGroupBox   , &QGroupBox::toggled  , this, &PlotOverTimeEditor::onPlotOverTimeGroupBoxCheckBox );
    connect( ui->applyPushButton        , &QPushButton::clicked, this, &PlotOverTimeEditor::onApply );
}

PlotOverTimeEditor::~PlotOverTimeEditor()
{
    delete ui;
}

void PlotOverTimeEditor::reset()
{
    m_has_last_snap_shot = false;
}

void PlotOverTimeEditor::onOperatorStateUpdate( const bool operatorState )
{
    m_is_operator = operatorState;

    ui->plotOverTimeGroupBox->setEnabled( m_is_operator );
    ui->resolutionSpinBox   ->setEnabled( m_is_operator );
    ui->targetComboBox      ->setEnabled( m_is_operator );
}

void PlotOverTimeEditor::onUpdateNumberOfVector( const int numberOfVector )
{
    m_variable_list.clear();
    for( int i = 0; i < numberOfVector; ++i )
    {
        const QString variableName = QString( "q%1" ).arg( i + 1 );
        m_variable_list << variableName;
    }

    ui->targetComboBox->clear();
    ui->targetComboBox->addItems( m_variable_list );
}

void PlotOverTimeEditor::onReceivePlotOverTimeParameter( const QJsonObject& payload )
{
    const auto kEnable      = QString::fromUtf8( Protocol::Key::Enable );
    const auto kResolution  = QString::fromUtf8( Protocol::Key::Resolution );
    const auto kTarget      = QString::fromUtf8( Protocol::Key::Target );
    const auto kCoords      = QString::fromUtf8( Protocol::Key::Coords );

    const auto kValue = QString::fromUtf8( Protocol::Key::ValueOnLine );
    const auto kXAxis = QString::fromUtf8( Protocol::Key::XAxis );
    const auto kMask  = QString::fromUtf8( Protocol::Key::Mask );

    if( payload.contains( kEnable ) )
    {
        ui->plotOverTimeGroupBox->setChecked( payload.value( kEnable ).toBool() );
    }

    if( payload.contains( kResolution ) )
    {
        ui->resolutionSpinBox->setValue( payload.value( kResolution ).toInt() );
    }

    if( payload.contains( kTarget ) )
    {
        const int targetIndex = payload.value( kTarget ).toInt();
        if( 0 <= targetIndex && targetIndex < ui->targetComboBox->count() )
            ui->targetComboBox->setCurrentIndex( targetIndex );
    }

    if( payload.contains( kCoords ) )
    {
        const QJsonArray coords = payload.value( kCoords ).toArray();
        if( coords.size() == 3 )
        {
            if( m_point_object != nullptr )
            {
                kvs::Xform currentObjectManagerXform = m_screen->scene()->objectManager()->xform(); // NOTE:ObjectManagerのxformを取得
                float scalingFactor = 1 / ( currentObjectManagerXform.inverse() * m_point_object->xform() ).scaling().x();
                float finalX = ( coords.at( 0 ).toDouble() - m_point_object->externalCenter().x() ) +
                               ( (
                                    ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][0] ) +
                                    ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][0] ) +
                                    ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][0] )
                                    ) * scalingFactor
                                ) +
                               ( m_point_initial_translation.x() * scalingFactor );

                float finalY = ( coords.at( 1 ).toDouble() - m_point_object->externalCenter().y() ) +
                               ( (
                                    ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][1] ) +
                                    ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][1] ) +
                                    ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][1] )
                                    ) * scalingFactor
                                ) +
                               ( m_point_initial_translation.y() * scalingFactor );

                float finalZ = ( coords.at( 2 ).toDouble() - m_point_object->externalCenter().z() ) +
                               ( (
                                    ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][2] ) +
                                    ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][2] ) +
                                    ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][2] )
                                    ) * scalingFactor
                                ) +
                               ( m_point_initial_translation.z() * scalingFactor );

                kvs::Xform rotation    = kvs::Xform::Rotation( m_point_object->xform().rotation() );
                kvs::Xform scaling     = kvs::Xform::Scaling( m_point_object->xform().scaling() );
                kvs::Xform translation = kvs::Xform::Translation( kvs::Vec3( finalX, finalY, finalZ ) );

                kvs::Xform newXform = rotation * scaling * translation;
                m_point_object->setXform( newXform );

                kvs::Mat3 inv_rotation       = currentObjectManagerXform.rotation().transposed();
                kvs::Vec3 translation1       = newXform.translation();
                kvs::Vec3 corrected_position = inv_rotation * translation1;

                m_screen->update();
            }
        }
    }

    if( payload.contains( kValue ) &&
        payload.contains( kXAxis ) &&
        payload.contains( kMask ) )
    {
        const QJsonArray valuesArr = payload.value( kValue ).toArray();
        const QJsonArray xAxisArr  = payload.value( kXAxis ).toArray();
        const QJsonArray maskArr   = payload.value( kMask ).toArray();

        std::vector<float> valuesOnLine;
        valuesOnLine.reserve( valuesArr.size() );
        for( const auto& v : valuesArr )
        {
            valuesOnLine.push_back( static_cast<float>( v.toDouble() ) );
        }

        std::vector<float> xAxis;
        xAxis.reserve( xAxisArr.size() );
        for( const auto& x : xAxisArr )
        {
            xAxis.push_back( static_cast<float>( x.toDouble() ) );
        }

        std::vector<bool> mask;
        mask.reserve( maskArr.size() );
        for( const auto& m : maskArr )
        {
            mask.push_back( m.toInt() != 0 );
        }

        setPlotData( xAxis, mask, valuesOnLine );
    }

    m_last_snap_shot     = captureUiSnapshot();
    m_has_last_snap_shot = true;
}

void PlotOverTimeEditor::onDrawVRPlotOverTime( kvs::Real32 coordArray[ 2 * 3 ] )
{
    if( !m_web_sockets->isConnected() ) return;

    if( m_point_object != nullptr )
    {
        kvs::Xform currentObjectManagerXform = m_screen->scene()->objectManager()->xform(); // NOTE:ObjectManagerのxformを取得
        float scalingFactor = 1 / ( currentObjectManagerXform.inverse() * m_point_object->xform() ).scaling().x();
        float finalX = ( coordArray[0] - m_point_object->externalCenter().x() ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][0] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][0] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][0] )
                            ) * scalingFactor
                        ) +
                       ( m_point_initial_translation.x() * scalingFactor );

        float finalY = ( coordArray[1] - m_point_object->externalCenter().y() ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][1] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][1] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][1] )
                            ) * scalingFactor
                        ) +
                       ( m_point_initial_translation.y() * scalingFactor );

        float finalZ = ( coordArray[2] - m_point_object->externalCenter().z() ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][2] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][2] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][2] )
                            ) * scalingFactor
                        ) +
                       ( m_point_initial_translation.z() * scalingFactor );

        kvs::Xform rotation    = kvs::Xform::Rotation( m_point_object->xform().rotation() );
        kvs::Xform scaling     = kvs::Xform::Scaling( m_point_object->xform().scaling() );
        kvs::Xform translation = kvs::Xform::Translation( kvs::Vec3( finalX, finalY, finalZ ) );

        kvs::Xform newXform = rotation * scaling * translation;
        m_point_object->setXform( newXform );

        kvs::Mat3 inv_rotation       = currentObjectManagerXform.rotation().transposed();
        kvs::Vec3 translation1       = newXform.translation();
        kvs::Vec3 corrected_position = inv_rotation * translation1;

        m_x_coord = coordArray[0];
        m_y_coord = coordArray[1];
        m_z_coord = coordArray[2];

        m_screen->update();
    }

    onApply();
}

void PlotOverTimeEditor::onToggleShowHideVRPlotOverTime()
{
    if( !m_web_sockets->isConnected() ) return;
    ui->plotOverTimeGroupBox->setChecked( !ui->plotOverTimeGroupBox->isChecked() );
    onApply();
}

void PlotOverTimeEditor::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void PlotOverTimeEditor::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

bool PlotOverTimeEditor::fuzzyEqual( double a, double b )
{
    const double diff  = qAbs( a - b );
    const double scale = qMax( 1.0, qMax( qAbs( a ), qAbs( b ) ) );
    return diff <= 1e-12 * scale;
}

bool PlotOverTimeEditor::sameVec3( const double a[3], const double b[3] )
{
    return fuzzyEqual( a[0], b[0] ) && fuzzyEqual( a[1], b[1] ) && fuzzyEqual( a[2], b[2] );
}

PlotOverTimeEditor::PlotOverTimeUiSnapshot PlotOverTimeEditor::captureUiSnapshot() const
{
    PlotOverTimeUiSnapshot s;
    s.enable     = ui->plotOverTimeGroupBox->isChecked();
    s.resolution = ui->resolutionSpinBox->value();
    s.target     = ui->targetComboBox->currentIndex();
    return s;
}

void PlotOverTimeEditor::calculateInitialTranslation()
{
    kvs::Vec3 tmp = ( m_screen->scene()->objectManager()->xform().inverse() * m_point_object->xform() ).translation();
    m_point_initial_translation = kvs::Vec3( tmp.x(), tmp.y(), tmp.z() );
}

void PlotOverTimeEditor::setPlotData( std::vector<float> xAxis, std::vector<bool> mask, std::vector<float> values )
{
    // メインスレッドで実行する必要がある場合
    if( QApplication::instance()->thread() != QThread::currentThread() )
    {
        QMetaObject::invokeMethod( this, [=]() { setPlotData(xAxis, mask, values); }, Qt::QueuedConnection );
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

void PlotOverTimeEditor::onResetPlotView()
{
    m_q_custom_plot->xAxis->setRange( m_x_min, m_x_max ); // x 軸の範囲
    m_q_custom_plot->yAxis->setRange( m_y_min, m_y_max ); // y 軸の範囲
    m_q_custom_plot->replot();
}

void PlotOverTimeEditor::onPlotOverTimeGroupBoxCheckBox()
{
    if( !m_point_object ) { return; }
    const bool visible = ui->plotOverTimeGroupBox->isChecked();
    m_point_object->setVisible( visible );
    m_screen->update();
}

void PlotOverTimeEditor::onApply()
{
    if( !m_web_sockets->isConnected() ) return;

    if( !m_has_last_snap_shot )
    {
        m_last_snap_shot     = captureUiSnapshot();
        m_has_last_snap_shot = true;
        return;
    }

    const auto kEvent      = QString::fromUtf8( Protocol::Key::Event );
    const auto kEnable     = QString::fromUtf8( Protocol::Key::Enable );
    const auto kResolution = QString::fromUtf8( Protocol::Key::Resolution );
    const auto kTarget     = QString::fromUtf8( Protocol::Key::Target );
    const auto kCoords     = QString::fromUtf8( Protocol::Key::Coords );

    const PlotOverTimeUiSnapshot now = captureUiSnapshot();
    QJsonObject diff;

    // Enable
    if( now.enable != m_last_snap_shot.enable )
        diff.insert( kEnable, now.enable );

    // Resolution
    if( now.resolution != m_last_snap_shot.resolution )
        diff.insert( kResolution, now.resolution );

    // Target
    if( now.target != m_last_snap_shot.target )
        diff.insert( kTarget, now.target );

    QJsonObject plotOverTimeParameter;
    plotOverTimeParameter.insert( kEvent, QString::fromUtf8( Protocol::Events::PlotOverTimeParameter ) );

    QJsonArray coords;
    coords.append( m_x_coord );
    coords.append( m_y_coord );
    coords.append( m_z_coord );
    plotOverTimeParameter.insert( kCoords, coords );

    for( auto it = diff.begin(); it != diff.end(); ++it )
        plotOverTimeParameter.insert( it.key(), it.value() );

    m_web_sockets->text()->sendTextMessage(
        QJsonDocument( plotOverTimeParameter ).toJson( QJsonDocument::Compact ) );

    emit plotOverTimeParameterUpdate();

    m_last_snap_shot     = now;
    m_has_last_snap_shot = true;

    // qDebug().noquote() << "[PlotOverTimeEditor::onApply] payload(pretty):\n" << QJsonDocument( plotOverTimeParameter ).toJson( QJsonDocument::Indented );
}
