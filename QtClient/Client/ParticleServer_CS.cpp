#include "ParticleServer.h"

#include "ParticleTransferClient.h"

#include <kvs/File>
#include <kvs/Directory>
#include <kvs/PointImporter>
#include <kvs/PointExporter>
#include <kvs/PointObject>

#include "VisualizationParameter.h"

#include <float.h>

#include <cstring>
#include <cstdlib>
#include <cassert>

#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

#include "v3defines.h"
#include "timer_simple.h"
#include "ParamExTransFunc.h"


#ifdef KVS_COMPILER_VC
#include <direct.h>
#define mkdir( dir, mode ) _mkdir( dir )
#endif

using namespace kvs::visclient;
char transferType; // 'A':abstruct 'D':detail

#ifdef IS_MODE
int lavel_time_step; // should be extern
#endif

#ifdef USE_SERVER_SIDE_PBVR_COORDS
kvs::PointObject* ParticleServer::getPointObjectFromServer( const VisualizationParameter& param, ReceivedMessage* result, const int numvol, const int stepno, float server_side_min_coords[3], float server_side_max_coords[3] )
#else // USE_SERVER_SIDE_PBVR_COORDS
kvs::PointObject* ParticleServer::getPointObjectFromServer( const VisualizationParameter& param, ReceivedMessage* result, const int numvol, const int stepno )
#endif // USE_SERVER_SIDE_PBVR_COORDS
{
    std::cout<<"IN getPointObjectFromServer()"<<std::endl;
    const ExtendedTransferFunctionMessage& paramExTransFunc = param.m_parameter_extend_transfer_function;

    PBVR_TIMER_STA( 100 );
    //if (m_number_repeat == 0) m_parameter_extend_transfer_function.ExTransFuncParaSet();
    m_number_repeat++;
    std::cout << " ### m_number_repeat = " << m_number_repeat << std::endl;
    std::cout << " getPointObjectFromServer param.m_time_step " <<  param.m_time_step << std::endl;
    //#if 0
    jpv::ParticleTransferClient client( param.m_hostname, param.m_port );
    jpv::ParticleTransferServerMessage reply;
    jpv::ParticleTransferClientMessage message;

#ifdef IS_MODE
    reply.camera = new kvs::Camera();
#endif
    std::cout<<" getPointObjectFromServer 64"<<std::endl;
    setStatus( Sending );
    if ( param.m_client_server_mode == 1 )
    {
        client.initClient();
        strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
        message.m_initialize_parameter = 1;
        message.m_rendering_id = param.m_rendering_id;
        if ( param.m_sampling_type == VisualizationParameter::UniformSampling )
        {
            message.m_sampling_method = 'u';
        }
        else if ( param.m_sampling_type == VisualizationParameter::RejectionSampling )
        {
            message.m_sampling_method = 'r';
        }
        else if ( param.m_sampling_type == VisualizationParameter::MetropolisSampling )
        {
            message.m_sampling_method = 'm';
        }
        else
        {
            assert( false );
        }
        std::cout<<" getPointObjectFromServer 88"<<std::endl;
        // APPEND START fp)m.tanaka 2014.03.11
        transferType = ( ( param.m_transfer_type == VisualizationParameter::Abstract ) ? 'A' : 'D' );
        // APPEND END   fp)m.tanaka 2014.03.11

        message.m_subpixel_level = ( param.m_transfer_type == VisualizationParameter::Abstract ) ? param.m_abstract_subpixel_level : param.m_detailed_subpixel_level;
        message.m_repeat_level = ( param.m_transfer_type == VisualizationParameter::Abstract ) ? param.m_abstract_repeat_level : param.m_detailed_repeat_level;
        if ( param.m_shuffle_type == VisualizationParameter::RandomBased )
        {
            message.m_shuffle_method = 'r';
        }
        else if ( param.m_shuffle_type == VisualizationParameter::SingleShuffleBased )
        {
            message.m_shuffle_method = 's';
        }
        else if ( param.m_shuffle_type == VisualizationParameter::CPUShuffleBased )
        {
            message.m_shuffle_method = 'o';
        }
        else if ( param.m_shuffle_type == VisualizationParameter::GPUShffleBased )
        {
            message.m_shuffle_method = 'c';
        }
        else if ( param.m_shuffle_type == VisualizationParameter::None )
        {
            message.m_shuffle_method = 'n';
        }
        else
        {
            assert( false );
        }
        std::cout<<" getPointObjectFromServer 119"<<std::endl;
        message.m_time_parameter = 2;
        message.m_trans_Parameter = 0;
        if ( param.m_transfer_type == VisualizationParameter::Detailed )
        {
            if ( param.m_detailed_transfer_type == VisualizationParameter::Summalized )
            {
                message.m_trans_Parameter = 2;
            }
            else if ( param.m_detailed_transfer_type == VisualizationParameter::Divided )
            {
                message.m_trans_Parameter = 1;
                message.m_level_index = param.m_repeat_level;
            }
        }
        message.m_step = stepno;// 2018.12.25  param.m_time_step;
        if ( param.m_node_type == VisualizationParameter::AllNodes )
        {
            message.m_node_type = 'a';
        }
        else if ( param.m_node_type == VisualizationParameter::SlaveNodes )
        {
            message.m_node_type = 's';
        }
        else
        {
            assert( false );
        }
        message.m_particle_limit = param.m_particle_limit;
        message.m_particle_density = param.m_particle_density;
//#ifdef IS_MODE
        message.particle_data_size_limit = param.particle_data_size_limit;
//#endif

#ifdef USE_DUMMY_CAMERA_FOR_VR_AT_COMMUNICATION
 //       m_dummy_camera.setWindowSize(param.m_camera->windowWidth(), param.m_camera->windowHeight());
        //m_dummy_camera.setWindowSize(1328,1584);
        message.m_camera = m_dummy_camera;
#else // USE_DUMMY_CAMERA_FOR_VR
        message.m_camera = param.m_camera;
#endif // USE_DUMMY_CAMERA_FOR_VR
        //      message.m_transfer_function = const_cast<TransferFunction*>(&param.m_transfer_function);
        message.m_message_size = message.byteSize();
        message.m_sampling_step = 1.0f;
        std::cout<<" getPointObjectFromServer 156"<<std::endl;
#ifdef CS_MODE
        message.m_x_synthesis = param.m_x_synthesis;
        message.m_y_synthesis = param.m_y_synthesis;
        message.m_z_synthesis = param.m_z_synthesis;
        message.m_enable_crop_region = PBVR_NOCROP;
#endif


        std::cout<<" getPointObjectFromServer 224"<<std::endl;
        //
        paramExTransFunc.applyToClientMessage( &message );

        //
        message.m_message_size = message.byteSize();
        std::cout<<" getPointObjectFromServer 230"<<std::endl;
        PBVR_TIMER_STA( 120 );
        PBVR_TIMER_STA( 121 );
        client.sendMessage( message );
        PBVR_TIMER_END( 121 );
    }

    float minX = FLT_MAX;
    float maxX = -FLT_MAX;
    float minY = FLT_MAX;
    float maxY = -FLT_MAX;
    float minZ = FLT_MAX;
    float maxZ = -FLT_MAX;
    size_t allParticle = 0;
    //  sleep (1);
#ifdef IS_MODE
    kvs::PointObject* object;
#else
    kvs::PointObject* object = new kvs::PointObject();
#endif

    // Loop for sub volumes.
    *result = ReceivedMessage();
    std::cout<<" getPointObjectFromServer 253"<<std::endl;
    int serve_numvol = numvol;
    if ( param.m_client_server_mode == 1 )
    {std::cout<<" getPointObjectFromServer 256"<<std::endl;
        setStatus( Recieving );
        client.recvMessage( &reply );
        serve_numvol = reply.m_number_volume_divide;
    }
#ifdef CS_MODE
    for ( int n = 0; n < serve_numvol; n++ )
    {
#endif
        if ( param.m_client_server_mode == 1 )
        {   std::cout<<" getPointObjectFromServer 266"<<std::endl;
            setStatus( Recieving );
            //          kvs::visclient::Profiler::get()->start( "receive time" );

#ifdef IS_MODE
            TIMER_STA( 130 );
            client.recvMessage( reply );
            std::cout<<" getPointObjectFromServer 273"<<std::endl;
            TIMER_END(130);
            if( reply.flag_send_bins )
            {std::cout<<" getPointObjectFromServer 275"<<std::endl;
                if( m_point_object != NULL ) delete m_point_object;
                m_point_object = new kvs::PointObject();
                object = m_point_object;
            }
            else
            {std::cout<<" getPointObjectFromServer 281"<<std::endl;
                if( m_point_object == NULL ) m_point_object = new kvs::PointObject();
                object = m_point_object;
            }
#else
            PBVR_TIMER_STA( 130 );
            // MODIFIED START FEAST 2016.01.07
            if ( client.recvMessage( &reply ) == 1 )
            {
                message.m_initialize_parameter = -1;
                message.m_message_size = message.byteSize();
                client.sendMessage( message );
                client.recvMessage( &reply );
                client.termClient();
                setStatus( Idle );
                return NULL;
            }
            PBVR_TIMER_END( 130 );
#endif
            // MODIFIED END FEAST 2016.01.07
            std::cout<<" getPointObjectFromServer 301"<<std::endl;
            std::cout << reply.m_number_particle << " received." <<  std::endl;
            std::cout<<" getPointObjectFromServer 303"<<std::endl;
            result->m_var_range.merge( reply.m_variable_range );

            /* 140319 for client stop by server Ctrl+c */
            if ( reply.m_number_particle == 0 )
            {
                //  client.termClient();
                //  exit(1);
            }
            /* 140319 for client stop by server Ctrl+c */

            int nmemb = reply.m_number_particle * 3;

#ifdef IS_MODE
            if( reply.flag_send_bins )
            {
#endif
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
#ifdef CS_MODE
                    delete[] reply.m_colors;
                    delete[] reply.m_normals;
                    delete[] reply.m_positions;
#endif
                }
#ifdef IS_MODE
                result->isUnderAnimation = true;
            }
#endif
        }
#ifdef CS_MODE
        else
        {
            std::cout<<" getPointObjectFromServer 347"<<std::endl;
            setStatus( Reading );

            std::stringstream suffix;
            suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << std::min( param.m_time_step, param.m_max_server_time_step )
                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( n + 1 )
                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << numvol;
            std::string filename = param.m_particle_directory + suffix.str() + ".kvsml";
            std::cout << filename << std::endl;

            const kvs::File file( filename );
            kvs::PointObject* impobj = new kvs::PointImporter( filename );
            size_t nmemb = impobj->coords().size();
            std::cout<<" getPointObjectFromServer 360"<<std::endl;
            if ( nmemb != 0 )
            {

                kvs::PointObject obj;
                obj.setCoords( impobj->coords() );
                obj.setNormals( impobj->normals() );
                obj.setColors( impobj->colors() );

                ( *object ) += obj;
                obj.clear();

                allParticle = allParticle + nmemb / 3;
            }
            delete impobj;
        }
#endif
#ifdef CS_MODE
    }
