#include "Communication.h"
#include "ui_Communication.h"

Communication::Communication( kvs::qt::jaea::Screen* screen, WebSocketPair* webSockets, Viz::Mode* vizMode, QWidget* parent )
    : QDockWidget( parent )
    , ui( new Ui::Communication )
    , m_screen( screen )
    , m_web_sockets( webSockets )
    , m_viz_mode( vizMode )
{
    ui->setupUi( this );

    ui->uniformRadioButton->setChecked( true );    // NOTE:起動時デフォルトはUniformサンプリング
    ui->disconnectPushButton->setEnabled( false ); // NOTE:起動時デフォルトは未接続のため無効
    ui->addressLineEdit->setText( "ws://127.0.0.1:60000" );

    connect( ui->connectPushButton                 ,&QPushButton::clicked             , this, &Communication::onConnectClicked );
    connect( ui->disconnectPushButton              ,&QPushButton::clicked             , this, &Communication::onDisconnectClicked );

    connect( ui->localVizRadioButton               ,&QRadioButton::clicked            , this, &Communication::onModeClicked );
    connect( ui->remoteVizClientServerRadioButton  ,&QRadioButton::clicked            , this, &Communication::onModeClicked );
    connect( ui->remoteVizInsituRadioButton        ,&QRadioButton::clicked            , this, &Communication::onModeClicked );
    connect( ui->volumeDataFilePathPushButton      ,&QPushButton::clicked             , this, &Communication::onVolumeDataFilePathClicked );
    connect( ui->transferFunctionFilePathPushButton,&QPushButton::clicked             , this, &Communication::onTransferFunctionFilePathClicked );
    connect( ui->settingApplyPushButton            ,&QPushButton::clicked             , this, &Communication::onSettingsApplyClicked );

    connect( ui->transferOperatorApplyPushButton   ,&QPushButton::clicked             , this, &Communication::onTransferOperator );
    connect( ui->chatLineEdit                      ,&QLineEdit::returnPressed         , this, &Communication::onSendChatMessage );
    connect( ui->shareViewPushButton               ,&QPushButton::clicked             , this, &Communication::onShareView );

    connect( m_web_sockets->binary()               ,&QWebSocket::connected            , this, &Communication::onBinaryWebSocketConnected );       // 接続成功(バイナリ)
    connect( m_web_sockets->binary()               ,&QWebSocket::disconnected         , this, &Communication::onBinaryWebSocketDisconnected );    // 接続切断(バイナリ)
    connect( m_web_sockets->binary()               ,&QWebSocket::binaryMessageReceived, this, &Communication::onBinaryWebSocketMessageReceived ); // メッセージ受信(バイナリ)
    connect( m_web_sockets->text()                 ,&QWebSocket::connected            , this, &Communication::onTextWebSocketConnected );         // 接続成功(テキスト)
    connect( m_web_sockets->text()                 ,&QWebSocket::disconnected         , this, &Communication::onTextWebSocketDisconnected );      // 接続切断(テキスト)
    connect( m_web_sockets->text()                 ,&QWebSocket::textMessageReceived  , this, &Communication::onTextWebSocketMessageReceived );   // メッセージ受信(テキスト)
}

Communication::~Communication()
{
    delete ui;
}

