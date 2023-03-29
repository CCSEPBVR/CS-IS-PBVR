#include "ParticleServer.h"

#include "ParticleTransferClient.h"
//#include "Communication/ParticleTransferProtocol.h"

#include <kvs/File>
#include <kvs/Directory>
#include <kvs/PointImporter>
#include <kvs/PointExporter>
#include <kvs/PointObject>

#include "PBVRParam.h"

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


kvs::PointObject* ParticleServer::getPointObjectFromServer( const PBVRParam& param, PBVRResult* result, const int numvol )
{

    const ParamExTransFunc& paramExTransFunc = param.paramExTransFunc;
//    const CropParam& paramCrop = param.paramCrop;

    TIMER_STA( 100 );
    //if (nrep == 0) paramExTransFunc.ExTransFuncParaSet();
    nrep++;
    std::cout << " ### nrep = " <<  nrep << std::endl;
    std::cout << " getPointObjectFromServer param.timeStep " <<  param.timeStep << std::endl;
//#if 0
    jpv::ParticleTransferClient client( param.hostname, param.port );
    jpv::ParticleTransferServerMessage reply;
    jpv::ParticleTransferClientMessage message;

    reply.camera = new kvs::Camera();

    setStatus( Sending );
    if ( param.CSmode == 1 )
    {
        client.initClient();
        strncpy( message.header, "JPTP /1.0\r\n", 11 );
        message.initParam = 1;
        message.renderingId = param.renderingId;
        if ( param.samplingType == PBVRParam::UniformSampling )
        {
            message.samplingMethod = 'u';
        }
        else if ( param.samplingType == PBVRParam::RejectionSampling )
        {
            message.samplingMethod = 'r';
        }
        else if ( param.samplingType == PBVRParam::MetropolisSampling )
        {
            message.samplingMethod = 'm';
        }
        else
        {
            assert( false );
        }

        // APPEND START fp)m.tanaka 2014.03.11
        transferType = ( ( param.transferType == PBVRParam::Abstract ) ? 'A' : 'D' );
        // APPEND END   fp)m.tanaka 2014.03.11

        message.subPixelLevel = ( param.transferType == PBVRParam::Abstract ) ? param.abstractSubpixelLevel : param.detailedSubpixelLevel;
        message.repeatLevel = ( param.transferType == PBVRParam::Abstract ) ? param.abstractRepeatLevel : param.detailedRepeatLevel;
        if ( param.shuffleType == PBVRParam::RandomBased )
        {
            message.shuffleMethod = 'r';
        }
        else if ( param.shuffleType == PBVRParam::SingleShuffleBased )
        {
            message.shuffleMethod = 's';
        }
        else if ( param.shuffleType == PBVRParam::CPUShuffleBased )
        {
            message.shuffleMethod = 'o';
        }
        else if ( param.shuffleType == PBVRParam::GPUShffleBased )
        {
            message.shuffleMethod = 'c';
        }
        else if ( param.shuffleType == PBVRParam::None )
        {
            message.shuffleMethod = 'n';
        }
        else
        {
            assert( false );
        }

        message.timeParam = 2;
        message.transParam = 0;
        if ( param.transferType == PBVRParam::Detailed )
        {
            if ( param.detailedTransferType == PBVRParam::Summalized )
            {
                message.transParam = 2;
            }
            else if ( param.detailedTransferType == PBVRParam::Divided )
            {
                message.transParam = 1;
                message.levelIndex = param.repeatLevel;
            }
        }
        message.step = param.timeStep;
        if ( param.nodeType == PBVRParam::AllNodes )
        {
            message.nodeType = 'a';
        }
        else if ( param.nodeType == PBVRParam::SlaveNodes )
        {
            message.nodeType = 's';
        }
        else
        {
            assert( false );
        }
        message.particle_limit = param.particle_limit;
        message.particle_density = param.particle_density;
        message.particle_data_size_limit = param.particle_data_size_limit;
        message.camera = param.camera;
//      message.transfunc = const_cast<TransferFunction*>(&param.transferFunction);
        message.messageSize = message.byteSize();
        message.samplingStep = 1.0f;


//        switch ( paramCrop.cropType )
//        {
//        case CropParam::Crop:
//            message.enable_crop_region = CROP;
//            message.crop_region[0] = paramCrop.rgcxminlevel;
//            message.crop_region[1] = paramCrop.rgcyminlevel;
//            message.crop_region[2] = paramCrop.rgczminlevel;
//            message.crop_region[3] = paramCrop.rgcxmaxlevel;
//            message.crop_region[4] = paramCrop.rgcymaxlevel;
//            message.crop_region[5] = paramCrop.rgczmaxlevel;
//            break;

//        case CropParam::Sphere:
//            message.enable_crop_region = SPHERE;
//            message.crop_region[0] = paramCrop.rgscenterx;
//            message.crop_region[1] = paramCrop.rgscentery;
//            message.crop_region[2] = paramCrop.rgscenterz;
//            message.crop_region[3] = paramCrop.rgsradius;
//            message.crop_region[4] = 0.0F;
//            message.crop_region[5] = 0.0F;
//            break;

//        case CropParam::Pillar:
//            message.enable_crop_region = PILLAR;
//            message.crop_region[0] = paramCrop.rgpcenterx;
//            message.crop_region[1] = paramCrop.rgpcentery;
//            message.crop_region[2] = paramCrop.rgpcenterz;
//            message.crop_region[3] = paramCrop.rgpradius;
//            message.crop_region[4] = paramCrop.rgpheight;
//            message.crop_region[5] = 0.0F;
//            break;

//        case CropParam::PillarYZ:
//            message.enable_crop_region = PILLARYZ;
//            message.crop_region[0] = paramCrop.rgpcenterx;
//            message.crop_region[1] = paramCrop.rgpcentery;
//            message.crop_region[2] = paramCrop.rgpcenterz;
//            message.crop_region[3] = paramCrop.rgpradius;
//            message.crop_region[4] = paramCrop.rgpheight;
//            message.crop_region[5] = 0.0F;
//            break;

//        case CropParam::PillarXZ:
//            message.enable_crop_region = PILLARXZ;
//            message.crop_region[0] = paramCrop.rgpcenterx;
//            message.crop_region[1] = paramCrop.rgpcentery;
//            message.crop_region[2] = paramCrop.rgpcenterz;
//            message.crop_region[3] = paramCrop.rgpradius;
//            message.crop_region[4] = paramCrop.rgpheight;
//            message.crop_region[5] = 0.0F;
//            break;

//        default:
//            message.enable_crop_region = 0;
//        }
        message.enable_crop_region = 0;
        //
        paramExTransFunc.applyToClientMessage( &message );

        //
        message.messageSize = message.byteSize();

//      kvs::visclient::Profiler::get()->start( "send time" );
        TIMER_STA( 120 );
        TIMER_STA( 121 );
        client.sendMessage( message );
        TIMER_END( 121 );
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
    kvs::PointObject* object;

    // Loop for sub volumes.
    *result = PBVRResult();

    int serve_numvol = numvol;
    if ( param.CSmode == 1 )
    {
        setStatus( Recieving );
        client.recvMessage( reply );
        serve_numvol = reply.numVolDiv;
    }

//  for ( int n = 0; n < serve_numvol; n++ )
//  {
        if ( param.CSmode == 1 )
        {
            setStatus( Recieving );
//          kvs::visclient::Profiler::get()->start( "receive time" );
            TIMER_STA( 130 );
            client.recvMessage( reply );
            TIMER_END( 130 );
            if( reply.flag_send_bins )
            {
                if( m_point_object != NULL ) delete m_point_object;
                m_point_object = new kvs::PointObject();
                object = m_point_object;
            }
            else
            {
                if( m_point_object == NULL ) m_point_object = new kvs::PointObject();
                object = m_point_object;
            }

//          kvs::visclient::Profiler::get()->end( "receive time" );
            std::cout << reply.numParticle << " received." <<  std::endl;

            result->varRange.merge( reply.varRange );

            /* 140319 for client stop by server Ctrl+c */
            if ( reply.numParticle == 0 )
            {
                //  client.termClient();
                //  exit(1);
            }
            /* 140319 for client stop by server Ctrl+c */

            int nmemb = reply.numParticle * 3;
//            std::cout<<"Particle Server 290" <<  std::endl;
            if( reply.flag_send_bins )
            {
                if ( nmemb != 0 )
                {
                    kvs::ValueArray<kvs::Real32> positions ( reply.positions, nmemb );
                    kvs::ValueArray<kvs::Real32> normals ( reply.normals, nmemb );
                    kvs::ValueArray<kvs::UInt8>  colors ( reply.colors, nmemb );
                    kvs::PointObject obj;
                    obj.setCoords( positions );
                    obj.setNormals( normals );
                    obj.setColors( colors );

                    ( *object ) += obj;
                    obj.clear();

                    allParticle = allParticle + reply.numParticle;
                }
                result->isUnderAnimation = true;
            }
//            std::cout<<"Particle Server 310" <<  std::endl;
            if (nmemb !=0){
            if( reply.colors    != NULL ){ delete[] reply.colors;     reply.colors    = NULL; }
            if( reply.normals   != NULL ){ delete[] reply.normals;    reply.normals   = NULL; }
            if( reply.positions != NULL ){ delete[] reply.positions;  reply.positions = NULL; }
            }
            result->minServerTimeStep = reply.staStep;
            result->maxServerTimeStep = reply.endStep;
            result->serverTimeStep = reply.timeStep;
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
    TIMER_END( 120 );

    if ( param.CSmode == 1 )
    {
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

        result->subPixelLevel = reply.subPixelLevel;
        TimecontrolPanel::lavel_time_step       = reply.timeStep;

        result->maxObjectCoord[0] = reply.maxObjectCoord[0];
        result->maxObjectCoord[1] = reply.maxObjectCoord[1];
        result->maxObjectCoord[2] = reply.maxObjectCoord[2];

        result->minObjectCoord[0] = reply.minObjectCoord[0];
        result->minObjectCoord[1] = reply.minObjectCoord[1];
        result->minObjectCoord[2] = reply.minObjectCoord[2];

        for ( int tf = 0; tf < reply.tf_count; tf++ )
        {
            delete[] reply.c_bins[tf];
            delete[] reply.o_bins[tf];
        }
        delete[] reply.c_nbins;
        delete[] reply.o_nbins;
    }
    result->numParticle=reply.numParticle;
    result->numVolDiv=reply.numVolDiv;
    result->staStep=reply.staStep;
    result->endStep=reply.endStep;
    result->numStep=reply.endStep-reply.staStep;
    result->minValue=reply.minValue;
    result->maxValue=reply.maxValue;
    result->numNodes=reply.numNodes;
    result->numElements=reply.numElements;
    result->elemType=reply.elemType;
    result->fileType=reply.fileType;
    result->numIngredients=reply.numIngredients;
    result->flag_send_bins=reply.flag_send_bins;
    result->tf_count=reply.tf_count;


    //サイズの代わりにサブピクセルレベルを代入.
    object->setSize( static_cast<kvs::Real32>( result->subPixelLevel ) );

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
                suffix << std::setw( 5 ) << std::setfill( '0' ) << ( param.timeStep )
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

    if( pointObject ) pointObject->setMinMaxObjectCoords( min, max );
    if( pointObject ) pointObject->setMinMaxExternalCoords( min, max );

    if ( param.CSmode == 1 )
    {
        message.initParam = -1;
        message.messageSize = message.byteSize();
        client.sendMessage( message );
        client.recvMessage( reply );
        client.termClient();
    }
    setStatus( Idle );
    delete reply.camera;
    TIMER_END( 100 );
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

void ParticleServer::close( const PBVRParam& param )
{
    setStatus( Exit );

    jpv::ParticleTransferClient client( param.hostname, param.port );
    jpv::ParticleTransferServerMessage reply;
    jpv::ParticleTransferClientMessage message;

    strncpy( message.header, "JPTP /1.0\r\n", 11 );
    message.initParam = -2;
    message.messageSize = message.byteSize();

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

