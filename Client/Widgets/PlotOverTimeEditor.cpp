#include "PlotOverTimeEditor.h"
#include "ui_PlotOverTimeEditor.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <QUuid>
#include <QSignalBlocker>

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
    m_calculation_status = new QLabel( tr("POT: Idle"), this );
    m_calculation_status->setWordWrap( true );
    ui->gridLayout_4->addWidget( m_calculation_status, 5, 0 );

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

    // NOTE:観測点のマーカーを予め登録しておく
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
                         m_auto_fit = false;
                     } );

    connect( m_q_custom_plot, &QCustomPlot::mouseRelease,
                     this, [&]( QMouseEvent* )
                     {
                         m_user_interacting = false;
                     } );

    connect( m_q_custom_plot, &QCustomPlot::mouseWheel,
                     this, [&]( QWheelEvent* )
                     {
                         m_auto_fit = false;
                     } );

    connect( m_q_custom_plot, &QCustomPlot::mouseDoubleClick,
                     this, [&]( QMouseEvent* )
                     {
                         m_auto_fit = true;
                     } );

    if( m_combo_connection ) disconnect( m_combo_connection );

    m_combo_connection = connect(
        ui->displayGraphComboBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &PlotOverTimeEditor::onDisplayGraphComboBoxChanged
        );


    connect( ui->resetPlotViewPushButton, &QPushButton::clicked, this, &PlotOverTimeEditor::onResetPlotView ) ;
    connect( ui->plotOverTimeCheckBox   , &QCheckBox::toggled  , this, &PlotOverTimeEditor::onPlotOverTimeCheckBox );
    connect( ui->applyPushButton        , &QPushButton::clicked, this, &PlotOverTimeEditor::onApply );

    connect( ui->pointTranslationXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverTimeEditor::onPointTranslationValueChanged );
    connect( ui->pointTranslationYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverTimeEditor::onPointTranslationValueChanged );
    connect( ui->pointTranslationZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverTimeEditor::onPointTranslationValueChanged );
    connect( ui->pointCoordsXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverTimeEditor::onPointCoordsValueChanged );
    connect( ui->pointCoordsYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverTimeEditor::onPointCoordsValueChanged );
    connect( ui->pointCoordsZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverTimeEditor::onPointCoordsValueChanged );

    onOperatorStateUpdate( m_is_operator );
    setPointCoords( 0.0, 0.0, 0.0 );
    m_plot_inited = true;
}

PlotOverTimeEditor::~PlotOverTimeEditor()
{
    delete ui;
}

void PlotOverTimeEditor::reset()
{
    m_applied_observation = QJsonObject();
    m_applied_point = QJsonArray();
    m_pending_apply.clear();
    m_calculation_state = "idle";
    clearPlot();
    m_auto_fit = true;
    updateCalculationStatus();

    onOperatorStateUpdate( false );
    setPointCoords( 0.0, 0.0, 0.0 );
}

void PlotOverTimeEditor::onOperatorStateUpdate( const bool operatorState )
{
    m_is_operator = operatorState;

    ui->plotOverTimeCheckBox->setEnabled( m_is_operator );
    ui->pointGroupBox->setEnabled( m_is_operator );
    ui->applyPushButton->setEnabled( m_is_operator );
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

    // Metadata may arrive after the reconnect snapshot; retain received samples.
    if ( !m_samples.isEmpty() ) redrawPlotFromBuffers( 0 );

}

// NOTE:フォーカス対象のオブジェクトが変更された場合に呼び出される。
void PlotOverTimeEditor::onUpdateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords )
{
    if( m_point_object )
    {
        m_point_object->setMinMaxObjectCoords( resultMinObjectCoords, resultMaxObjectCoords );
        m_point_object->setMinMaxExternalCoords( resultMinObjectCoords, resultMaxObjectCoords );
        onUpdateTranslation();
    }
}

