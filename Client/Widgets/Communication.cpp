#include "Communication.h"
#include "ui_Communication.h"

Communication::Communication( kvs::qt::jaea::Screen* screen, WebSocketPair* websockets, Viz::Mode* vizMode, QWidget* parent )
    : QDockWidget(parent)
    , ui(new Ui::Communication)
    , m_screen( screen )
    , m_web_sockets( websockets )
    , m_viz_mode( vizMode )
{
    ui->setupUi( this );

    ui->uniformRadioButton->setChecked( true ); // NOTE:デフォルトはUniformサンプリング
    ui->disconnectPushButton->setEnabled( false ); // NOTE:デフォルトでは無効
    ui->addressLineEdit->setText( "ws://127.0.0.1:60000" );

    connect( ui->localVizRadioButton               ,&QRadioButton::clicked            , this, &Communication::onModeClicked );
    connect( ui->remoteVizClientServerRadioButton  ,&QRadioButton::clicked            , this, &Communication::onModeClicked );
    connect( ui->remoteVizInsituRadioButton        ,&QRadioButton::clicked            , this, &Communication::onModeClicked );

    connect( ui->volumeDataFilePathPushButton      ,&QPushButton::clicked             , this, &Communication::onVolumeDataFilePathClicked );
    connect( ui->transferFunctionFilePathPushButton,&QPushButton::clicked             , this, &Communication::onTransferFunctionFilePathClicked );

    connect( ui->connectPushButton                 ,&QPushButton::clicked             , this, &Communication::onConnectClicked );
    connect( ui->disconnectPushButton              ,&QPushButton::clicked             , this, &Communication::onDisconnectClicked );
    connect( ui->transferOperatorApplyPushButton   ,&QPushButton::clicked             , this, &Communication::onTransferOperator );
    connect( ui->chatLineEdit                      ,&QLineEdit::returnPressed         , this, &Communication::onChatClicked );
    connect( ui->shareViewPushButton               ,&QPushButton::clicked             , this, &Communication::onShareView );

    connect( m_web_sockets->binary()               ,&QWebSocket::connected            , this, &Communication::onBinaryWebsocketConnected );         // 接続成功(バイナリ)
    connect( m_web_sockets->binary()               ,&QWebSocket::disconnected         , this, &Communication::onBinaryWebsocketDisconnected );      // 接続切断(バイナリ)
    connect( m_web_sockets->binary()               ,&QWebSocket::binaryMessageReceived, this, &Communication::onBinaryWebsocketMessageReceived );   // メッセージ受信(バイナリ)
    connect( m_web_sockets->text()                 ,&QWebSocket::connected            , this, &Communication::onTextWebsocketConnected );           // 接続成功(テキスト)
    connect( m_web_sockets->text()                 ,&QWebSocket::disconnected         , this, &Communication::onTextWebsocketDisconnected );        // 接続切断(テキスト)
    connect( m_web_sockets->text()                 ,&QWebSocket::textMessageReceived  , this, &Communication::onTextWebsocketMessageReceived );     // メッセージ受信(テキスト)
}

Communication::~Communication()
{
    delete ui;
}

void Communication::onVRSharePoint( kvs::Real32 CoordArray[ 2 * 3 ], kvs::Real32 DirectionArray[ 3 ] )
{
    if( !m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( "Not connected." );
        return;
    }

    double x = CoordArray[3];
    double y = CoordArray[4];
    double z = CoordArray[5];

    double dx = DirectionArray[0];
    double dy = DirectionArray[1];
    double dz = DirectionArray[2];

    m_web_sockets->text()->sendTextMessage( QJsonDocument( {
                                                             { QString::fromUtf8( Protocol::Key::Event ), QString::fromUtf8( Protocol::Events::SharePoint ) },
                                                             { QString::fromUtf8( Protocol::Key::X ), CoordArray[3] },
                                                             { QString::fromUtf8( Protocol::Key::Y ), CoordArray[4] },
                                                             { QString::fromUtf8( Protocol::Key::Z ), CoordArray[5] },
                                                             { QString::fromUtf8( Protocol::Key::Dx ), DirectionArray[0] },
                                                             { QString::fromUtf8( Protocol::Key::Dy ), DirectionArray[1] },
                                                             { QString::fromUtf8( Protocol::Key::Dz ), DirectionArray[2] }
                                                         }
                                                         ).toJson( QJsonDocument::Compact ) );
}

