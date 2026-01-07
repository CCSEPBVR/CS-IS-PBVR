#include "ObjectEditor.h"
#include "ui_ObjectEditor.h"

// FIXME:ISオブジェクトのタイムステップ増加、サーバ側フォーカスのバグ、 エクスポート
ObjectEditor::ObjectEditor( kvs::qt::jaea::Screen* screen, WebSocketPair* websockets, Viz::Mode* vizMode, QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::ObjectEditor )
    , m_screen( screen )
    , m_web_sockets( websockets )
    , m_viz_mode( vizMode )
    , m_model( new QStandardItemModel( this ) )
{
    ui->setupUi( this );

    m_model->setHorizontalHeaderLabels( { "Name", "Format", "Display", "Keep Initial", "Keep Final" } );
    ui->treeView->setModel( m_model );

    ui->colorClickableLabel->setAutoFillBackground( true );

    m_group_common_object_widgets =
        {
            ui->nameLabel             , ui->nameLineEdit       ,
            ui->formatLabel           , ui->formatLineEdit     ,
            ui->directoryLabel        , ui->directoryLineEdit  ,
            ui->timeStepLabel         , ui->timeStepMinLineEdit, ui->timeStepMaxLineEdit,
            ui->focusLabel            , ui->focusCheckBox      ,
            ui->objectCoordsGroupBox  ,
            ui->externalCoordsGroupBox,
        };

    m_group_common_server_point_object_widgets =
        {
            ui->particleLimitLabel      , ui->particleLimitSpinBox,
            ui->extraOpacityFactorLabel , ui->extraOpacityFactorDoubleSpinBox,
        };

    m_group_client_server_point_object_widgets =
        {
            ui->numberOfVectorLabel   , ui->numberOfVectorLineEdit   ,
            ui->numberOfElementsLabel , ui->numberOfElementsLineEdit ,
            ui->numberOfSubvolumeLabel, ui->numberOfSubvolumeLineEdit,
            ui->numberOfNodesLabel    , ui->numberOfNodesLineEdit    ,
            ui->elementTypeLabel      , ui->elementTypeLineEdit      ,
            ui->fileTypeLabel         , ui->fileTypeLineEdit         ,
            ui->stepNumberLabel       , ui->stepNumberLineEdit       ,
            ui->coordinateLabel       , ui->coordinateXLineEdit      , ui->coordinateYLineEdit, ui->coordinateZLineEdit,
            ui->exportLabel           , ui->exportPushButton         ,
        };

    m_group_nontexture_polygon_object_widgets =
        {
            ui->colorLabel  , ui->colorClickableLabel,
            ui->opacityLabel, ui->opacityDoubleSpinBox,
        };

    // 起動直後にオブジェクト情報はないので非表示にする。
    setWidgetsVisible( m_group_common_object_widgets             , false );
    setWidgetsVisible( m_group_common_server_point_object_widgets, false );
    setWidgetsVisible( m_group_client_server_point_object_widgets, false );
    setWidgetsVisible( m_group_nontexture_polygon_object_widgets , false );

    connect( ui->treeView->selectionModel()     , &QItemSelectionModel::selectionChanged, this, &ObjectEditor::onItemSelection );

    // 全オブジェクト共通
    connect( ui->focusCheckBox                  , &QCheckBox::toggled                   , this, &ObjectEditor::onFocusCheckBoxToggled );
    // サーバポイントオブジェクト(ClientServer/In-Situ共通)
    connect( ui->particleLimitSpinBox           , &QSpinBox::valueChanged               , this, &ObjectEditor::onParticleLimitSpinBoxValueChanged );
    connect( ui->extraOpacityFactorDoubleSpinBox, &QDoubleSpinBox::valueChanged         , this, &ObjectEditor::onExtraOpacityFactorDoubleSpinBoxValueChanged );
    // サーバポイントオブジェクト(ClientServerのみ)
    connect( ui->coordinateXLineEdit            , &QLineEdit::textChanged               , this, &ObjectEditor::onCoordinateLineEditTextChanged );
    connect( ui->coordinateYLineEdit            , &QLineEdit::textChanged               , this, &ObjectEditor::onCoordinateLineEditTextChanged );
    connect( ui->coordinateZLineEdit            , &QLineEdit::textChanged               , this, &ObjectEditor::onCoordinateLineEditTextChanged );
    // connect( ui->exportPushButton               , &QPushButton::clicked                 , this, &ObjectEditor::onExportButtonClicked );
    // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
    connect( ui->colorClickableLabel            , &ClickableLabel::doubleClicked        , this, &ObjectEditor::onColorLabelDoubleClicked );
    connect( ui->opacityDoubleSpinBox           , &QDoubleSpinBox::valueChanged         , this, &ObjectEditor::onOpacityDoubleSpinBoxValueChanged );

    connect( ui->browsePushButton               , &QPushButton::clicked                 , this, &ObjectEditor::onBrowse );
    connect( ui->deletePushButton               , &QPushButton::clicked                 , this, &ObjectEditor::onDelete );
    connect( ui->applyPushButton                , &QPushButton::clicked                 , this, &ObjectEditor::onApply );
}

ObjectEditor::~ObjectEditor()
{
    delete ui;
}

void ObjectEditor::onUpdateServerState( bool serverState ) // true:接続中
{}

void ObjectEditor::onOperatorStateUpdate( bool operatorState ) // true:権限あり
{}

void ObjectEditor::onReset()
{}

