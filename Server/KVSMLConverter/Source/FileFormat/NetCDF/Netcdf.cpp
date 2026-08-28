/*
 * Copyright (c) 2026 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "FileFormat/NetCDF/Netcdf.h"

#include <limits>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "FileFormat/VTK/VtkXmlImageData.h"
#include "FileFormat/VTK/VtkXmlPolyData.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "FileFormat/VTK/RectilinearGridToUnstructured.h"
#include "kvs/Message"
#include "kvs/Type"
#include <vtkAppendFilter.h>
#include <vtkCallbackCommand.h>
#include <vtkCellDataToPointData.h>
#include <vtkCellType.h>
#include <vtkCommand.h>
#include <vtkCompositeDataIterator.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataArray.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDataSet.h>
#include <vtkErrorCode.h>
#include <vtkFloatArray.h>
#include <vtkGlobFileNames.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkMPASReader.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkNetCDFCAMReader.h>
#include <vtkNetCDFCFReader.h>
#include <vtkNetCDFPOPReader.h>
#include <vtkNetCDFReader.h>
#include <vtkNetCDFUGRIDReader.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkSLACReader.h>
#include <vtkStringArray.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStructuredGrid.h>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtk_netcdf.h>

namespace cvt
{
namespace detail
{

struct NetcdfDiagnostics
{
    std::string phase;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

struct NetcdfDimensions
{
    vtkIdType x = 0;
    vtkIdType y = 0;
    vtkIdType z = 0;
};

/**
 * @brief VTKリーダーが通知したエラーまたは警告を診断情報へ記録する。
 * @param event_id VTKイベントID。
 * @param client_data 診断情報へのポインター。
 * @param call_data VTKが通知したメッセージ。
 */
void OnNetcdfVtkMessage( vtkObject*, unsigned long event_id, void* client_data, void* call_data )
{
    auto* diagnostics = static_cast<NetcdfDiagnostics*>( client_data );
    const char* message = static_cast<const char*>( call_data );
    std::ostringstream text;
    text << diagnostics->phase << ": " << ( message ? message : "(no message)" );

    if ( event_id == vtkCommand::ErrorEvent )
    {
        diagnostics->errors.push_back( text.str() );
    }
    else
    {
        diagnostics->warnings.push_back( text.str() );
    }
}

/**
 * @brief NetCDFリーダーにエラー・警告監視用コールバックを登録する。
 * @param reader 監視対象のNetCDFリーダー。
 * @param callback 登録するVTKコールバック。
 * @param diagnostics 診断情報の格納先。
 */
void ObserveNetcdfReader( vtkNetCDFCFReader* reader, vtkCallbackCommand* callback,
                          NetcdfDiagnostics& diagnostics )
{
    callback->SetClientData( &diagnostics );
    callback->SetCallback( OnNetcdfVtkMessage );
    reader->AddObserver( vtkCommand::ErrorEvent, callback );
    reader->AddObserver( vtkCommand::WarningEvent, callback );
}

/**
 * @brief 必須変数の存在と次元を検証し、読み込み対象として選択する。
 * @param reader 設定対象のNetCDFリーダー。
 * @param dimensions 必須の次元文字列。
 * @param variable_names 読み込む変数名の一覧。
 * @param phase 診断メッセージに付加する処理名。
 * @param diagnostics 診断情報の格納先。
 * @return すべての必須変数が条件を満たす場合はtrue、それ以外はfalse。
 */
bool SelectNetcdfVariables( vtkNetCDFCFReader* reader, const std::string& dimensions,
                            const std::vector<std::string>& variable_names,
                            const std::string& phase, NetcdfDiagnostics& diagnostics )
{
    // 以降に発生するVTKメッセージがメタデータ取得中のものだと分かるよう処理名を設定する。
    diagnostics.phase = phase + " metadata";

    // 変数名や次元を検証する前に、ファイルからメタデータだけを読み込む。
    if ( reader->UpdateMetaData() == 0 )
    {
        diagnostics.errors.push_back( diagnostics.phase + ": failed to read NetCDF metadata" );
        return false;
    }

    // VTKが保持する各変数の次元文字列を、変数一覧と同じ添字で参照する。
    vtkStringArray* variable_dimensions = reader->GetVariableDimensions();
    bool valid = true;

    // 呼び出し側が指定した必須変数を一つずつNetCDFファイル内から検索する。
    for ( const auto& variable_name : variable_names )
    {
        bool found = false;
        for ( int i = 0; i < reader->GetNumberOfVariableArrays(); ++i )
        {
            if ( variable_name == reader->GetVariableArrayName( i ) )
            {
                found = true;

                // 同名の変数でも次元が異なる場合は対象形式の変数として扱わない。
                const std::string actual_dimensions = variable_dimensions->GetValue( i );
                if ( actual_dimensions != dimensions )
                {
                    diagnostics.errors.push_back(
                        phase + ": variable " + variable_name + " has dimensions " +
                        actual_dimensions + "; expected " + dimensions );
                    valid = false;
                }
                break;
            }
        }
        if ( !found )
        {
            // 必須変数が存在しないことを記録し、ほかの必須変数の検証を続ける。
            diagnostics.errors.push_back( phase + ": required variable " + variable_name +
                                          " was not found" );
            valid = false;
        }
    }
    if ( !valid )
    {
        // 一つでも条件を満たさない変数があれば、読み込み対象の設定は変更しない。
        return false;
    }

    // 検証済みの必須変数だけをリーダーの出力対象にする。
    // まず全変数を無効化し、不要な配列が出力へ含まれないようにする。
    for ( int i = 0; i < reader->GetNumberOfVariableArrays(); ++i )
    {
        reader->SetVariableArrayStatus( reader->GetVariableArrayName( i ), 0 );
    }

    // 必須変数だけを再度有効化する。
    for ( const auto& variable_name : variable_names )
    {
        reader->SetVariableArrayStatus( variable_name.c_str(), 1 );
    }
    return true;
}

/**
 * @brief データセットから必須の点データ配列を取得する。
 * @param data_set 検索対象のデータセット。
 * @param name 配列名。
 * @param phase 診断メッセージに付加する処理名。
 * @param diagnostics 診断情報の格納先。
 * @return 配列が存在する場合はそのポインター、存在しない場合はnullptr。
 */
vtkDataArray* GetRequiredNetcdfArray( vtkDataSet* data_set, const char* name,
                                      const std::string& phase,
                                      NetcdfDiagnostics& diagnostics )
{
    if ( !data_set )
    {
        diagnostics.errors.push_back( phase + ": the reader returned no data set" );
        return nullptr;
    }

    auto* array = data_set->GetPointData()->GetArray( name );
    if ( !array )
    {
        diagnostics.errors.push_back( phase + ": point-data array " + name + " was not loaded" );
    }
    return array;
}

/**
 * @brief VTKリーダーのエラーコードを確認して診断情報へ記録する。
 * @param reader 確認対象のNetCDFリーダー。
 * @param phase 診断メッセージに付加する処理名。
 * @param diagnostics 診断情報の格納先。
 * @return エラーがない場合はtrue、エラーがある場合はfalse。
 */
bool CheckNetcdfReaderError( vtkNetCDFCFReader* reader, const std::string& phase,
                             NetcdfDiagnostics& diagnostics )
{
    const unsigned long error_code = reader->GetErrorCode();
    if ( error_code == vtkErrorCode::NoError )
    {
        return true;
    }

    std::ostringstream message;
    message << phase << ": VTK error code " << error_code << " ("
            << vtkErrorCode::GetStringFromErrorCode( error_code ) << ")";
    diagnostics.errors.push_back( message.str() );
    return false;
}

/**
 * @brief 警告を出力し、エラーが記録されていれば例外を送出する。
 * @param diagnostics 出力対象の診断情報。
 * @throws std::runtime_error 一つ以上のエラーが記録されている場合。
 */
void ThrowNetcdfDiagnostics( const NetcdfDiagnostics& diagnostics )
{
    for ( const auto& warning : diagnostics.warnings )
    {
        kvsMessageWarning( warning.c_str() );
    }

    if ( !diagnostics.errors.empty() )
    {
        std::ostringstream message;
        message << "Failed to read NetCDF";
        for ( const auto& error : diagnostics.errors )
        {
            message << "\n  " << error;
        }
        throw std::runtime_error( message.str() );
    }
}

/** 文字列をASCII小文字へ変換する。 */
std::string Lowercase( std::string value )
{
    std::transform( value.begin(), value.end(), value.begin(),
                    []( unsigned char c ) { return static_cast<char>( std::tolower( c ) ); } );
    return value;
}

/** ファイル名の拡張子を小文字で返す。 */
std::string FilenameExtension( const std::string& filename )
{
    const auto separator = filename.find_last_of( "/\\" );
    const auto dot = filename.find_last_of( '.' );
    if ( dot == std::string::npos ||
         ( separator != std::string::npos && dot < separator ) ) return "";
    return Lowercase( filename.substr( dot ) );
}

