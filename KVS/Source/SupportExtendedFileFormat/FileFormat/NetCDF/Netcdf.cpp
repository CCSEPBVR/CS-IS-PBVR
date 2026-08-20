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
#include "Netcdf.h"

#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <kvs/Message>
#include <kvs/Type>
#include <kvs/extendedfileformat/VtkXmlImageData>
#include <kvs/extendedfileformat/VtkXmlRectilinearGrid>
#include <kvs/extendedfileformat/VtkXmlStructuredGrid>
#include <kvs/extendedfileformat/VtkXmlUnstructuredGrid>
#include <vtkCallbackCommand.h>
#include <vtkCellType.h>
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkErrorCode.h>
#include <vtkFloatArray.h>
#include <vtkNetCDFCFReader.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkStringArray.h>
#include <vtkUnstructuredGrid.h>

namespace kvs
{
namespace ExtendedFileFormat
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
        diagnostics.errors.push_back( phase + ": point-data array " + name +
                                      " was not loaded" );
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

class GearnNetcdfFormatAdapter : public NetcdfFormatAdapter
{
public:
    /// 対応形式名を返す。
    const char* name() const override { return "GEARN"; }
    /// GEARNデータの変換先格子種別を返す。
    NetcdfGridType gridType() const override { return NetcdfGridType::UnstructuredGrid; }

    /**
     * @brief メタデータがGEARN形式に必要な変数構成を持つかを判定する。
     * @param metadata 判定対象の変数メタデータ。
     * @return GEARN形式の条件を満たす場合はtrue、それ以外はfalse。
     */
    bool matches( const NetcdfMetadata& metadata ) const override
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

        return std::make_shared<VtkXmlUnstructuredGrid>( normalized.GetPointer() );
    }

};

/**
 * @brief 利用可能なNetCDF形式アダプターの一覧を返す。
 * @return 登録済みアダプターの一覧。
 */
const std::vector<std::shared_ptr<NetcdfFormatAdapter>>& RegisteredNetcdfAdapters()
{
    static const std::vector<std::shared_ptr<NetcdfFormatAdapter>> adapters = {
        std::make_shared<GearnNetcdfFormatAdapter>()
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
                            NetcdfGridType grid_type )
{
    switch ( grid_type )
    {
    case NetcdfGridType::ImageData:
        return dynamic_cast<VtkXmlImageData*>( format.get() ) != nullptr;
    case NetcdfGridType::RectilinearGrid:
        return dynamic_cast<VtkXmlRectilinearGrid*>( format.get() ) != nullptr;
    case NetcdfGridType::StructuredGrid:
        return dynamic_cast<VtkXmlStructuredGrid*>( format.get() ) != nullptr;
    case NetcdfGridType::UnstructuredGrid:
        return dynamic_cast<VtkXmlUnstructuredGrid*>( format.get() ) != nullptr;
    case NetcdfGridType::Unknown:
    default:
        return false;
    }
}
} // namespace detail

/**
 * @brief 指定した名前と次元を持つ変数がメタデータに存在するかを判定する。
 */
bool NetcdfMetadata::hasVariable( const std::string& name, const std::string& dimensions ) const
{
    const auto found = m_variable_dimensions.find( name );
    return found != m_variable_dimensions.end() && found->second == dimensions;
}

/**
 * @brief NetCDFファイルから変数名と次元の対応を読み込む。
 * @param filename 入力ファイル名。
 * @param metadata 読み込んだメタデータの格納先。
 * @return 読み込みに成功した場合はtrue、それ以外はfalse。
 */
