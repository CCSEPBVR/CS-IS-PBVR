/*****************************************************************************/
/**
 *  @file   DataArray.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataArray.h 667 2011-02-22 16:07:54Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__DATA_ARRAY_H_INCLUDE
#define VIS_MODULE__KVSML__DATA_ARRAY_H_INCLUDE

#include <vismodule/File>
#include <vismodule/Tokenizer>
#include <vismodule/ValueArray>
#include <vismodule/AnyValueArray>
#include <vismodule/IgnoreUnusedVariable>
#include <iostream>
#include <fstream>
#include <sstream>

#include "endian2.h"

namespace vismodule
{

namespace kvsml
{

namespace DataArray
{

/*===========================================================================*/
/**
 *  @brief  Returns the data file name
 *  @param  filename [in] filename
 *  @param  type [in] data type (ex. 'value' or 'coord')
 *  @return data file name
 */
/*===========================================================================*/
inline const std::string GetDataFilename( const std::string& filename, const std::string& type )
{
    const std::string basename( vismodule::File( filename ).baseName() );
//    const std::string pathname( vismodule::File( filename ).pathName() );
//    const std::string sep( vismodule::File::Separator() );
    const std::string ext( "dat" );

//    return pathname + sep + basename + "_" + type + "." + ext;
    return basename + "_" + type + "." + ext;
}

/*===========================================================================*/
/**
 *  @brief  Returns the data type of the given value array.
 *  @param  data_array [in] data array
 *  @return data type as string
 */
/*===========================================================================*/
template <typename T>
inline const std::string GetDataType( const vismodule::ValueArray<T>& data_array )
{
    vismodule::IgnoreUnusedVariable( data_array );

    if (      typeid( T ) == typeid( vismodule::Int8 )   ) return "char";
    else if ( typeid( T ) == typeid( vismodule::UInt8 )  ) return "uchar";
    else if ( typeid( T ) == typeid( vismodule::Int16 )  ) return "short";
    else if ( typeid( T ) == typeid( vismodule::UInt16 ) ) return "ushort";
    else if ( typeid( T ) == typeid( vismodule::Int32 )  ) return "int";
    else if ( typeid( T ) == typeid( vismodule::UInt32 ) ) return "uint";
    else if ( typeid( T ) == typeid( vismodule::Int64 )  ) return "long";
    else if ( typeid( T ) == typeid( vismodule::UInt64 ) ) return "ulong";
    else if ( typeid( T ) == typeid( vismodule::Real32 ) ) return "float";
    else if ( typeid( T ) == typeid( vismodule::Real64 ) ) return "double";
    else return "unknown";
}

/*===========================================================================*/
/**
 *  @brief  Returns the data type of the given any-value array.
 *  @param  data_array [in] data array
 *  @return data type as string
 */
/*===========================================================================*/
inline const std::string GetDataType( const vismodule::AnyValueArray& data_array )
{
    const std::type_info& type = data_array.typeInfo()->type();
    if (      type == typeid( vismodule::Int8 )   ) return "char";
    else if ( type == typeid( vismodule::UInt8 )  ) return "uchar";
    else if ( type == typeid( vismodule::Int16 )  ) return "short";
    else if ( type == typeid( vismodule::UInt16 ) ) return "ushort";
    else if ( type == typeid( vismodule::Int32 )  ) return "int";
    else if ( type == typeid( vismodule::UInt32 ) ) return "uint";
    else if ( type == typeid( vismodule::Int64 )  ) return "long";
    else if ( type == typeid( vismodule::UInt64 ) ) return "ulong";
    else if ( type == typeid( vismodule::Real32 ) ) return "float";
    else if ( type == typeid( vismodule::Real64 ) ) return "double";
    else return "unknown";
}

