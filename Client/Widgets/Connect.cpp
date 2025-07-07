#include "Connect.h"
#include "ui_Connect.h"

Connect::Connect( kvs::qt::jaea::Screen* screen,
                  QWidget *parent )
    : QDialog(parent)
    , ui(new Ui::Connect)
    , m_screen( screen )
    , m_client_message()
    , m_server_message()
    , m_received_message()
{
    initialize();
}

Connect::~Connect()
{
    delete ui;
}

void Connect::initialize()
{
    ui->setupUi(this);
    this->setFixedSize( this->sizeHint() );
    ui->uniformRadioButton->setChecked( true );

    // パーティクルリミット、デンシティ、データサイズリミットを初期化
    m_client_message.m_particle_limit = 10000000;
    m_client_message.m_particle_density = 1;
    m_client_message.m_particle_data_size_limit = 20;

    m_glyph_scale_factor = 1;
    m_glyph_type = GlyphItem::GlyphType::Arrow;
    m_client_message.m_distribution_mode = jpv::GlyphMode::UniformDistribution;
    m_client_message.m_color_data_sampling_method=jpv::DataDefines::Constant;
    m_client_message.m_size_sampling_method=jpv::DataDefines::Constant;
    m_client_message.m_stride = 5;
    m_client_message.m_seed = 5;
    m_client_message.m_number_of_sampling_point =100;
    m_client_message.m_direction_variable[0] = "q1";
    m_client_message.m_direction_variable[1] = "q2";
    m_client_message.m_direction_variable[2] = "q3";
    m_client_message.m_color_data_variables.push_back("q1");
    m_client_message.m_size_variables.push_back("q1");

    m_is_plot_over_line = false;
    m_client_message.m_sampling_size = 256;
    m_client_message.m_start_point[0] = -1.f;
    m_client_message.m_start_point[1] = 0;
    m_client_message.m_start_point[2] = 0;
    m_client_message.m_end_point[0] = 1.f;
    m_client_message.m_end_point[1] = 0;
    m_client_message.m_end_point[2] = 0;
    m_client_message.m_plot_variable = "q1";

    connect( ui->cancelPushButton, &QPushButton::clicked, this, &Connect::close );
    connect( ui->connectPushButton, &QPushButton::clicked, this, &Connect::onConnect );
    connect( ui->volumeDataFilePathPushButton, &QPushButton::clicked, this, &Connect::onVolumeDataBrowse );
    connect( ui->transferFunctionFilePathPushButton, &QPushButton::clicked, this, &Connect::onTransferFunctionBrowse );
}

