#include "Communication.h"
#include "ui_Communication.h"

Communication::Communication( kvs::qt::jaea::Screen* screen, QWebSocket* binarySocket, QWebSocket* textSocket, QWidget* parent )
    : QDockWidget(parent)
    , ui(new Ui::Communication)
    , m_screen( screen )
    , m_web_binary_socket( binarySocket )
    , m_web_text_socket( textSocket )
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

    connect( ui->connectPushButton, &QPushButton::clicked, this, &Communication::onConnectClicked );
    connect( ui->disconnectPushButton, &QPushButton::clicked, this, &Communication::onDisconnectClicked );
    connect( ui->transferOperatorPushButton, &QPushButton::clicked, this, &Communication::onTransferOperator );
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

void Communication::onTransferOperator()
{
    if( !isSocketsConnected() )
    {
        qDebug() << "Not connected";
        return;
    }

    if( !m_is_operator )
    {
        qDebug() << "You are not the operator";
        return;
    }

    bool isValid = false;
    int targetID = ui->IDlineEdit->text().toInt( &isValid );

    if( !isValid )
    {
        qDebug() << "Invalid ID";
        return;
    }

    if( m_user_id == targetID )
    {
        qDebug() << "You cannot transfer yourself";
    }


    m_web_text_socket->sendTextMessage( QJsonDocument( {
                                                      {"event", "transferoperator"},
                                                      {"target_id", targetID},
                                                      } ).toJson( QJsonDocument::Compact) );
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
    if( isSocketsConnected() )
    {
        emit updateServerState( true );
    }
}

void Communication::binaryWebsocketDisconnected()
{
    if( !isSocketsConnected() )
    {
        emit updateOperatorState( m_is_operator );
        emit updateServerState( false );
        m_user_id = -1;        
        m_is_operator = false;
        ui->IDLabelDisplay->clear();
        ui->isOperatorLabelDisplay->clear();
        ui->textBrowser->clear();
    }
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
    if( isSocketsConnected() )
    {
        emit updateServerState( true );
    }
}

void Communication::textWebsocketDisconnected()
{
    if( !isSocketsConnected() )
    {
        emit updateOperatorState( m_is_operator );
        emit updateServerState( false );
        m_user_id = -1;
        m_is_operator = false;
        ui->IDLabelDisplay->clear();
        ui->isOperatorLabelDisplay->clear();
        ui->textBrowser->clear();        
    }
}