/** 入力と同じディレクトリにある全エントリを列挙する。 */
std::vector<std::string> SiblingFiles( const std::string& filename )
{
    const auto separator = filename.find_last_of( "/\\" );
    const std::string directory =
        separator == std::string::npos ? "." : filename.substr( 0, separator );
    vtkNew<vtkGlobFileNames> glob;
    glob->RecurseOff();
    glob->AddFileNames( ( directory + "/*" ).c_str() );

    std::vector<std::string> files;
    vtkStringArray* names = glob->GetFileNames();
    files.reserve( static_cast<std::size_t>( names->GetNumberOfValues() ) );
    for ( vtkIdType i = 0; i < names->GetNumberOfValues(); ++i )
    {
        files.push_back( names->GetValue( i ) );
    }
    std::sort( files.begin(), files.end() );
    return files;
}

/** 次元表記に含まれる次元数を返す。 */
std::size_t DimensionRank( const std::string& dimensions )
{
    if ( dimensions.size() < 2 ) return 0;
    return 1 + static_cast<std::size_t>(
                   std::count( dimensions.begin(), dimensions.end(), ',' ) );
}

/** 指定ファイルがNetCDFとして開けるかを静かに確認する。 */
bool IsReadableNetcdfFile( const std::string& filename )
{
    int file = -1;
    if ( nc_open( filename.c_str(), NC_NOWRITE, &file ) != NC_NOERR ) return false;
    nc_close( file );
    return true;
}

/** 入力と同じディレクトリにあるNetCDFファイルを列挙する。 */
std::vector<std::string> SiblingNetcdfFiles( const std::string& filename )
{
    std::vector<std::string> files;
    for ( const auto& candidate : SiblingFiles( filename ) )
    {
        if ( IsReadableNetcdfFile( candidate ) ) files.push_back( candidate );
    }
    return files;
}

/** NetCDFの指定した次元長を取得する。 */
bool ReadNetcdfDimensionLength( const std::string& filename, const char* dimension,
                                std::size_t& length )
{
    int file = -1;
    if ( nc_open( filename.c_str(), NC_NOWRITE, &file ) != NC_NOERR ) return false;
    int dimension_id = -1;
    const bool success = nc_inq_dimid( file, dimension, &dimension_id ) == NC_NOERR &&
                         nc_inq_dimlen( file, dimension_id, &length ) == NC_NOERR;
    nc_close( file );
    return success;
}

/** SLACのfrequencyまたはfrequencyrealスカラーを取得する。 */
bool ReadSlacModeValue( const std::string& filename, double& value )
{
    int file = -1;
    if ( nc_open( filename.c_str(), NC_NOWRITE, &file ) != NC_NOERR ) return false;
    int variable = -1;
    bool success = false;
    for ( const char* name : { "frequency", "frequencyreal" } )
    {
        int rank = -1;
        if ( nc_inq_varid( file, name, &variable ) == NC_NOERR &&
             nc_inq_varndims( file, variable, &rank ) == NC_NOERR && rank == 0 &&
             nc_get_var_double( file, variable, &value ) == NC_NOERR )
        {
            success = true;
            break;
        }
    }
    nc_close( file );
    return success;
}

template <typename Predicate>
std::vector<std::string> FindSiblingNetcdfFiles( const std::string& filename,
                                                 Predicate predicate )
{
    std::vector<std::string> matches;
    for ( const auto& candidate : SiblingNetcdfFiles( filename ) )
    {
        cvt::NetcdfMetadata metadata;
        if ( cvt::Netcdf::ReadMetadata( candidate, metadata ) && predicate( metadata ) )
        {
            matches.push_back( candidate );
        }
    }
    return matches;
}

/** VTKデータセットのセルデータを点データへ変換する。 */
vtkSmartPointer<vtkDataSet> PointCenteredDataSet( vtkDataSet* input )
{
    if ( !input ) return nullptr;
    vtkNew<vtkCellDataToPointData> converter;
    converter->SetInputData( input );
    converter->PassCellDataOff();
    converter->Update();
    vtkSmartPointer<vtkDataSet> output = vtkDataSet::SafeDownCast( converter->GetOutput() );
    return output;
}

/** VTKデータセットの実際の型に対応するKVSMLConverterラッパーを生成する。 */
std::shared_ptr<kvs::FileFormatBase> WrapDataSet( vtkDataSet* input )
{
    auto data = PointCenteredDataSet( input );
    if ( !data || data->GetNumberOfPoints() == 0 || data->GetNumberOfCells() == 0 )
    {
        throw std::runtime_error( "the VTK NetCDF reader returned an empty data set" );
    }
    if ( auto* image = vtkImageData::SafeDownCast( data ) )
    {
        return std::make_shared<cvt::VtkXmlImageData>( image );
    }
    if ( auto* rectilinear = vtkRectilinearGrid::SafeDownCast( data ) )
    {
        return std::make_shared<cvt::VtkXmlRectilinearGrid>( rectilinear );
    }
    if ( auto* structured = vtkStructuredGrid::SafeDownCast( data ) )
    {
        return std::make_shared<cvt::VtkXmlStructuredGrid>( structured );
    }
    if ( auto* unstructured = vtkUnstructuredGrid::SafeDownCast( data ) )
    {
        return std::make_shared<cvt::VtkXmlUnstructuredGrid>( unstructured );
    }
    if ( auto* poly = vtkPolyData::SafeDownCast( data ) )
    {
        return std::make_shared<cvt::VtkXmlPolyData>( poly );
    }
    throw std::runtime_error( std::string( "unsupported VTK NetCDF output type: " ) +
                              data->GetClassName() );
}

/** コンポジット出力から最初の非空データセットを取得する。 */
vtkDataSet* FirstDataSet( vtkCompositeDataSet* composite )
{
    if ( !composite ) return nullptr;
    vtkSmartPointer<vtkCompositeDataIterator> iterator;
    iterator.TakeReference( composite->NewIterator() );
    iterator->SkipEmptyNodesOn();
    for ( iterator->InitTraversal(); !iterator->IsDoneWithTraversal();
          iterator->GoToNextItem() )
    {
        auto* data = vtkDataSet::SafeDownCast( iterator->GetCurrentDataObject() );
        if ( data && data->GetNumberOfPoints() > 0 ) return data;
    }
    return nullptr;
}

bool IsSlacMesh( const cvt::NetcdfMetadata& metadata )
{
    return metadata.hasVariable( "coords" ) &&
           metadata.hasVariable( "tetrahedron_interior" ) &&
           metadata.hasVariable( "tetrahedron_exterior" );
}

bool IsSlacMode( const cvt::NetcdfMetadata& metadata )
{
    return metadata.hasVariable( "coords" ) &&
           ( metadata.hasVariable( "frequency" ) ||
             metadata.hasVariable( "frequencyreal" ) ) &&
           !metadata.hasVariable( "tetrahedron_interior" );
}

/** SLAC Particle入力を汎用NetCDFとして誤変換しないための未対応形式判定。 */
bool IsUnsupportedSlacParticle( const cvt::NetcdfMetadata& metadata )
{
    return metadata.hasVariable( "particlePos" ) &&
           metadata.hasVariable( "particleInfo" ) && metadata.hasVariable( "time" );
}

bool IsCamPoints( const cvt::NetcdfMetadata& metadata )
{
    return metadata.hasDimension( "ncol" ) && metadata.hasDimension( "time" ) &&
           metadata.hasVariable( "lon", "(ncol)" ) &&
           metadata.hasVariable( "lat", "(ncol)" ) &&
           metadata.hasVariable( "time", "(time)" );
}

bool IsCamConnectivity( const cvt::NetcdfMetadata& metadata )
{
    return metadata.hasVariable( "element_corners" );
}

enum class CamVerticalMode
{
    Midpoint,
    Interface,
    Single
};

struct CamConfiguration
{
    CamVerticalMode vertical_mode = CamVerticalMode::Single;
    std::size_t vertical_plane_count = 1;
    std::vector<std::string> physical_variables;
};

bool IsCamCoordinateVariable( const std::string& name )
{
    return name == "time" || name == "lon" || name == "lat" || name == "lev" ||
           name == "ilev";
}

std::vector<std::string> CamVariablesWithDimensions(
    const cvt::NetcdfMetadata& metadata, const std::string& dimensions )
{
    std::vector<std::string> variables;
    for ( const auto& variable : metadata.variableDimensions() )
    {
        if ( !IsCamCoordinateVariable( variable.first ) && variable.second == dimensions )
        {
            variables.push_back( variable.first );
        }
    }
    return variables;
}

