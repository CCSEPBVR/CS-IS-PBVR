#include "ObjectEditorWIP.h"
#include "ui_ObjectEditorWIP.h"

ObjectEditorWIP::ObjectEditorWIP( WebSocketPair* websockets, Viz::Mode* vizMode, kvs::qt::jaea::Screen* screen, QWidget *parent )
    : QDockWidget(parent)
    , ui(new Ui::ObjectEditorWIP)
    , m_web_sockets( websockets )
    , m_screen( screen )
    , m_viz_mode( vizMode )
{
    initialize();
}

ObjectEditorWIP::~ObjectEditorWIP()
{
    delete ui;
}

void ObjectEditorWIP::updateOperatorState( bool operatorState )
{
    ui->focusCheckBox                   ->setEnabled( operatorState );
    ui->particleLimitSpinBox            ->setEnabled( operatorState );
    ui->extraOpacityFactorDoubleSpinBox ->setEnabled( operatorState );
    ui->coordinateXLineEdit             ->setEnabled( operatorState );
    ui->coordinateYLineEdit             ->setEnabled( operatorState );
    ui->coordinateZLineEdit             ->setEnabled( operatorState );
    ui->exportPushButton                ->setEnabled( operatorState );
    ui->colorClickableLabel             ->setEnabled( operatorState );
    ui->opacityDoubleSpinBox            ->setEnabled( operatorState );
    ui->browsePushButton                ->setEnabled( operatorState );
    ui->deletePushButton                ->setEnabled( operatorState );
    ui->applyPushButton                 ->setEnabled( operatorState );
}

void ObjectEditorWIP::reset()
{
    ui->focusCheckBox                   ->setEnabled( true );
    ui->particleLimitSpinBox            ->setEnabled( true );
    ui->extraOpacityFactorDoubleSpinBox ->setEnabled( true );
    ui->coordinateXLineEdit             ->setEnabled( true );
    ui->coordinateYLineEdit             ->setEnabled( true );
    ui->coordinateZLineEdit             ->setEnabled( true );
    ui->exportPushButton                ->setEnabled( true );
    ui->colorClickableLabel             ->setEnabled( true );
    ui->opacityDoubleSpinBox            ->setEnabled( true );
    ui->browsePushButton                ->setEnabled( true );
    ui->deletePushButton                ->setEnabled( true );
    ui->applyPushButton                 ->setEnabled( true );

    if( m_model )
    {
        // 全行の ObjectInfo を取得してオブジェクト削除
        for( int row = 0; row < m_model->rowCount(); ++row )
        {
            QStandardItem* nameItem = m_model->item( row, 0 );
            if( !nameItem ) continue;

            QVariant var = nameItem->data( Qt::UserRole );
            if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

            // シーンからオブジェクト削除
            m_screen->scene()->removeObject( info.objectID.first );
        }

        // モデルの行をすべて削除
        m_model->removeRows( 0, m_model->rowCount() );
        calculateTotalMinMaxTimeStep();
        m_screen->update();
    }

    toggleCommonObjectWidgets( false );
    toggleCommonServerObjectWidgets( false );
    toggleClientServerObjectWidgets( false );
    toggleNontexturePolygonObjectWidgets( false );
}

// void ObjectEditorWIP::addObjectToModel( const ObjectInfoExtractor::ObjectInfo& objectInfo, ObjectInfoExtractor& oie )
void ObjectEditorWIP::addObjectToModel( const ObjectInfoExtractor::ObjectInfo& objectInfo )
{
    ObjectInfoExtractor::ObjectInfo info = objectInfo;

    // 最初に追加されるオブジェクトはフォーカス状態にする
    if( m_model->rowCount() == 0 )
    {
        info.tmpIsFocus = true;
    }

    QList<QStandardItem*> rowItems;
    auto nameItem         = new QStandardItem( QString::fromUtf8( info.name ) );
    auto formatItem       = new QStandardItem( QString::fromUtf8( ObjectInfoExtractor::formatToString( info.format ) ) );
    auto displayItem      = new QStandardItem( "" );
    auto keepInitialItem  = new QStandardItem( "" );
    auto keepFinalItem    = new QStandardItem( "" );

    // 編集禁止
    nameItem        ->setEditable( false );
    formatItem      ->setEditable( false );
    displayItem     ->setEditable( false );
    keepInitialItem ->setEditable( false );
    keepFinalItem   ->setEditable( false );

    displayItem     ->setCheckable( true );
    keepInitialItem ->setCheckable( true );
    keepFinalItem   ->setCheckable( true );

    // bool -> Qt::CheckState 変換
    displayItem     ->setCheckState( info.tmpIsDisplay     ? Qt::Checked : Qt::Unchecked );
    keepInitialItem ->setCheckState( info.tmpIsKeepInitial ? Qt::Checked : Qt::Unchecked );
    keepFinalItem   ->setCheckState( info.tmpIsKeepFinal   ? Qt::Checked : Qt::Unchecked );

    QVariant var;
    var.setValue( info );
    nameItem->setData( var, Qt::UserRole );

    rowItems << nameItem << formatItem << displayItem << keepInitialItem << keepFinalItem;
    m_model->appendRow( rowItems );

    QModelIndex index = m_model->indexFromItem( rowItems.first() );
    ui->treeView->setCurrentIndex( index );

    calculateTotalMinMaxTimeStep();
}

void ObjectEditorWIP::showAtTimeStep( int timeStep )
{
    switch( *m_viz_mode )
    {
    case Viz::Mode::Local:
    case Viz::Mode::LocalClientAndServer:
    {
        Worker* worker = new Worker( timeStep, m_model, m_screen );
        QThread* thread = new QThread;

        worker->moveToThread( thread );

        connect( thread, &QThread::started, worker, &Worker::process );
        connect( worker, &Worker::done, thread, &QThread::quit );
        connect( worker, &Worker::done, worker, &Worker::deleteLater );
        connect( thread, &QThread::finished, thread, &QThread::deleteLater );
        connect( worker, &Worker::done, this, [this, timeStep]() { doneObjectEditor( timeStep ); } );

        thread->start();
        break;
    }
    case Viz::Mode::RemoteClientAndServer:
    case Viz::Mode::RemoteInSitu:
    {
        if( m_web_sockets->isConnected() )
        {
            m_web_sockets->text()->sendTextMessage( QJsonDocument( {
                                                                  {"event", "showAtTimeStep"},
                                                                  {"timeStep", timeStep},
                                                                  } ).toJson( QJsonDocument::Compact ) );
        }
        break;
    }
    default:
        break;
    }
}

