#include "PlotOverLineEditor.h"
#include "ui_PlotOverLineEditor.h"

PlotOverLineEditor::PlotOverLineEditor( kvs::qt::jaea::Screen* screen, WebSocketPair* websockets, QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::PlotOverLineEditor )
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
    m_start_point_object = new kvs::PointObject( kvs::ValueArray<kvs::Real32>{ 0.0f, 0.0f, 0.0f } );
    m_start_point_object->setName( "StartPointObject" );
    m_start_point_object->setXform( initializeXform );
    m_start_point_object->setColor( kvs::RGBColor::Red() );
    m_start_point_object->setSize( 20 );
    kvs::StochasticPointRenderer* startPointRenderer = new kvs::StochasticPointRenderer();
    m_screen->registerObject( m_start_point_object, startPointRenderer );

    m_end_point_object = new kvs::PointObject( kvs::ValueArray<kvs::Real32>{ 0.0f, 0.0f, 0.0f } );
    m_end_point_object->setName( "EndPointObject" );
    m_end_point_object->setXform( initializeXform );
    m_end_point_object->setColor( kvs::RGBColor::Blue() );
    m_end_point_object->setSize( 20 );
    kvs::StochasticPointRenderer* endPointRenderer = new kvs::StochasticPointRenderer();
    m_screen->registerObject( m_end_point_object, endPointRenderer );

    const kvs::ValueArray<kvs::Real32> coords( k_initial_coords, k_N * 3 );
    const kvs::ValueArray<kvs::UInt8>  colors( k_initial_colors, k_N * 3 );
    const kvs::ValueArray<kvs::UInt32> connections( k_initial_connections, k_L * 2 );

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

    kvs::StochasticLineRenderer* lineRenderer = new kvs::StochasticLineRenderer();
    m_plot_over_line_object_id = m_screen->registerObject( m_plot_over_line_object, lineRenderer );

#ifdef OPENXR_SCREEN
    m_screen->openxrInteractor()->setStartPoint( m_start_point_object );
    m_screen->openxrInteractor()->setEndPoint( m_end_point_object );
#else
    m_start_point_object->hide();
    m_end_point_object->hide();
#endif

    calculateInitialTranslation();

    connect( ui->resetPlotViewPushButton       , &QPushButton::clicked        , this, &PlotOverLineEditor::onResetPlotView ) ;
    connect( ui->plotOverLineGroupBox          , &QGroupBox::toggled          , this, &PlotOverLineEditor::onPlotOverLineGroupBoxCheckBox );

    connect( ui->startTranslationXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onStartTranslationValueChanged );
    connect( ui->startTranslationYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onStartTranslationValueChanged );
    connect( ui->startTranslationZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onStartTranslationValueChanged );
    connect( ui->startCoordsXDoubleSpinBox     , &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onStartCoordsValueChanged );
    connect( ui->startCoordsYDoubleSpinBox     , &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onStartCoordsValueChanged );
    connect( ui->startCoordsZDoubleSpinBox     , &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onStartCoordsValueChanged );

    connect( ui->endTranslationXDoubleSpinBox  , &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onEndTranslationValueChanged );
    connect( ui->endTranslationYDoubleSpinBox  , &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onEndTranslationValueChanged );
    connect( ui->endTranslationZDoubleSpinBox  , &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onEndTranslationValueChanged );
    connect( ui->endCoordsXDoubleSpinBox       , &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onEndCoordsValueChanged );
    connect( ui->endCoordsYDoubleSpinBox       , &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onEndCoordsValueChanged );
    connect( ui->endCoordsZDoubleSpinBox       , &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::onEndCoordsValueChanged );

    connect( ui->createLinePushButton          , &QPushButton::clicked        , this, &PlotOverLineEditor::onCreateLine );
    connect( ui->applyPushButton               , &QPushButton::clicked        , this, &PlotOverLineEditor::onApply );
}

PlotOverLineEditor::~PlotOverLineEditor()
{
    delete ui;
}

void PlotOverLineEditor::reset()
{
    m_has_last_snap_shot = false;

    ui->plotOverLineGroupBox     ->setEnabled( false );
    ui->startCoordsXDoubleSpinBox->setValue( 0.0 );
    ui->startCoordsYDoubleSpinBox->setValue( 0.0 );
    ui->startCoordsZDoubleSpinBox->setValue( 0.0 );
    ui->endCoordsXDoubleSpinBox  ->setValue( 0.0 );
    ui->endCoordsYDoubleSpinBox  ->setValue( 0.0 );
    ui->endCoordsZDoubleSpinBox  ->setValue( 0.0 );
}