bool ValidateCamConnectivityIndices( const std::string& filename, std::size_t ncol,
                                     std::string& error )
{
    int file = -1;
    int status = nc_open( filename.c_str(), NC_NOWRITE, &file );
    if ( status != NC_NOERR )
    {
        error = std::string( "Failed to open the CAM connectivity file: " ) +
                nc_strerror( status ) + ": " + filename;
        return false;
    }
    auto close_file = [&]() {
        if ( file >= 0 ) nc_close( file );
        file = -1;
    };

    int variable = -1;
    status = nc_inq_varid( file, "element_corners", &variable );
    if ( status != NC_NOERR )
    {
        error = "The CAM connectivity file has no element_corners variable: " + filename;
        close_file();
        return false;
    }

    int rank = 0;
    int dimension_ids[NC_MAX_VAR_DIMS] = {};
    status = nc_inq_var( file, variable, nullptr, nullptr, &rank, dimension_ids, nullptr );
    if ( status != NC_NOERR || rank <= 0 )
    {
        error = "Failed to inspect CAM element_corners: " + filename;
        close_file();
        return false;
    }

    std::size_t value_count = 1;
    for ( int i = 0; i < rank; ++i )
    {
        std::size_t length = 0;
        if ( nc_inq_dimlen( file, dimension_ids[i], &length ) != NC_NOERR || length == 0 ||
             value_count > std::numeric_limits<std::size_t>::max() / length )
        {
            error = "Invalid CAM element_corners dimensions: " + filename;
            close_file();
            return false;
        }
        value_count *= length;
    }

    std::vector<long long> indices( value_count );
    status = nc_get_var_longlong( file, variable, indices.data() );
    close_file();
    if ( status != NC_NOERR )
    {
        error = std::string( "Failed to read CAM element_corners: " ) +
                nc_strerror( status ) + ": " + filename;
        return false;
    }
    for ( const long long index : indices )
    {
        if ( index < 1 || static_cast<unsigned long long>( index ) > ncol )
        {
            error = "CAM element_corners contains a node number outside the 1..ncol range: " +
                    filename;
            return false;
        }
    }
    return true;
}

bool ResolveCamConfiguration( const std::string& points_file,
                              const std::string& connectivity_file,
                              CamConfiguration& configuration, std::string& error )
{
    configuration = CamConfiguration{};
    error.clear();
    if ( connectivity_file.empty() )
    {
        error = "The CAM connectivity file path is empty";
        return false;
    }

    cvt::NetcdfMetadata points_metadata;
    if ( !cvt::Netcdf::ReadMetadata( points_file, points_metadata ) ||
         !IsCamPoints( points_metadata ) )
    {
        error = "The CAM points file requires ncol, lon(ncol), lat(ncol), and time(time): " +
                points_file;
        return false;
    }
    cvt::NetcdfMetadata connectivity_metadata;
    if ( !cvt::Netcdf::ReadMetadata( connectivity_file, connectivity_metadata ) ||
         !IsCamConnectivity( connectivity_metadata ) )
    {
        error = "The CAM connectivity file requires element_corners: " + connectivity_file;
        return false;
    }

    std::size_t ncol = 0;
    if ( !ReadNetcdfDimensionLength( points_file, "ncol", ncol ) || ncol == 0 )
    {
        error = "Failed to read a positive CAM ncol dimension: " + points_file;
        return false;
    }
    if ( !ValidateCamConnectivityIndices( connectivity_file, ncol, error ) ) return false;

    configuration.physical_variables =
        CamVariablesWithDimensions( points_metadata, "(time, lev, ncol)" );
    if ( !configuration.physical_variables.empty() )
    {
        configuration.vertical_mode = CamVerticalMode::Midpoint;
        if ( !ReadNetcdfDimensionLength(
                 points_file, "lev", configuration.vertical_plane_count ) ||
             configuration.vertical_plane_count == 0 )
        {
            error = "Failed to read a positive CAM lev dimension: " + points_file;
            return false;
        }
        return true;
    }
    configuration.physical_variables =
        CamVariablesWithDimensions( points_metadata, "(time, ilev, ncol)" );
    if ( !configuration.physical_variables.empty() )
    {
        configuration.vertical_mode = CamVerticalMode::Interface;
        if ( !ReadNetcdfDimensionLength(
                 points_file, "ilev", configuration.vertical_plane_count ) ||
             configuration.vertical_plane_count == 0 )
        {
            error = "Failed to read a positive CAM ilev dimension: " + points_file;
            return false;
        }
        return true;
    }
    configuration.physical_variables =
        CamVariablesWithDimensions( points_metadata, "(time, ncol)" );
    if ( !configuration.physical_variables.empty() )
    {
        configuration.vertical_mode = CamVerticalMode::Single;
        return true;
    }

    error = "The CAM points file has no physical variable with dimensions "
            "(time, lev, ncol), (time, ilev, ncol), or (time, ncol): " + points_file;
    return false;
}

void ConfigureCamReader( vtkNetCDFCAMReader* reader, const std::string& points_file,
                         const std::string& connectivity_file,
                         const CamConfiguration& configuration )
{
    reader->SetFileName( points_file.c_str() );
    reader->SetConnectivityFileName( connectivity_file.c_str() );
    switch ( configuration.vertical_mode )
    {
    case CamVerticalMode::Midpoint:
        reader->SetVerticalDimension(
            vtkNetCDFCAMReader::VERTICAL_DIMENSION_MIDPOINT_LAYERS );
        break;
    case CamVerticalMode::Interface:
        reader->SetVerticalDimension(
            vtkNetCDFCAMReader::VERTICAL_DIMENSION_INTERFACE_LAYERS );
        break;
    case CamVerticalMode::Single:
        reader->SetVerticalDimension(
            vtkNetCDFCAMReader::VERTICAL_DIMENSION_SINGLE_LAYER );
        break;
    }
}

class GearnNetcdfFormatAdapter : public cvt::NetcdfFormatAdapter
{
public:
    /// 対応形式名を返す。
    const char* name() const override { return "GEARN"; }
    /// GEARNデータの変換先格子種別を返す。
    cvt::NetcdfGridType gridType() const override
    {
        return cvt::NetcdfGridType::UnstructuredGrid;
    }

    /**
     * @brief メタデータがGEARN形式に必要な変数構成を持つかを判定する。
     * @param metadata 判定対象の変数メタデータ。
     * @return GEARN形式の条件を満たす場合はtrue、それ以外はfalse。
     */
    bool matches( const cvt::NetcdfMetadata& metadata ) const override
    {
        return metadata.hasVariable( "XDIS", "(NCX)" ) &&
               metadata.hasVariable( "YDIS", "(NCY)" ) &&
               metadata.hasVariable( "ZZT", "(NCZ, NCY, NCX)" ) &&
               metadata.hasVariable( "U", "(NCZ, NCY, NCX)" ) &&
               metadata.hasVariable( "V", "(NCZ, NCY, NCX)" ) &&
               metadata.hasVariable( "W", "(NCZ, NCY, NCX)" ) &&
               metadata.hasVariable( "CONC_Cs137", "(NCZ, NCY, NCX)" );
    }

