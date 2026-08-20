/*
 * Created by Japan Atomic Energy Agency
 *
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legal code along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Exporter/StructuredVolumeObjectExporter.h"
#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "FileFormat/NetCDF/Netcdf.h"
#include "FileFormat/VTK/VtkXmlImageData.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "Filesystem.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/Pfl.h"
#include "PBVRFileInformation/UnstructuredPfi.h"
#include "kvs/StructuredVolumeObject"
#include "kvs/UnstructuredVolumeObject"
#include "kvs/VolumeObjectBase"

/**
 * @brief 時系列変換の事前検証で取得したNetCDFファイルの情報。
 */
struct SequencedNetcdfFile
{
    std::string path; ///< 入力ファイルのパス。
    std::string format_name; ///< NetCDFアダプターが判定した形式名。
    cvt::NetcdfGridType grid_type = cvt::NetcdfGridType::Unknown; ///< VTK格子形式。
};

/**
 * @brief NetCDFアダプターの出力をKVSボリュームオブジェクトへ変換する。
 *
 * NetCDFアダプターが内部で生成したVTKデータの実際の型に対応する
 * インポーターを選択し、構造格子または非構造格子のボリュームを生成する。
 *
 * @param input 読み込み済みのNetCDFデータ。
 * @return 変換後のKVSボリューム。未対応形式または変換失敗時はnullptr。
 */
std::unique_ptr<kvs::VolumeObjectBase> ImportNetcdfVolume( cvt::Netcdf& input )
{
    // アダプターが生成したVTK格子形式ごとに対応するインポーターを選択する。
    if ( auto* format = dynamic_cast<cvt::VtkXmlImageData*>( input.format().get() ) )
    {
        auto importer = std::make_unique<cvt::VtkImporter<cvt::VtkXmlImageData>>( format );
        if ( importer->isFailure() )
        {
            return nullptr;
        }
        return importer;
    }
    if ( auto* format = dynamic_cast<cvt::VtkXmlRectilinearGrid*>( input.format().get() ) )
    {
        auto importer =
            std::make_unique<cvt::VtkImporter<cvt::VtkXmlRectilinearGrid>>( format );
        if ( importer->isFailure() )
        {
            return nullptr;
        }
        return importer;
    }
    if ( auto* format = dynamic_cast<cvt::VtkXmlStructuredGrid*>( input.format().get() ) )
    {
        auto importer =
            std::make_unique<cvt::VtkImporter<cvt::VtkXmlStructuredGrid>>( format );
        if ( importer->isFailure() )
        {
            return nullptr;
        }
        return importer;
    }
    if ( auto* format = dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( input.format().get() ) )
    {
        auto importer =
            std::make_unique<cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid>>( format );
        if ( importer->isFailure() )
        {
            return nullptr;
        }
        return importer;
    }

    std::cerr << "Unsupported VTK grid type returned by the " << input.formatName()
              << " NetCDF adapter." << std::endl;
    return nullptr;
}

/**
 * @brief 1タイムステップ分のボリュームをKVSMLへ出力し、PFIへ登録する。
 *
 * ボリュームの種類に応じたエクスポーターを選び、配列データを外部バイナリ
 * 形式で書き出す。構造格子の場合は、出力前に座標範囲も更新する。
 *
 * @param directory 出力先ディレクトリ。
 * @param local_base KVSMLおよび外部データに使用するベース名。
 * @param source エラー表示に使用する入力ファイル名。
 * @param time_step 出力対象のタイムステップ番号。
 * @param sub_volume_id サブボリューム番号。
 * @param sub_volume_count サブボリューム総数。
 * @param volume 出力対象のKVSボリューム。
 * @param pfi 出力情報の登録先となるPFIオブジェクト。
 * @return KVSMLの出力とPFIへの登録に成功した場合はtrue。
 */
bool WriteNetcdfVolume( const std::string& directory, const std::string& local_base,
                        const std::string& source, int time_step, int sub_volume_id,
                        int sub_volume_count, kvs::VolumeObjectBase* volume,
                        cvt::UnstructuredPfi& pfi )
{
    volume->print( std::cout, kvs::Indent( 4 ) );
    std::cout << "  Writing to " << directory << " ..." << std::endl;

    // 非構造格子はセルタイプを維持したままKVSMLへ出力する。
    if ( auto* unstructured = dynamic_cast<kvs::UnstructuredVolumeObject*>( volume ) )
    {
        std::cout << "  cell type: " << unstructured->cellType() << std::endl;
        cvt::UnstructuredVolumeObjectExporter exporter( unstructured );
        exporter.setWritingDataTypeToExternalBinary();
        if ( !exporter.write( directory, local_base, time_step, sub_volume_id,
                              sub_volume_count, false ) )
        {
            std::cerr << "Failed to write NetCDF time step " << time_step
                      << ": " << source << std::endl;
            return false;
        }
        pfi.registerObject( &exporter, time_step, sub_volume_id );
        return true;
    }

    // 構造格子はオブジェクト座標を外部座標として設定してから出力する。
    if ( auto* structured = dynamic_cast<kvs::StructuredVolumeObject*>( volume ) )
    {
        structured->updateMinMaxCoords();
        structured->setMinMaxExternalCoords( structured->minObjectCoord(),
                                             structured->maxObjectCoord() );
        cvt::StructuredVolumeObjectExporter exporter( structured );
        exporter.setWritingDataTypeToExternalBinary();
        if ( !exporter.write( directory, local_base, time_step, sub_volume_id,
                              sub_volume_count, false ) )
        {
            std::cerr << "Failed to write NetCDF time step " << time_step
                      << ": " << source << std::endl;
            return false;
        }
        pfi.registerObject( &exporter, time_step, sub_volume_id );
        return true;
    }

    std::cerr << "The NetCDF adapter did not produce a KVS volume object: "
              << source << std::endl;
    return false;
}

