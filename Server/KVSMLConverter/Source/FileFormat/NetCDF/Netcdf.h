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
#ifndef CVT_NETCDF_H_INCLUDE
#define CVT_NETCDF_H_INCLUDE

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "kvs/FileFormatBase"

namespace cvt
{

enum class NetcdfGridType
{
    Unknown,
    ImageData,
    RectilinearGrid,
    StructuredGrid,
    UnstructuredGrid,
    PolyData
};

enum class NetcdfInputRole
{
    Standard,
    CamPoints,
    CamConnectivity
};

struct NetcdfReadOptions
{
    std::string cam_connectivity_filename;
    bool has_requested_time = false;
    double requested_time = 0.0;
};

/**
 * @brief NetCDFファイルから取得した変数メタデータを保持するクラス。
 */
class NetcdfMetadata
{
public:
    /**
     * @brief 指定した名前と次元を持つ変数が存在するかを判定する。
     * @param name 変数名。
     * @param dimensions 次元を表す文字列。
     * @return 条件に一致する変数が存在する場合はtrue、それ以外はfalse。
     */
    bool hasVariable( const std::string& name, const std::string& dimensions ) const;

    /// 指定した名前の変数が存在するかを返す。
    bool hasVariable( const std::string& name ) const;

    /// 指定した名前の次元が存在するかを返す。
    bool hasDimension( const std::string& name ) const;

    /// 変数が持つ文字列属性を返す。存在しない場合は空文字列を返す。
    std::string variableAttribute( const std::string& variable,
                                   const std::string& attribute ) const;

    /// グローバル文字列属性を返す。存在しない場合は空文字列を返す。
    std::string globalAttribute( const std::string& attribute ) const;

    /**
     * @brief 変数名と次元文字列の対応表を返す。
     * @return 変数メタデータの連想配列。
     */
    const std::map<std::string, std::string>& variableDimensions() const
    {
        return m_variable_dimensions;
    }

private:
    std::map<std::string, std::string> m_variable_dimensions;
    std::map<std::string, std::size_t> m_dimensions;
    std::map<std::string, std::string> m_global_attributes;
    std::map<std::string, std::map<std::string, std::string>> m_variable_attributes;

    friend class Netcdf;
};

/**
 * @brief NetCDFファイルの判別結果を保持する構造体。
 */
struct NetcdfFileInfo
{
    std::string path;
    std::string format_name;
    NetcdfGridType grid_type = NetcdfGridType::Unknown;
    NetcdfInputRole input_role = NetcdfInputRole::Standard;
};

/**
 * @brief SLACメッシュに対応する1時刻分のフィールドファイル情報。
 */
struct SlacTimeStepFile
{
    std::string path;
    double time = 0.0;
};

/**
 * @brief 個別のNetCDFデータ形式を読み込むためのアダプターインターフェース。
 */
class NetcdfFormatAdapter
{
public:
    virtual ~NetcdfFormatAdapter() = default;

    /// 対応するデータ形式名を返す。
    virtual const char* name() const = 0;
    /// 読み込み後に生成される格子種別を返す。
    virtual NetcdfGridType gridType() const = 0;
    /// メタデータが対応形式の条件を満たすかを判定する。
    virtual bool matches( const NetcdfMetadata& metadata ) const = 0;
    /// NetCDFファイルを読み込み、対応するKVSファイル形式オブジェクトを返す。
    virtual std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const = 0;
    virtual std::shared_ptr<kvs::FileFormatBase> read(
        const std::string& filename, const NetcdfReadOptions& options ) const
    {
        (void)options;
        return this->read( filename );
    }
    virtual bool timeSteps( const std::string&, const NetcdfReadOptions&,
                            std::vector<double>&, std::string& error ) const
    {
        error = "This NetCDF format does not expose internal time steps";
        return false;
    }
};

/**
 * @brief NetCDFファイルを判別し、対応する形式へ変換して保持するクラス。
 */
class Netcdf : public kvs::FileFormatBase
{
public:
    Netcdf() = default;

    /**
     * @brief 指定したNetCDFファイルを読み込む。
     * @param filename 入力ファイル名。
     */
    explicit Netcdf( const std::string& filename );
    Netcdf( const std::string& filename, const NetcdfReadOptions& options );

public:
    /// NetCDFファイルを読み込む。
    bool read( const std::string& filename ) override;
    bool read( const std::string& filename, const NetcdfReadOptions& options );
    /// NetCDFファイルへ書き出す（未実装）。
    bool write( const std::string& filename ) override;

    /// 変換後のファイル形式オブジェクトを返す。
    const std::shared_ptr<kvs::FileFormatBase>& format() const { return m_format; }
    /// 判別されたデータ形式名を返す。
    const std::string& formatName() const { return m_format_name; }
    /// 判別された格子種別を返す。
    NetcdfGridType gridType() const { return m_grid_type; }

    /**
     * @brief ファイルを読み込まずにNetCDFデータ形式を判別する。
     * @param filename 判別対象のファイル名。
     * @param info 判別結果の格納先。
     * @return 判別に成功した場合はtrue、それ以外はfalse。
     */
    static bool Probe( const std::string& filename, NetcdfFileInfo& info );

    /// NetCDFファイルから形式判定用メタデータを読み込む。
    static bool ReadMetadata( const std::string& filename, NetcdfMetadata& metadata );

    static bool TimeSteps( const std::string& filename, const NetcdfReadOptions& options,
                           std::vector<double>& time_steps, std::string& error );

    /**
     * @brief .ncdfメッシュと同じディレクトリの.mod時系列を検証して時刻順に返す。
     * @param mesh_filename 選択されたSLACメッシュ（.ncdf）。
     * @param time_steps 検証済みの.modファイルと物理時刻。
     * @param error 失敗理由。
     * @return SLACの時系列入力として利用できる場合はtrue。
     */
    static bool ResolveSlacTimeSeries( const std::string& mesh_filename,
                                       std::vector<SlacTimeStepFile>& time_steps,
                                       std::string& error );

private:
    /// メタデータに適合する形式アダプターを選択する。
    static const NetcdfFormatAdapter* SelectAdapter( const NetcdfMetadata& metadata );

private:
    std::shared_ptr<kvs::FileFormatBase> m_format;
    std::string m_format_name;
    NetcdfGridType m_grid_type = NetcdfGridType::Unknown;
};
} // namespace cvt

#endif // CVT_NETCDF_H_INCLUDE
