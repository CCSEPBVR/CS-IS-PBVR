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

jpv::ParticleTransferClient::ParticleTransferClient( std::string _host, int _port )
    : m_hostname( _host ), m_port( _port )
{
}

jpv::ParticleTransferClient::~ParticleTransferClient( void )
{
}

int jpv::ParticleTransferClient::initClient( void )
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
                exit(53);
        return -1;
    }

    dest.sin_port = htons( m_port );
    dest.sin_family = AF_INET;
    memcpy( &dest.sin_addr, servhost->h_addr, servhost->h_length );
    std::cout << "connecting to:"<<m_hostname<<":"<<m_port << std::endl;
    m_sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( connect( m_sock, ( struct sockaddr* ) &dest, sizeof( dest ) ) == -1 )
    {
        std::cout << "connection error" << std::endl;
        exit(55);
        return -1;
    }
    return 0;
}

int jpv::ParticleTransferClient::termClient( void )
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
    std::cout << "close socket" << std::endl;
#endif
    return 0;
}

int jpv::ParticleTransferClient::sendMessage( ParticleTransferClientMessage& message )
{
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
    return 0;
}

// APPEND START fp)m.tanaka 2014.03.11
static int numParticle = 0;
static int particeCnt = 0;
extern char transferType; // 'A':abstruct 'D':detail
static char datafname[256];
// APPEND END   fp)m.tanaka 2014.03.11