/**
 * @brief 単一のNetCDFファイルをKVSML、PFI、PFLへ変換する。
 *
 * @param directory 出力先ディレクトリ。
 * @param base 出力ファイルに使用するベース名。
 * @param src 入力NetCDFファイルのパス。
 */
void Netcdf2Kvsml( const std::string& directory, const std::string& base,
                   const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::Netcdf input( src );
    if ( input.isFailure() )
    {
        std::cerr << "Failed to read the NetCDF file: " << src << std::endl;
        return;
    }

    auto volume = ImportNetcdfVolume( input );
    if ( !volume )
    {
        std::cerr << "Failed to import the " << input.formatName()
                  << " NetCDF file: " << src << std::endl;
        return;
    }

    // 単一ファイルをタイムステップ0、サブボリューム1個として扱う。
    constexpr int time_step = 0;
    constexpr int last_time_step = 0;
    constexpr int sub_volume_id = 1;
    constexpr int sub_volume_count = 1;
    std::string local_base = base;

    // 非構造格子ではセルタイプごとに出力名が重複しないよう識別子を付ける。
    if ( auto* unstructured =
             dynamic_cast<kvs::UnstructuredVolumeObject*>( volume.get() ) )
    {
        local_base += "_" + std::to_string( unstructured->cellType() );
    }

    // PFIにKVSMLの構成情報を記録する。
    cvt::UnstructuredPfi pfi( volume->veclen(), last_time_step, sub_volume_count );
    if ( !WriteNetcdfVolume( directory, local_base, src, time_step, sub_volume_id,
                             sub_volume_count, volume.get(), pfi ) )
    {
        return;
    }
    if ( !pfi.write( directory, local_base ) )
    {
        std::cerr << "Failed to write the NetCDF PFI file." << std::endl;
        return;
    }

    // PFLに作成したPFIへの参照を記録する。
    cvt::Pfl pfl;
    pfl.registerPfi( directory, local_base );
    if ( !pfl.write( directory, base ) )
    {
        std::cerr << "Failed to write the NetCDF PFL file." << std::endl;
    }
}

/**
 * @brief ワイルドカードに一致するNetCDF時系列ファイルを列挙して検証する。
 *
 * ファイルを数値順に列挙し、全ファイルのNetCDF形式とVTK格子形式が
 * 先頭ファイルと一致することを、実データを変換する前に確認する。
 *
 * @param file_paths 解決済みで数値順に整列された入力ファイル一覧。
 * @param sequenced_files 検証済みファイル情報の格納先。
 * @return 列挙と検証に成功した場合はtrue。
 */
bool ListNetcdfTimeSeriesFiles( const std::vector<std::string>& file_paths,
                                std::vector<SequencedNetcdfFile>& sequenced_files )
{
    if ( file_paths.empty() )
    {
        std::cerr << "The resolved NetCDF time series is empty." << std::endl;
        return false;
    }

    sequenced_files.reserve( file_paths.size() );
    std::string expected_format;
    cvt::NetcdfGridType expected_grid_type = cvt::NetcdfGridType::Unknown;
    for ( const auto& path : file_paths )
    {
        // 軽量な事前調査で形式を判定し、時系列全体の形式を統一する。
        cvt::NetcdfFileInfo info;
        if ( !cvt::Netcdf::Probe( path, info ) )
        {
            return false;
        }
        if ( sequenced_files.empty() )
        {
            expected_format = info.format_name;
            expected_grid_type = info.grid_type;
        }
        else if ( info.format_name != expected_format ||
                  info.grid_type != expected_grid_type )
        {
            std::cerr << "NetCDF time series mixes formats or VTK grid types: expected "
                      << expected_format << ", but " << path << " was detected as "
                      << info.format_name << std::endl;
            return false;
        }

        sequenced_files.push_back( { path, info.format_name, info.grid_type } );
    }
    return true;
}