// NOTE:フォーカス対象のオブジェクトが変更された場合に呼び出される。
void PlotOverTimeEditor::onUpdateTranslation()
{
    if( m_point_object != nullptr )
    {
        kvs::Xform currentObjectManagerXform = m_screen->scene()->objectManager()->xform(); // NOTE:ObjectManagerのxformを取得
        float scalingFactor = 1 / ( currentObjectManagerXform.inverse() * m_point_object->xform() ).scaling().x();

        float finalX = ( m_x_coord - m_point_object->externalCenter().x() ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][0] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][0] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][0] )
                            ) * scalingFactor
                        ) +
                       ( m_point_initial_translation.x() * scalingFactor );

        float finalY = ( m_y_coord - m_point_object->externalCenter().y() ) +
                       ( (
                            ( ( currentObjectManagerXform.translation().x() * currentObjectManagerXform.inverse().scaling().x() ) * currentObjectManagerXform.rotation()[0][1] ) +
                            ( ( currentObjectManagerXform.translation().y() * currentObjectManagerXform.inverse().scaling().y() ) * currentObjectManagerXform.rotation()[1][1] ) +
                            ( ( currentObjectManagerXform.translation().z() * currentObjectManagerXform.inverse().scaling().z() ) * currentObjectManagerXform.rotation()[2][1] )
                            ) * scalingFactor
                        ) +
                       ( m_point_initial_translation.y() * scalingFactor );

        float finalZ = ( m_z_coord - m_point_object->externalCenter().z() ) +
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

        const QSignalBlocker blockX( ui->pointTranslationXDoubleSpinBox );
        const QSignalBlocker blockY( ui->pointTranslationYDoubleSpinBox );
        const QSignalBlocker blockZ( ui->pointTranslationZDoubleSpinBox );
        ui->pointTranslationXDoubleSpinBox->setValue(
            corrected_position.x() * currentObjectManagerXform.inverse().scaling().x()
            + currentObjectManagerXform.inverse().translation().x() );
        ui->pointTranslationYDoubleSpinBox->setValue(
            corrected_position.y() * currentObjectManagerXform.inverse().scaling().y()
            + currentObjectManagerXform.inverse().translation().y() );
        ui->pointTranslationZDoubleSpinBox->setValue(
            corrected_position.z() * currentObjectManagerXform.inverse().scaling().z()
            + currentObjectManagerXform.inverse().translation().z() );

        m_screen->update();
    }
}

void PlotOverTimeEditor::setPointCoords( double x, double y, double z )
{
    if ( !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ) return;

    // Preserve received/VR precision; spin boxes only round the displayed values.
    m_x_coord = x;
    m_y_coord = y;
    m_z_coord = z;
    const QSignalBlocker blockX( ui->pointCoordsXDoubleSpinBox );
    const QSignalBlocker blockY( ui->pointCoordsYDoubleSpinBox );
    const QSignalBlocker blockZ( ui->pointCoordsZDoubleSpinBox );
    ui->pointCoordsXDoubleSpinBox->setValue( x );
    ui->pointCoordsYDoubleSpinBox->setValue( y );
    ui->pointCoordsZDoubleSpinBox->setValue( z );
    onUpdateTranslation();
}

void PlotOverTimeEditor::onPointCoordsValueChanged()
{
    setPointCoords( ui->pointCoordsXDoubleSpinBox->value(),
                    ui->pointCoordsYDoubleSpinBox->value(),
                    ui->pointCoordsZDoubleSpinBox->value() );
}

void PlotOverTimeEditor::onPointTranslationValueChanged()
{
    if ( !m_point_object ) return;

    // Use the same Translation-to-Coords conversion as Plot Over Line.
    const kvs::Xform manager = m_screen->scene()->objectManager()->xform();
    const kvs::Xform inverse = manager.inverse();
    const float scalingFactor = 1 / ( inverse * m_point_object->xform() ).scaling().x();
    const double translationValues[3] = {
        ui->pointTranslationXDoubleSpinBox->value(),
        ui->pointTranslationYDoubleSpinBox->value(),
        ui->pointTranslationZDoubleSpinBox->value()
    };
    kvs::Vec3 position;
    for ( int axis = 0; axis < 3; ++axis )
    {
        double offset = 0.0;
        for ( int component = 0; component < 3; ++component )
        {
            offset += manager.translation()[component] * inverse.scaling()[component]
                      * manager.rotation()[component][axis];
        }
        position[axis] = ( translationValues[axis] + offset ) * scalingFactor;
    }

    const kvs::Xform transform =
        kvs::Xform::Rotation( m_point_object->xform().rotation() ) *
        kvs::Xform::Scaling( m_point_object->xform().scaling() ) *
        kvs::Xform::Translation( position );
    const kvs::Vec3 relativeTranslation = ( inverse * transform ).translation();
    double coords[3];
    QDoubleSpinBox* coordInputs[3] = {
        ui->pointCoordsXDoubleSpinBox,
        ui->pointCoordsYDoubleSpinBox,
        ui->pointCoordsZDoubleSpinBox
    };
    for ( int axis = 0; axis < 3; ++axis )
    {
        const double movement = m_point_initial_translation[axis] - relativeTranslation[axis]
                                - manager.translation()[axis] * inverse.scaling()[axis];
        const double coordinate = -movement * scalingFactor
                                  - manager.translation()[axis] * scalingFactor * inverse.scaling()[axis]
                                  + m_point_object->externalCenter()[axis];
        // Match the Coords input range before moving the marker.
        coords[axis] = std::max( coordInputs[axis]->minimum(),
                               std::min( coordinate, coordInputs[axis]->maximum() ) );
    }
    setPointCoords( coords[0], coords[1], coords[2] );
}