void Connect::connectServer()
{
    jpv::ParticleTransferClient client( "localhost", ui->portSpinBox->value() );
    m_server_message.m_camera = new kvs::Camera();
    m_client_message.m_camera = m_screen->scene()->camera();

    if( m_server_mode == SERVERMODE::ClientServer )
    {
        QString filePath = ui->volumeDataFilePathLineEdit->text();
        QStringList validExtensions = { ".pfi", ".pfl", ".stl", ".vtp", ".xyz", ".vtr",
                                        ".vtk", ".vti", ".vts", ".pvts", ".inp", ".vtu",
                                        ".pvtu", ".vtm", ".case" };

        bool isValid = false;
        for( const QString &text : validExtensions )
        {
            if( filePath.endsWith( text, Qt::CaseInsensitive ) )
            {
                isValid = true;
                break;
            }
        }

        if( !isValid )
        {
            QMessageBox::information( this, tr( "Connection Error" ), tr( "The file path does not have a valid extension." ) );
            return;
        }
    }

    int init = client.initClient();
    if( init < 0 )
    {
        QMessageBox::information( this, tr( "Connection Error" ), tr( "The connection to the server failed. Please verify if the server is up and running." ) );
        return;
    }

    strncpy( m_client_message.m_header, "JPTP /1.0\r\n", 11 );

    m_client_message.m_input_directory = Utils::toNativePath( ui->volumeDataFilePathLineEdit->text() );

    // { // 伝達関数のスタブ
    //     m_client_message.m_import_flag = 0;
    //     m_client_message.m_transfer_function.clear();
    //     m_client_message.m_volume_equation.clear();

    //     m_client_message.m_color_transfer_function_synthesis = "C1";
    //     m_client_message.m_opacity_transfer_function_synthesis = "O1";

    //     NamedTransferFunctionParameter etf;
    //     jpv::ParticleTransferClientMessage::VolumeEquation volumeEquationColor, volumeEquationOpacity;

    //     std::stringstream ss;
    //     ss << "_F" << 0;
    //     etf.m_color_variable   = ss.str() + "_VAR_C";
    //     // etf.setColorMap( m_parameter.getTransferFunction(0)->colorMap() );
    //     etf.m_color_variable_min = 0.0;
    //     etf.m_color_variable_max = 1.0;
    //     volumeEquationColor.m_name = etf.m_color_variable;
    //     volumeEquationColor.m_equation = "q1";

    //     etf.m_opacity_variable   = ss.str() + "_VAR_O";
    //     // etf.setOpacityMap( m_parameter.getTransferFunction(0)->opacityMap() );
    //     etf.m_opacity_variable_min = 0.0;
    //     etf.m_opacity_variable_max = 1.0;
    //     volumeEquationOpacity.m_name = etf.m_opacity_variable;
    //     volumeEquationOpacity.m_equation = "q1";

    //     m_client_message.m_transfer_function.push_back( etf );
    //     m_client_message.m_volume_equation.push_back( volumeEquationColor );
    //     m_client_message.m_volume_equation.push_back( volumeEquationOpacity );
    // }

    emit importTransferFunctionFromFile( m_transfer_function_file_path );
    // m_client_message.m_import_flag = m_transfer_function_editor->importTransferFunctionFromFile( ui->transferFunctionFilePathLEdit->text().replace( "/","\\" ).toLocal8Bit().constData() ); // DONE

    // m_client_message.m_import_flag = m_transfer_function_editor->importTransferFunctionFromFile( ui->transferFunctionFilePathLEdit->text().toStdString() ); // DONE

// #endif
    emit applyTransferFunction();
    // m_transfer_function_editor->apply(); // DONE
    m_client_message.m_camera ->setWindowSize( m_screen->width() , m_screen->height() );


    //m_client_message.m_initialize_parameter = -3;
    m_client_message.m_initialize_parameter = jpv::InitializeParameter::initial_step;


    m_client_message.show();
    m_client_message.m_message_size = m_client_message.byteSize();
    client.sendMessage( m_client_message );

    if( m_server_mode == SERVERMODE::ClientServer )
    {
        client.recvMessage( &m_server_message );
        // std::cout << "file_enable_flag =" << static_cast<int>( m_server_message.m_file_enable_flag ) << std::endl;
        bool file_flag = false;
        if( m_server_message.m_file_enable_flag == jpv::FileEnableFlag::NotEnable_VTK )
        {
            file_flag = true;
            // std::cout << "Not available VTK file. Please check Server setting" << std::endl;
            QMessageBox::information( this, tr( "Not available VTK file." ), tr( "Not available VTK file. Please check Server setting" ) );
            //　サーバー側のコンパイル設定を変更するよう促すホップアップが欲しい
        }
        if( m_server_message.m_file_enable_flag == jpv::FileEnableFlag::NoFile )
        {
            file_flag = true;
            // std::cout << "Not find file. Please check file path" << std::endl;
            //　パス名を変更するよう促すホップアップが欲しい
            QMessageBox::information( this, tr( "Not available VTK file." ), tr( "Not find file. Please check file path" ) );
        }

        if( file_flag )
        {
            // std::cout << "Find No File!!" << std::endl;
            m_client_message.m_initialize_parameter = jpv::InitializeParameter::connection_reset;
            m_client_message.m_message_size = m_client_message.byteSize();
            client.sendMessage( m_client_message );
            client.recvMessage( &m_server_message );
            client.termClient();
            return;
        }
    }

    client.recvMessage( &m_server_message );
    m_server_message.show();

    if( m_server_mode == SERVERMODE::InSitu )
    {
        // m_render_options->updateParticleLimit(); // AFTER
    }

    int number_variable = m_server_message.m_number_ingredients ;
    //ヒストグラム更新用(CS, IS)
    m_received_message.m_var_range.merge( m_server_message.m_server_side_variable_range );
    m_received_message.m_color_bins.resize( m_server_message.m_transfer_function_count );
    m_received_message.m_opacity_bins.resize( m_server_message.m_transfer_function_count );

    for( int tf = 0; tf < m_server_message.m_transfer_function_count; tf++ )
    {
        char color_function_name[8] = {0x00};
        char opacity_function_name[8] = {0x00};
        sprintf( color_function_name, "C%d", tf+1 );
        sprintf( opacity_function_name, "O%d", tf+1 );
        if( m_server_message.m_color_nbins[tf] > 0 )
        {
            m_received_message.m_color_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_color_nbins[tf], (size_t *)m_server_message.m_color_bins[tf], std::string(color_function_name) );
        }
        if( m_server_message.m_opacity_nbins[tf] >0 )
        {
            m_received_message.m_opacity_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_opacity_nbins[tf],(size_t *) m_server_message.m_opacity_bins[tf], std::string(opacity_function_name) );
        }
    }

    {
        emit importTransferFunctionFromServer( m_server_message, m_received_message, m_client_message.m_import_flag );
        emit applyTransferFunction();
        // m_transfer_function_editor->importTransferFunctionFromServer(); // DONE
    }

    // ローカル変数に一時保持
    // サーバーポイントオブジェクト用(CS)
    ServerPointObjectPropertiesCS serverPointObjectPropertiesCS;
    serverPointObjectPropertiesCS.numberOfIngredients     = m_server_message.m_number_ingredients;
    serverPointObjectPropertiesCS.numberOfElements        = m_server_message.m_number_elements;
    serverPointObjectPropertiesCS.numberOfVolumeDivide    = m_server_message.m_number_volume_divide;
    serverPointObjectPropertiesCS.elementType             = m_server_message.m_element_type;
    serverPointObjectPropertiesCS.numberOfNodes           = m_server_message.m_number_nodes;
    serverPointObjectPropertiesCS.numberOfStep            = m_server_message.m_number_step;
    serverPointObjectPropertiesCS.fileType                = m_server_message.m_file_type;
    serverPointObjectPropertiesCS.minObjectCoords         = kvs::Vec3f( m_server_message.m_min_object_coord );
    serverPointObjectPropertiesCS.maxObjectCoords         = kvs::Vec3f( m_server_message.m_max_object_coord );

    // サーバーグリフオブジェクト用(CS)
    ServerGlyphObjectPropertiesCS serverGlyphObjectPropertiesCS;
    serverGlyphObjectPropertiesCS.minObjectCoords         = kvs::Vec3f( m_server_message.m_min_object_coord );
    serverGlyphObjectPropertiesCS.maxObjectCoords         = kvs::Vec3f( m_server_message.m_max_object_coord );

    // サーバーポイントオブジェクト用(InSitu)
    ServerPointObjectPropertiesIS serverPointObjectPropertiesIS;
    serverPointObjectPropertiesIS.numberOfIngredients   = m_server_message.m_number_ingredients;
    serverPointObjectPropertiesIS.particleLimit         = m_server_message.m_particle_limit;
    serverPointObjectPropertiesIS.particleDensity       = m_server_message.m_particle_density;
    serverPointObjectPropertiesIS.minObjectCoords       = kvs::Vec3f( m_server_message.m_min_object_coord );
    serverPointObjectPropertiesIS.maxObjectCoords       = kvs::Vec3f( m_server_message.m_max_object_coord );

    // サーバーグリフオブジェクト用(CS)
    ServerGlyphObjectPropertiesIS serverGlyphObjectPropertiesIS;
    serverGlyphObjectPropertiesIS.minObjectCoords         = kvs::Vec3f( m_server_message.m_min_object_coord );
    serverGlyphObjectPropertiesIS.maxObjectCoords         = kvs::Vec3f( m_server_message.m_max_object_coord );

    emit updateNumberOfVector( m_server_message.m_number_ingredients );
    // m_filter_infomation->updateFilterInfomation( ui->volumeDataFilePathLEdit->text(), m_server_message ); // DONE
    // m_glyph_editor->updateNumberOfVector( m_server_message ); // DONE
    // m_plot_over_line->updateNumberOfVector( m_server_message ); // DONE

    strncpy( m_client_message.m_header, "JPTP /1.0\r\n", 11 );

    m_client_message.m_initialize_parameter = jpv::InitializeParameter::connection_reset;
    m_client_message.m_message_size = m_client_message.byteSize();
    client.sendMessage( m_client_message );
    client.recvMessage( &m_server_message );
    client.termClient();

    if( m_server_mode == SERVERMODE::ClientServer )
    {
        // m_merge->serverPointObjectCS( ui->volumeDataFilePathLineEdit->text(), m_server_message.m_start_step, m_server_message.m_last_step ); // DONE
        serverPointObjectPropertiesCS.minTimeStep         = m_server_message.m_start_step;
        serverPointObjectPropertiesCS.maxTimeStep         = m_server_message.m_last_step;
        emit serverPointObjectCS( ui->volumeDataFilePathLineEdit->text(), serverPointObjectPropertiesCS );
        if( number_variable < 3  )
        {
        }
        else
        {
            serverGlyphObjectPropertiesCS.minTimeStep         = m_server_message.m_start_step;
            serverGlyphObjectPropertiesCS.maxTimeStep         = m_server_message.m_last_step;
            emit serverGlyphObjectCS( ui->volumeDataFilePathLineEdit->text(), serverGlyphObjectPropertiesCS );
            // m_merge->serverGlyphObjectCS( ui->volumeDataFilePathLineEdit->text(), m_server_message.m_start_step, m_server_message.m_last_step ); // DONE
        }

        if( m_client_message.m_import_flag )
        {
#ifdef Q_OS_WIN
            // emit importTransferFunctionFromFile( m_transfer_function_file_path ); // おそらくいらない?
            // m_transfer_function_editor->importTransferFunctionFromFile( ui->transferFunctionFilePathLineEdit->text().replace( "/","\\" ).toLocal8Bit().constData() ); // DONE
#else
            // m_transfer_function_editor->importTransferFunctionFromFile( ui->transferFunctionFilePathLineEdit->text().toStdString() ); // DONE
#endif
            // emit applyTransferFunction(); // おそらくいらない?
            // m_transfer_function_editor->apply(); // DONE
        }
    }
    else if( m_server_mode == SERVERMODE::InSitu )
    {
        serverPointObjectPropertiesIS.minTimeStep         = 0;
        serverPointObjectPropertiesIS.maxTimeStep         = 0;
        emit serverPointObjectIS( "IS-PointObject", serverPointObjectPropertiesIS );
        // m_merge->serverPointObjectIS( "IS-PointObject", 0, 0 ); // AFTER
        if( number_variable < 3  )
        {
        }
        else
        {
            m_has_insitu_object = true;
            serverGlyphObjectPropertiesIS.minTimeStep         = 0;
            serverGlyphObjectPropertiesIS.maxTimeStep         = 0;
            emit serverGlyphObjectIS( "IS-GlyphObject", serverGlyphObjectPropertiesIS );
            // m_merge->serverGlyphObjectIS( "IS-GlyphObject", 0, 0 );  // AFTER
        }

    }
    delete m_server_message.m_camera;
    ui->connectPushButton->setDisabled( true );
}