#endif
    std::cout << allParticle << " allParticle" <<  std::endl;
    std::cout<<" getPointObjectFromServer 381"<<std::endl;
    kvs::PointObject* pointObject = object;
    PBVR_TIMER_END( 120 );

    if ( param.m_client_server_mode == 1 )
    {std::cout<<" getPointObjectFromServer 386"<<std::endl;
#ifdef IS_MODE
        //      setStatus( Recieving );
        //      client.recvMessage( reply );
        result->c_bins.resize( reply.tf_count );
        result->o_bins.resize( reply.tf_count );
        for ( int tf = 0; tf < reply.tf_count; tf++ )
        {
            if ( reply.c_nbins[tf] > 0 )
            {
                result->c_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, reply.c_nbins[tf], reply.c_bins[tf] );
            }
            if ( reply.o_nbins[tf] )
            {
                result->o_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, reply.o_nbins[tf], reply.o_bins[tf] );
            }
        }
        std::cout<<" getPointObjectFromServer 403"<<std::endl;
        result->subPixelLevel = reply.subPixelLevel;
        lavel_time_step       = reply.timeStep;

        result->maxObjectCoord[0] = reply.maxObjectCoord[0];
        result->maxObjectCoord[1] = reply.maxObjectCoord[1];
        result->maxObjectCoord[2] = reply.maxObjectCoord[2];

        result->minObjectCoord[0] = reply.minObjectCoord[0];
        result->minObjectCoord[1] = reply.minObjectCoord[1];
        result->minObjectCoord[2] = reply.minObjectCoord[2];
