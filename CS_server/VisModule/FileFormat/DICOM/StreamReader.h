/****************************************************************************/
/**
 *  @file StreamReader.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StreamReader.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__DCM__STREAM_READER_H_INCLUDE
#define VIS_MODULE__DCM__STREAM_READER_H_INCLUDE

#include <iostream>
#include <fstream>
#include <string>
#include <typeinfo>
#include <cstring>
#include <vismodule/DebugNew>
#include <vismodule/Endian>
#include <vismodule/Message>
#include <vismodule/IgnoreUnusedVariable>


namespace vismodule
{

namespace dcm
{

namespace StreamReader
{

template <typename T>
inline T Get( std::ifstream& ifs, const bool swap = false )
{
    T value;
    ifs.read( reinterpret_cast<char*>(&value), sizeof(T) );
    if( ifs.bad() )
    {
        visModuleMessageError("Cannot get the value. [type:%s]",typeid(T).name());
        return( T(0) );
    }

    if( swap ) vismodule::Endian::Swap( value );

    return( value );
}

inline std::string Get( std::ifstream& ifs, const int size, const bool swap = false )
{
    vismodule::IgnoreUnusedVariable( swap );

    std::string ret("");

    char* value = new char [ size + 1 ];
    if( !value )
    {
        visModuleMessageError("Cannot allocate memory.");
        return( ret );
    }
    memset( value, 0, size + 1 );

    ifs.read( value, size );
    if( ifs.bad() )
    {
        visModuleMessageError("Cannot get the value. [type:%s, size:%d]","string",size);
        return( ret );
    }
    value[size] = '\0';

    ret = std::string( value );

    delete [] value;

    return( ret );
}

};

} // end of namespace dcm

} // end of namespace vismodule

#endif // VIS_MODULE__DCM__STREAM_READER_H_INCLUDE
