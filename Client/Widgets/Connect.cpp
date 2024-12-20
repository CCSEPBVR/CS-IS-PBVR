#include "Connect.h"
#include "ui_Connect.h"
#include "App/pbvrgui.h"
#include <QMessageBox>
#include <kvs/Camera>
#include <kvs/PointObject>
#include <kvs/PolygonObject>
#include <kvs/PolygonGlyphObject>
#include <QFileDialog>

#include "Widgets/MergePanel.h"
#include "Widgets/RenderOptions.h"


Connect::Connect(QWidget *parent, PBVRGUI *pbvr_gui, MergePanel* merge, DataProperties* filter_infomation, RenderOptions* render_options, TransferFunctionEditor* transfer_function_editor, GlyphEditor* glyph_editor ):
    QDialog(parent),
    ui(new Ui::Connect),
    m_pbvr_gui( pbvr_gui ),
    m_merge( merge  ),
    m_filter_infomation( filter_infomation ),
    m_render_options( render_options ),
    m_transfer_function_editor( transfer_function_editor ),
    m_glyph_editor( glyph_editor ),
    m_extended_transfer_function_message(),
    m_client_message(),
    m_server_message(),
    m_received_message()
{
    ui->setupUi(this);
    ui->uniformRBtn->setChecked( true );

    //Init particle limit, particle density, data size limit
    m_client_message.m_particle_limit = 10000000;
    m_client_message.m_particle_density = 1;
    m_client_message.m_particle_data_size_limit = 20;

    connect( ui->cancelPBtn, &QPushButton::clicked, this, &Connect::close );
    connect( ui->connectPBtn, &QPushButton::clicked, this, &Connect::onConnectButtonClicked );
    connect( ui->volumeDataFilePathBrowsePBtn, &QPushButton::clicked, this, &Connect::onVolumeDataBrowseButtonClicked );
    connect( ui->transferFunctionFilePathBrowsePBtn, &QPushButton::clicked, this, &Connect::onTransferFunctionFileBrowseButtonClicked );
}

Connect::~Connect()
{
    delete ui;
}

