#include "ParticleServer.h"

#include "ParticleTransferClient.h"
//#include "Communication/ParticleTransferProtocol.h"

#include <kvs/File>
#include <kvs/Directory>
#include <kvs/PointImporter>
#include <kvs/PointExporter>
#include <kvs/PointObject>

#include "VisualizationParameter.h"

//#include "Profiler.h"

#include <float.h>

#include <cstring>
#include <cstdlib>
#include <cassert>

#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

#include "ParamExTransFunc.h"
//#include "CropParam.h"
#include "Panels/timecontrolpanel.h"
//#include "sighandler.h"
#include "timer_simple.h"

#ifdef KVS_COMPILER_VC
#include <direct.h>
#define mkdir( dir, mode ) _mkdir( dir )
#endif

using namespace kvs::visclient;

// APPEND START fp)m.tanaka 2014.03.11
char transferType; // 'A':abstruct 'D':detail
// APPEND END   fp)m.tanaka 2014.03.11

//extern int lavel_time_step;
// APPEND START fp)m.takizawa 2014.05.20
#define CROP            1
#define SPHERE          2
#define PILLAR          3
#define PILLARYZ        4
#define PILLARXZ        5

// APPEND END fp)m.takizawa 2014.05.20


//kvs::PointObject* ParticleServer::getPointObjectFromServer( const VisualizationParameter& param, ReceivedMessage* result, const int numvol, const int stepno )
//kvs::PointObject* ParticleServer::getPointObjectFromServer( const VisualizationParameter& param, ReceivedMessage* result, const int numvol, const int stepno, float server_side_min_coords[3], float server_side_max_coords[3] )
//{

//    const ExtendedTransferFunctionMessage& paramExTransFunc = param.m_parameter_extend_transfer_function;
////    const CropParam& paramCrop = param.paramCrop;

//    PBVR_TIMER_STA( 100 );
//    //if (nrep == 0) paramExTransFunc.ExTransFuncParaSet();
//    m_number_repeat++;
//    std::cout << " ### nrep = " <<  m_number_repeat << std::endl;
//    std::cout << " getPointObjectFromServer param.timeStep " <<  param.m_time_step << std::endl;
////#if 0
//    jpv::ParticleTransferClient client( param.m_hostname, param.m_port );
//    jpv::ParticleTransferServerMessage reply;
//    jpv::ParticleTransferClientMessage message;

//    reply.camera = new kvs::Camera();

//    setStatus( Sending );
//    if ( param.m_client_server_mode == 1 )
//    {
//        client.initClient();
//        strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
//        message.m_initialize_parameter = 1;
//        message.m_rendering_id = param.m_rendering_id;
//        if ( param.m_sampling_type == VisualizationParameter::UniformSampling )
//        {
//            message.m_sampling_method = 'u';
//        }
//        else if ( param.m_sampling_type == VisualizationParameter::RejectionSampling )
//        {
//            message.m_sampling_method = 'r';
//        }
//        else if ( param.m_sampling_type == VisualizationParameter::MetropolisSampling )
//        {
//            message.m_sampling_method = 'm';
//        }
//        else
//        {
//            assert( false );
//        }

//        // APPEND START fp)m.tanaka 2014.03.11
//        transferType = ( ( param.m_transfer_type == VisualizationParameter::Abstract ) ? 'A' : 'D' );
//        // APPEND END   fp)m.tanaka 2014.03.11

//        message.m_subpixel_level = ( param.m_transfer_type == VisualizationParameter::Abstract ) ? param.m_abstract_subpixel_level : param.m_detailed_subpixel_level;
//        message.m_repeat_level = ( param.m_transfer_type == VisualizationParameter::Abstract ) ? param.m_abstract_repeat_level : param.m_detailed_repeat_level;
//        if ( param.m_shuffle_type == VisualizationParameter::RandomBased )
//        {
//            message.m_shuffle_method = 'r';
//        }
//        else if ( param.m_shuffle_type == VisualizationParameter::SingleShuffleBased )
//        {
//            message.m_shuffle_method = 's';
//        }
//        else if ( param.m_shuffle_type == VisualizationParameter::CPUShuffleBased )
//        {
//            message.m_shuffle_method = 'o';
//        }
//        else if ( param.m_shuffle_type == VisualizationParameter::GPUShffleBased )
//        {
//            message.m_shuffle_method = 'c';
//        }
//        else if ( param.m_shuffle_type == VisualizationParameter::None )
//        {
//            message.m_shuffle_method = 'n';
//        }
//        else
//        {
//            assert( false );
//        }

