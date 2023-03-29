#include "ParticleTransferProtocol.h"
#include "serializer.h"

#include <kvs/Camera>
#include <kvs/TransferFunction>

#include <cstring>

// リトルエンディアン環境では JKVS
// ビッグエンディアン環境では SVKJ
const int32_t jpv::ParticleTransferUtils::magicNumber = 0x53564b4a; // 1398164298;

bool jpv::ParticleTransferUtils::isLittleEndian( void )
{
    char buf[4];
    std::memcpy( buf, reinterpret_cast<const char*>( &jpv::ParticleTransferUtils::magicNumber ), sizeof( int32_t ) );
    if ( buf[0] == 'J' )
    {
        return true;
    }
    else
    {
        return false;
    }
}

jpv::ParticleTransferClientMessage::ParticleTransferClientMessage( void )
    : camera( NULL ), transfunc( NULL )
{
    std::fill( header, header + sizeof( header ), '\0' );
}

int32_t jpv::ParticleTransferClientMessage::byteSize( void ) const
{
    int32_t s = 0;
    s += sizeof( header );
    s += sizeof( messageSize );
    s += sizeof( initParam );
    if ( initParam == -3 )
    {
        s += sizeof( int64_t );
        s += sizeof( char ) * ( inputDir.size() + 1 );
    }
    if ( initParam == 1 )
    {
        s += sizeof( samplingMethod );
        s += sizeof( subPixelLevel );
        s += sizeof( repeatLevel );
        s += sizeof( shuffleMethod );
        s += sizeof( nodeType );
        s += sizeof( renderingId );
        s += sizeof( samplingStep );
        s += sizeof( enable_crop_region );
        s += sizeof( crop_region[0] ) * 6;
        s += sizeof( particle_limit );
        s += sizeof( particle_density );
        s += sizeof( particle_data_size_limit );
        s += sizeof( int64_t );
        s += sizeof( char ) * ( inputDir.size() + 1 );
        s += jpv::Serializer::byteSize<kvs::Camera>( *camera );
    }
    if ( initParam == 1 || initParam == -3 )
    {
        s += jpv::Serializer::byteSize( transfunc.size() );
        for ( size_t i = 0; i < transfunc.size(); i++ )
        {
            s += jpv::Serializer::byteSize( transfunc[i].Name );
            s += jpv::Serializer::byteSize( transfunc[i].ColorVar );
            s += jpv::Serializer::byteSize( transfunc[i].ColorVarMin );
            s += jpv::Serializer::byteSize( transfunc[i].ColorVarMax );
            s += jpv::Serializer::byteSize( transfunc[i].OpacityVar );
            s += jpv::Serializer::byteSize( transfunc[i].OpacityVarMin );
            s += jpv::Serializer::byteSize( transfunc[i].OpacityVarMax );
            s += jpv::Serializer::byteSize( transfunc[i].selection );
            if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
            {
                s += jpv::Serializer::byteSize( transfunc[i].Resolution );
                s += jpv::Serializer::byteSize( transfunc[i].EquationR );
                s += jpv::Serializer::byteSize( transfunc[i].EquationG );
                s += jpv::Serializer::byteSize( transfunc[i].EquationB );
                s += jpv::Serializer::byteSize( transfunc[i].EquationA );
            }
            else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
            {
                s += jpv::Serializer::byteSize<kvs::TransferFunction>( transfunc[i] );
            }
        }
        s += jpv::Serializer::byteSize( voleqn.size() );
        for ( size_t i = 0; i < voleqn.size(); i++ )
        {
            s += jpv::Serializer::byteSize( voleqn[i].Name );
            s += jpv::Serializer::byteSize( voleqn[i].Equation );
        }
        s += jpv::Serializer::byteSize( transferFunctionSynthesis );
    }
    if ( initParam >= 0 )
    {
        s += sizeof( timeParam );
        if ( timeParam == 0 )
        {
            s += sizeof( memorySize );
        }
        else if ( timeParam == 1 )
        {
            s += sizeof( beginTime );
            s += sizeof( endTime );
            s += sizeof( memorySize );
        }
        else if ( timeParam == 2 )
        {
            s += sizeof( step );
        }
        s += sizeof( transParam );
        if ( transParam == 1 )
        {
            s += sizeof( levelIndex );
        }
    }
    return s;
}