void Connect::connectServer()
{
    jpv::ParticleTransferClient client( "localhost", ui->portSBox->value() );
    m_server_message.m_camera = new kvs::Camera();
    m_client_message.m_camera = m_pbvr_gui->screen()->scene()->camera();

    if( ui-> clientServerRBtn -> isChecked() )
    {
        if( !ui->volumeDataFilePathLEdit->text().endsWith( ".pfi" ) && !ui->volumeDataFilePathLEdit->text().endsWith( ".pfl" ) )
        {
            QMessageBox::information( this, tr( "Connection Error" ), tr( "The file path does not end with .pfi or pfl" ) );
            return;
        }
    }

    int init = client.initClient();
    if( init < 0 )
    {
        QMessageBox::information(this, tr("Connection Error"), tr("The connection to the server failed. Please verify if the server is up and running."));
        return;
    }

    strncpy( m_client_message.m_header, "JPTP /1.0\r\n", 11 ); 

#ifdef Q_OS_WIN
    m_client_message.m_input_directory = ( ui->volumeDataFilePathLEdit->text().replace( "/","\\" ) ).toLocal8Bit().constData();
#else
    m_client_message.m_input_directory = ui->volumeDataFilePathLEdit->text().toStdString();
#endif

#ifdef Q_OS_WIN
        m_client_message.m_import_flag = m_transfer_function_editor->importFile( ui->transferFunctionFilePathLEdit->text().replace( "/","\\" ).toLocal8Bit().constData() );
#else

        m_client_message.m_import_flag = m_transfer_function_editor->importTransferFunctionFromFile( ui->transferFunctionFilePathLEdit->text().toStdString() );

#endif
        m_transfer_function_editor->apply();
        m_client_message.m_camera ->setWindowSize( m_pbvr_gui->screen()->width() , m_pbvr_gui->screen()->height() );


     //m_client_message.m_initialize_parameter = -3;
     m_client_message.m_initialize_parameter = jpv::InitializeParameter::initial_step;


    m_client_message.show();
    m_client_message.m_message_size = m_client_message.byteSize();
    client.sendMessage( m_client_message );
    client.recvMessage( &m_server_message );
    m_server_message.show();

    if ( ui->inSituRBtn->isChecked() )
    {
        m_render_options->updateParticleLimit();
    }


    //ヒストグラム更新用(CS, IS)
    m_received_message.m_var_range.merge( m_server_message.m_server_side_variable_range );
    m_received_message.m_color_bins.resize( m_server_message.m_transfer_function_count );
    m_received_message.m_opacity_bins.resize( m_server_message.m_transfer_function_count );
    for ( int tf = 0; tf < m_server_message.m_transfer_function_count; tf++ )
    {
        char color_function_name[8] = {0x00};
        char opacity_function_name[8] = {0x00};
        sprintf(color_function_name, "C%d", tf+1);
        sprintf(opacity_function_name, "O%d", tf+1);
        if ( m_server_message.m_color_nbins[tf] > 0 )
        {
            m_received_message.m_color_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_color_nbins[tf], (size_t *)m_server_message.m_color_bins[tf], std::string(color_function_name) );
        }
        if ( m_server_message.m_opacity_nbins[tf] >0 )
        {
            m_received_message.m_opacity_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_opacity_nbins[tf],(size_t *) m_server_message.m_opacity_bins[tf], std::string(opacity_function_name) );
        }
    }

    {
        m_transfer_function_editor->importTransferFunctionFromServer();
    }

    m_filter_infomation->updateFilterInfomation( ui->volumeDataFilePathLEdit->text(), m_server_message );
    m_glyph_editor->updateNumberOfVector( m_server_message );

    strncpy( m_client_message.m_header, "JPTP /1.0\r\n", 11 );

    //m_client_message.m_initialize_parameter = -1;
    m_client_message.m_initialize_parameter = jpv::InitializeParameter::empty;
    m_client_message.m_message_size = m_client_message.byteSize();
    client.sendMessage( m_client_message );
    client.recvMessage( &m_server_message );
    client.termClient();


    if( ui->clientServerRBtn->isChecked() )
    {
        m_merge->serverPointObjectCS( ui->volumeDataFilePathLEdit->text(), m_server_message.m_start_step, m_server_message.m_last_step );
        if( m_server_message.m_number_ingredients < 3  )
        {
        }
        else
        {
            m_merge->serverGlyphObjectCS( ui->volumeDataFilePathLEdit->text(), m_server_message.m_start_step, m_server_message.m_last_step );
        }

        if (m_client_message.m_import_flag)
        {
    #ifdef Q_OS_WIN
        m_transfer_function_editor->importTransferFunctionFromFile( ui->transferFunctionFilePathLEdit->text().replace( "/","\\" ).toLocal8Bit().constData() );
    #else
        m_transfer_function_editor->importTransferFunctionFromFile( ui->transferFunctionFilePathLEdit->text().toStdString() );
    #endif
        m_transfer_function_editor->apply();
        }
        //    qInfo() << m_server_message.m_variable_range.min( "t1_var_c" );
        //    qInfo() << m_server_message.m_min_value;
    }
    else if ( ui->inSituRBtn->isChecked() )
    {
        m_merge->serverPointObjectIS( "IS-Object", 0, 0 );
        std::cout << "m_server_message.m_number_ingredients = " << m_server_message.m_number_ingredients << std::endl;
        if( m_server_message.m_number_ingredients < 3  )
        {
        }
        else
        {
            m_merge->serverGlyphObjectIS( ui->volumeDataFilePathLEdit->text(), m_server_message.m_start_step, m_server_message.m_last_step );
        }

    }
    delete m_server_message.m_camera;
    ui->connectPBtn->setDisabled( true );
}