void Communication::textWebsocketMessageReceived( const QString& receivedMessage )
{
    QJsonDocument doc = QJsonDocument::fromJson( receivedMessage.toUtf8() );
    if( !doc.isObject() ) return; // オブジェクトでない場合は無視

    QJsonObject obj = doc.object();

    if( obj.contains("event") )
    {
        if( obj["event"].toString() == "join" )
        {
            int userID = obj["userID"].toInt();    // 入出者
            QString userIDStr = QString::number( userID );
            ui->textBrowser->append( "--- User[" + userIDStr + "] Join" );
        }

        if( obj["event"].toString() == "left" )
        {
            int userID = obj["userID"].toInt();    // 入出者
            QString userIDStr = QString::number( userID );
            ui->textBrowser->append( "--- User[" + userIDStr + "] Left" );
        }

        if( obj["event"].toString() == "id" )
        {
            int userID = obj["userID"].toInt(); // 自分自身のユーザID
            QString userIDStr = QString::number( userID );
            m_user_id = userID;
            ui->IDLabelDisplay->setText( userIDStr );
            ui->textBrowser->append("--- Your User ID is [" + userIDStr + "]" );
        }

        if( obj["event"].toString() == "operator" )
        {
            bool isOperator = obj["isOperator"].toBool(); // 自分に操作権限があるか
            m_is_operator = isOperator;
            emit updateOperatorState( m_is_operator );
            ui->isOperatorLabelDisplay->setText( m_is_operator ? "true" : "false" );
            if( m_is_operator )
            {
                ui->textBrowser->append("--- You have operator privilege");
            }
            else
            {
                ui->textBrowser->append("--- You are not operator");
            }
        }

        if( obj["event"].toString() == "operatortransfer" )
        {
            int oldOperatorID = obj["oldOperatorID"].toInt();
            int newOperatorID = obj["newOperatorID"].toInt();
            ui->textBrowser->append("--- Operator change [" + QString::number( oldOperatorID ) + "] to [" + QString::number( newOperatorID ) + "]" );
            if( m_user_id == newOperatorID )
            {
                m_is_operator = true;
                ui->isOperatorLabelDisplay->setText( "true" );
            }
            else
            {
                m_is_operator = false;
                ui->isOperatorLabelDisplay->setText( "false" );
            }
        }

        if( obj["event"].toString() == "chat" )
        {
            int userID = obj["userID"].toInt();    // 受信したチャットの送信者
            QString userIDStr = QString::number( userID );
            QString text = obj["text"].toString();      // 受信したチャット内容
            ui->textBrowser->append( "User[" + userIDStr + "]: " + text );
        }

        if( obj["event"].toString() == "shareview" )
        {
            int userID = obj["userID"].toInt();    // 受信した視点共有の送信者
            QString userIDStr = QString::number( userID );
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
            QString label = "User[" + userIDStr + "] View";

            // QStandardItem 作成し、xform をデータとして格納
            QStandardItem* item = new QStandardItem( label );
            item->setData( QVariant::fromValue( recieveXform ), Qt::UserRole + 1 );
            // 編集不可にする
            item->setFlags( item->flags() & ~Qt::ItemIsEditable );
            m_share_view_list_model->appendRow( item );
        }

        if( obj["event"].toString() == "sharepoint" )
        {
            int userID = obj["userID"].toInt();    // 着目点共有を行った送信者
            QString userIDStr = QString::number( userID );
            double x = obj["x"].toDouble();
            double y = obj["y"].toDouble();
            double z = obj["z"].toDouble();
            double dx = obj["dx"].toDouble();
            double dy = obj["dy"].toDouble();
            double dz = obj["dz"].toDouble();

            kvs::Real32 CoordArray[ 1 * 3 ] =
                {
                    kvs::Real32( x ),
                    kvs::Real32( y ),
                    kvs::Real32( z ),
                };

            kvs::Real32 DirectionArray[ 1 * 3 ] =
                {
                    kvs::Real32( dx ),
                    kvs::Real32( dy ),
                    kvs::Real32( dz ),
                };

#ifdef OPENXR_SCREEN
            // シーン全体のサイズを基準にした base_size
            kvs::Vec3 min_coord = m_screen->scene()->objectManager()->minObjectCoord();
            kvs::Vec3 max_coord = m_screen->scene()->objectManager()->maxObjectCoord();
            kvs::Vec3 diag = max_coord - min_coord;
            float scene_size = diag.length();
            float base_size = scene_size * 0.005f; // シーン全体に対する割合

            // 現在のスケーリング値を取得
            kvs::Xform xform = m_screen->scene()->objectManager()->xform();
            float scalingFactor = xform.scaling().x(); // (x,y,z が同じなら x でOK)

            // スケーリングに反比例させることで拡大縮小に依存しないサイズにする
            kvs::Real32 SizeArray[1] = { base_size / scalingFactor };
#else
            kvs::Xform currentObjectManagerXform = m_screen->scene()->objectManager()->xform(); // 現在のオブジェクトマネージャーのTranslation, Scaling, Rotationを取得する。
            float scalingFactor = 1 / ( currentObjectManagerXform.inverse() * m_screen->scene()->object( m_screen->scene()->numberOfObjects() - 1 )->xform() ).scaling().x();
            kvs::Real32 SizeArray[ 1 ] = {
                0.5f * scalingFactor,
            };
#endif

            kvs::UInt8 ColorArray[ 1 * 3 ] =
                {
                    0, 255, 0,
                };

            kvs::ValueArray<kvs::Real32> coords( CoordArray, 1 * 3 );
            kvs::ValueArray<kvs::Real32> direction( DirectionArray, 1 * 3 );
            kvs::ValueArray<kvs::Real32> size( SizeArray, 1 );
            kvs::ValueArray<kvs::UInt8> colors( ColorArray, 1 * 3 );

            if( m_screen->scene()->object( userID + "_SharedGlyph" ) == nullptr )
            {
                kvs::PolygonObject* sharedPolygon = createArrowGlyph( coords, direction, size, colors );
                sharedPolygon->setName( userID + "_SharedGlyph" );
                sharedPolygon->setXform( m_screen->scene()->objectManager()->xform() );
                sharedPolygon->setMinMaxObjectCoords( m_screen->scene()->object( m_server_point_object_ids.first )->minObjectCoord(), m_screen->scene()->object( m_server_point_object_ids.first )->maxObjectCoord() );
                sharedPolygon->setMinMaxExternalCoords( m_screen->scene()->object( m_server_point_object_ids.first )->minObjectCoord(), m_screen->scene()->object( m_server_point_object_ids.first )->maxObjectCoord() );
                kvs::StochasticPolygonRenderer* renderer = new kvs::StochasticPolygonRenderer();
                m_screen->registerObject( sharedPolygon, renderer );
                m_screen->update();
            }
            else
            {
                kvs::PolygonObject* sharedPolygon = createArrowGlyph( coords, direction, size, colors );
                sharedPolygon->setName( userID + "_SharedGlyph" );
                sharedPolygon->setXform( m_screen->scene()->objectManager()->xform() );
                sharedPolygon->setMinMaxObjectCoords( m_screen->scene()->object( m_server_point_object_ids.first )->minObjectCoord(), m_screen->scene()->object( m_server_point_object_ids.first )->maxObjectCoord() );
                sharedPolygon->setMinMaxExternalCoords( m_screen->scene()->object( m_server_point_object_ids.first )->minObjectCoord(), m_screen->scene()->object( m_server_point_object_ids.first )->maxObjectCoord() );
                m_screen->scene()->replaceObject( userID + "_SharedGlyph", sharedPolygon );
                m_screen->update();
            }
        }
    }
}