size_t jpv::ParticleTransferClientMessage::pack( char* buf ) const
{
    size_t index = 0;
    index += jpv::Serializer::writeArray( buf + index, header );
    index += jpv::Serializer::write( buf + index, messageSize );
    index += jpv::Serializer::write( buf + index, initParam );
    if ( initParam == -3 )
    {
        index += jpv::Serializer::write( buf + index, inputDir.size() );
        index += jpv::Serializer::writeArray( buf + index, inputDir.c_str(), inputDir.size() + 1 );
    }
    if ( initParam == 1 )
    {
        index += jpv::Serializer::write( buf + index, samplingMethod );
        index += jpv::Serializer::write( buf + index, subPixelLevel );
        index += jpv::Serializer::write( buf + index, repeatLevel );
        index += jpv::Serializer::write( buf + index, nodeType );
        index += jpv::Serializer::write( buf + index, renderingId );
        index += jpv::Serializer::write( buf + index, samplingStep );
        index += jpv::Serializer::write( buf + index, shuffleMethod );
        index += jpv::Serializer::write( buf + index, enable_crop_region );
        for ( int i = 0; i < 6; i++ )
        {
            index += jpv::Serializer::write( buf + index, crop_region[i] );
        }
        index += jpv::Serializer::write( buf + index, particle_limit );
        index += jpv::Serializer::write( buf + index, particle_density );
        index += jpv::Serializer::write( buf + index, particle_data_size_limit );
        index += jpv::Serializer::write( buf + index, inputDir.size() );
        index += jpv::Serializer::writeArray( buf + index, inputDir.c_str(), inputDir.size() + 1 );

        index += jpv::Serializer::pack( buf + index, *camera );
    }
    if ( initParam == 1 || initParam == -3 )
    {
        index += jpv::Serializer::write( buf + index, transfunc.size() );
        for ( size_t i = 0; i < transfunc.size(); i++ )
        {
            index += jpv::Serializer::write( buf + index, transfunc[i].Name );
            index += jpv::Serializer::write( buf + index, transfunc[i].ColorVar );
            index += jpv::Serializer::write( buf + index, transfunc[i].ColorVarMin );
            index += jpv::Serializer::write( buf + index, transfunc[i].ColorVarMax );
            index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVar );
            index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVarMin );
            index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVarMax );
            index += jpv::Serializer::write( buf + index, transfunc[i].selection );
            if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
            {
                index += jpv::Serializer::write( buf + index, transfunc[i].Resolution );
                index += jpv::Serializer::write( buf + index, transfunc[i].EquationR );
                index += jpv::Serializer::write( buf + index, transfunc[i].EquationG );
                index += jpv::Serializer::write( buf + index, transfunc[i].EquationB );
                index += jpv::Serializer::write( buf + index, transfunc[i].EquationA );
            }
            else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
            {
                index += jpv::Serializer::pack<kvs::TransferFunction>( buf + index, transfunc[i] );
            }
        }
        index += jpv::Serializer::write( buf + index, voleqn.size() );
        for ( size_t i = 0; i < voleqn.size(); i++ )
        {
            index += jpv::Serializer::write( buf + index, voleqn[i].Name );
            index += jpv::Serializer::write( buf + index, voleqn[i].Equation );
        }
        index += jpv::Serializer::write( buf + index, transferFunctionSynthesis );
    }
    if ( initParam >= 0 )
    {
        index += jpv::Serializer::write( buf + index, timeParam );
        if ( timeParam == 0 )
        {
            index += jpv::Serializer::write( buf + index, memorySize );
        }
        else if ( timeParam == 1 )
        {
            index += jpv::Serializer::write( buf + index, beginTime );
            index += jpv::Serializer::write( buf + index, endTime );
            index += jpv::Serializer::write( buf + index, memorySize );
        }
        else if ( timeParam == 2 )
        {
            index += jpv::Serializer::write( buf + index, step );
        }
        index += jpv::Serializer::write( buf + index, transParam );
        if ( transParam == 1 )
        {
            index += jpv::Serializer::write( buf + index, levelIndex );
        }
    }
    return index;
}

