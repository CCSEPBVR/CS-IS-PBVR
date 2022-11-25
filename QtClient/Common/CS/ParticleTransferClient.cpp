#if defined JPV_CLIENT

#include "ParticleTransferClient.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <fstream>

#if defined WIN32
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

// APPEND START fp)m.tanaka 2013.09.01
extern void update_sending();
extern void update_recieving();
//extern void update_smemory();
extern void update_gmemory();
extern char data_filename[100];
extern char limitdata_filename[100];
//extern int plimitlevel;
//extern int data_cnt;
extern bool data_output;
// APPEND END   fp)m.tanaka 2013.09.01
extern size_t c_smemory;
#define HOST_PARTICLE_LIMIT 20000000
#define MEMORY_USE_RATE 0.9

jpv::ParticleTransferClient::ParticleTransferClient( const std::string& _host, const int _port ):
    m_hostname( _host ),
    m_port( _port )
{
}

jpv::ParticleTransferClient::~ParticleTransferClient()
{
}

int jpv::ParticleTransferClient::initClient()
{
    struct hostent* servhost;
    struct sockaddr_in dest;
    memset( &dest, 0, sizeof( dest ) );

#if defined WIN32
    WSADATA data;
    WSAStartup( MAKEWORD( 2, 0 ), &data );
#endif
    servhost = gethostbyname( m_hostname.c_str() );
    if ( servhost == NULL )
    {
        std::cout << "resolv error: " << m_hostname << std::endl;
        return -1;
    }

    dest.sin_port = htons( m_port );
    dest.sin_family = AF_INET;
    memcpy( &dest.sin_addr, servhost->h_addr, servhost->h_length );

    m_sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( connect( m_sock, ( struct sockaddr* ) &dest, sizeof( dest ) ) == -1 )
    {
        std::cout << "connection error" << std::endl;
        return -1;
    }
    return 0;
}

int jpv::ParticleTransferClient::termClient()
{

#if defined WIN32
    shutdown( m_sock, SD_BOTH );
    std::cout << "shutdown connection" << std::endl;
    closesocket( m_sock );
    std::cout << "close socket" << std::endl;
    WSACleanup();
#else
    shutdown( m_sock, SHUT_RDWR );
    std::cout << "shutdown connection" << std::endl;
    close( m_sock );
    std::cout << "close m_socket" << std::endl;
#endif
    return 0;
}