void Communication::onVRSharePoint( kvs::Real32 CoordArray[ 2 * 3 ], kvs::Real32 DirectionArray[ 3 ] )
{
    if( !m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( k_not_connected_text );
        return;
    }

    double x = CoordArray[0];
    double y = CoordArray[1];
    double z = CoordArray[2];

    double dx = DirectionArray[0];
    double dy = DirectionArray[1];
    double dz = DirectionArray[2];

    m_web_sockets->text()->sendTextMessage( QJsonDocument( {
                                                             { QString::fromUtf8( Protocol::Key::Event ), QString::fromUtf8( Protocol::Events::SharePoint ) },
                                                             { QString::fromUtf8( Protocol::Key::X )    ,  x },
                                                             { QString::fromUtf8( Protocol::Key::Y )    ,  y },
                                                             { QString::fromUtf8( Protocol::Key::Z )    ,  z },
                                                             { QString::fromUtf8( Protocol::Key::Dx )   , dx },
                                                             { QString::fromUtf8( Protocol::Key::Dy )   , dy },
                                                             { QString::fromUtf8( Protocol::Key::Dz )   , dz }
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

void Communication::webSocketConnected()
{
    // NOTE:Unuse
}

void Communication::webSocketDisconnected()
{
    if( !m_web_sockets->isConnected() )
    {
        m_user_id = -1;
        m_is_operator = false;

        emit updateServerState( false );
        emit updateOperatorState( m_is_operator );

        ui->localVizRadioButton               ->setEnabled( true );
        ui->remoteVizClientServerRadioButton  ->setEnabled( true );
        ui->remoteVizInsituRadioButton        ->setEnabled( true );

        ui->uniformRadioButton                ->setEnabled( true );
        ui->metropolisRadioButton             ->setEnabled( true );
        ui->rejectionRadioButton              ->setEnabled( true );

        ui->volumeDataFilePathLineEdit        ->setEnabled( true );
        ui->volumeDataFilePathPushButton      ->setEnabled( true );
        ui->transferFunctionFilePathLineEdit  ->setEnabled( true );
        ui->transferFunctionFilePathPushButton->setEnabled( true );

        ui->addressLineEdit                   ->setEnabled( true );
        ui->connectPushButton                 ->setEnabled( true );
        ui->disconnectPushButton              ->setEnabled( false );

        ui->IDLineEdit->clear();
        ui->isOperatorLineEdit->clear();
        ui->textBrowser->clear();

        updateVizMode();
    }
}

void Communication::receiveJoin( const QJsonObject& payload )
{
    const int userID = payload.value( QString::fromUtf8( Protocol::Key::UserID ) ).toInt(); // NOTE:入室者のUserID

    ui->textBrowser->append( QString( "--- User[%1] Join" ).arg( userID ) );
}

void Communication::receiveLeft( const QJsonObject& payload )
{
    const int userID = payload.value( QString::fromUtf8( Protocol::Key::UserID ) ).toInt(); // NOTE:退出者のUserID

    ui->textBrowser->append( QString( "--- User[%1] Left" ).arg( userID ) );
}

void Communication::receiveID( const QJsonObject& payload )
{
    const int userID = payload.value( QString::fromUtf8( Protocol::Key::UserID ) ).toInt(); // NOTE:自分自身のUserID

    m_user_id = userID;
    ui->IDLineEdit->setText( QString::number( userID ) );
    ui->textBrowser->append( QString( "--- Your User ID is [%1]" ).arg( userID ) );
}

void Communication::receiveOperator( const QJsonObject& payload )
{
    const auto kVizMode    = QString::fromUtf8( "VizMode" );
    const auto kUserID     = QString::fromUtf8( Protocol::Key::UserID );
    const auto kIsOperator = QString::fromUtf8( Protocol::Key::IsOperator );


    emit updateServerState( true );

    if( payload.contains( kVizMode ))
    {
        *m_viz_mode = static_cast<Viz::Mode>( payload.value( kVizMode ).toInt() );
    }

    if( payload.contains( kUserID ) && payload.contains( kIsOperator ) )
    {
        const int userID = payload.value( kUserID ).toInt();           // NOTE:権限があるUserID
        const bool isOperator = payload.value( kIsOperator ).toBool(); // NOTE:操作権限の有無 true: 権限有り, false: 権限無し
        if( m_user_id == userID )
        {
            m_is_operator = isOperator;
            emit updateOperatorState( m_is_operator );
        }
        else
        {
            m_is_operator = false;
            emit updateOperatorState( m_is_operator );
        }
    }

    ui->isOperatorLineEdit->setText( m_is_operator ? "true" : "false" );
    ui->textBrowser->append( m_is_operator ? "--- You have operator privilege"
                                           : "--- You are not operator" );
}

void Communication::receiveTransferOperator( const QJsonObject& payload )
{
    const int oldOperatorID = payload.value( QString::fromUtf8( Protocol::Key::OldOperatorID ) ).toInt(); // NOTE:以前の操作権ユーザのUserID
    const int newOperatorID = payload.value( QString::fromUtf8( Protocol::Key::NewOperatorID ) ).toInt(); // NOTE:新しい操作権ユーザのUserID

    m_is_operator = ( m_user_id == newOperatorID );
    emit updateOperatorState( m_is_operator );

    ui->isOperatorLineEdit->setText( m_is_operator ? "true" : "false" );
    ui->textBrowser->append( QString( "--- Operator change [%1] to [%2]" ).arg( oldOperatorID ).arg( newOperatorID ) );
}

void Communication::receiveChat( const QJsonObject& payload )
{
    const int userID       = payload.value( QString::fromUtf8( Protocol::Key::UserID ) ).toInt();  // NOTE:発言者のUserID
    const QString chatText = payload.value( QString::fromUtf8( Protocol::Key::Text ) ).toString(); // NOTE:チャット内容

    ui->textBrowser->append( QString( "User[%1]: %2" ).arg( userID ).arg( chatText ) );
}

void Communication::receiveShareView( const QJsonObject& payload )
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

void Communication::receiveSharePoint( const QJsonObject& payload )
{
    // NOTE:PlotOverLineEditorがnullptrじゃない場合のみ機能します。
    const kvs::Vec3 startPointMinObjectCoords = m_screen->scene()->object( "StartPointObject" )->minObjectCoord();
    const kvs::Vec3 startPointMaxObjectCoords = m_screen->scene()->object( "StartPointObject" )->maxObjectCoord();
    if( startPointMinObjectCoords == startPointMaxObjectCoords ) { return; }

    const int userID = payload[QString::fromUtf8( Protocol::Key::UserID )].toInt(); // 着目点共有を行った送信者
    if( userID > m_max_shared_user_id )
    {
        m_max_shared_user_id = userID;
    }
    const QString userIDStr = QString::number( userID );
    const double x          = payload[QString::fromUtf8( Protocol::Key::X )].toDouble();
    const double y          = payload[QString::fromUtf8( Protocol::Key::Y )].toDouble();
    const double z          = payload[QString::fromUtf8( Protocol::Key::Z )].toDouble();
    const double dx         = payload[QString::fromUtf8( Protocol::Key::Dx )].toDouble();
    const double dy         = payload[QString::fromUtf8( Protocol::Key::Dy )].toDouble();
    const double dz         = payload[QString::fromUtf8( Protocol::Key::Dz )].toDouble();

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

    kvs::ValueArray<kvs::Real32> coords   ( CoordArray, 1 * 3 );
    kvs::ValueArray<kvs::Real32> direction( DirectionArray, 1 * 3 );
    kvs::ValueArray<kvs::Real32> size     ( SizeArray, 1 );
    kvs::ValueArray<kvs::UInt8>  colors   ( ColorArray, 1 * 3 );

    if( m_screen->scene()->object( userID + "_SharedGlyph" ) == nullptr )
    {
        kvs::PolygonObject* sharedPolygon = createArrowGlyph( coords, direction, size, colors );
        sharedPolygon->setName( userID + "_SharedGlyph" );
        sharedPolygon->setXform( m_screen->scene()->objectManager()->xform() );
        sharedPolygon->setMinMaxObjectCoords( startPointMinObjectCoords, startPointMaxObjectCoords );
        sharedPolygon->setMinMaxExternalCoords( startPointMinObjectCoords, startPointMaxObjectCoords );
        kvs::StochasticPolygonRenderer* renderer = new kvs::StochasticPolygonRenderer();
        m_screen->registerObject( sharedPolygon, renderer );
        m_screen->update();
    }
    else
    {
        kvs::PolygonObject* sharedPolygon = createArrowGlyph( coords, direction, size, colors );
        sharedPolygon->setName( userID + "_SharedGlyph" );
        sharedPolygon->setXform( m_screen->scene()->objectManager()->xform() );
        sharedPolygon->setMinMaxObjectCoords( startPointMinObjectCoords, startPointMaxObjectCoords );
        sharedPolygon->setMinMaxExternalCoords( startPointMinObjectCoords, startPointMaxObjectCoords );
        m_screen->scene()->replaceObject( userID + "_SharedGlyph", sharedPolygon );
        m_screen->update();
    }
}

void Communication::updateVizMode()
{
    // NOTE:Unuse
}

void Communication::onConnectClicked()
{
    if( m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( "Already connected." );
        return;
    }

    m_user_uuid = QUuid::createUuid().toString( QUuid::WithoutBraces );
    const QString address       = ui->addressLineEdit->text().toUtf8().constData(); // FIXME:wssで接続できません。SSL対応が必要かもしれません
    const QString binaryAddress = address + "/binary?uuid=" + m_user_uuid;
    const QString textAddress   = address + "/text?uuid=" + m_user_uuid;

    emit updateStatusBarMessage( "Connecting to " + address );
    if( m_web_sockets->binary() ) m_web_sockets->binary()->open( QUrl( binaryAddress ) );
    if( m_web_sockets->text() )   m_web_sockets->text()  ->open( QUrl( textAddress ) );
}

void Communication::onDisconnectClicked()
{
    if( !m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( k_not_connected_text );
        return;
    }

    m_user_uuid.clear();
    emit updateStatusBarMessage( "Disconnect." );
    m_web_sockets->closeAll();
}

void Communication::onModeClicked()
{
    const bool in_situ = ui->remoteVizInsituRadioButton->isChecked();
    auto setupPathInput = [&]( QLineEdit* edit, QPushButton* button, const QString& placeholder, bool enabled, bool clear )
    {
        if( clear ) { edit->clear(); }
        edit->setPlaceholderText( placeholder );
        edit->setEnabled( enabled );
        button->setEnabled( enabled );
    };

    if( in_situ )
    {
        const QString msg = "It is not required for In-Situ.";

        setupPathInput( ui->volumeDataFilePathLineEdit      , ui->volumeDataFilePathPushButton      , msg, /*enabled=*/false, /*clear=*/true );
        setupPathInput( ui->transferFunctionFilePathLineEdit, ui->transferFunctionFilePathPushButton, msg, /*enabled=*/false, /*clear=*/true );
    }
    else
    {
        setupPathInput( ui->volumeDataFilePathLineEdit      , ui->volumeDataFilePathPushButton      , "e.g., path/to/sample.pfi (or .pfl) (required)", /*enabled=*/true, /*clear=*/false );
        setupPathInput( ui->transferFunctionFilePathLineEdit, ui->transferFunctionFilePathPushButton, "e.g., path/to/sample.tf (or .tfe) (optional)" , /*enabled=*/true, /*clear=*/false );
    }
}

void Communication::onVolumeDataFilePathClicked()
{
    const QString filePath = QFileDialog::getOpenFileName( this, tr( "Load Volume Data File" ), QString(), tr( "Volume Data File (*.pfi *.pfl);;All Files (*)" ) );
    if( filePath.isEmpty() ) { return; }
    ui->volumeDataFilePathLineEdit->setText( filePath );
}

void Communication::onTransferFunctionFilePathClicked()
{
    const QString filePath = QFileDialog::getOpenFileName( this, tr( "Load Transfer Function File" ), QString(), tr( "Transfer Function File (*.tfe *.TFE *.tf *.TF);;All Files (*)" ) );
    if( filePath.isEmpty() ) { return; }
    ui->transferFunctionFilePathLineEdit->setText( filePath );
}

void Communication::onSettingsApplyClicked()
{
    if( m_web_sockets->isConnected() )
    {
        // NOTE:モードが選択されていない場合は警告
        if( !ui->localVizRadioButton             ->isChecked() &&
            !ui->remoteVizClientServerRadioButton->isChecked() &&
            !ui->remoteVizInsituRadioButton      ->isChecked() )
        {
            emit updateStatusBarMessage( "Please select a visualization mode." );
            return;
        }

        ui->localVizRadioButton               ->setEnabled( false );
        ui->remoteVizClientServerRadioButton  ->setEnabled( false );
        ui->remoteVizInsituRadioButton        ->setEnabled( false );

        ui->uniformRadioButton                ->setEnabled( false );
        ui->metropolisRadioButton             ->setEnabled( false );
        ui->rejectionRadioButton              ->setEnabled( false );

        ui->volumeDataFilePathLineEdit        ->setEnabled( false );
        ui->volumeDataFilePathPushButton      ->setEnabled( false );
        ui->transferFunctionFilePathLineEdit  ->setEnabled( false );
        ui->transferFunctionFilePathPushButton->setEnabled( false );

        ui->addressLineEdit                   ->setEnabled( false );
        ui->connectPushButton                 ->setEnabled( false );
        ui->disconnectPushButton              ->setEnabled( true );

        if( ui->localVizRadioButton->isChecked() )                   *m_viz_mode = Viz::Mode::LocalClientAndServer;
        else if( ui->remoteVizClientServerRadioButton->isChecked() ) *m_viz_mode = Viz::Mode::RemoteClientAndServer;
        else if( ui->remoteVizInsituRadioButton->isChecked() )       *m_viz_mode = Viz::Mode::RemoteInSitu;

        SamplingType samplingType;
        if( ui->uniformRadioButton->isChecked() )         samplingType = SamplingType::Uniform;
        else if( ui->metropolisRadioButton->isChecked() ) samplingType = SamplingType::Metropolis;
        else if( ui->rejectionRadioButton->isChecked() )  samplingType = SamplingType::Rejection;

        QString pointObjectUUID = QUuid::createUuid().toString( QUuid::WithoutBraces );
        QString glyphObjectUUID = QUuid::createUuid().toString( QUuid::WithoutBraces );

        const bool is_client_server_mode = ui->localVizRadioButton->isChecked() || ui->remoteVizClientServerRadioButton->isChecked();

        const ObjectInfoExtractor::Format pointObjectFormat =
            is_client_server_mode ?
                ObjectInfoExtractor::Format::ClientServerPointObject :
                ObjectInfoExtractor::Format::InsituServerPointObject;

        const ObjectInfoExtractor::Format glyphObjectFormat = ObjectInfoExtractor::Format::ServerGlyphObject;

        QJsonArray uuidArray;
        uuidArray.append( pointObjectUUID );
        uuidArray.append( glyphObjectUUID );

        QJsonArray formatArray;
        formatArray.append( pointObjectFormat );
        formatArray.append( glyphObjectFormat );

        m_web_sockets->text()->sendTextMessage(
            QJsonDocument( {
                           { QString::fromUtf8( Protocol::Key::Event)                    , QString::fromUtf8(Protocol::Events::Initialize) },
                           { "VizMode"                                                   , static_cast<int>( *m_viz_mode ) },
                           { "SamplingType"                                              , static_cast<int>( samplingType ) },
                           { QString::fromUtf8( Protocol::Key::VolumeDataFilePath )      , ui->volumeDataFilePathLineEdit->text() },
                           { QString::fromUtf8( Protocol::Key::TransferFunctionFilePath ), ui->transferFunctionFilePathLineEdit->text() },
                           { QString::fromUtf8( Protocol::Key::UUID )                    , uuidArray },
                           { QString::fromUtf8( Protocol::Key::Format )                  , formatArray },
                           } ).toJson( QJsonDocument::Compact )
            );
    }
}

void Communication::onBinaryWebSocketConnected()
{
    webSocketConnected();
}

void Communication::onBinaryWebSocketDisconnected()
{
    webSocketDisconnected();
}

void Communication::onBinaryWebSocketMessageReceived( const QByteArray& binary )
{
    emit unpack( binary );
}

void Communication::onTextWebSocketConnected()
{
    webSocketConnected();
}

void Communication::onTextWebSocketDisconnected()
{
    webSocketDisconnected();
}

void Communication::onTransferOperator()
{
    if( !m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( k_not_connected_text );
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
                                                          { QString::fromUtf8( Protocol::Key::Event )   , QString::fromUtf8( Protocol::Events::TransferOperator )},
                                                          { QString::fromUtf8( Protocol::Key::TargetID ), targetID },
                                                          } ).toJson( QJsonDocument::Compact ) );
}

void Communication::onSendChatMessage()
{
    if( !m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( k_not_connected_text );
        return;
    }

    QString text = ui->chatLineEdit->text().trimmed();
    if( text.isEmpty() ) return;

    m_web_sockets->text()->sendTextMessage( QJsonDocument( {
                                                          { QString::fromUtf8( Protocol::Key::Event ), QString::fromUtf8( Protocol::Events::Chat ) },
                                                          { QString::fromUtf8( Protocol::Key::Text ) , text },
                                                          } ).toJson( QJsonDocument::Compact ) );
    ui->chatLineEdit->clear();
}

void Communication::onShareView()
{
    if( !m_web_sockets->isConnected() )
    {
        emit updateStatusBarMessage( k_not_connected_text );
        return;
    }

    kvs::Xform currentXform     = m_screen->scene()->objectManager()->xform();
    const kvs::Matrix44f matrix = currentXform.toMatrix();

    QJsonArray matrixArray;
    for( int row = 0; row < 4; ++row )
    {
        QJsonArray rowArray;
        for( int col = 0; col < 4; ++col )
        {
            rowArray.append( matrix[row][col] );
        }
        matrixArray.append( rowArray );
    }

    m_web_sockets->text()->sendTextMessage( QJsonDocument( {
                                                          { QString::fromUtf8( Protocol::Key::Event ) , QString::fromUtf8( Protocol::Events::ShareView ) },
                                                          { QString::fromUtf8( Protocol::Key::Matrix ), matrixArray },
                                                          } ).toJson( QJsonDocument::Compact ) );
}

void Communication::onTextWebSocketMessageReceived( const QString& receivedMessage )
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson( receivedMessage.toUtf8(), &parseError );

    if( parseError.error != QJsonParseError::NoError )
    {
        emit updateStatusBarMessage( tr( "Invalid JSON received. (%1)" ).arg( parseError.errorString() ) );
        return;
    }

    if( !doc.isObject() )
    {
        emit updateStatusBarMessage( tr( "Invalid message format received." ) );
        return;
    }

    const QJsonObject obj = doc.object();

    const QString kEvent = QString::fromUtf8( Protocol::Key::Event );
    if( !obj.contains( kEvent ) )
    {
        emit updateStatusBarMessage( tr( "Unknown event received. Please check that the client and server versions match." ) );
        return;
    }

    // NOTE:以下でイベント判定を行う
    const QString event = obj.value( kEvent ).toString();
    if( event == "Template" ) qDebug() << __LINE__;
    else if( event == QString::fromUtf8( Protocol::Events::Join ) )             receiveJoin( obj );
    else if( event == QString::fromUtf8( Protocol::Events::Left ) )             receiveLeft( obj );
    else if( event == QString::fromUtf8( Protocol::Events::ID ) )               receiveID( obj );
    else if( event == QString::fromUtf8( Protocol::Events::Operator ) )         receiveOperator( obj );
    else if( event == QString::fromUtf8( Protocol::Events::TransferOperator ) ) receiveTransferOperator( obj );
    else if( event == QString::fromUtf8( Protocol::Events::Chat ) )             receiveChat( obj );
    else if( event == QString::fromUtf8( Protocol::Events::ShareView ) )        receiveShareView( obj );
    else if( event == QString::fromUtf8( Protocol::Events::SharePoint ) )       receiveSharePoint( obj );
    else if( event == QString::fromUtf8( Protocol::Events::Initialize ) )
    {
        const QString kGlyphParameter            = QString::fromUtf8( Protocol::Key::GlyphParameter );
        const QString kPlotOverLineParameter     = QString::fromUtf8( Protocol::Key::PlotOverLineParameter );
        const QString kTransferFunctionParameter = QString::fromUtf8( Protocol::Key::TransferFunctionParameter );
        if( obj.contains( kGlyphParameter ) && obj.value( kGlyphParameter ).isObject() )
        {
            const QJsonObject glyphParameter = obj.value( kGlyphParameter ).toObject();
            emit receiveGlyphParameter( glyphParameter );
        }
        if( obj.contains( kPlotOverLineParameter )     && obj.value( kPlotOverLineParameter ).isObject() )
        {
            const QJsonObject plotOverLineParameter = obj.value( kPlotOverLineParameter ).toObject();
            emit receivePlotOverLineParameter( plotOverLineParameter );
        }
        if( obj.contains( kTransferFunctionParameter ) && obj.value( kTransferFunctionParameter ).isObject() )
        {
            const QJsonObject transferFunctionParameter = obj.value( kTransferFunctionParameter ).toObject();
            emit receiveTransferFunctionParameter( transferFunctionParameter );
        }
    }
    else if( event == QString::fromUtf8( Protocol::Events::RequestDataAt ) )
    {
        const QString kPlotOverLineParameter     = QString::fromUtf8( Protocol::Key::PlotOverLineParameter );
        const QString kTransferFunctionParameter = QString::fromUtf8( Protocol::Key::TransferFunctionParameter );
        if( obj.contains( kPlotOverLineParameter ) && obj.value( kPlotOverLineParameter ).isObject() )
        {
            const QJsonObject plotOverLineParameter = obj.value( kPlotOverLineParameter ).toObject();
            emit receivePlotOverLineParameter( plotOverLineParameter );
        }
        if( obj.contains( kTransferFunctionParameter ) && obj.value( kTransferFunctionParameter ).isObject() )
        {
            const QJsonObject transferFunctionParameter = obj.value( kTransferFunctionParameter ).toObject();
            emit receiveRequestDataAtTransferFunctionParameter( transferFunctionParameter );
        }
    }
    else if( event == QString::fromUtf8( Protocol::Events::TimeStepControlParameter ) )  emit receiveTimeStepControlParameter( obj );
    else if( event == QString::fromUtf8( Protocol::Events::GlyphParameter ) )            emit receiveGlyphParameter( obj );
    else if( event == QString::fromUtf8( Protocol::Events::ObjectInfoParameter ) )       emit receiveObjectInfoParameter( obj );
    else if( event == QString::fromUtf8( Protocol::Events::PlotOverLineParameter ) )     emit receivePlotOverLineParameter( obj );
    else if( event == QString::fromUtf8( Protocol::Events::TransferFunctionParameter ) ) emit receiveTransferFunctionParameter( obj );
    else if( event == QString::fromUtf8( Protocol::Events::SelectedFile ) )              emit receiveSelectedFile( obj );
    else if( event == QString::fromUtf8( Protocol::Events::ObjectDelete ) )              emit receiveObjectDelete( obj );
    else if( event == QString::fromUtf8( Protocol::Events::LatestTimeStep ) )            emit updateObjectLatestTimeStep( obj );
}
