#include "PlotOverLineEditor.h"
#include "ui_PlotOverLineEditor.h"

PlotOverLineEditor::PlotOverLineEditor( QWebSocket* textSocket, kvs::qt::jaea::Screen* screen, QWidget *parent )
    : QDockWidget(parent)
    , ui(new Ui::PlotOverLineEditor)
    , m_web_text_socket( textSocket )
    , m_model( new QStandardItemModel( this ) )
    , m_q_custom_plot( new QCustomPlot( this ) )
    , m_screen( screen )
{
    initialize();
}

PlotOverLineEditor::~PlotOverLineEditor()
{
    if( m_q_custom_plot ) m_q_custom_plot->setParent( nullptr );  // ui 破棄前に親を外す
    delete ui;
}

void PlotOverLineEditor::initialize()
{
    ui->setupUi(this);
    ui->customPlotArea->addWidget( m_q_custom_plot );

    kvs::Vec3 initializeCoords( 0.0f, 0.0f, 0.0f );
    kvs::Xform initializeXform = kvs::Xform( kvs::Mat4( 1, 0, 0, 0,
                                                      0, 1, 0, 0,
                                                      0, 0, 1, 0,
                                                      0, 0, 0, 1 ) );

    m_start_point_object = new  kvs::PointObject( kvs::ValueArray<kvs::Real32>{ initializeCoords.x(), initializeCoords.y(), initializeCoords.z() } );
    m_start_point_object->setXform( initializeXform );
    m_start_point_object->setColor( kvs::RGBColor::Red() );
    m_start_point_object->setSize( 20 );
    kvs::StochasticPointRenderer* startPointRenderer = new kvs::StochasticPointRenderer();
    m_screen->registerObject( m_start_point_object, startPointRenderer );

    m_end_point_object = new  kvs::PointObject( kvs::ValueArray<kvs::Real32>{ initializeCoords.x(), initializeCoords.y(), initializeCoords.z() } );
    m_end_point_object->setXform( initializeXform );
    m_end_point_object->setColor( kvs::RGBColor::Blue() );
    m_end_point_object->setSize( 20 );
    kvs::StochasticPointRenderer* endPointRenderer = new kvs::StochasticPointRenderer();
    m_screen->registerObject( m_end_point_object, endPointRenderer );

#ifdef OPENXR_SCREEN
    m_screen->openxrInteractor()->setStartPoint( m_start_point_object );
    m_screen->openxrInteractor()->setEndPoint( m_end_point_object );
#else
    m_start_point_object->hide();
    m_end_point_object->hide();
#endif

    calculateInitialTranslation();

    connect( ui->resetPlotViewPushButton, &QPushButton::clicked, this, &PlotOverLineEditor::onResetPlotView );

    connect( ui->plotOverLineGroupBox, &QGroupBox::toggled, this, &PlotOverLineEditor::onPlotOverLineGroupBoxCheckBox );

    connect( ui->startTranslationXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::startTranslationUpdateXYZ );
    connect( ui->startTranslationYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::startTranslationUpdateXYZ );
    connect( ui->startTranslationZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::startTranslationUpdateXYZ );
    connect( ui->startCoordXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::startCoordsUpdateXYZ );
    connect( ui->startCoordYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::startCoordsUpdateXYZ );
    connect( ui->startCoordZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::startCoordsUpdateXYZ );

    connect( ui->endTranslationXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::endTranslationUpdateXYZ );
    connect( ui->endTranslationYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::endTranslationUpdateXYZ );
    connect( ui->endTranslationZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::endTranslationUpdateXYZ );
    connect( ui->endCoordXDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::startCoordsUpdateXYZ );
    connect( ui->endCoordYDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::startCoordsUpdateXYZ );
    connect( ui->endCoordZDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &PlotOverLineEditor::startCoordsUpdateXYZ );

    connect( ui->createLinePushButton , &QPushButton::clicked , this, &PlotOverLineEditor::onCreateLine );
    connect( ui->applyPushButton , &QPushButton::clicked , this, &PlotOverLineEditor::onApply );
}

