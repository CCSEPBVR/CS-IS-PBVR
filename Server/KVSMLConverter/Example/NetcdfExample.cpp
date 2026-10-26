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
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cerrno>
#include <cstdlib>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <filesystem>
#else
#include <sys/stat.h>
#endif

#include "Exporter/StructuredVolumeObjectExporter.h"
#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "FileFormat/NetCDF/Netcdf.h"
#include "FileFormat/VTK/VtkXmlImageData.h"
#include "FileFormat/VTK/VtkXmlPolyData.h"
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
#include "kvs/KVSMLPolygonObject"
#include "kvs/PolygonExporter"
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkGlobFileNames.h>
#include <vtkPointData.h>
#include <vtkStringArray.h>

/**
 * @brief 時系列変換の事前検証で取得したNetCDFファイルの情報。
 */
struct SequencedNetcdfFile
{
    std::string path; ///< 入力ファイルのパス。
    std::string format_name; ///< NetCDFアダプターが判定した形式名。
    cvt::NetcdfGridType grid_type = cvt::NetcdfGridType::Unknown; ///< VTK格子形式。
    cvt::NetcdfInputRole input_role = cvt::NetcdfInputRole::Standard; ///< 入力の役割。
};

struct NetcdfDataSignature
{
    vtkIdType number_of_points = 0;
    vtkIdType number_of_cells = 0;
    int cell_type = -1;
    int component_count = 0;
    std::vector<std::pair<std::string, int>> point_arrays;
    cvt::NetcdfGridType grid_type = cvt::NetcdfGridType::Unknown;
};

bool ExpandSlacModeInput( const std::string& input, std::vector<std::string>& paths,
                          std::string& error )
{
    paths.clear();
    const auto first = input.find_first_not_of( " \t\r\n" );
    const auto last = input.find_last_not_of( " \t\r\n" );
    const std::string pattern = first == std::string::npos
                                    ? std::string()
                                    : input.substr( first, last - first + 1 );
    std::vector<std::string> candidates;
    if ( pattern.find( '*' ) == std::string::npos )
    {
        candidates.push_back( pattern );
    }
    else
    {
        vtkNew<vtkGlobFileNames> glob;
        glob->RecurseOff();
        glob->AddFileNames( pattern.c_str() );
        vtkStringArray* names = glob->GetFileNames();
        for ( vtkIdType i = 0; i < names->GetNumberOfValues(); ++i )
            candidates.push_back( names->GetValue( i ) );
    }

#ifndef _WIN32
    std::set<std::pair<dev_t, ino_t>> identities;
#endif
    for ( const auto& candidate : candidates )
    {
#ifdef _WIN32
        std::error_code filesystem_error;
        if ( !std::filesystem::is_regular_file( candidate, filesystem_error ) ||
             filesystem_error )
        {
            continue;
        }

        bool is_duplicate = false;
        for ( const auto& path : paths )
        {
            filesystem_error.clear();
            if ( std::filesystem::equivalent( candidate, path, filesystem_error ) &&
                 !filesystem_error )
            {
                is_duplicate = true;
                break;
            }
        }
        if ( is_duplicate )
#else
        struct stat status = {};
        if ( ::stat( candidate.c_str(), &status ) != 0 || !S_ISREG( status.st_mode ) ) continue;
        if ( !identities.emplace( status.st_dev, status.st_ino ).second )
#endif
        {
            error = "The SLAC mode input contains duplicate paths to the same file: " +
                    candidate;
            paths.clear();
            return false;
        }
        paths.push_back( candidate );
    }
    std::sort( paths.begin(), paths.end() );
    if ( paths.empty() )
    {
        error = "The SLAC mode path or wildcard matched no regular files: " + pattern;
        return false;
    }
    return true;
}

vtkDataSet* GetNetcdfDataSet( cvt::Netcdf& input )
{
    if ( auto* format = dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( input.format().get() ) )
        return format->get();
    if ( auto* format = dynamic_cast<cvt::VtkXmlPolyData*>( input.format().get() ) )
        return format->get();
    return nullptr;
}