//        message.m_time_parameter = 2;
//        message.m_trans_Parameter = 0;
//        if ( param.m_transfer_type == VisualizationParameter::Detailed )
//        {
//            if ( param.m_detailed_transfer_type == VisualizationParameter::Summalized )
//            {
//                message.m_trans_Parameter = 2;
//            }
//            else if ( param.m_detailed_transfer_type == VisualizationParameter::Divided )
//            {
//                message.m_trans_Parameter = 1;
//                message.m_level_index = param.m_repeat_level;
//            }
//        }
//        message.m_step = param.m_time_step;
//        if ( param.m_node_type == VisualizationParameter::AllNodes )
//        {
//            message.m_node_type = 'a';
//        }
//        else if ( param.m_node_type == VisualizationParameter::SlaveNodes )
//        {
//            message.m_node_type = 's';
//        }
//        else
//        {
//            assert( false );
//        }
//        message.m_particle_limit = param.m_particle_limit;
//        message.m_particle_density = param.m_particle_density;
//        message.particle_data_size_limit = param.particle_data_size_limit;
//        message.m_camera = param.m_camera;
////      message.transfunc = const_cast<TransferFunction*>(&param.transferFunction);
//        message.m_message_size = message.byteSize();
//        message.m_sampling_step = 1.0f;


//        message.m_enable_crop_region = 0;
//        //
//        paramExTransFunc.applyToClientMessage( &message );

//        //
//        message.m_message_size = message.byteSize();

////      kvs::visclient::Profiler::get()->start( "send time" );
//        PBVR_TIMER_STA( 120 );
//        PBVR_TIMER_STA( 121 );
//        client.sendMessage( message );
//        PBVR_TIMER_END( 121 );
////      kvs::visclient::Profiler::get()->end( "send time" );
//    }

//    float minX = FLT_MAX;
//    float maxX = -FLT_MAX;
//    float minY = FLT_MAX;
//    float maxY = -FLT_MAX;
//    float minZ = FLT_MAX;
//    float maxZ = -FLT_MAX;
//    size_t allParticle = 0;
////  sleep (1);
//    kvs::PointObject* object=new kvs::PointObject();;

//    // Loop for sub volumes.
//    *result = ReceivedMessage();

//    int serve_numvol = numvol;
//    if ( param.m_client_server_mode == 1 )
//    {
//        setStatus( Recieving );
//        client.recvMessage( reply );
//        serve_numvol = reply.m_number_volume_divide;
//    }

////  for ( int n = 0; n < serve_numvol; n++ )
////  {
//        if ( param.m_client_server_mode == 1 )
//        {
//            setStatus( Recieving );
////          kvs::visclient::Profiler::get()->start( "receive time" );
//            PBVR_TIMER_STA( 130 );
//            client.recvMessage( reply );
//            PBVR_TIMER_END( 130 );
//            if( reply.m_flag_send_bins )
//            {
////                if( m_point_object != NULL ) delete m_point_object;
////                m_point_object = new kvs::PointObject();
////                object = m_point_object;
//            }
//            else
//            {
////                if( m_point_object == NULL ) m_point_object = new kvs::PointObject();
////                object = m_point_object;
//            }

////          kvs::visclient::Profiler::get()->end( "receive time" );

//            result->m_var_range.merge( reply.m_variable_range );

//            /* 140319 for client stop by server Ctrl+c */
//            if ( reply.m_number_particle == 0 )
//            {
//                //  client.termClient();
//                //  exit(1);
//            }
//            /* 140319 for client stop by server Ctrl+c */