void PlotOverTimeEditor::onReceivePlotOverTimeParameter( const QJsonObject& payload )
{
    const auto key = []( std::string_view value ) { return QString::fromUtf8(value); };
    const auto kCoords = key(Protocol::Key::Coords);
    const auto kApplied = key(Protocol::Key::AppliedObservation);
    const auto kObservation = key(Protocol::Key::Observation);
    const auto kSamples = key(Protocol::Key::Samples);
    const bool ownAcknowledgement = !m_pending_apply.isEmpty() &&
        payload.value(key(Protocol::Key::ApplyId)).toString() == m_pending_apply;

    if ( payload.contains(kApplied) )
    {
        const auto applied = payload.value(kApplied).toObject();
        const auto datasetKey = key(Protocol::Key::DatasetId);
        const bool newDataset = applied.value(datasetKey) != m_applied_observation.value(datasetKey);
        // Earlier Apply acknowledgements must not undo a more recent local Apply.
        if ( !m_pending_apply.isEmpty() && !ownAcknowledgement && !newDataset ) return;
        if ( applied != m_applied_observation ) clearPlot();
        m_applied_observation = applied;
        m_applied_point = applied.value(kCoords).toArray();
        if ( ownAcknowledgement || newDataset ) m_pending_apply.clear();
    }
    else if ( payload.contains(kCoords) )
    {
        m_calculation_state = "idle";
        // Legacy IS settings have no CS observation metadata.
        const auto point = payload.value(kCoords).toArray();
        // Retain IS history across observation point changes.
        m_applied_observation = QJsonObject();
        m_applied_point = point;
        m_pending_apply.clear();
    }

    // An acknowledgement updates applied state, without overwriting draft edits
    // made while it was in flight. Remote settings still update the marker.
    if ( !ownAcknowledgement )
    {
        if ( payload.contains(key(Protocol::Key::Enable)) )
            ui->plotOverTimeCheckBox->setChecked( payload.value(key(Protocol::Key::Enable)).toBool() );
        if ( payload.contains(kCoords) )
        {
            const auto coords = payload.value(kCoords).toArray();
            if ( coords.size() == 3 )
            {
                setPointCoords( coords[0].toDouble(), coords[1].toDouble(), coords[2].toDouble() );
            }
        }
    }

    if ( payload.contains(kObservation) )
    {
        // Use the actual calculation condition, never the current marker position.
        if ( payload.value(kObservation).toObject() != m_applied_observation ) return;
    }
    else if ( !m_applied_observation.isEmpty() && payload.contains(kSamples) ) return;

    if ( payload.value(key(Protocol::Key::Snapshot)).toBool() ) clearPlot();
    if ( payload.contains(key(Protocol::Key::CalculationState)) )
        m_calculation_state = payload.value(key(Protocol::Key::CalculationState)).toString();

    if ( m_applied_observation.isEmpty() && !payload.value(kSamples).toArray().isEmpty() )
        m_calculation_state = "receiving";
    const double missing = std::numeric_limits<double>::quiet_NaN();
    for ( const auto& value : payload.value(kSamples).toArray() )
    {
        if ( !value.isObject() ) continue;
        const auto sample = value.toObject();
        const auto stepValue = sample.value(key(Protocol::Key::TimeStep));
        const auto values = sample.value(key(Protocol::Key::ValueOnTime));
        if ( !stepValue.isDouble() || !values.isArray() ) continue;
        const double stepNumber = stepValue.toDouble();
        if ( !std::isfinite(stepNumber) || std::floor(stepNumber) != stepNumber ||
             stepNumber < std::numeric_limits<int>::min() || stepNumber > std::numeric_limits<int>::max() ) continue;
        const int step = static_cast<int>(stepNumber);
        const QString state = sample.value(key(Protocol::Key::SampleState)).toString();
        const bool valid = sample.value(key(Protocol::Key::Mask)).toBool(true) &&
            (state.isEmpty() || state == "valid");
        QVector<double> row;
        for ( const auto& item : values.toArray() )
        {
            const double v = item.isDouble() ? item.toDouble() : missing;
            row.push_back( valid && std::isfinite(v) ? v : missing );
        }
        m_samples[step] = row; // Sorted replacement also handles retries/reconnects.
        m_sample_states[step] = state;
    }
    if ( !m_samples.isEmpty() )
        redrawPlotFromBuffers( ui->displayGraphComboBox->currentIndex() );
    updateCalculationStatus();
}