void PlotOverLineEditor::onOperatorStateUpdate( const bool operatorState )
{
    m_is_operator = operatorState;

    ui->plotOverLineGroupBox          ->setEnabled( m_is_operator );
    ui->resolutionSpinBox             ->setEnabled( m_is_operator );
    ui->targetComboBox                ->setEnabled( m_is_operator );

    ui->startTranslationXDoubleSpinBox->setEnabled( m_is_operator );
    ui->startTranslationYDoubleSpinBox->setEnabled( m_is_operator );
    ui->startTranslationZDoubleSpinBox->setEnabled( m_is_operator );
    ui->startCoordsXDoubleSpinBox     ->setEnabled( m_is_operator );
    ui->startCoordsYDoubleSpinBox     ->setEnabled( m_is_operator );
    ui->startCoordsZDoubleSpinBox     ->setEnabled( m_is_operator );

    ui->endTranslationXDoubleSpinBox  ->setEnabled( m_is_operator );
    ui->endTranslationYDoubleSpinBox  ->setEnabled( m_is_operator );
    ui->endTranslationZDoubleSpinBox  ->setEnabled( m_is_operator );
    ui->endCoordsXDoubleSpinBox       ->setEnabled( m_is_operator );
    ui->endCoordsYDoubleSpinBox       ->setEnabled( m_is_operator );
    ui->endCoordsZDoubleSpinBox       ->setEnabled( m_is_operator );
}

void PlotOverLineEditor::onUpdateNumberOfVector( const int numberOfVector )
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

// NOTE:フォーカス対象のオブジェクトが変更された場合に呼び出される。
void PlotOverLineEditor::onUpdateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords )
{
    if( m_start_point_object && m_end_point_object && m_plot_over_line_object )
    {
        m_start_point_object->setMinMaxObjectCoords( resultMinObjectCoords, resultMaxObjectCoords );
        m_start_point_object->setMinMaxExternalCoords( resultMinObjectCoords, resultMaxObjectCoords );

        m_end_point_object->setMinMaxObjectCoords( resultMinObjectCoords, resultMaxObjectCoords );
        m_end_point_object->setMinMaxExternalCoords( resultMinObjectCoords, resultMaxObjectCoords );

        m_plot_over_line_object->setMinMaxObjectCoords( resultMinObjectCoords, resultMaxObjectCoords );
        m_plot_over_line_object->setMinMaxExternalCoords( resultMinObjectCoords, resultMaxObjectCoords );
    }
}

// NOTE:フォーカス対象のオブジェクトが変更された場合に呼び出される。
void PlotOverLineEditor::onUpdateTranslation()
{
    onStartCoordsValueChanged();
    onEndCoordsValueChanged();
}

