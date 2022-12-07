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
#ifndef KVS__KVSML__DATA_ARRAY_H_INCLUDE
#define KVS__KVSML__DATA_ARRAY_H_INCLUDE

#include <kvs/File>
#include <kvs/Tokenizer>
#include <kvs/ValueArray>
#include <kvs/AnyValueArray>
#include <kvs/IgnoreUnusedVariable>
#include <iostream>
#include <fstream>
#include <sstream>


namespace kvs
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
    const std::string basename( kvs::File( filename ).baseName() );
//    const std::string pathname( kvs::File( filename ).pathName() );
//    const std::string sep( kvs::File::Separator() );
    const std::string ext( "dat" );

//    return( pathname + sep + basename + "_" + type + "." + ext );
    return( basename + "_" + type + "." + ext );
}

/*===========================================================================*/
/**
 *  @brief  Returns the data type of the given value array.
 *  @param  data_array [in] data array
 *  @return data type as string
 */
/*===========================================================================*/
template <typename T>
inline const std::string GetDataType( const kvs::ValueArray<T>& data_array )
{
    kvs::IgnoreUnusedVariable( data_array );

    if (      typeid(T) == typeid(kvs::Int8)   ) return("char");
    else if ( typeid(T) == typeid(kvs::UInt8)  ) return("uchar");
    else if ( typeid(T) == typeid(kvs::Int16)  ) return("short");
    else if ( typeid(T) == typeid(kvs::UInt16) ) return("ushort");
    else if ( typeid(T) == typeid(kvs::Int32)  ) return("int");
    else if ( typeid(T) == typeid(kvs::UInt32) ) return("uint");
    else if ( typeid(T) == typeid(kvs::Int64)  ) return("long");
    else if ( typeid(T) == typeid(kvs::UInt64) ) return("ulong");
    else if ( typeid(T) == typeid(kvs::Real32) ) return("float");
    else if ( typeid(T) == typeid(kvs::Real64) ) return("double");
    else return("unknown");
}

/*===========================================================================*/
/**
 *  @brief  Returns the data type of the given any-value array.
 *  @param  data_array [in] data array
 *  @return data type as string
 */
/*===========================================================================*/
inline const std::string GetDataType( const kvs::AnyValueArray& data_array )
{
    const std::type_info& type = data_array.typeInfo()->type();
    if (      type == typeid(kvs::Int8)   ) return("char");
    else if ( type == typeid(kvs::UInt8)  ) return("uchar");
    else if ( type == typeid(kvs::Int16)  ) return("short");
    else if ( type == typeid(kvs::UInt16) ) return("ushort");
    else if ( type == typeid(kvs::Int32)  ) return("int");
    else if ( type == typeid(kvs::UInt32) ) return("uint");
    else if ( type == typeid(kvs::Int64)  ) return("long");
    else if ( type == typeid(kvs::UInt64) ) return("ulong");
    else if ( type == typeid(kvs::Real32) ) return("float");
    else if ( type == typeid(kvs::Real64) ) return("double");
    else return("unknown");
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
    kvs::AnyValueArray* data_array,
    const size_t nelements,
    kvs::Tokenizer& tokenizer )
{
    T* data = static_cast<T*>( data_array->template allocate<T>( nelements ) );
    if ( !data )
    {
        kvsMessageError( "Cannot allocate memory for the internal data." );
        return( false );
    }

    const size_t nloops = data_array->size();
    for( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<T>( atof( tokenizer.token().c_str() ) );
    }

    return( true );
}

template <>
inline const bool ReadInternalData<kvs::Int8>(
    kvs::AnyValueArray* data_array,
    const size_t nelements,
    kvs::Tokenizer& tokenizer )
{
    kvs::Int8* data = static_cast<kvs::Int8*>( data_array->allocate<kvs::Int8>( nelements ) );
    if ( !data )
    {
        kvsMessageError( "Cannot allocate memory for the internal data." );
        return( false );
    }

    const size_t nloops = data_array->size();
    for( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<kvs::Int8>( atoi( tokenizer.token().c_str() ) );
    }

    return( true );
}

template <>
inline const bool ReadInternalData<kvs::UInt8>(
    kvs::AnyValueArray* data_array,
    const size_t nelements,
    kvs::Tokenizer& tokenizer )
{
    kvs::UInt8* data = static_cast<kvs::UInt8*>( data_array->allocate<kvs::UInt8>( nelements ) );
    if ( !data )
    {
        kvsMessageError( "Cannot allocate memory for the internal data." );
        return( false );
    }

    const size_t nloops = data_array->size();
    for( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<kvs::UInt8>( atoi( tokenizer.token().c_str() ) );
    }

    return( true );
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
    kvs::ValueArray<T>* data_array,
    const size_t nelements,
    kvs::Tokenizer& tokenizer )
{
    T* data = static_cast<T*>( data_array->allocate( nelements ) );
    if ( !data )
    {
        kvsMessageError( "Cannot allocate memory for the internal data." );
        return( false );
    }

    const size_t nloops = data_array->size();
    for( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<T>( atof( tokenizer.token().c_str() ) );
    }

    return( true );
}