bool InspectNetcdfData( cvt::Netcdf& input, NetcdfDataSignature& signature,
                        std::string& error )
{
    vtkDataSet* data = GetNetcdfDataSet( input );
    if ( !data || data->GetNumberOfPoints() <= 0 || data->GetNumberOfCells() <= 0 )
    {
        error = "The NetCDF adapter returned an empty data set";
        return false;
    }

    signature.number_of_points = data->GetNumberOfPoints();
    signature.number_of_cells = data->GetNumberOfCells();
    signature.cell_type = data->GetCellType( 0 );
    signature.component_count = 0;
    signature.point_arrays.clear();
    signature.grid_type = input.gridType();
    for ( vtkIdType i = 0; i < data->GetNumberOfCells(); ++i )
    {
        if ( data->GetCellType( i ) != signature.cell_type )
        {
            error = "The NetCDF adapter output contains mixed cell types";
            return false;
        }
    }
    for ( int i = 0; i < data->GetPointData()->GetNumberOfArrays(); ++i )
    {
        vtkDataArray* array = data->GetPointData()->GetArray( i );
        if ( !array || array->GetNumberOfTuples() != signature.number_of_points )
        {
            error = "A NetCDF value array does not contain one tuple per point";
            return false;
        }
        signature.point_arrays.emplace_back(
            array->GetName() ? array->GetName() : std::string(),
            array->GetNumberOfComponents() );
        signature.component_count += array->GetNumberOfComponents();
    }
    if ( signature.component_count <= 0 )
    {
        error = "The NetCDF adapter output contains no physical value component";
        return false;
    }
    return true;
}

bool SameNetcdfDataSignature( const NetcdfDataSignature& lhs,
                              const NetcdfDataSignature& rhs )
{
    return lhs.number_of_points == rhs.number_of_points &&
           lhs.number_of_cells == rhs.number_of_cells &&
           lhs.cell_type == rhs.cell_type &&
           lhs.component_count == rhs.component_count &&
           lhs.point_arrays == rhs.point_arrays &&
           lhs.grid_type == rhs.grid_type;
}