void Communication::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void Communication::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void Communication::websocketConnected()
{
    if( m_web_sockets->isConnected() )
    {
        emit updateServerState( true );

        ui->localVizRadioButton                 ->setEnabled( false );
        ui->remoteVizClientServerRadioButton    ->setEnabled( false );
        ui->remoteVizInsituRadioButton          ->setEnabled( false );

        ui->uniformRadioButton                  ->setEnabled( false );
        ui->metropolisRadioButton               ->setEnabled( false );
        ui->rejectionRadioButton                ->setEnabled( false );
        ui->volumeDataFilePathLineEdit          ->setEnabled( false );
        ui->volumeDataFilePathPushButton        ->setEnabled( false );
        ui->transferFunctionFilePathLineEdit    ->setEnabled( false );
        ui->transferFunctionFilePathPushButton  ->setEnabled( false );

        ui->addressLineEdit                     ->setEnabled( false );
        ui->connectPushButton                   ->setEnabled( false );
        ui->disconnectPushButton                ->setEnabled( true );
        updateVizMode();

        QString pointObjectUUID = QUuid::createUuid().toString( QUuid::WithoutBraces );
        QString glyphObjectUUID = QUuid::createUuid().toString( QUuid::WithoutBraces );

        ObjectInfoExtractor::Format pointObjectFormat;
        if( ui->localVizRadioButton->isChecked() || ui->remoteVizClientServerRadioButton->isChecked() )
            pointObjectFormat = ObjectInfoExtractor::Format::ClientServerPointObject;
        else
            pointObjectFormat = ObjectInfoExtractor::Format::InsituServerPointObject;

        ObjectInfoExtractor::Format glyphObjectFormat = ObjectInfoExtractor::Format::ServerGlyphObject;

        QJsonArray uuidArray;
        uuidArray.append( pointObjectUUID );
        uuidArray.append( glyphObjectUUID );

        QJsonArray formatArray;
        formatArray.append( pointObjectFormat );
        formatArray.append( glyphObjectFormat );

        m_web_sockets->text()->sendTextMessage(
            QJsonDocument( {
                           { QString::fromUtf8( Protocol::Key::Event)                    , QString::fromUtf8(Protocol::Events::Initialize) },
                           { QString::fromUtf8( Protocol::Key::VolumeDataFilePath )      , ui->volumeDataFilePathLineEdit->text() },
                           { QString::fromUtf8( Protocol::Key::TransferFunctionFilePath ), ui->transferFunctionFilePathLineEdit->text() },
                           { QString::fromUtf8( Protocol::Key::UUID )                    , uuidArray },
                           { QString::fromUtf8( Protocol::Key::Format )                  , formatArray },
                           } ).toJson( QJsonDocument::Compact )
            );
    }
}

void Communication::websocketDisconnected()
{
    if( !m_web_sockets->isConnected() )
    {
        emit updateOperatorState( m_is_operator ); // FIXME:ここで本当にあってる??下で値書き換えてるけど??
        emit updateServerState( false );
        m_user_id = -1;
        m_is_operator = false;

        ui->localVizRadioButton                 ->setEnabled( true );
        ui->remoteVizClientServerRadioButton    ->setEnabled( true );
        ui->remoteVizInsituRadioButton          ->setEnabled( true );

        ui->uniformRadioButton                  ->setEnabled( true );
        ui->metropolisRadioButton               ->setEnabled( true );
        ui->rejectionRadioButton                ->setEnabled( true );
        ui->volumeDataFilePathLineEdit          ->setEnabled( true );
        ui->volumeDataFilePathPushButton        ->setEnabled( true );
        ui->transferFunctionFilePathLineEdit    ->setEnabled( true );
        ui->transferFunctionFilePathPushButton  ->setEnabled( true );

        ui->addressLineEdit                     ->setEnabled( true );
        ui->connectPushButton                   ->setEnabled( true );
        ui->disconnectPushButton                ->setEnabled( false );

        ui->IDLineEdit->clear();
        ui->isOperatorLineEdit->clear();
        ui->textBrowser->clear();
        updateVizMode();
    }
}

void Communication::updateVizMode()
{
    *m_viz_mode = Viz::Mode::Local;

    if( !m_web_sockets->isConnected() ) return;

    if( ui->localVizRadioButton->isChecked() )                   *m_viz_mode = Viz::Mode::LocalClientAndServer;
    else if( ui->remoteVizClientServerRadioButton->isChecked() ) *m_viz_mode = Viz::Mode::RemoteClientAndServer;
    else if( ui->remoteVizInsituRadioButton->isChecked() )       *m_viz_mode = Viz::Mode::RemoteInSitu;
}

void Communication::Join( const QJsonObject& dataArray )
{
    int userID = dataArray[QString::fromUtf8( Protocol::Key::UserID )].toInt();    // 入出者
    QString userIDStr = QString::number( userID );
    ui->textBrowser->append( "--- User[" + userIDStr + "] Join" );
}

