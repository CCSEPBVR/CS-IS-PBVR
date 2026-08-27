/*
 * Copyright (c) 2026 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#ifndef CVT__SERIES_FILE_RESOLVER_H_INCLUDE
#define CVT__SERIES_FILE_RESOLVER_H_INCLUDE

#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>
#include <vector>

#include <vtkNew.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>

#include "FileFormat/NetCDF/Netcdf.h"
#include "Filesystem.h"
#include "NumericFileSequence.h"

namespace cvt
{

/** 時系列ファイルとして扱うことができるファイル形式。 */
enum class SeriesFormat
{
    Vti,
    Vtu,
    Vtm,
    Pvtu,
    Netcdf
};

/** 解決済みの時系列ファイル情報。 */
struct ResolvedSeries
{
    /// 時系列を構成するファイルの形式。
    SeriesFormat format = SeriesFormat::Vti;
    /// ファイル形式に対応する正規化済みの拡張子。
    std::string canonical_extension;
    /// 出力ファイル名に使用する基底名。
    std::string output_base;
    /// 時系列順に整列された入力ファイルパス。
    std::vector<std::string> file_paths;
};

namespace detail
{

/**
 * @brief 文字列に含まれる英大文字を英小文字へ変換する。
 *
 * @param[in] value 変換対象の文字列。
 * @return 英小文字へ変換した文字列。
 */
inline std::string LowercaseSeriesString( std::string value )
{
    std::transform( value.begin(), value.end(), value.begin(), []( const unsigned char c ) {
        return static_cast<char>( std::tolower( c ) );
    } );
    return value;
}

/**
 * @brief 拡張子から時系列ファイル形式を判定する。
 *
 * 拡張子の英大文字と英小文字は区別しない。
 *
 * @param[in] extension 判定対象の拡張子。
 * @param[out] format 判定したファイル形式。
 * @return 対応する拡張子の場合は true、それ以外の場合は false。
 */
inline bool SeriesFormatFromExtension( const std::string& extension, SeriesFormat& format )
{
    const std::string lower = LowercaseSeriesString( extension );
    if ( lower == ".vti" ) format = SeriesFormat::Vti;
    else if ( lower == ".vtu" ) format = SeriesFormat::Vtu;
    else if ( lower == ".vtm" ) format = SeriesFormat::Vtm;
    else if ( lower == ".pvtu" ) format = SeriesFormat::Pvtu;
    else if ( lower == ".nc" || lower == ".ncdf" ) format = SeriesFormat::Netcdf;
    else return false;
    return true;
}

/**
 * @brief ファイル先頭のシグネチャが NetCDF 形式かどうかを判定する。
 *
 * NetCDF Classic、64-bit Offset、CDF-5、および NetCDF-4 で使用される
 * HDF5 のシグネチャを判定対象とする。
 *
 * @param[in] filename 判定対象のファイルパス。
 * @return NetCDF 形式のシグネチャを持つ場合は true、それ以外の場合は false。
 */
inline bool HasNetcdfSignature( const std::string& filename )
{
    // NetCDF Classic 系と NetCDF-4 のどちらも判定できるように、ファイル先頭から
    // 最長のシグネチャである 8 バイトを読み込む。
    std::ifstream stream( filename, std::ios::binary );
    unsigned char signature[8] = {};
    stream.read( reinterpret_cast<char*>( signature ), sizeof( signature ) );
    // 8 バイト未満のファイルでは read が失敗状態になるため、実際に読み込めた
    // バイト数を使って、以降の配列要素を参照できるかどうかを確認する。
    const std::streamsize size = stream.gcount();

    // NetCDF Classic 系の先頭 3 バイトは共通して "CDF" であり、4 バイト目が
    // 1: Classic、2: 64-bit Offset、5: CDF-5 の各形式を表す。
    const bool classic = size >= 4 && signature[0] == 'C' && signature[1] == 'D' &&
                         signature[2] == 'F' &&
                         ( signature[3] == 1 || signature[3] == 2 || signature[3] == 5 );

    // NetCDF-4 は HDF5 を格納形式として使用するため、HDF5 固有の 8 バイトの
    // シグネチャと完全に一致するかどうかを調べる。
    const unsigned char hdf5[] = { 0x89, 'H', 'D', 'F', '\r', '\n', 0x1a, '\n' };
    const bool netcdf4 = size == 8 && std::equal( signature, signature + 8, hdf5 );

    // Classic 系または NetCDF-4 のどちらかと判定できれば NetCDF とみなす。
    return classic || netcdf4;
}

/**
 * @brief VTK XML ファイルのルート要素から時系列ファイル形式を判定する。
 *
 * @param[in] filename 判定対象のファイルパス。
 * @param[out] format 判定したファイル形式。
 * @return 対応する VTK XML 形式の場合は true、それ以外の場合は false。
 */
inline bool SeriesFormatFromVtkXml( const std::string& filename, SeriesFormat& format )
{
    vtkNew<vtkXMLDataParser> parser;
    parser->SetFileName( filename.c_str() );
    if ( parser->Parse() == 0 ) return false;

    vtkXMLDataElement* root = parser->GetRootElement();
    if ( !root || !root->GetName() || std::string( root->GetName() ) != "VTKFile" )
    {
        return false;
    }

    const char* type = root->GetAttribute( "type" );
    if ( !type ) return false;
    const std::string value( type );
    if ( value == "ImageData" ) format = SeriesFormat::Vti;
    else if ( value == "UnstructuredGrid" ) format = SeriesFormat::Vtu;
    else if ( value == "vtkMultiBlockDataSet" ) format = SeriesFormat::Vtm;
    else if ( value == "PUnstructuredGrid" ) format = SeriesFormat::Pvtu;
    else return false;
    return true;
}

/**
 * @brief 出力基底名の候補から区切り文字を除去する。
 *
 * 文字列の先頭と末尾にあるアンダースコア、ハイフン、およびピリオドを
 * 取り除く。
 *
 * @param[in] value 処理対象の文字列。
 * @return 先頭と末尾の区切り文字を除去した文字列。
 */
inline std::string TrimSeriesBaseSegment( std::string value )
{
    const auto separator = []( const char c ) { return c == '_' || c == '-' || c == '.'; };
    while ( !value.empty() && separator( value.front() ) ) value.erase( value.begin() );
    while ( !value.empty() && separator( value.back() ) ) value.pop_back();
    return value;
}

} // namespace detail