void Connect::sendGlyphFlagFalse()
{
    while( m_connecting )
    {
        //qInfo() << "Other conneciton mode working !! waiting...    ";
    }

    m_connecting = true;
    if( m_server_mode == SERVERMODE::ClientServer )
    {
        qInfo() <<  "this botton doesn't work in CS_MODE !!!";
    }
    else if( m_server_mode == SERVERMODE::InSitu )
    {
        jpv::ParticleTransferClient client( "localhost", ui->portSpinBox->value() );
        m_server_message.m_camera = new kvs::Camera();
        client.initClient();
        strncpy( m_client_message.m_header, "JPTP /1.0\r\n", 11 );
        m_client_message.m_initialize_parameter = jpv::InitializeParameter::send_glyph_flag_false; // = 2
        m_client_message.m_rendering_id = 0;
        if( ui->uniformRadioButton->isChecked() == true )    { m_client_message.m_sampling_method = 'u'; }
        if( ui->metropolisRadioButton->isChecked() == true ) { m_client_message.m_sampling_method = 'm'; }
        if( ui->rejectionRadioButton->isChecked() == true )  { m_client_message.m_sampling_method = 'r'; }
        m_client_message.m_subpixel_level = 2;
        m_client_message.m_repeat_level = 16;
        m_client_message.m_shuffle_method = 'r';
        m_client_message.m_time_parameter = 2;
        m_client_message.m_trans_parameter = 2;
        m_client_message.m_node_type = 'a';
        m_client_message.m_camera = m_screen->scene()->camera();//足りないかも
        m_client_message.m_step = 0;
        m_client_message.m_message_size = m_client_message.byteSize();
        m_client_message.m_sampling_step = 1.0f;
        m_client_message.m_enable_crop_region = 0;
        m_client_message.m_glyph_flag =false;

        m_client_message.m_message_size = m_client_message.byteSize();
        // TF情報をサーバー側に送信　（サーバーからの受信はしない）
        client.sendMessage( m_client_message );

        m_client_message.m_initialize_parameter = jpv::InitializeParameter::connection_reset;
        m_client_message.m_message_size = m_client_message.byteSize();
        client.sendMessage( m_client_message );
        client.recvMessage( &m_server_message );

        client.termClient();
    }
    m_connecting = false;
}