void Communication::Left( const QJsonObject& dataArray )
{
    int userID = dataArray[QString::fromUtf8( Protocol::Key::UserID )].toInt();    // 退出者
    QString userIDStr = QString::number( userID );
    ui->textBrowser->append( "--- User[" + userIDStr + "] Left" );
}

void Communication::ID( const QJsonObject& dataArray )
{
    int userID = dataArray[QString::fromUtf8( Protocol::Key::UserID )].toInt(); // 自分自身のユーザID
    QString userIDStr = QString::number( userID );
    m_user_id = userID;
    ui->IDLineEdit->setText( userIDStr );
    ui->textBrowser->append("--- Your User ID is [" + userIDStr + "]" );
}

void Communication::Operator( const QJsonObject& dataArray )
{
    bool isOperator = dataArray[QString::fromUtf8( Protocol::Key::IsOperator )].toBool(); // 自分に操作権限があるか
    m_is_operator = isOperator;
    emit updateOperatorState( m_is_operator );
    ui->isOperatorLineEdit->setText( m_is_operator ? "true" : "false" );
    if( m_is_operator )
    {
        ui->textBrowser->append("--- You have operator privilege");
    }
    else
    {
        ui->textBrowser->append("--- You are not operator");
    }
}

void Communication::transferOperator( const QJsonObject& dataArray )
{
    int oldOperatorID = dataArray[QString::fromUtf8( Protocol::Key::OldOperatorID )].toInt();
    int newOperatorID = dataArray[QString::fromUtf8( Protocol::Key::NewOperatorID )].toInt();
    ui->textBrowser->append("--- Operator change [" + QString::number( oldOperatorID ) + "] to [" + QString::number( newOperatorID ) + "]" );
    if( m_user_id == newOperatorID )
    {
        m_is_operator = true;
        ui->isOperatorLineEdit->setText( "true" );
    }
    else
    {
        m_is_operator = false;
        ui->isOperatorLineEdit->setText( "false" );
    }
    emit updateOperatorState( m_is_operator );
}

void Communication::chat( const QJsonObject& dataArray )
{
    int userID = dataArray[QString::fromUtf8( Protocol::Key::UserID )].toInt();    // 受信したチャットの送信者
    QString userIDStr = QString::number( userID );
    QString text = dataArray[QString::fromUtf8( Protocol::Key::Text )].toString();      // 受信したチャット内容
    ui->textBrowser->append( "User[" + userIDStr + "]: " + text );
}

void Communication::shareView( const QJsonObject& dataArray )
{
    // int userID = dataArray[QString::fromUtf8( Protocol::Key::UserID )].toInt();    // 着目点共有を行った送信者
    // QString userIDStr = QString::number( userID );
    // QJsonArray matrixArray = dataArray[QString::fromUtf8( Protocol::Key::Matrix )].toArray();
    // kvs::Matrix44f mat;
    // for( int row = 0; row < 4; ++row )
    // {
    //     QJsonArray row_array = matrixArray.at( row ).toArray();
    //     for( int col = 0; col < 4; ++col )
    //     {
    //         mat[row][col] = static_cast<float>( row_array.at( col ).toDouble() );
    //     }
    // }

    // // kvs::Xform に変換
    // kvs::Xform recieveXform( mat );
    // if( !m_share_view_list_model )
    // {
    //     m_share_view_list_model = new QStandardItemModel( this );
    //     ui->shareListView->setModel( m_share_view_list_model );
    //     connect( ui->shareListView, &QListView::doubleClicked, this, &Communication::onItemDoubleClicked );
    // }
    // // 表示用ラベル作成
    // QString label = "User[" + userIDStr + "] View";

    // // QStandardItem 作成し、xform をデータとして格納
    // QStandardItem* item = new QStandardItem( label );
    // item->setData( QVariant::fromValue( recieveXform ), Qt::UserRole + 1 );
    // // 編集不可にする
    // item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    // m_share_view_list_model->appendRow( item );
}