void ObjectEditorWIP::loadParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ObjectEditorWIP::saveParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ObjectEditorWIP::initialize()
{
    ui->setupUi( this );
    m_model = qobject_cast<QStandardItemModel*>( ui->treeView->model() );
    if( !m_model ) // モデルがなかったら作成する。
    {
        m_model = new QStandardItemModel( this );
        // ヘッダーを設定（列名を設定）
        m_model->setHorizontalHeaderLabels( { "Name", "Format", "Display", "Keep Initial", "Keep Final" } );
        ui->treeView->setModel( m_model );
    }

    ui->colorClickableLabel->setAutoFillBackground( true );

    m_group_common_object_widgets =
        {
            ui->nameLabel       , ui->nameLineEdit,
            ui->formatLabel     , ui->formatLineEdit,
            ui->directoryLabel  , ui->directoryLineEdit,
            ui->timeStepLabel   , ui->timeStepMinLineEdit, ui->timeStepMaxLineEdit,
            ui->focusLabel      , ui->focusCheckBox,
            ui->objectCoordsGroupBox,
            ui->externalCoordsGroupBox,
        };

    m_group_common_server_point_object_widgets =
        {
            ui->particleLimitLabel      , ui->particleLimitSpinBox,
            ui->extraOpacityFactorLabel , ui->extraOpacityFactorDoubleSpinBox,
        };

    m_group_client_server_point_object_widgets =
        {
            ui->numberOfVectorLabel     , ui->numberOfVectorLineEdit,
            ui->numberOfElementsLabel   , ui->numberOfElementsLineEdit,
            ui->numberOfSubvolumeLabel  , ui->numberOfSubvolumeLineEdit,
            ui->numberOfNodesLabel      , ui->numberOfNodesLineEdit,
            ui->elementTypeLabel        , ui->elementTypeLineEdit,
            ui->fileTypeLabel           , ui->fileTypeLineEdit,
            ui->stepNumberLabel         , ui->stepNumberLineEdit,
            ui->coordinateLabel         , ui->coordinateXLineEdit, ui->coordinateYLineEdit, ui->coordinateZLineEdit,
            ui->exportLabel             , ui->exportPushButton,
        };

    m_group_nontexture_polygon_object_widgets =
        {
            ui->colorLabel  , ui->colorClickableLabel,
            ui->opacityLabel, ui->opacityDoubleSpinBox,
        };

    // 起動時はオブジェクトは存在しないため全て非表示
    toggleCommonObjectWidgets( false );
    toggleCommonServerObjectWidgets( false );
    toggleClientServerObjectWidgets( false );
    toggleNontexturePolygonObjectWidgets( false );

    connect( ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ObjectEditorWIP::onItemSelection );

    // 全オブジェクト共通
    connect( ui->focusCheckBox, &QCheckBox::toggled, this, &ObjectEditorWIP::onFocusCheckBoxToggled );

    // サーバポイントオブジェクトである。(ClientServer/Insitu共通)
    connect( ui->particleLimitSpinBox, &QSpinBox::valueChanged, this, &ObjectEditorWIP::onParticleLimitSpinBoxValueChanged );
    connect( ui->extraOpacityFactorDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &ObjectEditorWIP::onExtraOpacityFactorDoubleSpinBoxValueChanged );

    // サーバポイントオブジェクト(ClientServer)で表示させるウィジェット群
    connect( ui->coordinateXLineEdit, &QLineEdit::textChanged, this, &ObjectEditorWIP::onCoordinateLineEditTextChanged );
    connect( ui->coordinateYLineEdit, &QLineEdit::textChanged, this, &ObjectEditorWIP::onCoordinateLineEditTextChanged );
    connect( ui->coordinateZLineEdit, &QLineEdit::textChanged, this, &ObjectEditorWIP::onCoordinateLineEditTextChanged );

    // true:テクスチャ無しポリゴンオブジェクトである。(.stl, .kvsml)
    connect( ui->colorClickableLabel, &ClickableLabel::doubleClicked, this, &ObjectEditorWIP::onColorLabelDoubleClicked );
    connect( ui->opacityDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &ObjectEditorWIP::onOpacityDoubleSpinBoxValueChanged );

    connect( ui->browsePushButton, &QPushButton::clicked, this, &ObjectEditorWIP::onBrowse );
    connect( ui->deletePushButton, &QPushButton::clicked, this, &ObjectEditorWIP::onDelete );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &ObjectEditorWIP::onApply );

    // デバッグボタン作成
    QPushButton* debugPushButton = new QPushButton("Debug", this);
    debugPushButton->setGeometry(650, 50, 120, 40);
    debugPushButton->show();

    // ボタン押下でサーバにイベント送信
    connect( debugPushButton, &QPushButton::clicked, this, [this]()
            {
                if( !m_web_sockets->isConnected() )
                {
                    qDebug() << "Not connected.";
                    return;
                }
                m_web_sockets->text()->sendTextMessage( QJsonDocument( QJsonObject{ {"event", "debugSrvObjects"} } ).toJson( QJsonDocument::Compact ) );

                for( int row = 0; row < m_model->rowCount(); row++ )
                {
                    QStandardItem* nameItem = m_model->item( row, 0 );
                    QVariant var = nameItem->data( Qt::UserRole );
                    if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

                    ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

                    std::cout << "[Client]"                                                                                     << std::endl;
                    std::cout << "[Common Object Info]"                                                                         << std::endl;
                    std::cout << "info.tmpIsDisplay                 : " << info.tmpIsDisplay                                    << std::endl;
                    std::cout << "info.isDisplay                    : " << info.isDisplay                                       << std::endl;
                    std::cout << "info.tmpIsKeepInitial             : " << info.tmpIsKeepInitial                                << std::endl;
                    std::cout << "info.isKeepInitial                : " << info.isKeepInitial                                   << std::endl;
                    std::cout << "info.tmpIsKeepFinal               : " << info.tmpIsKeepFinal                                  << std::endl;
                    std::cout << "info.isKeepFinal                  : " << info.isKeepFinal                                     << std::endl;

                    std::cout << "info.name                         : " << info.name                                            << std::endl;
                    std::cout << "info.extension                    : " << info.extension                                       << std::endl;
                    std::cout << "info.directory                    : " << info.directory                                       << std::endl;
                    std::cout << "info.format                       : " << info.format                                          << std::endl;
                    std::cout << "info.timeStep                     : " << info.timeStep.first << ", " << info.timeStep.second  << std::endl;
                    std::cout << "info.tmpIsFocus                   : " << info.tmpIsFocus                                      << std::endl;
                    std::cout << "info.isFocus                      : " << info.isFocus                                         << std::endl;
                    std::cout << "info.minObjectCoord               : " << info.minObjectCoord                                  << std::endl;
                    std::cout << "info.maxObjectCoord               : " << info.maxObjectCoord                                  << std::endl;
                    std::cout << "info.minExternalCoord             : " << info.minExternalCoord                                << std::endl;
                    std::cout << "info.maxExternalCoord             : " << info.maxExternalCoord                                << std::endl;

                    std::cout << "[Common Server Point Object Info]"                                                            << std::endl;
                    std::cout << "info.tmpParticleLimit             : " << info.tmpParticleLimit                                << std::endl;
                    std::cout << "info.particleLimit                : " << info.particleLimit                                   << std::endl;
                    std::cout << "info.tmpExtraOpacityFactor        : " << info.tmpExtraOpacityFactor                           << std::endl;
                    std::cout << "info.extraOpacityFactor           : " << info.extraOpacityFactor                              << std::endl;

                    std::cout << "[Client Server Point Object Info]"                                                            << std::endl;
                    std::cout << "info.numberOfVector               : " << info.numberOfVector                                  << std::endl;
                    std::cout << "info.numberOfElements             : " << info.numberOfElements                                << std::endl;
                    std::cout << "info.numberOfSubvolume            : " << info.numberOfSubvolume                               << std::endl;
                    std::cout << "info.numberOfNodes                : " << info.numberOfNodes                                   << std::endl;
                    std::cout << "info.elementType                  : " << info.elementType                                     << std::endl;
                    std::cout << "info.fileType                     : " << info.fileType                                        << std::endl;
                    std::cout << "info.stepNumber                   : " << info.stepNumber                                      << std::endl;
                    std::cout << "info.tmpCoordinateX               : " << info.tmpCoordinateX                                  << std::endl;
                    std::cout << "info.coordinateX                  : " << info.coordinateX                                     << std::endl;
                    std::cout << "info.tmpCoordinateY               : " << info.tmpCoordinateY                                  << std::endl;
                    std::cout << "info.coordinateY                  : " << info.coordinateY                                     << std::endl;
                    std::cout << "info.tmpCoordinateZ               : " << info.tmpCoordinateZ                                  << std::endl;
                    std::cout << "info.coordinateZ                  : " << info.coordinateZ                                     << std::endl;
                    std::cout << "info.isExport                     : " << info.isExport                                        << std::endl;

                    std::cout << "[Nontexture Polygon Object Info]"                                                             << std::endl;
                    std::cout << "kvs::RGBColor tmpPolygonColor     : " << info.tmpPolygonColor                                 << std::endl;
                    std::cout << "kvs::RGBColor polygonColor        : " << info.polygonColor                                    << std::endl;
                    std::cout << "info.tmpPolygonOpacity            : " << info.tmpPolygonOpacity                               << std::endl;
                    std::cout << "info.polygonOpacity               : " << info.polygonOpacity                                  << std::endl;

                    std::cout << "[For client]"                                                                                 << std::endl;
                    std::cout << "info.object                       : " << info.object                                          << std::endl;
                    std::cout << "info.objectID                     : " << info.objectID.first << ", " << info.objectID.second  << std::endl;
                    std::cout << "info.currentMinObjectCoord        : " << info.currentMinObjectCoord                           << std::endl;
                    std::cout << "info.currentMaxObjectCoord        : " << info.currentMaxObjectCoord                           << std::endl;
                    std::cout << "info.currentImportedTimeStep      : " << info.currentImportedTimeStep                         << std::endl;
                    std::cout << "info.needSameTimeStepReplace      : " << info.needSameTimeStepReplace                         << std::endl;
                }
            } );
}