#else
        setStatus( Recieving );
        // MODIFIED START FEAST 2016.01.07
        if ( client.recvMessage( &reply ) == 1 )
        {
            message.m_initialize_parameter = -1;
            message.m_message_size = message.byteSize();
            client.sendMessage( message );

            client.recvMessage( &reply );
            client.termClient();
            setStatus( Idle );
            return NULL;
        }
        result->m_color_bins.resize( reply.m_transfer_function_count );
        result->m_opacity_bins.resize( reply.m_transfer_function_count );
        for ( int tf = 0; tf < reply.m_transfer_function_count; tf++ )
        {
            if ( reply.m_color_nbins[tf] > 0 )
            {
                result->m_color_bins[tf] = kvs::visclient::FrequencyTable(
                            0.0, 1.0, reply.m_color_nbins[tf], reply.m_color_bins[tf],
                            reply.m_color_bin_names[tf]);
            }
            if ( reply.m_opacity_bins_number[tf] )
            {
                result->m_opacity_bins[tf] = kvs::visclient::FrequencyTable(
                            0.0, 1.0, reply.m_opacity_bins_number[tf], reply.m_opacity_bins[tf],
                            reply.m_opacity_bin_names[tf]);
            }
        }
        result->m_subpixel_level = reply.m_subpixel_level;
