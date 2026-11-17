#include "Serializer.h"

#include <vismodule/Xform>

// namespace define for g++
namespace vismodule
{
template<>
size_t Serializer::byteSize<std::string>( const std::string& object )
{
    std::size_t size = 0;
    size += sizeof( std::size_t );
    size += object.size() + 1;
    return size;
}

template<>
size_t Serializer::write<std::string>( char* buf, const std::string& object )
{
    std::size_t index = 0;
    index += write( buf + index, object.size() + 1 );
    index += writeArray( buf + index, object.c_str(), object.size() );
    index += write( buf + index, '\0' );
    return index;
}

template<>
size_t Serializer::read<std::string>( const char* buf, std::string* object )
{
    std::size_t index = 0, size;
    char* tmp;
    index += read( buf + index, &size );
    tmp = new char[size];
    index += readArray( buf + index, tmp, size );
    *object = std::string( tmp );
    delete[] tmp;
    return index;
}

template<>
size_t Serializer::byteSize<vismodule::Camera>( const vismodule::Camera& object )
{
    std::size_t size = 0;
    size += sizeof( float ) * 9; // rotation
    size += sizeof( float ) * 3; // transition
    size += sizeof( float ) * 3; // scaling
    size += sizeof( vismodule::Camera::ProjectionType );
    size += sizeof( float ) * 3; // position
    size += sizeof( float ) * 3; // upvector
    size += sizeof( float ) * 3; // lookat
    size += sizeof( float ); // fieldofview
    size += sizeof( float ); // back
    size += sizeof( float ); // front
    size += sizeof( float ); // right
    size += sizeof( float ); // left
    size += sizeof( float ); // bottom
    size += sizeof( float ); // top
    size += sizeof( std::size_t ); // width
    size += sizeof( std::size_t ); // hight
    return size;
}

template<>
size_t Serializer::pack<vismodule::Camera>( char* buf, const vismodule::Camera& object )
{
    float r[9];
    for ( int i = 0; i < 9; ++i )
    {
        //KVS2.7.0
        //MOD BY)T0603 2020.05.28
        //r[i] = object.rotation()[i / 3][i % 3];
        r[i] = object.xform().rotation()[i / 3][i % 3];
    }
    float t[3];
    for ( int i = 0; i < 3; ++i )
    {
        //KVS2.7.0
        //MOD BY)T0603 2020.05.28
        //t[i] = object.translation()[i];
        t[i] = object.xform().translation()[i];
    }
    float s[3];
    for ( int i = 0; i < 3; ++i )
    {
        //KVS2.7.0
        //MOD BY)T0603 2020.05.28
        //s[i] = object.scaling()[i];
        s[i] = object.xform().scaling()[i];
    }
    std::size_t index = 0;
    index += writeArray( buf + index, r );
    index += writeArray( buf + index, t );
    index += writeArray( buf + index, s );
    index += write( buf + index, object.projectionType() );
    index += write( buf + index, object.position().x() );
    index += write( buf + index, object.position().y() );
    index += write( buf + index, object.position().z() );
    index += write( buf + index, object.upVector().x() );
    index += write( buf + index, object.upVector().y() );
    index += write( buf + index, object.upVector().z() );
    index += write( buf + index, object.lookAt().x() );
    index += write( buf + index, object.lookAt().y() );
    index += write( buf + index, object.lookAt().z() );
    index += write( buf + index, object.fieldOfView() );
    index += write( buf + index, object.back() );
    index += write( buf + index, object.front() );
    index += write( buf + index, object.right() );
    index += write( buf + index, object.left() );
    index += write( buf + index, object.bottom() );
    index += write( buf + index, object.top() );
    index += write( buf + index, object.windowWidth() );
    index += write( buf + index, object.windowHeight() );
    return index;
}

template<>
size_t Serializer::unpack<vismodule::Camera>( const char* buf, vismodule::Camera* object )
{
    float r[9];
    float t[3];
    float s[3];
    float x, y, z, v;
    std::size_t index = 0;
    index += readArray( buf + index, r );
    index += readArray( buf + index, t );
    index += readArray( buf + index, s );
    vismodule::Matrix33f rotation( r );
    vismodule::Vector3f translation( t );
    vismodule::Vector3f scaling( s );
    //KVS2.7.0
    //MOD BY)T0603 2020.05.28
    //object->set( translation, scaling, rotation );
    object->setXform( vismodule::Xform( translation, scaling, rotation ) );
    vismodule::Camera::ProjectionType pType;
    index += read( buf + index, &pType );
    object->setProjectionType( pType );
    index += read( buf + index, &x );
    index += read( buf + index, &y );
    index += read( buf + index, &z );
    object->setPosition( vismodule::Vector3f( x, y, z ) );
    index += read( buf + index, &x );
    index += read( buf + index, &y );
    index += read( buf + index, &z );
    object->setUpVector( vismodule::Vector3f( x, y, z ) );
    index += read( buf + index, &x );
    index += read( buf + index, &y );
    index += read( buf + index, &z );
    object->setLookAt( vismodule::Vector3f( x, y, z ) );
    index += read( buf + index, &v );
    object->setFieldOfView( v );
    index += read( buf + index, &v );
    object->setBack( v );
    index += read( buf + index, &v );
    object->setFront( v );
    index += read( buf + index, &v );
    object->setRight( v );
    index += read( buf + index, &v );
    object->setLeft( v );
    index += read( buf + index, &v );
    object->setBottom( v );
    index += read( buf + index, &v );
    object->setTop( v );
    std::size_t w, h;
    index += read( buf + index, &w );
    index += read( buf + index, &h );
    object->setWindowSize( w, h );
    return index;
}

template<>
size_t Serializer::byteSize<vismodule::TransferFunction>( const vismodule::TransferFunction& object )
{
    std::size_t size = 0;
    size += sizeof( object.resolution() );
    size += sizeof( object.maxValue() );
    size += sizeof( object.minValue() );
    size += object.resolution() * 3 * sizeof( vismodule::UInt8 );
    size += object.resolution() * sizeof( float );
//	size += object.colorMap().table().size() * 3 * sizeof(vismodule::UInt8);
//	size += object.opacityMap().table().size() * sizeof(float);
    return size;
}

template<>
size_t Serializer::pack<vismodule::TransferFunction>( char* buf, const vismodule::TransferFunction& object )
{
    std::size_t index = 0;
    const std::size_t resolution = object.resolution();
    index += write( buf + index, resolution );
    index += write( buf + index, object.maxValue() );
    index += write( buf + index, object.minValue() );
    const vismodule::ColorMap::Table& colorTable = object.colorMap().table();
    for ( std::size_t i = 0; i < resolution; ++i )
    {
        index += write( buf + index, colorTable[3 * i + 0] );
        index += write( buf + index, colorTable[3 * i + 1] );
        index += write( buf + index, colorTable[3 * i + 2] );
    }
    const vismodule::OpacityMap::Table& opacityTable = object.opacityMap().table();
    for ( std::size_t i = 0; i < resolution; ++i )
    {
        index += write( buf + index, opacityTable[i] );
    }
    return index;
}

template<>
size_t Serializer::unpack<vismodule::TransferFunction>( const char* buf, vismodule::TransferFunction* object )
{
    std::size_t index = 0;
    std::size_t resolution;
    float max_value, min_value;
    index += read( buf + index, &resolution );
    index += read( buf + index, &max_value );
    index += read( buf + index, &min_value );

    vismodule::ColorMap::Table colorTable( resolution * 3 );
    for ( std::size_t i = 0; i < resolution; ++i )
    {
        index += read( buf + index, &colorTable[3 * i + 0] );
        index += read( buf + index, &colorTable[3 * i + 1] );
        index += read( buf + index, &colorTable[3 * i + 2] );
    }
    vismodule::OpacityMap::Table opacityTable( resolution );
    for ( std::size_t i = 0; i < resolution; ++i )
    {
        index += read( buf + index, &opacityTable[i] );
    }
    vismodule::ColorMap colorMap( colorTable );
    vismodule::OpacityMap opacityMap( opacityTable );
    object->create( resolution );
    colorMap.setResolution( resolution );
    colorMap.setRange( min_value, max_value );
    opacityMap.setResolution( resolution );
    opacityMap.setRange( min_value, max_value );
    object->setColorMap( colorMap );
    object->setOpacityMap( opacityMap );
    return index;
}
}
