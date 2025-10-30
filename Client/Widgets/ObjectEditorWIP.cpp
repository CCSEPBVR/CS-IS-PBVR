#include "ObjectEditorWIP.h"
#include "ui_ObjectEditorWIP.h"

ObjectEditorWIP::ObjectEditorWIP( WebSocketPair* websockets, kvs::qt::jaea::Screen* screen, QWidget *parent )
    : QDockWidget(parent)
    , ui(new Ui::ObjectEditorWIP)
    , m_web_sockets( websockets )
    , m_screen( screen )
{
    initialize();
}

ObjectEditorWIP::~ObjectEditorWIP()
{
    delete ui;
}

void ObjectEditorWIP::updateOperatorState( bool operatorState )
{
    ui->focusCheckBox           ->setEnabled( operatorState );
    ui->particleLimitSpinBox    ->setEnabled( operatorState );
    ui->densityDoubleSpinBox    ->setEnabled( operatorState );
    ui->coordinateXLineEdit     ->setEnabled( operatorState );
    ui->coordinateYLineEdit     ->setEnabled( operatorState );
    ui->coordinateZLineEdit     ->setEnabled( operatorState );
    ui->exportPushButton        ->setEnabled( operatorState );
    ui->colorClickableLabel     ->setEnabled( operatorState );
    ui->opacityDoubleSpinBox    ->setEnabled( operatorState );
    ui->browsePushButton        ->setEnabled( operatorState );
    ui->deletePushButton        ->setEnabled( operatorState );
    ui->applyPushButton         ->setEnabled( operatorState );
}