void ObjectEditor::onReceiveSelectedFile( const QJsonObject& dataArray )
{
    ObjectInfoExtractor::ObjectInfo objectInfo;
    // 全オブジェクト共通
    objectInfo.uuid                  = dataArray[QString::fromUtf8( Protocol::Key::UUID )].toString().toUtf8();
    objectInfo.tmpIsDisplay          = dataArray[QString::fromUtf8( Protocol::Key::TmpIsDisplay )].toBool();
    objectInfo.isDisplay             = dataArray[QString::fromUtf8( Protocol::Key::IsDisplay )].toBool();
    objectInfo.tmpIsKeepInitial      = dataArray[QString::fromUtf8( Protocol::Key::TmpIsKeepInitial )].toBool();
    objectInfo.isKeepInitial         = dataArray[QString::fromUtf8( Protocol::Key::IsKeepInitial )].toBool();
    objectInfo.tmpIsKeepFinal        = dataArray[QString::fromUtf8( Protocol::Key::TmpIsKeepFinal )].toBool();
    objectInfo.isKeepFinal           = dataArray[QString::fromUtf8( Protocol::Key::IsKeepFinal )].toBool();
    objectInfo.name                  = dataArray[QString::fromUtf8( Protocol::Key::Name )].toString().toUtf8();
    objectInfo.extension             = dataArray[QString::fromUtf8( Protocol::Key::Extension )].toString().toUtf8();
    objectInfo.directory             = dataArray[QString::fromUtf8( Protocol::Key::Directory )].toString().toUtf8();
    objectInfo.format                = static_cast<ObjectInfoExtractor::Format>( dataArray[QString::fromUtf8( Protocol::Key::Format )].toInt() );
    QJsonArray timeStepArray         = dataArray[QString::fromUtf8( Protocol::Key::TimeStep )].toArray();
    objectInfo.timeStep              = { timeStepArray[0].toInt(), timeStepArray[1].toInt() };
    objectInfo.tmpIsFocus            = dataArray[QString::fromUtf8( Protocol::Key::TmpIsFocus )].toBool();
    objectInfo.isFocus               = dataArray[QString::fromUtf8( Protocol::Key::IsFocus )].toBool();
    auto minObjectCoordArray         = dataArray[QString::fromUtf8( Protocol::Key::MinObjectCoord )].toArray();
    objectInfo.minObjectCoord        = { minObjectCoordArray[0].toDouble(), minObjectCoordArray[1].toDouble(), minObjectCoordArray[2].toDouble() };
    auto maxObjectCoordArray         = dataArray[QString::fromUtf8( Protocol::Key::MaxObjectCoord )].toArray();
    objectInfo.maxObjectCoord        = { maxObjectCoordArray[0].toDouble(), maxObjectCoordArray[1].toDouble(), maxObjectCoordArray[2].toDouble() };
    auto minExternalCoordArray       = dataArray[QString::fromUtf8( Protocol::Key::MinExternalCoord )].toArray();
    objectInfo.minExternalCoord      = { minExternalCoordArray[0].toDouble(), minExternalCoordArray[1].toDouble(), minExternalCoordArray[2].toDouble() };
    auto maxExternalCoordArray       = dataArray[QString::fromUtf8( Protocol::Key::MaxExternalCoord )].toArray();
    objectInfo.maxExternalCoord      = { maxExternalCoordArray[0].toDouble(), maxExternalCoordArray[1].toDouble(), maxExternalCoordArray[2].toDouble() };
    // サーバポイントオブジェクト(ClientServer/In-Situ共通)
    objectInfo.tmpParticleLimit      = dataArray[QString::fromUtf8( Protocol::Key::TmpParticleLimit )].toInt();
    objectInfo.particleLimit         = dataArray[QString::fromUtf8( Protocol::Key::ParticleLimit )].toInt();
    objectInfo.tmpExtraOpacityFactor = static_cast<float>( dataArray[QString::fromUtf8( Protocol::Key::TmpExtraOpacityFactor )].toDouble() );
    objectInfo.extraOpacityFactor    = static_cast<float>( dataArray[QString::fromUtf8( Protocol::Key::ExtraOpacityFactor )].toDouble() );
    // サーバポイントオブジェクト(ClientServerのみ)
    objectInfo.numberOfVector        = dataArray[QString::fromUtf8( Protocol::Key::NumberOfVector )].toInt();
    objectInfo.numberOfElements      = dataArray[QString::fromUtf8( Protocol::Key::NumberOfElements )].toInt();
    objectInfo.numberOfSubvolume     = dataArray[QString::fromUtf8( Protocol::Key::NumberOfSubvolume)].toInt();
    objectInfo.numberOfNodes         = dataArray[QString::fromUtf8( Protocol::Key::NumberOfNodes )].toInt();
    objectInfo.elementType           = dataArray[QString::fromUtf8( Protocol::Key::ElementType )].toInt();
    objectInfo.fileType              = dataArray[QString::fromUtf8( Protocol::Key::FileType )].toInt();
    objectInfo.stepNumber            = dataArray[QString::fromUtf8( Protocol::Key::StepNumber )].toInt();
    objectInfo.tmpCoordinateX        = dataArray[QString::fromUtf8( Protocol::Key::TmpCoordinateX )].toString().toUtf8();
    objectInfo.coordinateX           = dataArray[QString::fromUtf8( Protocol::Key::CoordinateX )].toString().toUtf8();
    objectInfo.tmpCoordinateY        = dataArray[QString::fromUtf8( Protocol::Key::TmpCoordinateY )].toString().toUtf8();
    objectInfo.coordinateY           = dataArray[QString::fromUtf8( Protocol::Key::CoordinateY )].toString().toUtf8();
    objectInfo.tmpCoordinateZ        = dataArray[QString::fromUtf8( Protocol::Key::TmpCoordinateZ )].toString().toUtf8();
    objectInfo.coordinateZ           = dataArray[QString::fromUtf8( Protocol::Key::IsExport )].toBool();
    // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
    auto tmpPolygonColorArray        = dataArray[QString::fromUtf8( Protocol::Key::TmpPolygonColor )].toArray();
    objectInfo.tmpPolygonColor       = kvs::RGBColor( tmpPolygonColorArray[0].toInt(), tmpPolygonColorArray[1].toInt(), tmpPolygonColorArray[2].toInt() );
    auto polygonColorArray           = dataArray[QString::fromUtf8( Protocol::Key::PolygonColor )].toArray();
    objectInfo.polygonColor          = kvs::RGBColor( polygonColorArray[0].toInt(), polygonColorArray[1].toInt(), polygonColorArray[2].toInt() );
    objectInfo.tmpPolygonOpacity     = static_cast<float>( dataArray[QString::fromUtf8( Protocol::Key::TmpPolygonOpacity )].toDouble() );
    objectInfo.polygonOpacity        = static_cast<float>( dataArray[QString::fromUtf8( Protocol::Key::PolygonOpacity )].toDouble() );

    addObjectToModel( objectInfo );
}

void ObjectEditor::onReceiveObjectDelete( const QJsonObject& data )
{
    if( !data.contains( QString::fromUtf8( Protocol::Key::UUID ) ) ) return;

    const QString uuid = data[QString::fromUtf8( Protocol::Key::UUID )].toString();

    bool deletedWasFocus = false;

    for( int row = 0; row < m_model->rowCount(); ++row )
    {
        QStandardItem* item = m_model->item( row, 0 );
        if( !item ) continue;

        QVariant var = item->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        auto info = var.value<ObjectInfoExtractor::ObjectInfo>();
        if( QString::fromUtf8( info.uuid ) != uuid ) continue;

        deletedWasFocus = info.tmpIsFocus;  // ★削除前に覚える

        // --- Scene ---
        if( info.objectID.first != -1 ) m_screen->scene()->removeObject( info.objectID.first );

        // --- Model ---
        m_model->removeRow( row );
        break;
    }

    // ★フォーカス再割当（必要なら）
    if( deletedWasFocus )
    {
        bool otherFocusedFound = false;
        for( int row = 0; row < m_model->rowCount(); ++row )
        {
            QStandardItem* item = m_model->item( row, 0 );
            if( !item ) continue;

            QVariant var = item->data( Qt::UserRole );
            if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            auto info = var.value<ObjectInfoExtractor::ObjectInfo>();
            if( info.tmpIsFocus ) { otherFocusedFound = true; break; }
        }

        if( !otherFocusedFound && m_model->rowCount() > 0 )
        {
            // 先頭でも良いし、直前/直後優先など方針は好み
            QStandardItem* item = m_model->item( 0, 0 );
            if( item )
            {
                QVariant var = item->data( Qt::UserRole );
                if( var.canConvert<ObjectInfoExtractor::ObjectInfo>() )
                {
                    auto info = var.value<ObjectInfoExtractor::ObjectInfo>();
                    info.tmpIsFocus = true;
                    item->setData( QVariant::fromValue( info ), Qt::UserRole );
                }
            }
        }
    }

    calculateTotalMinMaxTimeStep();

    if( m_model->rowCount() == 0 )
    {
        setWidgetsVisible( m_group_common_object_widgets             , false );
        setWidgetsVisible( m_group_common_server_point_object_widgets, false );
        setWidgetsVisible( m_group_client_server_point_object_widgets, false );
        setWidgetsVisible( m_group_nontexture_polygon_object_widgets , false );
    }

    m_screen->update();

    QModelIndex current = ui->treeView->currentIndex();
    if( current.isValid() )
    {
        QModelIndex idx0 = current.sibling( current.row(), 0 ); // UserRoleが入ってる列に合わせる
        updateEditorFromIndex( idx0 );
    }
}

