#include "Connect.h"
#include "ui_Connect.h"

#include <kvs/Camera>
#include <kvs/PointObject>
#include <kvs/ParticleBasedRenderer>

#include "Widgets/Merge.h"

Connect::Connect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Connect)
{
    ui->setupUi(this);
    ui->uniformRBtn->setChecked( true );
    connect( ui->connectPBtn, &QPushButton::clicked, this, &Connect::onConnectButtonClicked );
}

Connect::~Connect()
{
    delete ui;
}

jpv::ParticleTransferClientMessage::EquationToken Connect::convertToken( std::string expression )
{
    FuncParser::ExpressionTokenizer tokenizer;
    FuncParser::ExpressionConverter exprconv;

    jpv::ParticleTransferClientMessage::EquationToken eq_token;

    tokenizer.tokenizeString( expression );
    exprconv.convertExpToken( tokenizer.m_exp_token );
    int size = exprconv.token_array.size();
    if( size > 128 ){ printf("Equation length too long\n");}

    for( int i = 0; i < 128; i++ )
    {
        if( i < size )
        {
            eq_token.exp_token[i]   = exprconv.token_array[i];
            eq_token.var_name[i]    = exprconv.var_array[i];
            eq_token.value_array[i] = exprconv.value_array[i];
        }
        else
        {
            eq_token.exp_token[i]   = 0;
            eq_token.var_name[i]    = 0;
            eq_token.value_array[i] = 0;
        }
    }

    std::cout << "exp" << std::endl;
    for(int i = 0; i < 128; i++)
    {
        std::cout << eq_token.exp_token[i] << ",";
    }
    std::cout << std::endl;
    std::cout << "var_name" << std::endl;
    for(int i = 0; i < 128; i++)
    {
        std::cout << eq_token.var_name[i] << ",";
    }
    std::cout << std::endl;
    std::cout << "value_array" << std::endl;
    for(int i = 0; i < 128; i++)
    {
        std::cout << eq_token.value_array[i] << ",";
    }
    std::cout << std::endl;

    return eq_token;
}

void Connect::connect1()
{
    jpv::ParticleTransferClient client( "localhost", ui->portSBox->value() );
    jpv::ParticleTransferClientMessage message;
    jpv::ParticleTransferServerMessage reply;
    reply.camera = m_camera;

    int init = client.initClient();
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
    qInfo() << reply.m_variable_range.min( "t1_var_c" );
    qInfo() << reply.m_min_value;
}

