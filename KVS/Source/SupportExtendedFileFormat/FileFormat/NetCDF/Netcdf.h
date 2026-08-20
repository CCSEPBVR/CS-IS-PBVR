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
#ifndef EXTENDED_FILE_FORMAT_NETCDF_H_INCLUDE
#define EXTENDED_FILE_FORMAT_NETCDF_H_INCLUDE

#include <map>
#include <memory>
#include <string>

#include <kvs/FileFormatBase>

namespace kvs
{
namespace ExtendedFileFormat
{

enum class NetcdfGridType
{
    Unknown,
    ImageData,
    RectilinearGrid,
    StructuredGrid,
    UnstructuredGrid
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

public:
    /// NetCDFファイルを読み込む。
    bool read( const std::string& filename ) override;
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

private:
    /// NetCDFファイルから変数メタデータを読み込む。
    static bool ReadMetadata( const std::string& filename, NetcdfMetadata& metadata );
    /// メタデータに適合する形式アダプターを選択する。
    static const NetcdfFormatAdapter* SelectAdapter( const NetcdfMetadata& metadata );

private:
    std::shared_ptr<kvs::FileFormatBase> m_format;
    std::string m_format_name;
    NetcdfGridType m_grid_type = NetcdfGridType::Unknown;
};
} // namespace ExtendedFileFormat
} // namespace kvs

#endif // EXTENDED_FILE_FORMAT_NETCDF_H_INCLUDE