void Connect::sendTransferFunction()
{
    if(connecting)
    {
        qInfo() << "Other conneciton mode working !!";
    }
    else
    {
        connecting = true;
        if( ui-> clientServerRBtn -> isChecked() )
        {
            qInfo() <<  "this botton doesn't work in CS_MODE !!!";
        }
        else if ( ui->inSituRBtn->isChecked() )
        {

            std::cout << "********" << std::endl;
            std::cout << "********" << std::endl;
            std::cout << "********" << std::endl;
            std::cout << "********" << std::endl;

            jpv::ParticleTransferClient client( "localhost", ui->portSBox->value() );
            //    jpv::ParticleTransferClientMessage m_client_message;
            //    jpv::ParticleTransferServerMessage reply;
            m_server_message.m_camera = new kvs::Camera();
            client.initClient();
            strncpy( m_client_message.m_header, "JPTP /1.0\r\n", 11 );
            m_client_message.m_initialize_parameter = jpv::InitializeParameter::export_TFfile; // = 2
            m_client_message.m_rendering_id = 0;
            if( ui->uniformRBtn->isChecked() == true ) { m_client_message.m_sampling_method = 'u'; }
            if( ui->metropolisRBtn->isChecked() == true ) { m_client_message.m_sampling_method = 'm'; }
            if( ui->rejectionRBtn->isChecked() == true ) { m_client_message.m_sampling_method = 'r'; }
            m_client_message.m_subpixel_level = 2;
            m_client_message.m_repeat_level = 16;
            m_client_message.m_shuffle_method = 'r';
            m_client_message.m_time_parameter = 2;
            m_client_message.m_trans_parameter = 2;
            m_client_message.m_node_type = 'a';
            m_client_message.m_camera = m_pbvr_gui->screen()->scene()->camera();//足りないかも
            m_client_message.m_step = 0;
            m_client_message.m_message_size = m_client_message.byteSize();
            m_client_message.m_sampling_step = 1.0f;
            m_client_message.m_enable_crop_region = 0;

            m_client_message.m_message_size = m_client_message.byteSize();
            // TF情報をサーバー側に送信　（サーバーからの受信はしない）
            client.sendMessage( m_client_message );

            m_client_message.m_initialize_parameter = jpv::InitializeParameter::empty;
            m_client_message.m_message_size = m_client_message.byteSize();
            client.sendMessage( m_client_message );
            client.recvMessage( &m_server_message );

            client.termClient();
        }
        connecting = false;
    }
}