/**
 * @brief 時系列ファイル形式の表示名を取得する。
 *
 * @param[in] format ファイル形式。
 * @return ファイル形式の表示名。不明な値の場合は "unknown"。
 */
inline const char* SeriesFormatName( const SeriesFormat format )
{
    switch ( format )
    {
    case SeriesFormat::Vti: return "VTI";
    case SeriesFormat::Vtu: return "VTU";
    case SeriesFormat::Vtm: return "VTM";
    case SeriesFormat::Pvtu: return "PVTU";
    case SeriesFormat::Netcdf: return "NetCDF";
    }
    return "unknown";
}

/**
 * @brief 時系列ファイル形式に対応する正規化済みの拡張子を取得する。
 *
 * @param[in] format ファイル形式。
 * @return 先頭にピリオドを含む拡張子。不明な値の場合は空文字列。
 */
inline std::string CanonicalSeriesExtension( const SeriesFormat format )
{
    switch ( format )
    {
    case SeriesFormat::Vti: return ".vti";
    case SeriesFormat::Vtu: return ".vtu";
    case SeriesFormat::Vtm: return ".vtm";
    case SeriesFormat::Pvtu: return ".pvtu";
    case SeriesFormat::Netcdf: return ".nc";
    }
    return "";
}

/**
 * @brief ファイルから対応する時系列ファイル形式を検出する。
 *
 * 既知の拡張子を優先して判定し、拡張子から判定できない場合は NetCDF の
 * シグネチャと VTK XML の内容を調べる。
 *
 * @param[in] filename 検出対象のファイルパス。
 * @param[out] format 検出したファイル形式。
 * @param[out] error 検出に失敗した場合のエラーメッセージ。
 * @return 対応するファイル形式を検出できた場合は true、それ以外の場合は false。
 */
inline bool DetectSeriesFileFormat( const std::string& filename, SeriesFormat& format,
                                    std::string& error )
{
    const filesystem::path path( filename );
    if ( detail::SeriesFormatFromExtension( path.extension(), format ) ) return true;

    if ( detail::HasNetcdfSignature( filename ) )
    {
        NetcdfFileInfo info;
        if ( Netcdf::Probe( filename, info ) )
        {
            format = SeriesFormat::Netcdf;
            return true;
        }
        error = "The NetCDF file is not a supported data format: " + filename;
        return false;
    }

    if ( detail::SeriesFormatFromVtkXml( filename, format ) ) return true;

    error = "Could not detect a supported time-series format: " + filename;
    return false;
}

/**
 * @brief 時系列パターンから出力ファイル名の基底名を抽出する。
 *
 * パターンのファイル名部分から拡張子とワイルドカードを除き、最初に得られた
 * 空でない文字列を基底名として使用する。
 *
 * @param[in] pattern 時系列ファイルを指定するパターン。
 * @param[in] canonical_extension 対象形式の正規化済み拡張子。
 * @param[out] output_base 抽出した出力ファイル名の基底名。
 * @return 基底名を抽出できた場合は true、それ以外の場合は false。
 */