bool PrepareCamReadOptions( const std::string& points_file,
                            cvt::NetcdfReadOptions& options,
                            std::vector<double>& physical_times,
                            std::string& error )
{
    std::cout << "Enter the CAM connectivity file path:" << std::endl;
    if ( !std::getline( std::cin, options.cam_connectivity_filename ) ||
         options.cam_connectivity_filename.find_first_not_of( " \t\r\n" ) ==
             std::string::npos )
    {
        error = "The CAM connectivity file path is empty.";
        return false;
    }

    return cvt::Netcdf::TimeSteps( points_file, options, physical_times, error );
}

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
    // NetCDF固有の内容判定と入力ロールの切り替えは、変換処理側で行う。
    cvt::NetcdfFileInfo info;
    if ( !cvt::Netcdf::Probe( src, info ) )
    {
        std::cerr << "Failed to identify the NetCDF input: " << src << std::endl;
        return;
    }
    if ( info.input_role == cvt::NetcdfInputRole::CamConnectivity )
    {
        std::cerr << "CAM connectivity cannot be used as the primary input. "
                     "Specify the CAM points file instead."
                  << std::endl;
        return;
    }
    if ( info.input_role == cvt::NetcdfInputRole::SlacMode )
    {
        std::cerr << "A SLAC mode cannot be the primary input. Specify the SLAC mesh "
                     "as the first argument, then enter the mode path when prompted."
                  << std::endl;
        return;
    }
    if ( info.input_role == cvt::NetcdfInputRole::SlacMesh )
    {
        cvt::NetcdfReadOptions options;
        std::cout << "Enter the SLAC mode file path or wildcard:" << std::endl;
        std::string mode_input;
        if ( !std::getline( std::cin, mode_input ) ||
             mode_input.find_first_not_of( " \t\r\n" ) == std::string::npos )
        {
            std::cerr << "The SLAC mode file path or wildcard is empty." << std::endl;
            return;
        }
        std::string error;
        if ( !ExpandSlacModeInput( mode_input, options.slac_mode_filenames, error ) )
        {
            std::cerr << error << std::endl;
            return;
        }

        std::vector<double> physical_times;
        if ( !cvt::Netcdf::TimeSteps( src, options, physical_times, error ) ||
             physical_times.empty() )
        {
            std::cerr << ( error.empty() ? "No SLAC output steps were resolved" : error )
                      << std::endl;
            return;
        }
        constexpr int sub_volume_id = 1;
        constexpr int sub_volume_count = 1;
        const int last_time_step = static_cast<int>( physical_times.size() ) - 1;
        std::unique_ptr<cvt::UnstructuredPfi> pfi;
        std::size_t expected_veclen = 0;
        std::size_t expected_nodes = 0;
        std::size_t expected_cells = 0;
        kvs::UnstructuredVolumeObject::CellType expected_cell_type =
            kvs::UnstructuredVolumeObject::UnknownCellType;
        std::string local_base;
        for ( std::size_t i = 0; i < physical_times.size(); ++i )
        {
            const int time_step = static_cast<int>( i );
            options.has_requested_time = true;
            options.requested_time = physical_times[i];
            std::cout << "Reading SLAC physical time " << physical_times[i]
                      << " as PBVR step " << time_step << " ..." << std::endl;
            cvt::Netcdf input( src, options );
            if ( input.isFailure() ||
                 input.gridType() != cvt::NetcdfGridType::UnstructuredGrid )
            {
                std::cerr << "Failed to read the SLAC unstructured volume at PBVR step "
                          << time_step << std::endl;
                return;
            }
            auto volume = ImportNetcdfVolume( input );
            auto* unstructured =
                volume ? dynamic_cast<kvs::UnstructuredVolumeObject*>( volume.get() ) : nullptr;
            if ( !unstructured ||
                 unstructured->cellType() != kvs::UnstructuredVolumeObject::Tetrahedra )
            {
                std::cerr << "The SLAC adapter did not produce a tetrahedral unstructured "
                             "volume."
                          << std::endl;
                return;
            }
            if ( time_step == 0 )
            {
                expected_veclen = unstructured->veclen();
                expected_nodes = unstructured->numberOfNodes();
                expected_cells = unstructured->numberOfCells();
                expected_cell_type = unstructured->cellType();
                local_base = base + "_" + std::to_string( unstructured->cellType() );
                pfi = std::make_unique<cvt::UnstructuredPfi>(
                    expected_veclen, last_time_step, sub_volume_count );
            }
            else if ( unstructured->veclen() != expected_veclen ||
                      unstructured->numberOfNodes() != expected_nodes ||
                      unstructured->numberOfCells() != expected_cells ||
                      unstructured->cellType() != expected_cell_type )
            {
                std::cerr << "SLAC unstructured volume topology differs at PBVR step "
                          << time_step << std::endl;
                return;
            }

            if ( !WriteNetcdfVolume( directory, local_base, src, time_step, sub_volume_id,
                                     sub_volume_count, volume.get(), *pfi ) )
                return;
        }
        if ( !pfi->write( directory, local_base ) )
        {
            std::cerr << "Failed to write the SLAC PFI file." << std::endl;
            return;
        }
        cvt::Pfl pfl;
        pfl.registerPfi( directory, local_base );
        if ( !pfl.write( directory, base ) )
            std::cerr << "Failed to write the SLAC PFL file." << std::endl;
        return;
    }
    if ( info.input_role == cvt::NetcdfInputRole::CamPoints )
    {
        cvt::NetcdfReadOptions options;
        std::vector<double> physical_times;
        std::string error;
        if ( !PrepareCamReadOptions( src, options, physical_times, error ) )
        {
            std::cerr << error << std::endl;
            return;
        }

        constexpr int sub_volume_id = 1;
        constexpr int sub_volume_count = 1;
        const int last_time_step = static_cast<int>( physical_times.size() ) - 1;
        NetcdfDataSignature expected;
        bool writes_volume = false;
        std::string local_base;
        std::unique_ptr<cvt::UnstructuredPfi> pfi;

        for ( std::size_t i = 0; i < physical_times.size(); ++i )
        {
            const int time_step = static_cast<int>( i );
            options.has_requested_time = true;
            options.requested_time = physical_times[i];
            std::cout << "Reading " << src << " at physical time "
                      << physical_times[i] << " as PBVR step " << time_step << " ..."
                      << std::endl;

            cvt::Netcdf input( src, options );
            if ( input.isFailure() || input.formatName() != "VTK CAM" )
            {
                std::cerr << "Failed to read CAM time step " << time_step << std::endl;
                return;
            }

            NetcdfDataSignature current;
            if ( !InspectNetcdfData( input, current, error ) )
            {
                std::cerr << error << " at PBVR step " << time_step << std::endl;
                return;
            }
            if ( time_step == 0 )
            {
                expected = current;
                writes_volume = current.grid_type == cvt::NetcdfGridType::UnstructuredGrid;
                if ( !writes_volume && current.grid_type != cvt::NetcdfGridType::PolyData )
                {
                    std::cerr << "The CAM adapter returned an unsupported grid type."
                              << std::endl;
                    return;
                }
                if ( writes_volume )
                {
                    auto volume = ImportNetcdfVolume( input );
                    if ( !volume )
                    {
                        std::cerr << "Failed to import CAM time step 0." << std::endl;
                        return;
                    }
                    auto* unstructured =
                        dynamic_cast<kvs::UnstructuredVolumeObject*>( volume.get() );
                    if ( !unstructured )
                    {
                        std::cerr << "The CAM hexahedral grid did not produce an unstructured "
                                     "volume."
                                  << std::endl;
                        return;
                    }
                    local_base = base + "_" + std::to_string( unstructured->cellType() );
                    pfi = std::make_unique<cvt::UnstructuredPfi>(
                        expected.component_count, last_time_step, sub_volume_count );
                    if ( !WriteNetcdfVolume( directory, local_base, src, time_step,
                                             sub_volume_id, sub_volume_count, volume.get(),
                                             *pfi ) )
                    {
                        return;
                    }
                    continue;
                }
            }
            else if ( !SameNetcdfDataSignature( current, expected ) )
            {
                std::cerr << "CAM topology or value components differ at PBVR step "
                          << time_step << std::endl;
                return;
            }

            if ( writes_volume )
            {
                auto volume = ImportNetcdfVolume( input );
                if ( !volume ||
                     !WriteNetcdfVolume( directory, local_base, src, time_step,
                                         sub_volume_id, sub_volume_count, volume.get(), *pfi ) )
                {
                    std::cerr << "Failed to write CAM volume time step " << time_step
                              << std::endl;
                    return;
                }
            }
            else
            {
                auto* format = dynamic_cast<cvt::VtkXmlPolyData*>( input.format().get() );
                cvt::VtkImporter<cvt::VtkXmlPolyData> importer( format );
                if ( importer.isFailure() )
                {
                    std::cerr << "Failed to import CAM polygon time step " << time_step
                              << std::endl;
                    return;
                }
                kvs::PolygonExporter<kvs::KVSMLPolygonObject> exporter( &importer );
                exporter.setWritingDataTypeToExternalBinary();
                std::ostringstream filename;
                filename << base << "_" << std::setfill( '0' ) << std::setw( 5 ) << time_step
                         << ".kvsml";
                const std::string destination =
                    directory + std::string( 1, cvt::filesystem::path::preferred_separator ) +
                    filename.str();
                if ( !exporter.write( destination ) )
                {
                    std::cerr << "Failed to write CAM polygon time step " << time_step
                              << ": " << destination << std::endl;
                    return;
                }
            }
        }

        if ( !writes_volume ) return;
        if ( !pfi->write( directory, local_base ) )
        {
            std::cerr << "Failed to write the CAM PFI file." << std::endl;
            return;
        }
        cvt::Pfl pfl;
        pfl.registerPfi( directory, local_base );
        if ( !pfl.write( directory, base ) )
        {
            std::cerr << "Failed to write the CAM PFL file." << std::endl;
        }
        return;
    }

    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::Netcdf input( src );
    if ( input.isFailure() )
    {
        std::cerr << "Failed to read the NetCDF file: " << src << std::endl;
        return;
    }

    // UGRIDなどの2次元非構造格子は、面トポロジを保ったPolygon KVSMLへ出力する。
    if ( auto* format = dynamic_cast<cvt::VtkXmlPolyData*>( input.format().get() ) )
    {
        cvt::VtkImporter<cvt::VtkXmlPolyData> importer( format );
        if ( importer.isFailure() )
        {
            std::cerr << "Failed to import the polygon NetCDF file: " << src << std::endl;
            return;
        }
        kvs::PolygonExporter<kvs::KVSMLPolygonObject> exporter( &importer );
        exporter.setWritingDataTypeToExternalBinary();
        const std::string destination =
            directory + std::string( 1, cvt::filesystem::path::preferred_separator ) + base +
            ".kvsml";
        if ( !exporter.write( destination ) )
        {
            std::cerr << "Failed to write the polygon NetCDF file: " << destination
                      << std::endl;
        }
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
    cvt::NetcdfInputRole expected_input_role = cvt::NetcdfInputRole::Standard;
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
            expected_input_role = info.input_role;
        }
        else if ( info.format_name != expected_format ||
                  info.grid_type != expected_grid_type ||
                  info.input_role != expected_input_role )
        {
            std::cerr << "NetCDF time series mixes formats or VTK grid types: expected "
                      << expected_format << ", but " << path << " was detected as "
                      << info.format_name << std::endl;
            return false;
        }

        sequenced_files.push_back(
            { path, info.format_name, info.grid_type, info.input_role } );
    }
    return true;
}

