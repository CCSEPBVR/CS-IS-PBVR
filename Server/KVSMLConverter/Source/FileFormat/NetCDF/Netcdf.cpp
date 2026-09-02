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
#include <cstdint>
#include <iomanip>
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
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkCommand.h>
#include <vtkCompositeDataIterator.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataArray.h>
#include <vtkDataSetAttributes.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkErrorCode.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
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
#include <vtkResampleToImage.h>
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

/** 次元表記に含まれる次元数を返す。 */
std::size_t DimensionRank( const std::string& dimensions )
{
    if ( dimensions.size() < 2 ) return 0;
    return 1 + static_cast<std::size_t>(
                   std::count( dimensions.begin(), dimensions.end(), ',' ) );
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

bool IsNumericNetcdfType( int type )
{
    switch ( type )
    {
    case NC_BYTE:
    case NC_SHORT:
    case NC_INT:
    case NC_FLOAT:
    case NC_DOUBLE:
    case NC_UBYTE:
    case NC_USHORT:
    case NC_UINT:
    case NC_INT64:
    case NC_UINT64: return true;
    default: return false;
    }
}

bool IsSlacMesh( const cvt::NetcdfMetadata& metadata )
{
    const auto& coords = metadata.variableShape( "coords" );
    const auto& tetrahedra = metadata.variableShape( "tetrahedron_interior" );
    const int coords_type = metadata.variableType( "coords" );
    const int tetrahedra_type = metadata.variableType( "tetrahedron_interior" );
    const bool numeric_coords = IsNumericNetcdfType( coords_type );
    const bool integer_tetrahedra =
        tetrahedra_type == NC_BYTE || tetrahedra_type == NC_SHORT ||
        tetrahedra_type == NC_INT || tetrahedra_type == NC_UBYTE ||
        tetrahedra_type == NC_USHORT || tetrahedra_type == NC_UINT ||
        tetrahedra_type == NC_INT64 || tetrahedra_type == NC_UINT64;
    return coords.size() == 2 && coords[1] == 3 && numeric_coords &&
           tetrahedra.size() == 2 && tetrahedra[1] == 5 && integer_tetrahedra;
}

bool IsSlacMode( const cvt::NetcdfMetadata& metadata )
{
    const auto& coords = metadata.variableShape( "coords" );
    const char* frequency_name = metadata.hasVariable( "frequency" )
                                     ? "frequency"
                                     : "frequencyreal";
    const auto& frequency = metadata.variableShape( frequency_name );
    const int frequency_type = metadata.variableType( frequency_name );
    const bool numeric_frequency = IsNumericNetcdfType( frequency_type );
    return coords.size() == 2 && coords[1] == 3 &&
           metadata.hasVariable( frequency_name ) && frequency.empty() && numeric_frequency &&
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
    enum class ModeKind { TimeStep, Frequency };

    struct VariableSignature
    {
        int type = NC_NAT;
        int rank = 0;
        std::size_t components = 0;
    };

    struct Configuration
    {
        ModeKind kind = ModeKind::TimeStep;
        std::vector<cvt::SlacTimeStepFile> modes;
    };

    static bool ModeVariables( const cvt::NetcdfMetadata& metadata,
                               std::size_t coordinate_count,
                               std::map<std::string, VariableSignature>& variables )
    {
        variables.clear();
        for ( const auto& entry : metadata.variableDimensions() )
        {
            const std::string& name = entry.first;
            const auto& shape = metadata.variableShape( name );
            if ( name == "coords" || name == "frequency" || name == "frequencyreal" ||
                 shape.empty() || shape.front() != coordinate_count ) continue;
            std::size_t components = 1;
            for ( std::size_t i = 1; i < shape.size(); ++i )
            {
                if ( shape[i] == 0 || components >
                     std::numeric_limits<std::size_t>::max() / shape[i] ) return false;
                components *= shape[i];
            }
            variables.emplace( name, VariableSignature{ metadata.variableType( name ),
                                                        static_cast<int>( shape.size() ),
                                                        components } );
        }
        return !variables.empty();
    }

    static bool CompatibleVariables(
        const std::map<std::string, VariableSignature>& lhs,
        const std::map<std::string, VariableSignature>& rhs )
    {
        if ( lhs.size() != rhs.size() ) return false;
        for ( const auto& entry : lhs )
        {
            const auto found = rhs.find( entry.first );
            if ( found == rhs.end() ) return false;
            const auto& a = entry.second;
            const auto& b = found->second;
            if ( a.type != b.type || a.rank != b.rank || a.components != b.components )
                return false;
        }
        return true;
    }

    static bool ResolveConfiguration( const std::string& mesh_filename,
                                      const cvt::NetcdfReadOptions& options,
                                      Configuration& configuration, std::string& error )
    {
        configuration = Configuration{};
        std::size_t sample_points = 1;
        for ( int dimension : options.slac_sampling_dimensions )
        {
            if ( dimension < 2 ||
                 sample_points > std::numeric_limits<std::size_t>::max() /
                                     static_cast<std::size_t>( dimension ) )
            {
                error = "Invalid or overflowing SLAC sampling dimensions";
                return false;
            }
            sample_points *= static_cast<std::size_t>( dimension );
        }
        cvt::NetcdfMetadata mesh;
        if ( !cvt::Netcdf::ReadMetadata( mesh_filename, mesh ) || !IsSlacMesh( mesh ) )
        {
            error = "The primary input is not a SLAC internal-volume mesh: " + mesh_filename;
            return false;
        }
        if ( options.slac_mode_filenames.empty() )
        {
            error = "At least one explicit SLAC mode file is required";
            return false;
        }
        const std::size_t coordinate_count = mesh.variableShape( "coords" ).front();
        std::map<std::string, VariableSignature> expected_variables;
        bool first = true;
        bool frequency_modes = false;
        for ( const auto& filename : options.slac_mode_filenames )
        {
            cvt::NetcdfMetadata mode;
            if ( !cvt::Netcdf::ReadMetadata( filename, mode ) || !IsSlacMode( mode ) )
            {
                error = "Invalid SLAC mode file: " + filename;
                return false;
            }
            if ( mode.variableShape( "coords" ).front() != coordinate_count )
            {
                error = "SLAC mesh/mode coords tuple counts differ: " + filename;
                return false;
            }
            double value = 0.0;
            if ( !ReadSlacModeValue( filename, value ) || !std::isfinite( value ) )
            {
                error = "SLAC mode requires a finite scalar frequency or frequencyreal: " +
                        filename;
                return false;
            }
            const bool current_frequency = value >= 100.0;
            std::map<std::string, VariableSignature> variables;
            if ( !ModeVariables( mode, coordinate_count, variables ) )
            {
                error = "SLAC mode has no compatible physical point-data arrays: " + filename;
                return false;
            }
            if ( first )
            {
                frequency_modes = current_frequency;
                expected_variables = variables;
                first = false;
            }
            else
            {
                if ( frequency_modes != current_frequency )
                {
                    error = "SLAC time-step and frequency modes cannot be mixed";
                    return false;
                }
                if ( frequency_modes )
                {
                    error = "Multiple SLAC frequency modes are not supported";
                    return false;
                }
                if ( !CompatibleVariables( expected_variables, variables ) )
                {
                    error = "SLAC time-step modes have incompatible physical arrays";
                    return false;
                }
            }
            configuration.modes.push_back( { filename, value } );
        }
        std::sort( configuration.modes.begin(), configuration.modes.end(),
                   []( const cvt::SlacTimeStepFile& a, const cvt::SlacTimeStepFile& b ) {
                       return a.time < b.time;
                   } );
        for ( std::size_t i = 1; i < configuration.modes.size(); ++i )
        {
            if ( configuration.modes[i - 1].time == configuration.modes[i].time )
            {
                error = "SLAC modes contain a duplicate mode value: " +
                        std::to_string( configuration.modes[i].time );
                return false;
            }
        }
        configuration.kind = frequency_modes ? ModeKind::Frequency : ModeKind::TimeStep;
        std::cout << "SLAC mesh/mode coords tuples: " << coordinate_count << std::endl;
        for ( const auto& mode : configuration.modes )
        {
            std::cout << "SLAC mode value: " << std::setprecision( 17 ) << mode.time
                      << " (" << mode.path << ")" << std::endl;
        }
        return true;
    }

    static vtkPointData* SharedPointData( vtkDataObject* object )
    {
        if ( auto* data = vtkDataSet::SafeDownCast( object ) ) return data->GetPointData();
        auto* composite = vtkCompositeDataSet::SafeDownCast( object );
        if ( !composite ) return nullptr;
        vtkSmartPointer<vtkCompositeDataIterator> iterator;
        iterator.TakeReference( composite->NewIterator() );
        iterator->SkipEmptyNodesOn();
        for ( iterator->InitTraversal(); !iterator->IsDoneWithTraversal();
              iterator->GoToNextItem() )
        {
            auto* data = vtkDataSet::SafeDownCast( iterator->GetCurrentDataObject() );
            if ( data && data->GetPointData() ) return data->GetPointData();
        }
        return nullptr;
    }

    static void LogMemoryEstimate( vtkDataObject* volume, const std::array<int, 3>& dims )
    {
        vtkPointData* point_data = SharedPointData( volume );
        std::uint64_t components = 0;
        std::uint64_t vtk_bytes_per_point = 0;
        if ( point_data )
        {
            for ( int i = 0; i < point_data->GetNumberOfArrays(); ++i )
            {
                vtkDataArray* array = point_data->GetArray( i );
                if ( !array ) continue;
                components += static_cast<std::uint64_t>( array->GetNumberOfComponents() );
                vtk_bytes_per_point +=
                    static_cast<std::uint64_t>( array->GetNumberOfComponents() ) *
                    static_cast<std::uint64_t>( array->GetDataTypeSize() );
            }
        }
        const std::uint64_t points = static_cast<std::uint64_t>( dims[0] ) * dims[1] * dims[2];
        const std::uint64_t cells = static_cast<std::uint64_t>( dims[0] - 1 ) *
                                    ( dims[1] - 1 ) * ( dims[2] - 1 );
        const std::uint64_t vtk_values = points * vtk_bytes_per_point;
        const std::uint64_t kvs_values = points * components * sizeof( float );
        const std::uint64_t auxiliaries = points * 2 + cells;
        std::cout << "SLAC resample memory estimate:" << std::endl
                  << "  point count: " << points << std::endl
                  << "  value components: " << components << std::endl
                  << "  VTK value arrays: " << vtk_values << " bytes" << std::endl
                  << "  KVS float values: " << kvs_values << " bytes" << std::endl
                  << "  mask/point ghost/cell ghost: " << auxiliaries << " bytes" << std::endl
                  << "  simultaneous estimated total: "
                  << vtk_values + kvs_values + auxiliaries << " bytes" << std::endl;
    }

    static void ValidateAndLogImage( vtkImageData* image, const char* mask_name,
                                     const std::array<int, 3>& expected_dimensions )
    {
        if ( !image || image->GetNumberOfPoints() <= 0 || image->GetNumberOfCells() <= 0 )
            throw std::runtime_error( "vtkResampleToImage returned an empty image" );
        int dimensions[3] = {};
        image->GetDimensions( dimensions );
        for ( int axis = 0; axis < 3; ++axis )
            if ( dimensions[axis] != expected_dimensions[axis] )
                throw std::runtime_error( "vtkResampleToImage dimensions do not match the request" );
        const double* bounds = image->GetBounds();
        const double* spacing = image->GetSpacing();
        for ( int i = 0; i < 6; ++i )
            if ( !std::isfinite( bounds[i] ) )
                throw std::runtime_error( "vtkResampleToImage returned non-finite bounds" );
        for ( int i = 0; i < 3; ++i )
            if ( !std::isfinite( spacing[i] ) )
                throw std::runtime_error( "vtkResampleToImage returned non-finite spacing" );
        std::cout << "SLAC resample dimensions: " << dimensions[0] << " " << dimensions[1]
                  << " " << dimensions[2] << std::endl
                  << "SLAC resample bounds: " << bounds[0] << " " << bounds[1] << " "
                  << bounds[2] << " " << bounds[3] << " " << bounds[4] << " "
                  << bounds[5] << std::endl
                  << "SLAC resample spacing: " << spacing[0] << " " << spacing[1] << " "
                  << spacing[2] << std::endl;

        vtkPointData* point_data = image->GetPointData();
        vtkDataArray* mask = point_data->GetArray( mask_name );
        if ( !mask ) throw std::runtime_error( "vtkResampleToImage returned no valid-point mask" );
        vtkIdType valid_count = 0;
        for ( vtkIdType i = 0; i < image->GetNumberOfPoints(); ++i )
            if ( mask->GetComponent( i, 0 ) != 0.0 ) ++valid_count;
        if ( valid_count == 0 )
            throw std::runtime_error( "vtkResampleToImage found no valid SLAC volume points" );

        int physical_arrays = 0;
        int component_offset = 0;
        bool any_finite = false;
        bool all_zero = true;
        for ( int i = 0; i < point_data->GetNumberOfArrays(); ++i )
        {
            vtkDataArray* array = point_data->GetArray( i );
            if ( !array ) continue;
            const std::string name = array->GetName() ? array->GetName() : "(unnamed)";
            if ( name == mask_name || name == vtkDataSetAttributes::GhostArrayName() ) continue;
            if ( array->GetNumberOfTuples() != image->GetNumberOfPoints() )
                throw std::runtime_error( "A resampled SLAC array has an invalid tuple count" );
            ++physical_arrays;
            const int components = array->GetNumberOfComponents();
            std::cout << "Array order " << physical_arrays - 1 << ": " << name << std::endl
                      << "  Components: " << components << std::endl
                      << "  KVS component range: " << component_offset << "-"
                      << component_offset + components - 1 << std::endl;
            for ( int component = 0; component < components; ++component )
            {
                double all_range[2] = {};
                array->GetRange( all_range, component );
                double valid_min = std::numeric_limits<double>::infinity();
                double valid_max = -std::numeric_limits<double>::infinity();
                for ( vtkIdType tuple = 0; tuple < array->GetNumberOfTuples(); ++tuple )
                {
                    const double value = array->GetComponent( tuple, component );
                    if ( std::isfinite( value ) )
                    {
                        any_finite = true;
                        if ( value != 0.0 ) all_zero = false;
                    }
                    if ( mask->GetComponent( tuple, 0 ) != 0.0 && std::isfinite( value ) )
                    {
                        valid_min = std::min( valid_min, value );
                        valid_max = std::max( valid_max, value );
                    }
                }
                std::cout << "  Component " << component << " all-point min/max: "
                          << all_range[0] << " " << all_range[1] << std::endl
                          << "  Component " << component << " valid-point min/max: "
                          << valid_min << " " << valid_max << std::endl;
            }
            component_offset += components;
        }
        if ( physical_arrays == 0 )
            throw std::runtime_error( "Resampled SLAC volume has no physical point data" );
        if ( !any_finite )
            throw std::runtime_error( "All resampled SLAC physical values are non-finite" );
        if ( all_zero ) kvsMessageWarning( "All resampled SLAC physical values are zero" );
        std::cout << "SLAC valid resample points: " << valid_count << "/"
                  << image->GetNumberOfPoints() << std::endl;
    }

public:
    const char* name() const override { return "VTK SLAC"; }
    cvt::NetcdfGridType gridType() const override
    {
        return cvt::NetcdfGridType::ImageData;
    }
    bool matches( const cvt::NetcdfMetadata& metadata ) const override
    {
        return IsSlacMesh( metadata ) || IsSlacMode( metadata );
    }
    std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const override
    {
        return this->read( filename, cvt::NetcdfReadOptions{} );
    }

    std::shared_ptr<kvs::FileFormatBase> read(
        const std::string& filename, const cvt::NetcdfReadOptions& options ) const override
    {
        Configuration configuration;
        std::string error;
        if ( !ResolveConfiguration( filename, options, configuration, error ) )
            throw std::runtime_error( error );

        vtkNew<vtkSLACReader> reader;
        reader->SetMeshFileName( filename.c_str() );
        for ( const auto& mode : configuration.modes ) reader->AddModeFileName( mode.path.c_str() );
        reader->ReadExternalSurfaceOff();
        reader->ReadInternalVolumeOn();
        reader->ReadMidpointsOff();
        reader->UpdateInformation();
        const double update_time = configuration.kind == ModeKind::Frequency
                                       ? 0.0
                                       : ( options.has_requested_time
                                               ? options.requested_time
                                               : configuration.modes.front().time );
        std::cout << "SLAC UPDATE_TIME_STEP: " << update_time << std::endl;
        reader->GetOutputInformation( vtkSLACReader::VOLUME_OUTPUT )
            ->Set( vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), update_time );
        reader->Update( vtkSLACReader::VOLUME_OUTPUT );

        vtkDataObject* volume = reader->GetOutputDataObject( vtkSLACReader::VOLUME_OUTPUT );
        if ( !vtkCompositeDataSet::SafeDownCast( volume ) && !vtkDataSet::SafeDownCast( volume ) )
            throw std::runtime_error( "vtkSLACReader returned an unsupported volume type" );
        if ( !SharedPointData( volume ) )
            throw std::runtime_error( "vtkSLACReader returned no internal volume" );
        LogMemoryEstimate( volume, options.slac_sampling_dimensions );

        vtkNew<vtkResampleToImage> resampler;
        resampler->SetInputDataObject( volume );
        resampler->UseInputBoundsOn();
        resampler->SetSamplingDimensions( options.slac_sampling_dimensions[0],
                                          options.slac_sampling_dimensions[1],
                                          options.slac_sampling_dimensions[2] );
        resampler->Update();
        vtkImageData* image = resampler->GetOutput();
        ValidateAndLogImage( image, resampler->GetMaskArrayName(),
                             options.slac_sampling_dimensions );
        image->GetPointData()->RemoveArray( resampler->GetMaskArrayName() );
        image->GetPointData()->RemoveArray( vtkDataSetAttributes::GhostArrayName() );
        image->GetCellData()->RemoveArray( vtkDataSetAttributes::GhostArrayName() );
        if ( image->GetPointData()->HasArray( resampler->GetMaskArrayName() ) ||
             image->GetPointData()->HasArray( vtkDataSetAttributes::GhostArrayName() ) )
            throw std::runtime_error( "SLAC resample auxiliary arrays could not be removed" );
        return std::make_shared<cvt::VtkXmlImageData>( image );
    }

    bool timeSteps( const std::string& filename, const cvt::NetcdfReadOptions& options,
                    std::vector<double>& time_steps, std::string& error ) const override
    {
        Configuration configuration;
        if ( !ResolveConfiguration( filename, options, configuration, error ) ) return false;
        time_steps.clear();
        if ( configuration.kind == ModeKind::Frequency )
        {
            time_steps.push_back( 0.0 );
        }
        else
        {
            for ( const auto& mode : configuration.modes ) time_steps.push_back( mode.time );
        }
        return true;
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
private:
    enum class Centering
    {
        Cell,
        Point
    };

    struct PhysicalVariable
    {
        std::string name;
        Centering centering = Centering::Cell;
        bool time_dependent = false;
    };

    struct Configuration
    {
        std::size_t time_count = 0;
        std::size_t cell_count = 0;
        std::size_t vertex_count = 0;
        std::size_t level_count = 0;
        std::size_t vertex_degree = 0;
        std::vector<PhysicalVariable> physical_variables;
    };

    static bool IsStructuralVariable( const std::string& name )
    {
        static const std::vector<std::string> names = {
            "xCell",          "yCell",          "zCell",
            "latCell",        "lonCell",        "indexToCellID",
            "xVertex",        "yVertex",        "zVertex",
            "latVertex",      "lonVertex",      "indexToVertexID",
            "xEdge",          "yEdge",          "zEdge",
            "latEdge",        "lonEdge",        "indexToEdgeID",
            "cellsOnVertex",  "verticesOnCell", "cellsOnCell",
            "edgesOnCell",    "nEdgesOnCell",   "cellsOnEdge",
            "verticesOnEdge", "edgesOnVertex",  "edgeSignOnCell",
            "edgeSignOnVertex", "kiteAreasOnVertex", "weightsOnEdge",
            "areaCell",       "areaTriangle",   "dvEdge",
            "dcEdge",         "angleEdge",      "meshDensity",
            "refBottomDepth", "refLayerThickness", "refZMid",
            "maxLevelCell",   "zgrid"
        };
        return std::find( names.begin(), names.end(), name ) != names.end();
    }

    class File
    {
    private:
        int m_id = -1;

    public:
        explicit File( const std::string& filename )
        {
            const int status = nc_open( filename.c_str(), NC_NOWRITE, &m_id );
            if ( status != NC_NOERR )
            {
                throw std::runtime_error( std::string( "failed to open MPAS input: " ) +
                                          nc_strerror( status ) + ": " + filename );
            }
        }
        ~File()
        {
            if ( m_id >= 0 ) nc_close( m_id );
        }
        File( const File& ) = delete;
        File& operator=( const File& ) = delete;
        int id() const { return m_id; }
    };

    static std::string TrimAttribute( std::string value )
    {
        const auto is_padding = []( unsigned char c ) {
            return c == '\0' || std::isspace( c ) != 0;
        };
        while ( !value.empty() && is_padding( value.front() ) ) value.erase( value.begin() );
        while ( !value.empty() && is_padding( value.back() ) ) value.pop_back();
        return value;
    }

    static std::size_t CheckedProduct( std::size_t lhs, std::size_t rhs,
                                       const std::string& description )
    {
        if ( lhs != 0 && rhs > std::numeric_limits<std::size_t>::max() / lhs )
            throw std::runtime_error( "MPAS " + description + " overflows size_t" );
        const std::size_t result = lhs * rhs;
        if ( result > static_cast<std::size_t>( std::numeric_limits<vtkIdType>::max() ) )
            throw std::runtime_error( "MPAS " + description + " exceeds vtkIdType" );
        return result;
    }

    static double LayerThickness() { return 100000.0; }

    static void CheckStatus( int status, const std::string& operation )
    {
        if ( status != NC_NOERR )
            throw std::runtime_error( "MPAS " + operation + ": " + nc_strerror( status ) );
    }

    static int VariableId( int file, const std::string& name )
    {
        int variable = -1;
        CheckStatus( nc_inq_varid( file, name.c_str(), &variable ),
                     "failed to locate variable " + name );
        return variable;
    }

    static std::vector<double> NumericAttribute( int file, int variable,
                                                 const char* name,
                                                 const std::string& variable_name )
    {
        nc_type type = NC_NAT;
        std::size_t length = 0;
        const int inquiry = nc_inq_att( file, variable, name, &type, &length );
        if ( inquiry == NC_ENOTATT ) return {};
        CheckStatus( inquiry, "failed to inspect " + variable_name + ":" + name );
        if ( !IsNumericNetcdfType( static_cast<int>( type ) ) )
            throw std::runtime_error( "MPAS " + variable_name + ":" + name +
                                      " must be numeric" );
        std::vector<double> values( length );
        if ( length > 0 )
            CheckStatus( nc_get_att_double( file, variable, name, values.data() ),
                         "failed to read " + variable_name + ":" + name );
        return values;
    }

    static void ValidateNumericValues( int file, int variable,
                                       const std::string& variable_name,
                                       const std::vector<double>& values )
    {
        const auto fill = NumericAttribute( file, variable, "_FillValue", variable_name );
        const auto missing =
            NumericAttribute( file, variable, "missing_value", variable_name );
        for ( std::size_t i = 0; i < values.size(); ++i )
        {
            const double value = values[i];
            if ( !std::isfinite( value ) )
                throw std::runtime_error( "MPAS variable " + variable_name +
                                          " contains NaN or Inf at value " +
                                          std::to_string( i ) );
            if ( std::find( fill.begin(), fill.end(), value ) != fill.end() ||
                 std::find( missing.begin(), missing.end(), value ) != missing.end() )
                throw std::runtime_error( "MPAS variable " + variable_name +
                                          " contains a fill or missing value at value " +
                                          std::to_string( i ) );
        }
    }

    static std::vector<double> ReadDoubleVariable( int file, const std::string& name,
                                                   std::size_t count )
    {
        const int variable = VariableId( file, name );
        std::vector<double> values( count );
        if ( count > 0 )
            CheckStatus( nc_get_var_double( file, variable, values.data() ),
                         "failed to read variable " + name );
        ValidateNumericValues( file, variable, name, values );
        return values;
    }

    static std::vector<double> ReadPhysicalVariable( int file,
                                                     const PhysicalVariable& variable,
                                                     const Configuration& configuration,
                                                     std::size_t time_index )
    {
        const std::size_t horizontal_count =
            variable.centering == Centering::Cell ? configuration.vertex_count
                                                  : configuration.cell_count;
        const std::size_t value_count =
            CheckedProduct( horizontal_count, configuration.level_count,
                            "physical value count" );
        const int variable_id = VariableId( file, variable.name );
        std::vector<double> values( value_count );
        if ( variable.time_dependent )
        {
            const std::size_t start[] = { time_index, 0, 0 };
            const std::size_t count[] = { 1, horizontal_count,
                                          configuration.level_count };
            CheckStatus( nc_get_vara_double( file, variable_id, start, count, values.data() ),
                         "failed to read Time record from " + variable.name );
        }
        else
        {
            CheckStatus( nc_get_var_double( file, variable_id, values.data() ),
                         "failed to read variable " + variable.name );
        }
        ValidateNumericValues( file, variable_id, variable.name, values );
        return values;
    }

    static bool ResolveConfiguration( const std::string& filename,
                                      Configuration& configuration, std::string& error )
    {
        configuration = Configuration{};
        error.clear();
        try
        {
            cvt::NetcdfMetadata metadata;
            if ( !cvt::Netcdf::ReadMetadata( filename, metadata ) )
                throw std::runtime_error( "failed to inspect the MPAS input" );

            const std::string sphere =
                Lowercase( TrimAttribute( metadata.globalAttribute( "on_a_sphere" ) ) );
            if ( sphere != "no" )
                throw std::runtime_error(
                    "only planar MPAS input with on_a_sphere=\"NO\" is supported" );

            if ( !ReadNetcdfDimensionLength( filename, "Time", configuration.time_count ) ||
                 configuration.time_count == 0 )
                throw std::runtime_error( "MPAS requires Time > 0" );
            if ( !ReadNetcdfDimensionLength( filename, "nCells", configuration.cell_count ) ||
                 configuration.cell_count == 0 )
                throw std::runtime_error( "MPAS requires nCells > 0" );
            if ( !ReadNetcdfDimensionLength(
                     filename, "nVertices", configuration.vertex_count ) ||
                 configuration.vertex_count == 0 )
                throw std::runtime_error( "MPAS requires nVertices > 0" );
            if ( !ReadNetcdfDimensionLength(
                     filename, "nVertLevels", configuration.level_count ) ||
                 configuration.level_count == 0 )
                throw std::runtime_error( "MPAS requires nVertLevels > 0" );
            if ( !ReadNetcdfDimensionLength(
                     filename, "vertexDegree", configuration.vertex_degree ) ||
                 ( configuration.vertex_degree != 3 && configuration.vertex_degree != 4 ) )
                throw std::runtime_error( "MPAS vertexDegree must be 3 or 4" );

            const auto require_variable = [&]( const char* name, const char* dimensions,
                                               bool integer ) {
                if ( !metadata.hasVariable( name, dimensions ) )
                    throw std::runtime_error( std::string( "MPAS requires " ) + name +
                                              dimensions );
                const int type = metadata.variableType( name );
                if ( integer )
                {
                    if ( type != NC_BYTE && type != NC_SHORT && type != NC_INT &&
                         type != NC_UBYTE && type != NC_USHORT && type != NC_UINT &&
                         type != NC_INT64 && type != NC_UINT64 )
                        throw std::runtime_error( std::string( "MPAS variable " ) + name +
                                                  " must have an integer type" );
                }
                else if ( !IsNumericNetcdfType( type ) )
                {
                    throw std::runtime_error( std::string( "MPAS variable " ) + name +
                                              " must have a numeric type" );
                }
            };
            require_variable( "xCell", "(nCells)", false );
            require_variable( "yCell", "(nCells)", false );
            require_variable( "zCell", "(nCells)", false );
            require_variable( "cellsOnVertex", "(nVertices, vertexDegree)", true );

            for ( const auto& entry : metadata.variableDimensions() )
            {
                if ( IsStructuralVariable( entry.first ) ) continue;
                if ( !IsNumericNetcdfType( metadata.variableType( entry.first ) ) ) continue;
                PhysicalVariable variable;
                variable.name = entry.first;
                if ( entry.second == "(Time, nVertices, nVertLevels)" )
                {
                    variable.centering = Centering::Cell;
                    variable.time_dependent = true;
                }
                else if ( entry.second == "(nVertices, nVertLevels)" )
                {
                    variable.centering = Centering::Cell;
                }
                else if ( entry.second == "(Time, nCells, nVertLevels)" )
                {
                    variable.centering = Centering::Point;
                    variable.time_dependent = true;
                }
                else if ( entry.second == "(nCells, nVertLevels)" )
                {
                    variable.centering = Centering::Point;
                }
                else
                {
                    continue;
                }
                configuration.physical_variables.push_back( variable );
            }
            if ( configuration.physical_variables.empty() )
                throw std::runtime_error(
                    "MPAS has no compatible numeric physical variables" );
            std::sort( configuration.physical_variables.begin(),
                       configuration.physical_variables.end(),
                       []( const PhysicalVariable& lhs, const PhysicalVariable& rhs ) {
                           return lhs.name < rhs.name;
                       } );

            if ( configuration.level_count == std::numeric_limits<std::size_t>::max() )
                throw std::runtime_error( "MPAS nVertLevels is too large" );
            CheckedProduct( configuration.cell_count, configuration.level_count + 1,
                            "point count" );
            CheckedProduct( configuration.vertex_count, configuration.level_count,
                            "cell count" );
            return true;
        }
        catch ( const std::exception& exception )
        {
            error = exception.what();
            return false;
        }
    }

    static std::size_t ResolveTimeIndex( const cvt::NetcdfReadOptions& options,
                                         const Configuration& configuration )
    {
        const double requested = options.has_requested_time ? options.requested_time : 0.0;
        if ( !std::isfinite( requested ) || requested < 0.0 || std::floor( requested ) != requested ||
             requested > static_cast<double>( std::numeric_limits<std::size_t>::max() ) )
            throw std::runtime_error(
                "MPAS requested_time must be a finite, non-negative integer record index" );
        const std::size_t index = static_cast<std::size_t>( requested );
        if ( index >= configuration.time_count )
            throw std::runtime_error( "MPAS requested_time is outside the Time dimension" );
        return index;
    }

    static vtkSmartPointer<vtkUnstructuredGrid> BuildGrid(
        const std::string& filename, const Configuration& configuration,
        std::size_t time_index )
    {
        File file( filename );
        const auto x = ReadDoubleVariable( file.id(), "xCell", configuration.cell_count );
        const auto y = ReadDoubleVariable( file.id(), "yCell", configuration.cell_count );
        const auto z = ReadDoubleVariable( file.id(), "zCell", configuration.cell_count );
        (void)z;

        const std::size_t connection_count =
            CheckedProduct( configuration.vertex_count, configuration.vertex_degree,
                            "connection count" );
        const int connection_variable = VariableId( file.id(), "cellsOnVertex" );
        std::vector<long long> one_based_connections( connection_count );
        CheckStatus( nc_get_var_longlong( file.id(), connection_variable,
                                          one_based_connections.data() ),
                     "failed to read cellsOnVertex" );
        const auto connection_fill =
            NumericAttribute( file.id(), connection_variable, "_FillValue", "cellsOnVertex" );
        const auto connection_missing = NumericAttribute(
            file.id(), connection_variable, "missing_value", "cellsOnVertex" );

        std::vector<vtkIdType> connections( connection_count );
        std::vector<bool> used( configuration.cell_count, false );
        for ( std::size_t vertex = 0; vertex < configuration.vertex_count; ++vertex )
        {
            for ( std::size_t corner = 0; corner < configuration.vertex_degree; ++corner )
            {
                const std::size_t offset = vertex * configuration.vertex_degree + corner;
                const long long one_based = one_based_connections[offset];
                const double numeric_connection = static_cast<double>( one_based );
                if ( std::find( connection_fill.begin(), connection_fill.end(),
                                numeric_connection ) != connection_fill.end() ||
                     std::find( connection_missing.begin(), connection_missing.end(),
                                numeric_connection ) != connection_missing.end() )
                    throw std::runtime_error(
                        "MPAS cellsOnVertex contains a fill or missing value" );
                if ( one_based < 1 ||
                     static_cast<unsigned long long>( one_based ) > configuration.cell_count )
                    throw std::runtime_error(
                        "MPAS cellsOnVertex contains an index outside 1..nCells" );
                const vtkIdType index = static_cast<vtkIdType>( one_based - 1 );
                for ( std::size_t previous = 0; previous < corner; ++previous )
                    if ( connections[vertex * configuration.vertex_degree + previous] == index )
                        throw std::runtime_error(
                            "MPAS cellsOnVertex contains a repeated node in one cell" );
                connections[offset] = index;
                used[static_cast<std::size_t>( index )] = true;
            }

            double twice_area = 0.0;
            double scale = 1.0;
            const auto origin = static_cast<std::size_t>(
                connections[vertex * configuration.vertex_degree] );
            for ( std::size_t corner = 0; corner < configuration.vertex_degree; ++corner )
            {
                const auto a = static_cast<std::size_t>(
                    connections[vertex * configuration.vertex_degree + corner] );
                const auto b = static_cast<std::size_t>( connections[
                    vertex * configuration.vertex_degree +
                    ( corner + 1 ) % configuration.vertex_degree] );
                const double ax = x[a] - x[origin];
                const double ay = y[a] - y[origin];
                const double bx = x[b] - x[origin];
                const double by = y[b] - y[origin];
                twice_area += ax * by - bx * ay;
                scale = std::max( scale, std::max( std::abs( ax ), std::abs( ay ) ) );
            }
            const double tolerance = std::numeric_limits<double>::epsilon() * scale * scale *
                                     static_cast<double>( configuration.vertex_degree ) * 16.0;
            if ( !std::isfinite( twice_area ) || std::abs( twice_area ) <= tolerance )
                throw std::runtime_error( "MPAS cellsOnVertex defines a degenerate cell" );
        }
        if ( std::find( used.begin(), used.end(), false ) != used.end() )
            throw std::runtime_error( "MPAS cellsOnVertex leaves one or more points unused" );

        const std::size_t point_count =
            CheckedProduct( configuration.cell_count, configuration.level_count + 1,
                            "point count" );
        vtkNew<vtkPoints> points;
        points->SetDataTypeToDouble();
        points->SetNumberOfPoints( static_cast<vtkIdType>( point_count ) );
        for ( std::size_t horizontal = 0; horizontal < configuration.cell_count; ++horizontal )
        {
            for ( std::size_t level = 0; level <= configuration.level_count; ++level )
            {
                const vtkIdType id = static_cast<vtkIdType>(
                    horizontal * ( configuration.level_count + 1 ) + level );
                points->SetPoint( id, x[horizontal], y[horizontal],
                                  -LayerThickness() * static_cast<double>( level ) );
            }
        }

        vtkNew<vtkUnstructuredGrid> grid;
        grid->SetPoints( points );
        const std::size_t generated_cell_count =
            CheckedProduct( configuration.vertex_count, configuration.level_count,
                            "cell count" );
        grid->Allocate( static_cast<vtkIdType>( generated_cell_count ) );
        for ( std::size_t vertex = 0; vertex < configuration.vertex_count; ++vertex )
        {
            for ( std::size_t level = 0; level < configuration.level_count; ++level )
            {
                vtkIdType ids[8] = {};
                for ( std::size_t corner = 0; corner < configuration.vertex_degree; ++corner )
                {
                    const vtkIdType horizontal =
                        connections[vertex * configuration.vertex_degree + corner];
                    ids[corner] = horizontal *
                                      static_cast<vtkIdType>( configuration.level_count + 1 ) +
                                  static_cast<vtkIdType>( level );
                    ids[corner + configuration.vertex_degree] = ids[corner] + 1;
                }
                grid->InsertNextCell( configuration.vertex_degree == 3 ? VTK_WEDGE
                                                                       : VTK_HEXAHEDRON,
                                      static_cast<vtkIdType>( configuration.vertex_degree * 2 ),
                                      ids );
            }
        }

        for ( const auto& variable : configuration.physical_variables )
        {
            const auto values =
                ReadPhysicalVariable( file.id(), variable, configuration, time_index );
            vtkNew<vtkDoubleArray> array;
            array->SetName( variable.name.c_str() );
            array->SetNumberOfComponents( 1 );
            if ( variable.centering == Centering::Cell )
            {
                array->SetNumberOfTuples( static_cast<vtkIdType>( generated_cell_count ) );
                for ( std::size_t vertex = 0; vertex < configuration.vertex_count; ++vertex )
                    for ( std::size_t level = 0; level < configuration.level_count; ++level )
                        array->SetValue(
                            static_cast<vtkIdType>( vertex * configuration.level_count + level ),
                            values[vertex * configuration.level_count + level] );
                grid->GetCellData()->AddArray( array );
            }
            else
            {
                array->SetNumberOfTuples( static_cast<vtkIdType>( point_count ) );
                for ( std::size_t horizontal = 0; horizontal < configuration.cell_count;
                      ++horizontal )
                    for ( std::size_t level = 0; level <= configuration.level_count; ++level )
                    {
                        const std::size_t source_level =
                            std::min( level, configuration.level_count - 1 );
                        array->SetValue(
                            static_cast<vtkIdType>(
                                horizontal * ( configuration.level_count + 1 ) + level ),
                            values[horizontal * configuration.level_count + source_level] );
                    }
                grid->GetPointData()->AddArray( array );
            }
        }

        vtkNew<vtkCellDataToPointData> cell_to_point;
        cell_to_point->SetInputData( grid );
        cell_to_point->PassCellDataOff();
        cell_to_point->Update();
        auto* centered = vtkUnstructuredGrid::SafeDownCast( cell_to_point->GetOutput() );
        if ( !centered )
            throw std::runtime_error( "MPAS cell-to-point conversion returned no grid" );

        vtkNew<vtkUnstructuredGrid> normalized;
        normalized->ShallowCopy( centered );
        normalized->GetPointData()->Initialize();
        normalized->GetCellData()->Initialize();
        for ( const auto& variable : configuration.physical_variables )
        {
            vtkDataArray* array = centered->GetPointData()->GetArray( variable.name.c_str() );
            if ( !array || array->GetNumberOfComponents() != 1 ||
                 array->GetNumberOfTuples() != static_cast<vtkIdType>( point_count ) )
                throw std::runtime_error( "MPAS point-centered array is inconsistent: " +
                                          variable.name );
            for ( vtkIdType tuple = 0; tuple < array->GetNumberOfTuples(); ++tuple )
                if ( !std::isfinite( array->GetComponent( tuple, 0 ) ) )
                    throw std::runtime_error( "MPAS point-centered array contains NaN or Inf: " +
                                              variable.name );
            normalized->GetPointData()->AddArray( array );
        }

        if ( normalized->GetNumberOfPoints() != static_cast<vtkIdType>( point_count ) ||
             normalized->GetNumberOfCells() !=
                 static_cast<vtkIdType>( generated_cell_count ) )
            throw std::runtime_error( "MPAS generated grid has inconsistent point/cell counts" );
        const int expected_cell_type =
            configuration.vertex_degree == 3 ? VTK_WEDGE : VTK_HEXAHEDRON;
        for ( vtkIdType cell = 0; cell < normalized->GetNumberOfCells(); ++cell )
            if ( normalized->GetCellType( cell ) != expected_cell_type )
                throw std::runtime_error( "MPAS generated grid has an unexpected cell type" );
        return normalized.GetPointer();
    }

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
        return this->read( filename, cvt::NetcdfReadOptions{} );
    }
    std::shared_ptr<kvs::FileFormatBase> read(
        const std::string& filename, const cvt::NetcdfReadOptions& options ) const override
    {
        Configuration configuration;
        std::string error;
        if ( !ResolveConfiguration( filename, configuration, error ) )
            throw std::runtime_error( error );
        const std::size_t time_index = ResolveTimeIndex( options, configuration );
        auto grid = BuildGrid( filename, configuration, time_index );
        return std::make_shared<cvt::VtkXmlUnstructuredGrid>( grid.GetPointer() );
    }
    bool timeSteps( const std::string& filename, const cvt::NetcdfReadOptions&,
                    std::vector<double>& time_steps, std::string& error ) const override
    {
        Configuration configuration;
        if ( !ResolveConfiguration( filename, configuration, error ) ) return false;
        time_steps.resize( configuration.time_count );
        for ( std::size_t i = 0; i < configuration.time_count; ++i )
            time_steps[i] = static_cast<double>( i );
        return true;
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

int cvt::NetcdfMetadata::variableType( const std::string& name ) const
{
    const auto found = m_variable_types.find( name );
    return found == m_variable_types.end() ? NC_NAT : found->second;
}

const std::vector<std::size_t>&
cvt::NetcdfMetadata::variableShape( const std::string& name ) const
{
    static const std::vector<std::size_t> empty;
    const auto found = m_variable_shapes.find( name );
    return found == m_variable_shapes.end() ? empty : found->second;
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
    metadata.m_variable_types.clear();
    metadata.m_variable_shapes.clear();
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
        metadata.m_variable_types.emplace( name, static_cast<int>( type ) );
        auto& shape = metadata.m_variable_shapes[name];
        shape.reserve( static_cast<std::size_t>( rank ) );
        for ( int j = 0; j < rank; ++j )
        {
            std::size_t length = 0;
            error = nc_inq_dimlen( file, dimension_ids[j], &length );
            if ( error != NC_NOERR ) return fail( "variable dimension length", error );
            shape.push_back( length );
        }

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
    if ( cvt::detail::IsSlacMesh( metadata ) )
    {
        info.input_role = cvt::NetcdfInputRole::SlacMesh;
    }
    else if ( cvt::detail::IsSlacMode( metadata ) )
    {
        info.input_role = cvt::NetcdfInputRole::SlacMode;
    }
    else if ( cvt::detail::IsCamPoints( metadata ) )
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