kvs::PointObject* Connect::generateParticles( int timeStep )
{
    if( m_connecting )
    {
        qInfo() << "Other conneciton mode working !!";
        kvs::PointObject* object = new kvs::PointObject();
        return  object;
    }
    else
    {
        m_connecting = true;
        qDebug() << "generateParticles";

        jpv::ParticleTransferClient client( "localhost", ui->portSpinBox->value() );
        m_server_message.m_camera = new kvs::Camera();
        client.initClient();
        strncpy( m_client_message.m_header, "JPTP /1.0\r\n", 11 );
        m_client_message.m_initialize_parameter = jpv::InitializeParameter::generate_particle;
        //m_client_message.m_initialize_parameter = 1;
        m_client_message.m_rendering_id = 0;
        if( ui->uniformRadioButton->isChecked() == true )    { m_client_message.m_sampling_method = 'u'; }
        if( ui->metropolisRadioButton->isChecked() == true ) { m_client_message.m_sampling_method = 'm'; }
        if( ui->rejectionRadioButton->isChecked() == true )  { m_client_message.m_sampling_method = 'r'; }
        m_client_message.m_subpixel_level = 2;
        m_client_message.m_repeat_level = 16;
        m_client_message.m_shuffle_method = 'r';
        m_client_message.m_time_parameter = 2;
        m_client_message.m_trans_parameter = 2;
        m_client_message.m_node_type = 'a';
        m_client_message.m_camera = m_screen->scene()->camera();//足りないかも
        m_client_message.m_step = timeStep;
        m_client_message.m_message_size = m_client_message.byteSize();
        m_client_message.m_sampling_step = 1.0f;
        m_client_message.m_enable_crop_region = 0;
        m_client_message.m_message_size = m_client_message.byteSize();
        client.sendMessage( m_client_message );
        client.recvMessage( &m_server_message );

        size_t allParticle = 0;
        kvs::PointObject* object = new kvs::PointObject();
        int serve_numvol = m_server_message.m_number_volume_divide;

        for( int n = 0; n < serve_numvol; n++ )
        {
            if( client.recvMessage( &m_server_message ) == 1 ){}

            int nmemb = m_server_message.m_number_particle * 3;
            if( nmemb != 0 )
            {
                kvs::ValueArray<kvs::Real32> positions ( m_server_message.m_positions.get(), nmemb );
                kvs::ValueArray<kvs::Real32> normals ( m_server_message.m_normals.get(), nmemb );
                kvs::ValueArray<kvs::UInt8>  colors ( m_server_message.m_colors.get(), nmemb );

                kvs::PointObject obj;
                obj.setCoords( positions );
                obj.setNormals( normals );
                obj.setColors( colors );

                object->add(obj);
                obj.clear();
                allParticle = allParticle + m_server_message.m_number_particle;
            }
        }

        kvs::PointObject* pointObject = object;

        kvs::Vector3f serverSideMinObjectCoords;
        kvs::Vector3f serverSideMaxObjectCoords;
        serverSideMinObjectCoords[0] = m_server_message.m_min_object_coord[0];
        serverSideMinObjectCoords[1] = m_server_message.m_min_object_coord[1];
        serverSideMinObjectCoords[2] = m_server_message.m_min_object_coord[2];
        serverSideMaxObjectCoords[0] = m_server_message.m_max_object_coord[0];
        serverSideMaxObjectCoords[1] = m_server_message.m_max_object_coord[1];
        serverSideMaxObjectCoords[2] = m_server_message.m_max_object_coord[2];

        int numberOfVector = m_server_message.m_number_ingredients;
        float particleLimit = m_server_message.m_particle_limit;
        float particleDensity = m_server_message.m_particle_density;

        if( m_server_mode == SERVERMODE::InSitu )
        {
            if( numberOfVector != 0 )
            {
                std::cout << m_server_message.m_number_ingredients << std::endl;
                std::cout << serverSideMinObjectCoords << std::endl;
                std::cout << serverSideMaxObjectCoords << std::endl;
                if( !m_has_insitu_object )
                {
                    emit updateNumberOfVector( numberOfVector );
                    if( numberOfVector < 3  )
                    {
                    }
                    else
                    {
                        ServerGlyphObjectPropertiesIS serverGlyphObjectPropertiesIS;
                        serverGlyphObjectPropertiesIS.minObjectCoords = serverSideMinObjectCoords;
                        serverGlyphObjectPropertiesIS.maxObjectCoords = serverSideMaxObjectCoords;
                        serverGlyphObjectPropertiesIS.minTimeStep         = m_server_message.m_start_step;
                        serverGlyphObjectPropertiesIS.maxTimeStep         = m_server_message.m_last_step;
                        emit serverGlyphObjectIS( "IS-GlyphObject", serverGlyphObjectPropertiesIS );
                        // m_merge->serverGlyphObjectIS( "IS-GlyphObject", 0, 0 );  // AFTER
                    }

                    emit insituObjectActive( serverSideMinObjectCoords, serverSideMaxObjectCoords, numberOfVector, particleLimit, particleDensity );
                }
                m_has_insitu_object = true;
            }
        }
        pointObject->setMinMaxObjectCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );
        pointObject->setMinMaxExternalCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );

        bool flag = false;

        if( m_server_message.m_flag_send_bins == 1  && m_server_mode == SERVERMODE::InSitu )
        {
            flag = true;
        }

        m_client_message.m_initialize_parameter = jpv::InitializeParameter::connection_reset;
        m_client_message.m_message_size = m_client_message.byteSize();
        client.sendMessage( m_client_message );
        client.recvMessage( &m_server_message );

        client.termClient();

        //ここでサーバのレンジが手に入る。
        // std::cout << m_server_message.m_server_side_variable_range.min( "t1_var_c" ) << std::endl;
        // std::cout << m_server_message.m_server_side_variable_range.max( "t1_var_c" ) << std::endl;
        // std::cout << m_server_message.m_server_side_variable_range.min( "t1_var_o" ) << std::endl;
        // std::cout << m_server_message.m_server_side_variable_range.max( "t1_var_o" ) << std::endl;

        //ヒストグラム更新用(CS, IS)
        m_received_message.m_var_range.merge( m_server_message.m_server_side_variable_range );
        m_received_message.m_color_bins.resize( m_server_message.m_transfer_function_count );
        m_received_message.m_opacity_bins.resize( m_server_message.m_transfer_function_count );
        for( int tf = 0; tf < m_server_message.m_transfer_function_count; tf++ )
        {
            char color_function_name[8] = {0x00};
            char opacity_function_name[8] = {0x00};
            sprintf( color_function_name, "C%d", tf+1 );
            sprintf( opacity_function_name, "O%d", tf+1 );
            if( m_server_message.m_color_nbins[tf] > 0 )
            {
                m_received_message.m_color_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_color_nbins[tf], (size_t *)m_server_message.m_color_bins[tf], std::string( color_function_name ) );
            }
            if( m_server_message.m_opacity_nbins[tf] >0 )
            {
                m_received_message.m_opacity_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_opacity_nbins[tf],(size_t *) m_server_message.m_opacity_bins[tf], std::string( opacity_function_name ) );
            }
        }

        emit updateTransferFunctionRangeAndView( &m_server_message, m_received_message );
        // m_transfer_function_editor->updateRangeView(); // DONE

        if( m_server_message.m_camera )
        {
            delete m_server_message.m_camera;
            m_server_message.m_camera = nullptr;
        }

        if( m_server_message.m_color_nbins )
        {
            delete[] m_server_message.m_color_nbins;
            m_server_message.m_color_nbins = nullptr;
        }

        if( m_server_message.m_opacity_nbins )
        {
            delete[] m_server_message.m_opacity_nbins;
            m_server_message.m_opacity_nbins = nullptr;
        }

        if( !m_server_message.m_color_bins.empty() )
        {
            for( auto ptr : m_server_message.m_color_bins )
            {
                delete ptr;
            }
            m_server_message.m_color_bins.clear();
        }

        if( !m_server_message.m_opacity_bins.empty() )
        {
            for( auto ptr : m_server_message.m_opacity_bins )
            {
                delete ptr;
            }
            m_server_message.m_opacity_bins.clear();
        }

        if( m_server_mode == SERVERMODE::InSitu )
        {
            qInfo() << m_server_message.m_start_step << ", " << m_server_message.m_last_step;
            emit updateInSituTimeStep( m_server_message.m_start_step, m_server_message.m_last_step );
            // m_merge->updateObjectTimeStepIS( m_server_message.m_start_step, m_server_message.m_last_step ); // AFTER
        }

        m_connecting = false;

        if( m_is_plot_over_line )
        {
            sendRecvPlotOverLine( timeStep );
        }

        // if( m_plot_over_line->enable_flag() ) // DONE
        // {
        //     sendRecvPlotOverLine( timeStep );
        // }
        if( flag &&  m_server_mode == SERVERMODE::InSitu )
        {
            return nullptr;
        }

        return pointObject;
    }
}

