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

#include "ExtendedTransferFunctionMessage.h"
#include "CropParameter.h"
#include "CoordinatepanelUI.h"

#include "sighandler.h"
#include "timer_simple.h"

#ifdef KVS_COMPILER_VC
#include <direct.h>
#define mkdir( dir, mode ) _mkdir( dir )
#endif

using namespace kvs::visclient;

// APPEND START fp)m.tanaka 2014.03.11
char transferType; // 'A':abstruct 'D':detail
// APPEND END   fp)m.tanaka 2014.03.11

// APPEND START fp)m.takizawa 2014.05.20
#define PBVR_CROP            1
#define PBVR_SPHERE          2
#define PBVR_PILLAR          3
#define PBVR_PILLARYZ        4
#define PBVR_PILLARXZ        5

// APPEND END fp)m.takizawa 2014.05.20

kvs::PointObject* ParticleServer::getPointObjectFromServer( const VisualizationParameter& param, ReceivedMessage* result, const int numvol, const int stepno )
{

    const ExtendedTransferFunctionMessage& m_parameter_extend_transfer_function = param.m_parameter_extend_transfer_function;
    const CropParameter& m_parameter_crop = param.m_parameter_crop;

    PBVR_TIMER_STA( 100 );
    //if (m_number_repeat == 0) m_parameter_extend_transfer_function.ExTransFuncParaSet();
    m_number_repeat++;
    std::cout << " ### m_number_repeat = " << m_number_repeat << std::endl;
    std::cout << " getPointObjectFromServer param.m_time_step " <<  param.m_time_step << std::endl;
//#if 0
    jpv::ParticleTransferClient client( param.m_hostname, param.m_port );
    jpv::ParticleTransferServerMessage reply;
    jpv::ParticleTransferClientMessage message;

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
        message.m_camera = param.m_camera;
//      message.m_transfer_function = const_cast<TransferFunction*>(&param.m_transfer_function);
        message.m_message_size = message.byteSize();
        message.m_sampling_step = 1.0f;

        // for Coodinate SYNTHESIZER
        message.m_x_synthesis = param.m_x_synthesis;
        message.m_y_synthesis = param.m_y_synthesis;
        message.m_z_synthesis = param.m_z_synthesis;

        switch ( m_parameter_crop.m_crop_type )
        {
        case CropParameter::Crop:
            message.m_enable_crop_region = PBVR_CROP;
            message.m_crop_region[0] = m_parameter_crop.m_region_crop_x_min_level;
            message.m_crop_region[1] = m_parameter_crop.m_region_crop_y_min_level;
            message.m_crop_region[2] = m_parameter_crop.m_region_crop_z_min_level;
            message.m_crop_region[3] = m_parameter_crop.m_region_crop_x_max_level;
            message.m_crop_region[4] = m_parameter_crop.m_region_crop_y_max_level;
            message.m_crop_region[5] = m_parameter_crop.m_region_crop_z_max_level;
            break;

        case CropParameter::Sphere:
            message.m_enable_crop_region = PBVR_SPHERE;
            message.m_crop_region[0] = m_parameter_crop.m_region_sphere_center_x;
            message.m_crop_region[1] = m_parameter_crop.m_region_sphere_center_y;
            message.m_crop_region[2] = m_parameter_crop.m_region_sphere_center_z;
            message.m_crop_region[3] = m_parameter_crop.m_region_sphere_radius;
            message.m_crop_region[4] = 0.0F;
            message.m_crop_region[5] = 0.0F;
            break;

        case CropParameter::Pillar:
            message.m_enable_crop_region = PBVR_PILLAR;
            message.m_crop_region[0] = m_parameter_crop.m_region_pillar_center_x;
            message.m_crop_region[1] = m_parameter_crop.m_region_pillar_center_y;
            message.m_crop_region[2] = m_parameter_crop.m_region_pillar_center_z;
            message.m_crop_region[3] = m_parameter_crop.m_region_pillar_radius;
            message.m_crop_region[4] = m_parameter_crop.m_region_pillar_height;
            message.m_crop_region[5] = 0.0F;
            break;

        case CropParameter::PillarYZ:
            message.m_enable_crop_region = PBVR_PILLARYZ;
            message.m_crop_region[0] = m_parameter_crop.m_region_pillar_center_x;
            message.m_crop_region[1] = m_parameter_crop.m_region_pillar_center_y;
            message.m_crop_region[2] = m_parameter_crop.m_region_pillar_center_z;
            message.m_crop_region[3] = m_parameter_crop.m_region_pillar_radius;
            message.m_crop_region[4] = m_parameter_crop.m_region_pillar_height;
            message.m_crop_region[5] = 0.0F;
            break;

        case CropParameter::PillarXZ:
            message.m_enable_crop_region = PBVR_PILLARXZ;
            message.m_crop_region[0] = m_parameter_crop.m_region_pillar_center_x;
            message.m_crop_region[1] = m_parameter_crop.m_region_pillar_center_y;
            message.m_crop_region[2] = m_parameter_crop.m_region_pillar_center_z;
            message.m_crop_region[3] = m_parameter_crop.m_region_pillar_radius;
            message.m_crop_region[4] = m_parameter_crop.m_region_pillar_height;
            message.m_crop_region[5] = 0.0F;
            break;

        default:
            message.m_enable_crop_region = 0;
        }

        //
        m_parameter_extend_transfer_function.applyToClientMessage( &message );

        //
        message.m_message_size = message.byteSize();

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
    kvs::PointObject* object = new kvs::PointObject();

    // Loop for sub volumes.
    *result = ReceivedMessage();

    int serve_numvol = numvol;
    if ( param.m_client_server_mode == 1 )
    {
        setStatus( Recieving );
        client.recvMessage( &reply );
        serve_numvol = reply.m_number_volume_divide;
    }

    for ( int n = 0; n < serve_numvol; n++ )
    {
        if ( param.m_client_server_mode == 1 )
        {
            setStatus( Recieving );
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
            // MODIFIED END FEAST 2016.01.07
            PBVR_TIMER_END( 130 );
            std::cout << reply.m_number_particle << " received." <<  std::endl;

            result->m_var_range.merge( reply.m_variable_range );

            /* 140319 for client stop by server Ctrl+c */
            if ( reply.m_number_particle == 0 )
            {
                //  client.termClient();
                //  exit(1);
            }
            /* 140319 for client stop by server Ctrl+c */

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

                ( *object ) += obj;
                obj.clear();

                allParticle = allParticle + reply.m_number_particle;
                delete[] reply.m_colors;
                delete[] reply.m_normals;
                delete[] reply.m_positions;
            }


        }
        else
        {

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
    }

    std::cout << allParticle << " allParticle" <<  std::endl;

    kvs::PointObject* pointObject = object;
    PBVR_TIMER_END( 120 );

    if ( param.m_client_server_mode == 1 )
    {
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
        // MODIFIED END FEAST 2016.01.07
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
                kvs::KVSMLObjectPoint* kvsml = new kvs::PointExporter<kvs::KVSMLObjectPoint>( object );
                kvsml->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
                kvsml->write( filename );
                std::cout << *kvsml << std::endl << std::endl;
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

    for ( size_t i = 0; i < allParticle; ++i )
    {
        minX = std::min( minX, pointObject->coords()[3 * i] );
        maxX = std::max( maxX, pointObject->coords()[3 * i] );
        minY = std::min( minY, pointObject->coords()[3 * i + 1] );
        maxY = std::max( maxY, pointObject->coords()[3 * i + 1] );
        minZ = std::min( minZ, pointObject->coords()[3 * i + 2] );
        maxZ = std::max( maxZ, pointObject->coords()[3 * i + 2] );
    }

    const kvs::Vector3f min( minX, minY, minZ );
    const kvs::Vector3f max( maxX, maxY, maxZ );

//        kvs::PointObject* pointObject = new kvs::PointObject( positions, colors, normals, 1 );

    pointObject->setMinMaxObjectCoords( min, max );
    pointObject->setMinMaxExternalCoords( min, max );

    if ( param.m_client_server_mode == 1 )
    {
        message.m_initialize_parameter = -1;
        message.m_message_size = message.byteSize();
        client.sendMessage( message );
        client.recvMessage( &reply );
        client.termClient();
    }
    setStatus( Idle );
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