void ObjectEditor::onReceiveObjectInfoParameter( const QJsonObject& dataArray )
{
    if( dataArray.contains( QString::fromUtf8( Protocol::Key::ResultMinObjectCoords ) ) )
    {
        QJsonArray a = dataArray[QString::fromUtf8( Protocol::Key::ResultMinObjectCoords )].toArray();
        if( a.size() == 3 )
        {
            m_result_min_object_coords = kvs::Vec3( a[0].toDouble(), a[1].toDouble(), a[2].toDouble() );
        }
    }

    if( dataArray.contains( QString::fromUtf8( Protocol::Key::ResultMaxObjectCoords ) ) )
    {
        QJsonArray a = dataArray[QString::fromUtf8( Protocol::Key::ResultMaxObjectCoords )].toArray();
        if( a.size() == 3 )
        {
            m_result_max_object_coords = kvs::Vec3( a[0].toDouble(), a[1].toDouble(), a[2].toDouble() );
        }
    }

    if( dataArray.contains( QString::fromUtf8( Protocol::Key::Objects ) ) && dataArray[QString::fromUtf8( Protocol::Key::Objects )].isArray() )
    {
        const QJsonArray objects = dataArray[QString::fromUtf8( Protocol::Key::Objects )].toArray();

        for( const QJsonValue& value : objects )
        {
            if( !value.isObject() ) continue;

            const QJsonObject patch = value.toObject();

            if( !patch.contains( QString::fromUtf8( Protocol::Key::UUID ) ) ) continue;

            const QString uuid = patch[QString::fromUtf8( Protocol::Key::UUID )].toString();

            for( int row = 0; row < m_model->rowCount(); ++row )
            {
                QStandardItem* nameItem         = m_model->item( row, 0 );
                QStandardItem* displayItem      = m_model->item( row, 2 );
                QStandardItem* keepInitialItem  = m_model->item( row, 3 );
                QStandardItem* keepFinalItem    = m_model->item( row, 4 );
                if( !nameItem ) continue;

                QVariant var = nameItem->data( Qt::UserRole );
                if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

                ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

                if( QString::fromUtf8( info.uuid ) != uuid ) continue;

                // 全オブジェクト共通
                if( patch.contains( QString::fromUtf8( Protocol::Key::IsDisplay ) ) )
                {
                    info.tmpIsDisplay = patch[QString::fromUtf8( Protocol::Key::IsDisplay )].toBool();
                    info.isDisplay    = patch[QString::fromUtf8( Protocol::Key::IsDisplay )].toBool();
                    displayItem       ->setCheckState( info.tmpIsDisplay     ? Qt::Checked : Qt::Unchecked );
                }
                if( patch.contains( QString::fromUtf8( Protocol::Key::IsKeepInitial ) ) )
                {
                    info.tmpIsKeepInitial = patch[QString::fromUtf8( Protocol::Key::IsKeepInitial )].toBool();
                    info.isKeepInitial    = patch[QString::fromUtf8( Protocol::Key::IsKeepInitial )].toBool();
                    keepInitialItem       ->setCheckState( info.tmpIsKeepInitial ? Qt::Checked : Qt::Unchecked );
                }
                if( patch.contains( QString::fromUtf8( Protocol::Key::IsKeepFinal ) ) )
                {
                    info.tmpIsKeepFinal = patch[QString::fromUtf8( Protocol::Key::IsKeepFinal )].toBool();
                    info.isKeepFinal    = patch[QString::fromUtf8( Protocol::Key::IsKeepFinal )].toBool();
                    keepFinalItem       ->setCheckState( info.tmpIsKeepFinal ? Qt::Checked : Qt::Unchecked );
                }
                if( patch.contains( QString::fromUtf8( Protocol::Key::IsFocus ) ) )
                {
                    info.tmpIsFocus = patch[QString::fromUtf8( Protocol::Key::IsFocus )].toBool();
                    info.isFocus    = patch[QString::fromUtf8( Protocol::Key::IsFocus )].toBool();
                }
                // サーバポイントオブジェクト(ClientServer/In-Situ共通)
                if( patch.contains( QString::fromUtf8( Protocol::Key::ParticleLimit ) ) )
                {
                    info.tmpParticleLimit = patch[QString::fromUtf8( Protocol::Key::ParticleLimit )].toInt();
                    info.particleLimit    = patch[QString::fromUtf8( Protocol::Key::ParticleLimit )].toInt();
                }
                if( patch.contains( QString::fromUtf8( Protocol::Key::ExtraOpacityFactor ) ) )
                {
                    info.tmpExtraOpacityFactor = static_cast<float>( patch[QString::fromUtf8( Protocol::Key::ExtraOpacityFactor )].toDouble() );
                    info.extraOpacityFactor    = static_cast<float>( patch[QString::fromUtf8( Protocol::Key::ExtraOpacityFactor )].toDouble() );
                }
                // サーバポイントオブジェクト(ClientServerのみ)
                if( patch.contains( QString::fromUtf8( Protocol::Key::CoordinateX ) ) )
                {
                    info.tmpCoordinateX = patch[QString::fromUtf8( Protocol::Key::CoordinateX )].toString().toUtf8().constData();
                    info.coordinateX    = patch[QString::fromUtf8( Protocol::Key::CoordinateX )].toString().toUtf8().constData();
                }
                if( patch.contains( QString::fromUtf8( Protocol::Key::CoordinateY ) ) )
                {
                    info.tmpCoordinateY = patch[QString::fromUtf8( Protocol::Key::CoordinateY )].toString().toUtf8().constData();
                    info.coordinateY    = patch[QString::fromUtf8( Protocol::Key::CoordinateY )].toString().toUtf8().constData();
                }
                if( patch.contains( QString::fromUtf8( Protocol::Key::CoordinateZ ) ) )
                {
                    info.tmpCoordinateZ = patch[QString::fromUtf8( Protocol::Key::CoordinateZ )].toString().toUtf8().constData();
                    info.coordinateZ    = patch[QString::fromUtf8( Protocol::Key::CoordinateZ )].toString().toUtf8().constData();
                }
                // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
                if( patch.contains( QString::fromUtf8( Protocol::Key::PolygonColor ) ) )
                {
                    QJsonArray c = patch[QString::fromUtf8( Protocol::Key::PolygonColor )].toArray();
                    if( c.size() == 3 )
                    {
                        info.polygonColor            = kvs::RGBColor( c[0].toInt(), c[1].toInt(), c[2].toInt() );
                        info.tmpPolygonColor            = kvs::RGBColor( c[0].toInt(), c[1].toInt(), c[2].toInt() );
                        info.needSameTimeStepReplace = true;
                    }
                }
                if( patch.contains( QString::fromUtf8( Protocol::Key::PolygonOpacity ) ) )
                {
                    info.polygonOpacity          = static_cast<float>( patch[QString::fromUtf8( Protocol::Key::PolygonOpacity )].toDouble() );
                    info.tmpPolygonOpacity       = static_cast<float>( patch[QString::fromUtf8( Protocol::Key::PolygonOpacity )].toDouble() );
                    info.needSameTimeStepReplace = true;
                }
                nameItem->setData( QVariant::fromValue( info ), Qt::UserRole );
                break;
            }
        }
    }

    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem = m_model->item( row, 0 );
        if( !nameItem ) continue;

        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        auto info = var.value<ObjectInfoExtractor::ObjectInfo>();
        if( info.objectID.first != -1 && info.objectID.second != -1 )
        {
            m_screen->scene()->object( info.objectID.first )->setMinMaxObjectCoords( m_result_min_object_coords, m_result_max_object_coords );
            m_screen->scene()->object( info.objectID.first )->setMinMaxExternalCoords( m_result_min_object_coords, m_result_max_object_coords );
        }
    }

    m_screen->scene()->objectManager()->push_centering_xform();
    emit updateFocus( m_result_min_object_coords, m_result_max_object_coords );
    m_screen->scene()->objectManager()->updateMinMaxCoords();
    m_screen->scene()->objectManager()->updateExternalCoords();
    m_screen->scene()->objectManager()->pop_centering_xform();
    emit updateTranslation();
    m_screen->update();

    QModelIndex current = ui->treeView->currentIndex();
    if( current.isValid() )
    {
        QModelIndex idx0 = current.sibling( current.row(), 0 ); // UserRoleが入ってる列に合わせる
        updateEditorFromIndex( idx0 );
    }
}

void ObjectEditor::onRequestDataAt( int requestTimeStep )
{
    switch( *m_viz_mode )
    {
    case Viz::Mode::Local:
    case Viz::Mode::LocalClientAndServer:
    {
        Worker* worker = new Worker( m_model, m_screen, requestTimeStep, m_result_min_object_coords, m_result_max_object_coords );
        QThread* thread = new QThread;

        worker->moveToThread( thread );

        connect( thread, &QThread::started , worker, &Worker::process );
        connect( worker, &Worker::done     , thread, &QThread::quit );
        connect( worker, &Worker::done     , worker, &Worker::deleteLater );
        connect( thread, &QThread::finished, thread, &QThread::deleteLater );
        connect( worker, &Worker::done     , this  , [this, requestTimeStep]() { dataRequestComplete( requestTimeStep ); } );

        thread->start();
        break;
    }
    case Viz::Mode::RemoteClientAndServer:
    case Viz::Mode::RemoteInSitu:
    {
        if( m_web_sockets->isConnected() )
        {
            QJsonArray resultMinObjectCoords;
            resultMinObjectCoords.append( m_result_min_object_coords.x() );
            resultMinObjectCoords.append( m_result_min_object_coords.y() );
            resultMinObjectCoords.append( m_result_min_object_coords.z() );

            QJsonArray resultMaxObjectCoords;
            resultMaxObjectCoords.append( m_result_max_object_coords.x() );
            resultMaxObjectCoords.append( m_result_max_object_coords.y() );
            resultMaxObjectCoords.append( m_result_max_object_coords.z() );

            m_web_sockets->text()->sendTextMessage( QJsonDocument( {
                                                                  { QString::fromUtf8( Protocol::Key::Event )                , QString::fromUtf8( Protocol::Events::RequestDataAt ) } ,
                                                                  { QString::fromUtf8( Protocol::Key::TimeStep )             , requestTimeStep },
                                                                  { QString::fromUtf8( Protocol::Key::ResultMinObjectCoords ), resultMinObjectCoords },
                                                                  { QString::fromUtf8( Protocol::Key::ResultMaxObjectCoords ), resultMaxObjectCoords },
                                                                  } ).toJson( QJsonDocument::Compact ) );
        }
        break;
    }
    default:
        break;
    }
}