kvs::PolygonObject* Connect::generateGlyphPolygons( int timeStep )
{
    // m_glyph_editor->disableGlyphUpdateButton(); // AFTER
    if( m_connecting )
    {
        qInfo() << "Other conneciton mode working !!";
        kvs::PolygonObject* object = new kvs::PolygonObject();
        return  object;
    }
    else
    {
        m_connecting = true;
        qDebug() << "generateGlyphPolygons";

        jpv::ParticleTransferClient client( "localhost", ui->portSpinBox->value() );
        m_server_message.m_camera = new kvs::Camera();
        client.initClient();
        strncpy( m_client_message.m_header, "JPTP /1.0\r\n", 11 );
        m_client_message.m_initialize_parameter = jpv::InitializeParameter::generate_glyph;
        m_client_message.m_rendering_id = 0;
        if( ui->uniformRadioButton->isChecked() == true )    { m_client_message.m_sampling_method = 'u'; }
        if( ui->metropolisRadioButton->isChecked() == true ) { m_client_message.m_sampling_method = 'm'; }
        if( ui->rejectionRadioButton->isChecked() == true )  { m_client_message.m_sampling_method = 'r'; }
        m_client_message.m_subpixel_level = 2;
        m_client_message.m_repeat_level = 16;
        m_client_message.m_shuffle_method = 'r';
        m_client_message.m_time_parameter = 2;
        m_client_message.m_trans_parameter = 2;
        m_client_message.m_node_type = 'a';
        m_client_message.m_camera = m_screen->scene()->camera();//足りないかも
        m_client_message.m_step = timeStep;
        m_client_message.m_message_size = m_client_message.byteSize();
        m_client_message.m_sampling_step = 1.0f;
        m_client_message.m_enable_crop_region = 0;
        m_client_message.m_glyph_flag =true;
        // m_client_message.m_glyph_color_min = m_glyph_editor->m_glyph_color_min; // DONE NO NEED?
        // m_client_message.m_glyph_color_max = m_glyph_editor->m_glyph_color_max; // DONE NO NEED?
        // m_client_message.m_glyph_size_min = m_glyph_editor->m_glyph_size_min; // DONE NO NEED?
        // m_client_message.m_glyph_size_max = m_glyph_editor->m_glyph_size_max; // DONE NO NEED?

        m_client_message.m_message_size = m_client_message.byteSize();
        client.sendMessage( m_client_message );
        client.recvMessage( &m_server_message );
        size_t allParticle = 0;

        kvs::PointObject* object = new kvs::PointObject();
        int serve_numvol = m_server_message.m_number_volume_divide;

        for( int n = 0; n < serve_numvol; n++ )
        {
            if( client.recvMessage( &m_server_message ) == 1 ){}

            int nmemb = m_server_message.m_number_glyph * 3;
            if( nmemb != 0 )
            {
                kvs::ValueArray<kvs::Real32> positions ( m_server_message.m_glyph_coords.get(), nmemb );
                kvs::ValueArray<kvs::Real32> vectors ( m_server_message.m_glyph_vectors.get(), nmemb );
                kvs::ValueArray<kvs::Real32> sizes ( m_server_message.m_glyph_sizes.get(), m_server_message.m_number_glyph );
                kvs::ValueArray<kvs::UInt8>  colors ( m_server_message.m_glyph_colors.get(), nmemb );

                kvs::PointObject obj;
                obj.setCoords( positions );
                obj.setNormals( vectors );
                obj.setSizes( sizes );
                obj.setColors( colors );
                object->add(obj);
                obj.clear();
                allParticle = allParticle + m_server_message.m_number_glyph;
            }
        }

        kvs::PolygonObject* polygonObject;
        kvs::PointObject* pointObject = object;

        kvs::Vector3f serverSideMinObjectCoords;
        kvs::Vector3f serverSideMaxObjectCoords;
        serverSideMinObjectCoords[0] = m_server_message.m_min_object_coord[0];
        serverSideMinObjectCoords[1] = m_server_message.m_min_object_coord[1];
        serverSideMinObjectCoords[2] = m_server_message.m_min_object_coord[2];
        serverSideMaxObjectCoords[0] = m_server_message.m_max_object_coord[0];
        serverSideMaxObjectCoords[1] = m_server_message.m_max_object_coord[1];
        serverSideMaxObjectCoords[2] = m_server_message.m_max_object_coord[2];
        pointObject->setMinMaxObjectCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );
        pointObject->setMinMaxExternalCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );

        int nvetices = pointObject->numberOfVertices();
        kvs::ValueArray<kvs::Real32> coords;
        coords.allocate(3 * nvetices);        // 3 * m_number_glyph
        kvs::ValueArray<kvs::Real32> directions;
        directions.allocate(3 * nvetices);
        kvs::ValueArray<kvs::Real32> sizes;
        sizes.allocate(nvetices);
        kvs::ValueArray<kvs::UInt8> colors;
        colors.allocate(3 * nvetices);

        for( int i = 0; i < nvetices; i++ )
        {
            // 座標を設定
            coords[3 * i + 0] = pointObject->coords()[3 * i + 0];
            coords[3 * i + 1] = pointObject->coords()[3 * i + 1];
            coords[3 * i + 2] = pointObject->coords()[3 * i + 2];

            // 法線を設定
            directions[3 * i + 0] = pointObject->normals()[3 * i + 0];
            directions[3 * i + 1] = pointObject->normals()[3 * i + 1];
            directions[3 * i + 2] = pointObject->normals()[3 * i + 2];

            // サイズを設定
            sizes[i] = pointObject->sizes()[i] * m_glyph_scale_factor;
            // sizes[i] = pointObject->sizes()[i] * m_glyph_editor->getScaleFactor() ; // DONE

            // 色を設定
            colors[3 * i + 0] = pointObject->colors()[3 * i + 0];
            colors[3 * i + 1] = pointObject->colors()[3 * i + 1];
            colors[3 * i + 2] = pointObject->colors()[3 * i + 2];
        }
        bool flag = false;

        if( m_server_message.m_flag_send_bins == 1 && m_server_mode == SERVERMODE::InSitu )
        {
            flag = true;
        }

        m_client_message.m_initialize_parameter = jpv::InitializeParameter::connection_reset;
        m_client_message.m_message_size = m_client_message.byteSize();
        client.sendMessage( m_client_message );
        client.recvMessage( &m_server_message );

        m_server_message.show();
        client.termClient();

        m_connecting = false;

        // m_glyph_editor->m_coords = coords; // DONE NO NEED?
        // m_glyph_editor->m_directions = directions; // DONE NO NEED?
        // m_glyph_editor->m_sizes = sizes; // DONE NO NEED?
        // m_glyph_editor->m_colors = colors; // DONE NO NEED?
        // m_glyph_editor->m_glyph_color_min =  m_server_message.m_glyph_color_min; // DONE NO NEED?
        // m_glyph_editor->m_glyph_color_max =  m_server_message.m_glyph_color_max; // DONE NO NEED?
        // m_glyph_editor->m_glyph_size_min =  m_server_message.m_glyph_size_min; // DONE NO NEED?
        // m_glyph_editor->m_glyph_size_max =  m_server_message.m_glyph_size_max; // DONE NO NEED?

        polygonObject = new kvs::PolygonGlyphObject( coords, directions, sizes, colors, static_cast<kvs::PolygonGlyphObject::GlyphType>( m_glyph_type ) );
        // polygonObject = new kvs::PolygonGlyphObject( coords, directions, sizes, colors, static_cast<kvs::PolygonGlyphObject::GlyphType>(　m_glyph_editor->getGlyphType()　) ); // DONE
        polygonObject->setMinMaxObjectCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );
        polygonObject->setMinMaxExternalCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );

        if( m_server_mode == SERVERMODE::InSitu )
        {
            emit updateInSituTimeStep( m_server_message.m_start_step, m_server_message.m_last_step );
            // m_merge->updateObjectTimeStepIS( m_server_message.m_start_step, m_server_message.m_last_step ); // AFTER
        }
        // m_glyph_editor->enableGlyphUpdateButton(); // AFTER

        if( flag && m_server_mode == SERVERMODE::InSitu )
        {
            return nullptr;
        }

        return polygonObject;
    }
}