// 着目点グリフ生成用メソッド
kvs::PolygonObject* Communication::createArrowGlyph(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::Real32>& directions,
    const kvs::ValueArray<kvs::Real32>& sizes,
    const kvs::ValueArray<kvs::UInt8>& colors )
{
    const size_t npoint = coords.size() / 3;
    const int slices = 20;

    std::vector<kvs::Vec3> all_vertices;
    std::vector<kvs::Vec3> all_normals;
    std::vector<kvs::UInt32> all_indices;
    std::vector<kvs::UInt8> all_colors;

    for( size_t i = 0, index = 0; i < npoint; i++, index += 3 )
    {
        kvs::Vec3 tip_position( coords.data() + index );   // 先端位置
        kvs::Vec3 direction( directions.data() + index );
        kvs::Real32 size = sizes[i];
        kvs::RGBColor color( colors.data() + index );

        if( direction.length() < 1e-6 )
        {
            std::cerr << "Error: Invalid direction vector." << std::endl;
            continue;
        }
        direction = direction.normalized();

        // --- 矢印寸法 ---
        const float cylinder_height = 0.7f * size;
        const float cylinder_radius = 0.07f * size;
        const float cone_height = 0.3f * size;
        const float cone_radius = 0.15f * size;
        const float arrow_height = cylinder_height + cone_height;

        std::vector<kvs::Vec3> vertices;
        std::vector<kvs::Vec3> normals;
        std::vector<kvs::UInt32> indices;

        // --- 円柱の構築 ---
        for( int s = 0; s < slices; ++s )
        {
            float angle = 2.0f * M_PI * s / slices;
            float x = cylinder_radius * std::cos( angle );
            float y = cylinder_radius * std::sin( angle );

            vertices.emplace_back( x, y, 0.0f );              // 底面
            normals.emplace_back( x, y, 0.0f );

            vertices.emplace_back( x, y, cylinder_height );   // 上面
            normals.emplace_back( x, y, 0.0f );
        }

        for( int s = 0; s < slices; ++s )
        {
            int next = ( s + 1 ) % slices;
            indices.push_back( s * 2 ); indices.push_back( next * 2 ); indices.push_back( s * 2 +1 );
            indices.push_back( s * 2 +1 ); indices.push_back( next * 2 ); indices.push_back( next * 2 +1 );
        }

        // --- 円錐の構築 ---
        std::vector<kvs::Vec3> base_vertices;
        for( int s = 0; s < slices; ++s )
        {
            float angle = 2.0f * M_PI * s / slices;
            base_vertices.emplace_back( cone_radius*std::cos( angle ), cone_radius*std::sin( angle ), cylinder_height );
        }

        for( int s = 0; s < slices; ++s )
        {
            int next = ( s + 1 ) % slices;
            kvs::Vec3 apex( 0, 0, cylinder_height + cone_height );
            kvs::Vec3 v1 = base_vertices[s];
            kvs::Vec3 v2 = base_vertices[next];

            vertices.push_back( v1 ); vertices.push_back( v2 ); vertices.push_back( apex );
            kvs::Vec3 normal = ( v2 - v1 ).cross( apex - v1 ).normalized();
            normals.push_back( normal ); normals.push_back( normal ); normals.push_back( normal );

            int base_index = vertices.size() - 3;
            indices.push_back( base_index ); indices.push_back( base_index + 1 ); indices.push_back( base_index + 2 );
        }

        // --- 先端が tip_position になるように Zを下方向にシフト ---
        for( auto& v : vertices ) v.z() -= arrow_height;

        // --- 回転・位置調整 ---
        kvs::Vec3 default_direction( 0, 0, 1 );
        kvs::Vec3 axis = default_direction.cross( direction );
        float angle = std::acos( default_direction.dot( direction ) );
        kvs::Mat3 rotation;
        if( axis.length() > 1e-6 )
        {

            rotation = kvs::Mat3::Rotation( axis.normalized(), angle*180.0/M_PI );
        }
        else
        {
            rotation = kvs::Mat3::Identity();
        }

        for( auto& v : vertices ) v = rotation * v + tip_position;
        for( auto& n : normals ) n = rotation * n;

        size_t offset = all_vertices.size();
        all_vertices.insert( all_vertices.end(), vertices.begin(), vertices.end() );
        all_normals.insert( all_normals.end(), normals.begin(), normals.end() );
        for( auto idx : indices ) all_indices.push_back( idx + offset );

        for( size_t c =0; c< vertices.size(); ++c )
        {
            all_colors.push_back( color.r() );
            all_colors.push_back( color.g() );
            all_colors.push_back( color.b() );
        }
    }

    kvs::PolygonObject* polygon = new kvs::PolygonObject();
    polygon->setCoords( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_vertices.data(), all_vertices.size()*3 ) );
    polygon->setConnections( kvs::ValueArray<kvs::UInt32>( all_indices.data(), all_indices.size() ) );
    polygon->setColors( kvs::ValueArray<kvs::UInt8>( all_colors.data(), all_colors.size() ) );
    polygon->setNormals( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_normals.data(), all_normals.size()*3 ) );
    polygon->setOpacity( 255 );
    polygon->setPolygonType(kvs::PolygonObject::Triangle);
    polygon->setColorType(kvs::PolygonObject::PolygonColor);
    polygon->setNormalType(kvs::PolygonObject::VertexNormal);

    return polygon;
}