//            int nmemb = reply.m_number_particle * 3;
////            std::cout<<"Particle Server 290" <<  std::endl;
//            if( reply.m_flag_send_bins )
//            {
//                if ( nmemb != 0 )
//                {
//                    kvs::ValueArray<kvs::Real32> positions ( reply.m_positions, nmemb );
//                    kvs::ValueArray<kvs::Real32> normals ( reply.m_normals, nmemb );
//                    kvs::ValueArray<kvs::UInt8>  colors ( reply.m_colors, nmemb );
//                    kvs::PointObject obj;
//                    obj.setCoords( positions );
//                    obj.setNormals( normals );
//                    obj.setColors( colors );

//                    ( *object ) += obj;
//                    obj.clear();

//                    allParticle = allParticle + reply.m_number_particle;
//                }
//                result->isUnderAnimation = true;
//            }
////            std::cout<<"Particle Server 310" <<  std::endl;
//            if (nmemb !=0){
//            if( reply.m_colors    != NULL ){ delete[] reply.m_colors;     reply.m_colors    = NULL; }
//            if( reply.m_normals   != NULL ){ delete[] reply.m_normals;    reply.m_normals   = NULL; }
//            if( reply.m_positions != NULL ){ delete[] reply.m_positions;  reply.m_positions = NULL; }
//            }
//            result->minServerTimeStep = reply.m_start_step;
//            result->maxServerTimeStep = reply.m_end_step;
//            result->serverTimeStep = reply.m_time_step;
//        }	//		std::cout<<"Particle Server 318" <<  std::endl;
///*
//        else
//        {

//            setStatus( Reading );

//            std::stringstream suffix;
//            suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << std::min( param.timeStep, param.maxServerTimeStep )
//                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( n + 1 )
//                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << numvol;
//            std::string filename = param.particledir + suffix.str() + ".kvsml";
//            std::cout << filename << std::endl;

//            const kvs::File file( filename );
//            kvs::PointObject* impobj = new kvs::PointImporter( filename );
//            size_t nmemb = impobj->coords().size();

//            if ( nmemb != 0 )
//            {

//                kvs::PointObject obj;
//                obj.setCoords( impobj->coords() );
//                obj.setNormals( impobj->normals() );
//                obj.setColors( impobj->colors() );

//                ( *object ) += obj;
//                obj.clear();

//                allParticle = allParticle + nmemb / 3;
//            }
//            delete impobj;
//        }
//*/
////  }

//    std::cout << allParticle << " allParticle" <<  std::endl;

//    kvs::PointObject* pointObject = object;
//    PBVR_TIMER_END( 120 );

//    if ( param.m_client_server_mode == 1 )
//    {
////      setStatus( Recieving );
////      client.recvMessage( reply );
//        result->m_color_bins.resize( reply.m_transfer_function_count );
//        result->m_opacity_bins.resize( reply.m_transfer_function_count );
//        for ( int tf = 0; tf < reply.m_transfer_function_count; tf++ )
//        {
//            if ( reply.m_color_nbins[tf] > 0 )
//            {
//                result->m_color_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, reply.m_color_nbins[tf], reply.m_color_bins[tf] );
//            }
//            if ( reply.m_opacity_bins_number[tf] )
//            {
//                result->m_opacity_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, reply.m_opacity_bins_number[tf], reply.m_opacity_bins[tf] );
//            }
//        }

//        result->m_subpixel_level = reply.m_subpixel_level;
//        TimecontrolPanel::lavel_time_step       = reply.m_time_step;

//        result->maxObjectCoord[0] = reply.m_max_object_coord[0];
//        result->maxObjectCoord[1] = reply.m_max_object_coord[1];
//        result->maxObjectCoord[2] = reply.m_max_object_coord[2];

//        result->minObjectCoord[0] = reply.m_min_object_coord[0];
//        result->minObjectCoord[1] = reply.m_min_object_coord[1];
//        result->minObjectCoord[2] = reply.m_min_object_coord[2];