void Connect::sendRecvPlotOverLine( int timeStep )
{
    if( m_connecting )
    {
        qInfo() << "Other conneciton mode working !!";
        return ;
    }
    else
    {
        m_connecting = true;
        qDebug() << "sendRecvPlotOverLine";

        jpv::ParticleTransferClient client( "localhost", ui->portSpinBox->value() );
        m_server_message.m_camera = new kvs::Camera();
        client.initClient();
        strncpy( m_client_message.m_header, "JPTP /1.0\r\n", 11 );
        m_client_message.m_initialize_parameter = jpv::InitializeParameter::plot_over_line;
        m_client_message.m_rendering_id = 0;
        if( ui->uniformRadioButton->isChecked() == true )    { m_client_message.m_sampling_method = 'u'; }
        if( ui->metropolisRadioButton->isChecked() == true ) { m_client_message.m_sampling_method = 'm'; }
        if( ui->rejectionRadioButton->isChecked() == true )  { m_client_message.m_sampling_method = 'r'; }
        m_client_message.m_subpixel_level = 2;
        m_client_message.m_repeat_level = 16;
        m_client_message.m_shuffle_method = 'r';
        m_client_message.m_time_parameter = 2;
        m_client_message.m_trans_parameter = 2;
        m_client_message.m_node_type = 'a';
        m_client_message.m_camera = m_screen->scene()->camera();//足りないかも
        m_client_message.m_step = timeStep;
        std::cout << "timestep = " <<  timeStep <<std::endl;
        m_client_message.m_message_size = m_client_message.byteSize();
        m_client_message.m_sampling_step = 1.0f;
        m_client_message.m_enable_crop_region = 0;
        m_client_message.m_plot_flag =true;

        std::vector<bool> mask;

        m_client_message.m_message_size = m_client_message.byteSize();
        client.sendMessage( m_client_message );
        client.recvMessage( &m_server_message );
        int serve_numvol = m_server_message.m_number_volume_divide;

        client.recvMessage( &m_server_message );

        mask.resize(m_server_message.m_resolution);
        for( int i =0; i< m_server_message.m_resolution; i++ )
        {
            if( m_server_message.m_mask[i] == 1 )  mask[i] = true;
            else mask[i] = false;
        }

        m_client_message.m_initialize_parameter = jpv::InitializeParameter::connection_reset;
        m_client_message.m_message_size = m_client_message.byteSize();
        client.sendMessage( m_client_message );
        client.recvMessage( &m_server_message );

        m_server_message.show();
        client.termClient();

        m_connecting = false;

        emit setPlotData( m_server_message.m_xAxis, mask, m_server_message.m_line_values );
        // m_plot_over_line->setPlotData( m_server_message.m_xAxis, mask, m_server_message.m_line_values ); // DONE NO NEED?
    }
}

