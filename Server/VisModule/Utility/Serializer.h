//
// instance の sizeof はコンパイラ、環境依存
// serialize するときのバイト列の大きさは、必要なメンバをインスタンス化するために改めて計算する
// std::string で受け渡すと '¥0' を途中で含むバイト列の時に、.c_str() で得られるバイト列から再構築できなくなる
//
#ifndef VISMODULE__JPV__SERIALIZER_H_INCLUDE
#define VISMODULE__JPV__SERIALIZER_H_INCLUDE

#include <string>
#include <cstdlib>
#include <cstring>

#include <vismodule/Camera>
#include <vismodule/TransferFunction>

namespace vismodule
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
    static size_t unpack( const char* buf, T* object )
    {
        std::memcpy( object, buf, sizeof( *object ) );
        return sizeof( *object );
    }
    template<typename T>
    static size_t writeArray( char* buf, const T& t )
    {
        std::memcpy( buf, t, sizeof( t ) );
        return sizeof( t );
    }
    template<typename T>
    static size_t writeArray( char* buf, const T* t, const size_t sz )
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
    static size_t readArray( const char* buf, T* t, const size_t sz )
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
    static size_t read( const char* buf, T* t )
    {
        std::memcpy( t, buf, sizeof( *t ) );
        return sizeof( *t );
    }

    template<>
    size_t byteSize<std::string>( const std::string& object );
    template<>
    size_t write<std::string>( char* buf, const std::string& object );
    template<>
    size_t read<std::string>( const char* buf, std::string* object );

    template<>
    size_t byteSize<vismodule::Camera>( const vismodule::Camera& object );
    template<>
    size_t pack<vismodule::Camera>( char* buf, const vismodule::Camera& object );
    template<>
    size_t unpack<vismodule::Camera>( const char* buf, vismodule::Camera* object );

    template<>
    size_t byteSize<vismodule::TransferFunction>( const vismodule::TransferFunction& object );
    template<>
    size_t pack<vismodule::TransferFunction>( char* buf, const vismodule::TransferFunction& object );
    template<>
    size_t unpack<vismodule::TransferFunction>( const char* buf, vismodule::TransferFunction* object );
};
}

#endif // VISMODULE__JPV__SERIALIZER_H_INCLUDE