/*===========================================================================*/
/**
 *  @brief  Reads the internal data as any-value array.
 *  @param  data_array [out] pointer to the any-value array
 *  @param  nelements  [in] number of elements
 *  @param  tokenizer  [in] tokenizer
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
template <typename T>
inline const bool ReadInternalData(
    vismodule::AnyValueArray* data_array,
    const size_t nelements,
    vismodule::Tokenizer& tokenizer )
{
    T* data = static_cast<T*>( data_array->template allocate<T>( nelements ) );
    if ( !data )
    {
        visModuleMessageError( "Cannot allocate memory for the internal data." );
        return false;
    }

    const size_t nloops = data_array->size();
    for ( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<T>( atof( tokenizer.token().c_str() ) );
    }

    return true;
}

template <>
inline const bool ReadInternalData<vismodule::Int8>(
    vismodule::AnyValueArray* data_array,
    const size_t nelements,
    vismodule::Tokenizer& tokenizer )
{
    vismodule::Int8* data = static_cast<vismodule::Int8*>( data_array->allocate<vismodule::Int8>( nelements ) );
    if ( !data )
    {
        visModuleMessageError( "Cannot allocate memory for the internal data." );
        return false;
    }

    const size_t nloops = data_array->size();
    for ( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<vismodule::Int8>( atoi( tokenizer.token().c_str() ) );
    }

    return true;
}

template <>
inline const bool ReadInternalData<vismodule::UInt8>(
    vismodule::AnyValueArray* data_array,
    const size_t nelements,
    vismodule::Tokenizer& tokenizer )
{
    vismodule::UInt8* data = static_cast<vismodule::UInt8*>( data_array->allocate<vismodule::UInt8>( nelements ) );
    if ( !data )
    {
        visModuleMessageError( "Cannot allocate memory for the internal data." );
        return false;
    }

    const size_t nloops = data_array->size();
    for ( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<vismodule::UInt8>( atoi( tokenizer.token().c_str() ) );
    }

    return true;
}

/*===========================================================================*/
/**
 *  @brief  Reads the internal data as value array.
 *  @param  data_array [out] pointer to the value array
 *  @param  nelements  [in] number of elements
 *  @param  tokenizer  [in] tokenizer
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
template <typename T>
inline const bool ReadInternalData(
    vismodule::ValueArray<T>* data_array,
    const size_t nelements,
    vismodule::Tokenizer& tokenizer )
{
    T* data = static_cast<T*>( data_array->allocate( nelements ) );
    if ( !data )
    {
        visModuleMessageError( "Cannot allocate memory for the internal data." );
        return false;
    }

    const size_t nloops = data_array->size();
    for ( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<T>( atof( tokenizer.token().c_str() ) );
    }

    return true;
}

template <>
inline const bool ReadInternalData<vismodule::Int8>(
    vismodule::ValueArray<vismodule::Int8>* data_array,
    const size_t nelements,
    vismodule::Tokenizer& tokenizer )
{
    vismodule::Int8* data = static_cast<vismodule::Int8*>( data_array->allocate( nelements ) );
    if ( !data )
    {
        visModuleMessageError( "Cannot allocate memory for the internal data." );
        return false;
    }

    const size_t nloops = data_array->size();
    for ( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<vismodule::Int8>( atoi( tokenizer.token().c_str() ) );
    }

    return true;
}

template <>
inline const bool ReadInternalData<vismodule::UInt8>(
    vismodule::ValueArray<vismodule::UInt8>* data_array,
    const size_t nelements,
    vismodule::Tokenizer& tokenizer )
{
    vismodule::UInt8* data = static_cast<vismodule::UInt8*>( data_array->allocate( nelements ) );
    if ( !data )
    {
        visModuleMessageError( "Cannot allocate memory for the internal data." );
        return false;
    }

    const size_t nloops = data_array->size();
    for ( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<vismodule::UInt8>( atoi( tokenizer.token().c_str() ) );
    }

    return true;
}

/*===========================================================================*/
/**
 *  @brief  Reads the external data as any-value array.
 *  @param  data_array [out] pointer to the any-value array
 *  @param  nelements  [in] number of elements
 *  @param  filename   [in] external file name
 *  @param  format     [in] file format (binary or ascii)
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
template <typename T>
inline const bool ReadExternalData(
    vismodule::AnyValueArray* data_array,
    const size_t nelements,
    const std::string& filename,
    const std::string& format )
{
    if ( !data_array->template allocate<T>( nelements ) )
    {
        visModuleMessageError( "Cannot allocate memory for the external data." );
        return false;
    }

    if ( format == "binary" )
    {
        FILE* ifs = fopen( filename.c_str(), "rb" );
        if ( !ifs )
        {
            visModuleMessageError( "Cannot open '%s'.", filename.c_str() );
            return false;
        }

        const size_t data_size = data_array->size();
        if ( fread( data_array->pointer(), sizeof( T ), data_size, ifs ) != data_size )
        {
            visModuleMessageError( "Cannot read '%s'.", filename.c_str() );
            fclose( ifs );
            return false;
        }

        fclose( ifs );
#if VIS_MODULE_BYTE_ORDER == VIS_MODULE_BIG_ENDIAN
        data_array->swapByte();
#endif
    }
    else if ( format == "ascii" )
    {
        FILE* ifs = fopen( filename.c_str(), "r" );
        if ( !ifs )
        {
            visModuleMessageError( "Cannot open '%s'.", filename.c_str() );
            return false;
        }

        fseek( ifs, 0, SEEK_END );
        const size_t size = ftell( ifs );

        char* buffer = static_cast<char*>( malloc( sizeof( char ) * size ) );
        if ( !buffer )
        {
            visModuleMessageError( "Cannot allocate memory for reading the external data." );
            fclose( ifs );
            return false;
        }

        fseek( ifs, 0, SEEK_SET );
        if ( size != fread( buffer, 1, size, ifs ) )
        {
            visModuleMessageError( "Cannot read '%s'.", filename.c_str() );
            return false;
        }

        T* data = static_cast<T*>( data_array->pointer() );

        const char* delim = " ,\t\n";
        char* value = strtok( buffer, delim );
        for ( size_t i = 0; i < nelements; i++ )
        {
            if ( value )
            {
                *( data++ ) = static_cast<T>( atof( value ) );
                value = strtok( 0, delim );
            }
        }

        free( buffer );

        fclose( ifs );
    }
    else
    {
        visModuleMessageError( "Unknown format '%s'.", format.c_str() );
        return false;
    }

    return true;
}

/*===========================================================================*/
/**
 *  @brief  Reads the external data as value array.
 *  @param  data_array [out] pointer to the value array
 *  @param  nelements  [in] number of elements
 *  @param  filename   [in] external file name
 *  @param  format     [in] file format (binary or ascii)
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
template <typename T1, typename T2>
inline const bool ReadExternalData(
    vismodule::ValueArray<T1>* data_array,
    const size_t nelements,
    const std::string& filename,
    const std::string& format )
{
    if ( !data_array->allocate( nelements ) )
    {
        visModuleMessageError( "Cannot allocate memory for the external data." );
        return false;
    }

    if ( format == "binary" )
    {
        FILE* ifs = fopen( filename.c_str(), "rb" );
        if ( !ifs )
        {
            visModuleMessageError( "Cannot open '%s'.", filename.c_str() );
            return false;
        }

        if ( typeid( T1 ) == typeid( T2 ) )
        {
            const size_t data_size = data_array->size();
            if ( fread( data_array->pointer(), sizeof( T1 ), data_size, ifs ) != data_size )
            {
                visModuleMessageError( "Cannot read '%s'.", filename.c_str() );
                fclose( ifs );
                return false;
            }
        }
        else
        {
            const size_t nloops = data_array->size();
            for ( size_t i = 0; i < nloops; i++ )
            {
                T2 data = T2( 0 );
                if ( fread( &data, sizeof( T2 ), 1, ifs ) != 1 )
                {
                    visModuleMessageError( "Cannot read '%s'.", filename.c_str() );
                    fclose( ifs );
                    return false;
                }
                data_array->at( i ) = static_cast<T1>( data );
            }
        }
        fclose( ifs );
#if VIS_MODULE_BYTE_ORDER == VIS_MODULE_BIG_ENDIAN
        data_array->swapByte();
#endif
    }
    else if ( format == "ascii" )
    {
        FILE* ifs = fopen( filename.c_str(), "r" );
        if ( !ifs )
        {
            visModuleMessageError( "Cannot open '%s'.", filename.c_str() );
            return false;
        }

        fseek( ifs, 0, SEEK_END );
        const size_t size = ftell( ifs );

        char* buffer = static_cast<char*>( malloc( sizeof( char ) * size ) );
        if ( !buffer )
        {
            visModuleMessageError( "Cannot allocate memory." );
            fclose( ifs );
            return false;
        }

        fseek( ifs, 0, SEEK_SET );
        if ( size != fread( buffer, 1, size, ifs ) )
        {
            visModuleMessageError( "Cannot read '%s'.", filename.c_str() );
            return false;
        }

        T1* data = data_array->pointer();

        const char* delim = " ,\t\n";
        char* value = strtok( buffer, delim );
        for ( size_t i = 0; i < nelements; i++ )
        {
            if ( value )
            {
                *( data++ ) = static_cast<T1>( atof( value ) );
                value = strtok( 0, delim );
            }
        }

        free( buffer );

        fclose( ifs );
    }
    else
    {
        visModuleMessageError( "Unknown format '%s'.", format.c_str() );
        return false;
    }

    return true;
}

/*===========================================================================*/
/**
 *  @brief  Writes the external data as any-value array.
 *  @param  data_array [in] data array
 *  @param  filename [in] output file name
 *  @param  format [in] output file format
 *  @return true, if the writting process is done successfully
 */