kvs::PointObject* Connect::generateParticles( int timeStep )
{

    if(connecting)
    {
        qInfo() << "Other conneciton mode working !!";
        kvs::PointObject* object = new kvs::PointObject();
        return  object;
    }
    else
    {
        connecting = true;
    std::cout << "********" << std::endl;
    std::cout << "********" << std::endl;
    std::cout << "********" << std::endl;
    std::cout << "********" << std::endl;

    jpv::ParticleTransferClient client( "localhost", ui->portSBox->value() );
//    jpv::ParticleTransferClientMessage m_client_message;
//    jpv::ParticleTransferServerMessage reply;
    m_server_message.m_camera = new kvs::Camera();
    client.initClient();
    strncpy( m_client_message.m_header, "JPTP /1.0\r\n", 11 );
    m_client_message.m_initialize_parameter = jpv::InitializeParameter::generate_particle;
    //m_client_message.m_initialize_parameter = 1;
    m_client_message.m_rendering_id = 0;
    if( ui->uniformRBtn->isChecked() == true ) { m_client_message.m_sampling_method = 'u'; }
    if( ui->metropolisRBtn->isChecked() == true ) { m_client_message.m_sampling_method = 'm'; }
    if( ui->rejectionRBtn->isChecked() == true ) { m_client_message.m_sampling_method = 'r'; }
    m_client_message.m_subpixel_level = 2;
    m_client_message.m_repeat_level = 16;
    m_client_message.m_shuffle_method = 'r';
    m_client_message.m_time_parameter = 2;
    m_client_message.m_trans_parameter = 2;
    m_client_message.m_node_type = 'a';
//    m_client_message.m_particle_limit = 10000000;
//    m_client_message.m_particle_density = 1;
//    m_client_message.particle_data_size_limit = 20;    
    m_client_message.m_camera = m_pbvr_gui->screen()->scene()->camera();//足りないかも
    m_client_message.m_step = timeStep;
    m_client_message.m_message_size = m_client_message.byteSize();
    m_client_message.m_sampling_step = 1.0f;
//    m_client_message.m_x_synthesis = "";
//    m_client_message.m_y_synthesis = "";
//    m_client_message.m_z_synthesis = "";
    m_client_message.m_enable_crop_region = 0;

    //paramExTransFunc.applyToClientMessage( &message ); //↓

    //gt5d
//    float min = -0.0791849;
//    float max = 0.074513;

    //spx
//    float min = 0.2;
//    float max = 1;

//    m_extended_transfer_function_message.applyToClientMessage( &m_client_message );


        m_client_message.m_message_size = m_client_message.byteSize();
        client.sendMessage( m_client_message );
        client.recvMessage( &m_server_message );

    size_t allParticle = 0;
    kvs::PointObject* object = new kvs::PointObject();
    int serve_numvol = m_server_message.m_number_volume_divide;


    for ( int n = 0; n < serve_numvol; n++ )
    {
            if ( client.recvMessage( &m_server_message ) == 1 ){}

        int nmemb = m_server_message.m_number_particle * 3;
        if ( nmemb != 0 )
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
            std::cout<<" getPointObjectFromServer 331"<<std::endl;
            allParticle = allParticle + m_server_message.m_number_particle;
            // delete[] m_server_message.m_colors;
            // delete[] m_server_message.m_normals;
            // delete[] m_server_message.m_positions;
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
    pointObject->setMinMaxObjectCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );
    pointObject->setMinMaxExternalCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );
//    pointObject->updateMinMaxCoords();

    std::cout << serverSideMinObjectCoords[0] << std::endl;
    std::cout << serverSideMinObjectCoords[1] << std::endl;
    std::cout << serverSideMinObjectCoords[2] << std::endl;
    std::cout << serverSideMaxObjectCoords[0] << std::endl;
    std::cout << serverSideMaxObjectCoords[1] << std::endl;
    std::cout << serverSideMaxObjectCoords[2] << std::endl;

    // m_client_message.m_initialize_parameter = -1;
    m_client_message.m_initialize_parameter = jpv::InitializeParameter::empty;
    m_client_message.m_message_size = m_client_message.byteSize();
    client.sendMessage( m_client_message );
    client.recvMessage( &m_server_message );

    client.termClient();

    //ここでサーバのレンジが手に入る。
    std::cout << m_server_message.m_server_side_variable_range.min( "t1_var_c" ) << std::endl;
    std::cout << m_server_message.m_server_side_variable_range.max( "t1_var_c" ) << std::endl;
    std::cout << m_server_message.m_server_side_variable_range.min( "t1_var_o" ) << std::endl;
    std::cout << m_server_message.m_server_side_variable_range.max( "t1_var_o" ) << std::endl;

    //ヒストグラム更新用(CS, IS)
        m_received_message.m_var_range.merge( m_server_message.m_server_side_variable_range );
        m_received_message.m_color_bins.resize( m_server_message.m_transfer_function_count );
        m_received_message.m_opacity_bins.resize( m_server_message.m_transfer_function_count );
        for ( int tf = 0; tf < m_server_message.m_transfer_function_count; tf++ )
        {
            char color_function_name[8] = {0x00};
            char opacity_function_name[8] = {0x00};
            sprintf(color_function_name, "C%d", tf+1);
            sprintf(opacity_function_name, "O%d", tf+1);
            if ( m_server_message.m_color_nbins[tf] > 0 )
            {
                m_received_message.m_color_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_color_nbins[tf], (size_t *)m_server_message.m_color_bins[tf], std::string(color_function_name) );
            }
            if ( m_server_message.m_opacity_nbins[tf] >0 )
            {
                m_received_message.m_opacity_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_opacity_nbins[tf],(size_t *) m_server_message.m_opacity_bins[tf], std::string(opacity_function_name) );
            }
        }

    m_transfer_function_editor->updateRangeView();

