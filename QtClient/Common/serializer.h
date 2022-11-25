//
// instance �� sizeof �̓R���p�C���A���ˑ�
// serialize �����Ƃ��̃o�C�g���̑傫���́A�K�v�ȃ����o���C���X�^���X�����邽�߂ɉ��߂Čv�Z����
// std::string �Ŏ󂯓n���� '\0' ���r���Ŋ܂ރo�C�g���̎��ɁA.c_str() �œ��������o�C�g�񂩂��č\�z�ł��Ȃ��Ȃ�
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