int jpv::ParticleTransferClient::sendMessage( const ParticleTransferClientMessage& message )
{
    std::cout << "[DEBUG protocol start]" << std::endl;
    std::cout << "m_header[0]                                                 :" << message.m_header[0] << std::endl;//
    std::cout << "m_header[1]                                                 :" << message.m_header[1] << std::endl;
    std::cout << "m_header[2]                                                 :" << message.m_header[2] << std::endl;
    std::cout << "m_header[3]                                                 :" << message.m_header[3] << std::endl;
    std::cout << "m_header[4]                                                 :" << message.m_header[4] << std::endl;
    std::cout << "m_header[5]                                                 :" << message.m_header[5] << std::endl;
    std::cout << "m_header[6]                                                 :" << message.m_header[6] << std::endl;
    std::cout << "m_header[7]                                                 :" << message.m_header[7] << std::endl;
    std::cout << "m_header[8]                                                 :" << message.m_header[8] << std::endl;
    std::cout << "m_header[9]                                                 :" << message.m_header[9] << std::endl;
    std::cout << "m_header[10]                                                :" << message.m_header[10] << std::endl;
    std::cout << "m_message_size                                              :" << message.m_message_size << std::endl;
    std::cout << "m_initialize_parameter                                      :" << message.m_initialize_parameter << std::endl;
    std::cout << "m_sampling_method                                           :" << message.m_sampling_method << std::endl;
    std::cout << "m_subpixel_level                                            :" << message.m_subpixel_level << std::endl;
    std::cout << "m_repeat_level                                              :" << message.m_repeat_level << std::endl;
    std::cout << "m_shuffle_method                                            :" << message.m_shuffle_method << std::endl;
    std::cout << "m_node_type                                                 :" << message.m_node_type << std::endl;
    std::cout << "m_sampling_step                                             :" << message.m_sampling_step << std::endl;
    std::cout << "m_rendering_id                                              :" << message.m_rendering_id << std::endl;
    std::cout << "m_camera                                                    :" << message.m_camera << std::endl;
    std::cout << "m_time_parameter                                            :" << message.m_time_parameter << std::endl;
    std::cout << "m_begin_time                                                :" << message.m_begin_time << std::endl;
    std::cout << "m_end_time                                                  :" << message.m_end_time << std::endl;
    std::cout << "m_memory_size                                               :" << message.m_memory_size << std::endl;
    std::cout << "m_step                                                      :" << message.m_step << std::endl;
    std::cout << "m_trans_Parameter                                           :" << message.m_trans_Parameter << std::endl;
    std::cout << "m_level_index                                               :" << message.m_level_index << std::endl;
    std::cout << "m_enable_crop_region                                        :" << message.m_enable_crop_region << std::endl;
    std::cout << "m_crop_region[0]                                            :" << message.m_crop_region[0] << std::endl;//
    std::cout << "m_crop_region[1]                                            :" << message.m_crop_region[1] << std::endl;
    std::cout << "m_crop_region[2]                                            :" << message.m_crop_region[2] << std::endl;
    std::cout << "m_crop_region[3]                                            :" << message.m_crop_region[3] << std::endl;
    std::cout << "m_crop_region[4]                                            :" << message.m_crop_region[4] << std::endl;
    std::cout << "m_crop_region[5]                                            :" << message.m_crop_region[5] << std::endl;
    std::cout << "m_particle_limit                                            :" << message.m_particle_limit << std::endl;
    std::cout << "m_particle_density                                          :" << message.m_particle_density << std::endl;
//    std::cout << "particle_data_size_limit             :" << message.particle_data_size_limit << std::endl;//
    std::cout << "m_input_directory                                           :" << message.m_input_directory << std::endl;
    std::cout << "m_filter_parameter_filename                                 :" << message.m_filter_parameter_filename << std::endl;
    std::cout << "m_x_synthesis                                               :" << message.m_x_synthesis << std::endl;
    std::cout << "m_y_synthesis                                               :" << message.m_y_synthesis << std::endl;
    std::cout << "m_z_synthesis                                               :" << message.m_z_synthesis << std::endl;

    //    std::cout << "m_transfer_function" << message.m_transfer_function << std::endl;
    //    std::cout << "m_volume_equation" << message.m_volume_equation << std::endl;
    std::cout << "[m_transfer_function FROM]" << std::endl;
    for(int i = 0; i < message.m_transfer_function.size(); i++){
        std::cout << "m_transfer_function[" << i << "].m_resolution                         :" << message.m_transfer_function[i].m_resolution << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_equation_red                       :" << message.m_transfer_function[i].m_equation_red << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_equation_green                     :" << message.m_transfer_function[i].m_equation_green << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_equation_blue                      :" << message.m_transfer_function[i].m_equation_blue << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_equation_opacity                   :" << message.m_transfer_function[i].m_equation_opacity << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_color_variable_min                 :" << message.m_transfer_function[i].m_color_variable_min << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_color_variable_max                 :" << message.m_transfer_function[i].m_color_variable_max << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_opacity_variable_min               :" << message.m_transfer_function[i].m_opacity_variable_min << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_opacity_variable_max               :" << message.m_transfer_function[i].m_opacity_variable_max << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_selection                          :" << message.m_transfer_function[i].m_selection << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_name                               :" << message.m_transfer_function[i].m_name << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_color_variable                     :" << message.m_transfer_function[i].m_color_variable << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_opacity_variable                   :" << message.m_transfer_function[i].m_opacity_variable << std::endl;
        std::cout << "m_transfer_function[" << i << "].m_range_initialized                  :" << message.m_transfer_function[i].m_range_initialized << std::endl;
    }
    std::cout << "[m_transfer_function  TO ]" << std::endl;

    std::cout << "[m_volume_equation   FROM]" << std::endl;
    for(int i = 0; i < message.m_volume_equation.size(); i++){
        std::cout << "m_volume_equation[" << i << "].m_name                                 :" << message.m_volume_equation[i].m_name << std::endl;
        std::cout << "m_volume_equation[" << i << "].m_equation                             :" << message.m_volume_equation[i].m_equation << std::endl;
    }
    std::cout << "[m_volume_equation    TO ]" << std::endl;
    std::cout << "m_transfer_function_synthesis                               :" << message.m_transfer_function_synthesis << std::endl;
    std::cout << "m_color_transfer_function_synthesis                         :" << message.m_color_transfer_function_synthesis << std::endl;
    std::cout << "m_opacity_transfer_function_synthesis                       :" << message.m_opacity_transfer_function_synthesis << std::endl;
    std::cout << "[DEBUG protocol end]" << std::endl;
    int size = message.byteSize();
//#ifdef WIN32
//	char* buf = new char [size+1];
//#else
    char* buf = new char [size];
//#endif

    std::cout << "Send Client Message Size = " << size << std::endl;

//  int th = omp_get_thread_num();
//  if (th == 1) {
// APPEND START fp)m.tanaka 2013.09.01
//	std::cout << "th==1 update_sending" << std::endl;
//	update_sending();
// APPEND END   fp)m.tanaka 2013.09.01
//  }
//  if (th == 0) {
    message.pack( buf );
    send( m_sock, buf, size, 0 );

#ifdef _DEBUG
    std::ofstream output;
    output.open( "debug_client.send" );
    output.write( buf, size );
    output.flush();
    output.close();
#endif

    delete[] buf;
// }
    return 0;
}