void Connect::deleteServerObject()
{
    ui->connectPushButton->setEnabled( true );
}

void Connect::failedTransferFunctionImport()
{
    m_client_message.m_import_flag = false;
}

void Connect::successTransferFunctionImport( const QString transferFunctionFile )
{
    m_client_message.m_import_flag = true;
    m_transfer_function_file_path = transferFunctionFile;
}

void Connect::updateTransferFunctionClientMessage( QString colorSynthesizer, QString opacitySynthesizer, QStandardItemModel* model )
{
    m_client_message.m_transfer_function.clear();
    m_client_message.m_volume_equation.clear();

    m_client_message.m_color_transfer_function_synthesis = colorSynthesizer.toUtf8().constData();
    m_client_message.m_opacity_transfer_function_synthesis = opacitySynthesizer.toUtf8().constData();

    for ( int i = 0; i < model->rowCount(); i++ )
    {
        QStandardItem* item = model->item(i);
        if ( !item ) continue;

        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation volumeEquationColor, volumeEquationOpacity;

        std::stringstream ss;
        ss << "_F" << i;

        etf.m_color_variable = ss.str() + "_VAR_C";
        int colorRangeStatus = item->data( TransferFunctionItem::TemporaryColorRangeMode ).toInt();
        if( colorRangeStatus == TransferFunctionItem::UserRange )
        {
            etf.m_color_variable_min = item->data( TransferFunctionItem::ColorUserRangeMin ).toFloat();
            etf.m_color_variable_max = item->data( TransferFunctionItem::ColorUserRangeMax ).toFloat();
        }
        else if( colorRangeStatus == TransferFunctionItem::ServerRange )
        {

            etf.m_color_variable_min = item->data( TransferFunctionItem::ColorServerRangeMin ).toFloat();
            etf.m_color_variable_max = item->data( TransferFunctionItem::ColorServerRangeMax ).toFloat();
        }
        volumeEquationColor.m_name = etf.m_color_variable;
        volumeEquationColor.m_equation = item->data( TransferFunctionItem::ColorVariable ).toString().toUtf8().constData();
        QVector<QColor> qcolors = item->data( TransferFunctionItem::ColorMap ).value<QVector<QColor>>();
        kvs::ValueArray<kvs::UInt8> table;
        table.allocate( qcolors.size() * 3 );
        for ( int i = 0; i < qcolors.size(); ++i )
        {
            const QColor& c = qcolors[i];
            table[3*i]     = static_cast<kvs::UInt8>( c.red() );
            table[3*i + 1] = static_cast<kvs::UInt8>( c.green() );
            table[3*i + 2] = static_cast<kvs::UInt8>( c.blue() );
        }
        kvs::ColorMap cmap( table );
        etf.setColorMap( cmap );

        etf.m_opacity_variable = ss.str() + "_VAR_O";
        int opacityRangeStatus = item->data( TransferFunctionItem::TemporaryOpacityRangeMode ).toInt();
        if( opacityRangeStatus == TransferFunctionItem::UserRange )
        {
            etf.m_opacity_variable_min = item->data( TransferFunctionItem::OpacityUserRangeMin ).toFloat();
            etf.m_opacity_variable_max = item->data( TransferFunctionItem::OpacityUserRangeMax ).toFloat();
        }
        else if( opacityRangeStatus == TransferFunctionItem::ServerRange )
        {
            etf.m_opacity_variable_min = item->data( TransferFunctionItem::OpacityServerRangeMin ).toFloat();
            etf.m_opacity_variable_max = item->data( TransferFunctionItem::OpacityServerRangeMax ).toFloat();
        }
        volumeEquationOpacity.m_name = etf.m_opacity_variable;
        volumeEquationOpacity.m_equation = item->data( TransferFunctionItem::OpacityVariable ).toString().toUtf8().constData();
        QVector<float> opacities = item->data( TransferFunctionItem::OpacityMap ).value<QVector<float>>();
        kvs::ValueArray<float> values( opacities.size() );
        for (int i = 0; i < opacities.size(); ++i)
        {
            values[i] = opacities[i];
        }
        kvs::OpacityMap omap( values );
        etf.setOpacityMap( omap );

        // 書き込み
        m_client_message.m_transfer_function.push_back( etf );
        m_client_message.m_volume_equation.push_back( volumeEquationColor );
        m_client_message.m_volume_equation.push_back( volumeEquationOpacity );
    }
}