//    pointObject->updateMinMaxCoords();


    if ( m_server_message.m_camera )
    {
        delete m_server_message.m_camera;
        m_server_message.m_camera = nullptr;
    }

    if ( m_server_message.m_color_nbins )
    {
        delete[] m_server_message.m_color_nbins;
        m_server_message.m_color_nbins = nullptr;
    }

    if ( m_server_message.m_opacity_nbins )
    {
        delete[] m_server_message.m_opacity_nbins;
        m_server_message.m_opacity_nbins = nullptr;
    }

    if ( !m_server_message.m_color_bins.empty() )
    {
        for( auto ptr : m_server_message.m_color_bins )
        {
            delete ptr;
        }
        m_server_message.m_color_bins.clear();
    }

    if ( !m_server_message.m_opacity_bins.empty() )
    {
        for( auto ptr : m_server_message.m_opacity_bins )
        {
            delete ptr;
        }
        m_server_message.m_opacity_bins.clear();
    }

    if( ui->inSituRBtn->isChecked() == true )
    {
        m_merge->updateObjectTimeStepIS( m_server_message.m_start_step, m_server_message.m_last_step );
    }

    connecting = false;
    return pointObject;
    }
}

kvs::PolygonObject* Connect::generateGlyphPolygons( int timeStep )
{

    if(connecting)
    {
        qInfo() << "Other conneciton mode working !!";
        kvs::PolygonObject* object = new kvs::PolygonObject();
        return  object;
    }
    else
    {
        connecting = true;
        std::cout << "********" << std::endl;
        std::cout << "********" << std::endl;
        std::cout << "********" << std::endl;
        std::cout << "********" << std::endl;

        jpv::ParticleTransferClient client( "localhost", ui->portSBox->value() );
        //    jpv::ParticleTransferClientMessage m_client_message;
        //    jpv::ParticleTransferServerMessage reply;
        m_server_message.m_camera = new kvs::Camera();
        client.initClient();
        strncpy( m_client_message.m_header, "JPTP /1.0\r\n", 11 );
        m_client_message.m_initialize_parameter = jpv::InitializeParameter::generate_glyph;
        m_client_message.m_rendering_id = 0;
        if( ui->uniformRBtn->isChecked() == true ) { m_client_message.m_sampling_method = 'u'; }
        if( ui->metropolisRBtn->isChecked() == true ) { m_client_message.m_sampling_method = 'm'; }
        if( ui->rejectionRBtn->isChecked() == true ) { m_client_message.m_sampling_method = 'r'; }
        m_client_message.m_subpixel_level = 2;
        m_client_message.m_repeat_level = 16;
        m_client_message.m_shuffle_method = 'r';
        m_client_message.m_time_parameter = 2;
        m_client_message.m_trans_parameter = 2;
        m_client_message.m_node_type = 'a';
        m_client_message.m_camera = m_pbvr_gui->screen()->scene()->camera();//足りないかも
        m_client_message.m_step = timeStep;
        m_client_message.m_message_size = m_client_message.byteSize();
        m_client_message.m_sampling_step = 1.0f;
        m_client_message.m_enable_crop_region = 0;

        // stab data
        m_client_message.m_glyph_flag = true;

        //m_client_message.m_direction_variable.resize(3);
        m_client_message.m_direction_variable[0] = "q1";
        m_client_message.m_direction_variable[1] = "q2";
        m_client_message.m_direction_variable[2] = "q3";

        m_client_message.m_size_sampling_method = jpv::DataDefines::VariableArray;
        m_client_message.m_size_variables.resize(2);
        m_client_message.m_size_variables[0] = "q1";
        m_client_message.m_size_variables[1] = "q2";

        m_client_message.m_distribution_mode = jpv::GlyphMode::UniformDistribution;
        m_client_message.m_number_of_sampling_point = 100;
        m_client_message.m_seed = 16;
        m_client_message.m_stride = 6;
        m_client_message.m_glyph_color_map_table.clear();
        for(int i =0; i < 256*3; i++ ) m_client_message.m_glyph_color_map_table.push_back( 255);
        m_client_message.m_color_data_sampling_method = jpv::DataDefines::VariableArray;
        m_client_message.m_color_data_variables.resize(2);
        m_client_message.m_color_data_variables[0] = "q2";
        m_client_message.m_color_data_variables[1] = "q1";

        //paramExTransFunc.applyToClientMessage( &message ); //↓

        //gt5d
        //    float min = -0.0791849;
        //    float max = 0.074513;

        //spx
        //    float min = 0.2;
        //    float max = 1;

        //    m_extended_transfer_function_message.applyToClientMessage( &m_client_message );



        m_client_message.m_message_size = m_client_message.byteSize();
        client.sendMessage( m_client_message );
        client.recvMessage( &m_server_message );
        size_t allParticle = 0;
        //kvs::PolygonObject* object = new kvs::PolygonObject();
        kvs::PointObject* object = new kvs::PointObject();
        int serve_numvol = m_server_message.m_number_volume_divide;
        std::cout << "m_server_message.m_number_volume_divide = " << m_server_message.m_number_volume_divide <<std::endl;

        for ( int n = 0; n < serve_numvol; n++ )
        {
            if ( client.recvMessage( &m_server_message ) == 1 ){}

            int nmemb = m_server_message.m_number_glyph * 3;
            if ( nmemb != 0 )
            {
                kvs::ValueArray<kvs::Real32> positions ( m_server_message.m_glyph_coords.get(), nmemb );
                kvs::ValueArray<kvs::Real32> vectors ( m_server_message.m_glyph_vectors.get(), nmemb );
                kvs::ValueArray<kvs::Real32> sizes ( m_server_message.m_glyph_sizes.get(), nmemb );
                kvs::ValueArray<kvs::UInt8>  colors ( m_server_message.m_glyph_colors.get(), nmemb );
                std::cout << __LINE__ <<std::endl;
                //kvs::PolygonObject obj;
                kvs::PointObject obj;
                obj.setCoords( positions );
                obj.setNormals( vectors );
                obj.setSizes( sizes );
                obj.setColors( colors );
                std::cout << __LINE__ <<std::endl;
                object->add(obj);
                obj.clear();
                std::cout<<" getpolygonObjectFromServer 331"<<std::endl;
                allParticle = allParticle + m_server_message.m_number_particle;
                // delete[] m_server_message.m_colors;
                // delete[] m_server_message.m_normals;
                // delete[] m_server_message.m_positions;
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
        //    polygonObject->updateMinMaxCoords();

        for ( int i = 0; i < 10; ++i )
        {
            std::cout << " pointObject->coords()[3 * i + 0]   =" <<pointObject->coords()[3 * i + 0]  << std::endl;
            std::cout << " pointObject->coords()[3 * i + 1]   =" <<pointObject->coords()[3 * i + 1]  << std::endl;
            std::cout << " pointObject->coords()[3 * i + 2]   =" <<pointObject->coords()[3 * i + 2]   << std::endl;
            std::cout << " pointObject->vectors()[3 * i + 0]  =" <<pointObject->normals()[3 * i + 0] << std::endl;
            std::cout << " pointObject->vectors()[3 * i + 1]  =" <<pointObject->normals()[3 * i + 1] << std::endl;
            std::cout << " pointObject->normals()[3 * i + 2]  =" <<pointObject->normals()[3 * i + 2] << std::endl;
            std::cout << " pointObject->colors()[3 * i + 0]   =" <<(int)pointObject->colors()[3 * i + 0]  << std::endl;
            std::cout << " pointObject->colors()[3 * i + 1]   =" <<(int)pointObject->colors()[3 * i + 1]  << std::endl;
            std::cout << " pointObject->colors()[3 * i + 2]   =" <<(int)pointObject->colors()[3 * i + 2]  << std::endl;
            std::cout << " pointObject->sizes()[3 * i + 0]    =" <<pointObject->sizes()[ i ]         << std::endl;
        }

        std::cout << serverSideMinObjectCoords[0] << std::endl;
        std::cout << serverSideMinObjectCoords[1] << std::endl;
        std::cout << serverSideMinObjectCoords[2] << std::endl;
        std::cout << serverSideMaxObjectCoords[0] << std::endl;
        std::cout << serverSideMaxObjectCoords[1] << std::endl;
        std::cout << serverSideMaxObjectCoords[2] << std::endl;

        kvs::ValueArray<kvs::Real32> coords;
        coords.allocate(3 * m_server_message.m_number_glyph); // 3 * m_number_glyph
        kvs::ValueArray<kvs::Real32> directions;
        directions.allocate(3 * m_server_message.m_number_glyph);
        kvs::ValueArray<kvs::Real32> sizes;
        sizes.allocate(m_server_message.m_number_glyph);
        kvs::ValueArray<kvs::UInt8> colors;
        colors.allocate(3 * m_server_message.m_number_glyph);

        for (int i = 0; i < m_server_message.m_number_glyph; i++)
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
            sizes[i] = pointObject->sizes()[i] * m_glyph_editor->getScaleFactor() ;

            // 色を設定
            colors[3 * i + 0] = pointObject->colors()[3 * i + 0];
            colors[3 * i + 1] = pointObject->colors()[3 * i + 1];
            colors[3 * i + 2] = pointObject->colors()[3 * i + 2];
        }



        // m_client_message.m_initialize_parameter = -1;
        m_client_message.m_initialize_parameter = jpv::InitializeParameter::empty;
        m_client_message.m_message_size = m_client_message.byteSize();
        client.sendMessage( m_client_message );
        client.recvMessage( &m_server_message );

        m_server_message.show();
        client.termClient();
#if 0
        //ここでサーバのレンジが手に入る。
        std::cout << m_server_message.m_server_side_variable_range.min( "t1_var_c" ) << std::endl;
        std::cout << m_server_message.m_server_side_variable_range.max( "t1_var_c" ) << std::endl;
        std::cout << m_server_message.m_server_side_variable_range.min( "t1_var_o" ) << std::endl;
        std::cout << m_server_message.m_server_side_variable_range.max( "t1_var_o" ) << std::endl;

        //ヒストグラム更新用(CS, IS)
        m_received_message.m_var_range.merge( m_server_message.m_server_side_variable_range );
        m_received_message.m_color_bins.resize( m_server_message.m_transfer_function_count );
        m_received_message.m_opacity_bins.resize( m_server_message.m_transfer_function_count );
        for ( int tf = 0; tf < m_server_message.m_transfer_function_count; tf++ )
        {
            char color_function_name[8] = {0x00};
            char opacity_function_name[8] = {0x00};
            sprintf(color_function_name, "C%d", tf+1);
            sprintf(opacity_function_name, "O%d", tf+1);
            if ( m_server_message.m_color_nbins[tf] > 0 )
            {
                m_received_message.m_color_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_color_nbins[tf], (size_t *)m_server_message.m_color_bins[tf], std::string(color_function_name) );
            }
            if ( m_server_message.m_opacity_nbins[tf] >0 )
            {
                m_received_message.m_opacity_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_opacity_nbins[tf],(size_t *) m_server_message.m_opacity_bins[tf], std::string(opacity_function_name) );
            }
        }

        m_transfer_function_editor->updateRangeView();

        //    polygonObject->updateMinMaxCoords();


        if ( m_server_message.m_camera )
        {
            delete m_server_message.m_camera;
            m_server_message.m_camera = nullptr;
        }

        if ( m_server_message.m_color_nbins )
        {
            delete[] m_server_message.m_color_nbins;
            m_server_message.m_color_nbins = nullptr;
        }

        if ( m_server_message.m_opacity_nbins )
        {
            delete[] m_server_message.m_opacity_nbins;
            m_server_message.m_opacity_nbins = nullptr;
        }

        if ( !m_server_message.m_color_bins.empty() )
        {
            for( auto ptr : m_server_message.m_color_bins )
            {
                delete ptr;
            }
            m_server_message.m_color_bins.clear();
        }

        if ( !m_server_message.m_opacity_bins.empty() )
        {
            for( auto ptr : m_server_message.m_opacity_bins )
            {
                delete ptr;
            }
            m_server_message.m_opacity_bins.clear();
        }

        if( ui->inSituRBtn->isChecked() == true )
        {
            m_merge->updateObjectTimeStepIS( m_server_message.m_start_step, m_server_message.m_last_step );
        }
#endif
        connecting = false;

        // kvs::ValueArray<kvs::Real32> coords;
        // coords.allocate(3 * m_server_message.m_number_glyph); // 3 * m_number_glyph
        // kvs::ValueArray<kvs::Real32> directions;
        // directions.allocate(3 * m_server_message.m_number_glyph);
        // kvs::ValueArray<kvs::Real32> sizes;
        // sizes.allocate(m_server_message.m_number_glyph);
        // kvs::ValueArray<kvs::UInt8> colors;
        // colors.allocate(3 * m_server_message.m_number_glyph);

        // for (int i = 0; i < m_server_message.m_number_glyph; i++)
        // {
        //     // 座標を設定
        //     coords[3 * i + 0] = pointObject->coords()[3 * i + 0];
        //     coords[3 * i + 1] = pointObject->coords()[3 * i + 1];
        //     coords[3 * i + 2] = pointObject->coords()[3 * i + 2];

        //     // 法線を設定
        //     directions[3 * i + 0] = pointObject->normals()[3 * i + 0];
        //     directions[3 * i + 1] = pointObject->normals()[3 * i + 1];
        //     directions[3 * i + 2] = pointObject->normals()[3 * i + 2];

        //     // サイズを設定
        //     sizes[i] = pointObject->sizes()[i] * m_glyph_editor->getScaleFactor() * 99;

        //     // 色を設定
        //     colors[3 * i + 0] = pointObject->colors()[3 * i + 0];
        //     colors[3 * i + 1] = pointObject->colors()[3 * i + 1];
        //     colors[3 * i + 2] = pointObject->colors()[3 * i + 2];
        // }


        // kvs::ValueArray<kvs::Real32> coords;
        // std::cout << "m_server_message.m_number_glyph = " << m_server_message.m_number_glyph <<std::endl;
        // coords.allocate(3 * m_server_message.m_number_glyph); // 3 * m_glyph_num
        // coords.at(0) = 0;
        // coords.at(1) = 0;
        // coords.at(2) = 0;
        // kvs::ValueArray<kvs::Real32> directions;
        // directions.allocate(3 * m_server_message.m_number_glyph);
        // directions.at(0) = 1;
        // directions.at(1) = 0;
        // directions.at(2) = 0;
        // kvs::ValueArray<kvs::Real32> sizes;
        // sizes.allocate(1* m_server_message.m_number_glyph );
        // sizes.at(0) = 1 * m_glyph_editor->getScaleFactor();
        // kvs::ValueArray<kvs::UInt8> colors;
        // colors.allocate(3* m_server_message.m_number_glyph);
        // colors.at(0) = 0;
        // colors.at(1) = 0;
        // colors.at(2) = 0;

        m_glyph_editor->m_coords = coords;
        m_glyph_editor->m_directions = directions;
        m_glyph_editor->m_sizes = sizes;
        m_glyph_editor->m_colors = colors;

        polygonObject = new kvs::PolygonGlyphObject( coords, directions, sizes, colors, static_cast<kvs::PolygonGlyphObject::GlyphType>(m_glyph_editor->getGlyphType()) );
        polygonObject->setMinMaxObjectCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );
        polygonObject->setMinMaxExternalCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );


        return polygonObject;
    }
}


void Connect::deletedServerObject()
{
    ui->connectPBtn->setEnabled( true );
}

void Connect::onVolumeDataBrowseButtonClicked()
{
    ui->volumeDataFilePathLEdit->setText( QFileDialog::getOpenFileName( this, tr("Select Volume Data File"), ".", tr("Volume Data Files (*.pfi *.pfl)") ) );
}

void Connect::onTransferFunctionFileBrowseButtonClicked()
{
    ui->transferFunctionFilePathLEdit->setText( QFileDialog::getOpenFileName( this, tr("Select Transfer Function File"), ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )") ) );
}

void Connect::onConnectButtonClicked()
{
    if( ui->clientServerRBtn->isChecked() )
    {
        qDebug("CS CONNECT!");
        connectServer();
    }
    else if( ui->inSituRBtn->isChecked() )
    {
        qDebug("IS CONNECT!");
        connectServer();
    }
    else
    {
        qDebug("No option selected!");
    }
}