// APPEND START fp)m.tanaka 2014.03.11
static int m_number_particle = 0;
static int particeCnt = 0;
extern char transferType; // 'A':abstruct 'D':detail
static char datafname[256];
// APPEND END   fp)m.tanaka 2014.03.11

int jpv::ParticleTransferClient::recvMessage( ParticleTransferServerMessage* message )
{
    // サーバーメッセージの受信
//    const size_t hSize = sizeof( message->m_header ) + sizeof( message->m_message_size );
    const size_t hSize = sizeof( message->m_header ) + sizeof( message->m_message_size )
                         + sizeof( message->m_server_status ); // 2015.12.23 Add Param

    char hsbuf[hSize];
    char* buf;
    std::stringstream ss;
    int recvSize( 0 );
    size_t mSize, rSize;
    size_t host_particle_limit; //HOST_PARTICLE_LIMIT

    // tantantan
    //MOVED TO 236)T.Osaki 2020.03.05
    //fprintf( stdout, "message->m_time_step : %d\n",  message->m_time_step );
    fprintf( stdout, "transferType : %c\n", transferType );

//  int th = omp_get_thread_num();
//  if (th == 0) {
    message->m_message_size = 0;
    for ( rSize = 0; rSize < hSize; rSize += recvSize )
    {
        recvSize = recv( m_sock, hsbuf, hSize - rSize, 0 );
// MODIFIED START Fj 2015.03.04
        if ( recvSize < 0 )
        {
            recvSize = 0;
        }
        else
        {
            ss.write( hsbuf, recvSize );
        }
// MODIFIED END   Fj 2015.03.04
    }
    ss.seekg( sizeof( message->m_header ) );
    ss.read( reinterpret_cast<char*>( &message->m_message_size ), sizeof( message->m_message_size ) );
    // MODIFIED START Fj 2015.12.23
    ss.read( reinterpret_cast<char*>( &message->m_server_status ), sizeof( message->m_server_status ) );
    // MODIFIED END   FEAST 2015.12.23

    std::cout << "Receive Server Message Size = " << message->m_message_size << std::endl;
    std::cout << "Receive Server Status(test) = " << message->m_server_status << std::endl;
    if ( message->m_server_status == 1 )
    {
        std::cout << "NaN Catch !!" << std::endl;
        return 1;
    }

    mSize = message->m_message_size - hSize;
    buf = new char[mSize];
    for ( rSize = 0; rSize < mSize; rSize += recvSize )
    {
        recvSize = recv( m_sock, buf, mSize - rSize, 0 );
// MODIFIED START Fj 2015.03.04
        if ( recvSize < 0 )
        {
            recvSize = 0;
        }
        else
        {
            ss.write( buf, recvSize );
        }
// MODIFIED END   Fj 2015.03.04
    }
    delete[] buf;

    message->unpack_message( ss.str().c_str() );
    //MOVED FROM 181)T.Osaki 2020.03.05
    fprintf( stdout, "message->m_time_step : %d\n",  message->m_time_step );
    std::cout << "Receive Particle Size = " << message->m_number_particle << std::endl;
//  }
//  if (th == 1) {
//	std::cout << "update_recieving update_smemory update_gmemory" << std::endl;
// APPEND START fp)m.tanaka 2013.09.01
//	update_recieving();
//	update_smemory();
//	update_gmemory();
// APPEND END   fp)m.tanaka 2013.09.01
//  }
//  if (th == 0) {
#ifndef OLD_CORD
// MODIFIED START fp)m.tanaka 2014.03.11

    m_number_particle = message->m_number_particle;
    host_particle_limit = ( c_smemory * 1024 * 1024 ) / ( 12 + 12 + 3 );
//std::cout << "host_particle_limit0 = " << host_particle_limit << std::endl;
    host_particle_limit = host_particle_limit * MEMORY_USE_RATE;
//std::cout << "host_particle_limit1 = " << host_particle_limit << std::endl;
//	std::cout << "c_smemory = " << c_smemory << std::endl;
//	std::cout << "m_number_particle=" << m_number_particle << "  host_particle_limit=" << host_particle_limit << std::endl;
//  fprintf(stdout, "***** message->m_number_particle : %d\n", message->m_number_particle);

    // modify by @hira at 2016/12/01
    // if ( m_number_particle > host_particle_limit )
    if ( host_particle_limit > 0
        && m_number_particle > host_particle_limit )
    {

        // 1) all data save to file
        int devnum = m_number_particle / host_particle_limit;
        std::cout << "c_smemory = " << c_smemory << std::endl;
        std::cout << "m_number_particle=" << m_number_particle << "  host_particle_limit=" << host_particle_limit << std::endl;
        std::cout << "devnum = " << devnum << std::endl;

        dataToFile( 0, NULL, 0 );

        for ( int i = 0; i < devnum; i++ )
        {

            const size_t pSize = ( 12 + 12 + 3 ) * host_particle_limit;
            buf = new char[pSize];
            for ( rSize = 0; rSize < pSize; rSize += recvSize )
            {
                recvSize = recv( m_sock, buf, pSize - rSize, 0 );

                dataToFile( 1, buf, recvSize );
            }
            delete[] buf;

        }

        // remain
        int remainnum = m_number_particle % host_particle_limit;
        if ( remainnum != 0 )
        {

            const size_t pSize = ( 12 + 12 + 3 ) * remainnum;
            buf = new char[pSize];
            for ( rSize = 0; rSize < pSize; rSize += recvSize )
            {
                recvSize = recv( m_sock, buf, pSize - rSize, 0 );

                dataToFile( 1, buf, recvSize );
            }
            delete[] buf;

        }

        dataToFile( 2, NULL, 0 );

        // restore from file

        dataFromFile( 0, NULL, 0 );

        for ( int i = 0; i < devnum; i++ )
        {

            const size_t pSize = ( 12 + 12 + 3 ) * host_particle_limit;
            buf = new char[pSize];

            dataFromFile( 1, buf, pSize );

            ss.write( buf, pSize );

            delete[] buf;
        }

        if ( remainnum != 0 )
        {

            const size_t pSize = ( 12 + 12 + 3 ) * remainnum;
            buf = new char[pSize];

            dataFromFile( 1, buf, pSize );
            ss.write( buf, pSize );
            delete[] buf;
        }

        message->unpack_particles( ss.str().c_str() );

        if ( data_output )
        {
            //sprintf(datafname, "%s_%03d_%c_%03d.dat", data_filename, data_cnt++, transferType, message->m_time_step);
            sprintf( datafname, "%s_%c_%03d.dat", data_filename, transferType, message->m_time_step );
            std::ofstream output;
            //output.open( datafname, std::ios::out | std::ios::app);
            output.open( datafname, std::ios::out );
            output.write( ss.str().c_str(), message->m_message_size + ( 12 + 12 + 3 ) * message->m_number_particle );
            output.flush();
            output.close();
        }

        dataFromFile( 2, NULL, 0 );

    }
    else
    {

        // orignal cording if m_number_particle <= plimitlevel

        if ( message->m_number_particle > 0 )
        {
            const size_t pSize = ( 12 + 12 + 3 ) * message->m_number_particle;
            buf = new char[pSize];
            for ( rSize = 0; rSize < pSize; rSize += recvSize )
            {
                recvSize = recv( m_sock, buf, pSize - rSize, 0 );
                ss.write( buf, recvSize );
            }
            message->unpack_particles( ss.str().c_str() );
            delete[] buf;

            if ( data_output )
            {
                //sprintf(datafname, "%s_%03d_%c_%03d.dat", data_filename, data_cnt++, transferType, message->m_time_step);
                sprintf( datafname, "%s_%c_%03d.dat", data_filename, transferType, message->m_time_step );
                std::ofstream output;
                //output.open( datafname, std::ios::out | std::ios::app);
                output.open( datafname, std::ios::out );
                output.write( ss.str().c_str(), message->m_message_size + pSize );
                output.flush();
                output.close();
            }

        }
    }

// MODIFIED END   fp)m.tanaka 2014.03.11
#else
    if ( message.m_number_particle > 0 )
    {
        const size_t pSize = ( 12 + 12 + 3 ) * message.m_number_particle;
        buf = new char[pSize];
        for ( rSize = 0; rSize < pSize; rSize += recvSize )
        {
            recvSize = recv( sock, buf, pSize - rSize, 0 );
            ss.write( buf, recvSize );
        }
        message.unpack_particles( ss.str().c_str() );
        delete[] buf;

// APPEND START fp)m.tanaka 2014.03.11
        //printf("%s\n", data_filename);

//		std::ofstream output;
//		output.open( data_filename, std::ios::out | std::ios::app);
//		output.write( ss.str().c_str(), message.m_message_size + pSize );
//		output.flush();
//		output.close();
// APPEND END   fp)m.tanaka 2014.03.11

    }

#endif

#ifdef _DEBUG
    std::ofstream output;
    output.open( "debug_client.recv" );
//	output.write( ss.str().c_str(), message->m_message_size + pSize );
    output.flush();
    output.close();
#endif
// }
    return 0;
}

// APPEND START fp)m.tanaka 2014.03.11
static FILE* fpdata;
int jpv::ParticleTransferClient::dataToFile( const int mode, char* buff, const int size )
{
    switch ( mode )
    {
    case 0:
        if ( ( fpdata = fopen( limitdata_filename, "wb" ) ) == NULL )
        {
            return 1;
        }
        break;
    case 1:
        fwrite( buff, 1, size, fpdata );
        fflush( fpdata );
        break;
    case 2:
        fclose( fpdata );
        break;
    }
    return 0;
}

int jpv::ParticleTransferClient::dataFromFile( const int mode, char* buff, const int size )
{
    switch ( mode )
    {
    case 0:
        if ( ( fpdata = fopen( limitdata_filename, "rb" ) ) == NULL )
        {
            return 1;
        }
        break;
    case 1:
        fread( buff, 1, size, fpdata );
        break;
    case 2:
        fclose( fpdata );
        break;
    }
    return 0;
}
// APPEND END   fp)m.tanaka 2014.03.11


#endif

