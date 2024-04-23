#include "Connect.h"
#include "ui_Connect.h"

#include <QMessageBox>
#include <kvs/Camera>
#include <kvs/PointObject>
#include <kvs/ParticleBasedRenderer>

#include "Widgets/MergePanel.h"

Connect::Connect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Connect),
    m_screen( nullptr ),
    m_camera( nullptr ),
    m_merge( nullptr ),
    m_filter_infomation( nullptr ),
    m_transfer_function_editor( nullptr ),
    m_extended_transfer_function_message(),
    m_client_message(),
    m_server_message(),
    m_received_message()
{
    ui->setupUi(this);
    ui->uniformRBtn->setChecked( true );
    connect( ui->connectPBtn, &QPushButton::clicked, this, &Connect::onConnectButtonClicked );
#include"QPushButton"
}

Connect::~Connect()
{
    delete ui;
}

//jpv::ParticleTransferClientMessage::EquationToken Connect::convertToken( std::string expression )
//{
//    FuncParser::ExpressionTokenizer tokenizer;
//    FuncParser::ExpressionConverter exprconv;

//    jpv::ParticleTransferClientMessage::EquationToken eq_token;

//    tokenizer.tokenizeString( expression );
//    exprconv.convertExpToken( tokenizer.m_exp_token );
//    int size = exprconv.token_array.size();
//    if( size > 128 ){ printf("Equation length too long\n");}

//    for( int i = 0; i < 128; i++ )
//    {
//        if( i < size )
//        {
//            eq_token.exp_token[i]   = exprconv.token_array[i];
//            eq_token.var_name[i]    = exprconv.var_array[i];
//            eq_token.value_array[i] = exprconv.value_array[i];
//        }
//        else
//        {
//            eq_token.exp_token[i]   = 0;
//            eq_token.var_name[i]    = 0;
//            eq_token.value_array[i] = 0;
//        }
//    }

//    std::cout << "exp" << std::endl;
//    for(int i = 0; i < 128; i++)
//    {
//        std::cout << eq_token.exp_token[i] << ",";
//    }
//    std::cout << std::endl;
//    std::cout << "var_name" << std::endl;
//    for(int i = 0; i < 128; i++)
//    {
//        std::cout << eq_token.var_name[i] << ",";
//    }
//    std::cout << std::endl;
//    std::cout << "value_array" << std::endl;
//    for(int i = 0; i < 128; i++)
//    {
//        std::cout << eq_token.value_array[i] << ",";
//    }
//    std::cout << std::endl;

//    return eq_token;
//}

void Connect::connect1()
{
    jpv::ParticleTransferClient client( "localhost", ui->portSBox->value() );
    jpv::ParticleTransferClientMessage message;
    jpv::ParticleTransferServerMessage reply;
    reply.camera = new kvs::Camera();

    int init = client.initClient();
    if( init < 0 )
    {
        QMessageBox::information(this, tr("Connection Error"), tr("The connection to the server failed. Please verify if the server is up and running."));
        return;
    }
    strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
    message.m_initialize_parameter = -3;
    message.m_input_directory = ui->volumeDataFilePathLEdit->text().toStdString();
    m_extended_transfer_function_message.applyToClientMessage( &message );
    message.m_message_size = message.byteSize();
    client.sendMessage( message );
    client.recvMessage( &reply );

    m_filter_infomation->updateFilterInfomation( ui->volumeDataFilePathLEdit->text(), reply );

    strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
    message.m_initialize_parameter = -1;
    message.m_message_size = message.byteSize();
    client.sendMessage( message );
    client.recvMessage( &reply );
    client.termClient();

    m_merge->serverObject( ui->volumeDataFilePathLEdit->text(), reply.m_start_step, reply.m_end_step );
    m_transfer_function_editor->applyVariableRange( reply.m_variable_range );
//    qInfo() << reply.m_variable_range.min( "t1_var_c" );
//    qInfo() << reply.m_min_value;
    ui->cancelPBtn->setDisabled( true );
}

kvs::PointObject* Connect::connect2( int timeStep )
{
    std::cout << "********" << std::endl;
    std::cout << "********" << std::endl;
    std::cout << "********" << std::endl;
    std::cout << "********" << std::endl;

    jpv::ParticleTransferClient client( "localhost", ui->portSBox->value() );
//    jpv::ParticleTransferClientMessage m_client_message;
//    jpv::ParticleTransferServerMessage reply;
    m_server_message.camera = new kvs::Camera();
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
    m_client_message.m_trans_Parameter = 2;
    m_client_message.m_node_type = 'a';
//    m_client_message.m_particle_limit = 10000000;
//    m_client_message.m_particle_density = 1;
//    m_client_message.particle_data_size_limit = 20;    
    m_client_message.m_camera = m_camera;//足りないかも
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
    std::cout << "SEND" << std::endl;
    client.sendMessage( m_client_message );
    client.recvMessage( &m_server_message );

    if ( client.recvMessage( &m_server_message ) == 1 ){}
    //    if ( client.recvMessage( &reply ) == 1 ){}

    size_t allParticle = 0;
    kvs::PointObject* object = new kvs::PointObject();

    int nmemb = m_server_message.m_number_particle * 3;
    if ( nmemb != 0 )
    {
        kvs::ValueArray<kvs::Real32> positions ( m_server_message.m_positions, nmemb );
        kvs::ValueArray<kvs::Real32> normals ( m_server_message.m_normals, nmemb );
        kvs::ValueArray<kvs::UInt8>  colors ( m_server_message.m_colors, nmemb );

        kvs::PointObject obj;
        obj.setCoords( positions );
        obj.setNormals( normals );
        obj.setColors( colors );

        object->add(obj);
        obj.clear();
        std::cout<<" getPointObjectFromServer 331"<<std::endl;
        allParticle = allParticle + m_server_message.m_number_particle;
        delete[] m_server_message.m_colors;
        delete[] m_server_message.m_normals;
        delete[] m_server_message.m_positions;
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
    m_client_message.m_message_size = m_client_message.byteSize();
    client.sendMessage( m_client_message );
    client.recvMessage( &m_server_message );
    client.termClient();

    //ここでサーバのレンジが手に入る。
    std::cout << m_server_message.m_variable_range.min( "t1_var_c" ) << std::endl;
    std::cout << m_server_message.m_variable_range.max( "t1_var_c" ) << std::endl;
    std::cout << m_server_message.m_variable_range.min( "t1_var_o" ) << std::endl;
    std::cout << m_server_message.m_variable_range.max( "t1_var_o" ) << std::endl;

    //ヒストグラム更新用
    m_received_message.m_var_range.merge( m_server_message.m_variable_range );
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

    m_transfer_function_editor->applyVariableRange( m_server_message.m_variable_range );
//    m_transfer_function_editor->updateRangeView( reply.m_variable_range );
//    m_transfer_function_editor->updateRangeView( reply );
    m_transfer_function_editor->updateRangeView();

//    pointObject->updateMinMaxCoords();
    return pointObject;
}

void Connect::deletedServerObject()
{
    ui->connectPBtn->setEnabled( true );
}

void Connect::onConnectButtonClicked()
{
    connect1();
}