void PlotOverTimeEditor::clearPlot()
{
    m_samples.clear();
    m_sample_states.clear();
    m_time_buffer.clear();
    m_value_buffers.clear();
    // Replacing samples must preserve a manually chosen view.
    for ( int i = 0; i < m_q_custom_plot->graphCount(); ++i )
        m_q_custom_plot->graph(i)->data()->clear();
    m_q_custom_plot->replot( QCustomPlot::rpQueuedReplot );
}

void PlotOverTimeEditor::updateCalculationStatus()
{
    int outside = 0, failed = 0;
    for ( const auto& state : m_sample_states )
    {
        if ( state == "outside_domain" ) ++outside;
        if ( state == "failed" ) ++failed;
    }
    QString state;
    if ( m_calculation_state == "computing" ) state = tr("Computing");
    else if ( m_calculation_state == "complete" ) state = tr("Complete");
    else if ( m_calculation_state == "failed" ) state = tr("Failed (retry on next update)");
    else if ( m_calculation_state == "pending" ) state = tr("Waiting for next time step");
    else if ( m_calculation_state == "receiving" ) state = tr("Receiving IS samples");
    else state = tr("Idle");
    m_calculation_status->setText( tr("POT: %1 | %2 steps | Outside: %3 | Failed: %4")
        .arg(state).arg(m_samples.size()).arg(outside).arg(failed) );
}

void PlotOverTimeEditor::onDrawVRPlotOverTime( kvs::Real32 coordArray[ 2 * 3 ] )
{
    if( !m_web_sockets->isConnected() ) return;

    setPointCoords( coordArray[0], coordArray[1], coordArray[2] );

    onApply();
}