void Communication::sharePoint( const QJsonObject& dataArray )
{
//     int userID = dataArray[QString::fromUtf8( Protocol::Key::UserID )].toInt();    // 着目点共有を行った送信者
//     QString userIDStr = QString::number( userID );
//     double x    = dataArray[QString::fromUtf8( Protocol::Key::X )].toDouble();
//     double y    = dataArray[QString::fromUtf8( Protocol::Key::Y )].toDouble();
//     double z    = dataArray[QString::fromUtf8( Protocol::Key::Z )].toDouble();
//     double dx   = dataArray[QString::fromUtf8( Protocol::Key::Dx )].toDouble();
//     double dy   = dataArray[QString::fromUtf8( Protocol::Key::Dy )].toDouble();
//     double dz   = dataArray[QString::fromUtf8( Protocol::Key::Dz )].toDouble();

//     kvs::Real32 CoordArray[ 1 * 3 ] =
//         {
//             kvs::Real32( x ),
//             kvs::Real32( y ),
//             kvs::Real32( z ),
//         };

//     kvs::Real32 DirectionArray[ 1 * 3 ] =
//         {
//             kvs::Real32( dx ),
//             kvs::Real32( dy ),
//             kvs::Real32( dz ),
//         };

// #ifdef OPENXR_SCREEN
//     // シーン全体のサイズを基準にした base_size
//     kvs::Vec3 min_coord = m_screen->scene()->objectManager()->minObjectCoord();
//     kvs::Vec3 max_coord = m_screen->scene()->objectManager()->maxObjectCoord();
//     kvs::Vec3 diag = max_coord - min_coord;
//     float scene_size = diag.length();
//     float base_size = scene_size * 0.005f; // シーン全体に対する割合

//     // 現在のスケーリング値を取得
//     kvs::Xform xform = m_screen->scene()->objectManager()->xform();
//     float scalingFactor = xform.scaling().x(); // (x,y,z が同じなら x でOK)

//     // スケーリングに反比例させることで拡大縮小に依存しないサイズにする
//     kvs::Real32 SizeArray[1] = { base_size / scalingFactor };
// #else
//     kvs::Xform currentObjectManagerXform = m_screen->scene()->objectManager()->xform(); // 現在のオブジェクトマネージャーのTranslation, Scaling, Rotationを取得する。
//     float scalingFactor = 1 / ( currentObjectManagerXform.inverse() * m_screen->scene()->object( m_screen->scene()->numberOfObjects() - 1 )->xform() ).scaling().x();
//     kvs::Real32 SizeArray[ 1 ] = {
//         0.5f * scalingFactor,
//     };
// #endif

//     kvs::UInt8 ColorArray[ 1 * 3 ] =
//         {
//             0, 255, 0,
//         };

//     kvs::ValueArray<kvs::Real32> coords( CoordArray, 1 * 3 );
//     kvs::ValueArray<kvs::Real32> direction( DirectionArray, 1 * 3 );
//     kvs::ValueArray<kvs::Real32> size( SizeArray, 1 );
//     kvs::ValueArray<kvs::UInt8> colors( ColorArray, 1 * 3 );

//     if( m_screen->scene()->object( userID + "_SharedGlyph" ) == nullptr )
//     {
//         kvs::PolygonObject* sharedPolygon = createArrowGlyph( coords, direction, size, colors );
//         sharedPolygon->setName( userID + "_SharedGlyph" );
//         sharedPolygon->setXform( m_screen->scene()->objectManager()->xform() );
//         sharedPolygon->setMinMaxObjectCoords( m_screen->scene()->object( m_server_point_object_ids.first )->minObjectCoord(), m_screen->scene()->object( m_server_point_object_ids.first )->maxObjectCoord() );
//         sharedPolygon->setMinMaxExternalCoords( m_screen->scene()->object( m_server_point_object_ids.first )->minObjectCoord(), m_screen->scene()->object( m_server_point_object_ids.first )->maxObjectCoord() );
//         kvs::StochasticPolygonRenderer* renderer = new kvs::StochasticPolygonRenderer();
//         m_screen->registerObject( sharedPolygon, renderer );
//         m_screen->update();
//     }
//     else
//     {
//         kvs::PolygonObject* sharedPolygon = createArrowGlyph( coords, direction, size, colors );
//         sharedPolygon->setName( userID + "_SharedGlyph" );
//         sharedPolygon->setXform( m_screen->scene()->objectManager()->xform() );
//         sharedPolygon->setMinMaxObjectCoords( m_screen->scene()->object( m_server_point_object_ids.first )->minObjectCoord(), m_screen->scene()->object( m_server_point_object_ids.first )->maxObjectCoord() );
//         sharedPolygon->setMinMaxExternalCoords( m_screen->scene()->object( m_server_point_object_ids.first )->minObjectCoord(), m_screen->scene()->object( m_server_point_object_ids.first )->maxObjectCoord() );
//         m_screen->scene()->replaceObject( userID + "_SharedGlyph", sharedPolygon );
//         m_screen->update();
//     }
}