size_t jpv::ParticleTransferClientMessage::unpack( const char* buf )
{
    size_t index = 0;
    int64_t tmp_char_size;
    char*  tmp_char = NULL;
    int64_t tmp_size[6];
    char*  tmp_char_array[6];

    index += jpv::Serializer::readArray( buf + index, header );
    if ( strncmp( "JPTP /1.0", header, 9 ) != 0 )
    {
        return 0;
    }
    index += jpv::Serializer::read( buf + index, messageSize );
    index += jpv::Serializer::read( buf + index, initParam );
    if ( initParam == -3 )
    {
        index += jpv::Serializer::read( buf + index, tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        inputDir = std::string( tmp_char );
        delete[] tmp_char;
    }
    if ( initParam == 1 )
    {
        index += jpv::Serializer::read( buf + index, samplingMethod );
        index += jpv::Serializer::read( buf + index, subPixelLevel );
        index += jpv::Serializer::read( buf + index, repeatLevel );
        index += jpv::Serializer::read( buf + index, nodeType );
        index += jpv::Serializer::read( buf + index, renderingId );
        index += jpv::Serializer::read( buf + index, samplingStep );
        index += jpv::Serializer::read( buf + index, shuffleMethod );
        index += jpv::Serializer::read( buf + index, enable_crop_region );
        for ( int i = 0; i < 6; i++ )
        {
            index += jpv::Serializer::read( buf + index, crop_region[i] );
        }
        index += jpv::Serializer::read( buf + index, particle_limit );
        index += jpv::Serializer::read( buf + index, particle_density );
        index += jpv::Serializer::read( buf + index, particle_data_size_limit );
        index += jpv::Serializer::read( buf + index, tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        inputDir = std::string( tmp_char );
        delete[] tmp_char;

        index += jpv::Serializer::unpack( buf + index, *camera );
    }
    if ( initParam == 1 || initParam == -3 )
    {
        size_t s;
        index += jpv::Serializer::read( buf + index, s );
        transfunc.clear();
        for ( size_t i = 0; i < s; i++ )
        {
            transfunc.push_back( NamedTransferFunctionParameter() );
            index += jpv::Serializer::read( buf + index, transfunc[i].Name );
            index += jpv::Serializer::read( buf + index, transfunc[i].ColorVar );
            index += jpv::Serializer::read( buf + index, transfunc[i].ColorVarMin );
            index += jpv::Serializer::read( buf + index, transfunc[i].ColorVarMax );
            index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVar );
            index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVarMin );
            index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVarMax );
            index += jpv::Serializer::read( buf + index, transfunc[i].selection );
            if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
            {
                index += jpv::Serializer::read( buf + index, transfunc[i].Resolution );
                index += jpv::Serializer::read( buf + index, transfunc[i].EquationR );
                index += jpv::Serializer::read( buf + index, transfunc[i].EquationG );
                index += jpv::Serializer::read( buf + index, transfunc[i].EquationB );
                index += jpv::Serializer::read( buf + index, transfunc[i].EquationA );
            }
            else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
            {
                index += jpv::Serializer::unpack<kvs::TransferFunction>( buf + index, transfunc[i] );
            }
        }
        index += jpv::Serializer::read( buf + index, s );
        voleqn.clear();
        for ( size_t i = 0; i < s; i++ )
        {
            voleqn.push_back( VolumeEquation() );
            index += jpv::Serializer::read( buf + index, voleqn[i].Name );
            index += jpv::Serializer::read( buf + index, voleqn[i].Equation );
        }
        index += jpv::Serializer::read( buf + index, transferFunctionSynthesis );
    }
    if ( initParam >= 0 )
    {
        index += jpv::Serializer::read( buf + index, timeParam );
        if ( timeParam == 0 )
        {
            index += jpv::Serializer::read( buf + index, memorySize );
        }
        else if ( timeParam == 1 )
        {
            index += jpv::Serializer::read( buf + index, beginTime );
            index += jpv::Serializer::read( buf + index, endTime );
            index += jpv::Serializer::read( buf + index, memorySize );
        }
        else if ( timeParam == 2 )
        {
            index += jpv::Serializer::read( buf + index, step );
        }
        index += jpv::Serializer::read( buf + index, transParam );
        if ( transParam == 1 )
        {
            index += jpv::Serializer::read( buf + index, levelIndex );
        }
    }

    return index;
}

jpv::ParticleTransferServerMessage::ParticleTransferServerMessage( void ):
    camera( NULL )
{
    std::fill( header, header + sizeof( header ), '\0' );
}