//        for ( int tf = 0; tf < reply.m_transfer_function_count; tf++ )
//        {
//            delete[] reply.m_color_bins[tf];
//            delete[] reply.m_opacity_bins[tf];
//        }
//        delete[] reply.m_color_nbins;
//        delete[] reply.m_opacity_bins_number;
//    }
//    result->numParticle=reply.m_number_particle;
//    result->numVolDiv=reply.m_number_volume_divide;
//    result->staStep=reply.m_start_step;
//    result->endStep=reply.m_end_step;
//    result->numStep=reply.m_end_step-reply.m_start_step;
//    result->minValue=reply.m_min_value;
//    result->maxValue=reply.m_max_value;
//    result->numNodes=reply.m_number_nodes;
//    result->numElements=reply.m_number_elements;
//    result->elemType=reply.m_element_type;
//    result->fileType=reply.m_file_type;
//    result->numIngredients=reply.m_number_ingredients;
//    result->flag_send_bins=reply.m_flag_send_bins;
//    result->tf_count=reply.m_transfer_function_count;


//    //サイズの代わりにサブピクセルレベルを代入.
//    object->setSize( static_cast<kvs::Real32>( result->m_subpixel_level ) );

//    setStatus( Exit );
//// particle output
//    std::string outdir, prefix;
//    std::string pout = "PARTICLE_OUTDIR";
//    std::string prfx = "PARTICLE_CLIENT_PREFIX";
//#ifdef KVS_COMPILER_VC
//    if ( getenv( pout.c_str() ) != NULL )
//    {
//#else
//    if ( std::string( getenv( pout.c_str() ) ) != "" )
//    {
//#endif
//        kvs::File outdir_pfx = std::string( getenv( pout.c_str() ) );
//        outdir = outdir_pfx.pathName();
//        kvs::Directory kvsoutdir( outdir );
//        if ( kvsoutdir.isExisted() )
//        {

//            if ( outdir_pfx.fileName() != "" )
//            {
//                prefix = outdir_pfx.Separator() + std::string( outdir_pfx.fileName().c_str() ) + "_";

//                if ( getenv( prfx.c_str() ) != NULL ) prefix = std::string( getenv( prfx.c_str() ) );
//                std::stringstream suffix;
//                suffix << std::setw( 5 ) << std::setfill( '0' ) << ( param.m_time_step )
//                       << '_' << std::setw( 7 ) << std::setfill( '0' ) << 1
//                       << '_' << std::setw( 7 ) << std::setfill( '0' ) << 1;
//                std::string filename = outdir + prefix + suffix.str() + ".kvsml";
//                std::cout << filename << std::endl;
//                const kvs::File file( filename );
//                //kvs::KVSMLObjectPoint* kvsml = new kvs::PointExporter<kvs::KVSMLObjectPoint>( object );
//                //kvsml->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
//                kvs::KVSMLPointObject* kvsml = new kvs::PointExporter<kvs::KVSMLPointObject>(object);
//                kvsml->setWritingDataType( kvs::KVSMLPointObject::ExternalBinary );
//                kvsml->write( filename );
//                //std::cout << *kvsml << std::endl << std::endl;
//                kvsml->print( std::cout );
//                delete kvsml;
//            }
//            else
//            {
//                std::cerr << "Error \"-pout\" : " << "Prefix does not exist" << std::endl;
//            }
//        }
//        else
//        {
//            std::cout << "Error: PARTICLE_OUTDIR " << outdir << " is not existed." << std::endl;
//        }
//    }
//    //2020.12.17 OSAKI
//    //粒子側のminmaxCoordsを使用するのではなく、サーバ側のminmaxCoordsを使用するように変更する。
////    for ( size_t i = 0; i < allParticle; ++i )
////    {
////        minX = std::min( minX, pointObject->coords()[3 * i] );
////        maxX = std::max( maxX, pointObject->coords()[3 * i] );
////        minY = std::min( minY, pointObject->coords()[3 * i + 1] );
////        maxY = std::max( maxY, pointObject->coords()[3 * i + 1] );
////        minZ = std::min( minZ, pointObject->coords()[3 * i + 2] );
////        maxZ = std::max( maxZ, pointObject->coords()[3 * i + 2] );
////    }
//    minX = server_side_min_coords[0];
//    maxX = server_side_max_coords[0];
//    minY = server_side_min_coords[1];
//    maxY = server_side_max_coords[1];
//    minZ = server_side_min_coords[2];
//    maxZ = server_side_max_coords[2];