kvs::PolygonObject* Communication::createArrowGlyph(
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::Real32>& directions,
    const kvs::ValueArray<kvs::Real32>& sizes,
    const kvs::ValueArray<kvs::UInt8>& colors )
{
    // const size_t npoint = coords.size() / 3;
    // const int slices = 20;

    // std::vector<kvs::Vec3> all_vertices;
    // std::vector<kvs::Vec3> all_normals;
    // std::vector<kvs::UInt32> all_indices;
    // std::vector<kvs::UInt8> all_colors;

    // for( size_t i = 0, index = 0; i < npoint; i++, index += 3 )
    // {
    //     kvs::Vec3 tip_position( coords.data() + index );   // 先端位置
    //     kvs::Vec3 direction( directions.data() + index );
    //     kvs::Real32 size = sizes[i];
    //     kvs::RGBColor color( colors.data() + index );

    //     if( direction.length() < 1e-6 )
    //     {
    //         std::cerr << "Error: Invalid direction vector." << std::endl;
    //         continue;
    //     }
    //     direction = direction.normalized();

    //     // --- 矢印寸法 ---
    //     const float cylinder_height = 0.7f * size;
    //     const float cylinder_radius = 0.07f * size;
    //     const float cone_height = 0.3f * size;
    //     const float cone_radius = 0.15f * size;
    //     const float arrow_height = cylinder_height + cone_height;

    //     std::vector<kvs::Vec3> vertices;
    //     std::vector<kvs::Vec3> normals;
    //     std::vector<kvs::UInt32> indices;

    //     // --- 円柱の構築 ---
    //     for( int s = 0; s < slices; ++s )
    //     {
    //         float angle = 2.0f * M_PI * s / slices;
    //         float x = cylinder_radius * std::cos( angle );
    //         float y = cylinder_radius * std::sin( angle );

    //         vertices.emplace_back( x, y, 0.0f );              // 底面
    //         normals.emplace_back( x, y, 0.0f );

    //         vertices.emplace_back( x, y, cylinder_height );   // 上面
    //         normals.emplace_back( x, y, 0.0f );
    //     }

    //     for( int s = 0; s < slices; ++s )
    //     {
    //         int next = ( s + 1 ) % slices;
    //         indices.push_back( s * 2 ); indices.push_back( next * 2 ); indices.push_back( s * 2 +1 );
    //         indices.push_back( s * 2 +1 ); indices.push_back( next * 2 ); indices.push_back( next * 2 +1 );
    //     }

    //     // --- 円錐の構築 ---
    //     std::vector<kvs::Vec3> base_vertices;
    //     for( int s = 0; s < slices; ++s )
    //     {
    //         float angle = 2.0f * M_PI * s / slices;
    //         base_vertices.emplace_back( cone_radius*std::cos( angle ), cone_radius*std::sin( angle ), cylinder_height );
    //     }

    //     for( int s = 0; s < slices; ++s )
    //     {
    //         int next = ( s + 1 ) % slices;
    //         kvs::Vec3 apex( 0, 0, cylinder_height + cone_height );
    //         kvs::Vec3 v1 = base_vertices[s];
    //         kvs::Vec3 v2 = base_vertices[next];

    //         vertices.push_back( v1 ); vertices.push_back( v2 ); vertices.push_back( apex );
    //         kvs::Vec3 normal = ( v2 - v1 ).cross( apex - v1 ).normalized();
    //         normals.push_back( normal ); normals.push_back( normal ); normals.push_back( normal );

    //         int base_index = vertices.size() - 3;
    //         indices.push_back( base_index ); indices.push_back( base_index + 1 ); indices.push_back( base_index + 2 );
    //     }

    //     // --- 先端が tip_position になるように Zを下方向にシフト ---
    //     for( auto& v : vertices ) v.z() -= arrow_height;

    //     // --- 回転・位置調整 ---
    //     kvs::Vec3 default_direction( 0, 0, 1 );
    //     kvs::Vec3 axis = default_direction.cross( direction );
    //     float angle = std::acos( default_direction.dot( direction ) );
    //     kvs::Mat3 rotation;
    //     if( axis.length() > 1e-6 )
    //     {

    //         rotation = kvs::Mat3::Rotation( axis.normalized(), angle*180.0/M_PI );
    //     }
    //     else
    //     {
    //         rotation = kvs::Mat3::Identity();
    //     }

    //     for( auto& v : vertices ) v = rotation * v + tip_position;
    //     for( auto& n : normals ) n = rotation * n;

    //     size_t offset = all_vertices.size();
    //     all_vertices.insert( all_vertices.end(), vertices.begin(), vertices.end() );
    //     all_normals.insert( all_normals.end(), normals.begin(), normals.end() );
    //     for( auto idx : indices ) all_indices.push_back( idx + offset );

    //     for( size_t c =0; c< vertices.size(); ++c )
    //     {
    //         all_colors.push_back( color.r() );
    //         all_colors.push_back( color.g() );
    //         all_colors.push_back( color.b() );
    //     }
    // }

    kvs::PolygonObject* polygon = new kvs::PolygonObject();
    // polygon->setCoords( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_vertices.data(), all_vertices.size()*3 ) );
    // polygon->setConnections( kvs::ValueArray<kvs::UInt32>( all_indices.data(), all_indices.size() ) );
    // polygon->setColors( kvs::ValueArray<kvs::UInt8>( all_colors.data(), all_colors.size() ) );
    // polygon->setNormals( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_normals.data(), all_normals.size()*3 ) );
    // polygon->setOpacity( 255 );
    // polygon->setPolygonType( kvs::PolygonObject::Triangle );
    // polygon->setColorType( kvs::PolygonObject::PolygonColor );
    // polygon->setNormalType( kvs::PolygonObject::VertexNormal );

    return polygon;
}