void ObjectEditorWIP::toggleCommonObjectWidgets( bool isObject )
{
    for( auto w : m_group_common_object_widgets ) w->setVisible( isObject );
}

void ObjectEditorWIP::toggleCommonServerObjectWidgets( bool isCommonServerObject )
{
    for( auto w : m_group_common_server_point_object_widgets ) w->setVisible( isCommonServerObject );
}

void ObjectEditorWIP::toggleClientServerObjectWidgets( bool isClientServerObject )
{
    for( auto w : m_group_client_server_point_object_widgets ) w->setVisible( isClientServerObject );
}

void ObjectEditorWIP::toggleNontexturePolygonObjectWidgets( bool isNonTexturePolygonObject )
{
    for( auto w : m_group_nontexture_polygon_object_widgets ) w->setVisible( isNonTexturePolygonObject );
}

void ObjectEditorWIP::calculateTotalMinMaxTimeStep()
{
    if( !m_model ) return;

    int totalMin = std::numeric_limits<int>::max();
    int totalMax = std::numeric_limits<int>::min();

    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem = m_model->item( row, 0 ); // UserRoleにObjectInfoが格納されている列
        if( !nameItem ) continue;

        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

        totalMin = std::min( totalMin, info.timeStep.first );
        totalMax = std::max( totalMax, info.timeStep.second );
    }

    bool isSingleObject = false;
    if( m_model->rowCount() == 1 ) isSingleObject = true;

    emit updateTotalTimeStepRange( totalMin, totalMax, isSingleObject );
}