void PlotOverLineEditor::calculateInitialTranslation()
{
#ifdef OPENXR_SCREEN
    kvs::Vec3 initialCoods( 0.0f, 0.0f, 0.0f );
    kvs::Xform initializeXform = kvs::Xform( kvs::Mat4( 1, 0, 0, 0,
                                                      0, 1, 0, 0,
                                                      0, 0, 1, 0,
                                                      0, 0, 0, 1 ) );
    m_start_point_object->setXform( initializeXform );
#endif

    kvs::Vec3 tmp = ( m_screen->scene()->objectManager()->xform().inverse() * m_start_point_object->xform() ).translation();
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

void PlotOverLineEditor::updateTranslationXYZ( kvs::PointObject* point,
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

void PlotOverLineEditor::updateCoordinateXYZ( kvs::PointObject* point,
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

void PlotOverLineEditor::onResetPlotView()
{
    m_q_custom_plot->xAxis->setRange(m_x_min, m_x_max); // x 軸の範囲
    m_q_custom_plot->yAxis->setRange(m_y_min, m_y_max); // y 軸の範囲
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
            if( m_screen->scene()->object( PlotOverLineObjectName ) )
            {
                m_screen->scene()->object( PlotOverLineObjectName )->show();
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
            if( m_screen->scene()->object( PlotOverLineObjectName ) )
            {
                m_screen->scene()->object( PlotOverLineObjectName )->hide();
            }
            m_screen->update();
        }
    }
}

void PlotOverLineEditor::startTranslationUpdateXYZ()
{
    updateTranslationXYZ(
        m_start_point_object,
        ui->startTranslationXDoubleSpinBox,
        ui->startTranslationYDoubleSpinBox,
        ui->startTranslationZDoubleSpinBox,
        ui->startCoordXDoubleSpinBox,
        ui->startCoordYDoubleSpinBox,
        ui->startCoordZDoubleSpinBox,
        m_start_point_initial_translation
        );
}

void PlotOverLineEditor::startCoordsUpdateXYZ()
{
    updateCoordinateXYZ(
        m_start_point_object,
        ui->startTranslationXDoubleSpinBox,
        ui->startTranslationYDoubleSpinBox,
        ui->startTranslationZDoubleSpinBox,
        ui->startCoordXDoubleSpinBox,
        ui->startCoordYDoubleSpinBox,
        ui->startCoordZDoubleSpinBox,
        m_start_point_initial_translation
        );
}

void PlotOverLineEditor::endTranslationUpdateXYZ()
{
    updateTranslationXYZ(
        m_end_point_object,
        ui->endTranslationXDoubleSpinBox,
        ui->endTranslationYDoubleSpinBox,
        ui->endTranslationZDoubleSpinBox,
        ui->endCoordXDoubleSpinBox,
        ui->endCoordYDoubleSpinBox,
        ui->endCoordZDoubleSpinBox,
        m_end_point_initial_translation
        );
}

void PlotOverLineEditor::endCoordsUpdateXYZ()
{
    updateCoordinateXYZ(
        m_end_point_object,
        ui->endTranslationXDoubleSpinBox,
        ui->endTranslationYDoubleSpinBox,
        ui->endTranslationZDoubleSpinBox,
        ui->endCoordXDoubleSpinBox,
        ui->endCoordYDoubleSpinBox,
        ui->endCoordZDoubleSpinBox,
        m_end_point_initial_translation
        );
}

void PlotOverLineEditor::onCreateLine()
{
    // 点の数
    const size_t N = 2;

    // 線分の数
    const size_t L = 1;

    // 座標定義
    kvs::Real32 CoordArray[ N * 3 ] =
        {
            kvs::Real32( ui->startCoordXDoubleSpinBox->value()), kvs::Real32(ui->startCoordYDoubleSpinBox->value()), kvs::Real32(ui->startCoordZDoubleSpinBox->value() ),
            kvs::Real32( ui->endCoordXDoubleSpinBox->value()), kvs::Real32(ui->endCoordYDoubleSpinBox->value()), kvs::Real32(ui->endCoordZDoubleSpinBox->value() )
        };

    // 色定義
    kvs::UInt8 ColorArray[ N * 3 ] =
        {
            0, 255, 0,
            0, 255, 0,
        };

    // 接続定義
    kvs::UInt32 ConnectionArray[ L * 2 ] =
        {
            0, 1,  // 点0と点1を接続
        };

    kvs::ValueArray<kvs::Real32> coords( CoordArray, N * 3 );
    kvs::ValueArray<kvs::UInt8> colors( ColorArray, N * 3 );
    kvs::ValueArray<kvs::UInt32> connections( ConnectionArray, L * 2 );

    if( m_start_point_object->minObjectCoord() != m_start_point_object->maxObjectCoord() )
    {
        if( m_screen->scene()->object( PlotOverLineObjectName ) == nullptr ) // オブジェクト登録
        {
            kvs::LineObject* plotOverLineObject = new kvs::LineObject();
            plotOverLineObject->setName( PlotOverLineObjectName );
            plotOverLineObject->setXform( m_screen->scene()->objectManager()->xform( ) );
            plotOverLineObject->setCoords( coords );
            plotOverLineObject->setColors( colors );
            plotOverLineObject->setConnections( connections );
            plotOverLineObject->setSize( 10 );
            plotOverLineObject->setLineType( kvs::LineObject::Segment );
            plotOverLineObject->setColorType( kvs::LineObject::VertexColor );
            plotOverLineObject->setMinMaxObjectCoords( m_start_point_object->minObjectCoord(), m_start_point_object->maxObjectCoord() );
            plotOverLineObject->setMinMaxExternalCoords( m_start_point_object->minExternalCoord(), m_start_point_object->maxExternalCoord() );

            kvs::StochasticLineRenderer* renderer = new kvs::StochasticLineRenderer();
            m_screen->registerObject( plotOverLineObject, renderer );
        }
        else // リプレイス
        {
            kvs::LineObject* plotOverLineObject = new kvs::LineObject();
            plotOverLineObject->setName( PlotOverLineObjectName );
            plotOverLineObject->setXform( m_screen->scene()->objectManager()->xform( ) );
            plotOverLineObject->setCoords( coords );
            plotOverLineObject->setColors( colors );
            plotOverLineObject->setConnections( connections );
            plotOverLineObject->setSize( 10 );
            plotOverLineObject->setLineType( kvs::LineObject::Segment );
            plotOverLineObject->setColorType( kvs::LineObject::VertexColor );
            plotOverLineObject->setMinMaxObjectCoords( m_start_point_object->minObjectCoord(), m_start_point_object->maxObjectCoord() );
            plotOverLineObject->setMinMaxExternalCoords( m_start_point_object->minExternalCoord(), m_start_point_object->maxExternalCoord() );
            m_screen->scene()->replaceObject( PlotOverLineObjectName, plotOverLineObject );
        }
        m_start_coords.x() = CoordArray[0]; m_start_coords.y() = CoordArray[1]; m_start_coords.z() = CoordArray[2];
        m_end_coords.x() = CoordArray[3]; m_end_coords.y() = CoordArray[4]; m_end_coords.z() = CoordArray[5];
        m_is_send_available = true;
        m_screen->update();
    }
    else
    {
        m_is_send_available = false;
    }
}

void PlotOverLineEditor::onApply()
{
    if( m_is_send_available )
    {
        m_model->clear();

        QVector3D startCoords(
            ui->startCoordXDoubleSpinBox->value(),
            ui->startCoordYDoubleSpinBox->value(),
            ui->startCoordZDoubleSpinBox->value()
            );
        QVector3D endCoords(
            ui->endCoordXDoubleSpinBox->value(),
            ui->endCoordYDoubleSpinBox->value(),
            ui->endCoordZDoubleSpinBox->value()
            );

        auto* rootItem = new QStandardItem( "PlotOverLineParams" );
        rootItem->setData( ui->plotOverLineGroupBox->isChecked() , PlotOverLineItem::IsEnable );
        rootItem->setData( ui->resolutionSpinBox->value(), PlotOverLineItem::Resolution );
        rootItem->setData( ui->targetComboBox->currentText(), PlotOverLineItem::Variable );
        rootItem->setData( QVariant::fromValue( startCoords ), PlotOverLineItem::StartPoints );
        rootItem->setData( QVariant::fromValue( endCoords ), PlotOverLineItem::EndPoints );

        m_model->appendRow( rootItem );

        QJsonObject root;
        root["event"] = "plotoverlineparameter";
        root["enabled"] = rootItem->data(PlotOverLineItem::IsEnable).toBool();
        root["resolution"] = rootItem->data(PlotOverLineItem::Resolution).toInt();
        root["variable"] = rootItem->data(PlotOverLineItem::Variable).toString();

        auto start = rootItem->data(PlotOverLineItem::StartPoints).value<QVector3D>();
        auto end   = rootItem->data(PlotOverLineItem::EndPoints).value<QVector3D>();
        root["start"] = QJsonArray{ m_start_coords.x(), m_start_coords.y(), m_start_coords.z() };
        root["end"]   = QJsonArray{ m_end_coords.x(), m_end_coords.y(), m_end_coords.z() };

        m_web_text_socket->sendTextMessage( QJsonDocument(root).toJson(QJsonDocument::Compact) );
    }
}

void PlotOverLineEditor::updateNumberOfVector( const int numberOfVector )
{
    m_vector_list.clear();
    for( int i = 0; i < numberOfVector; i++ )
    {
        m_vector_list << QString( "q%1" ).arg( i + 1 );
    }
    ui->targetComboBox->clear();
    ui->targetComboBox->addItems( m_vector_list );
}

void PlotOverLineEditor::focusObjectUpdate( kvs::Vec3 focusTartgetObjectMinCoord, kvs::Vec3 focusTartgetObjectMaxCoord ) // NOTE:フォーカス対象のオブジェクトが変更された場合に呼び出される。
{
    if( m_start_point_object && m_end_point_object )
    {
        m_start_point_object->setMinMaxObjectCoords( focusTartgetObjectMinCoord, focusTartgetObjectMaxCoord );
        m_start_point_object->setMinMaxExternalCoords( focusTartgetObjectMinCoord, focusTartgetObjectMaxCoord );
        m_end_point_object->setMinMaxObjectCoords( focusTartgetObjectMinCoord, focusTartgetObjectMaxCoord );
        m_end_point_object->setMinMaxExternalCoords( focusTartgetObjectMinCoord, focusTartgetObjectMaxCoord );
        if( m_screen->scene()->object( PlotOverLineObjectName ) )
        {
            m_screen->scene()->object( PlotOverLineObjectName )->setMinMaxObjectCoords( focusTartgetObjectMinCoord, focusTartgetObjectMaxCoord );
            m_screen->scene()->object( PlotOverLineObjectName )->setMinMaxExternalCoords( focusTartgetObjectMinCoord, focusTartgetObjectMaxCoord );
        }
        m_screen->scene()->objectManager()->updateMinMaxCoords();
        m_screen->scene()->objectManager()->updateExternalCoords();
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

void PlotOverLineEditor::drawPlotOverLineFromVRHands( kvs::Real32 CoordArray[ 2 * 3 ] )
{
    // 色定義
    kvs::UInt8 ColorArray[ 2 * 3 ] =
        {
            0, 255, 0,
            0, 255, 0,
        };

    // 接続定義
    kvs::UInt32 ConnectionArray[ 1 * 2 ] =
        {
            0, 1,  // 点0と点1を接続
        };

    kvs::ValueArray<kvs::Real32> coords( CoordArray, 2 * 3 );
    kvs::ValueArray<kvs::UInt8> colors( ColorArray, 2 * 3 );
    kvs::ValueArray<kvs::UInt32> connections( ConnectionArray, 1 * 2 );

    if( m_start_point_object->minObjectCoord() != m_start_point_object->maxObjectCoord() )
    {
        if( m_screen->scene()->object( PlotOverLineObjectName ) == nullptr ) // オブジェクト登録
        {
            kvs::LineObject* plotOverLineObject = new kvs::LineObject();
            plotOverLineObject->setName( PlotOverLineObjectName );
            plotOverLineObject->setXform( m_screen->scene()->objectManager()->xform( ) );
            plotOverLineObject->setCoords( coords );
            plotOverLineObject->setColors( colors );
            plotOverLineObject->setConnections( connections );
            plotOverLineObject->setSize( 10 );
            plotOverLineObject->setLineType( kvs::LineObject::Segment );
            plotOverLineObject->setColorType( kvs::LineObject::VertexColor );
            plotOverLineObject->setMinMaxObjectCoords( m_start_point_object->minObjectCoord(), m_start_point_object->maxObjectCoord() );
            plotOverLineObject->setMinMaxExternalCoords( m_start_point_object->minExternalCoord(), m_start_point_object->maxExternalCoord() );

            kvs::StochasticLineRenderer* renderer = new kvs::StochasticLineRenderer();
            m_screen->registerObject( plotOverLineObject, renderer );
        }
        else // リプレイス
        {
            kvs::LineObject* plotOverLineObject = new kvs::LineObject();
            plotOverLineObject->setName( PlotOverLineObjectName );
            plotOverLineObject->setXform( m_screen->scene()->objectManager()->xform( ) );
            plotOverLineObject->setCoords( coords );
            plotOverLineObject->setColors( colors );
            plotOverLineObject->setConnections( connections );
            plotOverLineObject->setSize( 10 );
            plotOverLineObject->setLineType( kvs::LineObject::Segment );
            plotOverLineObject->setColorType( kvs::LineObject::VertexColor );
            plotOverLineObject->setMinMaxObjectCoords( m_start_point_object->minObjectCoord(), m_start_point_object->maxObjectCoord() );
            plotOverLineObject->setMinMaxExternalCoords( m_start_point_object->minExternalCoord(), m_start_point_object->maxExternalCoord() );
            m_screen->scene()->replaceObject( PlotOverLineObjectName, plotOverLineObject );
        }
    }
    m_screen->update();
}

void PlotOverLineEditor::togglePlotOverLineFromVRHands()
{
    bool current = ui->plotOverLineGroupBox->isChecked();
    ui->plotOverLineGroupBox->setChecked( !current );
}

void PlotOverLineEditor::updateOperatorState( bool operatorState )
{
    ui->resolutionSpinBox->setEnabled( operatorState );
    ui->targetComboBox->setEnabled( operatorState );
    ui->plotOverLineGroupBox->setEnabled( operatorState );

    ui->startTranslationXDoubleSpinBox->setEnabled( operatorState );
    ui->startTranslationYDoubleSpinBox->setEnabled( operatorState );
    ui->startTranslationZDoubleSpinBox->setEnabled( operatorState );
    ui->startCoordXDoubleSpinBox->setEnabled( operatorState );
    ui->startCoordYDoubleSpinBox->setEnabled( operatorState );
    ui->startCoordZDoubleSpinBox->setEnabled( operatorState );

    ui->endTranslationXDoubleSpinBox->setEnabled( operatorState );
    ui->endTranslationYDoubleSpinBox->setEnabled( operatorState );
    ui->endTranslationZDoubleSpinBox->setEnabled( operatorState );
    ui->endCoordXDoubleSpinBox->setEnabled( operatorState );
    ui->endCoordYDoubleSpinBox->setEnabled( operatorState );
    ui->endCoordZDoubleSpinBox->setEnabled( operatorState );

    ui->createLinePushButton->setEnabled( operatorState );
    ui->applyPushButton->setEnabled( operatorState );
}

void PlotOverLineEditor::reset()
{
    updateNumberOfVector( 0 );
    ui->resolutionSpinBox->setValue( 256 );
    ui->plotOverLineGroupBox->setChecked( false );

    ui->startTranslationXDoubleSpinBox->setValue( 0 );
    ui->startTranslationYDoubleSpinBox->setValue( 0 );
    ui->startTranslationZDoubleSpinBox->setValue( 0 );
    ui->startCoordXDoubleSpinBox->setValue( 0 );
    ui->startCoordYDoubleSpinBox->setValue( 0 );
    ui->startCoordZDoubleSpinBox->setValue( 0 );

    ui->endTranslationXDoubleSpinBox->setValue( 0 );
    ui->endTranslationYDoubleSpinBox->setValue( 0 );
    ui->endTranslationZDoubleSpinBox->setValue( 0 );
    ui->endCoordXDoubleSpinBox->setValue( 0 );
    ui->endCoordYDoubleSpinBox->setValue( 0 );
    ui->endCoordZDoubleSpinBox->setValue( 0 );
}

void PlotOverLineEditor::loadParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void PlotOverLineEditor::saveParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}