void Connect::updateGlyphParameterClientMessage( QStandardItemModel* model )
{
    QStandardItem* root = model->item(0);

    QStringList directionList = root->data(GlyphItem::Direction).toStringList();

    m_glyph_type = root->data(GlyphItem::UserGlyphType).value<GlyphItem::GlyphType>();
    m_glyph_scale_factor = root->data(GlyphItem::ScaleFactor).toFloat();

    m_client_message.m_direction_variable[0] = directionList.at(0).toUtf8().constData();
    m_client_message.m_direction_variable[1] = directionList.at(1).toUtf8().constData();
    m_client_message.m_direction_variable[2] = directionList.at(2).toUtf8().constData();

    root->data( GlyphItem::SizeDataDefines ).toString();

    QString sizeData = root->data(GlyphItem::SizeDataDefines).toString();
    // sizeDataの内容に応じてm_size_sampling_methodを設定する例
    if( sizeData == "Constant" )
    {
        m_client_message.m_size_sampling_method = jpv::DataDefines::Constant;
    }
    else if( sizeData == "Variable" )
    {
        m_client_message.m_size_sampling_method = jpv::DataDefines::VariableArray;
        m_client_message.m_size_variables.clear();

        QStringList sizeVariableList = root->data(GlyphItem::SizeVariables).toStringList();
        if ( sizeVariableList.isEmpty() )
        {
            m_client_message.m_size_variables.push_back("q1");
        }
        else
        {
            for( const QString& var : sizeVariableList )
            {
                m_client_message.m_size_variables.push_back(var.toUtf8().constData());
            }
        }
    }
    else
    {
        m_client_message.m_size_sampling_method = jpv::DataDefines::Constant;
    }

    int distributionMode = root->data( GlyphItem::DistributionMode ).toInt();

    if( distributionMode == GlyphItem::DistributionMode::UniformDistribution )
    {
        m_client_message.m_distribution_mode = jpv::GlyphMode::UniformDistribution;

        int numberOfSamplingPoint = root->data( GlyphItem::NumberOfSamplePoints ).toInt();
        int seed = root->data( GlyphItem::Seed ).toInt();

        m_client_message.m_number_of_sampling_point = numberOfSamplingPoint;
        m_client_message.m_seed = seed;
    }
    else if( distributionMode == GlyphItem::DistributionMode::AllPoints )
    {
        m_client_message.m_distribution_mode = jpv::GlyphMode::AllPoints;
    }
    else if( distributionMode == GlyphItem::DistributionMode::EveryNthPoints )
    {
        m_client_message.m_distribution_mode = jpv::GlyphMode::EveryNthPoints;

        int stride = root->data( GlyphItem::Stride ).toInt();
        m_client_message.m_stride = stride;
    }

    QVector<QColor> qcolors = root->data( GlyphItem::ColorMap ).value<QVector<QColor>>();

    std::vector<int32_t> table;
    table.reserve( qcolors.size() * 3 );

    for( const QColor& color : qcolors )
    {
        table.push_back( static_cast<kvs::UInt8>( color.red() ) );
        table.push_back( static_cast<kvs::UInt8>( color.green() ) );
        table.push_back( static_cast<kvs::UInt8>( color.blue() ) );
    }

    m_client_message.m_glyph_color_map_table = table;

    QString colorData = root->data(GlyphItem::ColorDataDefines).toString();
    // ColorDataDataの内容に応じてm_ColorData_sampling_methodを設定する例
    if( colorData == "Constant" )
    {
        m_client_message.m_color_data_sampling_method = jpv::DataDefines::Constant;
    }
    else if( colorData == "Variable" )
    {
        m_client_message.m_color_data_sampling_method = jpv::DataDefines::VariableArray;
        m_client_message.m_color_data_variables.clear();

        QStringList colorDataVariableList = root->data(GlyphItem::ColorVariables).toStringList();
        if ( colorDataVariableList.isEmpty() )
        {
            m_client_message.m_color_data_variables.push_back("q1");
        }
        else
        {
            for( const QString& var : colorDataVariableList )
            {
                m_client_message.m_color_data_variables.push_back(var.toUtf8().constData());
            }
        }
    }
    else
    {
        m_client_message.m_color_data_sampling_method = jpv::DataDefines::Constant;
    }
}

void Connect::updatePlotOverLineParameterClientMessage( QStandardItemModel* model )
{
    QStandardItem* root = model->item(0);

    m_is_plot_over_line = root->data( PlotOverLineItem::IsEnable ).toBool();

    m_client_message.m_sampling_size = root->data(PlotOverLineItem::Resolution).toInt();
    m_client_message.m_plot_variable = root->data(PlotOverLineItem::Variable).toString().toUtf8();

    // QVector3D を取り出して start_point に代入
    const QVector3D start_point = root->data(PlotOverLineItem::StartPoints).value<QVector3D>();
    const QVector3D end_point = root->data(PlotOverLineItem::EndPoints).value<QVector3D>();

    m_client_message.m_start_point[0] = start_point.x();
    m_client_message.m_start_point[1] = start_point.y();
    m_client_message.m_start_point[2] = start_point.z();

    m_client_message.m_end_point[0] = end_point.x();
    m_client_message.m_end_point[1] = end_point.y();
    m_client_message.m_end_point[2] = end_point.z();
}

void Connect::updateRenderParameterClientMessage( QStandardItemModel* model )
{
    for( int row = 0; row < model->rowCount(); row++ )
    {
        QStandardItem* nameItem                 = model->item( row, 0 );
        QStandardItem* formatItem               = model->item( row, 1 );
        enum ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum  ObjectItem::Format>();
        switch( format )
        {
        case ObjectItem::Format::ServerPointObjectCS:
        case ObjectItem::Format::ServerPointObjectIS:
            m_client_message.m_particle_limit = nameItem->data( ObjectItem::nameItemRole::CurrentParticleLimit ).toInt();
            m_client_message.m_particle_density = nameItem->data( ObjectItem::nameItemRole::CurrentDensity ).toFloat();
        }
    }
}

void Connect::updateCoordinateParameterClientMessage( QStandardItemModel* model )
{
    for( int row = 0; row < model->rowCount(); row++ )
    {
        QStandardItem* nameItem                 = model->item( row, 0 );
        QStandardItem* formatItem               = model->item( row, 1 );
        enum  ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>();
        switch( format )
        {
        case ObjectItem::Format::ServerPointObjectCS:
            m_client_message.m_x_synthesis = nameItem->data( ObjectItem::nameItemRole::CurrentCoordinate1 ).toString().toUtf8().constData();
            m_client_message.m_y_synthesis = nameItem->data( ObjectItem::nameItemRole::CurrentCoordinate2 ).toString().toUtf8().constData();
            m_client_message.m_z_synthesis = nameItem->data( ObjectItem::nameItemRole::CurrentCoordinate3 ).toString().toUtf8().constData();
        }
    }
}

void Connect::onConnect()
{
    if( ui->clientServerRadioButton->isChecked() || ui->inSituRadioButton->isChecked() )
    {
        if( ui->clientServerRadioButton->isChecked() )
        {
            m_server_mode = SERVERMODE::ClientServer;
        }
        else if( ui->inSituRadioButton->isChecked() )
        {
            m_server_mode = SERVERMODE::InSitu;
        }
        else
        {
            m_server_mode = SERVERMODE::None;
        }

        if( m_server_mode != SERVERMODE::None )
        {
            connectServer();
        }
    }
    else // モードが選択されていない場合
    {
        m_server_mode = SERVERMODE::None;
        qWarning( "No option selected!" );
    }
}

void Connect::onVolumeDataBrowse()
{
    ui->volumeDataFilePathLineEdit->setText(
        QFileDialog::getOpenFileName(
            this,
            tr( "Select Volume Data File" ),
            ".",
            tr( "Volume Data Files (*.pfi *.pfl *.stl *.vtp *.xyz *.vtr *.vtk *.vti *.vts *.pvts *.inp *.vtu *.pvtu *.vtm *.case)" )
            )
        );
}

void Connect::onTransferFunctionBrowse()
{
    m_transfer_function_file_path = QFileDialog::getOpenFileName( this, tr("Select Transfer Function File"), ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )") );
    ui->transferFunctionFilePathLineEdit->setText( m_transfer_function_file_path );
}
