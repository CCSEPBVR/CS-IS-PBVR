#include "LAS.h"
#include <cstring>
#include <kvs/IgnoreUnusedVariable>
#include <kvs/File>
#include <kvs/Assert>
/****************************************************************************/
/**
 *  @file   LAS.cpp
 *  @author
 */
/****************************************************************************/

namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Checks the file extension.
 *  @param  filename [in] filename
 *  @return true, if the given filename has the supported extension
 */
/*===========================================================================*/
bool LAS::CheckExtension( const std::string& filename )
{
    const kvs::File file( filename );
    if ( file.extension() == "las" || file.extension() == "LAS" )
    {
        return true;
    }

    return false;
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new LAS class.
 */
/*===========================================================================*/
LAS::LAS()
{

}

/*===========================================================================*/
/**
 *  @brief  Constructs a new LAS class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
LAS::LAS( const std::string& filename )
{
    this->read( filename );
}


/*===========================================================================*/
/**
 *  @brief  Read a LAS point object file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is successfully
 */
/*===========================================================================*/
bool LAS::read( const std::string& filename )
{
    BaseClass::setFilename( filename );
    BaseClass::setSuccess( true );

    //LAS format blocks are
    //Public Header Block (PHB), Variable Length Records (VLRs)
    //Point Data Records (PDRs), and Extended VLRs

    // LASヘッダー情報を読み込む
    LASPublicHeaderBlock header;
    std::ifstream file(filename, std::ios::binary);
    if ( !file.is_open() )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        return false;
    }
    file.seekg( 4+2+2+4+2+2+8, std::ios_base::beg ); // バージョン番号までシーク
    file.read( reinterpret_cast<char*>(&header.version_major), sizeof(kvs::UInt8) );
    file.read( reinterpret_cast<char*>(&header.version_minor), sizeof(kvs::UInt8) );
    file.seekg( 32+32+2+2, std::ios_base::cur ); // ヘッダーサイズまでシーク
    file.read( reinterpret_cast<char*>(&header.PHB_size), sizeof(kvs::UInt16) );
    file.read( reinterpret_cast<char*>(&header.offset_to_PDR), sizeof(kvs::UInt32) );
    file.read( reinterpret_cast<char*>(&header.number_of_VLRs), sizeof(kvs::UInt32) );
    file.read( reinterpret_cast<char*>(&header.PDR_format), sizeof(kvs::UInt8) );
    file.read( reinterpret_cast<char*>(&header.PDR_length), sizeof(kvs::UInt16) );
    file.read( reinterpret_cast<char*>(&header.legacy_number_of_PDRs), sizeof(kvs::UInt32) );
    file.seekg( 20, std::ios_base::cur ); // X Scale Factorまでシーク
    file.read( reinterpret_cast<char*>(&header.x_scale_factor), sizeof(kvs::Real64) );
    file.read( reinterpret_cast<char*>(&header.y_scale_factor), sizeof(kvs::Real64) );
    file.read( reinterpret_cast<char*>(&header.z_scale_factor), sizeof(kvs::Real64) );
    file.read( reinterpret_cast<char*>(&header.x_offset), sizeof(kvs::Real64) );
    file.read( reinterpret_cast<char*>(&header.y_offset), sizeof(kvs::Real64) );
    file.read( reinterpret_cast<char*>(&header.z_offset), sizeof(kvs::Real64) );

    if( (int)header.version_minor <= 3 )
    {
        header.number_of_PDRs = header.legacy_number_of_PDRs;
    }
    else if( (int)header.version_minor == 4 )
    {
        file.seekg( 8+8+8+ 8+8+8+ 8+8+4, std::ios_base::cur ); // Number of PDRsまでシーク
        file.read( reinterpret_cast<char*>(&header.number_of_PDRs), sizeof(kvs::UInt64) );
    }
    else
    {
        kvsMessageError( "Error: Unsupported LAS format version" );
        file.close();
        return false;
    }

    int offset; // Point Data RecordのX,Y,ZからRGBまでの間のバイト数を計算
    switch( (int)header.PDR_format )
    {
    case 2:
        offset = 12+2+1+1+1+1+2;
        break;
    case 3:
        offset = 12+2+1+1+1+1+2+8;
        break;
    case 5:
        offset = 12+2+1+1+1+1+2+8;
        break;
    case 7:
        offset = 12+2+1+1+1+1+2+2+8;
        break;
    case 8:
        offset = 12+2+1+1+1+1+2+2+8;
        break;
    case 10:
        offset = 12+2+1+1+1+1+2+2+8;
        break;
    default:
        kvsMessageError( "Error: Unsupported Point Data Record format version '%d' This version has no RGB values.", (int)header.PDR_format );
        file.close();
        return false;
    }

    char* PDR_buffer = new char[header.PDR_length];

    // 読み取り位置をファイル先頭からポイントブロックまでシーク
    file.seekg( header.offset_to_PDR, std::ios_base::beg );

    // LASポイント情報を読み込む
    LASPointDataRecords point;

    kvs::ValueArray<kvs::Real32> coords( 3*header.number_of_PDRs );
    kvs::ValueArray<kvs::UInt8>  colors     ( 3*header.number_of_PDRs );

    for( kvs::UInt64 i=0; i<header.number_of_PDRs; i++ )
    {
        file.read( PDR_buffer, header.PDR_length );

        std::memcpy( &point.X, PDR_buffer    , 4 );
        std::memcpy( &point.Y, PDR_buffer + 4, 4 );
        std::memcpy( &point.Z, PDR_buffer + 8, 4 );
        std::memcpy( &point.R, PDR_buffer + offset    , 2 );
        std::memcpy( &point.G, PDR_buffer + offset + 2, 2 );
        std::memcpy( &point.B, PDR_buffer + offset + 4, 2 );

        coords[3*i  ] = point.X * header.x_scale_factor + header.x_offset;
        coords[3*i+1] = point.Y * header.y_scale_factor + header.y_offset;
        coords[3*i+2] = point.Z * header.z_scale_factor + header.z_offset;

        colors[3*i  ] = this->int2byte( point.R );
        colors[3*i+1] = this->int2byte( point.G );
        colors[3*i+2] = this->int2byte( point.B );
    }

    m_coords = kvs::ValueArray<kvs::Real32>( coords );
    m_colors = kvs::ValueArray<UInt8>( colors );

    delete [] PDR_buffer;
    file.close();
    return true;
}

/*===========================================================================*/
/**
 *  @brief  Writes LAS data.
 *  @param  filename [in] filename
 *  @return false (not yet implemented)
 */
/*===========================================================================*/
bool LAS::write( const std::string& filename )
{
    kvs::IgnoreUnusedVariable( filename );
    return false;
}

kvs::UInt8 LAS::int2byte( uint16_t value )
{
    return value >> 8;
}

} // end of namespace kvs