    /**
     * @brief GEARN形式のNetCDFファイルを非構造格子へ変換して読み込む。
     * @param filename 入力ファイル名。
     * @return 変換後の非構造格子ファイル形式オブジェクト。
     */
    std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const override
    {
        NetcdfDiagnostics diagnostics;
        vtkNew<vtkNetCDFCFReader> x_reader;
        vtkNew<vtkNetCDFCFReader> y_reader;
        vtkNew<vtkNetCDFCFReader> data_reader;
        vtkNew<vtkCallbackCommand> callback;

        // 各リーダーのVTKメッセージを共通の診断情報へ集約する。
        ObserveNetcdfReader( x_reader, callback, diagnostics );
        ObserveNetcdfReader( y_reader, callback, diagnostics );
        ObserveNetcdfReader( data_reader, callback, diagnostics );

        x_reader->SetFileName( filename.c_str() );
        y_reader->SetFileName( filename.c_str() );
        data_reader->SetFileName( filename.c_str() );
        data_reader->SetOutputTypeToUnstructured();

        // 座標軸と3次元物理量を、それぞれ適切な次元を持つ変数に限定する。
        if ( !SelectNetcdfVariables( x_reader, "(NCX)", { "XDIS" }, "XDIS reader",
                                     diagnostics ) ||
             !SelectNetcdfVariables( y_reader, "(NCY)", { "YDIS" }, "YDIS reader",
                                     diagnostics ) ||
             !SelectNetcdfVariables(
                 data_reader, "(NCZ, NCY, NCX)", { "ZZT", "U", "V", "W", "CONC_Cs137" },
                 "3D data reader", diagnostics ) )
        {
            ThrowNetcdfDiagnostics( diagnostics );
        }

        // メタデータ検証後に実データを読み込む。
        diagnostics.phase = "XDIS reader Update";
        x_reader->Update();
        diagnostics.phase = "YDIS reader Update";
        y_reader->Update();
        diagnostics.phase = "3D data reader Update";
        data_reader->Update();

        CheckNetcdfReaderError( x_reader, "XDIS reader", diagnostics );
        CheckNetcdfReaderError( y_reader, "YDIS reader", diagnostics );
        CheckNetcdfReaderError( data_reader, "3D data reader", diagnostics );

        auto* x_data_set = vtkDataSet::SafeDownCast( x_reader->GetOutputDataObject( 0 ) );
        auto* y_data_set = vtkDataSet::SafeDownCast( y_reader->GetOutputDataObject( 0 ) );
        auto* data_grid =
            vtkUnstructuredGrid::SafeDownCast( data_reader->GetOutputDataObject( 0 ) );
        auto* xdis = GetRequiredNetcdfArray( x_data_set, "XDIS", "XDIS reader", diagnostics );
        auto* ydis = GetRequiredNetcdfArray( y_data_set, "YDIS", "YDIS reader", diagnostics );
        auto* zzt = GetRequiredNetcdfArray( data_grid, "ZZT", "3D data reader", diagnostics );
        auto* u = GetRequiredNetcdfArray( data_grid, "U", "3D data reader", diagnostics );
        auto* v = GetRequiredNetcdfArray( data_grid, "V", "3D data reader", diagnostics );
        auto* w = GetRequiredNetcdfArray( data_grid, "W", "3D data reader", diagnostics );
        auto* cs137 =
            GetRequiredNetcdfArray( data_grid, "CONC_Cs137", "3D data reader", diagnostics );
        ThrowNetcdfDiagnostics( diagnostics );

        // 座標配列と3次元格子の要素数が整合していることを確認する。
        const vtkIdType x_size = xdis->GetNumberOfTuples();
        const vtkIdType y_size = ydis->GetNumberOfTuples();
        const vtkIdType number_of_points = data_grid->GetNumberOfPoints();

        // X方向とY方向にセルを構成できる2点以上があり、総格子点数が正であることを確認する。
        // また、XY平面の点数をオーバーフローせずに計算でき、総格子点数が
        // XY平面1層分の点数で割り切れることを確認する。
        if ( x_size < 2 || y_size < 2 || number_of_points <= 0 ||
             x_size > std::numeric_limits<vtkIdType>::max() / y_size ||
             number_of_points % ( x_size * y_size ) != 0 )
        {
            throw std::runtime_error(
                "GEARN NetCDF coordinate assembly: NCX, NCY, and 3D point counts are "
                "inconsistent" );
        }

        NetcdfDimensions dimensions;
        dimensions.x = x_size;
        dimensions.y = y_size;
        dimensions.z = number_of_points / ( x_size * y_size );
        if ( dimensions.z < 2 )
        {
            throw std::runtime_error( "GEARN NetCDF coordinate assembly: NCZ must be at least 2" );
        }

        const vtkIdType number_of_cells =
            ( dimensions.x - 1 ) * ( dimensions.y - 1 ) * ( dimensions.z - 1 );
        if ( data_grid->GetNumberOfCells() != number_of_cells ||
             zzt->GetNumberOfTuples() != number_of_points ||
             u->GetNumberOfTuples() != number_of_points ||
             v->GetNumberOfTuples() != number_of_points ||
             w->GetNumberOfTuples() != number_of_points ||
             cs137->GetNumberOfTuples() != number_of_points )
        {
            throw std::runtime_error(
                "GEARN NetCDF coordinate assembly: point-data or hexahedral-cell counts are "
                "inconsistent" );
        }

        // KVSへ渡す物理量はすべて1成分の点データでなければならない。
        vtkDataArray* value_arrays[] = { u, v, w, cs137 };
        const char* value_names[] = { "U", "V", "W", "CONC_Cs137" };
        for ( std::size_t i = 0; i < 4; ++i )
        {
            if ( value_arrays[i]->GetNumberOfComponents() != 1 )
            {
                throw std::runtime_error( std::string( "GEARN NetCDF point-data array " ) +
                                          value_names[i] + " must have one component" );
            }
        }
        for ( vtkIdType i = 0; i < number_of_cells; ++i )
        {
            if ( data_grid->GetCellType( i ) != VTK_HEXAHEDRON )
            {
                throw std::runtime_error(
                    "GEARN NetCDF grid contains a non-hexahedral VTK cell" );
            }
        }
        if ( static_cast<unsigned long long>( number_of_points - 1 ) >
             std::numeric_limits<kvs::UInt32>::max() )
        {
            throw std::runtime_error(
                "GEARN NetCDF node count exceeds the KVS UInt32 connection limit" );
        }

        // 1次元のX/Y座標と3次元のZ座標から各格子点の座標を再構成する。
        vtkNew<vtkFloatArray> point_array;
        point_array->SetNumberOfComponents( 3 );
        point_array->SetNumberOfTuples( number_of_points );
        float* point_values = point_array->GetPointer( 0 );

#pragma omp parallel for
        for ( vtkIdType k = 0; k < dimensions.z; ++k )
        {
            for ( vtkIdType j = 0; j < dimensions.y; ++j )
            {
                for ( vtkIdType i = 0; i < dimensions.x; ++i )
                {
                    const vtkIdType id =
                        i + j * dimensions.x + k * dimensions.x * dimensions.y;
                    point_values[id * 3] = static_cast<float>( xdis->GetComponent( i, 0 ) );
                    point_values[id * 3 + 1] =
                        static_cast<float>( ydis->GetComponent( j, 0 ) );
                    point_values[id * 3 + 2] =
                        static_cast<float>( zzt->GetComponent( id, 0 ) );
                }
            }
        }

        // 再構成した座標と必要な物理量だけを持つ非構造格子を生成する。
        vtkNew<vtkPoints> points;
        points->SetData( point_array );
        vtkSmartPointer<vtkUnstructuredGrid> normalized =
            vtkSmartPointer<vtkUnstructuredGrid>::New();
        normalized->ShallowCopy( data_grid );
        normalized->SetPoints( points );
        normalized->GetPointData()->Initialize();
        normalized->GetPointData()->AddArray( u );
        normalized->GetPointData()->AddArray( v );
        normalized->GetPointData()->AddArray( w );
        normalized->GetPointData()->AddArray( cs137 );

        return std::make_shared<cvt::VtkXmlUnstructuredGrid>( normalized.GetPointer() );
    }

};

class SlacNetcdfFormatAdapter : public cvt::NetcdfFormatAdapter
{
public:
    const char* name() const override { return "VTK SLAC"; }
    cvt::NetcdfGridType gridType() const override
    {
        return cvt::NetcdfGridType::UnstructuredGrid;
    }
    bool matches( const cvt::NetcdfMetadata& metadata ) const override
    {
        return IsSlacMesh( metadata ) || IsSlacMode( metadata );
    }
    std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const override
    {
        cvt::NetcdfMetadata input_metadata;
        if ( !cvt::Netcdf::ReadMetadata( filename, input_metadata ) )
        {
            throw std::runtime_error( "failed to inspect the SLAC input" );
        }

        std::string mesh_file;
        if ( IsSlacMesh( input_metadata ) )
        {
            if ( FilenameExtension( filename ) != ".ncdf" )
            {
                throw std::runtime_error( "SLAC mesh input must use the .ncdf extension" );
            }
            mesh_file = filename;
        }
        else
        {
            std::size_t mode_coordinate_count = 0;
            if ( !ReadNetcdfDimensionLength( filename, "ncoord", mode_coordinate_count ) )
            {
                throw std::runtime_error( "failed to read the SLAC mode coordinate count" );
            }
            auto meshes = FindSiblingNetcdfFiles( filename, IsSlacMesh );
            meshes.erase(
                std::remove_if(
                    meshes.begin(), meshes.end(), [&]( const std::string& candidate ) {
                        std::size_t mesh_coordinate_count = 0;
                        return !ReadNetcdfDimensionLength(
                                   candidate, "ncoord", mesh_coordinate_count ) ||
                               mesh_coordinate_count != mode_coordinate_count;
                    } ),
                meshes.end() );
            if ( meshes.size() != 1 )
            {
                throw std::runtime_error(
                    "SLAC mode input requires exactly one coordinate-compatible companion "
                    "mesh file" );
            }
            mesh_file = meshes.front();
        }

        std::vector<cvt::SlacTimeStepFile> time_steps;
        std::string resolve_error;
        if ( !cvt::Netcdf::ResolveSlacTimeSeries( mesh_file, time_steps, resolve_error ) )
        {
            throw std::runtime_error( resolve_error );
        }
        std::vector<std::string> mode_files;
        mode_files.reserve( time_steps.size() );
        for ( const auto& step : time_steps ) mode_files.push_back( step.path );

        double requested_time = time_steps.front().time;
        if ( IsSlacMode( input_metadata ) && !ReadSlacModeValue( filename, requested_time ) )
        {
            throw std::runtime_error( "failed to read the requested SLAC time value" );
        }

        vtkNew<vtkSLACReader> reader;
        reader->SetMeshFileName( mesh_file.c_str() );
        for ( const auto& mode_file : mode_files )
        {
            reader->AddModeFileName( mode_file.c_str() );
        }
        reader->ReadExternalSurfaceOff();
        reader->ReadInternalVolumeOn();
        reader->ReadMidpointsOff();
        reader->UpdateInformation();
        reader->GetOutputInformation( vtkSLACReader::VOLUME_OUTPUT )
            ->Set( vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), requested_time );
        reader->Update( vtkSLACReader::VOLUME_OUTPUT );

        auto* blocks = vtkMultiBlockDataSet::SafeDownCast(
            reader->GetOutputDataObject( vtkSLACReader::VOLUME_OUTPUT ) );
        vtkDataSet* volume = FirstDataSet( blocks );
        if ( !volume )
        {
            throw std::runtime_error( "vtkSLACReader returned no internal volume" );
        }
        return WrapDataSet( volume );
    }
};

