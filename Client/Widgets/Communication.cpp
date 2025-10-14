#include "Communication.h"
#include "ui_Communication.h"

Communication::Communication( kvs::qt::jaea::Screen* screen, QWidget* parent )
    : QDockWidget(parent)
    , ui(new Ui::Communication)
    , m_screen( screen )
{
    initialize();
}

Communication::~Communication()
{
    delete ui;
}

void Communication::initialize()
{
    ui->setupUi(this);

    m_web_binary_socket = new QWebSocket();
    m_web_text_socket   = new QWebSocket();
    m_web_binary_socket->setParent( this );
    m_web_text_socket->setParent( this );

    connect( ui->connectPushButton, &QPushButton::clicked, this, &Communication::onConnectClicked );
    connect( ui->disconnectPushButton, &QPushButton::clicked, this, &Communication::onDisconnectClicked );
    connect( ui->chatSendPushButton, &QPushButton::clicked, this, &Communication::onChatClicked );
    connect( ui->shareViewPushButton , &QPushButton::clicked, this, &Communication::onShareView );

    connect( ui->debugPushButton, &QPushButton::clicked, this, [this]() {
        if( !isSocketsConnected() )
        {
            return;
        }
        m_web_text_socket->sendTextMessage( QJsonDocument( QJsonObject{ {"event", "debug"} } ).toJson( QJsonDocument::Compact ) );
    });

    connect( ui->editColorMapPushButton, &QPushButton::clicked, this, [this]()
            {
                ColorMapEditor colorMapEditor;
                colorMapEditor.adjustSize();
                colorMapEditor.setDefaultColorMap( ui->colorMap->getColors() );

                if( colorMapEditor.exec() == QDialog::Accepted )
                {
                    QVector<QColor> qcolors = colorMapEditor.getColorMap();
                    ui->colorMap->setColors( qcolors );
                }
            } );

    connect( ui->editOpacityMapPushButton, &QPushButton::clicked, this, [this]()
            {
                OpacityMapEditor OpacityMapEditor;
                OpacityMapEditor.adjustSize();
                OpacityMapEditor.setDefaultOpacityMap( ui->opacityMap->getOpacities() );

                if( OpacityMapEditor.exec() == QDialog::Accepted )
                {
                    QVector<float> opacities = OpacityMapEditor.getOpacityMap();
                    ui->opacityMap->setOpacities( opacities );
                }
            } );

    connect( ui->sendTransferFunctionPushButton, &QPushButton::clicked, this, [this]()
            {
                if( !isSocketsConnected() )
                {
                    return;
                }

                QJsonArray colorArray;
                for( const QColor& color : ui->colorMap->getColors() )
                {
                    QJsonArray rgb;
                    rgb.append( color.red() );
                    rgb.append( color.green() );
                    rgb.append( color.blue() );
                    colorArray.append( rgb );
                }

                QJsonArray opacityArray;
                for( float opacity : ui->opacityMap->getOpacities() )
                {
                    opacityArray.append( opacity );
                }

                m_web_text_socket->sendTextMessage(
                    QJsonDocument(
                        QJsonObject{
                            { "event", "transferfunction" },
                            { "colorMap", colorArray },
                            { "opacityMap", opacityArray }
                        }
                        ).toJson( QJsonDocument::Compact )
                    );
            } );

    connect( ui->generatePushButton, &QPushButton::clicked, this, [this]()
            {
                if( !isSocketsConnected() )
                {
                    return;
                }
                m_web_text_socket->sendTextMessage( QJsonDocument( QJsonObject{ {"event", "generate"} } ).toJson( QJsonDocument::Compact ) );
            } );

    connect( m_web_binary_socket, &QWebSocket::connected    , this, &Communication::binaryWebsocketConnected );     // 接続成功(バイナリ)
    connect( m_web_binary_socket, &QWebSocket::disconnected , this, &Communication::binaryWebsocketDisconnected );  // 接続切断(バイナリ)
    connect( m_web_binary_socket, &QWebSocket::binaryMessageReceived , this, &Communication::binaryWebsocketMessageReceived );  // メッセージ受信(バイナリ)

    connect( m_web_text_socket, &QWebSocket::connected      , this, &Communication::textWebsocketConnected );       // 接続成功(テキスト)
    connect( m_web_text_socket, &QWebSocket::disconnected   , this, &Communication::textWebsocketDisconnected );    // 接続切断(テキスト)
    connect( m_web_text_socket, &QWebSocket::textMessageReceived , this, &Communication::textWebsocketMessageReceived );  // メッセージ受信(テキスト)
}

/**
 * @brief バイナリとテキストの両ソケットが接続済みか確認する
 * @return true  両方のソケットが接続中または接続済み
 * @return false どちらかが未接続
 */
