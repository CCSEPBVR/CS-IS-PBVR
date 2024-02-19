#ifndef __KVS_VISCLIENT_PBVR_PARAM_H__
#define __KVS_VISCLIENT_PBVR_PARAM_H__

#include <cstring>
#include "kvs/TransferFunction"
#include "kvs/Camera"
#include "ExtendedTransferFunctionMessage.h"
#include "CropParam.h"
#include "ParticleMergeParam.h"


namespace kvs
{
namespace visclient
{

struct PBVRParam
{
    enum ParticleShuffleType
    {
        RandomBased = 1,
        SingleShuffleBased = 2,
        CPUShuffleBased = 3,
        GPUShffleBased = 4,
        None = 0,
    };

    enum DetailedParticleTransferType
    {
        Summalized = 1,
        Divided = 2,
    };

    enum TransferType
    {
        Abstract = 1,
        Detailed = 2,
    };

    enum SamplingType
    {
        UniformSampling = 1,
        RejectionSampling = 2,
        MetropolisSampling = 3
    };

    enum NodeType
    {
        AllNodes = 1,
        SlaveNodes = 2,
    };

    std::string inputdir;
    std::string hostname;
    std::string particledir;
    int port;
    int timeStep;
    int abstractSubpixelLevel;
    int detailedSubpixelLevel;
    int abstractRepeatLevel;
    int detailedRepeatLevel;
// APPEND START FP)K.YAJIMA 2015.02.27
    int ResolutionW;
    int ResolutionH;
// APPEND END FP)K.YAJIMA 2015.02.27
    int minTimeStep;
    int maxTimeStep;
    int minServerTimeStep;
    int maxServerTimeStep;
    int repeatLevel;
    int CSmode;
    int renderingId;
    int32_t particle_limit; // APPEND Fj) 2015.03.04
    float particle_density;
    float particle_data_size_limit;
    int timeStepKeyFrame;

    kvs::TransferFunction transferFunction;
    kvs::Camera* camera;
    TransferType transferType;
    ParticleShuffleType shuffleType;
    DetailedParticleTransferType detailedTransferType;
    SamplingType samplingType;
    NodeType nodeType;

    bool shadingType_f;

    ExtendedTransferFunctionMessage paramExTransFunc;
    CropParam paramCrop;
    int norepsampling;

    PBVRParam() :
        timeStep( 0 ),
        abstractSubpixelLevel( 2 ),
        detailedSubpixelLevel( 2 ),
        abstractRepeatLevel( 4 ),
        detailedRepeatLevel( 2 ), /* change default 16 to 4. 2014.07.21 */
// APPEND START FP)K.YAJIMA 2015.02.27
        ResolutionW( 2 ),
        ResolutionH( 2 ),
        particle_limit( 999999999 ),
// APPEND END FP)K.YAJIMA 2015.02.27
        minTimeStep( 0 ),
        maxTimeStep( 1 ),
        maxServerTimeStep( 0 ),
        CSmode( 1 ),
        renderingId( 0 ),
        shadingType_f( false ),
        norepsampling( 0 )
    {
        repeatLevel = detailedRepeatLevel;
        shuffleType = RandomBased;
        detailedTransferType = Summalized;
//		transferType = Abstract;
        transferType = Detailed; /* change default Abstract to Detailed. 2014.07.21 */
        samplingType = UniformSampling;
        nodeType = AllNodes;
    }

    ParticleMergeParam particle_merge_param;
};

}
}

#endif
