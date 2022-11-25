#ifndef PBVR__KVS__VISCLIENT__PARTICLE_SERVER_H_INCLUDE
#define PBVR__KVS__VISCLIENT__PARTICLE_SERVER_H_INCLUDE

#include <string>
#include <iostream>
#include <vector>

#include "VisualizationParameter.h"
#include "ReceivedMessage.h"
#include <kvs/PointObject>
#include "ParticleTransferClient.h"
#include <kvs/Mutex>

namespace kvs
{
namespace visclient
{

class ParticleServer
{
public:
    enum Status
    {
        Idle      = 0,
        Sending   = 1,
        Recieving = 2,
        Reading   = 3,
        Exit      = 4,
    };

private:
    int m_number_repeat;
    Status m_status;
    kvs::Mutex m_status_mutex;
//    kvs::PointObject* m_point_object;

public:

    ParticleServer( void ) :
        m_status( Idle )
//        m_point_object( NULL )
    {
        m_number_repeat = 0;
    }

	// 2018.12.25 
    kvs::PointObject* getPointObjectFromServer( const VisualizationParameter& param, ReceivedMessage* result, const int numvol, const int stepno, float server_side_min_coords[3], float server_side_max_coords[3] );

    kvs::PointObject* getPointObjectFromLocal();
    void close( const VisualizationParameter& param );

    Status getStatus();

private:
    void setStatus( const Status status );
};

}
}

#endif    // PBVR__KVS__VISCLIENT__PARTICLE_SERVER_H_INCLUDE