int jpv::ParticleTransferClient::recvMessage( ParticleTransferServerMessage& message )
{
    // �T�[�o�[���b�Z�[�W�̎��M
    const size_t hSize = sizeof( message.m_header ) + sizeof( message.m_message_size );
    char hsbuf[hSize];
    char* buf;
    std::stringstream ss;
    int recvSize( 0 );
    size_t mSize, rSize;
    size_t host_particle_limit; //HOST_PARTICLE_LIMIT

    // tantantan
    fprintf( stdout, "message.timeStep : %d\n",  message.m_time_step );
    fprintf( stdout, "transferType : %c\n", transferType );

//  int th = omp_get_thread_num();
//  if (th == 0) {
        message.m_message_size = 0;
        for ( rSize = 0; rSize < hSize; rSize += recvSize )
        {
//            fprintf( stdout,"ParticleTransferClient_Debug0");
            recvSize = recv( m_sock, hsbuf, hSize - rSize, 0 );
// MODIFIED START Fj 2015.03.04
            if ( recvSize < 0 )
            {
                recvSize = 0;
            }
            else
            {
//                        fprintf( stdout,"ParticleTransferClient_Debug1");
                ss.write( hsbuf, recvSize );
            }
// MODIFIED END   Fj 2015.03.04
        }
//        fprintf( stdout,"ParticleTransferClient_Debug2");
        ss.seekg( sizeof( message.m_header ) );
//                fprintf( stdout,"ParticleTransferClient_Debug3");
        ss.read( reinterpret_cast<char*>( &message.m_message_size ), sizeof( message.m_message_size ) );
        std::cout << "Receive Server Message Size = " << message.m_message_size << std::endl;

        mSize = message.m_message_size - hSize;
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

        message.unpack_message( ss.str().c_str() );
        std::cout << "Receive Particle Size = " << message.m_number_particle << std::endl;
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

        numParticle = message.m_number_particle;
        host_particle_limit = ( c_smemory * 1024 * 1024 ) / ( 12 + 12 + 3 );
//std::cout << "host_particle_limit0 = " << host_particle_limit << std::endl;
        host_particle_limit = host_particle_limit * MEMORY_USE_RATE;
//std::cout << "host_particle_limit1 = " << host_particle_limit << std::endl;
//	std::cout << "c_smemory = " << c_smemory << std::endl;
//	std::cout << "numParticle=" << numParticle << "  host_particle_limit=" << host_particle_limit << std::endl;
//  fprintf(stdout, "***** message.numParticle : %d\n", message.numParticle);

        if ( numParticle > host_particle_limit )
        {

            // 1) all data save to file
            int devnum = numParticle / host_particle_limit;
            std::cout << "c_smemory = " << c_smemory << std::endl;
            std::cout << "numParticle=" << numParticle << "  host_particle_limit=" << host_particle_limit << std::endl;
            std::cout << "devnum = " << devnum << std::endl;

            datatofile( 0, NULL, NULL );

            for ( int i = 0; i < devnum; i++ )
            {

                const size_t pSize = ( 12 + 12 + 3 ) * host_particle_limit;
                buf = new char[pSize];
                for ( rSize = 0; rSize < pSize; rSize += recvSize )
                {
                    recvSize = recv( m_sock, buf, pSize - rSize, 0 );

                    datatofile( 1, buf, recvSize );
                }
                delete[] buf;

            }

            // remain
            int remainnum = numParticle % host_particle_limit;
            if ( remainnum != 0 )
            {

                const size_t pSize = ( 12 + 12 + 3 ) * remainnum;
                buf = new char[pSize];
                for ( rSize = 0; rSize < pSize; rSize += recvSize )
                {
                    recvSize = recv( m_sock, buf, pSize - rSize, 0 );

                    datatofile( 1, buf, recvSize );
                }
                delete[] buf;

            }

            datatofile( 2, NULL, NULL );

            // restore from file

            datafromfile( 0, NULL, NULL );

            for ( int i = 0; i < devnum; i++ )
            {

                const size_t pSize = ( 12 + 12 + 3 ) * host_particle_limit;
                buf = new char[pSize];

                datafromfile( 1, buf, pSize );

                ss.write( buf, pSize );

                delete[] buf;
            }

            if ( remainnum != 0 )
            {

                const size_t pSize = ( 12 + 12 + 3 ) * remainnum;
                buf = new char[pSize];

                datafromfile( 1, buf, pSize );
                ss.write( buf, pSize );
                delete[] buf;
            }

            message.unpack_particles( ss.str().c_str() );

            if ( data_output )
            {
                //sprintf(datafname, "%s_%03d_%c_%03d.dat", data_filename, data_cnt++, transferType, message.timeStep);
                sprintf( datafname, "%s_%c_%03d.dat", data_filename, transferType, message.m_time_step );
                std::ofstream output;
                //output.open( datafname, std::ios::out | std::ios::app);
                output.open( datafname, std::ios::out );
                output.write( ss.str().c_str(), message.m_message_size + ( 12 + 12 + 3 ) * message.m_number_particle );
                output.flush();
                output.close();
            }

            datafromfile( 2, NULL, NULL );

        }
        else
        {

            // orignal cording if numParticle <= plimitlevel

            if ( message.m_number_particle > 0 )
            {
                const size_t pSize = ( 12 + 12 + 3 ) * message.m_number_particle;
                buf = new char[pSize];
                for ( rSize = 0; rSize < pSize; rSize += recvSize )
                {
                    recvSize = recv( m_sock, buf, pSize - rSize, 0 );
                    ss.write( buf, recvSize );
                }
                message.unpack_particles( ss.str().c_str() );
                delete[] buf;

                if ( data_output )
                {
                    //sprintf(datafname, "%s_%03d_%c_%03d.dat", data_filename, data_cnt++, transferType, message.timeStep);
                    sprintf( datafname, "%s_%c_%03d.dat", data_filename, transferType, message.m_time_step );
                    std::ofstream output;
                    //output.open( datafname, std::ios::out | std::ios::app);
                    output.open( datafname, std::ios::out );
                    output.write( ss.str().c_str(), message.m_message_size + pSize );
                    output.flush();
                    output.close();
                }

            }
        }

// MODIFIED END   fp)m.tanaka 2014.03.11
#else
        if ( message.numParticle > 0 )
        {
            const size_t pSize = ( 12 + 12 + 3 ) * message.numParticle;
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
//		output.write( ss.str().c_str(), message.messageSize + pSize );
//		output.flush();
//		output.close();
// APPEND END   fp)m.tanaka 2014.03.11

        }

#endif

#ifdef _DEBUG
        std::ofstream output;
        output.open( "debug_client.recv" );
//	output.write( ss.str().c_str(), message.messageSize + pSize );
        output.flush();
        output.close();
#endif
// }
    return 0;
}

// APPEND START fp)m.tanaka 2014.03.11
static FILE* fpdata;
int jpv::ParticleTransferClient::datatofile( int mode, char* buff, int size )
{
    switch ( mode )
    {
    case 0:
        if ( ( fpdata = fopen( limitdata_filename, "wb" ) ) == NULL )
        {
            return ( 1 );
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
    return ( 0 );
}

int jpv::ParticleTransferClient::datafromfile( int mode, char* buff, int size )
{
    switch ( mode )
    {
    case 0:
        if ( ( fpdata = fopen( limitdata_filename, "rb" ) ) == NULL )
        {
            return ( 1 );
        }
        break;
    case 1:
        fread( buff, 1, size, fpdata );
        break;
    case 2:
        fclose( fpdata );
        break;
    }
    return ( 0 );
}
// APPEND END   fp)m.tanaka 2014.03.11


#endif