inline bool ExtractSeriesOutputBase( const std::string& pattern,
                                     const std::string& canonical_extension,
                                     std::string& output_base )
{
    // パターンからディレクトリ部分を除き、ファイル名部分だけを取り出す。
    std::string filename = filesystem::path( pattern ).filename().string();

    // 拡張子を大文字・小文字の違いに関係なく検索する。検索専用の小文字文字列を
    // 用意することで、出力基底名に使う元のファイル名の表記は維持する。
    const std::string lower_filename = detail::LowercaseSeriesString( filename );
    const auto extension_position = lower_filename.find( canonical_extension );
    // 拡張子が見つかった場合は、その開始位置から末尾までを元のファイル名から
    // 削除する。これにより、以降は拡張子を含まないパターンを処理できる。
    if ( extension_position != std::string::npos ) filename.erase( extension_position );

    // ファイル名をワイルドカード '*' で区切り、各区間を先頭から順に調べる。
    std::size_t begin = 0;
    while ( begin <= filename.size() )
    {
        // 次のワイルドカードまでを現在の区間とする。ワイルドカードがなければ、
        // std::string::npos を長さに指定して文字列の末尾までを切り出す。
        const auto wildcard = filename.find( '*', begin );
        const auto length = wildcard == std::string::npos ? std::string::npos : wildcard - begin;

        // 区間の先頭と末尾にある区切り文字（'_'、'-'、'.'）を取り除く。
        std::string segment = detail::TrimSeriesBaseSegment( filename.substr( begin, length ) );
        // 最初に見つかった空でない区間を出力ファイル名の基底名として採用する。
        if ( !segment.empty() )
        {
            output_base = segment;
            return true;
        }

        // ワイルドカードがなければ末尾まで調査済みなので終了する。見つかった場合は
        // その直後へ開始位置を進め、次の区間を調べる。
        if ( wildcard == std::string::npos ) break;
        begin = wildcard + 1;
    }

    // 基底名に使用できる区間がない場合は、呼び出し元へ古い値を返さないように
    // 出力を空にして失敗を通知する。
    output_base.clear();
    return false;
}

/**
 * @brief 数値を含む時系列パターンを解決し、ファイル構成を検証する。
 *
 * パターンに一致するファイルを時系列順に整列し、すべてのファイル形式が
 * 一致することを確認する。各形式固有の内容検証は変換処理側で行う。
 *
 * @param[in] pattern 解決対象の時系列ファイルパターン。
 * @param[out] resolved 解決した時系列ファイル情報。
 * @param[out] error 解決または検証に失敗した場合のエラーメッセージ。
 * @return 時系列を正常に解決できた場合は true、それ以外の場合は false。
 */
inline bool ResolveSeries( const std::string& pattern, ResolvedSeries& resolved,
                           std::string& error )
{
    // 途中で処理に失敗した場合に以前の結果を残さないよう、出力を初期化する。
    resolved = ResolvedSeries{};
    error.clear();

    // パターンに一致するファイルを検索し、ファイル名に含まれる数値の順に並べた
    // 時系列ファイル列を作成する。
    const NumericFileSequence sequence( pattern );
    if ( sequence.isFailure() )
    {
        error = sequence.errorMessage();
        return false;
    }

    // 先頭ファイルの形式を、以降のファイルと比較する基準として保持する。
    SeriesFormat expected_format = SeriesFormat::Vti;
    bool has_expected_format = false;

    // 時系列を構成する全ファイルを調べ、同じ形式のデータで統一されていることを
    // 確認する。
    for ( const auto& path : sequence.filePaths() )
    {
        // 拡張子、ファイルシグネチャ、または VTK XML の内容から形式を検出する。
        SeriesFormat actual_format;
        if ( !DetectSeriesFileFormat( path, actual_format, error ) ) return false;

        // 最初のファイルで基準形式を決定し、2番目以降では基準形式と比較する。
        if ( !has_expected_format )
        {
            expected_format = actual_format;
            has_expected_format = true;
        }
        else if ( actual_format != expected_format )
        {
            error = "Time series mixes file formats: expected " +
                    std::string( SeriesFormatName( expected_format ) ) + ", but " + path +
                    " was detected as " + SeriesFormatName( actual_format );
            return false;
        }
    }

    // 検証済みの形式と、その形式に対応する正規化済み拡張子を結果へ格納する。
    resolved.format = expected_format;
    resolved.canonical_extension = CanonicalSeriesExtension( expected_format );

    // 入力パターンから、変換後のファイル名に使用する基底名を抽出する。
    if ( !ExtractSeriesOutputBase( pattern, resolved.canonical_extension,
                                   resolved.output_base ) )
    {
        error = "Could not derive an output prefix from time-series pattern: " + pattern;
        return false;
    }

    // 検証と並べ替えが完了したファイルパスを結果へ格納し、成功を通知する。
    resolved.file_paths = sequence.filePaths();
    error.clear();
    return true;
}

} // namespace cvt

#endif // CVT__SERIES_FILE_RESOLVER_H_INCLUDE