int32_t jpv::ParticleTransferServerMessage::byteSize( void ) const
{
    int32_t s = 0;
    s += sizeof( header );
    s += sizeof( messageSize );
    s += sizeof( timeStep );
    s += sizeof( subPixelLevel );
    s += sizeof( repeatLevel );
    s += sizeof( levelIndex );
    s += sizeof( numParticle );
    s += sizeof( numVolDiv );
    s += sizeof( staStep );
    s += sizeof( endStep );
    s += sizeof( numStep );
    s += sizeof( minObjectCoord[0] ) * 3;
    s += sizeof( maxObjectCoord[0] ) * 3;
    s += sizeof( minValue );
    s += sizeof( maxValue );
    s += sizeof( numNodes );
    s += sizeof( numElements );
    s += sizeof( elemType );
    s += sizeof( fileType );
    s += sizeof( numIngredients );
    s += varRange.byteSize();
    s += sizeof( flag_send_bins );
    s += sizeof( particle_limit );
    s += sizeof( particle_density );
    s += sizeof( particle_data_size_limit );
    s += jpv::Serializer::byteSize<kvs::Camera>( *camera );
    s += sizeof( tf_count );
    for ( int i = 0; i < tf_count; i++ )
    {
        s += sizeof( c_nbins[i] );
        s += sizeof( c_bins[i] ) * c_nbins[i];
        s += sizeof( o_nbins[i] );
        s += sizeof( o_bins[i] ) * o_nbins[i];
    }

    s += jpv::Serializer::byteSize( transfunc.size() );
    for ( size_t i = 0; i < transfunc.size(); i++ )
    {
        s += jpv::Serializer::byteSize( transfunc[i].Name );
        s += jpv::Serializer::byteSize( transfunc[i].ColorVar );
        s += jpv::Serializer::byteSize( transfunc[i].ColorVarMin );
        s += jpv::Serializer::byteSize( transfunc[i].ColorVarMax );
        s += jpv::Serializer::byteSize( transfunc[i].OpacityVar );
        s += jpv::Serializer::byteSize( transfunc[i].OpacityVarMin );
        s += jpv::Serializer::byteSize( transfunc[i].OpacityVarMax );
        s += jpv::Serializer::byteSize( transfunc[i].selection );
        if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            s += jpv::Serializer::byteSize( transfunc[i].Resolution );
            s += jpv::Serializer::byteSize( transfunc[i].EquationR );
            s += jpv::Serializer::byteSize( transfunc[i].EquationG );
            s += jpv::Serializer::byteSize( transfunc[i].EquationB );
            s += jpv::Serializer::byteSize( transfunc[i].EquationA );
        }
        else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            s += jpv::Serializer::byteSize<kvs::TransferFunction>( transfunc[i] );
        }
    }
    s += jpv::Serializer::byteSize( voleqn.size() );
    for ( size_t i = 0; i < voleqn.size(); i++ )
    {
        s += jpv::Serializer::byteSize( voleqn[i].Name );
        s += jpv::Serializer::byteSize( voleqn[i].Equation );
    }
    s += jpv::Serializer::byteSize( transferFunctionSynthesis );

    return s;
}

