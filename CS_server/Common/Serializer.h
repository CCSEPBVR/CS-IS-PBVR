//
// instance の sizeof はコンパイラ、環境依存
// serialize するときのバイト列の大きさは、必要なメンバをインスタンス化するために改めて計算する
// std::string で受け渡すと '\0' を途中で含むバイト列の時に、.c_str() で得られるバイト列から再構築できなくなる
//
#pragma once

#include <string>
#include <cstdlib>
#include <cstring>

namespace kvs
{
class Camera;
class TransferFunction;
}

namespace jpv
{

class Serializer
{
public:
    template<typename T>
    static size_t byteSize( const T& object )
    {
        return sizeof( object );
    }
    template<typename T>
    static size_t pack( char* buf, const T& object )
    {
        std::memcpy( buf, object, sizeof( object ) );
        return sizeof( object );
    }
    template<typename T>
    static size_t unpack( const char* buf, T& object )
    {
        std::memcpy( object, buf, sizeof( object ) );
        return sizeof( object );
    }
    template<typename T>
    static size_t writeArray( char* buf, const T& t )
    {
        std::memcpy( buf, t, sizeof( t ) );
        return sizeof( t );
    }
    template<typename T>
    static size_t writeArray( char* buf, const T* t, size_t sz )
    {
        std::memcpy( buf, t, sizeof( T )*sz );
        return sizeof( T ) * sz;
    }
    template<typename T>
    static size_t readArray( const char* buf, T& t )
    {
        std::memcpy( t, buf, sizeof( t ) );
        return sizeof( t );
    }
    template<typename T>
    static size_t readArray( const char* buf, T* t, size_t sz )
    {
        std::memcpy( t, buf, sizeof( T )*sz );
        return sizeof( T ) * sz;
    }
    template<typename T>
    static size_t write( char* buf, const T& t )
    {
        std::memcpy( buf, &t, sizeof( t ) );
        return sizeof( t );
    }
    template<typename T>
    static size_t read( const char* buf, T& t )
    {
        std::memcpy( &t, buf, sizeof( t ) );
        return sizeof( t );
    }
};

template<>
size_t Serializer::byteSize<std::string>( const std::string& object );
template<>
size_t Serializer::write<std::string>( char* buf, const std::string& object );
template<>
size_t Serializer::read<std::string>( const char* buf, std::string& object );

template<>
size_t Serializer::byteSize<kvs::Camera>( const kvs::Camera& object );
template<>
size_t Serializer::pack<kvs::Camera>( char* buf, const kvs::Camera& object );
template<>
size_t Serializer::unpack<kvs::Camera>( const char* buf, kvs::Camera& object );

template<>
size_t Serializer::byteSize<kvs::TransferFunction>( const kvs::TransferFunction& object );
template<>
size_t Serializer::pack<kvs::TransferFunction>( char* buf, const kvs::TransferFunction& object );
template<>
size_t Serializer::unpack<kvs::TransferFunction>( const char* buf, kvs::TransferFunction& object );

}

#ifdef FJ_BX900
#include "Serializer.cpp"
#endif