void ObjectEditor::onTransferFunctionUpdate()
{
    if( !( *m_viz_mode == Viz::Mode::LocalClientAndServer  ||
          *m_viz_mode == Viz::Mode::RemoteClientAndServer ||
          *m_viz_mode == Viz::Mode::RemoteInSitu ) ||
        !m_web_sockets->isConnected() )
    {
        return;
    }

    QJsonArray patchArray;

    for( int row = 0; row < m_model->rowCount(); ++row )
    {
        QStandardItem* nameItem = m_model->item( row, 0 );
        if( !nameItem ) continue;

        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        auto objectInfo = var.value<ObjectInfoExtractor::ObjectInfo>();

        // 対象フォーマットのみ
        const auto fmt = objectInfo.format; // ←メンバ名が違うなら合わせてください
        const bool is_target =
            ( fmt == ObjectInfoExtractor::Format::ClientServerPointObject ) ||
            ( fmt == ObjectInfoExtractor::Format::InsituServerPointObject );

        if( !is_target ) continue;

        // needSameTimeStep を有効化
        // （既存コードでは needSameTimeStepReplace があるので、実際のフラグ名に合わせてどちらか/両方）
        // objectInfo.needSameTimeStep = true;          // ←存在するならこれ
        objectInfo.needSameTimeStepReplace = true;   // ←既存にあるならこっち

        // モデルへ反映
        nameItem->setData( QVariant::fromValue( objectInfo ), Qt::UserRole );

        // 送信用パッチ（uuid + フラグのみ）
        QJsonObject patch;
        patch[QString::fromUtf8( Protocol::Key::UUID )] = QString::fromUtf8( objectInfo.uuid );
        patch[QString::fromUtf8( Protocol::Key::NeedSameTimeStepReplace )] = true; // ←Key があるならこれ
        // もし Protocol に Key が無いなら、サーバ側が見ているキー名に合わせてください

        patchArray.append( patch );
    }

    // 対象が無ければ送らない
    if( patchArray.isEmpty() ) return;

    QJsonObject objectInfoParameter;
    objectInfoParameter[QString::fromUtf8( Protocol::Key::Event )] = QString::fromUtf8( Protocol::Events::ObjectInfoParameter );

    // onApply() と同じイベントに載せるなら Objects で差分送信
    objectInfoParameter[QString::fromUtf8( Protocol::Key::Objects )] = patchArray;

    m_web_sockets->text()->sendTextMessage( QJsonDocument( objectInfoParameter ).toJson( QJsonDocument::Compact ) );
}