/*===========================================================================*/
inline const bool WriteExternalData(
    const vismodule::AnyValueArray& data_array,
    const std::string& filename,
    const std::string& format )
{
    if ( format == "ascii" )
    {
        std::ofstream ofs( filename.c_str() );
        if ( ofs.fail() )
        {
            visModuleMessageError( "Cannot open file '%s'.", filename.c_str() );
            return false;
        }

        const std::string delim( ", " );
        const std::type_info& data_type = data_array.typeInfo()->type();
        const size_t data_size = data_array.size();
        if ( data_type == typeid( vismodule::Int8 ) )
        {
            const vismodule::Int8* values = data_array.pointer<vismodule::Int8>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << vismodule::Int16( values[i] ) << delim;
        }
        else if ( data_type == typeid( vismodule::UInt8 ) )
        {
            const vismodule::UInt8* values = data_array.pointer<vismodule::UInt8>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << vismodule::UInt16( values[i] ) << delim;
        }
        else if ( data_type == typeid( vismodule::Int16 ) )
        {
            const vismodule::Int16* values = data_array.pointer<vismodule::Int16>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }
        else if ( data_type == typeid( vismodule::UInt16 ) )
        {
            const vismodule::UInt16* values = data_array.pointer<vismodule::UInt16>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }
        else if ( data_type == typeid( vismodule::Int32 ) )
        {
            const vismodule::Int32* values = data_array.pointer<vismodule::Int32>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }
        else if ( data_type == typeid( vismodule::UInt32 ) )
        {
            const vismodule::UInt32* values = data_array.pointer<vismodule::UInt32>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }
        else if ( data_type == typeid( vismodule::Real32 ) )
        {
            const vismodule::Real32* values = data_array.pointer<vismodule::Real32>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }
        else if ( data_type == typeid( vismodule::Real64 ) )
        {
            const vismodule::Real64* values = data_array.pointer<vismodule::Real64>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }

        ofs.close();
    }
    else if ( format == "binary" )
    {
        std::ofstream ofs( filename.c_str(), std::ios::out | std::ios::binary );
        if ( ofs.fail() )
        {
            visModuleMessageError( "Cannot open file '%s'.", filename.c_str() );
            return false;
        }
        const void* data_pointer = data_array.pointer();
        const size_t data_byte_size = data_array.byteSize();
        ofs.write( static_cast<const char*>( data_pointer ), data_byte_size );
        ofs.close();
    }
    else
    {
        visModuleMessageError( "Unknown format '%s'.", format.c_str() );
        return false;
    }

    return true;
}