void PlotOverLineEditor::onReceivePlotOverLineParameter( const QJsonObject& payload )
{
    const auto kEnable      = QString::fromUtf8( Protocol::Key::Enable );
    const auto kResolution  = QString::fromUtf8( Protocol::Key::Resolution );
    const auto kTarget      = QString::fromUtf8( Protocol::Key::Target );
    const auto kStartCoords = QString::fromUtf8( Protocol::Key::StartCoords );
    const auto kEndCoords   = QString::fromUtf8( Protocol::Key::EndCoords );

    const auto kValue = QString::fromUtf8( Protocol::Key::ValueOnLine );
    const auto kXAxis = QString::fromUtf8( Protocol::Key::XAxis );
    const auto kMask  = QString::fromUtf8( Protocol::Key::Mask );

    const bool uiParameterUpdated =
        payload.contains( kEnable ) ||
        payload.contains( kResolution ) ||
        payload.contains( kTarget ) ||
        payload.contains( kStartCoords ) ||
        payload.contains( kEndCoords );

    if( payload.contains( kEnable ) )
    {
        ui->plotOverLineGroupBox->setChecked( payload.value( kEnable ).toBool() );
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

    if( payload.contains( kStartCoords ) )
    {
        const QJsonArray startCoords = payload.value( kStartCoords ).toArray();
        if( startCoords.size() == 3 )
        {
            ui->startCoordsXDoubleSpinBox->setValue( startCoords.at( 0 ).toDouble() );
            ui->startCoordsYDoubleSpinBox->setValue( startCoords.at( 1 ).toDouble() );
            ui->startCoordsZDoubleSpinBox->setValue( startCoords.at( 2 ).toDouble() );
        }
    }

    if( payload.contains( kEndCoords ) )
    {
        const QJsonArray endCoords = payload.value( kEndCoords ).toArray();
        if( endCoords.size() == 3 )
        {
            ui->endCoordsXDoubleSpinBox->setValue( endCoords.at( 0 ).toDouble() );
            ui->endCoordsYDoubleSpinBox->setValue( endCoords.at( 1 ).toDouble() );
            ui->endCoordsZDoubleSpinBox->setValue( endCoords.at( 2 ).toDouble() );
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

    if( uiParameterUpdated )
    {
        m_last_snap_shot     = captureUiSnapshot();
        m_has_last_snap_shot = true;
    }

    onCreateLine();
}

void PlotOverLineEditor::onDrawVRPlotOverLine( kvs::Real32 coordArray[ 2 * 3 ] )
{
    if( !m_web_sockets->isConnected() ) return;

    ui->startCoordsXDoubleSpinBox->setValue( coordArray[0] );
    ui->startCoordsYDoubleSpinBox->setValue( coordArray[1] );
    ui->startCoordsZDoubleSpinBox->setValue( coordArray[2] );
    ui->endCoordsXDoubleSpinBox  ->setValue( coordArray[3] );
    ui->endCoordsYDoubleSpinBox  ->setValue( coordArray[4] );
    ui->endCoordsZDoubleSpinBox  ->setValue( coordArray[5] );
    onApply();
}

void PlotOverLineEditor::onToggleShowHideVRPlotOverLine()
{
    if( !m_web_sockets->isConnected() ) return;
    ui->plotOverLineGroupBox->setChecked( !ui->plotOverLineGroupBox->isChecked() );
    onApply();
}

void PlotOverLineEditor::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void PlotOverLineEditor::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

bool PlotOverLineEditor::fuzzyEqual( double a, double b )
{
    const double diff  = qAbs( a - b );
    const double scale = qMax( 1.0, qMax( qAbs( a ), qAbs( b ) ) );
    return diff <= 1e-12 * scale;
}

bool PlotOverLineEditor::sameVec3( const double a[3], const double b[3] )
{
    return fuzzyEqual( a[0], b[0] ) && fuzzyEqual( a[1], b[1] ) && fuzzyEqual( a[2], b[2] );
}

PlotOverLineEditor::PlotOverLineUiSnapshot PlotOverLineEditor::captureUiSnapshot() const
{
    PlotOverLineUiSnapshot s;
    s.enable     = ui->plotOverLineGroupBox->isChecked();
    s.resolution = ui->resolutionSpinBox->value();
    s.target     = ui->targetComboBox->currentIndex();

    s.start[0] = ui->startCoordsXDoubleSpinBox->value();
    s.start[1] = ui->startCoordsYDoubleSpinBox->value();
    s.start[2] = ui->startCoordsZDoubleSpinBox->value();

    s.end[0] = ui->endCoordsXDoubleSpinBox->value();
    s.end[1] = ui->endCoordsYDoubleSpinBox->value();
    s.end[2] = ui->endCoordsZDoubleSpinBox->value();

    return s;
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
    ui->startTranslationXDoubleSpinBox->setValue( m_start_point_initial_translation.x() );
    ui->startTranslationYDoubleSpinBox->setValue( m_start_point_initial_translation.y() );
    ui->startTranslationZDoubleSpinBox->setValue( m_start_point_initial_translation.z() );
    ui->endTranslationXDoubleSpinBox  ->setValue( m_end_point_initial_translation.x() );
    ui->endTranslationYDoubleSpinBox  ->setValue( m_end_point_initial_translation.y() );
    ui->endTranslationZDoubleSpinBox  ->setValue( m_end_point_initial_translation.z() );
}

void PlotOverLineEditor::updateTranslationSpinBox( kvs::PointObject* point,
                                                     QDoubleSpinBox* xTranslationSpinBox, QDoubleSpinBox* yTranslationSpinBox, QDoubleSpinBox* zTranslationSpinBox,
                                                     QDoubleSpinBox* xCoordsSpinBox     , QDoubleSpinBox* yCoordsSpinBox     , QDoubleSpinBox* zCoordsSpinBox,
                                                     const kvs::Vec3& initialTranslation )
{
    if( point != nullptr )
    {
        kvs::Xform currentObjectManagerXform = m_screen->scene()->objectManager()->xform(); // NOTE:ObjectManagerのxformを取得
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

        kvs::Xform rotation    = kvs::Xform::Rotation( point->xform().rotation() );
        kvs::Xform scaling     = kvs::Xform::Scaling( point->xform().scaling() );
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

        { // NOTE:ObjectEditorとの連携時に必要
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
                                                QDoubleSpinBox* xTranslationSpinBox, QDoubleSpinBox* yTranslationSpinBox, QDoubleSpinBox* zTranslationSpinBox,
                                                QDoubleSpinBox* xCoordsSpinBox     , QDoubleSpinBox* yCoordsSpinBox     , QDoubleSpinBox* zCoordsSpinBox,
                                                const kvs::Vec3& initialTranslation )
{
    if( point != nullptr )
    {
        kvs::Xform currentObjectManagerXform = m_screen->scene()->objectManager()->xform(); // NOTE:ObjectManagerのxformを取得
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

        kvs::Xform rotation    = kvs::Xform::Rotation( point->xform().rotation() );
        kvs::Xform scaling     = kvs::Xform::Scaling( point->xform().scaling() );
        kvs::Xform translation = kvs::Xform::Translation( kvs::Vec3( finalX, finalY, finalZ ) );

        kvs::Xform newXform = rotation * scaling * translation;
        point->setXform( newXform );

        kvs::Mat3 inv_rotation       = currentObjectManagerXform.rotation().transposed();
        kvs::Vec3 translation1       = newXform.translation();
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

void PlotOverLineEditor::setPlotData( std::vector<float> xAxis, std::vector<bool> mask, std::vector<float> values )
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
    for( std::size_t i = 0; i < x.size(); i++ )
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
    for( std::size_t i = 0; i < x.size(); i++ )
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


void PlotOverLineEditor::onResetPlotView()
{
    m_q_custom_plot->xAxis->setRange( m_x_min, m_x_max ); // x 軸の範囲
    m_q_custom_plot->yAxis->setRange( m_y_min, m_y_max ); // y 軸の範囲
    m_q_custom_plot->replot();
}

void PlotOverLineEditor::onPlotOverLineGroupBoxCheckBox()
{
    if( !m_start_point_object || !m_end_point_object ) { return; }

    const bool visible = ui->plotOverLineGroupBox->isChecked();

#ifndef OPENXR_SCREEN
    m_start_point_object->setVisible( visible );
    m_end_point_object->setVisible( visible );
#endif

    if( m_plot_over_line_object )
    {
        m_plot_over_line_object->setVisible( visible );
    }

    m_screen->update();
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
    kvs::Real32 CoordArray[ k_N * 3 ] =
        {
            kvs::Real32( ui->startCoordsXDoubleSpinBox->value() ), kvs::Real32(ui->startCoordsYDoubleSpinBox->value() ), kvs::Real32(ui->startCoordsZDoubleSpinBox->value() ),
            kvs::Real32( ui->endCoordsXDoubleSpinBox->value() )  , kvs::Real32(ui->endCoordsYDoubleSpinBox->value() )  , kvs::Real32(ui->endCoordsZDoubleSpinBox->value() )
        };

    const kvs::ValueArray<kvs::Real32> coords( CoordArray, k_N * 3 );
    const kvs::ValueArray<kvs::UInt8> colors( k_initial_colors, k_N * 3 );
    const kvs::ValueArray<kvs::UInt32> connections( k_initial_connections, k_L * 2 );
    if( m_start_point_object->minObjectCoord() != m_start_point_object->maxObjectCoord() )
    {
        if( m_plot_over_line_object )
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
    if( !m_web_sockets->isConnected() ) return;

    onCreateLine();

    if( !m_has_last_snap_shot )
    {
        m_last_snap_shot     = captureUiSnapshot();
        m_has_last_snap_shot = true;
        return;
    }

    const auto kEvent       = QString::fromUtf8( Protocol::Key::Event );
    const auto kEnable      = QString::fromUtf8( Protocol::Key::Enable );
    const auto kResolution  = QString::fromUtf8( Protocol::Key::Resolution );
    const auto kTarget      = QString::fromUtf8( Protocol::Key::Target );
    const auto kStartCoords = QString::fromUtf8( Protocol::Key::StartCoords );
    const auto kEndCoords   = QString::fromUtf8( Protocol::Key::EndCoords );

    const PlotOverLineUiSnapshot now = captureUiSnapshot();
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

    // Start Coords
    if( !sameVec3( now.start, m_last_snap_shot.start ) )
    {
        QJsonArray a;
        a.append( now.start[0] );
        a.append( now.start[1] );
        a.append( now.start[2] );
        diff.insert( kStartCoords, a );
    }

    // End Coords
    if( !sameVec3( now.end, m_last_snap_shot.end ) )
    {
        QJsonArray a;
        a.append( now.end[0] );
        a.append( now.end[1] );
        a.append( now.end[2] );
        diff.insert( kEndCoords, a );
    }

    if( diff.isEmpty() ) return;

    QJsonObject plotOverLineParameter;
    plotOverLineParameter.insert( kEvent, QString::fromUtf8( Protocol::Events::PlotOverLineParameter ) );

    for( auto it = diff.begin(); it != diff.end(); ++it )
        plotOverLineParameter.insert( it.key(), it.value() );

    m_web_sockets->text()->sendTextMessage( QJsonDocument( plotOverLineParameter ).toJson( QJsonDocument::Compact ) );
    emit plotOverLineParameterUpdate();

    m_last_snap_shot = now;
    m_has_last_snap_shot = true;

    // qDebug().noquote() << "[PlotOverLineEditor::onApply] diff(pretty):\n" << QJsonDocument( plotOverLineParameter ).toJson( QJsonDocument::Indented );
}