bool Netcdf::ReadMetadata( const std::string& filename, NetcdfMetadata& metadata )
{
    try
    {
        detail::NetcdfDiagnostics diagnostics;
        diagnostics.phase = "NetCDF format detection";
        vtkNew<vtkNetCDFCFReader> reader;
        vtkNew<vtkCallbackCommand> callback;
        detail::ObserveNetcdfReader( reader, callback, diagnostics );
        reader->SetFileName( filename.c_str() );
        if ( reader->UpdateMetaData() == 0 )
        {
            diagnostics.errors.push_back(
                diagnostics.phase + ": failed to read NetCDF metadata from " + filename );
        }
        detail::CheckNetcdfReaderError( reader, diagnostics.phase, diagnostics );
        detail::ThrowNetcdfDiagnostics( diagnostics );

        // 形式判別で再利用できるよう、全変数の名前と次元を保存する。
        vtkStringArray* dimensions = reader->GetVariableDimensions();
        metadata.m_variable_dimensions.clear();
        for ( int i = 0; i < reader->GetNumberOfVariableArrays(); ++i )
        {
            metadata.m_variable_dimensions.emplace( reader->GetVariableArrayName( i ),
                                                    dimensions->GetValue( i ) );
        }
        return true;
    }
    catch ( const std::exception& e )
    {
        kvsMessageError( e.what() );
        return false;
    }
}

/**
 * @brief メタデータに適合するNetCDF形式アダプターを一つ選択する。
 * @param metadata 判定対象の変数メタデータ。
 * @return 一意に選択できたアダプター。未対応または曖昧な場合はnullptr。
 */
const NetcdfFormatAdapter* Netcdf::SelectAdapter( const NetcdfMetadata& metadata )
{
    std::vector<const NetcdfFormatAdapter*> matches;
    for ( const auto& adapter : detail::RegisteredNetcdfAdapters() )
    {
        if ( adapter->matches( metadata ) )
        {
            matches.push_back( adapter.get() );
        }
    }

    if ( matches.empty() )
    {
        std::ostringstream message;
        message << "Unsupported NetCDF data format; variables:";
        for ( const auto& variable : metadata.variableDimensions() )
        {
            message << " " << variable.first << variable.second;
        }
        kvsMessageError( message.str().c_str() );
        return nullptr;
    }
    if ( matches.size() > 1 )
    {
        std::ostringstream message;
        message << "Ambiguous NetCDF data format; matched adapters:";
        for ( const auto* adapter : matches )
        {
            message << " " << adapter->name();
        }
        kvsMessageError( message.str().c_str() );
        return nullptr;
    }
    return matches.front();
}

/**
 * @brief 指定したNetCDFファイルを読み込んでオブジェクトを初期化する。
 * @param filename 入力ファイル名。
 */
Netcdf::Netcdf( const std::string& filename ) { this->read( filename ); }

/**
 * @brief NetCDFファイルの形式を判別し、対応する格子データへ変換する。
 * @param filename 入力ファイル名。
 * @return 読み込みと変換に成功した場合はtrue、それ以外はfalse。
 */
bool Netcdf::read( const std::string& filename )
{
    // 前回の読み込み結果を破棄し、失敗状態から処理を開始する。
    this->setFilename( filename );
    this->setSuccess( false );
    m_format.reset();
    m_format_name.clear();
    m_grid_type = NetcdfGridType::Unknown;

    // メタデータに基づいて入力形式を判別する。
    NetcdfMetadata metadata;
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
        m_format = adapter->read( filename );
        if ( !m_format || !detail::MatchesNetcdfGridType( m_format, adapter->gridType() ) )
        {
            kvsMessageError( ( std::string( adapter->name() ) +
                               " NetCDF adapter returned an invalid VTK grid type for " +
                               filename )
                                 .c_str() );
            m_format.reset();
            return false;
        }
        m_format_name = adapter->name();
        m_grid_type = adapter->gridType();
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
bool Netcdf::write( const std::string& filename )
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
bool Netcdf::Probe( const std::string& filename, NetcdfFileInfo& info )
{
    NetcdfMetadata metadata;
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
    return true;
}
} // namespace ExtendedFileFormat
} // namespace kvs
