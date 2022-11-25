#pragma once

#include <string>
#include "Types.h"
#include <kvs/Vector3>
#include "ExtendedTransferFunctionParameter.h"
#include "VariableRange.h"

#include <iostream>
#include <sstream>
#include <vector>

namespace kvs
{
class Camera;
class TransferFunction;
}

namespace jpv
{

class ParticleTransferUtils
{
public:
    template<typename T>
    static void appendMessage( std::ostringstream& ss, T& content )
    {
        ss.write( reinterpret_cast<const char*>( &content ), sizeof( content ) );
    }
    static const int32_t magicNumber;
    static bool isLittleEndian( void );
};

class ParticleTransferClientMessage
{
public:
    struct VolumeEquation
    {
        std::string Name;
        std::string Equation;
    };

public:
    char header[11];
    int32_t messageSize;

    int32_t initParam;
    char samplingMethod;
    int32_t subPixelLevel;
    int32_t repeatLevel;
    char shuffleMethod;
    char nodeType;
    float samplingStep;
    int32_t renderingId;
    kvs::Camera* camera;

    int32_t timeParam;
    int32_t beginTime;
    int32_t endTime;
    int32_t memorySize;
    int32_t step;

    int32_t transParam;
    int32_t levelIndex;

    int32_t enable_crop_region;
    float crop_region[6];
    int32_t particle_limit;
    float particle_density;
    float particle_data_size_limit;
    std::string inputDir;

    std::vector<NamedTransferFunctionParameter> transfunc;
    std::vector<VolumeEquation> voleqn;

    std::string transferFunctionSynthesis;

    // message �̃T�C�Y���v�Z
    int32_t byteSize( void ) const;
    // ���b�Z�[�W�� byte ���� pack
    size_t pack( char* buf ) const;
    // byte �񂩂烁�b�Z�[�W�� unpack
    size_t unpack( const char* buf );

    ParticleTransferClientMessage( void );
};

class ParticleTransferServerMessage
{
public:
    struct VolumeEquation
    {
        std::string Name;
        std::string Equation;
    };
public:
    char header[18];
    int32_t messageSize;
    int32_t timeStep;
    int32_t subPixelLevel;
    int32_t repeatLevel;
    int32_t levelIndex;
    int32_t numParticle;
    int32_t numVolDiv;
    int32_t staStep;
    int32_t endStep;
    int32_t numStep;
    float* positions;
    float* normals;
    unsigned char* colors;
    float minObjectCoord[3];
    float maxObjectCoord[3];
    float minValue;
    float maxValue;
    int32_t numNodes;
    int32_t numElements;
    int32_t elemType;
    int32_t fileType;
    int32_t numIngredients;
    int32_t flag_send_bins;
    int32_t tf_count;

    int32_t particle_limit;
    float particle_density;
    float particle_data_size_limit;
    kvs::Camera* camera;
    std::vector<NamedTransferFunctionParameter> transfunc;
    std::vector<VolumeEquation> voleqn;
    std::string transferFunctionSynthesis;

    kvs::UInt64* c_nbins;
    kvs::UInt64* o_nbins;
    std::vector<kvs::UInt64*> c_bins;
    std::vector<kvs::UInt64*> o_bins;


    // message �̃T�C�Y���v�Z
    int32_t byteSize( void ) const;
    // ���b�Z�[�W�� byte ���� pack
    size_t pack( char* buf ) const;
    // byte �񂩂烁�b�Z�[�W�� unpack
    size_t unpack_message( const char* buf );
    size_t unpack_particles( const char* buf );
    size_t unpack_bins( size_t index, const char* buf );

    float transferFunctionMinValue;
    float transferFunctionMaxValue;

    VariableRange varRange;

    ParticleTransferServerMessage( void );
};

}
