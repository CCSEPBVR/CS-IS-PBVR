#include "Connect.h"
#include "ui_Connect.h"

#include <QMessageBox>
#include <kvs/Camera>
#include <kvs/PointObject>

#include "Widgets/MergePanel2.h"

Connect::Connect(QWidget *parent, MergePanel2* merge, DataProperties* filter_infomation, TransferFunctionEditor* transfer_function_editor) :
    QDialog(parent),
    ui(new Ui::Connect),
    m_screen( nullptr ),
    m_camera( nullptr ),
    m_merge( merge ),
    m_filter_infomation( filter_infomation ),
    m_transfer_function_editor( transfer_function_editor ),
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

void Connect::connectServerCS()
{
    jpv::ParticleTransferClient client( "localhost", ui->portSBox->value() );
    jpv::ParticleTransferClientMessage message;
    jpv::ParticleTransferServerMessage reply;
    reply.m_camera = new kvs::Camera();

    if( !ui->volumeDataFilePathLEdit->text().endsWith( ".pfi" ) && !ui->volumeDataFilePathLEdit->text().endsWith( ".pfl" ) )
    {
        QMessageBox::information( this, tr( "Connection Error" ), tr( "The file path does not end with .pfi or pfl" ) );
        return;
    }

    int init = client.initClient();
    if( init < 0 )
    {
        QMessageBox::information(this, tr("Connection Error"), tr("The connection to the server failed. Please verify if the server is up and running."));
        return;
    }
    strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
    message.m_initialize_parameter = -3;
#ifdef Q_OS_WIN
    message.m_input_directory = ( ui->volumeDataFilePathLEdit->text().replace( "/","\\" ) ).toLocal8Bit().constData();
#else
    message.m_input_directory = ui->volumeDataFilePathLEdit->text().toStdString();
#endif
    m_extended_transfer_function_message.applyToClientMessageCS( &message );
    message.m_message_size = message.byteSizeCS();
    client.sendMessageCS( message );
    client.recvMessageCS( &reply );

    m_filter_infomation->updateFilterInfomation( ui->volumeDataFilePathLEdit->text(), reply );

    strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
    message.m_initialize_parameter = -1;
    message.m_message_size = message.byteSizeCS();
    client.sendMessageCS( message );
    client.recvMessageCS( &reply );
    client.termClient();

    m_merge->serverObjectCS( ui->volumeDataFilePathLEdit->text(), reply.m_start_step, reply.m_end_step );
    m_transfer_function_editor->applyVariableRange( reply.m_server_side_variable_range );
#ifdef Q_OS_WIN
    m_transfer_function_editor->importFile( ui->transferFunctionFilePathLEdit->text().replace( "/","\\" ).toLocal8Bit().constData() );
#else
    m_transfer_function_editor->importFile( ui->transferFunctionFilePathLEdit->text().toStdString() );
#endif
    m_transfer_function_editor->onApplyButtonClicked();
    //    qInfo() << reply.m_variable_range.min( "t1_var_c" );
    //    qInfo() << reply.m_min_value;
    delete reply.m_camera;
    ui->connectPBtn->setDisabled( true );
}

void Connect::connectServerIS()
{
    jpv::ParticleTransferClient client( "localhost", ui->portSBox->value() );
    jpv::ParticleTransferClientMessage message;
    m_server_message.m_camera = new kvs::Camera();

//    if( !ui->volumeDataFilePathLEdit->text().endsWith( ".pfi" ) && !ui->volumeDataFilePathLEdit->text().endsWith( ".pfl" ) )
//    {
//        QMessageBox::information( this, tr( "Connection Error" ), tr( "The file path does not end with .pfi or pfl" ) );
//        return;
//    }

    int init = client.initClient();
    if( init < 0 )
    {
        QMessageBox::information(this, tr("Connection Error"), tr("The connection to the server failed. Please verify if the server is up and running."));
        return;
    }
    strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
    message.m_initialize_parameter = -3;
#ifdef Q_OS_WIN
    message.m_input_directory = ( ui->volumeDataFilePathLEdit->text().replace( "/","\\" ) ).toLocal8Bit().constData();
#else
    message.m_input_directory = ui->volumeDataFilePathLEdit->text().toStdString();
#endif
    m_extended_transfer_function_message.applyToClientMessageIS( &message );
    message.m_message_size = message.byteSizeIS();
    client.sendMessageIS( message );
    client.recvMessageIS( &m_server_message );

    m_transfer_function_editor->importFromServerIS();

    m_filter_infomation->updateFilterInfomation( ui->volumeDataFilePathLEdit->text(), m_server_message );

    strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
    message.m_initialize_parameter = -1;
    message.m_message_size = message.byteSizeIS();
    client.sendMessageIS( message );
    client.recvMessageIS( &m_server_message );
    client.termClient();

//    m_merge->serverObjectIS( "IS-Object", 0, 0 );
//    m_transfer_function_editor->applyVariableRange( reply.m_variable_range );
//#ifdef Q_OS_WIN
//    m_transfer_function_editor->importFile( ui->transferFunctionFilePathLEdit->text().replace( "/","\\" ).toLocal8Bit().constData() );
//#else
//    m_transfer_function_editor->importFile( ui->transferFunctionFilePathLEdit->text().toStdString() );
//#endif
//    m_transfer_function_editor->onApplyButtonClicked();
//    qInfo() << reply.m_variable_range.min( "t1_var_c" );
//    qInfo() << reply.m_min_value;
    delete m_server_message.m_camera;
    ui->connectPBtn->setDisabled( true );
}