class CamNetcdfFormatAdapter : public cvt::NetcdfFormatAdapter
{
public:
    const char* name() const override { return "VTK CAM"; }
    cvt::NetcdfGridType gridType() const override { return cvt::NetcdfGridType::Unknown; }
    bool matches( const cvt::NetcdfMetadata& metadata ) const override
    {
        return IsCamPoints( metadata ) || IsCamConnectivity( metadata );
    }
    std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const override
    {
        return this->read( filename, cvt::NetcdfReadOptions{} );
    }
    std::shared_ptr<kvs::FileFormatBase> read(
        const std::string& filename, const cvt::NetcdfReadOptions& options ) const override
    {
        cvt::NetcdfMetadata metadata;
        if ( !cvt::Netcdf::ReadMetadata( filename, metadata ) )
        {
            throw std::runtime_error( "failed to inspect the CAM input" );
        }
        if ( IsCamConnectivity( metadata ) && !IsCamPoints( metadata ) )
        {
            throw std::runtime_error(
                "CAM connectivity cannot be used as the primary input; specify the points file" );
        }
        if ( !IsCamPoints( metadata ) )
        {
            throw std::runtime_error( "the primary CAM input is not a points file" );
        }

        CamConfiguration configuration;
        std::string error;
        if ( !ResolveCamConfiguration( filename, options.cam_connectivity_filename,
                                       configuration, error ) )
        {
            throw std::runtime_error( error );
        }

        vtkNew<vtkNetCDFCAMReader> reader;
        ConfigureCamReader( reader, filename, options.cam_connectivity_filename,
                            configuration );
        reader->UpdateInformation();
        if ( options.has_requested_time )
        {
            reader->GetOutputInformation( 0 )->Set(
                vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(),
                options.requested_time );
        }
        reader->Update();

        auto* output = vtkUnstructuredGrid::SafeDownCast( reader->GetOutputDataObject( 0 ) );
        if ( !output || output->GetNumberOfPoints() == 0 ||
             output->GetNumberOfCells() == 0 )
        {
            throw std::runtime_error( "vtkNetCDFCAMReader returned an empty grid" );
        }

        const int cell_type = output->GetCellType( 0 );
        if ( cell_type != VTK_HEXAHEDRON && cell_type != VTK_QUAD )
        {
            throw std::runtime_error(
                "vtkNetCDFCAMReader returned a cell type other than hexahedron or quad" );
        }
        const int expected_cell_type = configuration.vertical_plane_count >= 2
                                           ? VTK_HEXAHEDRON
                                           : VTK_QUAD;
        if ( cell_type != expected_cell_type )
        {
            throw std::runtime_error(
                "vtkNetCDFCAMReader cell type does not match the selected vertical mode" );
        }
        for ( vtkIdType i = 1; i < output->GetNumberOfCells(); ++i )
        {
            if ( output->GetCellType( i ) != cell_type )
            {
                throw std::runtime_error( "vtkNetCDFCAMReader returned mixed cell types" );
            }
        }

        std::vector<vtkSmartPointer<vtkDataArray>> physical_arrays;
        physical_arrays.reserve( configuration.physical_variables.size() );
        for ( const auto& name : configuration.physical_variables )
        {
            vtkDataArray* array = output->GetPointData()->GetArray( name.c_str() );
            if ( !array )
            {
                throw std::runtime_error( "vtkNetCDFCAMReader did not return physical array " +
                                          name );
            }
            if ( array->GetNumberOfComponents() != 1 ||
                 array->GetNumberOfTuples() != output->GetNumberOfPoints() )
            {
                throw std::runtime_error(
                    "CAM physical array must have one tuple per point and one component: " +
                    name );
            }
            physical_arrays.push_back( array );
        }

        vtkNew<vtkUnstructuredGrid> normalized;
        normalized->ShallowCopy( output );
        normalized->GetPointData()->Initialize();
        for ( const auto& array : physical_arrays )
            normalized->GetPointData()->AddArray( array );

        if ( cell_type == VTK_HEXAHEDRON )
        {
            return std::make_shared<cvt::VtkXmlUnstructuredGrid>( normalized.GetPointer() );
        }

        vtkNew<vtkDataSetSurfaceFilter> surface;
        surface->SetInputData( normalized );
        vtkNew<vtkTriangleFilter> triangles;
        triangles->SetInputConnection( surface->GetOutputPort() );
        triangles->Update();
        vtkPolyData* polygon = triangles->GetOutput();
        if ( !polygon || polygon->GetNumberOfPoints() == 0 ||
             polygon->GetNumberOfPolys() == 0 )
        {
            throw std::runtime_error( "failed to triangulate the CAM quad surface" );
        }
        return std::make_shared<cvt::VtkXmlPolyData>( polygon );
    }

    bool timeSteps( const std::string& filename, const cvt::NetcdfReadOptions& options,
                    std::vector<double>& time_steps, std::string& error ) const override
    {
        time_steps.clear();
        CamConfiguration configuration;
        if ( !ResolveCamConfiguration( filename, options.cam_connectivity_filename,
                                       configuration, error ) )
        {
            return false;
        }

        vtkNew<vtkNetCDFCAMReader> reader;
        ConfigureCamReader( reader, filename, options.cam_connectivity_filename,
                            configuration );
        reader->UpdateInformation();
        vtkInformation* information = reader->GetOutputInformation( 0 );
        auto* key = vtkStreamingDemandDrivenPipeline::TIME_STEPS();
        if ( !information || !information->Has( key ) )
        {
            error = "vtkNetCDFCAMReader did not publish TIME_STEPS";
            return false;
        }
        const int count = information->Length( key );
        if ( count <= 0 )
        {
            error = "vtkNetCDFCAMReader published an empty TIME_STEPS list";
            return false;
        }
        time_steps.reserve( static_cast<std::size_t>( count ) );
        for ( int i = 0; i < count; ++i )
        {
            const double time = information->Get( key, i );
            if ( !std::isfinite( time ) ||
                 std::find( time_steps.begin(), time_steps.end(), time ) != time_steps.end() )
            {
                error = "vtkNetCDFCAMReader published a non-finite or duplicate time step";
                time_steps.clear();
                return false;
            }
            time_steps.push_back( time );
        }
        std::sort( time_steps.begin(), time_steps.end() );
        return true;
    }
};

class MpasNetcdfFormatAdapter : public cvt::NetcdfFormatAdapter
{
public:
    const char* name() const override { return "VTK MPAS"; }
    cvt::NetcdfGridType gridType() const override
    {
        return cvt::NetcdfGridType::UnstructuredGrid;
    }
    bool matches( const cvt::NetcdfMetadata& metadata ) const override
    {
        return metadata.hasDimension( "nCells" ) && metadata.hasDimension( "nVertices" ) &&
               metadata.hasDimension( "vertexDegree" ) && metadata.hasDimension( "Time" );
    }
    std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const override
    {
        vtkNew<vtkMPASReader> reader;
        reader->SetFileName( filename.c_str() );
        // 2D表面ではなくPBVRが扱える体積セルを生成し、全垂直層を保持する。
        reader->SetShowMultilayerView( true );
        reader->Update();
        return WrapDataSet( vtkDataSet::SafeDownCast( reader->GetOutputDataObject( 0 ) ) );
    }
};

class UgridNetcdfFormatAdapter : public cvt::NetcdfFormatAdapter
{
public:
    const char* name() const override { return "VTK UGRID"; }
    cvt::NetcdfGridType gridType() const override { return cvt::NetcdfGridType::PolyData; }
    bool matches( const cvt::NetcdfMetadata& metadata ) const override
    {
        for ( const auto& variable : metadata.variableDimensions() )
        {
            if ( Lowercase( metadata.variableAttribute( variable.first, "cf_role" ) ) ==
                 "mesh_topology" )
            {
                return true;
            }
        }
        return false;
    }
    std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const override
    {
        vtkNew<vtkNetCDFUGRIDReader> reader;
        reader->SetFileName( filename.c_str() );
        reader->Update();

        auto point_centered = PointCenteredDataSet(
            vtkDataSet::SafeDownCast( reader->GetOutputDataObject( 0 ) ) );
        vtkNew<vtkDataSetSurfaceFilter> surface;
        surface->SetInputData( point_centered );
        vtkNew<vtkTriangleFilter> triangles;
        triangles->SetInputConnection( surface->GetOutputPort() );
        triangles->Update();
        auto* output = triangles->GetOutput();
        if ( !output || output->GetNumberOfPolys() == 0 )
        {
            throw std::runtime_error( "vtkNetCDFUGRIDReader returned no polygon cells" );
        }
        return std::make_shared<cvt::VtkXmlPolyData>( output );
    }
};

/** CF/COARDSファイルで最も高次元の物理量群を選択する。 */
bool SelectHighestRankDimensions( vtkNetCDFReader* reader )
{
    if ( reader->UpdateMetaData() == 0 ) return false;
    vtkStringArray* dimensions = reader->GetVariableDimensions();
    std::string selected;
    std::size_t selected_rank = 0;
    for ( int i = 0; i < reader->GetNumberOfVariableArrays(); ++i )
    {
        const std::string current = dimensions->GetValue( i );
        const std::size_t rank = DimensionRank( current );
        if ( rank > selected_rank )
        {
            selected = current;
            selected_rank = rank;
        }
    }
    if ( selected.empty() ) return false;
    reader->SetDimensions( selected.c_str() );
    return true;
}