bool Communication::isSocketsConnected() const
{
    auto isConnectedOrConnecting = [](QWebSocket* socket)
    {
        return socket && ( socket->state() == QAbstractSocket::ConnectedState || socket->state() == QAbstractSocket::ConnectingState );
    };
    return isConnectedOrConnecting( m_web_binary_socket ) && isConnectedOrConnecting( m_web_text_socket );
}

void Communication::registerObject( kvs::PointObject* pointObject )
{
    kvs::glsl::ParticleBasedRenderer* renderer = new kvs::glsl::ParticleBasedRenderer();
    renderer->enableShuffle();

    kvs::Xform m_initial_camera_xfom
        (
            kvs::Mat4(
                1, 0, 0, 0 ,
                0, 1, 0, 0 ,
                0, 0, 1, 12,
                0, 0, 0, 1
                )
            );

    kvs::Vec3 translationOffset = m_screen->scene()->camera()->xform().translation() - m_initial_camera_xfom.translation();
    renderer->setTranslationOffset( translationOffset );
    renderer->setObjectDepth( m_screen->scene()->objectManager()->xform().scaling().z() / m_screen->scene()->camera()->xform().scaling().z() );

    m_server_point_object_ids = m_screen->scene()->registerObject( pointObject, renderer );
    m_screen->update();
}

void Communication::replaceObject( kvs::PointObject* pointObject )
{
    m_screen->scene()->replaceObject( m_server_point_object_ids.first, pointObject );
    m_screen->update();
}

void Communication::onConnectClicked()
{
    if( isSocketsConnected() )
    {
        qDebug() << "Already connected";
        return;
    }

    m_uuid = QUuid::createUuid().toString(); // ユーザUUID
    const QString address = "ws://127.0.0.1:60000"; // AFTER_WEBSOCKET
    QString uuidStr = m_uuid;
    uuidStr.remove('{').remove('}'); // { } を除去
    const QString binaryAddress = address + "/binary?uuid=" + uuidStr;
    const QString textAddress   = address + "/text?uuid=" + uuidStr;

    qDebug() << "Connecting to" << address;
    if( m_web_binary_socket ) m_web_binary_socket->open( QUrl( binaryAddress ) );
    if( m_web_text_socket ) m_web_text_socket->open( QUrl( textAddress ) );
}

void Communication::onDisconnectClicked()
{
    if( !isSocketsConnected() )
    {
        qDebug() << "Not connected";
        return;
    }

    m_uuid.clear();
    qDebug() << "Disconnecting...";
    if( m_web_binary_socket ) m_web_binary_socket->close();
    if( m_web_text_socket ) m_web_text_socket->close();
}

void Communication::onChatClicked()
{
    if( !isSocketsConnected() )
    {
        qDebug() << "Not connected";
        return;
    }

    QString text = ui->chatLineEdit->text().trimmed();
    if( text.isEmpty() ) return; // 何も入力されていない場合は何もしない

    m_web_text_socket->sendTextMessage( QJsonDocument( {
                                                      {"event", "chat"},
                                                      {"text", text},
                                                      } ).toJson( QJsonDocument::Compact) );
    ui->chatLineEdit->clear();
}

void Communication::onShareView()
{
    if( !isSocketsConnected() )
    {
        qDebug() << "Not connected";
        return;
    }

    kvs::Xform currentXform = m_screen->scene()->objectManager()->xform();
    const kvs::Matrix44f matrix = currentXform.toMatrix();

    std::cout << matrix << std::endl;

    QJsonArray matrix_array;
    for( int row = 0; row < 4; ++row )
    {
        QJsonArray row_array;
        for( int col = 0; col < 4; ++col )
        {
            row_array.append( matrix[row][col] );
        }
        matrix_array.append( row_array );
    }

    m_web_text_socket->sendTextMessage( QJsonDocument( {
                                                      {"event", "shareview"},
                                                      {"matrix", matrix_array},
                                                      } ).toJson( QJsonDocument::Compact) );
}

void Communication::onItemDoubleClicked(const QModelIndex& index)
{
    if( !index.isValid() ) return;

    QVariant data = index.data( Qt::UserRole + 1 );
    if( data.canConvert<kvs::Xform>() )
    {
        m_screen->reset();
        kvs::Xform xform = data.value<kvs::Xform>();
        m_screen->scene()->objectManager()->rotate( xform.rotation() );
        m_screen->scene()->objectManager()->translate( xform.translation() );
        m_screen->scene()->objectManager()->scale( xform.scaling() );
    }
    m_screen->update();
}

void Communication::binaryWebsocketConnected()
{
}

void Communication::binaryWebsocketDisconnected()
{
}