#endif

        std::cout<<" getPointObjectFromServer 448"<<std::endl;
        for ( int tf = 0; tf < reply.m_transfer_function_count; tf++ )
        {
            delete[] reply.m_color_bins[tf];
            delete[] reply.m_opacity_bins[tf];
        }
        delete[] reply.m_color_nbins;
        delete[] reply.m_opacity_bins_number;

        if (reply.m_number_particle != 0) {
            delete[] reply.m_colors;
            delete[] reply.m_normals;
            delete[] reply.m_positions;
        }

    }

    //サイズの代わりにサブピクセルレベルを代入.
    object->setSize( static_cast<kvs::Real32>( result->m_subpixel_level ) );
    std::cout<<" getPointObjectFromServer 460"<<std::endl;
    setStatus( Exit );
    // particle output
    std::string outdir, prefix;
    std::string pout = "PARTICLE_OUTDIR";
    std::string prfx = "PARTICLE_CLIENT_PREFIX";
#ifdef KVS_COMPILER_VC
    if ( getenv( pout.c_str() ) != NULL )
    {
#else
    if ( std::string( getenv( pout.c_str() ) ) != "" )
    {
#endif
        kvs::File outdir_pfx = std::string( getenv( pout.c_str() ) );
        outdir = outdir_pfx.pathName();
        kvs::Directory kvsoutdir( outdir );
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.06.06
        //if ( kvsoutdir.isExisted() )
        if ( kvsoutdir.exists() )
        {
            std::cout<<" getPointObjectFromServer 478"<<std::endl;
            if ( outdir_pfx.fileName() != "" )
            {
                prefix = outdir_pfx.Separator() + std::string( outdir_pfx.fileName().c_str() ) + "_";

                if ( getenv( prfx.c_str() ) != NULL ) prefix = std::string( getenv( prfx.c_str() ) );
                std::stringstream suffix;
                suffix << std::setw( 5 ) << std::setfill( '0' ) << ( param.m_time_step )
                       << '_' << std::setw( 7 ) << std::setfill( '0' ) << 1
                       << '_' << std::setw( 7 ) << std::setfill( '0' ) << 1;
                std::string filename = outdir + prefix + suffix.str() + ".kvsml";
                std::cout << filename << std::endl;
                const kvs::File file( filename );
                //KVS2.7.0
                //MOD BY)T.Osaki 2020.06.06
                //kvs::KVSMLObjectPoint* kvsml = new kvs::PointExporter<kvs::KVSMLObjectPoint>( object );
                //kvsml->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
                kvs::KVSMLPointObject* kvsml = new kvs::PointExporter<kvs::KVSMLPointObject>(object);
                kvsml->setWritingDataType( kvs::KVSMLPointObject::ExternalBinary );
                kvsml->write( filename );
                //KVS2.7.0
                //MOD BY)T.Osaki 2020.06.06
                //std::cout << *kvsml << std::endl << std::endl;
                kvsml->print( std::cout );
                delete kvsml;
            }
            else
            {
                std::cerr << "Error \"-pout\" : " << "Prefix does not exist" << std::endl;
            }
        }
        else
        {
            std::cout << "Error: PARTICLE_OUTDIR " << outdir << " is not existed." << std::endl;
        }
    }
    std::cout<<" getPointObjectFromServer 507"<<std::endl;
#ifdef USE_SERVER_SIDE_PBVR_COORDS
    minX = server_side_min_coords[0];
    maxX = server_side_max_coords[0];
    minY = server_side_min_coords[1];
    maxY = server_side_max_coords[1];
    minZ = server_side_min_coords[2];
    maxZ = server_side_max_coords[2];
#else // USE_SERVER_SIDE_PBVR_COORDS
    for ( size_t i = 0; i < allParticle; ++i )
    {
        minX = std::min( minX, pointObject->coords()[3 * i] );
        maxX = std::max( maxX, pointObject->coords()[3 * i] );
        minY = std::min( minY, pointObject->coords()[3 * i + 1] );
        maxY = std::max( maxY, pointObject->coords()[3 * i + 1] );
        minZ = std::min( minZ, pointObject->coords()[3 * i + 2] );
        maxZ = std::max( maxZ, pointObject->coords()[3 * i + 2] );
    }
#endif //USE_SERVER_SIDE_PBVR_COORDS

    const kvs::Vector3f min( minX, minY, minZ );
    const kvs::Vector3f max( maxX, maxY, maxZ );

    //        kvs::PointObject* pointObject = new kvs::PointObject( positions, colors, normals, 1 );

    if( pointObject )   pointObject->setMinMaxObjectCoords( min, max );
    if( pointObject )   pointObject->setMinMaxExternalCoords( min, max );
    std::cout<<" getPointObjectFromServer 525"<<std::endl;
    if ( param.m_client_server_mode == 1 )
    {
        message.m_initialize_parameter = -1;
        message.m_message_size = message.byteSize();
        client.sendMessage( message );
        client.recvMessage( &reply );
        client.termClient();
    }
    setStatus( Idle );
#ifdef IS_MODE
    delete reply.camera;
#endif
    PBVR_TIMER_END( 100 );
    std::cout<<"END getPointObjectFromServer()"<<std::endl;
    return pointObject;

}