kvs::PointObject* Connect::connect2( int timeStep )
{
    std::cout << "********" << std::endl;
    std::cout << "********" << std::endl;
    std::cout << "********" << std::endl;
    std::cout << "********" << std::endl;

    jpv::ParticleTransferClient client( "localhost", ui->portSBox->value() );
//    jpv::ParticleTransferClientMessage m_client_message;
    jpv::ParticleTransferServerMessage reply;
    reply.camera = m_camera;
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
    m_client_message.m_x_synthesis = "";
    m_client_message.m_y_synthesis = "";
    m_client_message.m_z_synthesis = "";
    m_client_message.m_enable_crop_region = 0;

    //paramExTransFunc.applyToClientMessage( &message ); //↓

    //gt5d
//    float min = -0.0791849;
//    float max = 0.074513;

    //spx
    float min = 0.2;
    float max = 1;

    m_client_message.m_transfer_function.clear();
    m_client_message.m_volume_equation.clear();
    m_client_message.opacity_var.clear();
    m_client_message.color_var.clear();

    m_client_message.m_transfer_function.resize(10);
    m_client_message.m_transfer_function[0].m_resolution = 256;
    m_client_message.m_transfer_function[0].m_equation_red = "";
    m_client_message.m_transfer_function[0].m_equation_green = "";
    m_client_message.m_transfer_function[0].m_equation_blue = "";
    m_client_message.m_transfer_function[0].m_equation_opacity = "";
    m_client_message.m_transfer_function[0].m_color_variable_min = min;
    m_client_message.m_transfer_function[0].m_color_variable_max = max;
    m_client_message.m_transfer_function[0].m_opacity_variable_min = 0;
    m_client_message.m_transfer_function[0].m_opacity_variable_max = 1;
    m_client_message.m_transfer_function[0].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    m_client_message.m_transfer_function[0].m_name ="C1";
    m_client_message.m_transfer_function[0].m_color_variable = "_F1_VAR_C";
    m_client_message.m_transfer_function[0].m_opacity_variable ="";
    m_client_message.m_transfer_function[0].m_range_initialized = 1;

    m_client_message.m_transfer_function[1].m_resolution = 256;
    m_client_message.m_transfer_function[1].m_equation_red = "";
    m_client_message.m_transfer_function[1].m_equation_green = "";
    m_client_message.m_transfer_function[1].m_equation_blue = "";
    m_client_message.m_transfer_function[1].m_equation_opacity = "";
    m_client_message.m_transfer_function[1].m_color_variable_min = min;
    m_client_message.m_transfer_function[1].m_color_variable_max = max;
    m_client_message.m_transfer_function[1].m_opacity_variable_min = 0;
    m_client_message.m_transfer_function[1].m_opacity_variable_max = 1;
    m_client_message.m_transfer_function[1].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    m_client_message.m_transfer_function[1].m_name ="C2";
    m_client_message.m_transfer_function[1].m_color_variable = "_F2_VAR_C";
    m_client_message.m_transfer_function[1].m_opacity_variable ="";
    m_client_message.m_transfer_function[1].m_range_initialized = 1;

    m_client_message.m_transfer_function[2].m_resolution = 256;
    m_client_message.m_transfer_function[2].m_equation_red = "";
    m_client_message.m_transfer_function[2].m_equation_green = "";
    m_client_message.m_transfer_function[2].m_equation_blue = "";
    m_client_message.m_transfer_function[2].m_equation_opacity = "";
    m_client_message.m_transfer_function[2].m_color_variable_min = min;
    m_client_message.m_transfer_function[2].m_color_variable_max = max;
    m_client_message.m_transfer_function[2].m_opacity_variable_min = 0;
    m_client_message.m_transfer_function[2].m_opacity_variable_max = 1;
    m_client_message.m_transfer_function[2].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    m_client_message.m_transfer_function[2].m_name ="C3";
    m_client_message.m_transfer_function[2].m_color_variable = "_F3_VAR_C";
    m_client_message.m_transfer_function[2].m_opacity_variable ="";
    m_client_message.m_transfer_function[2].m_range_initialized = 1;

    m_client_message.m_transfer_function[3].m_resolution = 256;
    m_client_message.m_transfer_function[3].m_equation_red = "";
    m_client_message.m_transfer_function[3].m_equation_green = "";
    m_client_message.m_transfer_function[3].m_equation_blue = "";
    m_client_message.m_transfer_function[3].m_equation_opacity = "";
    m_client_message.m_transfer_function[3].m_color_variable_min = min;
    m_client_message.m_transfer_function[3].m_color_variable_max = max;
    m_client_message.m_transfer_function[3].m_opacity_variable_min = 0;
    m_client_message.m_transfer_function[3].m_opacity_variable_max = 1;
    m_client_message.m_transfer_function[3].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    m_client_message.m_transfer_function[3].m_name ="C4";
    m_client_message.m_transfer_function[3].m_color_variable = "_F4_VAR_C";
    m_client_message.m_transfer_function[3].m_opacity_variable ="";
    m_client_message.m_transfer_function[3].m_range_initialized = 1;

    m_client_message.m_transfer_function[4].m_resolution = 256;
    m_client_message.m_transfer_function[4].m_equation_red = "";
    m_client_message.m_transfer_function[4].m_equation_green = "";
    m_client_message.m_transfer_function[4].m_equation_blue = "";
    m_client_message.m_transfer_function[4].m_equation_opacity = "";
    m_client_message.m_transfer_function[4].m_color_variable_min = min;
    m_client_message.m_transfer_function[4].m_color_variable_max = max;
    m_client_message.m_transfer_function[4].m_opacity_variable_min = 0;
    m_client_message.m_transfer_function[4].m_opacity_variable_max = 1;
    m_client_message.m_transfer_function[4].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    m_client_message.m_transfer_function[4].m_name ="C5";
    m_client_message.m_transfer_function[4].m_color_variable = "_F5_VAR_C";
    m_client_message.m_transfer_function[4].m_opacity_variable ="";
    m_client_message.m_transfer_function[4].m_range_initialized = 1;

    m_client_message.m_transfer_function[5].m_resolution = 256;
    m_client_message.m_transfer_function[5].m_equation_red = "";
    m_client_message.m_transfer_function[5].m_equation_green = "";
    m_client_message.m_transfer_function[5].m_equation_blue = "";
    m_client_message.m_transfer_function[5].m_equation_opacity = "";
    m_client_message.m_transfer_function[5].m_color_variable_min = 0;
    m_client_message.m_transfer_function[5].m_color_variable_max = 1;
    m_client_message.m_transfer_function[5].m_opacity_variable_min = min;
    m_client_message.m_transfer_function[5].m_opacity_variable_max = max;
    m_client_message.m_transfer_function[5].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    m_client_message.m_transfer_function[5].m_name ="O1";
    m_client_message.m_transfer_function[5].m_color_variable = "";
    m_client_message.m_transfer_function[5].m_opacity_variable ="_F1_VAR_O";
    m_client_message.m_transfer_function[5].m_range_initialized = 1;

    m_client_message.m_transfer_function[6].m_resolution = 256;
    m_client_message.m_transfer_function[6].m_equation_red = "";
    m_client_message.m_transfer_function[6].m_equation_green = "";
    m_client_message.m_transfer_function[6].m_equation_blue = "";
    m_client_message.m_transfer_function[6].m_equation_opacity = "";
    m_client_message.m_transfer_function[6].m_color_variable_min = 0;
    m_client_message.m_transfer_function[6].m_color_variable_max = 1;
    m_client_message.m_transfer_function[6].m_opacity_variable_min = min;
    m_client_message.m_transfer_function[6].m_opacity_variable_max = max;
    m_client_message.m_transfer_function[6].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    m_client_message.m_transfer_function[6].m_name ="O2";
    m_client_message.m_transfer_function[6].m_color_variable = "";
    m_client_message.m_transfer_function[6].m_opacity_variable ="_F2_VAR_O";
    m_client_message.m_transfer_function[6].m_range_initialized = 1;

    m_client_message.m_transfer_function[7].m_resolution = 256;
    m_client_message.m_transfer_function[7].m_equation_red = "";
    m_client_message.m_transfer_function[7].m_equation_green = "";
    m_client_message.m_transfer_function[7].m_equation_blue = "";
    m_client_message.m_transfer_function[7].m_equation_opacity = "";
    m_client_message.m_transfer_function[7].m_color_variable_min = 0;
    m_client_message.m_transfer_function[7].m_color_variable_max = 1;
    m_client_message.m_transfer_function[7].m_opacity_variable_min = min;
    m_client_message.m_transfer_function[7].m_opacity_variable_max = max;
    m_client_message.m_transfer_function[7].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    m_client_message.m_transfer_function[7].m_name ="O3";
    m_client_message.m_transfer_function[7].m_color_variable = "";
    m_client_message.m_transfer_function[7].m_opacity_variable ="_F3_VAR_O";
    m_client_message.m_transfer_function[7].m_range_initialized = 1;

    m_client_message.m_transfer_function[8].m_resolution = 256;
    m_client_message.m_transfer_function[8].m_equation_red = "";
    m_client_message.m_transfer_function[8].m_equation_green = "";
    m_client_message.m_transfer_function[8].m_equation_blue = "";
    m_client_message.m_transfer_function[8].m_equation_opacity = "";
    m_client_message.m_transfer_function[8].m_color_variable_min = 0;
    m_client_message.m_transfer_function[8].m_color_variable_max = 1;
    m_client_message.m_transfer_function[8].m_opacity_variable_min = min;
    m_client_message.m_transfer_function[8].m_opacity_variable_max = max;
    m_client_message.m_transfer_function[8].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    m_client_message.m_transfer_function[8].m_name ="O4";
    m_client_message.m_transfer_function[8].m_color_variable = "";
    m_client_message.m_transfer_function[8].m_opacity_variable ="_F4_VAR_O";
    m_client_message.m_transfer_function[8].m_range_initialized = 1;

    m_client_message.m_transfer_function[9].m_resolution = 256;
    m_client_message.m_transfer_function[9].m_equation_red = "";
    m_client_message.m_transfer_function[9].m_equation_green = "";
    m_client_message.m_transfer_function[9].m_equation_blue = "";
    m_client_message.m_transfer_function[9].m_equation_opacity = "";
    m_client_message.m_transfer_function[9].m_color_variable_min = 0;
    m_client_message.m_transfer_function[9].m_color_variable_max = 1;
    m_client_message.m_transfer_function[9].m_opacity_variable_min = min;
    m_client_message.m_transfer_function[9].m_opacity_variable_max = max;
    m_client_message.m_transfer_function[9].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    m_client_message.m_transfer_function[9].m_name ="O5";
    m_client_message.m_transfer_function[9].m_color_variable = "";
    m_client_message.m_transfer_function[9].m_opacity_variable ="_F5_VAR_O";
    m_client_message.m_transfer_function[9].m_range_initialized = 1;

    m_client_message.m_volume_equation.resize(10);
    m_client_message.m_volume_equation[0].m_name = "_F1_VAR_C";
    m_client_message.m_volume_equation[0].m_equation = "q1";

    m_client_message.m_volume_equation[1].m_name = "_F2_VAR_C";
    m_client_message.m_volume_equation[1].m_equation = "q1";

    m_client_message.m_volume_equation[2].m_name = "_F3_VAR_C";
    m_client_message.m_volume_equation[2].m_equation = "q1";

    m_client_message.m_volume_equation[3].m_name = "_F4_VAR_C";
    m_client_message.m_volume_equation[3].m_equation = "q1";

    m_client_message.m_volume_equation[4].m_name = "_F5_VAR_C";
    m_client_message.m_volume_equation[4].m_equation = "q1";

    m_client_message.m_volume_equation[5].m_name = "_F1_VAR_O";
    m_client_message.m_volume_equation[5].m_equation = "q1";

    m_client_message.m_volume_equation[6].m_name = "_F2_VAR_O";
    m_client_message.m_volume_equation[6].m_equation = "q1";

    m_client_message.m_volume_equation[7].m_name = "_F3_VAR_O";
    m_client_message.m_volume_equation[7].m_equation = "q1";

    m_client_message.m_volume_equation[8].m_name = "_F4_VAR_O";
    m_client_message.m_volume_equation[8].m_equation = "q1";

    m_client_message.m_volume_equation[9].m_name = "_F5_VAR_O";
    m_client_message.m_volume_equation[9].m_equation = "q1";

    m_client_message.m_color_transfer_function_synthesis = "C1";
    m_client_message.m_opacity_transfer_function_synthesis = "O1";

    std::cout << "[color_func]" << std::endl;
    m_client_message.color_func = convertToken("c1");
    std::cout << "[opacity_func]" << std::endl;
    m_client_message.opacity_func = convertToken("a1");

    //    message.color_var.resize(5);
    //    message.opacity_var.resize(5);

    //    std::cout << "[color_var0]" << std::endl;
    //    message.color_var.push_back(convertToken("q1"));
    //    std::cout << "[opacity_var0]" << std::endl;
    //    message.opacity_var.push_back(convertToken("q1"));

    //    std::cout << "[color_var1]" << std::endl;
    //    message.color_var.push_back(convertToken("q1"));
    //    std::cout << "[opacity_var1]" << std::endl;
    //    message.opacity_var.push_back(convertToken("q1"));

    //    std::cout << "[color_var2]" << std::endl;
    //    message.color_var.push_back(convertToken("q1"));
    //    std::cout << "[opacity_var2]" << std::endl;
    //    message.opacity_var.push_back(convertToken("q1"));

    //    std::cout << "[color_var3]" << std::endl;
    //    message.color_var.push_back(convertToken("q1"));
    //    std::cout << "[opacity_var3]" << std::endl;
    //    message.opacity_var.push_back(convertToken("q1"));

    //    std::cout << "[color_var4]" << std::endl;
    //    message.color_var.push_back(convertToken("q1"));
    //    std::cout << "[opacity_var4]" << std::endl;
    //    message.opacity_var.push_back(convertToken("q1"));

    m_client_message.color_var.push_back( convertToken( "q1" ) );
    m_client_message.color_var.push_back( convertToken( "q1" ) );
    m_client_message.color_var.push_back( convertToken( "q1" ) );
    m_client_message.color_var.push_back( convertToken( "q1" ) );
    m_client_message.color_var.push_back( convertToken( "q1" ) );

    m_client_message.opacity_var.push_back( convertToken( "q1" ) );
    m_client_message.opacity_var.push_back( convertToken( "q1" ) );
    m_client_message.opacity_var.push_back( convertToken( "q1" ) );
    m_client_message.opacity_var.push_back( convertToken( "q1" ) );
    m_client_message.opacity_var.push_back( convertToken( "q1" ) );

    //    message.x_synthesis_token = convertToken( message.m_x_synthesis );
    //    message.y_synthesis_token = convertToken( message.m_y_synthesis );
    //    message.z_synthesis_token = convertToken( message.m_z_synthesis );

    m_client_message.m_message_size = m_client_message.byteSize();
    std::cout << "SEND" << std::endl;
    client.sendMessage( m_client_message );
    client.recvMessage( &reply );

    if ( client.recvMessage( &reply ) == 1 ){}
    //    if ( client.recvMessage( &reply ) == 1 ){}

    size_t allParticle = 0;
    kvs::PointObject* object = new kvs::PointObject();

    int nmemb = reply.m_number_particle * 3;
    if ( nmemb != 0 )
    {
        kvs::ValueArray<kvs::Real32> positions ( reply.m_positions, nmemb );
        kvs::ValueArray<kvs::Real32> normals ( reply.m_normals, nmemb );
        kvs::ValueArray<kvs::UInt8>  colors ( reply.m_colors, nmemb );

        kvs::PointObject obj;
        obj.setCoords( positions );
        obj.setNormals( normals );
        obj.setColors( colors );

        object->add(obj);
        obj.clear();
        std::cout<<" getPointObjectFromServer 331"<<std::endl;
        allParticle = allParticle + reply.m_number_particle;
        delete[] reply.m_colors;
        delete[] reply.m_normals;
        delete[] reply.m_positions;
    }
    kvs::PointObject* pointObject = object;

    kvs::Vector3f serverSideMinObjectCoords;
    kvs::Vector3f serverSideMaxObjectCoords;
    serverSideMinObjectCoords[0] = reply.m_min_object_coord[0];
    serverSideMinObjectCoords[1] = reply.m_min_object_coord[1];
    serverSideMinObjectCoords[2] = reply.m_min_object_coord[2];
    serverSideMaxObjectCoords[0] = reply.m_max_object_coord[0];
    serverSideMaxObjectCoords[1] = reply.m_max_object_coord[1];
    serverSideMaxObjectCoords[2] = reply.m_max_object_coord[2];
    pointObject->setMinMaxObjectCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );
    pointObject->setMinMaxExternalCoords( serverSideMinObjectCoords, serverSideMaxObjectCoords );

    std::cout << serverSideMinObjectCoords[0] << std::endl;
    std::cout << serverSideMinObjectCoords[1] << std::endl;
    std::cout << serverSideMinObjectCoords[2] << std::endl;
    std::cout << serverSideMaxObjectCoords[0] << std::endl;
    std::cout << serverSideMaxObjectCoords[1] << std::endl;
    std::cout << serverSideMaxObjectCoords[2] << std::endl;

    m_client_message.m_initialize_parameter = -1;
    m_client_message.m_message_size = m_client_message.byteSize();
    client.sendMessage( m_client_message );
    client.recvMessage( &reply );
    client.termClient();
//    pointObject->updateMinMaxCoords();


    return pointObject;

}

void Connect::onConnectButtonClicked()
{
    connect1();
}