//    const kvs::Vector3f min( minX, minY, minZ );
//    const kvs::Vector3f max( maxX, maxY, maxZ );

////        kvs::PointObject* pointObject = new kvs::PointObject( positions, colors, normals, 1 );

//    if( pointObject ) pointObject->setMinMaxObjectCoords( min, max );
//    if( pointObject ) pointObject->setMinMaxExternalCoords( min, max );

//    if ( param.m_client_server_mode == 1 )
//    {
//        message.m_initialize_parameter = -1;
//        message.m_message_size = message.byteSize();
//        client.sendMessage( message );
//        client.recvMessage( reply );
//        client.termClient();
//    }
//    setStatus( Idle );
//    delete reply.camera;
//    PBVR_TIMER_END( 100 );
//    return pointObject;
//}

//kvs::PointObject* ParticleServer::getPointObjectFromServer( const VisualizationParameter& param, ReceivedMessage* result, const int numvol, const int stepno, float server_side_min_coords[3], float server_side_max_coords[3] )
kvs::PointObject* ParticleServer::getPointObjectFromServer( const VisualizationParameter& param, ReceivedMessage* result, const int numvol, const int stepno, float server_side_min_coords[3], float server_side_max_coords[3] )
{

    const ExtendedTransferFunctionMessage& paramExTransFunc = param.m_parameter_extend_transfer_function;
//    const CropParam& paramCrop = param.paramCrop;

    PBVR_TIMER_STA( 100 );
    //if (nrep == 0) paramExTransFunc.ExTransFuncParaSet();
    m_number_repeat++;
    std::cout << " ### nrep = " <<  m_number_repeat << std::endl;
    std::cout << " getPointObjectFromServer param.timeStep " <<  param.m_time_step << std::endl;
//#if 0
    jpv::ParticleTransferClient client( param.m_hostname, param.m_port );
    jpv::ParticleTransferServerMessage reply;
    jpv::ParticleTransferClientMessage message;

    reply.camera = new kvs::Camera();

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
        message.m_step = param.m_time_step;
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
        message.particle_data_size_limit = param.particle_data_size_limit;
        message.m_camera = param.m_camera;
//      message.transfunc = const_cast<TransferFunction*>(&param.transferFunction);
        message.m_message_size = message.byteSize();
        message.m_sampling_step = 1.0f;


        message.m_enable_crop_region = 0;
        //
        paramExTransFunc.applyToClientMessage( &message );

        //
        message.m_message_size = message.byteSize();

//      kvs::visclient::Profiler::get()->start( "send time" );
        PBVR_TIMER_STA( 120 );
        PBVR_TIMER_STA( 121 );
        client.sendMessage( message );
        PBVR_TIMER_END( 121 );
//      kvs::visclient::Profiler::get()->end( "send time" );
    }

    float minX = FLT_MAX;
    float maxX = -FLT_MAX;
    float minY = FLT_MAX;
    float maxY = -FLT_MAX;
    float minZ = FLT_MAX;
    float maxZ = -FLT_MAX;
    size_t allParticle = 0;
//  sleep (1);
    kvs::PointObject* object=new kvs::PointObject();;

    // Loop for sub volumes.
    *result = ReceivedMessage();

    int serve_numvol = numvol;
    if ( param.m_client_server_mode == 1 )
    {
        setStatus( Recieving );
        client.recvMessage( reply );
        serve_numvol = reply.m_number_volume_divide;
    }