/** vtkNetCDFReaderの次元表記を外側から順に分解する。 */
std::vector<std::string> ParseDimensionNames( const std::string& dimensions )
{
    const auto begin = dimensions.find( '(' );
    const auto end = dimensions.rfind( ')' );
    const std::size_t content_begin = begin == std::string::npos ? 0 : begin + 1;
    const std::size_t content_end = end == std::string::npos ? dimensions.size() : end;
    if ( content_begin >= content_end ) return {};

    std::vector<std::string> names;
    std::stringstream stream(
        dimensions.substr( content_begin, content_end - content_begin ) );
    std::string name;
    while ( std::getline( stream, name, ',' ) )
    {
        const auto first = name.find_first_not_of( " \t\r\n" );
        const auto last = name.find_last_not_of( " \t\r\n" );
        if ( first == std::string::npos ) return {};
        names.push_back( name.substr( first, last - first + 1 ) );
    }
    return names;
}

/** NetCDFメタデータから最大ランク物理量の次元名を返す。 */
std::vector<std::string> HighestRankDimensionNames(
    const cvt::NetcdfMetadata& metadata )
{
    std::string selected;
    std::size_t selected_rank = 0;
    for ( const auto& variable : metadata.variableDimensions() )
    {
        const std::size_t rank = DimensionRank( variable.second );
        if ( rank > selected_rank )
        {
            selected = variable.second;
            selected_rank = rank;
        }
    }
    return ParseDimensionNames( selected );
}

/** 同名1次元NetCDF座標変数を、単位変換せずdouble値として読む。 */
std::vector<double> ReadCoordinateVariable( int file, const std::string& dimension_name,
                                            std::size_t expected_length )
{
    int dimension = -1;
    std::size_t dimension_length = 0;
    int variable = -1;
    nc_type variable_type = NC_NAT;
    int rank = 0;
    int variable_dimensions[NC_MAX_VAR_DIMS] = {};
    if ( nc_inq_dimid( file, dimension_name.c_str(), &dimension ) != NC_NOERR ||
         nc_inq_dimlen( file, dimension, &dimension_length ) != NC_NOERR ||
         dimension_length != expected_length )
    {
        throw std::runtime_error( "coordinate count does not match dimension " +
                                  dimension_name );
    }
    if ( nc_inq_varid( file, dimension_name.c_str(), &variable ) != NC_NOERR ||
         nc_inq_var( file, variable, nullptr, &variable_type, &rank, variable_dimensions,
                     nullptr ) != NC_NOERR ||
         rank != 1 || variable_dimensions[0] != dimension )
    {
        throw std::runtime_error( "dimension " + dimension_name +
                                  " requires a same-named 1D coordinate variable" );
    }
    if ( variable_type == NC_CHAR
#ifdef NC_STRING
         || variable_type == NC_STRING
#endif
    )
    {
        throw std::runtime_error( "coordinate variable " + dimension_name +
                                  " is not numeric" );
    }

    std::vector<double> coordinates( expected_length );
    const int error = nc_get_var_double( file, variable, coordinates.data() );
    if ( error != NC_NOERR )
    {
        throw std::runtime_error( "failed to read coordinate variable " + dimension_name +
                                  ": " + nc_strerror( error ) );
    }
    return coordinates;
}

/** Generic NetCDFの3本の物理座標軸をx、y、z順で読む。 */
std::vector<std::vector<double>> ReadGenericRectilinearCoordinates(
    const std::string& filename, const std::vector<std::string>& dimension_names,
    const int image_dimensions[3] )
{
    if ( dimension_names.size() != 4 || Lowercase( dimension_names[0] ) != "time" ||
         Lowercase( dimension_names[1] ) != "z" ||
         Lowercase( dimension_names[2] ) != "y" ||
         Lowercase( dimension_names[3] ) != "x" )
    {
        throw std::runtime_error(
            "generic rectilinear NetCDF requires dimensions (time, z, y, x)" );
    }

    int file = -1;
    const int open_error = nc_open( filename.c_str(), NC_NOWRITE, &file );
    if ( open_error != NC_NOERR )
    {
        throw std::runtime_error( std::string( "failed to open coordinates: " ) +
                                  nc_strerror( open_error ) );
    }
    try
    {
        std::vector<std::vector<double>> coordinates( 3 );
        coordinates[0] = ReadCoordinateVariable(
            file, dimension_names[3], static_cast<std::size_t>( image_dimensions[0] ) );
        coordinates[1] = ReadCoordinateVariable(
            file, dimension_names[2], static_cast<std::size_t>( image_dimensions[1] ) );
        coordinates[2] = ReadCoordinateVariable(
            file, dimension_names[1], static_cast<std::size_t>( image_dimensions[2] ) );
        nc_close( file );
        return coordinates;
    }
    catch ( ... )
    {
        nc_close( file );
        throw;
    }
}

class CfNetcdfFormatAdapter : public cvt::NetcdfFormatAdapter
{
public:
    const char* name() const override { return "VTK CF"; }
    cvt::NetcdfGridType gridType() const override
    {
        return cvt::NetcdfGridType::UnstructuredGrid;
    }
    bool matches( const cvt::NetcdfMetadata& metadata ) const override
    {
        return Lowercase( metadata.globalAttribute( "Conventions" ) ).find( "cf-" ) !=
               std::string::npos;
    }
    std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const override
    {
        vtkNew<vtkNetCDFCFReader> reader;
        reader->SetFileName( filename.c_str() );
        if ( !SelectHighestRankDimensions( reader ) )
        {
            throw std::runtime_error( "vtkNetCDFCFReader found no readable data variable" );
        }
        reader->SetOutputTypeToUnstructured();
        reader->Update();
        return WrapDataSet( vtkDataSet::SafeDownCast( reader->GetOutputDataObject( 0 ) ) );
    }
};

class PopNetcdfFormatAdapter : public cvt::NetcdfFormatAdapter
{
public:
    const char* name() const override { return "VTK POP"; }
    cvt::NetcdfGridType gridType() const override
    {
        return cvt::NetcdfGridType::UnstructuredGrid;
    }
    bool matches( const cvt::NetcdfMetadata& metadata ) const override
    {
        if ( metadata.hasDimension( "time" ) || metadata.hasDimension( "Time" ) )
            return false;
        for ( const auto& variable : metadata.variableDimensions() )
        {
            if ( DimensionRank( variable.second ) == 3 ) return true;
        }
        return false;
    }
    std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const override
    {
        vtkNew<vtkNetCDFPOPReader> reader;
        reader->SetFileName( filename.c_str() );
        reader->Update();
        auto point_centered = PointCenteredDataSet(
            vtkDataSet::SafeDownCast( reader->GetOutputDataObject( 0 ) ) );
        auto* rectilinear = vtkRectilinearGrid::SafeDownCast( point_centered );
        if ( !rectilinear )
        {
            throw std::runtime_error(
                "vtkNetCDFPOPReader did not return a vtkRectilinearGrid" );
        }

        auto unstructured =
            cvt::RectilinearGridToLinearHexahedra( rectilinear );
        return std::make_shared<cvt::VtkXmlUnstructuredGrid>( unstructured );
    }
};

class GenericNetcdfFormatAdapter : public cvt::NetcdfFormatAdapter
{
public:
    const char* name() const override { return "VTK generic"; }
    cvt::NetcdfGridType gridType() const override
    {
        return cvt::NetcdfGridType::UnstructuredGrid;
    }
    bool matches( const cvt::NetcdfMetadata& metadata ) const override
    {
        return !metadata.variableDimensions().empty();
    }
    std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const override
    {
        vtkNew<vtkNetCDFReader> reader;
        reader->SetFileName( filename.c_str() );
        if ( !SelectHighestRankDimensions( reader ) )
        {
            throw std::runtime_error( "vtkNetCDFReader found no readable data variable" );
        }
        cvt::NetcdfMetadata metadata;
        if ( !cvt::Netcdf::ReadMetadata( filename, metadata ) )
        {
            throw std::runtime_error( "failed to read Generic NetCDF dimensions" );
        }
        const auto dimension_names = HighestRankDimensionNames( metadata );

        // A single NetCDF file is intentionally treated as one PBVR step. Request
        // the first physical time explicitly when VTK advertises time values.
        reader->UpdateInformation();
        vtkInformation* output_information = reader->GetOutputInformation( 0 );
        if ( output_information &&
             output_information->Has( vtkStreamingDemandDrivenPipeline::TIME_STEPS() ) &&
             output_information->Length( vtkStreamingDemandDrivenPipeline::TIME_STEPS() ) > 0 )
        {
            const double first_time =
                output_information->Get( vtkStreamingDemandDrivenPipeline::TIME_STEPS(), 0 );
            output_information->Set( vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(),
                                     first_time );
        }
        reader->Update();
        auto* image = vtkImageData::SafeDownCast( reader->GetOutputDataObject( 0 ) );
        if ( !image )
        {
            throw std::runtime_error( "vtkNetCDFReader did not return vtkImageData" );
        }

        int image_dimensions[3] = {};
        image->GetDimensions( image_dimensions );
        auto coordinates = ReadGenericRectilinearCoordinates(
            filename, dimension_names, image_dimensions );

        // Coordinate variables describe geometry and must not become PBVR value
        // components if a VTK version happens to expose them as point arrays.
        vtkNew<vtkImageData> point_fields;
        point_fields->ShallowCopy( image );
        for ( const auto& name : dimension_names )
        {
            point_fields->GetPointData()->RemoveArray( name.c_str() );
        }
        auto unstructured = cvt::RectilinearGridToLinearHexahedra(
            coordinates[0], coordinates[1], coordinates[2], point_fields );
        return std::make_shared<cvt::VtkXmlUnstructuredGrid>( unstructured );
    }
};

