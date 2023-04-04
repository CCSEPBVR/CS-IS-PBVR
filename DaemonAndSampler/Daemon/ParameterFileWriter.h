#ifndef __PARAMETER_FILE_WRITER_H__
#define __PARAMETER_FILE_WRITER_H__

#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <kvs/Type>
#include <kvs/Vector3>
#include <kvs/Camera>
#include <kvs/ColorMap>
#include <kvs/OpacityMap>
#include "ParticleTransferProtocol.h"
#include "PBVRParam.h"
#include "NameListFile.h"

class ParameterFileWriter
{
private:
    NameListFile m_name_list_file;

public:
    void inputMessage( const jpv::ParticleTransferClientMessage& clntMes );
    void inputParameterMessage( const jpv::ParticleTransferClientMessage& clntMes );
    void inputTransferFunctionMessage( const jpv::ParticleTransferClientMessage& clntMes );
    void writeParameterFile( const char* fname );
    void writeDifferentParameterFile( const char* fname1, const char *fname2 );
    const NameListFile& getNameListFile() const;
};

#endif