void ObjectEditor::onUnpack( const QByteArray& binary )
{
    const char* dataPtr = binary.constData();
    size_t offset = 0;

    // Time Step
    int timeStep = 0;
    std::memcpy( &timeStep, dataPtr + offset, sizeof(int) );
    offset += sizeof(int);

    // 送信されてきたUUIDを記録する
    std::unordered_set<std::string> receivedUUIDSs;

    while( offset < static_cast<size_t>( binary.size() ) )
    {
        // UUIDの読み取り
        uint32_t uuidLen = 0;

        std::memcpy( &uuidLen, dataPtr + offset, sizeof(uint32_t) );
        offset += sizeof(uint32_t);

        if( offset + uuidLen > static_cast<size_t>( binary.size() ) ) break; // uuidLenが正常値か検証

        std::string uuid( uuidLen, '\0' );
        std::memcpy( uuid.data(), dataPtr + offset, uuidLen );
        offset += uuidLen;

        receivedUUIDSs.insert( uuid );

        int currentImportedTimeStep;
        std::memcpy( &currentImportedTimeStep, dataPtr + offset, sizeof(int) );
        offset += sizeof(int);

        // モデル内のUUIDと一致する場合objectを更新する。
        for( int row = 0; row < m_model->rowCount(); ++row )
        {
            QStandardItem* nameItem = m_model->item( row, 0 );
            QVariant var = nameItem->data( Qt::UserRole );
            if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

            if( info.uuid != uuid ) continue;

            // UUIDが一致したのでobjectを更新

            info.currentImportedTimeStep = currentImportedTimeStep;

            switch( info.format )
            {
            case ObjectInfoExtractor::ClientServerPointObject:
            case ObjectInfoExtractor::InsituServerPointObject:
            case ObjectInfoExtractor::PointObjectKVSML:
            {
                size_t numberOfVertices = 0;
                std::memcpy( &numberOfVertices, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> coords( numberOfVertices * 3 );
                std::memcpy( coords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 * numberOfVertices );
                offset += sizeof(kvs::Real32) * 3 * numberOfVertices;

                kvs::ValueArray<kvs::UInt8> colors( numberOfVertices * 3 );
                std::memcpy( colors.data(), dataPtr + offset, sizeof(kvs::UInt8) * 3 * numberOfVertices );
                offset += sizeof(kvs::UInt8) * 3 * numberOfVertices;

                kvs::ValueArray<kvs::Real32> normals( numberOfVertices * 3 );
                std::memcpy( normals.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 * numberOfVertices );
                offset += sizeof(kvs::Real32) * 3 * numberOfVertices;

                kvs::Vec3 minObjectCoords;
                std::memcpy( minObjectCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 );
                offset += sizeof(kvs::Real32) * 3;

                kvs::Vec3 maxObjectCoords;
                std::memcpy ( maxObjectCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 );
                offset += sizeof(kvs::Real32) * 3;

                auto* object = new kvs::PointObject();
                object->setCoords( coords );
                object->setColors( colors );
                object->setNormals( normals );
                object->setMinMaxObjectCoords( minObjectCoords, maxObjectCoords );
                object->setMinMaxExternalCoords( minObjectCoords, maxObjectCoords );

                info.object = object;
                break;
            }
            case ObjectInfoExtractor::PointObjectLAS:
            case ObjectInfoExtractor::PointObjectPTS:
            {
                size_t numberOfVertices = 0;
                std::memcpy( &numberOfVertices, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> coords( numberOfVertices * 3 );
                std::memcpy( coords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 * numberOfVertices );
                offset += sizeof(kvs::Real32) * 3 * numberOfVertices;

                kvs::ValueArray<kvs::UInt8> colors( numberOfVertices * 3 );
                std::memcpy( colors.data(), dataPtr + offset, sizeof(kvs::UInt8) * 3 * numberOfVertices );
                offset += sizeof(kvs::UInt8) * 3 * numberOfVertices;

                kvs::Vec3 minObjectCoords;
                std::memcpy( minObjectCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 );
                offset += sizeof(kvs::Real32) * 3;

                kvs::Vec3 maxObjectCoords;
                std::memcpy ( maxObjectCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 );
                offset += sizeof(kvs::Real32) * 3;

                auto* object = new kvs::PointObject();
                object->setCoords( coords );
                object->setColors( colors );;
                object->setMinMaxObjectCoords( minObjectCoords, maxObjectCoords );
                object->setMinMaxExternalCoords( minObjectCoords, maxObjectCoords );

                info.object = object;
                break;
            }
            case ObjectInfoExtractor::ServerGlyphObject:
            case ObjectInfoExtractor::PolygonObjectKVSML:
            case ObjectInfoExtractor::PolygonObjectSTL:
            {
                kvs::PolygonObject::PolygonType polygonType;
                std::memcpy( &polygonType, dataPtr + offset, sizeof(kvs::PolygonObject::PolygonType) );
                offset += sizeof(kvs::PolygonObject::PolygonType);

                kvs::PolygonObject::ColorType colorType;
                std::memcpy( &colorType, dataPtr + offset, sizeof(kvs::PolygonObject::ColorType) );
                offset += sizeof(kvs::PolygonObject::ColorType);

                kvs::PolygonObject::NormalType normalType;
                std::memcpy( &normalType, dataPtr + offset, sizeof(kvs::PolygonObject::NormalType) );
                offset += sizeof(kvs::PolygonObject::NormalType);

                size_t nCoords;
                std::memcpy( &nCoords, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> coords( nCoords );
                std::memcpy( coords.data(), dataPtr + offset, sizeof(kvs::Real32) * nCoords );
                offset += sizeof(kvs::Real32) * nCoords;

                size_t nColors;
                std::memcpy( &nColors, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt8> colors( nColors );
                std::memcpy( colors.data(), dataPtr + offset, sizeof(kvs::UInt8) * nColors );
                offset += sizeof(kvs::UInt8) * nColors;

                size_t nNormals;
                std::memcpy( &nNormals, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> normals( nNormals );
                std::memcpy( normals.data(), dataPtr + offset, sizeof(kvs::Real32) * nNormals );
                offset += sizeof(kvs::Real32) * nNormals;

                size_t nConnections;
                std::memcpy( &nConnections, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt32> connections( nConnections );
                std::memcpy( connections.data(), dataPtr + offset, sizeof(kvs::UInt32) * nConnections );
                offset += sizeof(kvs::UInt32) * nConnections;

                size_t nOpacities;
                std::memcpy( &nOpacities, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt8> opacities( nOpacities );
                std::memcpy( opacities.data(), dataPtr + offset, sizeof(kvs::UInt8) * nOpacities );
                offset += sizeof(kvs::UInt8) * nOpacities;

                kvs::Vec3 minObjectCoords;
                std::memcpy( minObjectCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 );
                offset += sizeof(kvs::Real32) * 3;

                kvs::Vec3 maxObjectCoords;
                std::memcpy ( maxObjectCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 );
                offset += sizeof(kvs::Real32) * 3;

                auto* object = new kvs::PolygonObject();
                object->setPolygonType( polygonType );
                object->setColorType( colorType );
                object->setNormalType( normalType );
                object->setCoords( coords );
                object->setColors( colors );
                object->setNormals( normals );
                object->setConnections( connections );
                object->setOpacities( opacities );
                object->setMinMaxObjectCoords( minObjectCoords, maxObjectCoords );
                object->setMinMaxExternalCoords( minObjectCoords, maxObjectCoords );

                info.object = object;
                break;
            }
#ifdef ASSIMP
            case ObjectInfoExtractor::PolygonObject3DS:
            case ObjectInfoExtractor::PolygonObjectFBX:
            {
                kvs::TexturedPolygonObject::PolygonType polygonType;
                std::memcpy( &polygonType, dataPtr + offset, sizeof(kvs::TexturedPolygonObject::PolygonType) );
                offset += sizeof(kvs::TexturedPolygonObject::PolygonType);

                kvs::TexturedPolygonObject::ColorType colorType;
                std::memcpy( &colorType, dataPtr + offset, sizeof(kvs::TexturedPolygonObject::ColorType) );
                offset += sizeof(kvs::TexturedPolygonObject::ColorType);

                kvs::TexturedPolygonObject::NormalType normalType;
                std::memcpy( &normalType, dataPtr + offset, sizeof(kvs::TexturedPolygonObject::NormalType) );
                offset += sizeof(kvs::TexturedPolygonObject::NormalType);

                size_t nCoords;
                std::memcpy( &nCoords, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> coords( nCoords );
                std::memcpy( coords.data(), dataPtr + offset, sizeof(kvs::Real32) * nCoords );
                offset += sizeof(kvs::Real32) * nCoords;

                size_t nColors;
                std::memcpy( &nColors, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt8> colors( nColors );
                std::memcpy( colors.data(), dataPtr + offset, sizeof(kvs::UInt8) * nColors );
                offset += sizeof(kvs::UInt8) * nColors;

                size_t nNormals;
                std::memcpy( &nNormals, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> normals( nNormals );
                std::memcpy( normals.data(), dataPtr + offset, sizeof(kvs::Real32) * nNormals );
                offset += sizeof(kvs::Real32) * nNormals;

                size_t nConnections;
                std::memcpy( &nConnections, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt32> connections( nConnections );
                std::memcpy( connections.data(), dataPtr + offset, sizeof(kvs::UInt32) * nConnections );
                offset += sizeof(kvs::UInt32) * nConnections;

                size_t nOpacities;
                std::memcpy( &nOpacities, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt8> opacities( nOpacities );
                std::memcpy( opacities.data(), dataPtr + offset, sizeof(kvs::UInt8) * nOpacities );
                offset += sizeof(kvs::UInt8) * nOpacities;

                size_t nTexture2DCoords;
                std::memcpy( &nTexture2DCoords, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> texture2DCoords( nTexture2DCoords );
                std::memcpy( texture2DCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * nTexture2DCoords );
                offset += sizeof(kvs::Real32) * nTexture2DCoords;

                size_t nTextureIds;
                std::memcpy( &nTextureIds, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt32> textureIds( nTextureIds );
                std::memcpy( textureIds.data(), dataPtr + offset, sizeof(kvs::UInt32) * nTextureIds );
                offset += sizeof(kvs::UInt32) * nTextureIds;

                // --- mapIdToColorArray ---
                size_t mapColorSize;
                memcpy( &mapColorSize, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                std::map<kvs::UInt32, kvs::ValueArray<kvs::UInt8>> mapColor;

                for( size_t i = 0; i < mapColorSize; i++ )
                {
                    kvs::UInt32 id;
                    memcpy( &id, dataPtr + offset, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);

                    size_t arrSize;
                    memcpy( &arrSize, dataPtr + offset, sizeof(size_t) );
                    offset += sizeof(size_t);

                    kvs::ValueArray<kvs::UInt8> arr(arrSize);
                    memcpy( arr.data(), dataPtr + offset, arrSize );
                    offset += arrSize;

                    mapColor[id] = arr;
                }

                // --- width ---
                size_t mapWSize;
                memcpy( &mapWSize, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                std::map<kvs::UInt32, kvs::UInt32> mapW;

                for( size_t i = 0; i < mapWSize; i++ )
                {
                    kvs::UInt32 id, w;
                    memcpy( &id, dataPtr + offset, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);
                    memcpy( &w, dataPtr + offset, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);
                    mapW[id] = w;
                }

                // --- height ---
                size_t mapHSize;
                memcpy( &mapHSize, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                std::map<kvs::UInt32, kvs::UInt32> mapH;

                for( size_t i = 0; i < mapHSize; i++ )
                {
                    kvs::UInt32 id, h;
                    memcpy( &id, dataPtr + offset, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);
                    memcpy( &h, dataPtr + offset, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);
                    mapH[id] = h;
                }

                kvs::Vec3 minObjectCoords;
                std::memcpy( minObjectCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 );
                offset += sizeof(kvs::Real32) * 3;

                kvs::Vec3 maxObjectCoords;
                std::memcpy ( maxObjectCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 );
                offset += sizeof(kvs::Real32) * 3;

                auto* object = new kvs::TexturedPolygonObject();
                object->setPolygonType( polygonType );
                object->setColorType( colorType );
                object->setNormalType( normalType );
                object->setCoords( coords );
                object->setColors( colors );
                object->setNormals( normals );
                object->setConnections( connections );
                object->setOpacities( opacities );
                object->setTexture2DCoords( texture2DCoords );
                object->setTextureIds( textureIds );
                object->setMapIdToColorArray( mapColor );
                object->setMapIdToImageWidth( mapW );
                object->setMapIdToImageHeight( mapH );
                object->setMinMaxObjectCoords( minObjectCoords, maxObjectCoords );
                object->setMinMaxExternalCoords( minObjectCoords, maxObjectCoords );

                info.object = object;
                break;
            }
#endif
            case ObjectInfoExtractor::LineObjectKVSML:
            {
                kvs::LineObject::LineType lineType;
                std::memcpy( &lineType, dataPtr + offset, sizeof(kvs::LineObject::LineType) );
                offset += sizeof(kvs::LineObject::LineType);

                kvs::LineObject::ColorType colorType;
                std::memcpy( &colorType, dataPtr + offset, sizeof(kvs::LineObject::ColorType) );
                offset += sizeof(kvs::LineObject::ColorType);

                // kvs::LineObject::NormalType normalType;
                // std::memcpy( &normalType, dataPtr + offset, sizeof(kvs::LineObject::NormalType) );
                // offset += sizeof(kvs::LineObject::NormalType);

                size_t nCoords;
                std::memcpy( &nCoords, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> coords( nCoords );
                std::memcpy( coords.data(), dataPtr + offset, sizeof(kvs::Real32) * nCoords );
                offset += sizeof(kvs::Real32) * nCoords;

                size_t nColors;
                std::memcpy( &nColors, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt8> colors( nColors );
                std::memcpy( colors.data(), dataPtr + offset, sizeof(kvs::UInt8) * nColors );
                offset += sizeof(kvs::UInt8) * nColors;

                size_t nNormals;
                std::memcpy( &nNormals, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> normals( nNormals );
                std::memcpy( normals.data(), dataPtr + offset, sizeof(kvs::Real32) * nNormals );
                offset += sizeof(kvs::Real32) * nNormals;

                size_t nConnections;
                std::memcpy( &nConnections, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt32> connections( nConnections );
                std::memcpy( connections.data(), dataPtr + offset, sizeof(kvs::UInt32) * nConnections );
                offset += sizeof(kvs::UInt32) * nConnections;

                size_t nSizes;
                std::memcpy( &nSizes, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> sizes( nSizes );
                std::memcpy( sizes.data(), dataPtr + offset, sizeof(kvs::Real32) * nSizes );
                offset += sizeof(kvs::Real32) * nSizes;

                kvs::Vec3 minObjectCoords;
                std::memcpy( minObjectCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 );
                offset += sizeof(kvs::Real32) * 3;

                kvs::Vec3 maxObjectCoords;
                std::memcpy ( maxObjectCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * 3 );
                offset += sizeof(kvs::Real32) * 3;

                auto* object = new kvs::LineObject();
                object->setLineType( lineType );
                object->setColorType( colorType );
                // object->setNormalType( normalType2 );
                object->setCoords( coords );
                object->setColors( colors );
                object->setNormals( normals );
                object->setConnections( connections );
                object->setSizes( sizes );
                object->setMinMaxObjectCoords( minObjectCoords, maxObjectCoords );
                object->setMinMaxExternalCoords( minObjectCoords, maxObjectCoords );

                info.object = object;
                break;
            }
            default:
                break;
            }
            nameItem->setData( QVariant::fromValue( info ), Qt::UserRole );
            break;
        }
    }

    // 受信していないUUIDのobjectをnullptrに更新する
    for( int row = 0; row < m_model->rowCount(); ++row )
    {
        QStandardItem* nameItem = m_model->item( row, 0 );
        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

        if( receivedUUIDSs.find( info.uuid ) == receivedUUIDSs.end() )
        {
            info.object = nullptr;
            nameItem->setData( QVariant::fromValue( info ), Qt::UserRole );
        }
    }

    dataRequestComplete( timeStep );
}

void ObjectEditor::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ObjectEditor::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ObjectEditor::updateEditorFromIndex( const QModelIndex& index )
{
    if( !index.isValid() ) return;

    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) return;

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
    // サーバポイントオブジェクト(ClientServer/In-Situ共通)
    ui->particleLimitSpinBox            ->setValue( info.tmpParticleLimit );
    ui->extraOpacityFactorDoubleSpinBox ->setValue( info.tmpExtraOpacityFactor );
    // サーバポイントオブジェクト(ClientServerのみ)
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
    // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
    QPalette palette = ui->colorClickableLabel->palette();
    palette.setColor( QPalette::Window, QColor( info.tmpPolygonColor.r(), info.tmpPolygonColor.g(), info.tmpPolygonColor.b() ) );
    ui->colorClickableLabel         ->setPalette( palette );
    ui->opacityDoubleSpinBox        ->setValue( info.tmpPolygonOpacity );

    bool isObject                  = false;
    bool isCommonServerObject      = false;
    bool isClientServerObject      = false;
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

    setWidgetsVisible( m_group_common_object_widgets             , isObject );
    setWidgetsVisible( m_group_common_server_point_object_widgets, isCommonServerObject );
    setWidgetsVisible( m_group_client_server_point_object_widgets, isClientServerObject );
    setWidgetsVisible( m_group_nontexture_polygon_object_widgets , isNonTexturePolygonObject );
}

void ObjectEditor::setWidgetsVisible( const QList<QWidget*>& widgets, const bool visible )
{
    for( auto* w : widgets ) w->setVisible( visible );
}

void ObjectEditor::addObjectToModel( ObjectInfoExtractor::ObjectInfo& objectInfo ) // FIXME:スロットではないのでは
{
    // NOTE:最初に追加されるオブジェクトはフォーカス対象になる
    if( m_model->rowCount() == 0 ) objectInfo.tmpIsFocus = true;

    QList<QStandardItem*> rowItems;
    auto nameItem         = new QStandardItem( QString::fromUtf8( objectInfo.name ) );
    auto formatItem       = new QStandardItem( QString::fromUtf8( ObjectInfoExtractor::formatToString( objectInfo.format ) ) );
    auto displayItem      = new QStandardItem( "" );
    auto keepInitialItem  = new QStandardItem( "" );
    auto keepFinalItem    = new QStandardItem( "" );

    // NOTE:アイテム名等の編集を無効にする
    nameItem        ->setEditable( false );
    formatItem      ->setEditable( false );
    displayItem     ->setEditable( false );
    keepInitialItem ->setEditable( false );
    keepFinalItem   ->setEditable( false );

    // NOTE:display,keepInitial/Finalのチェックボックス有効にする。
    displayItem     ->setCheckable( true );
    keepInitialItem ->setCheckable( true );
    keepFinalItem   ->setCheckable( true );

    // NOTE:boolからQt::CheckStateに変換
    displayItem     ->setCheckState( objectInfo.tmpIsDisplay     ? Qt::Checked : Qt::Unchecked );
    keepInitialItem ->setCheckState( objectInfo.tmpIsKeepInitial ? Qt::Checked : Qt::Unchecked );
    keepFinalItem   ->setCheckState( objectInfo.tmpIsKeepFinal   ? Qt::Checked : Qt::Unchecked );

    QVariant var;
    var.setValue( objectInfo );
    nameItem->setData( var, Qt::UserRole );

    rowItems << nameItem << formatItem << displayItem << keepInitialItem << keepFinalItem;
    m_model->appendRow( rowItems );

    QModelIndex index = m_model->indexFromItem( rowItems.first() );
    ui->treeView->setCurrentIndex( index );

    calculateTotalMinMaxTimeStep();
}

void ObjectEditor::calculateTotalMinMaxTimeStep()
{
    int totalMin = std::numeric_limits<int>::max();
    int totalMax = std::numeric_limits<int>::min();

    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem = m_model->item( row, 0 ); // NOTE:ObjectInfoが格納されている列
        if( !nameItem ) continue;

        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

        totalMin = std::min( totalMin, info.timeStep.first );
        totalMax = std::max( totalMax, info.timeStep.second );
    }

    bool isSingleObject = false; // NOTE:登録されているオブジェクトが1つのみの場合を判別する
    if( m_model->rowCount() == 1 ) isSingleObject = true;
    emit updateTotalTimeStepRange( totalMin, totalMax, isSingleObject );
}

template<typename F>
void ObjectEditor::updateSelectedObject( F func )
{
    auto selectedRows = ui->treeView->selectionModel()->selectedRows(0); // column 0
    if( selectedRows.isEmpty() ) return;

    QModelIndex index = selectedRows.first();
    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) return;

    ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

    func( info ); // 渡された処理で info を更新する

    m_model->setData( index, QVariant::fromValue( info ), Qt::UserRole );
}

void ObjectEditor::updateVisibility( int requestTimeStep )
{
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* item = m_model->item( row, 0 ); // UserRoleにObjectInfoが格納されている列
        if( !item ) continue;

        QVariant var = item->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

        int resultTimeStep = -1;

        if( info.isDisplay )
        {
            // info.timeStep.first ～ info.timeStep.second の範囲内に m_time_step が含まれる場合、
            // 対応するタイムステップとして resultTimeStep に設定する
            if( requestTimeStep >= info.timeStep.first && requestTimeStep <= info.timeStep.second )
            {
                resultTimeStep = requestTimeStep;
            }

            if( requestTimeStep < info.timeStep.first )
            {
                if( info.isKeepInitial )
                {
                    resultTimeStep = info.timeStep.first;
                }
            }

            if( requestTimeStep > info.timeStep.second )
            {
                if( info.isKeepFinal )
                {
                    resultTimeStep = info.timeStep.second;
                }
            }

            if( resultTimeStep != -1 )
            {
                if( resultTimeStep == info.currentImportedTimeStep )
                {
                    if( info.needSameTimeStepReplace == false )
                    {
                        if( info.objectID.first != -1 && info.objectID.second != -1 )
                        {
                            m_screen->scene()->object( info.objectID.first )->show();
                        }
                    }
                    else
                    {
                        info.needSameTimeStepReplace = false;
                    }
                }
                else
                {
                    info.needSameTimeStepReplace = false;
                }

                QVariant newVar;
                newVar.setValue( info );
                item->setData( newVar, Qt::UserRole );
            }
            else
            {
                if( info.objectID.first == -1 && info.objectID.second == -1 ) // 一度も登録されてない場合
                {
                }
                else // 登録されている場合
                {
                    if( info.objectID.first != -1 && info.objectID.second != -1 )
                    {
                        m_screen->scene()->object( info.objectID.first )->hide();
                    }
                }
            }
        }
        else
        {
            if( info.objectID.first == -1 && info.objectID.second == -1 ) // 一度も登録されてない場合
            {

            }
            else // 登録されている場合
            {
                if( info.objectID.first != -1 && info.objectID.second != -1 )
                {
                    m_screen->scene()->object( info.objectID.first )->hide();
                }
            }
        }
    }
}

void ObjectEditor::registerObject( ObjectInfoExtractor::ObjectInfo& info )
{
    std::unique_ptr<kvs::glsl::ParticleBasedRenderer> particleBasedRenderer;
    std::unique_ptr<kvs::StochasticPolygonRenderer> stochasticPolygonRenderer;
    std::unique_ptr<kvs::StochasticLineRenderer> stochasticLineRenderer;
    std::unique_ptr<kvs::StochasticTexturedPolygonRenderer> stochasticTexturedPolygonRenderer;

    switch( info.format )
    {
    case ObjectInfoExtractor::ClientServerPointObject:
    case ObjectInfoExtractor::InsituServerPointObject:
        particleBasedRenderer = std::make_unique<kvs::glsl::ParticleBasedRenderer>();
        particleBasedRenderer.get()->enableShuffle();
        emit shading( particleBasedRenderer.get() );
        info.objectID = m_screen->registerObject( info.object, particleBasedRenderer.release() );
        break;
    case ObjectInfoExtractor::ServerGlyphObject:
        stochasticPolygonRenderer = std::make_unique<kvs::StochasticPolygonRenderer>();
        emit shading( stochasticPolygonRenderer.get() );
        info.objectID = m_screen->registerObject( info.object, stochasticPolygonRenderer.release() );
        break;
    case ObjectInfoExtractor::PointObjectKVSML:
    case ObjectInfoExtractor::PointObjectLAS:
    case ObjectInfoExtractor::PointObjectPTS:
        particleBasedRenderer = std::make_unique<kvs::glsl::ParticleBasedRenderer>();
        particleBasedRenderer.get()->enableShuffle();
        emit shading( particleBasedRenderer.get() );
        info.objectID = m_screen->registerObject( info.object, particleBasedRenderer.release() );
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

void ObjectEditor::replaceObject( ObjectInfoExtractor::ObjectInfo& info )
{
    std::unique_ptr<kvs::StochasticPolygonRenderer> stochasticPolygonRenderer;

    switch( info.format )
    {
    case ObjectInfoExtractor::ClientServerPointObject:
    case ObjectInfoExtractor::InsituServerPointObject:
        m_screen->scene()->replaceObject( info.objectID.first, info.object );
        break;
    case ObjectInfoExtractor::ServerGlyphObject:
        stochasticPolygonRenderer = std::make_unique<kvs::StochasticPolygonRenderer>();
        emit shading( stochasticPolygonRenderer.get() );
        m_screen->scene()->replaceObject( info.objectID.first, info.object );
        m_screen->scene()->replaceRenderer( info.objectID.second, stochasticPolygonRenderer.release() );
        break;
    case ObjectInfoExtractor::PointObjectKVSML:
    case ObjectInfoExtractor::PointObjectLAS:
    case ObjectInfoExtractor::PointObjectPTS:
        m_screen->scene()->replaceObject( info.objectID.first, info.object );
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

void ObjectEditor::onItemSelection( const QItemSelection &selected, const QItemSelection &deselected )
{
    Q_UNUSED( deselected );
    if( selected.indexes().isEmpty() ) return;

    updateEditorFromIndex( selected.indexes().first() );
}

void ObjectEditor::onFocusCheckBoxToggled( bool checked )
{
    updateSelectedObject( [checked]( auto &info )
                         {
                             info.tmpIsFocus = checked;
                         } );
}

void ObjectEditor::onParticleLimitSpinBoxValueChanged( int value )
{
    updateSelectedObject( [value]( auto &info )
                         {
                             info.tmpParticleLimit = value;
                         } );
}

void ObjectEditor::onExtraOpacityFactorDoubleSpinBoxValueChanged( double value )
{
    updateSelectedObject( [value]( auto &info )
                         {
                             info.tmpExtraOpacityFactor = value;
                         } );
}

void ObjectEditor::onCoordinateLineEditTextChanged()
{
    updateSelectedObject( [this] ( auto &info )
                         {
                             info.tmpCoordinateX = ui->coordinateXLineEdit->text().toUtf8().constData();
                             info.tmpCoordinateY = ui->coordinateYLineEdit->text().toUtf8().constData();
                             info.tmpCoordinateZ = ui->coordinateZLineEdit->text().toUtf8().constData();
                         } );
}

void ObjectEditor::onColorLabelDoubleClicked()
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

void ObjectEditor::onOpacityDoubleSpinBoxValueChanged( double value )
{
    updateSelectedObject( [value]( auto &info )
                         {
                             info.tmpPolygonOpacity = value;
                         } );
}

void ObjectEditor::onBrowse()
{
    // Note:
    // Local(サーバと接続せずローカルマシンのデータを可視化するモード)
    // LocalClientAndServer(クライアントとサーバを同マシンで起動/接続するモード)
    // -> QFileDialog
    // RemoteClientAndServer(クライアントとサーバを別マシンで起動/接続するモード)
    // RemoteInSitu(クライアントとサーバを別マシンで起動/接続するモード)
    // -> RemoteFileDialog
    switch( *m_viz_mode )
    {
    case Viz::Mode::Local:
    case Viz::Mode::LocalClientAndServer:
    {
        QStringList filePaths = QFileDialog::getOpenFileNames( this, tr("Select 3D data files"), QDir::homePath(),
#ifdef ASSIMP
            tr("Support Files (*.kvsml *.las *.pts *.stl *.fbx *.3ds);;All Files (*.*)")
#else
            tr("Support Files (*.kvsml *.las *.pts *.stl);;All Files (*.*)")
#endif
            );

        if( filePaths.isEmpty() ) return;

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
                // FIXME:MainWindowのStatusBarで通知
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
                                  { QString::fromUtf8( Protocol::Key::Event ), QString::fromUtf8( Protocol::Events::SelectedFile ) },
                                  { QString::fromUtf8( Protocol::Key::File ) , dlg.selectedFile() },
                                  { QString::fromUtf8( Protocol::Key::UUID ) , uuid }
                              } ).toJson( QJsonDocument::Compact )
                );
        }
        break;
    }
    default:
        break;
    }
}

void ObjectEditor::onDelete()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) return;

    ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

    // NOTE:フォーカス再割当が必要か判定
    bool requireFocusOnOther = false;

    if( info.tmpIsFocus )
    {
        bool otherFocusedFound = false;

        for( int row = 0; row < m_model->rowCount(); row++ )
        {
            if( row == index.row() ) continue;

            QStandardItem* nameItem = m_model->item( row, 0 );
            if( !nameItem ) continue;

            QVariant otherVar = nameItem->data( Qt::UserRole );
            if( !otherVar.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            ObjectInfoExtractor::ObjectInfo otherInfo = otherVar.value<ObjectInfoExtractor::ObjectInfo>();

            if( otherInfo.tmpIsFocus )
            {
                otherFocusedFound = true;
                break;
            }
        }

        requireFocusOnOther = !otherFocusedFound;
    }

    // NOTE:フォーカスを他オブジェクトへ移す
    if( requireFocusOnOther )
    {
        for( int row = 0; row < m_model->rowCount(); ++row )
        {
            if( row == index.row() ) continue;

            QStandardItem* item = m_model->item( row, 0 );
            if( !item ) continue;

            QVariant v = item->data( Qt::UserRole );
            if( !v.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            ObjectInfoExtractor::ObjectInfo otherInfo = v.value<ObjectInfoExtractor::ObjectInfo>();

            otherInfo.tmpIsFocus = true;
            item->setData( QVariant::fromValue( otherInfo ), Qt::UserRole );
            break; // NOTE:1つだけフォーカス
        }
    }

    // NOTE:シーンから削除
    if( info.objectID.first != -1 ) m_screen->scene()->removeObject( info.objectID.first );

    // NOTE:Model から削除
    m_model->removeRow( index.row() );

    calculateTotalMinMaxTimeStep();
    m_screen->update();

    // NOTE:全削除時の UI 非表示
    if( m_model->rowCount() == 0 )
    {
        setWidgetsVisible( m_group_common_object_widgets             , false );
        setWidgetsVisible( m_group_common_server_point_object_widgets, false );
        setWidgetsVisible( m_group_client_server_point_object_widgets, false );
        setWidgetsVisible( m_group_nontexture_polygon_object_widgets , false );
    }

    // NOTE:WebSocket：削除通知（即時確定イベント）
    if( ( *m_viz_mode == Viz::Mode::LocalClientAndServer  ||
          *m_viz_mode == Viz::Mode::RemoteClientAndServer ||
          *m_viz_mode == Viz::Mode::RemoteInSitu ) &&
          m_web_sockets->isConnected() )
    {
        QJsonObject msg;
        msg[QString::fromUtf8( Protocol::Key::Event )] = QString::fromUtf8( Protocol::Events::ObjectDelete );
        msg[QString::fromUtf8( Protocol::Key::UUID )] = QString::fromUtf8( info.uuid );
        m_web_sockets->text()->sendTextMessage( QJsonDocument( msg ).toJson( QJsonDocument::Compact ) );
    }
}

void ObjectEditor::onApply()
{
    // NOTE:フォーカス対象の最小最大オブジェクト座標の計算結果格納用変数
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

    // === 差分送信用 ===
    QJsonArray patchArray;

    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem        = m_model->item( row, 0 );
        QStandardItem* displayItem     = m_model->item( row, 2 );
        QStandardItem* keepInitialItem = m_model->item( row, 3 );
        QStandardItem* keepFinalItem   = m_model->item( row, 4 );
        if( !nameItem || !displayItem || !keepInitialItem || !keepFinalItem ) continue;

        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo objectInfo = var.value<ObjectInfoExtractor::ObjectInfo>();

        // NOTE:display, keepInitial/Finalのチェックボックスの状態をtmpに格納 // FIXME:チェックボックスのtmpIsは不要なのでは
        objectInfo.tmpIsDisplay     = ( displayItem->checkState()     == Qt::Checked );
        objectInfo.tmpIsKeepInitial = ( keepInitialItem->checkState() == Qt::Checked );
        objectInfo.tmpIsKeepFinal   = ( keepFinalItem->checkState()   == Qt::Checked );

        // オブジェクト用パッチ
        QJsonObject patch;
        patch[QString::fromUtf8( Protocol::Key::UUID )] = QString::fromUtf8( objectInfo.uuid );

        // NOTE:isHogeの確定
        // 全オブジェクト共通
        if( objectInfo.isDisplay != objectInfo.tmpIsDisplay )
        {
            objectInfo.isDisplay = objectInfo.tmpIsDisplay;
            patch[QString::fromUtf8( Protocol::Key::IsDisplay )] = objectInfo.isDisplay;
        }
        if( objectInfo.isKeepInitial != objectInfo.tmpIsKeepInitial )
        {
            objectInfo.isKeepInitial = objectInfo.tmpIsKeepInitial;
            patch[QString::fromUtf8(Protocol::Key::IsKeepInitial)] = objectInfo.isKeepInitial;
        }
        if( objectInfo.isKeepFinal != objectInfo.tmpIsKeepFinal )
        {
            objectInfo.isKeepFinal = objectInfo.tmpIsKeepFinal;
            patch[QString::fromUtf8( Protocol::Key::IsKeepFinal )] = objectInfo.isKeepFinal;
        }
        if( objectInfo.isFocus != objectInfo.tmpIsFocus )
        {
            objectInfo.isFocus = objectInfo.tmpIsFocus;
            patch[QString::fromUtf8( Protocol::Key::IsFocus )] = objectInfo.isFocus;
        }
        // サーバポイントオブジェクト(ClientServer/In-Situ共通)
        if( objectInfo.particleLimit != objectInfo.tmpParticleLimit )
        {
            objectInfo.particleLimit = objectInfo.tmpParticleLimit;
            patch[QString::fromUtf8( Protocol::Key::ParticleLimit )] = objectInfo.particleLimit;
        }
        if( objectInfo.extraOpacityFactor != objectInfo.tmpExtraOpacityFactor )
        {
            objectInfo.extraOpacityFactor = objectInfo.tmpExtraOpacityFactor;
            patch[QString::fromUtf8( Protocol::Key::ExtraOpacityFactor )] = objectInfo.extraOpacityFactor;
        }
        // サーバポイントオブジェクト(ClientServerのみ)
        auto updateCoord = [&](const QString& key, std::string& dst, const std::string& src)
        {
            if( dst != src )
            {
                dst = src;
                patch[key] = QString::fromUtf8( dst );
            }
        };
        updateCoord( QString::fromUtf8( Protocol::Key::CoordinateX ), objectInfo.coordinateX, objectInfo.tmpCoordinateX );
        updateCoord( QString::fromUtf8( Protocol::Key::CoordinateY ), objectInfo.coordinateY, objectInfo.tmpCoordinateY );
        updateCoord( QString::fromUtf8( Protocol::Key::CoordinateZ ), objectInfo.coordinateZ, objectInfo.tmpCoordinateZ );
        // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
        if( objectInfo.polygonColor.r() != objectInfo.tmpPolygonColor.r() ||
            objectInfo.polygonColor.g() != objectInfo.tmpPolygonColor.g() ||
            objectInfo.polygonColor.b() != objectInfo.tmpPolygonColor.b() )
        {
            objectInfo.polygonColor = objectInfo.tmpPolygonColor;
            objectInfo.needSameTimeStepReplace = true;

            QJsonArray color;
            color.append( objectInfo.polygonColor.r() );
            color.append( objectInfo.polygonColor.g() );
            color.append( objectInfo.polygonColor.b() );
            patch[QString::fromUtf8( Protocol::Key::PolygonColor )] = color;
        }
        if( objectInfo.polygonOpacity != objectInfo.tmpPolygonOpacity )
        {
            objectInfo.polygonOpacity = objectInfo.tmpPolygonOpacity;
            objectInfo.needSameTimeStepReplace = true;
            patch[QString::fromUtf8( Protocol::Key::PolygonOpacity )] = objectInfo.polygonOpacity;
        }

        // NOTE:フォーカス対象の
        if( objectInfo.isFocus )
        {
            resultMinObjectCoords.x() = std::min( resultMinObjectCoords.x(), objectInfo.minObjectCoord.x() );
            resultMinObjectCoords.y() = std::min( resultMinObjectCoords.y(), objectInfo.minObjectCoord.y() );
            resultMinObjectCoords.z() = std::min( resultMinObjectCoords.z(), objectInfo.minObjectCoord.z() );

            resultMaxObjectCoords.x() = std::max( resultMaxObjectCoords.x(), objectInfo.maxObjectCoord.x() );
            resultMaxObjectCoords.y() = std::max( resultMaxObjectCoords.y(), objectInfo.maxObjectCoord.y() );
            resultMaxObjectCoords.z() = std::max( resultMaxObjectCoords.z(), objectInfo.maxObjectCoord.z() );
        }

        // モデルへ反映
        nameItem->setData( QVariant::fromValue( objectInfo ), Qt::UserRole );

        // uuid 以外が入っていれば送信対象
        if( patch.keys().size() > 1 )
        {
            patchArray.append( patch );
        }
    }

    // NOTE:resultObjectCoordsの反映
    m_result_min_object_coords = resultMinObjectCoords;
    m_result_max_object_coords = resultMaxObjectCoords;

    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem = m_model->item( row, 0 );
        if( !nameItem ) continue;

        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        auto info = var.value<ObjectInfoExtractor::ObjectInfo>();
        if( info.objectID.first != -1 && info.objectID.second != -1 )
        {
            m_screen->scene()->object( info.objectID.first )->setMinMaxObjectCoords( m_result_min_object_coords, m_result_max_object_coords );
            m_screen->scene()->object( info.objectID.first )->setMinMaxExternalCoords( m_result_min_object_coords, m_result_max_object_coords );
        }
    }

    m_screen->scene()->objectManager()->push_centering_xform();
    emit updateFocus( m_result_min_object_coords, m_result_max_object_coords );
    m_screen->scene()->objectManager()->updateMinMaxCoords();
    m_screen->scene()->objectManager()->updateExternalCoords();
    m_screen->scene()->objectManager()->pop_centering_xform();
    emit updateTranslation();
    m_screen->update();

    // WebSocket：部分更新 + 全体座標送信
    if( ( *m_viz_mode == Viz::Mode::LocalClientAndServer  ||
          *m_viz_mode == Viz::Mode::RemoteClientAndServer ||
          *m_viz_mode == Viz::Mode::RemoteInSitu ) &&
          m_web_sockets->isConnected() )
    {
        QJsonObject objectInfoParameter;
        objectInfoParameter[QString::fromUtf8( Protocol::Key::Event )] = QString::fromUtf8( Protocol::Events::ObjectInfoParameter );

        QJsonArray minCoords;
        minCoords.append( m_result_min_object_coords.x() );
        minCoords.append( m_result_min_object_coords.y() );
        minCoords.append( m_result_min_object_coords.z() );

        QJsonArray maxCoords;
        maxCoords.append( m_result_max_object_coords.x() );
        maxCoords.append( m_result_max_object_coords.y() );
        maxCoords.append( m_result_max_object_coords.z() );

        objectInfoParameter[QString::fromUtf8( Protocol::Key::ResultMinObjectCoords )] = minCoords;
        objectInfoParameter[QString::fromUtf8( Protocol::Key::ResultMaxObjectCoords )] = maxCoords;

        // 差分がある場合のみ Objects を付ける
        if( !patchArray.isEmpty() ) objectInfoParameter[QString::fromUtf8( Protocol::Key::Objects )] = patchArray;

        m_web_sockets->text()->sendTextMessage( QJsonDocument( objectInfoParameter ).toJson( QJsonDocument::Compact ) );
    }
}

void ObjectEditor::dataRequestComplete( int requestTimeStep )
{
    updateVisibility( requestTimeStep );

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

    emit dataRequestCompleted( requestTimeStep );
    m_screen->update();
}
