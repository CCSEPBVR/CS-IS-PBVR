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

    if( m_q_custom_plot->graphCount() == 0 )
    {
        m_q_custom_plot->addGraph();
        QPen pen; pen.setWidth( 2 );
        m_q_custom_plot->graph( 0 )->setPen( pen );
        m_q_custom_plot->graph( 0 )->setLineStyle( QCPGraph::lsLine );
        m_q_custom_plot->graph( 0 )->setScatterStyle( QCPScatterStyle::ssNone );
        m_q_custom_plot->graph( 0 )->setAdaptiveSampling( true );
    }

    if( m_q_custom_plot->graphCount() == 1 )
    {
        m_q_custom_plot->addGraph();
        QPen pen; pen.setWidth( 2 );
        pen.setColor( QColor( 0, 0, 0, 80 ) );
        m_q_custom_plot->graph( 1 )->setPen( pen );
        m_q_custom_plot->graph( 1 )->setLineStyle( QCPGraph::lsLine );
        m_q_custom_plot->graph( 1 )->setScatterStyle( QCPScatterStyle::ssNone );
        m_q_custom_plot->graph( 1 )->setAdaptiveSampling( true );
    }

    m_q_custom_plot->setNotAntialiasedElements( QCP::aeAll );

    m_q_custom_plot->setInteractions( QCP::iRangeDrag | QCP::iRangeZoom );
    m_q_custom_plot->axisRect()->setRangeDrag( Qt::Horizontal | Qt::Vertical );
    m_q_custom_plot->axisRect()->setRangeZoom( Qt::Horizontal | Qt::Vertical );

    m_q_custom_plot->xAxis->setLabel( "Step" );
    m_q_custom_plot->yAxis->setLabel( "Wave" );
    m_q_custom_plot->yAxis->setRange( -1.5, 1.5 );

    m_q_custom_plot->xAxis->setRange( 0.0, 5.0 );

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

    connect( m_q_custom_plot, &QCustomPlot::mousePress,
                     this, [&]( QMouseEvent* )
                     {
                         m_user_interacting = true;
                         m_follow_right_edge = false;
                     } );

    connect( m_q_custom_plot, &QCustomPlot::mouseRelease,
                     this, [&]( QMouseEvent* )
                     {
                         m_user_interacting = false;
                     } );

    connect( m_q_custom_plot, &QCustomPlot::mouseWheel,
                     this, [&]( QWheelEvent* )
                     {
                         m_follow_right_edge = false;
                     } );

    connect( m_q_custom_plot, &QCustomPlot::mouseDoubleClick,
                     this, [&]( QMouseEvent* )
                     {
                         m_follow_right_edge = true;
                     } );

    if( m_combo_connection ) disconnect( m_combo_connection );

    m_combo_connection = connect(
        ui->displayGraphComboBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &PlotOverTimeEditor::onDisplayGraphComboBoxChanged
        );


    connect( ui->resetPlotViewPushButton, &QPushButton::clicked, this, &PlotOverTimeEditor::onResetPlotView ) ;
    connect( ui->plotOverTimeGroupBox   , &QGroupBox::toggled  , this, &PlotOverTimeEditor::onPlotOverTimeGroupBoxCheckBox );
    connect( ui->applyPushButton        , &QPushButton::clicked, this, &PlotOverTimeEditor::onApply );

    m_plot_inited = true;
    m_need_rebuild = true;
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
}

void PlotOverTimeEditor::onUpdateNumberOfVector( const int numberOfVector )
{
    m_variable_list.clear();
    for( int i = 0; i < numberOfVector; ++i )
    {
        const QString variableName = QString( "q%1" ).arg( i + 1 );
        m_variable_list << variableName;
    }

    ui->displayGraphComboBox->blockSignals( true );
    ui->displayGraphComboBox->clear();
    ui->displayGraphComboBox->addItems( m_variable_list );
    ui->displayGraphComboBox->setCurrentIndex( 0 );
    ui->displayGraphComboBox->blockSignals( false );

    m_value_buffers.resize( numberOfVector );
    for( int i = 0; i < m_value_buffers.size(); ++i )
    {
        m_value_buffers[i].clear();
    }
    m_time_buffer.clear();

    m_need_rebuild = true;
}