void ObjectEditorWIP::reset()
{
    ui->focusCheckBox           ->setEnabled( true );
    ui->particleLimitSpinBox    ->setEnabled( true );
    ui->densityDoubleSpinBox    ->setEnabled( true );
    ui->coordinateXLineEdit     ->setEnabled( true );
    ui->coordinateYLineEdit     ->setEnabled( true );
    ui->coordinateZLineEdit     ->setEnabled( true );
    ui->exportPushButton        ->setEnabled( true );
    ui->colorClickableLabel     ->setEnabled( true );
    ui->opacityDoubleSpinBox    ->setEnabled( true );
    ui->browsePushButton        ->setEnabled( true );
    ui->deletePushButton        ->setEnabled( true );
    ui->applyPushButton         ->setEnabled( true );

    toggleCommonObjectWidgets( false );
    toggleCommonServerObjectWidgets( false );
    toggleClientServerObjectWidgets( false );
    toggleNontexturePolygonObjectWidgets( false );

    if( m_model )
    {
        m_model->removeRows( 0, m_model->rowCount() );
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

void ObjectEditorWIP::showAtTimeStep( int timeStep )
{
    Worker* worker = new Worker( timeStep, m_model, m_screen );
    QThread* thread = new QThread;

    worker->moveToThread( thread );

    connect( thread, &QThread::started, worker, &Worker::process );
    connect( worker, &Worker::done, thread, &QThread::quit );
    connect( worker, &Worker::done, worker, &Worker::deleteLater );
    connect( thread, &QThread::finished, thread, &QThread::deleteLater );
    connect( worker, &Worker::done, this, [this, timeStep]() { onDone( timeStep ); } );

    thread->start();
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
            ui->particleLimitLabel  , ui->particleLimitSpinBox,
            ui->densityLabel        , ui->densityDoubleSpinBox,
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
    connect( ui->densityDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &ObjectEditorWIP::onDensityDoubleSpinBoxValueChanged );

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

void ObjectEditorWIP::addObjectToModel( const ObjectInfoExtractor::ObjectInfo& objectInfo, ObjectInfoExtractor& oie )
{
    ObjectInfoExtractor::ObjectInfo info = objectInfo;

    // 最初に追加されるオブジェクトはフォーカス状態にする
    if( m_model->rowCount() == 0 )
    {
        info.isFocus = true;
    }

    QList<QStandardItem*> rowItems;
    auto nameItem         = new QStandardItem( QString::fromUtf8( info.name ) );
    auto formatItem       = new QStandardItem( QString::fromUtf8( oie.formatToString( info.format ) ) );
    auto displayItem      = new QStandardItem( "" );
    auto keepInitialItem  = new QStandardItem( "" );
    auto keepFinalItem    = new QStandardItem( "" );

    // 編集禁止
    nameItem        ->setEditable( false );
    formatItem      ->setEditable( false );
    displayItem     ->setEditable( false );
    keepInitialItem ->setEditable( false );
    keepFinalItem   ->setEditable( false );

    QVariant var;
    var.setValue( info );
    nameItem->setData( var, Qt::UserRole );

    rowItems << nameItem << formatItem << displayItem << keepInitialItem << keepFinalItem;
    m_model->appendRow( rowItems );

    QModelIndex index = m_model->indexFromItem( rowItems.first() );
    ui->treeView->setCurrentIndex( index );

    calculateTotalMinMaxTimeStep();
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
    // FIXME:ツールバー通知用のシグナルを発火してください。
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
        info.objectID = m_screen->registerObject( static_cast<kvs::PointObject*>(info.object), particleBasedRenderer.release() );
        break;
    case ObjectInfoExtractor::PolygonObjectKVSML:
    case ObjectInfoExtractor::PolygonObjectSTL:
        stochasticPolygonRenderer = std::make_unique<kvs::StochasticPolygonRenderer>();
        info.objectID = m_screen->registerObject( info.object, stochasticPolygonRenderer.release() );
        break;
#ifdef ASSIMP
    case ObjectInfoExtractor::PolygonObject3DS:
    case ObjectInfoExtractor::PolygonObjectFBX:
        stochasticTexturedPolygonRenderer = std::make_unique<kvs::StochasticTexturedPolygonRenderer>();
        info.objectID = m_screen->registerObject( info.object, stochasticTexturedPolygonRenderer.release() );
        break;
#endif
    case ObjectInfoExtractor::LineObjectKVSML:
        stochasticLineRenderer = std::make_unique<kvs::StochasticLineRenderer>();
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
    if (selected.indexes().isEmpty()) return;

    QModelIndex index = selected.indexes().first();
    QVariant var = index.data(Qt::UserRole);
    if (!var.canConvert<ObjectInfoExtractor::ObjectInfo>()) return;

    ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

    // 全オブジェクト共通
    ui->nameLineEdit                ->setText( QString::fromUtf8( info.name.c_str() ) );
    ui->directoryLineEdit           ->setText( QString::fromUtf8( info.directory.c_str() ) );
    ui->formatLineEdit              ->setText( QString::fromUtf8( ObjectInfoExtractor::formatToString( info.format ) ) );
    ui->timeStepMinLineEdit         ->setText( QString::number( info.timeStep.first ) );
    ui->timeStepMaxLineEdit         ->setText( QString::number( info.timeStep.second ) );
    ui->focusCheckBox               ->setChecked( info.isFocus );
    ui->minObjectXCoordLineEdit     ->setText( QString::number( info.minObjectCoord.x() ) );
    ui->minObjectYCoordLineEdit     ->setText( QString::number( info.minObjectCoord.y() ) );
    ui->minObjectZCoordLineEdit     ->setText( QString::number( info.minObjectCoord.z() ) );
    ui->maxObjectXCoordLineEdit     ->setText( QString::number( info.maxObjectCoord.x() ) );
    ui->maxObjectYCoordLineEdit     ->setText( QString::number( info.maxObjectCoord.y() ) );
    ui->maxObjectZCoordLineEdit     ->setText( QString::number( info.maxObjectCoord.z() ) );
    ui->minExternalXCoordLineEdit   ->setText( QString::number( info.minExternalCoord.x() ) );
    ui->minExternalYCoordLineEdit   ->setText( QString::number( info.minExternalCoord.y() ) );
    ui->minExternalZCoordLineEdit   ->setText( QString::number( info.minExternalCoord.z() ) );
    ui->maxExternalXCoordLineEdit   ->setText( QString::number( info.maxExternalCoord.x() ) );
    ui->maxExternalYCoordLineEdit   ->setText( QString::number( info.maxExternalCoord.y() ) );
    ui->maxExternalZCoordLineEdit   ->setText( QString::number( info.maxExternalCoord.z() ) );

    // サーバポイントオブジェクトである。(ClientServer/Insitu共通)
    ui->particleLimitSpinBox        ->setValue( info.particleLimit );
    ui->densityDoubleSpinBox        ->setValue( info.density );

    // サーバポイントオブジェクト(ClientServer)で表示させるウィジェット群
    ui->numberOfVectorLineEdit      ->setText( QString::number( info.numberOfVector ) );
    ui->numberOfElementsLineEdit    ->setText( QString::number( info.numberOfElements ) );
    ui->numberOfSubvolumeLineEdit   ->setText( QString::number( info.numberOfSubvolume ) );
    ui->numberOfNodesLineEdit       ->setText( QString::number( info.numberOfNodes ) );
    ui->elementTypeLineEdit         ->setText( QString::number( info.elementType ) );
    ui->fileTypeLineEdit            ->setText( QString::number( info.fileType ) );
    ui->stepNumberLineEdit          ->setText( QString::number( info.stepNumber ) );
    ui->coordinateXLineEdit         ->setText( QString::fromUtf8( info.coordinateX ) );
    ui->coordinateYLineEdit         ->setText( QString::fromUtf8( info.coordinateY ) );
    ui->coordinateZLineEdit         ->setText( QString::fromUtf8( info.coordinateZ ) );

    // true:テクスチャ無しポリゴンオブジェクトである。(.stl, .kvsml)
    QPalette palette = ui->colorClickableLabel->palette();
    palette.setColor( QPalette::Window, QColor( info.polygonColor.r(), info.polygonColor.g(), info.polygonColor.b() ) );
    ui->colorClickableLabel         ->setPalette( palette );
    ui->opacityDoubleSpinBox        ->setValue( info.polygonOpacity );

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
                             info.isFocus = checked;
                         } );
}

void ObjectEditorWIP::onParticleLimitSpinBoxValueChanged( int value )
{
    updateSelectedObject( [value]( auto &info )
                         {
                             info.particleLimit = value;
                         } );
}

void ObjectEditorWIP::onDensityDoubleSpinBoxValueChanged(double value)
{
    updateSelectedObject( [value]( auto &info )
                         {
                             info.density = value;
                         } );
}

void ObjectEditorWIP::onCoordinateLineEditTextChanged()
{
    updateSelectedObject( [this] ( auto &info )
                         {
                             info.coordinateX = ui->coordinateXLineEdit->text().toUtf8().constData();
                             info.coordinateY = ui->coordinateYLineEdit->text().toUtf8().constData();
                             info.coordinateZ = ui->coordinateZLineEdit->text().toUtf8().constData();
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
                                 info.polygonColor.set( color.red(), color.green(), color.blue() );
                             }
                         } );
}