template<typename F>
void ObjectEditorWIP::updateSelectedObject( F func )
{
    auto selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) return;

    ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

    func( info ); // 渡された処理で info を更新する

    m_model->setData( index, QVariant::fromValue( info ), Qt::UserRole );
}

void ObjectEditorWIP::registerObject( ObjectInfoExtractor::ObjectInfo& info )
{
    std::unique_ptr<kvs::glsl::ParticleBasedRenderer> particleBasedRenderer;
    std::unique_ptr<kvs::StochasticPolygonRenderer> stochasticPolygonRenderer;
    std::unique_ptr<kvs::StochasticLineRenderer> stochasticLineRenderer;
    std::unique_ptr<kvs::StochasticTexturedPolygonRenderer> stochasticTexturedPolygonRenderer;

    switch( info.format )
    {
    case ObjectInfoExtractor::ClientServerPointObject:
        break;
    case ObjectInfoExtractor::InsituServerPointObject:
        break;
    case ObjectInfoExtractor::ServerGlyphObject:
        break;
    case ObjectInfoExtractor::PointObjectKVSML:
    case ObjectInfoExtractor::PointObjectLAS:
    case ObjectInfoExtractor::PointObjectPTS:
        particleBasedRenderer = std::make_unique<kvs::glsl::ParticleBasedRenderer>();
        particleBasedRenderer.get()->enableShuffle();
        emit shading( particleBasedRenderer.get() );
        info.objectID = m_screen->registerObject( static_cast<kvs::PointObject*>(info.object), particleBasedRenderer.release() );
        break;
    case ObjectInfoExtractor::PolygonObjectKVSML:
    case ObjectInfoExtractor::PolygonObjectSTL:
        stochasticPolygonRenderer = std::make_unique<kvs::StochasticPolygonRenderer>();
        emit shading( stochasticPolygonRenderer.get() );
        info.objectID = m_screen->registerObject( info.object, stochasticPolygonRenderer.release() );
        break;
#ifdef ASSIMP
    case ObjectInfoExtractor::PolygonObject3DS:
    case ObjectInfoExtractor::PolygonObjectFBX:
        stochasticTexturedPolygonRenderer = std::make_unique<kvs::StochasticTexturedPolygonRenderer>();
        emit shading( stochasticTexturedPolygonRenderer.get() );
        info.objectID = m_screen->registerObject( info.object, stochasticTexturedPolygonRenderer.release() );
        break;
#endif
    case ObjectInfoExtractor::LineObjectKVSML:
        stochasticLineRenderer = std::make_unique<kvs::StochasticLineRenderer>();
        emit shading( stochasticLineRenderer.get() );
        info.objectID = m_screen->registerObject( info.object, stochasticLineRenderer.release() );
        break;
    default:
        return;
    }
}

void ObjectEditorWIP::replaceObject( ObjectInfoExtractor::ObjectInfo& info )
{
    std::unique_ptr<kvs::StochasticPolygonRenderer> stochasticPolygonRenderer;

    switch( info.format )
    {
    case ObjectInfoExtractor::ClientServerPointObject:
        break;
    case ObjectInfoExtractor::InsituServerPointObject:
        break;
    case ObjectInfoExtractor::ServerGlyphObject:
        break;
    case ObjectInfoExtractor::PointObjectKVSML:
    case ObjectInfoExtractor::PointObjectLAS:
    case ObjectInfoExtractor::PointObjectPTS:
        m_screen->scene()->replaceObject( 1, dynamic_cast<kvs::PointObject*>(info.object) );
        break;
    case ObjectInfoExtractor::PolygonObjectKVSML:
    case ObjectInfoExtractor::PolygonObjectSTL:
        stochasticPolygonRenderer = std::make_unique<kvs::StochasticPolygonRenderer>();
        emit shading( stochasticPolygonRenderer.get() );
        m_screen->scene()->replaceObject( info.objectID.first, info.object );
        m_screen->scene()->replaceRenderer( info.objectID.second, stochasticPolygonRenderer.release() );
        break;
#ifdef ASSIMP
    case ObjectInfoExtractor::PolygonObject3DS:
    case ObjectInfoExtractor::PolygonObjectFBX:
        m_screen->scene()->replaceObject( info.objectID.first, info.object );
        break;
#endif
    case ObjectInfoExtractor::LineObjectKVSML:
        m_screen->scene()->replaceObject( info.objectID.first, info.object );
        break;
    default:
        return;
    }
}