void PlotOverTimeEditor::onToggleShowHideVRPlotOverTime()
{
    if( !m_web_sockets->isConnected() ) return;
    ui->plotOverTimeCheckBox->setChecked( !ui->plotOverTimeCheckBox->isChecked() );
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

void PlotOverTimeEditor::redrawPlotFromBuffers( int index, bool resetView )
{
    if ( !m_q_custom_plot || m_q_custom_plot->graphCount() < 2 || m_samples.isEmpty() ) return;
    int nvariables = 0;
    for ( const auto& row : m_samples ) nvariables = std::max(nvariables, static_cast<int>(row.size()));
    if ( nvariables <= 0 ) return;
    if ( ui->displayGraphComboBox->count() != nvariables )
    {
        const QSignalBlocker blocker(ui->displayGraphComboBox);
        ui->displayGraphComboBox->clear();
        for ( int i = 0; i < nvariables; ++i ) ui->displayGraphComboBox->addItem(QString("q%1").arg(i + 1));
        ui->displayGraphComboBox->setCurrentIndex(std::max(0, std::min(index, nvariables - 1)));
    }
    index = std::max(0, std::min(index, nvariables - 1));
    const double missing = std::numeric_limits<double>::quiet_NaN();
    m_time_buffer.clear();
    m_value_buffers = QVector<QVector<double>>(nvariables);
    for ( auto it = m_samples.cbegin(); it != m_samples.cend(); ++it )
    {
        // Break the line across unsent steps; never invent zero measurements.
        if ( !m_time_buffer.isEmpty() && double(it.key()) > m_time_buffer.back() + 1.0 )
        {
            m_time_buffer.push_back(m_time_buffer.back() + 1.0);
            for ( auto& buffer : m_value_buffers ) buffer.push_back(missing);
        }
        m_time_buffer.push_back(it.key());
        for ( int v = 0; v < nvariables; ++v )
            m_value_buffers[v].push_back(v < it.value().size() ? it.value()[v] : missing);
    }
    constexpr double windowSteps = 10.0;
    const double latest = m_time_buffer.back();
    const auto& values = m_value_buffers[index];
    auto* gWindow = m_q_custom_plot->graph(0);
    auto* gHistory = m_q_custom_plot->graph(1);
    // CS results carry an applied observation; legacy IS results do not.
    if ( m_applied_observation.isEmpty() )
    {
        gHistory->setData(m_time_buffer, values, true);
        QVector<double> recentTimes;
        QVector<double> recentValues;
        for ( int i = 0; i < m_time_buffer.size(); ++i )
        {
            if ( m_time_buffer[i] >= latest - windowSteps )
            {
                recentTimes.push_back(m_time_buffer[i]);
                recentValues.push_back(values[i]);
            }
        }
        gWindow->setData(recentTimes, recentValues, true);
    }
    else
    {
        // Keep all CS results in the normal line for manual navigation.
        gHistory->data()->clear();
        gWindow->setData(m_time_buffer, values, true);
    }
    if ( resetView || (m_auto_fit && !m_user_interacting) )
    {
        m_q_custom_plot->xAxis->setRange(latest, windowSteps, Qt::AlignRight);
        // Fit the selected variable over all received steps, including history.
        double low = std::numeric_limits<double>::infinity();
        double high = -low;
        for ( double value : m_value_buffers[index] )
            if ( std::isfinite(value) ) { low = std::min(low, value); high = std::max(high, value); }
        if ( std::isfinite(low) )
        {
            const double margin = high > low ? (high - low) * 0.05 : std::max(1.0, std::abs(low) * 0.05);
            m_q_custom_plot->yAxis->setRange(low - margin, high + margin);
        }
    }
    m_q_custom_plot->replot(QCustomPlot::rpQueuedReplot);
}

void PlotOverTimeEditor::calculateInitialTranslation()
{
    kvs::Vec3 tmp = ( m_screen->scene()->objectManager()->xform().inverse() * m_point_object->xform() ).translation();
    m_point_initial_translation = kvs::Vec3( tmp.x(), tmp.y(), tmp.z() );
}

void PlotOverTimeEditor::onDisplayGraphComboBoxChanged( int index )
{
    if( m_time_buffer.isEmpty() ) return;

    redrawPlotFromBuffers( index );
}

void PlotOverTimeEditor::onResetPlotView()
{
    // One-shot fit: leave automatic following enabled/disabled as it was.
    redrawPlotFromBuffers(ui->displayGraphComboBox->currentIndex(), true);
}

void PlotOverTimeEditor::onPlotOverTimeCheckBox()
{
    if( !m_point_object ) { return; }
    const bool visible = ui->plotOverTimeCheckBox->isChecked();
    m_point_object->setVisible( visible );
    m_screen->update();
}

void PlotOverTimeEditor::onApply()
{
    if ( !m_web_sockets->isConnected() ) return;
    const auto kCoords = QString::fromUtf8(Protocol::Key::Coords);
    // The interpolator uses float coordinates. Compare/send that exact condition.
    QJsonArray coords;
    coords.append(static_cast<double>(static_cast<float>(m_x_coord)));
    coords.append(static_cast<double>(static_cast<float>(m_y_coord)));
    coords.append(static_cast<double>(static_cast<float>(m_z_coord)));
    // Only CS starts a new plot when the observation point changes.
    if ( !m_applied_observation.isEmpty() && coords != m_applied_point )
    {
        clearPlot();
        m_calculation_state = ui->plotOverTimeCheckBox->isChecked() ? "pending" : "idle";
    }
    m_applied_point = coords;
    QJsonObject message;
    message.insert(QString::fromUtf8(Protocol::Key::Event), QString::fromUtf8(Protocol::Events::PlotOverTimeParameter));
    message.insert(QString::fromUtf8(Protocol::Key::Enable), ui->plotOverTimeCheckBox->isChecked());
    message.insert(kCoords, coords);
    if ( !m_applied_observation.isEmpty() )
    {
        m_applied_observation.insert(kCoords, coords);
        m_pending_apply = QUuid::createUuid().toString(QUuid::WithoutBraces);
        message.insert(QString::fromUtf8(Protocol::Key::ApplyId), m_pending_apply);
    }
    m_web_sockets->text()->sendTextMessage(QJsonDocument(message).toJson(QJsonDocument::Compact));
    updateCalculationStatus();
    // Apply only transfers settings; it never requests a time-step update.
}