/*===========================================================================*/
/**
 *  @brief  Writes the external data as value array.
 *  @param  data_array [in] data array
 *  @param  filename [in] output file name
 *  @param  format [in] output file format
 *  @return true, if the writting process is done successfully
 */
/*===========================================================================*/
template <typename T>
inline const bool WriteExternalData(
    const vismodule::ValueArray<T>& data_array,
    const std::string& filename,
    const std::string& format )
{
    if ( format == "ascii" )
    {
        std::ofstream ofs( filename.c_str() );
        if ( ofs.fail() )
        {
            visModuleMessageError( "Cannot open file '%s'.", filename.c_str() );
            return false;
        }

        const std::string delim( ", " );
        const size_t data_size = data_array.size();
        if ( typeid( T ) == typeid( vismodule::Int8 ) || typeid( T ) == typeid( vismodule::UInt8 ) )
        {
            for ( size_t i = 0; i < data_size; i++ ) ofs << int( data_array.at( i ) ) << delim;
        }
        else
        {
            for ( size_t i = 0; i < data_size; i++ ) ofs << data_array.at( i ) << delim;
        }

        ofs.close();
    }
    else if ( format == "binary" )
    {
        std::ofstream ofs( filename.c_str(), std::ios::out | std::ios::binary );
        if ( ofs.fail() )
        {
            visModuleMessageError( "Cannot open file '%s'.", filename.c_str() );
            return false;
        }
        const char* data_pointer = reinterpret_cast<const char*>( data_array.pointer() );
        const size_t data_byte_size = data_array.byteSize();
        ofs.write( data_pointer, data_byte_size );
        ofs.close();
    }

    return true;
}

} // end of namespace DataArray

} // end of namespace vismoduleml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__DATA_ARRAY_H_INCLUDE