void PlotOverTimeEditor::onReceivePlotOverTimeParameter( const QJsonObject& payload )
{
    const auto kEnable      = QString::fromUtf8( Protocol::Key::Enable );
    const auto kCoords      = QString::fromUtf8( Protocol::Key::Coords );

    const auto kTimeStep    = QString::fromUtf8( Protocol::Key::TimeStep );
    const auto kValueOnTime = QString::fromUtf8( Protocol::Key::ValueOnTime );
    const auto kSamples     = QString::fromUtf8( Protocol::Key::Samples );

    if( payload.contains( kEnable ) )
    {
        ui->plotOverTimeGroupBox->setChecked( payload.value( kEnable ).toBool() );
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

    bool plotUpdated  = false;
    double tLatest    = 0.0;
    bool forceRebuild = false;

    // ★追加：今回の受信で「新規に追加した点」の数
    int addedCount = 0;

    auto safeVal = []( const QJsonValue& v ) -> double
    {
        const double x = v.isDouble() ? v.toDouble() : 0.0;
        return std::isfinite( x ) ? x : 0.0;
    };

    auto applyOneSample = [&]( int step, const QJsonArray& arr )
    {
        const int n = arr.size();
        if( n <= 0 ) return;

        if( m_value_buffers.size() != n )
        {
            m_value_buffers.resize( n );
            for( auto& buf : m_value_buffers ) buf.clear();
            m_time_buffer.clear();
            m_need_rebuild = true;
            forceRebuild   = true;
        }

        const double t = static_cast<double>( step );

        if( !m_time_buffer.isEmpty() && static_cast<int>( m_time_buffer.back() ) == step )
        {
            // 上書き（追加ではない）
            for( int i = 0; i < n; ++i )
            {
                if( !m_value_buffers[i].isEmpty() )
                    m_value_buffers[i].back() = safeVal( arr[i] );
            }
        }
        else
        {
            // 穴埋めが走ったら描画は必ず再構築
            if( !m_time_buffer.isEmpty() )
            {
                const int lastStep = static_cast<int>( m_time_buffer.back() );
                if( step > lastStep + 1 )
                {
                    for( int s = lastStep + 1; s < step; ++s )
                    {
                        m_time_buffer.push_back( static_cast<double>( s ) );
                        for( int i = 0; i < n; ++i ) m_value_buffers[i].push_back( 0.0 );
                    }
                    m_need_rebuild = true;
                    forceRebuild   = true;
                }
            }

            m_time_buffer.push_back( t );
            for( int i = 0; i < n; ++i )
                m_value_buffers[i].push_back( safeVal( arr[i] ) );

            // ★追加：新規追加したのでカウント
            ++addedCount;
        }

        // 長さ合わせ
        const int len = m_time_buffer.size();
        for( int i = 0; i < m_value_buffers.size(); ++i )
        {
            while( m_value_buffers[i].size() < len ) m_value_buffers[i].push_back( 0.0 );
            while( m_value_buffers[i].size() > len ) m_value_buffers[i].removeLast();
        }

        plotUpdated = true;
        tLatest     = t;
    };

    if( payload.contains( kSamples ) && payload.value( kSamples ).isArray() )
    {
        const QJsonArray samples = payload.value( kSamples ).toArray();

        for( const QJsonValue& v : samples )
        {
            if( !v.isObject() ) continue;

            const QJsonObject one = v.toObject();
            if( !one.contains( kTimeStep ) || !one.contains( kValueOnTime ) ) continue;

            const QJsonValue stepVal  = one.value( kTimeStep );
            const QJsonValue valueVal = one.value( kValueOnTime );

            if( !stepVal.isDouble() ) continue;
            if( !valueVal.isArray() ) continue;

            applyOneSample( stepVal.toInt(), valueVal.toArray() );
        }
    }

    if( addedCount >= 2 ) forceRebuild = true;

    if( plotUpdated )
    {
        int idx = ui->displayGraphComboBox ? ui->displayGraphComboBox->currentIndex() : 0;
        redrawPlotFromBuffers( idx, tLatest, /*forceRebuild=*/(forceRebuild || m_need_rebuild) );
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
    return s;
}

void PlotOverTimeEditor::redrawPlotFromBuffers( int index, double tLatest, bool forceRebuild )
{
    if( !m_q_custom_plot ) return;
    if( m_q_custom_plot->graphCount() < 2 ) return;
    if( m_time_buffer.isEmpty() ) return;
    if( m_value_buffers.isEmpty() ) return;

    const int n = m_value_buffers.size();
    if( n <= 0 ) return;

    if( index < 0 ) index = 0;
    if( index >= n ) index = n - 1;

    const QVector<double>& yBuf = m_value_buffers[index];

    auto gWindow  = m_q_custom_plot->graph(0);
    auto gHistory = m_q_custom_plot->graph(1);

    constexpr double windowSteps = 10.0;

    const bool rebuild = forceRebuild || m_need_rebuild;

    if( rebuild )
    {
        gWindow->data()->clear();
        gHistory->data()->clear();

        gHistory->setData( m_time_buffer, yBuf );

        double viewWidth = m_q_custom_plot->xAxis->range().size();
        if( !( viewWidth > 0.0 ) ) viewWidth = windowSteps;

        const double tMin = tLatest - viewWidth;

        QVector<double> tWin;
        QVector<double> yWin;
        tWin.reserve( m_time_buffer.size() );
        yWin.reserve( m_time_buffer.size() );

        for( int i = 0; i < m_time_buffer.size(); ++i )
        {
            if( m_time_buffer[i] >= tMin )
            {
                tWin.push_back( m_time_buffer[i] );
                yWin.push_back( yBuf[i] );
            }
        }

        gWindow->setData( tWin, yWin );

        m_need_rebuild = false;
    }
    else
    {
        const double yLatest = yBuf.back();
        gHistory->addData( tLatest, yLatest );
        gWindow->addData( tLatest, yLatest );
        gWindow->data()->removeBefore( tLatest - windowSteps );
    }

    if( m_follow_right_edge && !m_user_interacting )
    {
        m_q_custom_plot->xAxis->setRange( tLatest, windowSteps, Qt::AlignRight );
    }

    m_q_custom_plot->replot( QCustomPlot::rpQueuedReplot );
}

void PlotOverTimeEditor::calculateInitialTranslation()
{
    kvs::Vec3 tmp = ( m_screen->scene()->objectManager()->xform().inverse() * m_point_object->xform() ).translation();
    m_point_initial_translation = kvs::Vec3( tmp.x(), tmp.y(), tmp.z() );
}

void PlotOverTimeEditor::onDisplayGraphComboBoxChanged( int index )
{
    if( m_time_buffer.isEmpty() ) return;

    const double tLatest = m_time_buffer.back();
    redrawPlotFromBuffers( index, tLatest, /*forceRebuild=*/true );
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
    const auto kCoords     = QString::fromUtf8( Protocol::Key::Coords );

    const PlotOverTimeUiSnapshot now = captureUiSnapshot();
    QJsonObject diff;

    // Enable
    if( now.enable != m_last_snap_shot.enable )
        diff.insert( kEnable, now.enable );

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