void ObjectEditorWIP::onItemSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    if( selected.indexes().isEmpty() ) return;

    QModelIndex index = selected.indexes().first();
    QVariant var = index.data(Qt::UserRole);
    if (!var.canConvert<ObjectInfoExtractor::ObjectInfo>()) return;

    ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

    // 全オブジェクト共通
    ui->nameLineEdit                    ->setText( QString::fromUtf8( info.name.c_str() ) );
    ui->directoryLineEdit               ->setText( QString::fromUtf8( info.directory.c_str() ) );
    ui->formatLineEdit                  ->setText( QString::fromUtf8( ObjectInfoExtractor::formatToString( info.format ) ) );
    ui->timeStepMinLineEdit             ->setText( QString::number( info.timeStep.first ) );
    ui->timeStepMaxLineEdit             ->setText( QString::number( info.timeStep.second ) );
    ui->focusCheckBox                   ->setChecked( info.tmpIsFocus );
    ui->minObjectXCoordLineEdit         ->setText( QString::number( info.minObjectCoord.x() ) );
    ui->minObjectYCoordLineEdit         ->setText( QString::number( info.minObjectCoord.y() ) );
    ui->minObjectZCoordLineEdit         ->setText( QString::number( info.minObjectCoord.z() ) );
    ui->maxObjectXCoordLineEdit         ->setText( QString::number( info.maxObjectCoord.x() ) );
    ui->maxObjectYCoordLineEdit         ->setText( QString::number( info.maxObjectCoord.y() ) );
    ui->maxObjectZCoordLineEdit         ->setText( QString::number( info.maxObjectCoord.z() ) );
    ui->minExternalXCoordLineEdit       ->setText( QString::number( info.minExternalCoord.x() ) );
    ui->minExternalYCoordLineEdit       ->setText( QString::number( info.minExternalCoord.y() ) );
    ui->minExternalZCoordLineEdit       ->setText( QString::number( info.minExternalCoord.z() ) );
    ui->maxExternalXCoordLineEdit       ->setText( QString::number( info.maxExternalCoord.x() ) );
    ui->maxExternalYCoordLineEdit       ->setText( QString::number( info.maxExternalCoord.y() ) );
    ui->maxExternalZCoordLineEdit       ->setText( QString::number( info.maxExternalCoord.z() ) );

    // サーバポイントオブジェクトである。(ClientServer/Insitu共通)
    ui->particleLimitSpinBox            ->setValue( info.tmpParticleLimit );
    ui->extraOpacityFactorDoubleSpinBox ->setValue( info.tmpExtraOpacityFactor );

    // サーバポイントオブジェクト(ClientServer)で表示させるウィジェット群
    ui->numberOfVectorLineEdit          ->setText( QString::number( info.numberOfVector ) );
    ui->numberOfElementsLineEdit        ->setText( QString::number( info.numberOfElements ) );
    ui->numberOfSubvolumeLineEdit       ->setText( QString::number( info.numberOfSubvolume ) );
    ui->numberOfNodesLineEdit           ->setText( QString::number( info.numberOfNodes ) );
    ui->elementTypeLineEdit             ->setText( QString::number( info.elementType ) );
    ui->fileTypeLineEdit                ->setText( QString::number( info.fileType ) );
    ui->stepNumberLineEdit              ->setText( QString::number( info.stepNumber ) );
    ui->coordinateXLineEdit             ->setText( QString::fromUtf8( info.tmpCoordinateX ) );
    ui->coordinateYLineEdit             ->setText( QString::fromUtf8( info.tmpCoordinateY ) );
    ui->coordinateZLineEdit             ->setText( QString::fromUtf8( info.tmpCoordinateZ ) );

    // true:テクスチャ無しポリゴンオブジェクトである。(.stl, .kvsml)
    QPalette palette = ui->colorClickableLabel->palette();
    palette.setColor( QPalette::Window, QColor( info.tmpPolygonColor.r(), info.tmpPolygonColor.g(), info.tmpPolygonColor.b() ) );
    ui->colorClickableLabel         ->setPalette( palette );
    ui->opacityDoubleSpinBox        ->setValue( info.tmpPolygonOpacity );

    bool isObject = false;
    bool isCommonServerObject = false;
    bool isClientServerObject = false;
    bool isNonTexturePolygonObject = false;
    // フラグをマップで管理(isObject, isCommonServerObject, isClientServerObject, isNonTexturePolygonObject )
    static const std::map<ObjectInfoExtractor::Format, std::tuple<bool,bool,bool,bool>> formatFlags =
        {
         { ObjectInfoExtractor::Format::Unknown,                   { false, false, false, false } },
         { ObjectInfoExtractor::Format::ClientServerPointObject,   { true,  true,  true,  false } },
         { ObjectInfoExtractor::Format::InsituServerPointObject,   { true,  true,  false, false } },
         { ObjectInfoExtractor::Format::ServerGlyphObject,         { true,  false, false, false } },
         { ObjectInfoExtractor::Format::PointObjectKVSML,          { true,  false, false, false } },
         { ObjectInfoExtractor::Format::PointObjectLAS,            { true,  false, false, false } },
         { ObjectInfoExtractor::Format::PointObjectPTS,            { true,  false, false, false } },
         { ObjectInfoExtractor::Format::PolygonObjectKVSML,        { true,  false, false, true  } },
         { ObjectInfoExtractor::Format::PolygonObjectSTL,          { true,  false, false, true  } },
         { ObjectInfoExtractor::Format::PolygonObject3DS,          { true,  false, false, false } },
         { ObjectInfoExtractor::Format::PolygonObjectFBX,          { true,  false, false, false } },
         { ObjectInfoExtractor::Format::LineObjectKVSML,           { true,  false, false, false } },
         };

    auto it = formatFlags.find( info.format );
    if( it != formatFlags.end() )
    {
        std::tie( isObject, isCommonServerObject, isClientServerObject, isNonTexturePolygonObject ) = it->second;
    }

    toggleCommonObjectWidgets( isObject );
    toggleCommonServerObjectWidgets( isCommonServerObject );
    toggleClientServerObjectWidgets( isClientServerObject );
    toggleNontexturePolygonObjectWidgets( isNonTexturePolygonObject );
}

void ObjectEditorWIP::onFocusCheckBoxToggled( bool checked )
{
    updateSelectedObject( [checked]( auto &info )
                         {
                             info.tmpIsFocus = checked;
                         } );
}

void ObjectEditorWIP::onParticleLimitSpinBoxValueChanged( int value )
{
    updateSelectedObject( [value]( auto &info )
                         {
                             info.tmpParticleLimit = value;
                         } );
}

void ObjectEditorWIP::onExtraOpacityFactorDoubleSpinBoxValueChanged( double value )
{
    updateSelectedObject( [value]( auto &info )
                         {
                             info.tmpExtraOpacityFactor = value;
                         } );
}

void ObjectEditorWIP::onCoordinateLineEditTextChanged()
{
    updateSelectedObject( [this] ( auto &info )
                         {
                             info.tmpCoordinateX = ui->coordinateXLineEdit->text().toUtf8().constData();
                             info.tmpCoordinateY = ui->coordinateYLineEdit->text().toUtf8().constData();
                             info.tmpCoordinateZ = ui->coordinateZLineEdit->text().toUtf8().constData();
                         } );
}