/**
 * @brief NetCDF時系列のPolygonを、連続するタイムステップ名でKVSMLへ出力する。
 *
 * 先頭ステップの格子型、点・セル数、セル型および物理値配列を基準として、
 * 後続ステップの構造が一致することを確認する。Polygonはvolumeではないため
 * PFI/PFLには登録しない。
 */
bool WriteNetcdfPolygonSeries(
    const std::string& directory, const std::string& base,
    const std::vector<SequencedNetcdfFile>& sequenced_files )
{
    NetcdfDataSignature expected;
    std::string error;
    for ( std::size_t i = 0; i < sequenced_files.size(); ++i )
    {
        const int time_step = static_cast<int>( i );
        const auto& file = sequenced_files[i];
        std::cout << "Reading " << file.path << " as time step " << time_step << " ..."
                  << std::endl;

        cvt::Netcdf input( file.path );
        if ( input.isFailure() || input.formatName() != file.format_name ||
             input.gridType() != cvt::NetcdfGridType::PolyData )
        {
            std::cerr << "Failed to read the preflighted polygon NetCDF time step "
                      << time_step << ": " << file.path << std::endl;
            return false;
        }

        NetcdfDataSignature current;
        if ( !InspectNetcdfData( input, current, error ) )
        {
            std::cerr << error << " at time step " << time_step << ": " << file.path
                      << std::endl;
            return false;
        }
        if ( time_step == 0 )
        {
            expected = current;
        }
        else if ( !SameNetcdfDataSignature( current, expected ) )
        {
            std::cerr << "Polygon NetCDF topology or physical value components differ at "
                      << "time step " << time_step << ": " << file.path << std::endl;
            return false;
        }

        auto* format = dynamic_cast<cvt::VtkXmlPolyData*>( input.format().get() );
        if ( !format )
        {
            std::cerr << "The polygon NetCDF adapter did not return vtkPolyData at time "
                      << "step " << time_step << ": " << file.path << std::endl;
            return false;
        }
        cvt::VtkImporter<cvt::VtkXmlPolyData> importer( format );
        if ( importer.isFailure() )
        {
            std::cerr << "Failed to import polygon NetCDF time step " << time_step
                      << ": " << file.path << std::endl;
            return false;
        }

        kvs::PolygonExporter<kvs::KVSMLPolygonObject> exporter( &importer );
        exporter.setWritingDataTypeToExternalBinary();
        std::ostringstream filename;
        filename << base << "_" << std::setfill( '0' ) << std::setw( 5 ) << time_step
                 << ".kvsml";
        const std::string destination =
            directory + std::string( 1, cvt::filesystem::path::preferred_separator ) +
            filename.str();
        if ( !exporter.write( destination ) )
        {
            std::cerr << "Failed to write polygon NetCDF time step " << time_step
                      << ": " << destination << std::endl;
            return false;
        }
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

    const bool is_cam_series =
        sequenced_files.front().input_role == cvt::NetcdfInputRole::CamPoints;
    if ( sequenced_files.front().input_role == cvt::NetcdfInputRole::CamConnectivity )
    {
        std::cerr << "CAM connectivity cannot be used as the primary input. "
                     "Specify the CAM points files instead."
                  << std::endl;
        return;
    }

    // UGRIDなどのPolyData時系列はPolygonとして出力し、volume用PFI/PFLは作らない。
    if ( sequenced_files.front().grid_type == cvt::NetcdfGridType::PolyData )
    {
        WriteNetcdfPolygonSeries( directory, base, sequenced_files );
        return;
    }

    cvt::NetcdfReadOptions read_options;
    if ( is_cam_series )
    {
        std::vector<double> physical_times;
        std::string error;
        if ( !PrepareCamReadOptions( sequenced_files.front().path, read_options,
                                     physical_times, error ) )
        {
            std::cerr << error << std::endl;
            return;
        }
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
    NetcdfDataSignature expected_cam_signature;

    // 数値順に並んだ各ファイルを連続するタイムステップとして変換する。
    for ( std::size_t i = 0; i < sequenced_files.size(); ++i )
    {
        const int time_step = static_cast<int>( i );
        const auto& file = sequenced_files[i];

        std::cout << "Reading " << file.path << " as time step " << time_step << " ..."
                  << std::endl;
        cvt::Netcdf input( file.path, read_options );
        if ( input.isFailure() || input.formatName() != file.format_name ||
             ( file.grid_type != cvt::NetcdfGridType::Unknown &&
               input.gridType() != file.grid_type ) )
        {
            std::cerr << "Failed to read the preflighted NetCDF time step "
                      << time_step << ": " << file.path << std::endl;
            return;
        }

        if ( is_cam_series )
        {
            NetcdfDataSignature current;
            std::string error;
            if ( !InspectNetcdfData( input, current, error ) )
            {
                std::cerr << error << " at PBVR step " << time_step << std::endl;
                return;
            }
            if ( time_step == 0 )
            {
                expected_cam_signature = current;
            }
            else if ( !SameNetcdfDataSignature( current, expected_cam_signature ) )
            {
                std::cerr << "CAM topology or value components differ at PBVR step "
                          << time_step << std::endl;
                return;
            }

            if ( current.grid_type == cvt::NetcdfGridType::PolyData )
            {
                auto* format = dynamic_cast<cvt::VtkXmlPolyData*>( input.format().get() );
                cvt::VtkImporter<cvt::VtkXmlPolyData> importer( format );
                if ( importer.isFailure() )
                {
                    std::cerr << "Failed to import CAM polygon time step " << time_step
                              << std::endl;
                    return;
                }
                kvs::PolygonExporter<kvs::KVSMLPolygonObject> exporter( &importer );
                exporter.setWritingDataTypeToExternalBinary();
                std::ostringstream filename;
                filename << base << "_" << std::setfill( '0' ) << std::setw( 5 )
                         << time_step << ".kvsml";
                const std::string destination =
                    directory +
                    std::string( 1, cvt::filesystem::path::preferred_separator ) +
                    filename.str();
                if ( !exporter.write( destination ) )
                {
                    std::cerr << "Failed to write CAM polygon time step " << time_step
                              << ": " << destination << std::endl;
                    return;
                }
                continue;
            }
            if ( current.grid_type != cvt::NetcdfGridType::UnstructuredGrid )
            {
                std::cerr << "The CAM adapter returned an unsupported grid type."
                          << std::endl;
                return;
            }
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

    if ( is_cam_series &&
         expected_cam_signature.grid_type == cvt::NetcdfGridType::PolyData )
    {
        return;
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