//  for ( int n = 0; n < serve_numvol; n++ )
//  {
        if ( param.m_client_server_mode == 1 )
        {
            setStatus( Recieving );
//          kvs::visclient::Profiler::get()->start( "receive time" );
            PBVR_TIMER_STA( 130 );
            client.recvMessage( reply );
            PBVR_TIMER_END( 130 );
            if( reply.m_flag_send_bins )
            {
//                if( m_point_object != NULL ) delete m_point_object;
//                m_point_object = new kvs::PointObject();
//                object = m_point_object;
            }
            else
            {
//                if( m_point_object == NULL ) m_point_object = new kvs::PointObject();
//                object = m_point_object;
            }

//          kvs::visclient::Profiler::get()->end( "receive time" );

            result->m_var_range.merge( reply.m_variable_range );

            /* 140319 for client stop by server Ctrl+c */
            if ( reply.m_number_particle == 0 )
            {
                //  client.termClient();
                //  exit(1);
            }
            /* 140319 for client stop by server Ctrl+c */

            int nmemb = reply.m_number_particle * 3;
//            std::cout<<"Particle Server 290" <<  std::endl;
            if( reply.m_flag_send_bins )
            {
                if ( nmemb != 0 )
                {
                    kvs::ValueArray<kvs::Real32> positions ( reply.m_positions, nmemb );
                    kvs::ValueArray<kvs::Real32> normals ( reply.m_normals, nmemb );
                    kvs::ValueArray<kvs::UInt8>  colors ( reply.m_colors, nmemb );
                    kvs::PointObject obj;
                    obj.setCoords( positions );
                    obj.setNormals( normals );
                    obj.setColors( colors );

                    ( *object ) += obj;
                    obj.clear();

                    allParticle = allParticle + reply.m_number_particle;
                }
                result->isUnderAnimation = true;
            }
//            std::cout<<"Particle Server 310" <<  std::endl;
            if (nmemb !=0){
            if( reply.m_colors    != NULL ){ delete[] reply.m_colors;     reply.m_colors    = NULL; }
            if( reply.m_normals   != NULL ){ delete[] reply.m_normals;    reply.m_normals   = NULL; }
            if( reply.m_positions != NULL ){ delete[] reply.m_positions;  reply.m_positions = NULL; }
            }
            result->minServerTimeStep = reply.m_start_step;
            result->maxServerTimeStep = reply.m_end_step;
            result->serverTimeStep = reply.m_time_step;
        }	//		std::cout<<"Particle Server 318" <<  std::endl;