void Communication::onVRSharePoint( kvs::Real32 CoordArray[ 2 * 3 ], kvs::Real32 DirectionArray[ 3 ] )
{
    if( !isSocketsConnected() )
    {
        qDebug() << "Not connected";
        return;
    }

    double x = CoordArray[3];
    double y = CoordArray[4];
    double z = CoordArray[5];

    double dx = DirectionArray[0];
    double dy = DirectionArray[1];
    double dz = DirectionArray[2];

    QJsonObject positionObject;
    positionObject["x"] = x;
    positionObject["y"] = y;
    positionObject["z"] = z;
    positionObject["dx"] = dx;
    positionObject["dy"] = dy;
    positionObject["dz"] = dz;


    m_web_text_socket->sendTextMessage( QJsonDocument( {
                                                         {"event","sharepoint"},
                                                         {"x",CoordArray[3]},
                                                         {"y",CoordArray[4]},
                                                         {"z",CoordArray[5]},
                                                         {"dx",DirectionArray[0]},
                                                         {"dy",DirectionArray[1]},
                                                         {"dz",DirectionArray[2]}
                                                     }
                                                     ).toJson( QJsonDocument::Compact ) );
}

void Communication::loadParameter( const QString& filePath )
{
    // TODO KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void Communication::saveParameter( const QString& filePath )
{
    // TODO KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}