void ObjectEditorWIP::onColorLabelDoubleClicked()
{
    updateSelectedObject( [this]( auto &info )
                         {
                             QColorDialog colorDialog;
                             colorDialog.adjustSize();
                             if( colorDialog.exec() == QDialog::Accepted )
                             {
                                 QColor color = colorDialog.selectedColor();
                                 QPalette palette = ui->colorClickableLabel->palette();
                                 palette.setColor( QPalette::Window, color );
                                 ui->colorClickableLabel->setPalette( palette );
                                 info.tmpPolygonColor.set( color.red(), color.green(), color.blue() );
                             }
                         } );
}

void ObjectEditorWIP::onOpacityDoubleSpinBoxValueChanged( double value )
{
    updateSelectedObject( [value]( auto &info )
                         {
                             info.tmpPolygonOpacity = value;
                         } );
}

void ObjectEditorWIP::onBrowse()
{
    /*
     * FIXME:
     * ローカルモード(サーバと接続せずにローカルデータのみ閲覧するモード)の場合:QFileDialogでローカルファイルを参照
     * スタンドアロンモード(クライアントとサーバを同じマシンで起動、接続するモード)の場合:QFileDialogでローカルファイルを参照
     * クラサバモード(クライアントとサーバを別マシンで起動、接続するモード)の場合:RemoteFileDialogでリモートファイルを参照
     * In-situモード(クライアントとサーバを別マシンで起動、接続するモード)の場合:RemoteFileDialogでリモートファイルを参照
     */

    switch( *m_viz_mode )
    {
    case Viz::Mode::Local:
    case Viz::Mode::LocalClientAndServer:
    {
        QStringList filePaths = QFileDialog::getOpenFileNames(
            this,
            tr("Select 3D data files"),
            QDir::homePath(),
#ifdef ASSIMP
            tr("Support Files (*.kvsml *.las *.pts *.stl *.fbx *.3ds);;All Files (*.*)")
#else
            tr("Support Files (*.kvsml *.las *.pts *.stl);;All Files (*.*)")
#endif
            );

        if (filePaths.isEmpty()) return;

        for( const QString& filePath : filePaths )
        {
            QString uuid = QUuid::createUuid().toString( QUuid::WithoutBraces );
            ObjectInfoExtractor oie( filePath.toUtf8().constData() );
            if( auto objectInfoOpt = oie.extractFromLocalFile() )
            {
                objectInfoOpt->uuid = uuid.toUtf8();
                addObjectToModel( *objectInfoOpt );
            }
            else
            {
                // FIXME: MainWindowのStatusBarで通知
            }
        }
        break;
    }
    case Viz::Mode::RemoteClientAndServer:
    case Viz::Mode::RemoteInSitu:
    {
        RemoteFileDialog dlg( m_web_sockets, this );
        if( dlg.exec() == QDialog::Accepted )
        {
            QString uuid = QUuid::createUuid().toString( QUuid::WithoutBraces );
            m_web_sockets->text()->sendTextMessage(
                QJsonDocument( QJsonObject{
                                  { "event", "selectedFile" },
                                  { "file", dlg.selectedFile() },
                                  { "uuid", uuid }
                              } ).toJson( QJsonDocument::Compact )
                );
        }
        break;
    }
    default:
        break;
    }
}

void ObjectEditorWIP::onDelete()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) return;

    bool requireFocusOnOther = false; // 他のオブジェクトをフォーカスするかどうかのフラグ

    ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();
    if( info.tmpIsFocus )
    {
        bool otherFocusedFound = false;
        for( int row = 0; row < m_model->rowCount(); row++ )
        {
            if( row == index.row() ) continue; // 自分自身はスキップ

            QStandardItem* nameItem = m_model->item( row, 0 ); // UserRoleにObjectInfoが格納されている列
            if( !nameItem ) continue;

            QVariant otherVar = nameItem->data( Qt::UserRole );
            if( !otherVar.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            ObjectInfoExtractor::ObjectInfo otherInfo = otherVar.value<ObjectInfoExtractor::ObjectInfo>();
            if( otherInfo.tmpIsFocus )
            {
                otherFocusedFound = true;
                break; // 他にフォーカス中のアイテムがあるので終了
            }
        }
        requireFocusOnOther = !otherFocusedFound; // 他にフォーカス中のアイテムがない場合はtrue
    }

    if( requireFocusOnOther )
    {
        for( int row = 0; row < m_model->rowCount(); ++row )
        {
            QStandardItem* item = m_model->item( row, 0 ); // UserRoleにObjectInfoが格納されている列
            if( !item ) continue;

            QVariant var = item->data( Qt::UserRole );
            if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();
            // 自分自身（削除対象）はスキップ
            if( row == index.row() ) continue;

            // 一番若い（最初に見つかった）アイテムをフォーカス
            info.tmpIsFocus = true;
            item->setData( QVariant::fromValue(info), Qt::UserRole );
            break; // 1つだけ更新したら終了
        }
    }

    m_model->removeRow( index.row() );

    calculateTotalMinMaxTimeStep();

    m_screen->scene()->removeObject( info.objectID.first );
    m_screen->update();

    if( m_model->rowCount() == 0 )
    {
        toggleCommonObjectWidgets( false );
        toggleCommonServerObjectWidgets( false );
        toggleClientServerObjectWidgets( false );
        toggleNontexturePolygonObjectWidgets( false );
    }
}

