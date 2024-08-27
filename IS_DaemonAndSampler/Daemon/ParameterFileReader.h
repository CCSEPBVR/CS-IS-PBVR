#ifndef __PARAMETER_FILE_READER_H__
#define __PARAMETER_FILE_READER_H__

#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <algorithm>
#include <kvs/Type>
#include <kvs/Vector3>
#include <kvs/Camera>
#include <kvs/ColorMap>
#include <kvs/OpacityMap>
#include "ParticleTransferProtocol.h"
#include "PBVRParam.h"
//#include "../Client/ExtendedTransferFunctionMessage.h"
#include "NameListFile.h"

class ParameterFileReader
{
private:
    NameListFile m_name_list_file;
    
    void set_default_parameter();

public:
    void outputMessage( jpv::ParticleTransferServerMessage* clntMes );
    void outputParameterMessage( jpv::ParticleTransferServerMessage* clntMes );
    void outputTransferFunctionMessage( jpv::ParticleTransferServerMessage* clntMes );
    void readParameterFile( const char* fname );
    const NameListFile& getNameListFile() const;
};

#endif