void Communication::onModeClicked()
{
    if( ui->remoteVizInsituRadioButton->isChecked() )
    {
        ui->volumeDataFilePathLineEdit->clear();
        ui->volumeDataFilePathLineEdit->setPlaceholderText( "It is not required for In-Situ." );
        ui->volumeDataFilePathLineEdit->setEnabled( false );
        ui->volumeDataFilePathPushButton->setEnabled( false );

        ui->transferFunctionFilePathLineEdit->clear();
        ui->transferFunctionFilePathLineEdit->setPlaceholderText( "It is not required for In-Situ." );
        ui->transferFunctionFilePathLineEdit->setEnabled( false );
        ui->transferFunctionFilePathPushButton->setEnabled( false );
    }
    else
    {
        ui->volumeDataFilePathLineEdit->setPlaceholderText( "e.g., path/to/sample.pfi (or .pfl) (required)" );
        ui->volumeDataFilePathLineEdit->setEnabled( true );
        ui->volumeDataFilePathPushButton->setEnabled( true );
        ui->transferFunctionFilePathLineEdit->setPlaceholderText( "e.g., path/to/sample.tf (or .tfe) (optional)" );
        ui->transferFunctionFilePathLineEdit->setEnabled( true );
        ui->transferFunctionFilePathPushButton->setEnabled( true );
    }
}

void Communication::onVolumeDataFilePathClicked()
{
    const QString filePath = QFileDialog::getOpenFileName( this, tr( "Load Volume Data File" ), QString(), tr( "Volume Data File (*.pfi *.pfl);;All Files (*)" ) );
    ui->volumeDataFilePathLineEdit->setText( filePath );
}

void Communication::onTransferFunctionFilePathClicked()
{
    const QString filePath = QFileDialog::getOpenFileName( this, tr("Load Transfer Function File"), QString(), tr("Transfer Function File (*.tfe *.TFE *.tf *.TF);;All Files (*)") );
    ui->transferFunctionFilePathLineEdit->setText( filePath );
}

void Communication::onConnectClicked()
{
    if( m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( "Already connected." );
        return;
    }

    if( !ui->localVizRadioButton                ->isChecked() &&
        !ui->remoteVizClientServerRadioButton   ->isChecked() &&
        !ui->remoteVizInsituRadioButton         ->isChecked() )
    {
        QMessageBox::warning( this, tr( "Warning" ), tr( "Please select a visualization mode." ) ); // Modeを選択してください。
        return;
    }

    // if( ui->localVizRadioButton             ->isChecked() && ui->volumeDataFilePathLineEdit->text().isEmpty() ||
    //     ui->remoteVizClientServerRadioButton->isChecked() && ui->volumeDataFilePathLineEdit->text().isEmpty() )
    // {
    //     QMessageBox::warning( this, tr( "Warning" ), tr( "Volume Data File is required for this mode." ) ); // FIXME:入力がなかった場合はゲスト
    // }

    if( ui->addressLineEdit->text().isEmpty() )
    {
        QMessageBox::warning( this, tr( "Warning" ), tr( "Please enter the address." ) ); // 接続先アドレスを入力してください。
        return;
    }

    // FIXME:Local Viz.(Client Only)でオブジェクトを登録していた場合、サーバ接続時に削除されてしまうので警告ダイアログを表示するようにしてください。

    m_user_uuid = QUuid::createUuid().toString( QUuid::WithoutBraces ); // ユーザUUID
    const QString address = ui->addressLineEdit->text().toUtf8().constData(); // FIXME:wss:で接続できない。サーバ側の修正が必要かもしれません。要SSL対応
    const QString binaryAddress = address + "/binary?uuid=" + m_user_uuid;
    const QString textAddress   = address + "/text?uuid=" + m_user_uuid;

    emit updateStatusBarMessage( "Connecting to " + address );
    if( m_web_sockets->binary() )   m_web_sockets->binary() ->open( QUrl( binaryAddress ) );
    if( m_web_sockets->text() )     m_web_sockets->text()   ->open( QUrl( textAddress ) );
}