/**
 * @brief 利用可能なNetCDF形式アダプターの一覧を返す。
 * @return 登録済みアダプターの一覧。
 */
const std::vector<std::shared_ptr<cvt::NetcdfFormatAdapter>>& RegisteredNetcdfAdapters()
{
    static const std::vector<std::shared_ptr<cvt::NetcdfFormatAdapter>> adapters = {
        std::make_shared<GearnNetcdfFormatAdapter>(),
        std::make_shared<SlacNetcdfFormatAdapter>(),
        std::make_shared<CamNetcdfFormatAdapter>(),
        std::make_shared<MpasNetcdfFormatAdapter>(),
        std::make_shared<UgridNetcdfFormatAdapter>(),
        std::make_shared<CfNetcdfFormatAdapter>(),
        std::make_shared<PopNetcdfFormatAdapter>(),
        std::make_shared<GenericNetcdfFormatAdapter>()
    };
    return adapters;
}

/**
 * @brief 変換結果の実際の型がアダプターの格子種別と一致するかを判定する。
 * @param format 変換後のファイル形式オブジェクト。
 * @param grid_type 期待する格子種別。
 * @return 型が一致する場合はtrue、それ以外はfalse。
 */
bool MatchesNetcdfGridType( const std::shared_ptr<kvs::FileFormatBase>& format,
                            cvt::NetcdfGridType grid_type )
{
    switch ( grid_type )
    {
    case cvt::NetcdfGridType::ImageData:
        return dynamic_cast<cvt::VtkXmlImageData*>( format.get() ) != nullptr;
    case cvt::NetcdfGridType::RectilinearGrid:
        return dynamic_cast<cvt::VtkXmlRectilinearGrid*>( format.get() ) != nullptr;
    case cvt::NetcdfGridType::StructuredGrid:
        return dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) != nullptr;
    case cvt::NetcdfGridType::UnstructuredGrid:
        return dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) != nullptr;
    case cvt::NetcdfGridType::PolyData:
        return dynamic_cast<cvt::VtkXmlPolyData*>( format.get() ) != nullptr;
    case cvt::NetcdfGridType::Unknown:
        return dynamic_cast<cvt::VtkXmlImageData*>( format.get() ) != nullptr ||
               dynamic_cast<cvt::VtkXmlRectilinearGrid*>( format.get() ) != nullptr ||
               dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) != nullptr ||
               dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) != nullptr ||
               dynamic_cast<cvt::VtkXmlPolyData*>( format.get() ) != nullptr;
    default:
        return false;
    }
}

cvt::NetcdfGridType ActualNetcdfGridType(
    const std::shared_ptr<kvs::FileFormatBase>& format )
{
    if ( dynamic_cast<cvt::VtkXmlImageData*>( format.get() ) )
        return cvt::NetcdfGridType::ImageData;
    if ( dynamic_cast<cvt::VtkXmlRectilinearGrid*>( format.get() ) )
        return cvt::NetcdfGridType::RectilinearGrid;
    if ( dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) )
        return cvt::NetcdfGridType::StructuredGrid;
    if ( dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) )
        return cvt::NetcdfGridType::UnstructuredGrid;
    if ( dynamic_cast<cvt::VtkXmlPolyData*>( format.get() ) )
        return cvt::NetcdfGridType::PolyData;
    return cvt::NetcdfGridType::Unknown;
}
} // namespace detail
} // namespace cvt

bool cvt::Netcdf::ResolveSlacTimeSeries(
    const std::string& mesh_filename, std::vector<cvt::SlacTimeStepFile>& time_steps,
    std::string& error )
{
    time_steps.clear();
    error.clear();
    if ( cvt::detail::FilenameExtension( mesh_filename ) != ".ncdf" )
    {
        error = "SLAC mesh input must use the .ncdf extension: " + mesh_filename;
        return false;
    }

    std::vector<std::string> mesh_candidates;
    std::vector<std::string> mode_candidates;
    for ( const auto& candidate : cvt::detail::SiblingFiles( mesh_filename ) )
    {
        const auto extension = cvt::detail::FilenameExtension( candidate );
        if ( extension == ".ncdf" ) mesh_candidates.push_back( candidate );
        else if ( extension == ".mod" ) mode_candidates.push_back( candidate );
    }
    if ( mesh_candidates.size() != 1 )
    {
        error = "A SLAC input directory must contain exactly one .ncdf mesh file";
        return false;
    }

    cvt::NetcdfMetadata mesh_metadata;
    if ( !ReadMetadata( mesh_filename, mesh_metadata ) ||
         !cvt::detail::IsSlacMesh( mesh_metadata ) )
    {
        error = "The selected .ncdf file is not a SLAC mesh: " + mesh_filename;
        return false;
    }
    std::size_t mesh_coordinate_count = 0;
    if ( !cvt::detail::ReadNetcdfDimensionLength(
             mesh_filename, "ncoord", mesh_coordinate_count ) )
    {
        error = "Failed to read the SLAC mesh coordinate count: " + mesh_filename;
        return false;
    }
    if ( mode_candidates.empty() )
    {
        error = "The SLAC mesh has no companion .mod field files";
        return false;
    }

    for ( const auto& candidate : mode_candidates )
    {
        cvt::NetcdfMetadata metadata;
        if ( !ReadMetadata( candidate, metadata ) || !cvt::detail::IsSlacMode( metadata ) )
        {
            error = "Invalid SLAC .mod field file: " + candidate;
            return false;
        }
        std::size_t mode_coordinate_count = 0;
        if ( !cvt::detail::ReadNetcdfDimensionLength(
                 candidate, "ncoord", mode_coordinate_count ) ||
             mode_coordinate_count != mesh_coordinate_count )
        {
            error = "SLAC .mod ncoord does not match the .ncdf mesh: " + candidate;
            return false;
        }
        double time = 0.0;
        if ( !cvt::detail::ReadSlacModeValue( candidate, time ) || !std::isfinite( time ) )
        {
            error = "SLAC .mod requires a finite scalar frequency or frequencyreal: " +
                    candidate;
            return false;
        }
        if ( time >= 100.0 )
        {
            error = "SLAC frequency synthesis modes (frequency >= 100) are not supported: " +
                    candidate;
            return false;
        }
        time_steps.push_back( { candidate, time } );
    }

    std::sort( time_steps.begin(), time_steps.end(),
               []( const cvt::SlacTimeStepFile& lhs,
                   const cvt::SlacTimeStepFile& rhs ) { return lhs.time < rhs.time; } );
    for ( std::size_t i = 1; i < time_steps.size(); ++i )
    {
        if ( time_steps[i - 1].time == time_steps[i].time )
        {
            error = "SLAC .mod files contain a duplicate time value: " +
                    std::to_string( time_steps[i].time );
            time_steps.clear();
            return false;
        }
    }
    return true;
}

/**
 * @brief 指定した名前と次元を持つ変数がメタデータに存在するかを判定する。
 */
bool cvt::NetcdfMetadata::hasVariable( const std::string& name,
                                       const std::string& dimensions ) const
{
    const auto found = m_variable_dimensions.find( name );
    return found != m_variable_dimensions.end() && found->second == dimensions;
}

bool cvt::NetcdfMetadata::hasVariable( const std::string& name ) const
{
    return m_variable_dimensions.find( name ) != m_variable_dimensions.end();
}

bool cvt::NetcdfMetadata::hasDimension( const std::string& name ) const
{
    return m_dimensions.find( name ) != m_dimensions.end();
}

std::string cvt::NetcdfMetadata::variableAttribute( const std::string& variable,
                                                    const std::string& attribute ) const
{
    const auto found_variable = m_variable_attributes.find( variable );
    if ( found_variable == m_variable_attributes.end() ) return "";
    const auto found_attribute = found_variable->second.find( attribute );
    return found_attribute == found_variable->second.end() ? "" : found_attribute->second;
}

std::string cvt::NetcdfMetadata::globalAttribute( const std::string& attribute ) const
{
    const auto found = m_global_attributes.find( attribute );
    return found == m_global_attributes.end() ? "" : found->second;
}

/**
 * @brief NetCDFファイルから変数名と次元の対応を読み込む。
 * @param filename 入力ファイル名。
 * @param metadata 読み込んだメタデータの格納先。
 * @return 読み込みに成功した場合はtrue、それ以外はfalse。
 */