size_t jpv::ParticleTransferServerMessage::pack( char* buf ) const
{
    size_t index = 0;
    index += jpv::Serializer::writeArray( buf + index, header );
    index += jpv::Serializer::write( buf + index, messageSize );
    index += jpv::Serializer::write( buf + index, timeStep );
    index += jpv::Serializer::write( buf + index, subPixelLevel );
    index += jpv::Serializer::write( buf + index, repeatLevel );
    index += jpv::Serializer::write( buf + index, levelIndex );
    index += jpv::Serializer::write( buf + index, numParticle );
    index += jpv::Serializer::write( buf + index, numVolDiv );
    index += jpv::Serializer::write( buf + index, staStep );
    index += jpv::Serializer::write( buf + index, endStep );
    index += jpv::Serializer::write( buf + index, numStep );
    index += jpv::Serializer::write( buf + index, minObjectCoord[0] );
    index += jpv::Serializer::write( buf + index, minObjectCoord[1] );
    index += jpv::Serializer::write( buf + index, minObjectCoord[2] );
    index += jpv::Serializer::write( buf + index, maxObjectCoord[0] );
    index += jpv::Serializer::write( buf + index, maxObjectCoord[1] );
    index += jpv::Serializer::write( buf + index, maxObjectCoord[2] );
    index += jpv::Serializer::write( buf + index, minValue );
    index += jpv::Serializer::write( buf + index, maxValue );
    index += jpv::Serializer::write( buf + index, numNodes );
    index += jpv::Serializer::write( buf + index, numElements );
    index += jpv::Serializer::write( buf + index, elemType );
    index += jpv::Serializer::write( buf + index, fileType );
    index += jpv::Serializer::write( buf + index, numIngredients );
    index += varRange.pack( buf + index );
    index += jpv::Serializer::write( buf + index, flag_send_bins );
    index += jpv::Serializer::write( buf + index, particle_limit );
    index += jpv::Serializer::write( buf + index, particle_density );
    index += jpv::Serializer::write( buf + index, particle_data_size_limit );
    index += jpv::Serializer::pack( buf + index, *camera );
    index += jpv::Serializer::write( buf + index, tf_count );

    for ( int i = 0; i < tf_count; i++ )
    {
        index += jpv::Serializer::write( buf + index, c_nbins[i] );
        index += jpv::Serializer::writeArray<kvs::UInt64>( buf + index, c_bins[i], c_nbins[i] );
        index += jpv::Serializer::write( buf + index, o_nbins[i] );
        index += jpv::Serializer::writeArray<kvs::UInt64>( buf + index, o_bins[i], o_nbins[i] );
    }

    index += jpv::Serializer::write( buf + index, transfunc.size() );
    for ( size_t i = 0; i < transfunc.size(); i++ )
    {
        index += jpv::Serializer::write( buf + index, transfunc[i].Name );
        index += jpv::Serializer::write( buf + index, transfunc[i].ColorVar );
        index += jpv::Serializer::write( buf + index, transfunc[i].ColorVarMin );
        index += jpv::Serializer::write( buf + index, transfunc[i].ColorVarMax );
        index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVar );
        index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVarMin );
        index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVarMax );
        index += jpv::Serializer::write( buf + index, transfunc[i].selection );
        if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            index += jpv::Serializer::write( buf + index, transfunc[i].Resolution );
            index += jpv::Serializer::write( buf + index, transfunc[i].EquationR );
            index += jpv::Serializer::write( buf + index, transfunc[i].EquationG );
            index += jpv::Serializer::write( buf + index, transfunc[i].EquationB );
            index += jpv::Serializer::write( buf + index, transfunc[i].EquationA );
        }
        else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            index += jpv::Serializer::pack<kvs::TransferFunction>( buf + index, transfunc[i] );
        }
    }
    index += jpv::Serializer::write( buf + index, voleqn.size() );
    for ( size_t i = 0; i < voleqn.size(); i++ )
    {
        index += jpv::Serializer::write( buf + index, voleqn[i].Name );
        index += jpv::Serializer::write( buf + index, voleqn[i].Equation );
    }
    index += jpv::Serializer::write( buf + index, transferFunctionSynthesis );

    if ( flag_send_bins == 1 )
    {
        index += jpv::Serializer::writeArray<float>( buf + index, positions, 3 * numParticle );
        index += jpv::Serializer::writeArray<float>( buf + index, normals, 3 * numParticle );
        index += jpv::Serializer::writeArray<unsigned char>( buf + index, colors, 3 * numParticle );
    }
    return index;
}

