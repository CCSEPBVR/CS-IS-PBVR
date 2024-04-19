#ifndef LASIMPORTER_H
#define LASIMPORTER_H
#include <kvs/PointObject>
#include <fstream>
//#include <liblas/liblas.hpp>

class LASImporter : public kvs::PointObject
{
public:
    // LASヘッダー構造体
    struct LASPublicHeaderBlock
    {
        kvs::UInt8 version_major;
        kvs::UInt8 version_minor;
        kvs::UInt16 PHB_size; // Public Header Blockのサイズ
        kvs::UInt32 offset_to_PDR; // ファイルの先頭から最初のPDRの最初のフィールドまでの実際のバイト数
        kvs::UInt32 number_of_VLRs; // VLRsの数
        kvs::UInt8 PDR_format; // PDRのフォーマット。LAS 1.4では、タイプ0から10までが定義されている。
        kvs::UInt16 PDR_length; // PDR(1粒子)のサイズ（バイト単位）
        kvs::UInt32 legacy_number_of_PDRs; //PDRフォーマットが6未満の場合のPDRの数(粒子数)
        kvs::Real64 x_scale_factor; // X = x_scale_cactor * x + x_offset
        kvs::Real64 y_scale_factor;
        kvs::Real64 z_scale_factor;
        kvs::Real64 x_offset;
        kvs::Real64 y_offset;
        kvs::Real64 z_offset;
        kvs::UInt64 number_of_PDRs; // PDRの数(粒子数)
    };

    // LASポイント構造体
    struct LASPointDataRecords
    {
        kvs::Int32 X, Y, Z;
        kvs::UInt16 R, G, B;
    };

    LASImporter( std::string filename );
    kvs::UInt8 int2byte( const kvs::UInt16 value );
};

#endif // LASIMPORTER_H
