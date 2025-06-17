#ifndef __PARAMETER_FILE_WRITER_H__
#define __PARAMETER_FILE_WRITER_H__

#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include <vismodule/Camera>
#include <vismodule/ColorMap>
#include <vismodule/OpacityMap>
#include "ParticleTransferProtocol.h"
#include "PBVRParam.h"
#include <vismodule/NameListFile>

class ParameterFileWriter
{
private:
    NameListFile m_name_list_file;

public:
    void inputMessage( const jpv::ParticleTransferClientMessage& clntMes );
    void inputParameterMessage( const jpv::ParticleTransferClientMessage& clntMes );
    void inputTransferFunctionMessage( const jpv::ParticleTransferClientMessage& clntMes );
    void inputGlyphParameterMessage( const jpv::ParticleTransferClientMessage& clntMes );
    void inputPlotOverLineParameterMessage( const jpv::ParticleTransferClientMessage& clntMes );
    void writeParameterFile( const char* fname );
    void writeDifferentParameterFile( const char* fname1, const char *fname2 );
    const NameListFile& getNameListFile() const;
};

#endif