kvs::PointObject* ParticleServer::getPointObjectFromLocal()
{
    kvs::ValueArray<float> point( 3 );
    point[0] = 0.0;
    point[1] = 0.0;
    point[2] = 0.0;
    kvs::ValueArray<kvs::UInt8> color( 3 );
    color[0] = 255;
    color[1] = 0;
    color[2] = 0;
    kvs::PointObject* pointObject = new kvs::PointObject( point, color, 30 );

    pointObject->setMinMaxExternalCoords( Vector3f( -1.0, -1.0, -1.0 ), Vector3f( 1.0, 1.0, 1.0 ) );
    return pointObject;
}

void ParticleServer::close( const VisualizationParameter& param )
{
    std::cout<<"#### !!! ParticleServer::close() !!!! #####"<<std::endl;
    setStatus( Exit );
    exit(99);
    jpv::ParticleTransferClient client( param.m_hostname, param.m_port );
    jpv::ParticleTransferServerMessage reply;
    jpv::ParticleTransferClientMessage message;

    strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
    message.m_initialize_parameter = -2;
    message.m_message_size = message.byteSize();

    client.initClient();
    client.sendMessage( message );
    client.recvMessage( &reply );
    client.termClient();

    setStatus( Idle );
}

ParticleServer::Status ParticleServer::getStatus()
{
    Status stat;
    m_status_mutex.lock();
    stat = m_status;
    m_status_mutex.unlock();
    return stat;
}

void ParticleServer::setStatus( ParticleServer::Status status )
{
    m_status_mutex.lock();
    m_status = status;
    m_status_mutex.unlock();
}

