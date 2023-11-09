#ifndef PBVR__VIZ_PARAMETER_FILE_H_INCLUDE
#define PBVR__VIZ_PARAMETER_FILE_H_INCLUDE

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fstream>
#include "Types.h"
#include <kvs/RGBColor>
#include <kvs/Xform>

class VizParameterFile
{

public:

    static int ReadParamFile( const char* fname, const int readflag = 1 );
    static int WriteParamFile( const char* fname );
    static kvs::Xform ConversionFloatToClass();
    static void ConversionClassToFloat( kvs::Xform);
    static kvs::RGBColor getBackgroundColor();

};

#endif // PBVR__VIZ_PARAMETER_FILE_H_INCLUDE