/**
 * @brief NetCDF時系列ファイルを一連のKVSML、PFI、PFLへ変換する。
 *
 * 各ファイルを数値順のタイムステップとして読み込み、先頭ステップで決定した
 * ボリューム形式、セル形式、格子形式および成分数が後続ステップでも一致する
 * ことを確認しながら出力する。
 *
 * @param directory 出力先ディレクトリ。
 * @param base 時系列全体で共通する出力ベース名。
 * @param file_paths 解決済みで数値順に整列された入力ファイル一覧。
 */
void SeriesNetcdf2Kvsml( const std::string& directory, const std::string& base,
                         const std::vector<std::string>& file_paths )
{
    if ( base.empty() )
    {
        std::cerr << "Could not derive an output prefix from the NetCDF wildcard."
                  << std::endl;
        return;
    }

    // 全ファイルを先に列挙し、形式が混在していないことを確認する。
    std::vector<SequencedNetcdfFile> sequenced_files;
    if ( !ListNetcdfTimeSeriesFiles( file_paths, sequenced_files ) )
    {
        return;
    }

    const int last_time_step = static_cast<int>( sequenced_files.size() ) - 1;
    constexpr int sub_volume_id = 1;
    constexpr int sub_volume_count = 1;
    int expected_cell_type = -1;
    int expected_grid_type = -1;
    int expected_veclen = -1;
    kvs::VolumeObjectBase::VolumeType expected_volume_type =
        kvs::VolumeObjectBase::UnknownVolumeType;
    std::string local_base;
    std::unique_ptr<cvt::UnstructuredPfi> pfi;

    // 数値順に並んだ各ファイルを連続するタイムステップとして変換する。
    for ( std::size_t i = 0; i < sequenced_files.size(); ++i )
    {
        const int time_step = static_cast<int>( i );
        const auto& file = sequenced_files[i];

        std::cout << "Reading " << file.path << " as time step " << time_step << " ..."
                  << std::endl;
        cvt::Netcdf input( file.path );
        if ( input.isFailure() || input.formatName() != file.format_name ||
             input.gridType() != file.grid_type )
        {
            std::cerr << "Failed to read the preflighted NetCDF time step "
                      << time_step << ": " << file.path << std::endl;
            return;
        }

        auto volume = ImportNetcdfVolume( input );
        if ( !volume )
        {
            std::cerr << "Failed to import NetCDF time step " << time_step
                      << ": " << file.path << std::endl;
            return;
        }

        int cell_type = -1;
        int grid_type = -1;
        if ( auto* unstructured =
                 dynamic_cast<kvs::UnstructuredVolumeObject*>( volume.get() ) )
        {
            cell_type = static_cast<int>( unstructured->cellType() );
        }
        else if ( auto* structured =
                      dynamic_cast<kvs::StructuredVolumeObject*>( volume.get() ) )
        {
            grid_type = static_cast<int>( structured->gridType() );
        }

        // 先頭ステップの構造を基準として、出力名とPFIを初期化する。
        if ( time_step == 0 )
        {
            expected_volume_type = volume->volumeType();
            expected_cell_type = cell_type;
            expected_grid_type = grid_type;
            expected_veclen = static_cast<int>( volume->veclen() );
            local_base =
                expected_volume_type == kvs::VolumeObjectBase::Unstructured
                    ? base + "_" + std::to_string( expected_cell_type )
                    : base;
            pfi = std::make_unique<cvt::UnstructuredPfi>(
                expected_veclen, last_time_step, sub_volume_count );
        }
        // 後続ステップのボリューム構造が先頭ステップと一致することを保証する。
        else if ( volume->volumeType() != expected_volume_type ||
                  cell_type != expected_cell_type ||
                  grid_type != expected_grid_type ||
                  static_cast<int>( volume->veclen() ) != expected_veclen )
        {
            std::cerr << "NetCDF time-series structure differs at " << file.path
                      << ": expected volume type " << expected_volume_type
                      << ", cell type " << expected_cell_type << ", grid type "
                      << expected_grid_type << ", and veclen " << expected_veclen
                      << ", but got volume type " << volume->volumeType()
                      << ", cell type " << cell_type << ", grid type " << grid_type
                      << ", and veclen " << volume->veclen() << std::endl;
            return;
        }

        if ( !WriteNetcdfVolume( directory, local_base, file.path, time_step,
                                 sub_volume_id, sub_volume_count, volume.get(), *pfi ) )
        {
            return;
        }
    }

    // 全タイムステップの出力後に、時系列全体のPFIとPFLを書き出す。
    if ( !pfi->write( directory, local_base ) )
    {
        std::cerr << "Failed to write the NetCDF time-series PFI file." << std::endl;
        return;
    }

    cvt::Pfl pfl;
    pfl.registerPfi( directory, local_base );
    if ( !pfl.write( directory, base ) )
    {
        std::cerr << "Failed to write the NetCDF time-series PFL file." << std::endl;
    }
}