kvs::PointObject* Connect::generateParticles( int timeStep )
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
    m_client_message.m_initialize_parameter = 1;
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
    m_client_message.m_camera = m_camera;//足りないかも
    m_client_message.m_step = timeStep;
    if( m_transfer_function_editor->getMode() == TransferFunctionEditor::Mode::CS )
    {
        m_client_message.m_message_size = m_client_message.byteSizeCS();
    }
    else if( m_transfer_function_editor->getMode() == TransferFunctionEditor::Mode::IS )
    {
        m_client_message.m_message_size = m_client_message.byteSizeIS();
    }
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


    std::cout << "SEND" << std::endl;

    if( m_transfer_function_editor->getMode() == TransferFunctionEditor::Mode::CS )
    {
        m_client_message.m_message_size = m_client_message.byteSizeCS();
        client.sendMessageCS( m_client_message );
        client.recvMessageCS( &m_server_message );
    }
    else if( m_transfer_function_editor->getMode() == TransferFunctionEditor::Mode::IS )
    {
        m_client_message.m_message_size = m_client_message.byteSizeIS();
        client.sendMessageIS( m_client_message );
        client.recvMessageIS( &m_server_message );
    }

    size_t allParticle = 0;
    kvs::PointObject* object = new kvs::PointObject();
    int serve_numvol = m_server_message.m_number_volume_divide;


    for ( int n = 0; n < serve_numvol; n++ )
    {
        if( m_transfer_function_editor->getMode() == TransferFunctionEditor::Mode::CS )
        {
            if ( client.recvMessageCS( &m_server_message ) == 1 ){}
        }
        else if( m_transfer_function_editor->getMode() == TransferFunctionEditor::Mode::IS )
        {
            if ( client.recvMessageIS( &m_server_message ) == 1 ){}
        }

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

    //ヒストグラム更新用(IS)
    if( m_transfer_function_editor->getMode() == TransferFunctionEditor::Mode::IS )
    {
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
            if ( m_server_message.m_opacity_nbins[tf] )
            {
                m_received_message.m_opacity_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_opacity_nbins[tf],(size_t *) m_server_message.m_opacity_bins[tf], std::string(opacity_function_name) );
            }
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

    m_client_message.m_initialize_parameter = -1;

    if( m_transfer_function_editor->getMode() == TransferFunctionEditor::Mode::CS )
    {
        m_client_message.m_message_size = m_client_message.byteSizeCS();
        client.sendMessageCS( m_client_message );
        client.recvMessageCS( &m_server_message );
    }
    else if( m_transfer_function_editor->getMode() == TransferFunctionEditor::Mode::IS )
    {
        m_client_message.m_message_size = m_client_message.byteSizeIS();
        client.sendMessageIS( m_client_message );
        client.recvMessageIS( &m_server_message );
    }

    client.termClient();

    //ここでサーバのレンジが手に入る。
    std::cout << m_server_message.m_server_side_variable_range.min( "t1_var_c" ) << std::endl;
    std::cout << m_server_message.m_server_side_variable_range.max( "t1_var_c" ) << std::endl;
    std::cout << m_server_message.m_server_side_variable_range.min( "t1_var_o" ) << std::endl;
    std::cout << m_server_message.m_server_side_variable_range.max( "t1_var_o" ) << std::endl;

    //ヒストグラム更新用(CS)
    if( m_transfer_function_editor->getMode() == TransferFunctionEditor::Mode::CS )
    {
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
            if ( m_server_message.m_opacity_nbins[tf] )
            {
                m_received_message.m_opacity_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, m_server_message.m_opacity_nbins[tf],(size_t *) m_server_message.m_opacity_bins[tf], std::string(opacity_function_name) );
            }
        }
    }

    m_transfer_function_editor->applyVariableRange( m_server_message.m_server_side_variable_range );
//    m_transfer_function_editor->updateRangeView( reply.m_variable_range );
//    m_transfer_function_editor->updateRangeView( reply );
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

    if( m_transfer_function_editor->getMode() == TransferFunctionEditor::Mode::IS )
    {
//        m_merge->updateObjectTimeStepIS( m_server_message.m_start_step, m_server_message.m_end_step );
    }

    return pointObject;
}

void Connect::deletedServerObject()
{
    ui->connectPBtn->setEnabled( true );
}
#include <QFileDialog>
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
        m_transfer_function_editor->setMode( TransferFunctionEditor::Mode::CS );
        connectServerCS();
    }
    else if( ui->inSituRBtn->isChecked() )
    {
        qDebug("IS CONNECT!");
        m_transfer_function_editor->setMode( TransferFunctionEditor::Mode::IS );
        connectServerIS();
    }
    else
    {
        qDebug("No option selected!");
    }
}