void Communication::binaryWebsocketMessageReceived( const QByteArray& binary )
{
    const char* data_ptr = binary.constData();
    size_t offset = 0;

    // 頂点数を読み出す
    size_t numberOfVertices = 0;
    std::memcpy( &numberOfVertices, data_ptr + offset, sizeof( size_t ) );
    offset += sizeof( size_t );

    // 座標（float3 * N）
    kvs::ValueArray<kvs::Real32> coords( numberOfVertices * 3 );
    std::memcpy( coords.data(), data_ptr + offset, sizeof( kvs::Real32 ) * 3 * numberOfVertices );
    offset += sizeof( kvs::Real32 ) * 3 * numberOfVertices;

    // 色（uchar3 * N）
    kvs::ValueArray<kvs::UInt8> colors( numberOfVertices * 3 );
    std::memcpy( colors.data(), data_ptr + offset, sizeof( kvs::UInt8 ) * 3 * numberOfVertices );
    offset += sizeof( kvs::UInt8 ) * 3 * numberOfVertices;

    // 法線（float3 * N）
    kvs::ValueArray<kvs::Real32> normals( numberOfVertices * 3 );
    std::memcpy( normals.data(), data_ptr + offset, sizeof( kvs::Real32 ) * 3 * numberOfVertices );
    offset += sizeof( kvs::Real32 ) * 3 * numberOfVertices;

    // minObjectCoords（float3）
    kvs::Vec3 minObjectCoords;
    std::memcpy( minObjectCoords.data(), data_ptr + offset, sizeof( kvs::Real32 ) * 3 );
    offset += sizeof( kvs::Real32 ) * 3;

    // maxObjectCoords（float3）
    kvs::Vec3 maxObjectCoords;
    std::memcpy( maxObjectCoords.data(), data_ptr + offset, sizeof( kvs::Real32 ) * 3 );
    offset += sizeof( kvs::Real32 ) * 3;

    // kvs::PointObject の生成
    auto* object = new kvs::PointObject();
    object->setCoords( coords );
    object->setColors( colors );
    object->setNormals( normals );
    object->setMinMaxObjectCoords( minObjectCoords, maxObjectCoords );
    object->setMinMaxExternalCoords( minObjectCoords, maxObjectCoords );

    emit updateFocus( minObjectCoords, maxObjectCoords ); // AFTER_WEBSOCKET
    emit updatePointsTranslation(); // AFTER_WEBSOCKET

    if( m_server_point_object_ids == QPair<int,int>(-1, -1) )
    {
        registerObject(object);
    }
    else
    {
        replaceObject(object);
    }
}

void Communication::textWebsocketConnected()
{
}

void Communication::textWebsocketDisconnected()
{
}

void Communication::textWebsocketMessageReceived( const QString& receivedMessage )
{
    QJsonDocument doc = QJsonDocument::fromJson( receivedMessage.toUtf8() );
    if( !doc.isObject() ) return; // オブジェクトでない場合は無視

    QJsonObject obj = doc.object();

    if( obj.contains("event") && obj["event"].toString() == "join" )
    {
        int userID = obj["userID"].toInt();    // 入出者
        ui->textBrowser->append( "--- User[" + QString::number( userID ) + "] Join ---" );
    }

    if( obj.contains("event") && obj["event"].toString() == "left" )
    {
        int userID = obj["userID"].toInt();    // 退出者
        ui->textBrowser->append( "--- User[" + QString::number( userID ) + "] Left ---" );
    }

    if( obj.contains("event") && obj["event"].toString() == "chat" )
    {
        int userID = obj["userID"].toInt();    // 受信したチャットの送信者
        QString text = obj["text"].toString();      // 受信したチャット内容
        ui->textBrowser->append( "User[" + QString::number( userID ) + "]: " + text );
    }

    if( obj.contains("event") && obj["event"].toString() == "shareview" )
    {
        int userID = obj["userID"].toInt();    // 受信した視点共有の送信者
        QJsonArray matrixArray = obj["matrix"].toArray();
        kvs::Matrix44f mat;
        for( int row = 0; row < 4; ++row )
        {
            QJsonArray row_array = matrixArray.at( row ).toArray();
            for( int col = 0; col < 4; ++col )
            {
                mat[row][col] = static_cast<float>( row_array.at( col ).toDouble() );
            }
        }

        // kvs::Xform に変換
        kvs::Xform recieveXform( mat );
        if( !m_share_view_list_model )
        {
            m_share_view_list_model = new QStandardItemModel( this );
            ui->shareListView->setModel( m_share_view_list_model );
            connect( ui->shareListView, &QListView::doubleClicked, this, &Communication::onItemDoubleClicked );
        }
        // 表示用ラベル作成
        QString label = "User[" + QString::number( userID ) + "] View";

        // QStandardItem 作成し、xform をデータとして格納
        QStandardItem* item = new QStandardItem( label );
        item->setData( QVariant::fromValue( recieveXform ), Qt::UserRole + 1 );
        // 編集不可にする
        item->setFlags( item->flags() & ~Qt::ItemIsEditable );
        m_share_view_list_model->appendRow( item );
    }
}
