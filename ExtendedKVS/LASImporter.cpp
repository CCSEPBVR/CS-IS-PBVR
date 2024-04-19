#include "LASImporter.h"

LASImporter::LASImporter( std::string filename )
{
    std::cout << "LAS format blocks are " << std::endl;
    std::cout << "Public Header Block (PHB), Variable Length Records (VLRs)," << std::endl;
    std::cout << "Point Data Records (PDRs), and Extended VLRs." << std::endl;

    // LASヘッダー情報を読み込む
    LASPublicHeaderBlock header;
    std::ifstream file(filename, std::ios::binary);
    if ( !file.is_open() )
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
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
        std::cerr << "Error: Unsupported LAS format version" << std::endl;
        file.close();
        return;
    }

    std::cout << "version: " << (int)header.version_major << "." << (int)header.version_minor << std::endl;
    std::cout << "PHB_size: " << header.PHB_size << std::endl;
    std::cout << "offset_to_PDR: " << header.offset_to_PDR << std::endl;
    std::cout << "number_of_VLRs: " << header.number_of_VLRs << std::endl;
    std::cout << "PDR_format: " << (int)header.PDR_format << std::endl;
    std::cout << "PDR_length: " << header.PDR_length << std::endl;
    std::cout << "legacy_number_of_PDRs: " << header.legacy_number_of_PDRs << std::endl;
    std::cout << "x_scale_factor: " << header.x_scale_factor << std::endl;
    std::cout << "y_scale_factor: " << header.y_scale_factor << std::endl;
    std::cout << "z_scale_factor: " << header.z_scale_factor << std::endl;
    std::cout << "x_offset: " << header.x_offset << std::endl;
    std::cout << "y_offset: " << header.y_offset << std::endl;
    std::cout << "z_offset: " << header.z_offset << std::endl;
    std::cout << "number_of_PDRs: " << header.number_of_PDRs << std::endl;

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
        std::cerr << "Error: Unsupported Point Data Record format version " << (int)header.PDR_format
                  << ". This version has no RGB values." << std::endl;
        file.close();
        return;
    }

    char* PDR_buffer = new char[header.PDR_length];

    // 読み取り位置をファイル先頭からポイントブロックまでシーク
    file.seekg( header.offset_to_PDR, std::ios_base::beg );

    // LASポイント情報を読み込む
    LASPointDataRecords point;

    kvs::ValueArray<kvs::Real32> coordinates( 3*header.number_of_PDRs );
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

        coordinates[3*i  ] = point.X * header.x_scale_factor + header.x_offset;
        coordinates[3*i+1] = point.Y * header.y_scale_factor + header.y_offset;
        coordinates[3*i+2] = point.Z * header.z_scale_factor + header.z_offset;

        colors[3*i  ] = this->int2byte( point.R );
        colors[3*i+1] = this->int2byte( point.G );
        colors[3*i+2] = this->int2byte( point.B );
    }

    this->setCoords ( coordinates );
    this->setColors ( colors );
    this->updateMinMaxCoords();

    delete [] PDR_buffer;
    file.close();
}

kvs::UInt8 LASImporter::int2byte( uint16_t value )
{
    return value >> 8;
}