size_t jpv::ParticleTransferServerMessage::unpack_message( const char* buf )
{
    size_t index = 0;
    index += jpv::Serializer::readArray( buf + index, header );
    if ( strncmp( "JPTP /1.0", header, 9 ) != 0 )
    {
        return 0;
    }
    index += jpv::Serializer::read( buf + index, messageSize );
    index += jpv::Serializer::read( buf + index, timeStep );
    index += jpv::Serializer::read( buf + index, subPixelLevel );
    index += jpv::Serializer::read( buf + index, repeatLevel );
    index += jpv::Serializer::read( buf + index, levelIndex );
    index += jpv::Serializer::read( buf + index, numParticle );
    index += jpv::Serializer::read( buf + index, numVolDiv );
    index += jpv::Serializer::read( buf + index, staStep );
    index += jpv::Serializer::read( buf + index, endStep );
    index += jpv::Serializer::read( buf + index, numStep );
    index += jpv::Serializer::read( buf + index, minObjectCoord[0] );
    index += jpv::Serializer::read( buf + index, minObjectCoord[1] );
    index += jpv::Serializer::read( buf + index, minObjectCoord[2] );
    index += jpv::Serializer::read( buf + index, maxObjectCoord[0] );
    index += jpv::Serializer::read( buf + index, maxObjectCoord[1] );
    index += jpv::Serializer::read( buf + index, maxObjectCoord[2] );
    index += jpv::Serializer::read( buf + index, minValue );
    index += jpv::Serializer::read( buf + index, maxValue );
    index += jpv::Serializer::read( buf + index, numNodes );
    index += jpv::Serializer::read( buf + index, numElements );
    index += jpv::Serializer::read( buf + index, elemType );
    index += jpv::Serializer::read( buf + index, fileType );
    index += jpv::Serializer::read( buf + index, numIngredients );
    index += varRange.unpack( buf + index );
    index += jpv::Serializer::read( buf + index, flag_send_bins );
    index += jpv::Serializer::read( buf + index, particle_limit );
    index += jpv::Serializer::read( buf + index, particle_density );
    index += jpv::Serializer::read( buf + index, particle_data_size_limit );
    index += jpv::Serializer::unpack( buf + index, *camera );
    index += jpv::Serializer::read( buf + index, tf_count );
    c_nbins = new kvs::UInt64[ tf_count ];
    o_nbins = new kvs::UInt64[ tf_count ];

    c_bins.resize( tf_count );
    o_bins.resize( tf_count );
    for ( int i = 0; i < tf_count; i++ )
    {
        index += jpv::Serializer::read( buf + index, c_nbins[i] );
        c_bins[i] =  new kvs::UInt64[ c_nbins[i] ];
        index += jpv::Serializer::readArray<kvs::UInt64>( buf + index, c_bins[i], c_nbins[i] );
        index += jpv::Serializer::read( buf + index, o_nbins[i] );
        o_bins[i] =  new kvs::UInt64[ o_nbins[i] ];
        index += jpv::Serializer::readArray<kvs::UInt64>( buf + index, o_bins[i], o_nbins[i] );
    }

    size_t s;
    index += jpv::Serializer::read( buf + index, s );
    transfunc.clear();
    for ( size_t i = 0; i < s; i++ )
    {
        transfunc.push_back( NamedTransferFunctionParameter() );
        index += jpv::Serializer::read( buf + index, transfunc[i].Name );
        index += jpv::Serializer::read( buf + index, transfunc[i].ColorVar );
        index += jpv::Serializer::read( buf + index, transfunc[i].ColorVarMin );
        index += jpv::Serializer::read( buf + index, transfunc[i].ColorVarMax );
        index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVar );
        index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVarMin );
        index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVarMax );
        index += jpv::Serializer::read( buf + index, transfunc[i].selection );
        if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            index += jpv::Serializer::read( buf + index, transfunc[i].Resolution );
            index += jpv::Serializer::read( buf + index, transfunc[i].EquationR );
            index += jpv::Serializer::read( buf + index, transfunc[i].EquationG );
            index += jpv::Serializer::read( buf + index, transfunc[i].EquationB );
            index += jpv::Serializer::read( buf + index, transfunc[i].EquationA );
        }
        else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            index += jpv::Serializer::unpack<kvs::TransferFunction>( buf + index, transfunc[i] );
        }
    }
    index += jpv::Serializer::read( buf + index, s );
    voleqn.clear();
    for ( size_t i = 0; i < s; i++ )
    {
        voleqn.push_back( VolumeEquation() );
        index += jpv::Serializer::read( buf + index, voleqn[i].Name );
        index += jpv::Serializer::read( buf + index, voleqn[i].Equation );
    }
    index += jpv::Serializer::read( buf + index, transferFunctionSynthesis );

    return index;
}

size_t jpv::ParticleTransferServerMessage::unpack_particles( const char* buf )
{
    size_t index = messageSize;
    positions = new float[3 * numParticle];
    normals = new float[3 * numParticle];
    colors = new unsigned char[3 * numParticle];
    index += jpv::Serializer::readArray<float>( buf + index, positions, 3 * numParticle );
    index += jpv::Serializer::readArray<float>( buf + index, normals, 3 * numParticle );
    index += jpv::Serializer::readArray<unsigned char>( buf + index, colors, 3 * numParticle );
    return index;
}