bool cvt::Netcdf::ReadMetadata( const std::string& filename, cvt::NetcdfMetadata& metadata )
{
    int file = -1;
    const int open_error = nc_open( filename.c_str(), NC_NOWRITE, &file );
    if ( open_error != NC_NOERR )
    {
        kvsMessageError( ( std::string( "Failed to open NetCDF metadata: " ) +
                           nc_strerror( open_error ) + ": " + filename )
                             .c_str() );
        return false;
    }

    auto close_file = [&]() {
        if ( file >= 0 )
        {
            nc_close( file );
            file = -1;
        }
    };
    auto fail = [&]( const std::string& phase, int error ) {
        kvsMessageError( ( std::string( "Failed to read NetCDF " ) + phase + ": " +
                           nc_strerror( error ) + ": " + filename )
                             .c_str() );
        close_file();
        return false;
    };
    auto read_text_attribute = [&]( int variable, const char* name ) {
        nc_type type = NC_NAT;
        std::size_t length = 0;
        if ( nc_inq_att( file, variable, name, &type, &length ) != NC_NOERR ) return std::string();
        if ( type == NC_CHAR )
        {
            std::string value( length, '\0' );
            if ( length > 0 && nc_get_att_text( file, variable, name, value.data() ) != NC_NOERR )
                return std::string();
            return value;
        }
#ifdef NC_STRING
        if ( type == NC_STRING && length > 0 )
        {
            char* value = nullptr;
            if ( nc_get_att_string( file, variable, name, &value ) != NC_NOERR || !value )
                return std::string();
            std::string result( value );
            nc_free_string( 1, &value );
            return result;
        }
#endif
        return std::string();
    };

    metadata.m_variable_dimensions.clear();
    metadata.m_dimensions.clear();
    metadata.m_global_attributes.clear();
    metadata.m_variable_attributes.clear();

    int dimension_count = 0;
    int variable_count = 0;
    int global_attribute_count = 0;
    int unlimited_dimension = -1;
    int error = nc_inq( file, &dimension_count, &variable_count, &global_attribute_count,
                        &unlimited_dimension );
    if ( error != NC_NOERR ) return fail( "header", error );

    for ( int i = 0; i < dimension_count; ++i )
    {
        char name[NC_MAX_NAME + 1] = {};
        std::size_t length = 0;
        error = nc_inq_dim( file, i, name, &length );
        if ( error != NC_NOERR ) return fail( "dimension", error );
        metadata.m_dimensions.emplace( name, length );
    }

    for ( int i = 0; i < global_attribute_count; ++i )
    {
        char name[NC_MAX_NAME + 1] = {};
        error = nc_inq_attname( file, NC_GLOBAL, i, name );
        if ( error != NC_NOERR ) return fail( "global attribute", error );
        const std::string value = read_text_attribute( NC_GLOBAL, name );
        if ( !value.empty() ) metadata.m_global_attributes.emplace( name, value );
    }

    for ( int variable = 0; variable < variable_count; ++variable )
    {
        char name[NC_MAX_NAME + 1] = {};
        nc_type type = NC_NAT;
        int rank = 0;
        int dimension_ids[NC_MAX_VAR_DIMS] = {};
        int attribute_count = 0;
        error = nc_inq_var( file, variable, name, &type, &rank, dimension_ids,
                            &attribute_count );
        if ( error != NC_NOERR ) return fail( "variable", error );

        std::ostringstream dimensions;
        dimensions << "(";
        for ( int j = 0; j < rank; ++j )
        {
            char dimension_name[NC_MAX_NAME + 1] = {};
            error = nc_inq_dimname( file, dimension_ids[j], dimension_name );
            if ( error != NC_NOERR ) return fail( "variable dimension", error );
            if ( j > 0 ) dimensions << ", ";
            dimensions << dimension_name;
        }
        dimensions << ")";
        metadata.m_variable_dimensions.emplace( name, dimensions.str() );

        auto& attributes = metadata.m_variable_attributes[name];
        for ( int j = 0; j < attribute_count; ++j )
        {
            char attribute_name[NC_MAX_NAME + 1] = {};
            error = nc_inq_attname( file, variable, j, attribute_name );
            if ( error != NC_NOERR ) return fail( "variable attribute", error );
            const std::string value = read_text_attribute( variable, attribute_name );
            if ( !value.empty() ) attributes.emplace( attribute_name, value );
        }
    }

    close_file();
    return true;
}

/**
 * @brief メタデータに適合するNetCDF形式アダプターを一つ選択する。
 * @param metadata 判定対象の変数メタデータ。
 * @return 一意に選択できたアダプター。未対応または曖昧な場合はnullptr。
 */
const cvt::NetcdfFormatAdapter* cvt::Netcdf::SelectAdapter(
    const cvt::NetcdfMetadata& metadata )
{
    if ( cvt::detail::IsUnsupportedSlacParticle( metadata ) )
    {
        kvsMessageError( "vtkSLACParticleReader input is not supported" );
        return nullptr;
    }
    for ( const auto& adapter : cvt::detail::RegisteredNetcdfAdapters() )
    {
        if ( adapter->matches( metadata ) )
        {
            // 特殊規約から汎用規約の順に登録しているため、最初の一致を採用する。
            return adapter.get();
        }
    }
    std::ostringstream message;
    message << "Unsupported NetCDF data format; variables:";
    for ( const auto& variable : metadata.variableDimensions() )
    {
        message << " " << variable.first << variable.second;
    }
    kvsMessageError( message.str().c_str() );
    return nullptr;
}

/**
 * @brief 指定したNetCDFファイルを読み込んでオブジェクトを初期化する。
 * @param filename 入力ファイル名。
 */
cvt::Netcdf::Netcdf( const std::string& filename ) { this->read( filename ); }

cvt::Netcdf::Netcdf( const std::string& filename, const cvt::NetcdfReadOptions& options )
{
    this->read( filename, options );
}

/**
 * @brief NetCDFファイルの形式を判別し、対応する格子データへ変換する。
 * @param filename 入力ファイル名。
 * @return 読み込みと変換に成功した場合はtrue、それ以外はfalse。
 */
bool cvt::Netcdf::read( const std::string& filename )
{
    return this->read( filename, cvt::NetcdfReadOptions{} );
}

bool cvt::Netcdf::read( const std::string& filename,
                        const cvt::NetcdfReadOptions& options )
{
    // 前回の読み込み結果を破棄し、失敗状態から処理を開始する。
    this->setFilename( filename );
    this->setSuccess( false );
    m_format.reset();
    m_format_name.clear();
    m_grid_type = cvt::NetcdfGridType::Unknown;

    // メタデータに基づいて入力形式を判別する。
    cvt::NetcdfMetadata metadata;
    if ( !ReadMetadata( filename, metadata ) )
    {
        return false;
    }

    const auto* adapter = SelectAdapter( metadata );
    if ( !adapter )
    {
        return false;
    }

    // 選択したアダプターで実データを読み込み、戻り値の格子型も検証する。
    try
    {
        m_format = adapter->read( filename, options );
        if ( !m_format ||
             !cvt::detail::MatchesNetcdfGridType( m_format, adapter->gridType() ) )
        {
            kvsMessageError( ( std::string( adapter->name() ) +
                               " NetCDF adapter returned an invalid VTK grid type for " +
                               filename )
                                 .c_str() );
            m_format.reset();
            return false;
        }
        m_format_name = adapter->name();
        m_grid_type = cvt::detail::ActualNetcdfGridType( m_format );
        this->setSuccess( true );
        return true;
    }
    catch ( const std::exception& e )
    {
        kvsMessageError( ( std::string( adapter->name() ) + " NetCDF: " + e.what() ).c_str() );
        return false;
    }
    catch ( ... )
    {
        kvsMessageError(
            ( std::string( "Unknown error while reading " ) + adapter->name() + " NetCDF" )
                .c_str() );
        return false;
    }
}

/**
 * @brief NetCDFファイルへの書き出し要求を未実装エラーとして処理する。
 * @param filename 出力ファイル名。
 * @return 常にfalse。
 */
bool cvt::Netcdf::write( const std::string& filename )
{
    this->setFilename( filename );
    this->setSuccess( false );
    kvsMessageError( "Writing NetCDF has not been implemented" );
    return false;
}

/**
 * @brief NetCDFファイルのメタデータから対応形式と格子種別を判別する。
 * @param filename 判別対象のファイル名。
 * @param info 判別結果の格納先。
 * @return 判別に成功した場合はtrue、それ以外はfalse。
 */
bool cvt::Netcdf::Probe( const std::string& filename, cvt::NetcdfFileInfo& info )
{
    cvt::NetcdfMetadata metadata;
    if ( !ReadMetadata( filename, metadata ) )
    {
        return false;
    }

    const auto* adapter = SelectAdapter( metadata );
    if ( !adapter )
    {
        return false;
    }

    info.path = filename;
    info.format_name = adapter->name();
    info.grid_type = adapter->gridType();
    if ( cvt::detail::IsCamPoints( metadata ) )
    {
        info.input_role = cvt::NetcdfInputRole::CamPoints;
    }
    else if ( cvt::detail::IsCamConnectivity( metadata ) )
    {
        info.input_role = cvt::NetcdfInputRole::CamConnectivity;
    }
    else
    {
        info.input_role = cvt::NetcdfInputRole::Standard;
    }
    return true;
}

bool cvt::Netcdf::TimeSteps( const std::string& filename,
                             const cvt::NetcdfReadOptions& options,
                             std::vector<double>& time_steps, std::string& error )
{
    time_steps.clear();
    error.clear();
    cvt::NetcdfMetadata metadata;
    if ( !ReadMetadata( filename, metadata ) )
    {
        error = "Failed to inspect the NetCDF file: " + filename;
        return false;
    }
    const auto* adapter = SelectAdapter( metadata );
    if ( !adapter )
    {
        error = "Unsupported NetCDF data format: " + filename;
        return false;
    }
    return adapter->timeSteps( filename, options, time_steps, error );
}