/*
        else
        {

            setStatus( Reading );

            std::stringstream suffix;
            suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << std::min( param.timeStep, param.maxServerTimeStep )
                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( n + 1 )
                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << numvol;
            std::string filename = param.particledir + suffix.str() + ".kvsml";
            std::cout << filename << std::endl;

            const kvs::File file( filename );
            kvs::PointObject* impobj = new kvs::PointImporter( filename );
            size_t nmemb = impobj->coords().size();

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
*/
//  }

    std::cout << allParticle << " allParticle" <<  std::endl;

    kvs::PointObject* pointObject = object;
    PBVR_TIMER_END( 120 );

    if ( param.m_client_server_mode == 1 )
    {
//      setStatus( Recieving );
//      client.recvMessage( reply );
        result->m_color_bins.resize( reply.m_transfer_function_count );
        result->m_opacity_bins.resize( reply.m_transfer_function_count );
        for ( int tf = 0; tf < reply.m_transfer_function_count; tf++ )
        {
            if ( reply.m_color_nbins[tf] > 0 )
            {
                result->m_color_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, reply.m_color_nbins[tf],(size_t *) reply.m_color_bins[tf] );
            }
            if ( reply.m_opacity_bins_number[tf] )
            {
                result->m_opacity_bins[tf] = kvs::visclient::FrequencyTable( 0.0, 1.0, reply.m_opacity_bins_number[tf],(size_t *) reply.m_opacity_bins[tf] );
            }
        }

        result->m_subpixel_level = reply.m_subpixel_level;
        TimecontrolPanel::lavel_time_step       = reply.m_time_step;

        result->maxObjectCoord[0] = reply.m_max_object_coord[0];
        result->maxObjectCoord[1] = reply.m_max_object_coord[1];
        result->maxObjectCoord[2] = reply.m_max_object_coord[2];

        result->minObjectCoord[0] = reply.m_min_object_coord[0];
        result->minObjectCoord[1] = reply.m_min_object_coord[1];
        result->minObjectCoord[2] = reply.m_min_object_coord[2];

        for ( int tf = 0; tf < reply.m_transfer_function_count; tf++ )
        {
            delete[] reply.m_color_bins[tf];
            delete[] reply.m_opacity_bins[tf];
        }
        delete[] reply.m_color_nbins;
        delete[] reply.m_opacity_bins_number;
    }
    result->numParticle=reply.m_number_particle;
    result->numVolDiv=reply.m_number_volume_divide;
    result->staStep=reply.m_start_step;
    result->endStep=reply.m_end_step;
    result->numStep=reply.m_end_step-reply.m_start_step;
    result->minValue=reply.m_min_value;
    result->maxValue=reply.m_max_value;
    result->numNodes=reply.m_number_nodes;
    result->numElements=reply.m_number_elements;
    result->elemType=reply.m_element_type;
    result->fileType=reply.m_file_type;
    result->numIngredients=reply.m_number_ingredients;
    result->flag_send_bins=reply.m_flag_send_bins;
    result->tf_count=reply.m_transfer_function_count;


    //サイズの代わりにサブピクセルレベルを代入.
    object->setSize( static_cast<kvs::Real32>( result->m_subpixel_level ) );

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
        if ( kvsoutdir.isExisted() )
        {

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
                //kvs::KVSMLObjectPoint* kvsml = new kvs::PointExporter<kvs::KVSMLObjectPoint>( object );
                //kvsml->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
                kvs::KVSMLPointObject* kvsml = new kvs::PointExporter<kvs::KVSMLPointObject>(object);
                kvsml->setWritingDataType( kvs::KVSMLPointObject::ExternalBinary );
                kvsml->write( filename );
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

    //2020.12.17 OSAKI
    //粒子側のminmaxCoordsを使用するのではなく、サーバ側のminmaxCoordsを使用するように変更する。
//    for ( size_t i = 0; i < allParticle; ++i )
//    {
//        minX = std::min( minX, pointObject->coords()[3 * i] );
//        maxX = std::max( maxX, pointObject->coords()[3 * i] );
//        minY = std::min( minY, pointObject->coords()[3 * i + 1] );
//        maxY = std::max( maxY, pointObject->coords()[3 * i + 1] );
//        minZ = std::min( minZ, pointObject->coords()[3 * i + 2] );
//        maxZ = std::max( maxZ, pointObject->coords()[3 * i + 2] );
//    }
    minX = server_side_min_coords[0];
    maxX = server_side_max_coords[0];
    minY = server_side_min_coords[1];
    maxY = server_side_max_coords[1];
    minZ = server_side_min_coords[2];
    maxZ = server_side_max_coords[2];

    const kvs::Vector3f min( minX, minY, minZ );
    const kvs::Vector3f max( maxX, maxY, maxZ );

//        kvs::PointObject* pointObject = new kvs::PointObject( positions, colors, normals, 1 );

    if( pointObject ) pointObject->setMinMaxObjectCoords( min, max );
    if( pointObject ) pointObject->setMinMaxExternalCoords( min, max );

    if ( param.m_client_server_mode == 1 )
    {
        message.m_initialize_parameter = -1;
        message.m_message_size = message.byteSize();
        client.sendMessage( message );
        client.recvMessage( reply );
        client.termClient();
    }
    setStatus( Idle );
    delete reply.camera;
    PBVR_TIMER_END( 100 );
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
    setStatus( Exit );

    jpv::ParticleTransferClient client( param.m_hostname, param.m_port );
    jpv::ParticleTransferServerMessage reply;
    jpv::ParticleTransferClientMessage message;

    strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
    message.m_initialize_parameter = -2;
    message.m_message_size = message.byteSize();

    client.initClient();
    client.sendMessage( message );
//    client.recvMessage( reply );
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

