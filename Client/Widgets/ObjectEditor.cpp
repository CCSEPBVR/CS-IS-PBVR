#include "ObjectEditor.h"
#include "ui_ObjectEditor.h"
#include "EnsembleTransferFunctionEditor.h"

#include <QDebug>

namespace
{
bool IsHybridServerManagedFormat( const ObjectInfoExtractor::Format format )
{
    return format == ObjectInfoExtractor::Format::ClientServerPointObject ||
           format == ObjectInfoExtractor::Format::InsituServerPointObject ||
           format == ObjectInfoExtractor::Format::ServerGlyphObject;
}

bool IsServerManagedObject( const Viz::Mode mode, const ObjectInfoExtractor::Format format )
{
    switch( mode )
    {
    case Viz::Mode::Local:
        return false;
    case Viz::Mode::LocalClientAndServer:
        return IsHybridServerManagedFormat( format );
    case Viz::Mode::RemoteClientAndServer:
    case Viz::Mode::RemoteInSitu:
        return true;
    default:
        return false;
    }
}
}

// FIXME:エクスポート機能が未実装です
ObjectEditor::ObjectEditor( kvs::qt::jaea::Screen* screen, WebSocketPair* websockets, Viz::Mode* vizMode, QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::ObjectEditor )
    , m_screen( screen )
    , m_web_sockets( websockets )
    , m_viz_mode( vizMode )
    , m_model( new QStandardItemModel( this ) )
{
    ui->setupUi( this );

    // NOTE:(extraOpacityFactor)は一時的に無効にしています。必要になったら下記のコードの削除とextraOpacityFactorに関係するコードのコメント文を元に戻せば使えるはずです。
    ui->extraOpacityFactorLabel->setVisible( false );
    ui->extraOpacityFactorDoubleSpinBox->setVisible( false );

    m_model->setHorizontalHeaderLabels( { "Name", "Format", "Display", "Keep Initial", "Keep Final" } );
    ui->treeView->setModel( m_model );

    ui->exportPushButton->setCheckable( true );
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
            // NOTE:(extraOpacityFactor)は一時的に無効にしています。必要になったら下記のコードの削除とextraOpacityFactorに関係するコードのコメント文を元に戻せば使えるはずです。
            // ui->extraOpacityFactorLabel , ui->extraOpacityFactorDoubleSpinBox,
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

    // NOTE:起動直後にオブジェクトはないので非表示にする。
    setWidgetsVisible( m_group_common_object_widgets             , false );
    setWidgetsVisible( m_group_common_server_point_object_widgets, false );
    setWidgetsVisible( m_group_client_server_point_object_widgets, false );
    setWidgetsVisible( m_group_nontexture_polygon_object_widgets , false );

    connect( ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ObjectEditor::onSelectionChanged );

    // 全オブジェクト共通
    connect( ui->focusCheckBox                  , &QCheckBox::toggled                   , this, &ObjectEditor::onFocusCheckBoxToggled );

    // サーバポイントオブジェクト(ClientServer/In-Situ共通)
    connect( ui->particleLimitSpinBox           , &QSpinBox::valueChanged               , this, &ObjectEditor::onParticleLimitSpinBoxValueChanged );
    // NOTE:(extraOpacityFactor)は一時的に無効にしています。必要になったら下記のコードの削除とextraOpacityFactorに関係するコードのコメント文を元に戻せば使えるはずです。
    // connect( ui->extraOpacityFactorDoubleSpinBox, &QDoubleSpinBox::valueChanged         , this, &ObjectEditor::onExtraOpacityFactorDoubleSpinBoxValueChanged );

    // サーバポイントオブジェクト(ClientServerのみ)
    connect( ui->coordinateXLineEdit            , &QLineEdit::textChanged               , this, &ObjectEditor::onCoordinateLineEditTextChanged );
    connect( ui->coordinateYLineEdit            , &QLineEdit::textChanged               , this, &ObjectEditor::onCoordinateLineEditTextChanged );
    connect( ui->coordinateZLineEdit            , &QLineEdit::textChanged               , this, &ObjectEditor::onCoordinateLineEditTextChanged );
    connect( ui->exportPushButton               , &QPushButton::clicked                 , this, &ObjectEditor::onExportButtonClicked );

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

void ObjectEditor::reset()
{
    if( !m_model ) { return; }

    m_pending_request_time_step = -1;
    m_is_waiting_local_request = false;
    m_is_waiting_server_request = false;

    if( m_screen && m_screen->scene() )
    {
        const int rows = m_model->rowCount();
        for( int row = 0; row < rows; ++row )
        {
            QStandardItem* item = m_model->item( row, 0 );
            if( !item ) continue;

            const QVariant v = item->data( Qt::UserRole );
            if( !v.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            const auto info = v.value<ObjectInfoExtractor::ObjectInfo>();
            if( info.objectID.first != -1 )
            {
                m_screen->scene()->removeObject( info.objectID.first );
            }
        }
    }

    if( m_model->rowCount() > 0 )
    {
        m_model->removeRows( 0, m_model->rowCount() );
    }

    calculateTotalMinMaxTimeStep();
    if( m_screen ) m_screen->update();

    setWidgetsVisible( m_group_common_object_widgets,              false );
    setWidgetsVisible( m_group_common_server_point_object_widgets, false );
    setWidgetsVisible( m_group_client_server_point_object_widgets, false );
    setWidgetsVisible( m_group_nontexture_polygon_object_widgets,  false );
}

void ObjectEditor::setEnsembleTransferFunctionEditor( EnsembleTransferFunctionEditor* editor )
{
    m_ensemble_transfer_function_editor = editor;
}

void ObjectEditor::onOperatorStateUpdate( const bool operatorState )
{
    m_is_operator = operatorState;

    const int rows = m_model->rowCount();
    for( int r = 0; r < rows; ++r )
    {
        if( auto* displayItem     = m_model->item( r, 2 ) ) { displayItem    ->setEnabled( m_is_operator ); }
        if( auto* keepInitialItem = m_model->item( r, 3 ) ) { keepInitialItem->setEnabled( m_is_operator ); }
        if( auto* keepFinalItem   = m_model->item( r, 4 ) ) { keepFinalItem  ->setEnabled( m_is_operator ); }
    }

    ui->focusCheckBox                  ->setEnabled( m_is_operator );
    ui->particleLimitSpinBox           ->setEnabled( m_is_operator );
    ui->extraOpacityFactorDoubleSpinBox->setEnabled( m_is_operator );
    ui->coordinateXLineEdit            ->setEnabled( m_is_operator );
    ui->coordinateYLineEdit            ->setEnabled( m_is_operator );
    ui->coordinateZLineEdit            ->setEnabled( m_is_operator );
    ui->colorClickableLabel            ->setEnabled( m_is_operator );
    ui->opacityDoubleSpinBox           ->setEnabled( m_is_operator );
    ui->browsePushButton               ->setEnabled( m_is_operator );
    ui->deletePushButton               ->setEnabled( m_is_operator );
    ui->applyPushButton                ->setEnabled( m_is_operator );
}

void ObjectEditor::onUnpack( const QByteArray& binary )
{
    qDebug() << "[Client][ObjectEditor] onUnpack bytes =" << binary.size();
    if( binary.size() < static_cast<int>( sizeof(int) ) )
    {
        qDebug() << "[Client][ObjectEditor] binary too small for timestep.";
        return;
    }

    const char* dataPtr = binary.constData();
    std::size_t offset = 0;

    // Time Step
    int timeStep = 0;
    std::memcpy( &timeStep, dataPtr + offset, sizeof(int) );
    offset += sizeof(int);
    qDebug() << "[Client][ObjectEditor] unpack timestep =" << timeStep
             << "model rows =" << m_model->rowCount();

    // 送信されてきたUUIDを記録する
    std::unordered_set<std::string> receivedUUIDs;
    int receivedObjects = 0;
    int matchedObjects = 0;

    while( offset < static_cast<size_t>( binary.size() ) )
    {
        // NOTE:UUIDの読み取り
        uint32_t uuidLen = 0;

        std::memcpy( &uuidLen, dataPtr + offset, sizeof(uint32_t) );
        offset += sizeof(uint32_t);

        if( offset + uuidLen > static_cast<size_t>( binary.size() ) ) break; // uuidLenが正常値か検証

        std::string uuid( uuidLen, '\0' );
        std::memcpy( uuid.data(), dataPtr + offset, uuidLen );
        offset += uuidLen;
        ++receivedObjects;
        qDebug() << "[Client][ObjectEditor] object payload uuid =" << QString::fromStdString( uuid )
                 << "uuidLen =" << uuidLen
                 << "offset =" << static_cast<qulonglong>( offset );

        receivedUUIDs.insert( uuid );

        int currentImportedTimeStep;
        std::memcpy( &currentImportedTimeStep, dataPtr + offset, sizeof(int) );
        offset += sizeof(int);

        // NOTE:モデル内のUUIDと一致する場合objectを更新する。
        bool matched = false;
        for( int row = 0; row < m_model->rowCount(); ++row )
        {
            QStandardItem* nameItem = m_model->item( row, 0 );
            QVariant var = nameItem->data( Qt::UserRole );
            if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

            if( info.uuid != uuid ) continue;
            matched = true;
            ++matchedObjects;

            // NOTE:UUIDが一致したのでobjectを更新

            info.currentImportedTimeStep = currentImportedTimeStep;

            switch( info.format )
            {
            case ObjectInfoExtractor::ClientServerPointObject:
            case ObjectInfoExtractor::InsituServerPointObject:
            case ObjectInfoExtractor::PointObjectKVSML:
            {
                std::size_t numberOfVertices = 0;
                std::memcpy( &numberOfVertices, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);
                qDebug() << "[Client][ObjectEditor] point object matched row =" << row
                         << "vertices =" << static_cast<qulonglong>( numberOfVertices )
                         << "currentImportedTimeStep =" << currentImportedTimeStep;

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
                std::size_t numberOfVertices = 0;
                std::memcpy( &numberOfVertices, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);
                qDebug() << "[Client][ObjectEditor] point object LAS/PTS matched row =" << row
                         << "vertices =" << static_cast<qulonglong>( numberOfVertices )
                         << "currentImportedTimeStep =" << currentImportedTimeStep;

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

                std::size_t nCoords;
                std::memcpy( &nCoords, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);
                qDebug() << "[Client][ObjectEditor] polygon/glyph matched row =" << row
                         << "coords count =" << static_cast<qulonglong>( nCoords )
                         << "currentImportedTimeStep =" << currentImportedTimeStep;

                kvs::ValueArray<kvs::Real32> coords( nCoords );
                std::memcpy( coords.data(), dataPtr + offset, sizeof(kvs::Real32) * nCoords );
                offset += sizeof(kvs::Real32) * nCoords;

                std::size_t nColors;
                std::memcpy( &nColors, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt8> colors( nColors );
                std::memcpy( colors.data(), dataPtr + offset, sizeof(kvs::UInt8) * nColors );
                offset += sizeof(kvs::UInt8) * nColors;

                std::size_t nNormals;
                std::memcpy( &nNormals, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> normals( nNormals );
                std::memcpy( normals.data(), dataPtr + offset, sizeof(kvs::Real32) * nNormals );
                offset += sizeof(kvs::Real32) * nNormals;

                std::size_t nConnections;
                std::memcpy( &nConnections, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt32> connections( nConnections );
                std::memcpy( connections.data(), dataPtr + offset, sizeof(kvs::UInt32) * nConnections );
                offset += sizeof(kvs::UInt32) * nConnections;

                std::size_t nOpacities;
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

                std::size_t nCoords;
                std::memcpy( &nCoords, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> coords( nCoords );
                std::memcpy( coords.data(), dataPtr + offset, sizeof(kvs::Real32) * nCoords );
                offset += sizeof(kvs::Real32) * nCoords;

                std::size_t nColors;
                std::memcpy( &nColors, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt8> colors( nColors );
                std::memcpy( colors.data(), dataPtr + offset, sizeof(kvs::UInt8) * nColors );
                offset += sizeof(kvs::UInt8) * nColors;

                std::size_t nNormals;
                std::memcpy( &nNormals, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> normals( nNormals );
                std::memcpy( normals.data(), dataPtr + offset, sizeof(kvs::Real32) * nNormals );
                offset += sizeof(kvs::Real32) * nNormals;

                std::size_t nConnections;
                std::memcpy( &nConnections, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt32> connections( nConnections );
                std::memcpy( connections.data(), dataPtr + offset, sizeof(kvs::UInt32) * nConnections );
                offset += sizeof(kvs::UInt32) * nConnections;

                std::size_t nOpacities;
                std::memcpy( &nOpacities, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt8> opacities( nOpacities );
                std::memcpy( opacities.data(), dataPtr + offset, sizeof(kvs::UInt8) * nOpacities );
                offset += sizeof(kvs::UInt8) * nOpacities;

                std::size_t nTexture2DCoords;
                std::memcpy( &nTexture2DCoords, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> texture2DCoords( nTexture2DCoords );
                std::memcpy( texture2DCoords.data(), dataPtr + offset, sizeof(kvs::Real32) * nTexture2DCoords );
                offset += sizeof(kvs::Real32) * nTexture2DCoords;

                std::size_t nTextureIds;
                std::memcpy( &nTextureIds, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt32> textureIds( nTextureIds );
                std::memcpy( textureIds.data(), dataPtr + offset, sizeof(kvs::UInt32) * nTextureIds );
                offset += sizeof(kvs::UInt32) * nTextureIds;

                // --- mapIdToColorArray ---
                std::size_t mapColorSize;
                memcpy( &mapColorSize, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                std::map<kvs::UInt32, kvs::ValueArray<kvs::UInt8>> mapColor;

                for( std::size_t i = 0; i < mapColorSize; i++ )
                {
                    kvs::UInt32 id;
                    memcpy( &id, dataPtr + offset, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);

                    std::size_t arrSize;
                    memcpy( &arrSize, dataPtr + offset, sizeof(size_t) );
                    offset += sizeof(size_t);

                    kvs::ValueArray<kvs::UInt8> arr(arrSize);
                    memcpy( arr.data(), dataPtr + offset, arrSize );
                    offset += arrSize;

                    mapColor[id] = arr;
                }

                // --- width ---
                std::size_t mapWSize;
                memcpy( &mapWSize, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                std::map<kvs::UInt32, kvs::UInt32> mapW;

                for( std::size_t i = 0; i < mapWSize; i++ )
                {
                    kvs::UInt32 id, w;
                    memcpy( &id, dataPtr + offset, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);
                    memcpy( &w, dataPtr + offset, sizeof(kvs::UInt32) );
                    offset += sizeof(kvs::UInt32);
                    mapW[id] = w;
                }

                // --- height ---
                std::size_t mapHSize;
                memcpy( &mapHSize, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                std::map<kvs::UInt32, kvs::UInt32> mapH;

                for( std::size_t i = 0; i < mapHSize; i++ )
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

                std::size_t nCoords;
                std::memcpy( &nCoords, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> coords( nCoords );
                std::memcpy( coords.data(), dataPtr + offset, sizeof(kvs::Real32) * nCoords );
                offset += sizeof(kvs::Real32) * nCoords;

                std::size_t nColors;
                std::memcpy( &nColors, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt8> colors( nColors );
                std::memcpy( colors.data(), dataPtr + offset, sizeof(kvs::UInt8) * nColors );
                offset += sizeof(kvs::UInt8) * nColors;

                std::size_t nNormals;
                std::memcpy( &nNormals, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::Real32> normals( nNormals );
                std::memcpy( normals.data(), dataPtr + offset, sizeof(kvs::Real32) * nNormals );
                offset += sizeof(kvs::Real32) * nNormals;

                std::size_t nConnections;
                std::memcpy( &nConnections, dataPtr + offset, sizeof(size_t) );
                offset += sizeof(size_t);

                kvs::ValueArray<kvs::UInt32> connections( nConnections );
                std::memcpy( connections.data(), dataPtr + offset, sizeof(kvs::UInt32) * nConnections );
                offset += sizeof(kvs::UInt32) * nConnections;

                std::size_t nSizes;
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
        if( !matched )
        {
            qDebug() << "[Client][ObjectEditor] WARNING: received uuid is not in model. uuid ="
                     << QString::fromStdString( uuid )
                     << "offset =" << static_cast<qulonglong>( offset );
        }
    }

    qDebug() << "[Client][ObjectEditor] unpack loop done receivedObjects =" << receivedObjects
             << "matchedObjects =" << matchedObjects
             << "final offset =" << static_cast<qulonglong>( offset )
             << "binary size =" << binary.size();

    // NOTE:受信していないUUIDのobjectをnullptrに更新する
    for( int row = 0; row < m_model->rowCount(); ++row )
    {
        QStandardItem* nameItem = m_model->item( row, 0 );
        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

        if( IsServerManagedObject( *m_viz_mode, info.format ) && receivedUUIDs.find( info.uuid ) == receivedUUIDs.end() )
        {
            info.object = nullptr;
            nameItem->setData( QVariant::fromValue( info ), Qt::UserRole );
        }
    }

    completeServerDataRequest( timeStep );
}

void ObjectEditor::onReceiveObjectInfoParameter( const QJsonObject& payload )
{
    if( payload.contains( QString::fromUtf8( Protocol::Key::ResultMinObjectCoords ) ) )
    {
        QJsonArray a = payload[QString::fromUtf8( Protocol::Key::ResultMinObjectCoords )].toArray();
        if( a.size() == 3 )
        {
            m_result_min_object_coords = kvs::Vec3( a[0].toDouble(), a[1].toDouble(), a[2].toDouble() );
        }
    }

    if( payload.contains( QString::fromUtf8( Protocol::Key::ResultMaxObjectCoords ) ) )
    {
        QJsonArray a = payload[QString::fromUtf8( Protocol::Key::ResultMaxObjectCoords )].toArray();
        if( a.size() == 3 )
        {
            m_result_max_object_coords = kvs::Vec3( a[0].toDouble(), a[1].toDouble(), a[2].toDouble() );
        }
    }

    if( payload.contains( QString::fromUtf8( Protocol::Key::Objects ) ) && payload[QString::fromUtf8( Protocol::Key::Objects )].isArray() )
    {
        const QJsonArray objects = payload[QString::fromUtf8( Protocol::Key::Objects )].toArray();

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
                // NOTE:(extraOpacityFactor)は一時的に無効にしています。必要になったら下記のコードの削除とextraOpacityFactorに関係するコードのコメント文を元に戻せば使えるはずです。
                // if( patch.contains( QString::fromUtf8( Protocol::Key::ExtraOpacityFactor ) ) )
                // {
                //     info.tmpExtraOpacityFactor = static_cast<float>( patch[QString::fromUtf8( Protocol::Key::ExtraOpacityFactor )].toDouble() );
                //     info.extraOpacityFactor    = static_cast<float>( patch[QString::fromUtf8( Protocol::Key::ExtraOpacityFactor )].toDouble() );
                // }

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

    QModelIndex currentIndex = ui->treeView->currentIndex();
    if( currentIndex.isValid() )
    {
        updateUI( currentIndex );
    }
}

void ObjectEditor::onReceiveSelectedFile( const QJsonObject& payload )
{
    ObjectInfoExtractor::ObjectInfo objectInfo;

    // 全オブジェクト共通
    objectInfo.uuid                  = payload[QString::fromUtf8( Protocol::Key::UUID )].toString().toUtf8();
    objectInfo.tmpIsDisplay          = payload[QString::fromUtf8( Protocol::Key::TmpIsDisplay )].toBool();
    objectInfo.isDisplay             = payload[QString::fromUtf8( Protocol::Key::IsDisplay )].toBool();
    objectInfo.tmpIsKeepInitial      = payload[QString::fromUtf8( Protocol::Key::TmpIsKeepInitial )].toBool();
    objectInfo.isKeepInitial         = payload[QString::fromUtf8( Protocol::Key::IsKeepInitial )].toBool();
    objectInfo.tmpIsKeepFinal        = payload[QString::fromUtf8( Protocol::Key::TmpIsKeepFinal )].toBool();
    objectInfo.isKeepFinal           = payload[QString::fromUtf8( Protocol::Key::IsKeepFinal )].toBool();
    objectInfo.name                  = payload[QString::fromUtf8( Protocol::Key::Name )].toString().toUtf8();
    objectInfo.extension             = payload[QString::fromUtf8( Protocol::Key::Extension )].toString().toUtf8();
    objectInfo.directory             = payload[QString::fromUtf8( Protocol::Key::Directory )].toString().toUtf8();
    objectInfo.format                = static_cast<ObjectInfoExtractor::Format>( payload[QString::fromUtf8( Protocol::Key::Format )].toInt() );
    QJsonArray timeStepArray         = payload[QString::fromUtf8( Protocol::Key::TimeStep )].toArray();
    objectInfo.timeStep              = { timeStepArray[0].toInt(), timeStepArray[1].toInt() };
    objectInfo.tmpIsFocus            = payload[QString::fromUtf8( Protocol::Key::TmpIsFocus )].toBool();
    objectInfo.isFocus               = payload[QString::fromUtf8( Protocol::Key::IsFocus )].toBool();
    auto minObjectCoordArray         = payload[QString::fromUtf8( Protocol::Key::MinObjectCoord )].toArray();
    objectInfo.minObjectCoord        = { minObjectCoordArray[0].toDouble(), minObjectCoordArray[1].toDouble(), minObjectCoordArray[2].toDouble() };
    auto maxObjectCoordArray         = payload[QString::fromUtf8( Protocol::Key::MaxObjectCoord )].toArray();
    objectInfo.maxObjectCoord        = { maxObjectCoordArray[0].toDouble(), maxObjectCoordArray[1].toDouble(), maxObjectCoordArray[2].toDouble() };
    auto minExternalCoordArray       = payload[QString::fromUtf8( Protocol::Key::MinExternalCoord )].toArray();
    objectInfo.minExternalCoord      = { minExternalCoordArray[0].toDouble(), minExternalCoordArray[1].toDouble(), minExternalCoordArray[2].toDouble() };
    auto maxExternalCoordArray       = payload[QString::fromUtf8( Protocol::Key::MaxExternalCoord )].toArray();
    objectInfo.maxExternalCoord      = { maxExternalCoordArray[0].toDouble(), maxExternalCoordArray[1].toDouble(), maxExternalCoordArray[2].toDouble() };

    // サーバポイントオブジェクト(ClientServer/In-Situ共通)
    objectInfo.tmpParticleLimit      = payload[QString::fromUtf8( Protocol::Key::TmpParticleLimit )].toInt();
    objectInfo.particleLimit         = payload[QString::fromUtf8( Protocol::Key::ParticleLimit )].toInt();
    // NOTE:(extraOpacityFactor)は一時的に無効にしています。必要になったら下記のコードの削除とextraOpacityFactorに関係するコードのコメント文を元に戻せば使えるはずです。
    // objectInfo.tmpExtraOpacityFactor = static_cast<float>( payload[QString::fromUtf8( Protocol::Key::TmpExtraOpacityFactor )].toDouble() );
    // objectInfo.extraOpacityFactor    = static_cast<float>( payload[QString::fromUtf8( Protocol::Key::ExtraOpacityFactor )].toDouble() );

    // サーバポイントオブジェクト(ClientServerのみ)
    objectInfo.numberOfVector        = payload[QString::fromUtf8( Protocol::Key::NumberOfVector )].toInt();
    objectInfo.numberOfElements      = payload[QString::fromUtf8( Protocol::Key::NumberOfElements )].toInt();
    objectInfo.numberOfSubvolume     = payload[QString::fromUtf8( Protocol::Key::NumberOfSubvolume)].toInt();
    objectInfo.numberOfNodes         = payload[QString::fromUtf8( Protocol::Key::NumberOfNodes )].toInt();
    objectInfo.elementType           = payload[QString::fromUtf8( Protocol::Key::ElementType )].toInt();
    objectInfo.fileType              = payload[QString::fromUtf8( Protocol::Key::FileType )].toInt();
    objectInfo.stepNumber            = payload[QString::fromUtf8( Protocol::Key::StepNumber )].toInt();
    objectInfo.tmpCoordinateX        = payload[QString::fromUtf8( Protocol::Key::TmpCoordinateX )].toString().toUtf8();
    objectInfo.coordinateX           = payload[QString::fromUtf8( Protocol::Key::CoordinateX )].toString().toUtf8();
    objectInfo.tmpCoordinateY        = payload[QString::fromUtf8( Protocol::Key::TmpCoordinateY )].toString().toUtf8();
    objectInfo.coordinateY           = payload[QString::fromUtf8( Protocol::Key::CoordinateY )].toString().toUtf8();
    objectInfo.tmpCoordinateZ        = payload[QString::fromUtf8( Protocol::Key::TmpCoordinateZ )].toString().toUtf8();
    objectInfo.coordinateZ           = payload[QString::fromUtf8( Protocol::Key::IsExport )].toBool();

    // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
    auto tmpPolygonColorArray        = payload[QString::fromUtf8( Protocol::Key::TmpPolygonColor )].toArray();
    objectInfo.tmpPolygonColor       = kvs::RGBColor( tmpPolygonColorArray[0].toInt(), tmpPolygonColorArray[1].toInt(), tmpPolygonColorArray[2].toInt() );
    auto polygonColorArray           = payload[QString::fromUtf8( Protocol::Key::PolygonColor )].toArray();
    objectInfo.polygonColor          = kvs::RGBColor( polygonColorArray[0].toInt(), polygonColorArray[1].toInt(), polygonColorArray[2].toInt() );
    objectInfo.tmpPolygonOpacity     = static_cast<float>( payload[QString::fromUtf8( Protocol::Key::TmpPolygonOpacity )].toDouble() );
    objectInfo.polygonOpacity        = static_cast<float>( payload[QString::fromUtf8( Protocol::Key::PolygonOpacity )].toDouble() );

    addObjectInfoToModel( objectInfo );

    // FIXME:ここで成分数のアップデートを行っていますが、適切な場所なのか不明です
    const bool isServerPointObject = ( objectInfo.format == ObjectInfoExtractor::ClientServerPointObject ) || ( objectInfo.format == ObjectInfoExtractor::InsituServerPointObject );
    if( isServerPointObject ) emit updateNumberOfVector( objectInfo.numberOfVector );
}

void ObjectEditor::onReceiveObjectDelete( const QJsonObject& payload )
{
    if( !payload.contains( QString::fromUtf8( Protocol::Key::UUID ) ) ) return;

    const QString uuid = payload[QString::fromUtf8( Protocol::Key::UUID )].toString().toUtf8();

    bool deletedWasFocus = false;

    for( int row = 0; row < m_model->rowCount(); ++row )
    {
        QStandardItem* item = m_model->item( row, 0 );
        if( !item ) continue;

        QVariant var = item->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        auto info = var.value<ObjectInfoExtractor::ObjectInfo>();
        if( QString::fromUtf8( info.uuid ) != uuid ) continue;

        deletedWasFocus = info.tmpIsFocus;

        if( info.objectID.first != -1 ) m_screen->scene()->removeObject( info.objectID.first );

        m_model->removeRow( row );
        break;
    }

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

    QModelIndex currentIndex = ui->treeView->currentIndex();
    if( currentIndex.isValid() )
    {
        updateUI( currentIndex );
    }
}

void ObjectEditor::onUpdateObjectLatestTimeStep( const QJsonObject& payload )
{
    const QString kUpdateMaxTimeStep = QString::fromUtf8( Protocol::Key::UpdateMaxTimeStep );
    if( !payload.contains( kUpdateMaxTimeStep ) ) return;
    const QJsonValue v = payload.value( kUpdateMaxTimeStep );
    const int latest   = v.toInt();

    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem = m_model->item( row, 0 );
        if( !nameItem ) continue;

        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        auto info = var.value<ObjectInfoExtractor::ObjectInfo>();

        if( info.format == ObjectInfoExtractor::Format::InsituServerPointObject ||
            info.format == ObjectInfoExtractor::Format::ServerGlyphObject )
        {
            info.timeStep.second = std::max( info.timeStep.first, latest );
            nameItem->setData( QVariant::fromValue(info), Qt::UserRole );
        }
    }

    calculateTotalMinMaxTimeStep();
}

void ObjectEditor::onGlyphParameterUpdate()
{
    sendNeedSameTimeStepReplacePatches(
        []( ObjectInfoExtractor::Format format )
        {
            return format == ObjectInfoExtractor::Format::ServerGlyphObject;
        } );
}

void ObjectEditor::onTransferFunctionUpdate()
{
    sendNeedSameTimeStepReplacePatches(
        []( ObjectInfoExtractor::Format format )
        {
            return ( format == ObjectInfoExtractor::Format::ClientServerPointObject ) ||
                   ( format == ObjectInfoExtractor::Format::InsituServerPointObject );
        } );
}

void ObjectEditor::onRequestDataAt( int requestTimeStep )
{
    switch( *m_viz_mode )
    {
    case Viz::Mode::Local:
    {
        Worker* worker = new Worker( m_model, m_screen, requestTimeStep, m_result_min_object_coords, m_result_max_object_coords, false );
        QThread* thread = new QThread;

        worker->moveToThread( thread );

        connect( thread, &QThread::started , worker, &Worker::process );
        connect( worker, &Worker::done     , thread, &QThread::quit );
        connect( worker, &Worker::done     , worker, &Worker::deleteLater );
        connect( thread, &QThread::finished, thread, &QThread::deleteLater );
        connect( worker, &Worker::done     , this  , [this, requestTimeStep]() { completeLocalDataRequest( requestTimeStep ); } );

        thread->start();
        break;
    }
    case Viz::Mode::LocalClientAndServer:
    {
        beginHybridDataRequest( requestTimeStep );
        break;
    }
    case Viz::Mode::RemoteClientAndServer:
    case Viz::Mode::RemoteInSitu:
    {
        requestServerDataAt( requestTimeStep );
        break;
    }
    default:
        break;
    }
}

void ObjectEditor::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ObjectEditor::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ObjectEditor::setWidgetsVisible( const QList<QWidget*>& widgets, const bool visible )
{
    for( auto* w : widgets ) w->setVisible( visible );
}

void ObjectEditor::updateUI( const QModelIndex& index )
{
    if( !index.isValid() ) return;

    const QModelIndex nameIndex = index.sibling( index.row(), 0 );
    const QVariant var = nameIndex.data( Qt::UserRole );
    if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) return;

    const ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

    // 全オブジェクト共通
    ui->nameLineEdit             ->setText( QString::fromUtf8( info.name.c_str() ) );
    ui->directoryLineEdit        ->setText( QString::fromUtf8( info.directory.c_str() ) );
    ui->formatLineEdit           ->setText( QString::fromUtf8( ObjectInfoExtractor::formatToString( info.format ) ) );
    ui->timeStepMinLineEdit      ->setText( QString::number( info.timeStep.first ) );
    ui->timeStepMaxLineEdit      ->setText( QString::number( info.timeStep.second ) );
    ui->focusCheckBox            ->setChecked( info.tmpIsFocus );
    ui->minObjectXCoordLineEdit  ->setText( QString::number( info.minObjectCoord.x() ) );
    ui->minObjectYCoordLineEdit  ->setText( QString::number( info.minObjectCoord.y() ) );
    ui->minObjectZCoordLineEdit  ->setText( QString::number( info.minObjectCoord.z() ) );
    ui->maxObjectXCoordLineEdit  ->setText( QString::number( info.maxObjectCoord.x() ) );
    ui->maxObjectYCoordLineEdit  ->setText( QString::number( info.maxObjectCoord.y() ) );
    ui->maxObjectZCoordLineEdit  ->setText( QString::number( info.maxObjectCoord.z() ) );
    ui->minExternalXCoordLineEdit->setText( QString::number( info.minExternalCoord.x() ) );
    ui->minExternalYCoordLineEdit->setText( QString::number( info.minExternalCoord.y() ) );
    ui->minExternalZCoordLineEdit->setText( QString::number( info.minExternalCoord.z() ) );
    ui->maxExternalXCoordLineEdit->setText( QString::number( info.maxExternalCoord.x() ) );
    ui->maxExternalYCoordLineEdit->setText( QString::number( info.maxExternalCoord.y() ) );
    ui->maxExternalZCoordLineEdit->setText( QString::number( info.maxExternalCoord.z() ) );

    // サーバポイントオブジェクト(ClientServer/In-Situ共通)
    ui->particleLimitSpinBox            ->setValue( info.tmpParticleLimit );
    // NOTE:(extraOpacityFactor)は一時的に無効にしています。必要になったら下記のコードの削除とextraOpacityFactorに関係するコードのコメント文を元に戻せば使えるはずです。
    // ui->extraOpacityFactorDoubleSpinBox ->setValue( info.tmpExtraOpacityFactor );

    // サーバポイントオブジェクト(ClientServerのみ)
    ui->numberOfVectorLineEdit   ->setText( QString::number( info.numberOfVector ) );
    ui->numberOfElementsLineEdit ->setText( QString::number( info.numberOfElements ) );
    ui->numberOfSubvolumeLineEdit->setText( QString::number( info.numberOfSubvolume ) );
    ui->numberOfNodesLineEdit    ->setText( QString::number( info.numberOfNodes ) );
    ui->elementTypeLineEdit      ->setText( QString::number( info.elementType ) );
    ui->fileTypeLineEdit         ->setText( QString::number( info.fileType ) );
    ui->stepNumberLineEdit       ->setText( QString::number( info.stepNumber ) );
    ui->coordinateXLineEdit      ->setText( QString::fromUtf8( info.tmpCoordinateX ) );
    ui->coordinateYLineEdit      ->setText( QString::fromUtf8( info.tmpCoordinateY ) );
    ui->coordinateZLineEdit      ->setText( QString::fromUtf8( info.tmpCoordinateZ ) );
    ui->exportPushButton         ->setChecked( info.isExport );

    // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
    QPalette palette = ui->colorClickableLabel->palette();
    palette.setColor( QPalette::Window, QColor( info.tmpPolygonColor.r(), info.tmpPolygonColor.g(), info.tmpPolygonColor.b() ) );
    ui->colorClickableLabel      ->setPalette( palette );
    ui->opacityDoubleSpinBox     ->setValue( info.tmpPolygonOpacity );

    // NOTE:info.formatに応じて各ウィジェットグループの表示/非表示を切り替える
    bool isObject                  = false;
    bool isCommonServerObject      = false;
    bool isClientServerObject      = false;
    bool isNonTexturePolygonObject = false;

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

void ObjectEditor::onSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
    Q_UNUSED( deselected );

    const auto indexes = selected.indexes();
    if( indexes.isEmpty() ) return;

    updateUI( indexes.first() );
}

template<typename F>
void ObjectEditor::updateSelectedObjectInfoParameter( F func )
{
    const QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    index = index.sibling( index.row(), 0 );

    const QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) return;

    auto info = var.value<ObjectInfoExtractor::ObjectInfo>();
    func( info );

    m_model->setData( index, QVariant::fromValue( info ), Qt::UserRole );
}

void ObjectEditor::addObjectInfoToModel( ObjectInfoExtractor::ObjectInfo& objectInfo )
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

    // NOTE:display,keepInitial,keepFinalのチェックボックス有効にする。
    displayItem     ->setCheckable( true );
    keepInitialItem ->setCheckable( true );
    keepFinalItem   ->setCheckable( true );

    // NOTE:操作権限がない場合はチェックボックスを操作不可にする
    if( !m_is_operator )
    {
        displayItem    ->setEnabled( false );
        keepInitialItem->setEnabled( false );
        keepFinalItem  ->setEnabled( false );
    }

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

    if( objectInfo.tmpIsFocus ) { onApply(); }
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

void ObjectEditor::sendNeedSameTimeStepReplacePatches( const std::function<bool( ObjectInfoExtractor::Format )>& isTargetFormat )
{
    if( !( *m_viz_mode == Viz::Mode::LocalClientAndServer  ||
          *m_viz_mode == Viz::Mode::RemoteClientAndServer ||
          *m_viz_mode == Viz::Mode::RemoteInSitu ) ||
        !m_web_sockets->isConnected() )
    {
        return;
    }

    QJsonArray patchArray;

    const int rows = m_model->rowCount();
    for( int row = 0; row < rows; ++row )
    {
        QStandardItem* nameItem = m_model->item( row, 0 );
        if( !nameItem ) { continue; }

        const QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) { continue; }

        auto objectInfo = var.value<ObjectInfoExtractor::ObjectInfo>();

        if( !isTargetFormat( objectInfo.format ) ) { continue; }

        objectInfo.needSameTimeStepReplace = true;
        nameItem->setData( QVariant::fromValue( objectInfo ), Qt::UserRole );

        QJsonObject patch;
        patch[ QString::fromUtf8( Protocol::Key::UUID ) ] = QString::fromUtf8( objectInfo.uuid );
        patch[ QString::fromUtf8( Protocol::Key::NeedSameTimeStepReplace ) ] = true;
        patchArray.append( patch );
    }

    if( patchArray.isEmpty() ) { return; }

    QJsonObject msg;
    msg[ QString::fromUtf8( Protocol::Key::Event ) ]   = QString::fromUtf8( Protocol::Events::ServerSideSameTimeStepReplace );
    msg[ QString::fromUtf8( Protocol::Key::Objects ) ] = patchArray;

    m_web_sockets->text()->sendTextMessage( QJsonDocument( msg ).toJson( QJsonDocument::Compact ) );
}

void ObjectEditor::updateVisibility( int requestTimeStep )
{
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* item = m_model->item( row, 0 );
        if( !item ) continue;

        QVariant var = item->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

        int resultTimeStep = -1;

        if( info.isDisplay )
        {
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

#ifdef OPENXR_SCREEN
    const kvs::Xform initialXform
        (
            kvs::Mat4(
                1, 0, 0, 0 ,
                0, 1, 0, 0 ,
                0, 0, 1, 12,
                0, 0, 0, 1
                )
            );
    kvs::Vec3 translationOffset = m_screen->scene()->camera()->xform().translation() - initialXform.translation();
#endif

    switch( info.format )
    {
    case ObjectInfoExtractor::ClientServerPointObject:
    case ObjectInfoExtractor::InsituServerPointObject:
        particleBasedRenderer = std::make_unique<kvs::glsl::ParticleBasedRenderer>();
        particleBasedRenderer.get()->enableShuffle();
#ifdef OPENXR_SCREEN
        particleBasedRenderer.get()->setTranslationOffset( translationOffset );
        particleBasedRenderer.get()->setObjectDepth( m_screen->scene()->objectManager()->xform().scaling().z() / m_screen->scene()->camera()->xform().scaling().z() );
#endif
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
#ifdef OPENXR_SCREEN
        particleBasedRenderer.get()->setTranslationOffset( translationOffset );
        particleBasedRenderer.get()->setObjectDepth( m_screen->scene()->objectManager()->xform().scaling().z() / m_screen->scene()->camera()->xform().scaling().z() );
#endif
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

void ObjectEditor::exportPointObject( const ObjectInfoExtractor::ObjectInfo& info, int requestTimeStep )
{
    if( !info.isExport ) { return; }

    if( info.format != ObjectInfoExtractor::Format::ClientServerPointObject &&
        info.format != ObjectInfoExtractor::Format::InsituServerPointObject )
    {
        return;
    }

    auto* pointObject = static_cast<kvs::PointObject*>( info.object );
    if( !pointObject ) { return; }

    const QString exportDir = QString::fromStdString( info.exportFilePath );
    if( exportDir.isEmpty() ) return;

    const QString fullPath =
        exportDir + "/" +
        QString::fromStdString( info.name ) + "_" +
        QString( "%1" ).arg( requestTimeStep, 5, 10, QChar( '0' ) ) +
        ".kvsml";

    if( QFileInfo( fullPath ).exists() ) { return; }

    kvs::KVSMLPointObject* kvsml = new kvs::PointExporter<kvs::KVSMLPointObject>( pointObject );

    if( !kvsml ) return;

    kvsml->setWritingDataTypeToExternalBinary();
    kvsml->write( toNativePath( fullPath.toStdString() ) );
    delete kvsml;
}

void ObjectEditor::requestServerDataAt( const int requestTimeStep )
{
    if( !m_web_sockets->isConnected() ) return;

    QJsonArray resultMinObjectCoords;
    resultMinObjectCoords.append( m_result_min_object_coords.x() );
    resultMinObjectCoords.append( m_result_min_object_coords.y() );
    resultMinObjectCoords.append( m_result_min_object_coords.z() );

    QJsonArray resultMaxObjectCoords;
    resultMaxObjectCoords.append( m_result_max_object_coords.x() );
    resultMaxObjectCoords.append( m_result_max_object_coords.y() );
    resultMaxObjectCoords.append( m_result_max_object_coords.z() );

    QJsonObject request;
    request[QString::fromUtf8( Protocol::Key::Event )] = QString::fromUtf8( Protocol::Events::RequestDataAt );
    request[QString::fromUtf8( Protocol::Key::TimeStep )] = requestTimeStep;
    request[QString::fromUtf8( Protocol::Key::ResultMinObjectCoords )] = resultMinObjectCoords;
    request[QString::fromUtf8( Protocol::Key::ResultMaxObjectCoords )] = resultMaxObjectCoords;
    QString statistic;
    if( *m_viz_mode == Viz::Mode::RemoteInSitu )
    {
        statistic = m_ensemble_transfer_function_editor ?
            m_ensemble_transfer_function_editor->selectedStatistic() :
            QStringLiteral( "average" );
        request[QString::fromUtf8( Protocol::Key::Statistic )] = statistic;
    }

    qDebug() << "[Client][ObjectEditor] send RequestDataAt timestep =" << requestTimeStep
             << "statistics =" << ( statistic.isEmpty() ? QStringLiteral( "<none>" ) : statistic )
             << "text connected =" << m_web_sockets->text()->isValid()
             << "binary connected =" << m_web_sockets->binary()->isValid();
    m_web_sockets->text()->sendTextMessage( QJsonDocument( request ).toJson( QJsonDocument::Compact ) );
}

void ObjectEditor::beginHybridDataRequest( const int requestTimeStep )
{
    m_pending_request_time_step = requestTimeStep;
    m_is_waiting_local_request = true;
    m_is_waiting_server_request = m_web_sockets->isConnected();

    Worker* worker = new Worker( m_model, m_screen, requestTimeStep, m_result_min_object_coords, m_result_max_object_coords, true );
    QThread* thread = new QThread;

    worker->moveToThread( thread );

    connect( thread, &QThread::started , worker, &Worker::process );
    connect( worker, &Worker::done     , thread, &QThread::quit );
    connect( worker, &Worker::done     , worker, &Worker::deleteLater );
    connect( thread, &QThread::finished, thread, &QThread::deleteLater );
    connect( worker, &Worker::done     , this  , [this, requestTimeStep]() { completeLocalDataRequest( requestTimeStep ); } );

    thread->start();
    if( m_is_waiting_server_request )
    {
        requestServerDataAt( requestTimeStep );
    }
}

void ObjectEditor::completeLocalDataRequest( const int requestTimeStep )
{
    const bool is_hybrid_request =
        *m_viz_mode == Viz::Mode::LocalClientAndServer &&
        m_pending_request_time_step == requestTimeStep;

    dataRequestComplete( requestTimeStep, !is_hybrid_request, true, !is_hybrid_request );

    if( is_hybrid_request )
    {
        m_is_waiting_local_request = false;
        if( !m_is_waiting_server_request )
        {
            m_pending_request_time_step = -1;
            m_is_waiting_local_request = false;
            m_is_waiting_server_request = false;
            emit dataRequestCompleted( requestTimeStep );
        }
    }
}

void ObjectEditor::completeServerDataRequest( const int requestTimeStep )
{
    const bool is_hybrid_request =
        *m_viz_mode == Viz::Mode::LocalClientAndServer &&
        m_pending_request_time_step == requestTimeStep;

    dataRequestComplete( requestTimeStep, !is_hybrid_request, false, true );

    if( is_hybrid_request )
    {
        m_is_waiting_server_request = false;
        if( !m_is_waiting_local_request )
        {
            m_pending_request_time_step = -1;
            m_is_waiting_local_request = false;
            m_is_waiting_server_request = false;
            emit dataRequestCompleted( requestTimeStep );
        }
    }
}

void ObjectEditor::dataRequestComplete(
    const int requestTimeStep,
    const bool notifyCompletion,
    const bool processLocalObjects,
    const bool processServerObjects )
{
    updateVisibility( requestTimeStep );

    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* item = m_model->item( row, 0 ); // UserRoleにObjectInfoが格納されている列
        if( !item ) continue;

        QVariant var = item->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();
        const bool isServerManagedObject = IsServerManagedObject( *m_viz_mode, info.format );

        if( ( isServerManagedObject && !processServerObjects ) ||
            ( !isServerManagedObject && !processLocalObjects ) )
        {
            continue;
        }

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

            exportPointObject( info, requestTimeStep );
        }
        QVariant newVar;
        newVar.setValue( info );
        item->setData( newVar, Qt::UserRole );
    }

    if( notifyCompletion )
    {
        emit dataRequestCompleted( requestTimeStep );
    }
    m_screen->update();
}

void ObjectEditor::onFocusCheckBoxToggled( bool checked )
{
    updateSelectedObjectInfoParameter( [checked]( auto &info )
                                      {
                                          info.tmpIsFocus = checked;
                                      } );
}

void ObjectEditor::onParticleLimitSpinBoxValueChanged( int value )
{
    updateSelectedObjectInfoParameter( [value]( auto &info )
                                      {
                                          info.tmpParticleLimit = value;
                                      } );
}

void ObjectEditor::onExtraOpacityFactorDoubleSpinBoxValueChanged( double value )
{
    updateSelectedObjectInfoParameter( [value]( auto &info )
                                      {
                                          info.tmpExtraOpacityFactor = value;
                                      } );
}

void ObjectEditor::onCoordinateLineEditTextChanged()
{
    updateSelectedObjectInfoParameter( [this] ( auto &info )
                                      {
                                          info.tmpCoordinateX = ui->coordinateXLineEdit->text().toUtf8().constData();
                                          info.tmpCoordinateY = ui->coordinateYLineEdit->text().toUtf8().constData();
                                          info.tmpCoordinateZ = ui->coordinateZLineEdit->text().toUtf8().constData();
                                      } );
}

void ObjectEditor::onExportButtonClicked()
{
    bool checked = ui->exportPushButton->isChecked();

    if( !checked )
    {
        updateSelectedObjectInfoParameter(
            []( auto &info )
            {
                info.isExport       = false;
                info.exportFilePath = "";
            }
            );
        return;
    }

    QString exportDirPath = QFileDialog::getExistingDirectory(
        this,
        tr( "Select Export Directory" ),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    // キャンセルされた場合
    if( exportDirPath.isEmpty() )
    {
        ui->exportPushButton->setChecked( false );

        updateSelectedObjectInfoParameter(
            []( auto &info )
            {
                info.isExport       = false;
                info.exportFilePath = "";
            }
            );
        return;
    }

    // 正常に選択された場合
    updateSelectedObjectInfoParameter(
        [exportDirPath]( auto &info )
        {
            info.exportFilePath = exportDirPath.toStdString();
            info.isExport       = true;
        }
        );
}

void ObjectEditor::onColorLabelDoubleClicked()
{
    updateSelectedObjectInfoParameter( [this]( auto &info )
                                      {
                                          QColorDialog colorDialog;
                                          colorDialog.adjustSize();
                                          if( colorDialog.exec() == QDialog::Accepted )
                                          {
                                              QColor color     = colorDialog.selectedColor();
                                              QPalette palette = ui->colorClickableLabel->palette();
                                              palette.setColor( QPalette::Window, color );
                                              ui->colorClickableLabel->setPalette( palette );
                                              info.tmpPolygonColor.set( color.red(), color.green(), color.blue() );
                                          }
                                      } );
}

void ObjectEditor::onOpacityDoubleSpinBoxValueChanged( double value )
{
    updateSelectedObjectInfoParameter( [value]( auto &info )
                                      {
                                          info.tmpPolygonOpacity = value;
                                      } );
}

void ObjectEditor::onBrowse()
{
    // NOTE:
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
                addObjectInfoToModel( *objectInfoOpt );
            }
            else
            {
                emit updateStatusBarMessage( tr( "Failed to load file: %1" ).arg( filePath ) );
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
    const QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    index = index.sibling( index.row(), 0 );

    const QVariant var = index.data( Qt::UserRole );
    if ( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) return;

    const ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

    // NOTE:フォーカス再割当が必要か判定(削除対象がフォーカスで、他にフォーカスが無い場合)
    bool requireFocusOnOther = false;
    if( info.tmpIsFocus )
    {
        bool otherFocusedFound = false;

        for( int row = 0; row < m_model->rowCount(); ++row )
        {
            if( row == index.row() ) continue;

            QStandardItem* nameItem = m_model->item( row, 0 );
            if ( !nameItem ) continue;

            const QVariant otherVar = nameItem->data( Qt::UserRole );
            if( !otherVar.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            const auto otherInfo = otherVar.value<ObjectInfoExtractor::ObjectInfo>();
            if( otherInfo.tmpIsFocus )
            {
                otherFocusedFound = true;
                break;
            }
        }

        requireFocusOnOther = !otherFocusedFound;
    }

    // NOTE: フォーカスを他オブジェクトへ移す（必要な場合のみ、先頭の1つに付与）
    if( requireFocusOnOther )
    {
        for( int row = 0; row < m_model->rowCount(); ++row )
        {
            if( row == index.row() ){ continue; }

            QStandardItem* item = m_model->item( row, 0 );
            if( !item ) continue;

            const QVariant v = item->data( Qt::UserRole );
            if( !v.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            auto otherInfo = v.value<ObjectInfoExtractor::ObjectInfo>();
            otherInfo.tmpIsFocus = true;
            item->setData( QVariant::fromValue( otherInfo ), Qt::UserRole );
            break; // NOTE:1つだけフォーカス
        }
    }

    // NOTE:シーンから削除
    if( info.objectID.first != -1 )
    {
        m_screen->scene()->removeObject( info.objectID.first );
    }

    // NOTE:Modelから削除
    m_model->removeRow( index.row() );

    calculateTotalMinMaxTimeStep();
    m_screen->update();

    // NOTE:全削除時のUI非表示
    if( m_model->rowCount() == 0 )
    {
        setWidgetsVisible( m_group_common_object_widgets,              false );
        setWidgetsVisible( m_group_common_server_point_object_widgets, false );
        setWidgetsVisible( m_group_client_server_point_object_widgets, false );
        setWidgetsVisible( m_group_nontexture_polygon_object_widgets,  false );
    }

    const bool is_remote_mode =
        ( *m_viz_mode == Viz::Mode::LocalClientAndServer  ||
         *m_viz_mode == Viz::Mode::RemoteClientAndServer ||
         *m_viz_mode == Viz::Mode::RemoteInSitu );

    if( is_remote_mode && m_web_sockets->isConnected() )
    {
        QJsonObject msg;
        msg[QString::fromUtf8( Protocol::Key::Event )] = QString::fromUtf8( Protocol::Events::ObjectDelete );
        msg[QString::fromUtf8( Protocol::Key::UUID )]  = QString::fromUtf8( info.uuid );
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

    // NOTE:差分送信用パッチ
    QJsonArray patchArray;

    for( int row = 0; row < m_model->rowCount(); ++row )
    {
        QStandardItem* nameItem        = m_model->item( row, 0 );
        QStandardItem* displayItem     = m_model->item( row, 2 );
        QStandardItem* keepInitialItem = m_model->item( row, 3 );
        QStandardItem* keepFinalItem   = m_model->item( row, 4 );
        if( !nameItem || !displayItem || !keepInitialItem || !keepFinalItem ) continue;

        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        ObjectInfoExtractor::ObjectInfo objectInfo = var.value<ObjectInfoExtractor::ObjectInfo>();

        // NOTE:display, keepInitial, keepFinalのチェックボックス状態をtmpに格納
        // FIXME:dispaly, keepInitial, keepFinalのtmpIsは不要かもしれません
        objectInfo.tmpIsDisplay     = ( displayItem->checkState()     == Qt::Checked );
        objectInfo.tmpIsKeepInitial = ( keepInitialItem->checkState() == Qt::Checked );
        objectInfo.tmpIsKeepFinal   = ( keepFinalItem->checkState()   == Qt::Checked );

        // NOTE:オブジェクト用パッチ
        QJsonObject patch;
        patch[QString::fromUtf8( Protocol::Key::UUID )] = QString::fromUtf8( objectInfo.uuid );

        // NOTE:以下isHogeの確定
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
        // NOTE:(extraOpacityFactor)は一時的に無効にしています。必要になったら下記のコードの削除とextraOpacityFactorに関係するコードのコメント文を元に戻せば使えるはずです。
        // if( objectInfo.extraOpacityFactor != objectInfo.tmpExtraOpacityFactor )
        // {
        //     objectInfo.extraOpacityFactor = objectInfo.tmpExtraOpacityFactor;
        //     patch[QString::fromUtf8( Protocol::Key::ExtraOpacityFactor )] = objectInfo.extraOpacityFactor;
        // }

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

        // NOTE:フォーカス対象のオブジェクトのみ、全体のmin/max座標計算に含める
        if( objectInfo.isFocus )
        {
            resultMinObjectCoords.x() = std::min( resultMinObjectCoords.x(), objectInfo.minObjectCoord.x() );
            resultMinObjectCoords.y() = std::min( resultMinObjectCoords.y(), objectInfo.minObjectCoord.y() );
            resultMinObjectCoords.z() = std::min( resultMinObjectCoords.z(), objectInfo.minObjectCoord.z() );

            resultMaxObjectCoords.x() = std::max( resultMaxObjectCoords.x(), objectInfo.maxObjectCoord.x() );
            resultMaxObjectCoords.y() = std::max( resultMaxObjectCoords.y(), objectInfo.maxObjectCoord.y() );
            resultMaxObjectCoords.z() = std::max( resultMaxObjectCoords.z(), objectInfo.maxObjectCoord.z() );
        }

        // NOTE:モデルへ反映
        nameItem->setData( QVariant::fromValue( objectInfo ), Qt::UserRole );

        // NOTE:uuid以外が入っていれば送信対象
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

        if( !patchArray.isEmpty() ) objectInfoParameter[QString::fromUtf8( Protocol::Key::Objects )] = patchArray;

        m_web_sockets->text()->sendTextMessage( QJsonDocument( objectInfoParameter ).toJson( QJsonDocument::Compact ) );
    }
}