void ObjectEditorWIP::onOpacityDoubleSpinBoxValueChanged( double value )
{
    updateSelectedObject( [value]( auto &info )
                         {
                             info.polygonOpacity = value;
                         } );
}

void ObjectEditorWIP::onBrowse()
{
    QString filePath;
    /*
     * FIXME:
     * ローカルモード(サーバと接続せずにローカルデータのみ閲覧するモード)の場合:QFileDialogでローカルファイルを参照
     * スタンドアロンモード(クライアントとサーバを同じマシンで起動、接続するモード)の場合:QFileDialogでローカルファイルを参照
     * クラサバモード(クライアントとサーバを別マシンで起動、接続するモード)の場合:RemoteFileDialogでリモートファイルを参照
     * In-situモード(クライアントとサーバを別マシンで起動、接続するモード)の場合:RemoteFileDialogでリモートファイルを参照
     */
    filePath = QFileDialog::getOpenFileName( this, tr( "ファイルを選択" ), QString(), tr( "すべてのファイル (*.*)" ) );

    if( !filePath.isEmpty() )
    {
        ObjectInfoExtractor oie( filePath.toUtf8().constData() );
        if( auto objectInfoOpt = oie.extractFromLocalFile() )
        {
            addObjectToModel( *objectInfoOpt, oie );
        }
        else
        {
            // FIXME:MainWinodwのStatusBarで通知した方がいいかも。
        }
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
    if( info.isFocus )
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
            if( otherInfo.isFocus )
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
            info.isFocus = true;
            item->setData( QVariant::fromValue(info), Qt::UserRole );
            break; // 1つだけ更新したら終了
        }
    }
    m_model->removeRow( index.row() );
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

    if( m_web_sockets->isConnected() )
    {
        QJsonObject root;
        root["event"] = "ObjectInfoParameter";

        QJsonArray objectInfoArray;

        for( int row = 0; row < m_model->rowCount(); row++ )
        {
            QStandardItem* nameItem = m_model->item( row, 0 );
            if( !nameItem ) continue;

            QVariant var = nameItem->data( Qt::UserRole );
            if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            ObjectInfoExtractor::ObjectInfo objectInfo = var.value<ObjectInfoExtractor::ObjectInfo>();

            QJsonObject jsonObjectInfo;
            jsonObjectInfo["name"]              = QString::fromUtf8( objectInfo.name );
            jsonObjectInfo["directory"]         = QString::fromUtf8( objectInfo.directory );
            jsonObjectInfo["format"]            = QString::fromUtf8( ObjectInfoExtractor::formatToString( objectInfo.format ).c_str() );
            jsonObjectInfo["timeStepMin"]       = objectInfo.timeStep.first;
            jsonObjectInfo["timeStepMax"]       = objectInfo.timeStep.second;
            jsonObjectInfo["isFocus"]           = objectInfo.isFocus;

            jsonObjectInfo["minObjectCoord"]    = QJsonArray{ objectInfo.minObjectCoord.x(), objectInfo.minObjectCoord.y(), objectInfo.minObjectCoord.z() };
            jsonObjectInfo["maxObjectCoord"]    = QJsonArray{ objectInfo.maxObjectCoord.x(), objectInfo.maxObjectCoord.y(), objectInfo.maxObjectCoord.z() };
            jsonObjectInfo["minExternalCoord"]  = QJsonArray{ objectInfo.minExternalCoord.x(), objectInfo.minExternalCoord.y(), objectInfo.minExternalCoord.z() };
            jsonObjectInfo["maxExternalCoord"]  = QJsonArray{ objectInfo.maxExternalCoord.x(), objectInfo.maxExternalCoord.y(), objectInfo.maxExternalCoord.z() };

            jsonObjectInfo["particleLimit"]     = objectInfo.particleLimit;
            jsonObjectInfo["density"]           = objectInfo.density;

            jsonObjectInfo["numberOfVector"]    = objectInfo.numberOfVector;
            jsonObjectInfo["numberOfElements"]  = objectInfo.numberOfElements;
            jsonObjectInfo["numberOfSubvolume"] = objectInfo.numberOfSubvolume;
            jsonObjectInfo["numberOfNodes"]     = objectInfo.numberOfNodes;
            jsonObjectInfo["elementType"]       = objectInfo.elementType;
            jsonObjectInfo["fileType"]          = objectInfo.fileType;
            jsonObjectInfo["stepNumber"]        = objectInfo.stepNumber;
            jsonObjectInfo["coordinateX"]       = QString::fromUtf8( objectInfo.coordinateX );
            jsonObjectInfo["coordinateY"]       = QString::fromUtf8( objectInfo.coordinateY );
            jsonObjectInfo["coordinateZ"]       = QString::fromUtf8( objectInfo.coordinateZ );

            jsonObjectInfo["polygonColor"]      = QJsonArray{ objectInfo.polygonColor.r(), objectInfo.polygonColor.g(), objectInfo.polygonColor.b() };
            jsonObjectInfo["polygonOpacity"]    = objectInfo.polygonOpacity;

            objectInfoArray.append( jsonObjectInfo );
        }
        root["objects"] = objectInfoArray;
        qDebug() << root;
        m_web_sockets->text()->sendTextMessage( QJsonDocument( root ).toJson( QJsonDocument::Compact ) );
    }
    else // ローカルモード
    {
        // FIXME:接続中でない場合、ローカルモードと判断し表示を行う。
    }

    showAtTimeStep( 1 );
}

void ObjectEditorWIP::onDone( int requestTimeStep )
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
    m_screen->update();
}