void ObjectEditorWIP::onApply()
{
    // フォーカス対象の最小最大オブジェクト座標の計算結果格納用変数
    kvs::Vec3 resultMinObjectCoords(
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
        );

    kvs::Vec3 resultMaxObjectCoords(
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest()
        );

    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem         = m_model->item( row, 0 );
        QStandardItem* displayItem      = m_model->item( row, 2 );
        QStandardItem* keepInitialItem  = m_model->item( row, 3 );
        QStandardItem* keepFinalItem    = m_model->item( row, 4 );
        if( !nameItem || !displayItem || !keepInitialItem || !keepFinalItem ) continue;

        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo objectInfo = var.value<ObjectInfoExtractor::ObjectInfo>();

        // チェックボックスの状態を tmp に反映
        objectInfo.tmpIsDisplay         = ( displayItem->checkState()        == Qt::Checked );
        objectInfo.tmpIsKeepInitial     = ( keepInitialItem->checkState()    == Qt::Checked );
        objectInfo.tmpIsKeepFinal       = ( keepFinalItem->checkState()      == Qt::Checked );

        // tmpの確定
        objectInfo.isDisplay            = objectInfo.tmpIsDisplay;
        objectInfo.isKeepInitial        = objectInfo.tmpIsKeepInitial;
        objectInfo.isKeepFinal          = objectInfo.tmpIsKeepFinal;

        objectInfo.isFocus              = objectInfo.tmpIsFocus;

        objectInfo.particleLimit        = objectInfo.tmpParticleLimit;
        objectInfo.extraOpacityFactor   = objectInfo.tmpExtraOpacityFactor;

        objectInfo.coordinateX          = objectInfo.tmpCoordinateX;
        objectInfo.coordinateY          = objectInfo.tmpCoordinateY;
        objectInfo.coordinateZ          = objectInfo.tmpCoordinateZ;

        if( objectInfo.polygonColor.r() != objectInfo.tmpPolygonColor.r() ||
            objectInfo.polygonColor.g() != objectInfo.tmpPolygonColor.g() ||
            objectInfo.polygonColor.b() != objectInfo.tmpPolygonColor.b() )
        {
            objectInfo.polygonColor     = objectInfo.tmpPolygonColor;
            objectInfo.needSameTimeStepReplace = true;
        }
        if( objectInfo.polygonOpacity != objectInfo.tmpPolygonOpacity )
        {
            objectInfo.polygonOpacity   = objectInfo.tmpPolygonOpacity;
            objectInfo.needSameTimeStepReplace = true;
        }

        // フォーカス対象のオブジェクトに対して、全体の最小・最大座標を更新する。
        if( objectInfo.isFocus )
        {
            resultMinObjectCoords.x() = std::min( resultMinObjectCoords.x(), objectInfo.minObjectCoord.x() );
            resultMinObjectCoords.y() = std::min( resultMinObjectCoords.y(), objectInfo.minObjectCoord.y() );
            resultMinObjectCoords.z() = std::min( resultMinObjectCoords.z(), objectInfo.minObjectCoord.z() );

            resultMaxObjectCoords.x() = std::max( resultMaxObjectCoords.x(), objectInfo.maxObjectCoord.x() );
            resultMaxObjectCoords.y() = std::max( resultMaxObjectCoords.y(), objectInfo.maxObjectCoord.y() );
            resultMaxObjectCoords.z() = std::max( resultMaxObjectCoords.z(), objectInfo.maxObjectCoord.z() );
        }

        QVariant newVar;
        newVar.setValue( objectInfo );
        nameItem->setData( newVar, Qt::UserRole );
    }

    // resultObjectCoordsの反映
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem         = m_model->item( row, 0 );
        QStandardItem* displayItem      = m_model->item( row, 2 );
        QStandardItem* keepInitialItem  = m_model->item( row, 3 );
        QStandardItem* keepFinalItem    = m_model->item( row, 4 );
        if( !nameItem || !displayItem || !keepInitialItem || !keepFinalItem ) continue;

        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo objectInfo = var.value<ObjectInfoExtractor::ObjectInfo>();

        objectInfo.currentMinObjectCoord = resultMinObjectCoords;
        objectInfo.currentMaxObjectCoord = resultMaxObjectCoords;

        if( objectInfo.objectID.first != -1 && objectInfo.objectID.second != -1 )
        {
            m_screen->scene()->object( objectInfo.objectID.first )->setMinMaxObjectCoords( resultMinObjectCoords, resultMaxObjectCoords );
            m_screen->scene()->object( objectInfo.objectID.first )->setMinMaxExternalCoords( resultMinObjectCoords, resultMaxObjectCoords );
        }

        QVariant newVar;
        newVar.setValue( objectInfo );
        nameItem->setData( newVar, Qt::UserRole );
    }

    m_screen->scene()->objectManager()->push_centering_xform();
    emit updateFocus( resultMinObjectCoords, resultMaxObjectCoords ); // NOTE:Plot Over Line用
    m_screen->scene()->objectManager()->updateMinMaxCoords();
    m_screen->scene()->objectManager()->updateExternalCoords();
    m_screen->scene()->objectManager()->pop_centering_xform();
    emit updateTranslation(); // NOTE:Plot Over Line用
    m_screen->update();

    switch( *m_viz_mode )
    {
    case Viz::Mode::Local:
    case Viz::Mode::LocalClientAndServer:
    {
        break;
    }
    case Viz::Mode::RemoteClientAndServer:
    case Viz::Mode::RemoteInSitu:
    {
        if( m_web_sockets->isConnected() ) // FIXME:以下でObjectInfoの内容をサーバ側へ送信していますが、必要なもの並びに更新があったもののみ送信するようにしてください。
        {
            QJsonObject root;
            root["event"] = "objectInfoUpdate";

            QJsonArray objectInfoArray;
            for( int row = 0; row < m_model->rowCount(); row++ )
            {
                QStandardItem* nameItem = m_model->item( row, 0 );
                if( !nameItem ) continue;
                QVariant var = nameItem->data( Qt::UserRole );
                if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

                ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

                QJsonObject jsonObjectInfo;
                // Common Object Info
                jsonObjectInfo["uuid"]                      = QString::fromUtf8( info.uuid );
                jsonObjectInfo["tmpIsDisplay"]              = info.tmpIsDisplay;
                jsonObjectInfo["isDisplay"]                 = info.isDisplay;
                jsonObjectInfo["tmpIsKeepInitial"]          = info.tmpIsKeepInitial;
                jsonObjectInfo["isKeepInitial"]             = info.isKeepInitial;
                jsonObjectInfo["tmpIsKeepFinal"]            = info.tmpIsKeepFinal;
                jsonObjectInfo["isKeepFinal"]               = info.isKeepFinal;

                jsonObjectInfo["name"]                      = QString::fromUtf8( info.name );
                jsonObjectInfo["extension"]                 = QString::fromUtf8( info.extension );
                jsonObjectInfo["directory"]                 = QString::fromUtf8( info.directory );
                jsonObjectInfo["format"]                    = info.format;
                QJsonArray timeStepArray;
                timeStepArray.append( info.timeStep.first );
                timeStepArray.append( info.timeStep.second );
                jsonObjectInfo["timeStep"]                  = timeStepArray;
                jsonObjectInfo["tmpIsFocus"]                = info.tmpIsFocus;
                jsonObjectInfo["isFocus"]                   = info.isFocus;
                QJsonArray minObjectCoordArray;
                minObjectCoordArray.append( info.minObjectCoord.x() );
                minObjectCoordArray.append( info.minObjectCoord.y() );
                minObjectCoordArray.append( info.minObjectCoord.z() );
                jsonObjectInfo["minObjectCoord"]            = minObjectCoordArray;
                QJsonArray maxObjectCoordArray;
                maxObjectCoordArray.append( info.maxObjectCoord.x() );
                maxObjectCoordArray.append( info.maxObjectCoord.y() );
                maxObjectCoordArray.append( info.maxObjectCoord.z() );
                jsonObjectInfo["maxObjectCoord"]            = maxObjectCoordArray;
                QJsonArray minExternalCoordArray;
                minExternalCoordArray.append( info.minExternalCoord.x() );
                minExternalCoordArray.append( info.minExternalCoord.y() );
                minExternalCoordArray.append( info.minExternalCoord.z() );
                jsonObjectInfo["minExternalCoord"]          = minExternalCoordArray;
                QJsonArray maxExternalCoordArray;
                maxExternalCoordArray.append( info.maxExternalCoord.x() );
                maxExternalCoordArray.append( info.maxExternalCoord.y() );
                maxExternalCoordArray.append( info.maxExternalCoord.z() );
                jsonObjectInfo["maxExternalCoord"]          = maxExternalCoordArray;

                // Common Server Point Object Info
                jsonObjectInfo["tmpParticleLimit"]          = info.tmpParticleLimit;
                jsonObjectInfo["particleLimit"]             = info.particleLimit;
                jsonObjectInfo["tmpExtraOpacityFactor"]     = info.tmpExtraOpacityFactor;
                jsonObjectInfo["extraOpacityFactor"]        = info.extraOpacityFactor;

                // Client Server Point Object Info
                jsonObjectInfo["numberOfVector"]            = info.numberOfVector;
                jsonObjectInfo["numberOfElements"]          = info.numberOfElements;
                jsonObjectInfo["numberOfSubvolume"]         = info.numberOfSubvolume;
                jsonObjectInfo["numberOfNodes"]             = info.numberOfNodes;
                jsonObjectInfo["elementType"]               = info.elementType;
                jsonObjectInfo["fileType"]                  = info.fileType;
                jsonObjectInfo["stepNumber"]                = info.stepNumber;
                jsonObjectInfo["tmpCoordinateX"]            = QString::fromUtf8( info.tmpCoordinateX );
                jsonObjectInfo["coordinateX"]               = QString::fromUtf8( info.coordinateX );
                jsonObjectInfo["tmpCoordinateY"]            = QString::fromUtf8( info.tmpCoordinateY );
                jsonObjectInfo["coordinateY"]               = QString::fromUtf8( info.coordinateY );
                jsonObjectInfo["tmpCoordinateZ"]            = QString::fromUtf8( info.tmpCoordinateZ );
                jsonObjectInfo["coordinateZ"]               = QString::fromUtf8( info.coordinateZ );
                jsonObjectInfo["isExport"]                  = info.isExport;

                // Nontexture Polygon Object Info
                QJsonArray tmpPolygonColorArray;
                tmpPolygonColorArray.append( info.tmpPolygonColor.r() );
                tmpPolygonColorArray.append( info.tmpPolygonColor.g() );
                tmpPolygonColorArray.append( info.tmpPolygonColor.b() );
                jsonObjectInfo["tmpPolygonColor"]           = tmpPolygonColorArray;
                QJsonArray polygonColorArray;
                polygonColorArray.append( info.polygonColor.r() );
                polygonColorArray.append( info.polygonColor.g() );
                polygonColorArray.append( info.polygonColor.b() );
                jsonObjectInfo["polygonColor"]              = polygonColorArray;
                jsonObjectInfo["tmpPolygonOpacity"]         = info.tmpPolygonOpacity;
                jsonObjectInfo["polygonOpacity"]            = info.polygonOpacity;

                // For Client
                // jsonObjectInfo["object"]                    = info.object;
                // jsonObjectInfo["objectID"]                  = info.objectID;
                // jsonObjectInfo["currentMinObjectCoord"]     = info.currentMinObjectCoord;
                // jsonObjectInfo["currentMaxObjectCoord"]     = info.currentMaxObjectCoord;
                // jsonObjectInfo["currentImportedTimeStep"]   = info.currentImportedTimeStep;
                // jsonObjectInfo["needSameTimeStepReplace"]   = info.needSameTimeStepReplace;
                objectInfoArray.append( jsonObjectInfo );
            }
            root["objects"] = objectInfoArray;
            m_web_sockets->text()->sendTextMessage( QJsonDocument( root ).toJson( QJsonDocument::Compact ) );
        }
        break;
    }
    default:
        break;
    }
}

void ObjectEditorWIP::doneObjectEditor( int requestTimeStep )
{
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* item = m_model->item( row, 0 ); // UserRoleにObjectInfoが格納されている列
        if( !item ) continue;

        QVariant var = item->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();        
        if( info.object != nullptr )
        {
            info.object->setXform( m_screen->scene()->objectManager()->xform() );
            if( info.objectID.first == -1 && info.objectID.second == -1 )
            {
                registerObject( info );
            }
            else
            {
                replaceObject( info );
            }
        }
        QVariant newVar;
        newVar.setValue( info );
        item->setData( newVar, Qt::UserRole );
    }

    emit done( requestTimeStep );
    m_screen->update();
}