void Communication::onDisconnectClicked()
{
    if( !m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( "Not connected." );
        return;
    }

    m_user_uuid.clear();
    emit updateStatusBarMessage( "Disconnect." );
    m_web_sockets->closeAll();
}

void Communication::onTransferOperator()
{
    if( !m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( "Not connected." );
        return;
    }

    if( !m_is_operator )
    {
        emit updateStatusBarMessage( "You are not the operator." );
        return;
    }

    bool isValid = false;
    int targetID = ui->transferOperatorIDlineEdit->text().toInt( &isValid );

    if( !isValid )
    {
        emit updateStatusBarMessage( "Invalid ID." );
        return;
    }

    if( m_user_id == targetID )
    {
        emit updateStatusBarMessage( "You cannot transfer yourself." );
    }

    m_web_sockets->text()->sendTextMessage( QJsonDocument( {
                                                          { QString::fromUtf8( Protocol::Key::Event ), QString::fromUtf8( Protocol::Events::TransferOperator )},
                                                          { QString::fromUtf8( Protocol::Key::TargetID ), targetID },
                                                          } ).toJson( QJsonDocument::Compact ) );
}

void Communication::onChatClicked()
{
    if( !m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( "Not connected." );
        return;
    }

    QString text = ui->chatLineEdit->text().trimmed();
    if( text.isEmpty() ) return; // 何も入力されていない場合は何もしない

    m_web_sockets->text()->sendTextMessage( QJsonDocument( {
                                                          { QString::fromUtf8( Protocol::Key::Event ), QString::fromUtf8( Protocol::Events::Chat ) },
                                                          { QString::fromUtf8( Protocol::Key::Text ), text },
                                                          } ).toJson( QJsonDocument::Compact ) );
    ui->chatLineEdit->clear();
}

void Communication::onShareView()
{
    if( !m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( "Not connected." );
        return;
    }

    kvs::Xform currentXform = m_screen->scene()->objectManager()->xform();
    const kvs::Matrix44f matrix = currentXform.toMatrix();

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

    m_web_sockets->text()->sendTextMessage( QJsonDocument( {
                                                          { QString::fromUtf8( Protocol::Key::Event ), QString::fromUtf8( Protocol::Events::ShareView ) },
                                                          { QString::fromUtf8( Protocol::Key::Matrix ), matrix_array },
                                                          } ).toJson( QJsonDocument::Compact ) );
}

void Communication::onBinaryWebsocketConnected()
{
    websocketConnected();
}

void Communication::onBinaryWebsocketDisconnected()
{
    websocketDisconnected();
}

void Communication::onBinaryWebsocketMessageReceived( const QByteArray& binary )
{
    emit unpack( binary );
}

void Communication::onTextWebsocketConnected()
{
    websocketConnected();
}

void Communication::onTextWebsocketDisconnected()
{
    websocketDisconnected();
}