template <>
inline const bool ReadInternalData<kvs::Int8>(
    kvs::ValueArray<kvs::Int8>* data_array,
    const size_t nelements,
    kvs::Tokenizer& tokenizer )
{
    kvs::Int8* data = static_cast<kvs::Int8*>( data_array->allocate( nelements ) );
    if ( !data )
    {
        kvsMessageError( "Cannot allocate memory for the internal data." );
        return( false );
    }

    const size_t nloops = data_array->size();
    for( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<kvs::Int8>( atoi( tokenizer.token().c_str() ) );
    }

    return( true );
}

template <>
inline const bool ReadInternalData<kvs::UInt8>(
    kvs::ValueArray<kvs::UInt8>* data_array,
    const size_t nelements,
    kvs::Tokenizer& tokenizer )
{
    kvs::UInt8* data = static_cast<kvs::UInt8*>( data_array->allocate( nelements ) );
    if ( !data )
    {
        kvsMessageError( "Cannot allocate memory for the internal data." );
        return( false );
    }

    const size_t nloops = data_array->size();
    for( size_t i = 0; i < nloops; i++ )
    {
        data[i] = static_cast<kvs::UInt8>( atoi( tokenizer.token().c_str() ) );
    }

    return( true );
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
    kvs::AnyValueArray* data_array,
    const size_t nelements,
    const std::string& filename,
    const std::string& format )
{
    if ( !data_array->template allocate<T>( nelements ) )
    {
        kvsMessageError( "Cannot allocate memory for the external data." );
        return( false );
    }

    if ( format == "binary" )
    {
        FILE* ifs = fopen( filename.c_str(), "rb" );
        if( !ifs )
        {
            kvsMessageError("Cannot open '%s'.", filename.c_str());
            return( false );
        }

        const size_t data_size = data_array->size();
        if ( fread( data_array->pointer(), sizeof(T), data_size, ifs ) != data_size )
        {
            kvsMessageError("Cannot read '%s'.", filename.c_str());
            fclose( ifs );
            return( false );
        }

        fclose( ifs );
    }
    else if ( format == "ascii" )
    {
        FILE* ifs = fopen( filename.c_str(), "r" );
        if( !ifs )
        {
            kvsMessageError("Cannot open '%s'.", filename.c_str());
            return( false );
        }

        fseek( ifs, 0, SEEK_END );
        const size_t size = ftell( ifs );

        char* buffer = static_cast<char*>( malloc( sizeof(char) * size ) );
        if ( !buffer )
        {
            kvsMessageError("Cannot allocate memory for reading the external data.");
            fclose( ifs );
            return( false );
        }

        fseek( ifs, 0, SEEK_SET );
        if ( size != fread( buffer, 1, size, ifs ) )
        {
            kvsMessageError( "Cannot read '%s'.", filename.c_str() );
            return( false );
        }

        T* data = static_cast<T*>( data_array->pointer() );

        const char* delim = " ,\t\n";
        char* value = strtok( buffer, delim );
        for ( size_t i = 0; i < nelements; i++ )
        {
            if ( value )
            {
                *(data++) = static_cast<T>( atof( value ) );
                value = strtok( 0, delim );
            }
        }

        free( buffer );

        fclose( ifs );
    }
    else
    {
        kvsMessageError("Unknown format '%s'.",format.c_str());
        return( false );
    }

    return( true );
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
    kvs::ValueArray<T1>* data_array,
    const size_t nelements,
    const std::string& filename,
    const std::string& format )
{
    if ( !data_array->allocate( nelements ) )
    {
        kvsMessageError( "Cannot allocate memory for the external data." );
        return( false );
    }

    if( format == "binary" )
    {
        FILE* ifs = fopen( filename.c_str(), "rb" );
        if( !ifs )
        {
            kvsMessageError("Cannot open '%s'.", filename.c_str());
            return( false );
        }

        if( typeid(T1) == typeid(T2) )
        {
            const size_t data_size = data_array->size();
            if ( fread( data_array->pointer(), sizeof(T1), data_size, ifs ) != data_size )
            {
                kvsMessageError("Cannot read '%s'.",filename.c_str());
                fclose( ifs );
                return( false );
            }
        }
        else
        {
            const size_t nloops = data_array->size();
            for( size_t i = 0; i < nloops; i++ )
            {
                T2 data = T2(0);
                if ( fread( &data, sizeof(T2), 1, ifs ) != 1 )
                {
                    kvsMessageError("Cannot read '%s'.",filename.c_str());
                    fclose( ifs );
                    return( false );
                }
                data_array->at(i) = static_cast<T1>( data );
            }
        }
        fclose( ifs );
    }
    else if( format == "ascii" )
    {
        FILE* ifs = fopen( filename.c_str(), "r" );
        if( !ifs )
        {
            kvsMessageError("Cannot open '%s'.", filename.c_str());
            return( false );
        }

        fseek( ifs, 0, SEEK_END );
        const size_t size = ftell( ifs );

        char* buffer = static_cast<char*>( malloc( sizeof(char) * size ) );
        if ( !buffer )
        {
            kvsMessageError("Cannot allocate memory.");
            fclose( ifs );
            return( false );
        }

        fseek( ifs, 0, SEEK_SET );
        if ( size != fread( buffer, 1, size, ifs ) )
        {
            kvsMessageError( "Cannot read '%s'.", filename.c_str() );
            return( false );
        }

        T1* data = data_array->pointer();

        const char* delim = " ,\t\n";
        char* value = strtok( buffer, delim );
        for ( size_t i = 0; i < nelements; i++ )
        {
            if ( value )
            {
                *(data++) = static_cast<T1>( atof( value ) );
                value = strtok( 0, delim );
            }
        }

        free( buffer );

        fclose( ifs );
    }
    else
    {
        kvsMessageError("Unknown format '%s'.",format.c_str());
        return( false );
    }

    return( true );
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
    const kvs::AnyValueArray& data_array,
    const std::string& filename,
    const std::string& format )
{
    if ( format == "ascii" )
    {
        std::ofstream ofs( filename.c_str() );
        if ( ofs.fail() )
        {
            kvsMessageError("Cannot open file '%s'.", filename.c_str() );
            return( false );
        }

        const std::string delim(", ");
        const std::type_info& data_type = data_array.typeInfo()->type();
        const size_t data_size = data_array.size();
        if ( data_type == typeid(kvs::Int8) )
        {
            const kvs::Int8* values = data_array.pointer<kvs::Int8>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << kvs::Int16(values[i]) << delim;
        }
        else if ( data_type == typeid(kvs::UInt8) )
        {
            const kvs::UInt8* values = data_array.pointer<kvs::UInt8>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << kvs::UInt16(values[i]) << delim;
        }
        else if ( data_type == typeid(kvs::Int16) )
        {
            const kvs::Int16* values = data_array.pointer<kvs::Int16>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }
        else if ( data_type == typeid(kvs::UInt16) )
        {
            const kvs::UInt16* values = data_array.pointer<kvs::UInt16>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }
        else if ( data_type == typeid(kvs::Int32) )
        {
            const kvs::Int32* values = data_array.pointer<kvs::Int32>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }
        else if ( data_type == typeid(kvs::UInt32) )
        {
            const kvs::UInt32* values = data_array.pointer<kvs::UInt32>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }
        else if ( data_type == typeid(kvs::Real32) )
        {
            const kvs::Real32* values = data_array.pointer<kvs::Real32>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }
        else if ( data_type == typeid(kvs::Real64) )
        {
            const kvs::Real64* values = data_array.pointer<kvs::Real64>();
            for ( size_t i = 0; i < data_size; i++ ) ofs << values[i] << delim;
        }

        ofs.close();
    }
    else if ( format == "binary" )
    {
        std::ofstream ofs( filename.c_str(), std::ios::out | std::ios::binary );
        if ( ofs.fail() )
        {
            kvsMessageError("Cannot open file '%s'.", filename.c_str() );
            return( false );
        }
        const void* data_pointer = data_array.pointer();
        const size_t data_byte_size = data_array.byteSize();
        ofs.write( static_cast<const char*>(data_pointer), data_byte_size );
        ofs.close();
    }
    else
    {
        kvsMessageError("Unknown format '%s'.",format.c_str());
        return( false );
    }

    return( true );
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
    const kvs::ValueArray<T>& data_array,
    const std::string& filename,
    const std::string& format )
{
    if ( format == "ascii" )
    {
        std::ofstream ofs( filename.c_str() );
        if ( ofs.fail() )
        {
            kvsMessageError("Cannot open file '%s'.", filename.c_str() );
            return( false );
        }

        const std::string delim(", ");
        const size_t data_size = data_array.size();
        if ( typeid(T) == typeid(kvs::Int8) || typeid(T) == typeid(kvs::UInt8) )
        {
            for ( size_t i = 0; i < data_size; i++ ) ofs << int(data_array.at(i)) << delim;
        }
        else
        {
            for ( size_t i = 0; i < data_size; i++ ) ofs << data_array.at(i) << delim;
        }

        ofs.close();
    }
    else if ( format == "binary" )
    {
        std::ofstream ofs( filename.c_str(), std::ios::out | std::ios::binary );
        if ( ofs.fail() )
        {
            kvsMessageError("Cannot open file '%s'.", filename.c_str() );
            return( false );
        }
        const char* data_pointer = reinterpret_cast<const char*>( data_array.pointer() );
        const size_t data_byte_size = data_array.byteSize();
        ofs.write( data_pointer, data_byte_size );
        ofs.close();
    }

    return( true );
}

} // end of namespace DataArray

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__DATA_ARRAY_H_INCLUDE