void Communication::onTextWebsocketMessageReceived( const QString& receivedMessage )
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson( receivedMessage.toUtf8(), &parseError );
    // if( parseError.error != QJsonParseError::NoError || !doc.isObject() )
    // {
    //     qWarning() << "Invalid JSON received";
    //     return;
    // }

    QJsonObject obj = doc.object();

    // "Event" キーが存在するかチェック
    const QString keyEvent = QString::fromUtf8( Protocol::Key::Event );
    if( !obj.contains( keyEvent ) ) return;

    const QString event = obj.value( keyEvent ).toString();

    // ==== イベント分岐 ====
    if( event == "Template" ) qDebug() << __LINE__;
    else if( event == QString::fromUtf8( Protocol::Events::Join ) )                         Join( obj );
    else if( event == QString::fromUtf8( Protocol::Events::Left ) )                         Left( obj );
    else if( event == QString::fromUtf8( Protocol::Events::ID ) )                           ID( obj );
    else if( event == QString::fromUtf8( Protocol::Events::Operator ) )                     Operator( obj );
    else if( event == QString::fromUtf8( Protocol::Events::TransferOperator ) )             transferOperator( obj );
    else if( event == QString::fromUtf8( Protocol::Events::Initialize ) )
    {
        // TransferFunctionParameter
        const auto tfKey = QString::fromUtf8( Protocol::Key::TransferFunctionParameter );
        if( obj.contains( tfKey ) && obj.value( tfKey ).isObject() )
        {
            const QJsonObject transferFunctionObject = obj.value( tfKey ).toObject();

            const QString colorSynth   = transferFunctionObject.value( QString::fromUtf8( Protocol::Key::ColorSynthesizer ) ).toString();
            const QString opacitySynth = transferFunctionObject.value( QString::fromUtf8( Protocol::Key::OpacitySynthesizer ) ).toString();
            const QJsonArray dataArray = transferFunctionObject.value( QString::fromUtf8( Protocol::Key::Data ) ).toArray();

            emit receiveInitializeTransferFunctionParameter( colorSynth, opacitySynth, dataArray );
        }

        // GlyphParameter
        const auto glyphKey = QString::fromUtf8( Protocol::Key::GlyphParameter );
        if( obj.contains( glyphKey ) && obj.value( glyphKey ).isObject() )
        {
            const QJsonObject glyphObject = obj.value( glyphKey ).toObject();
            emit receiveInitializeGlyphParameter( glyphObject );
        }

        // PlotOverLineParameter
        const auto polKey = QString::fromUtf8( Protocol::Key::PlotOverLineParameter );
        if( obj.contains( polKey ) && obj.value(polKey).isObject() )
        {
            const QJsonObject plotOverLineObject = obj.value( polKey ).toObject();
            emit receiveInitializePlotOverLineParameter( plotOverLineObject );
        }
    }
    else if( event == QString::fromUtf8( Protocol::Events::Chat ) )                         chat( obj );
    else if( event == QString::fromUtf8( Protocol::Events::SelectedFile ) )                 emit receiveSelectedFile( obj );
    else if( event == QString::fromUtf8( Protocol::Events::ObjectDelete ) )                 emit receiveObjectDelete( obj );
    else if( event == QString::fromUtf8( Protocol::Events::ShareView ) )                    shareView( obj );
    else if( event == QString::fromUtf8( Protocol::Events::SharePoint ) )                   sharePoint( obj );
    // else if( event == QString::fromUtf8( Protocol::Events::FileList ) )                     fileList( obj );
    else if( event == QString::fromUtf8( Protocol::Events::ObjectInfoParameter ) )          emit receiveObjectInfoParameter( obj );
    // {
        // QJsonArray resultMinObjectCoordsArray   = obj[QString::fromUtf8( Protocol::Key::ResultMinObjectCoords )].toArray();
        // QJsonArray resultMaxObjectCoordsArray   = obj[QString::fromUtf8( Protocol::Key::ResultMaxObjectCoords )].toArray();
        // QJsonArray objectsArray                 = obj[QString::fromUtf8( Protocol::Key::Objects )].toArray();
        // receiveObjectInfoParameter( resultMinObjectCoordsArray, resultMaxObjectCoordsArray, objectsArray );
    // }
    else if( event == QString::fromUtf8( Protocol::Events::TransferFunctionParameter ) )
    {
        // synthesize 情報
        QString colorSynth      = obj.value(QString::fromUtf8( Protocol::Key::ColorSynthesizer ) ).toString().toUtf8();
        QString opacitySynth    = obj.value(QString::fromUtf8( Protocol::Key::OpacitySynthesizer ) ).toString().toUtf8();
        QJsonArray dataArray    = obj.value(QString::fromUtf8( Protocol::Key::Data ) ).toArray();
        receiveTransferFunctionParameter( colorSynth, opacitySynth, dataArray );
    }
    else if( event == QString::fromUtf8( Protocol::Events::GlyphParameter ) )               receiveGlyphParameter( obj );
    else if( event == QString::fromUtf8( Protocol::Events::PlotOverLineParameter ) )        receivePlotOverLineParameter( obj );
    else if( event == QString::fromUtf8( Protocol::Events::RequestDataAt ) )
    {
        // TransferFunctionParameter
        const auto tfKey = QString::fromUtf8( Protocol::Key::TransferFunctionParameter );
        if( obj.contains( tfKey ) && obj.value( tfKey ).isObject() )
        {
            const QJsonObject transferFunctionObject = obj.value( tfKey ).toObject();;
            const QJsonArray dataArray = transferFunctionObject.value( QString::fromUtf8( Protocol::Key::Data ) ).toArray();

            emit receiveRequestDataAtTransferFunctionParameter( dataArray );
        }

        // PlotOverLineParameter
        const auto polKey = QString::fromUtf8( Protocol::Key::PlotOverLineParameter );
        if( obj.contains( polKey ) && obj.value( polKey ).isObject() )
        {
            const QJsonObject polObj = obj.value( polKey ).toObject();
            emit receiveRequestDataAtPlotOverLineParameter( polObj );
        }
    }
    else if( event == QString::fromUtf8( Protocol::Events::TimeStepControlParameter ) )     receiveTimeStepControlParameter( obj );
    else if( event == QString::fromUtf8( Protocol::Events::LatestTimeStep ) )
    {
        const QString key = QString::fromUtf8( Protocol::Key::UpdateMaxTimeStep );

        if( obj.contains(key) )
        {
            const int latest = obj.value( key ).toInt();
            emit updateMaxTimeStep( latest );
        }
    }
    else emit updateStatusBarMessage( "Unknown event received. Please check that the client and server versions match." );
}
