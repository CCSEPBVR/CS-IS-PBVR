/*
 * Copyright (c) 2026 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 */
#ifndef CVT__SINGLE_FILE_FORMAT_H_INCLUDE
#define CVT__SINGLE_FILE_FORMAT_H_INCLUDE

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <string>
#include <vector>

#include "Filesystem.h"

namespace cvt
{

/** KVSMLConverter が単一ファイル入力として扱うファイル形式。 */
enum class ConverterInputFormat
{
    Stl,
    Vtp,
    Vtk,
    Vti,
    Vtr,
    Vts,
    Pvts,
    Inp,
    Netcdf,
    Vtu,
    Pvtu,
    Vtm,
    Case,
    Cgns,
    Xyz
};

/** 単一ファイル名から解決した形式と出力基底名。 */
struct SingleFileFormatInfo
{
    ConverterInputFormat format = ConverterInputFormat::Vtk;
    std::string canonical_extension;
    std::string output_base;
    bool embedded_extension = false;
};

/** 単一ファイル名の形式解決結果。 */
enum class SingleFileFormatResolution
{
    Success,
    NotFound,
    Invalid
};

namespace detail
{

struct SingleFileFormatSpec
{
    ConverterInputFormat format;
    const char* extension;
    bool example_cli;
    bool xml_single;
};

/**
 * @brief 単一ファイル入力として認識する形式の一覧を返す。
 *
 * 各形式について、正規拡張子とExample CLI／XML single入力での対応可否を保持する。
 *
 * @return 単一ファイル形式の定義一覧。
 */
inline const std::array<SingleFileFormatSpec, 15>& SingleFileFormatSpecs()
{
    static const std::array<SingleFileFormatSpec, 15> specs = { {
        { ConverterInputFormat::Pvtu, ".pvtu", true, true },
        { ConverterInputFormat::Pvts, ".pvts", true, true },
        { ConverterInputFormat::Cgns, ".cgns", false, true },
        { ConverterInputFormat::Case, ".case", true, true },
        { ConverterInputFormat::Vtp, ".vtp", true, false },
        { ConverterInputFormat::Vtk, ".vtk", true, true },
        { ConverterInputFormat::Vti, ".vti", true, true },
        { ConverterInputFormat::Vtr, ".vtr", false, true },
        { ConverterInputFormat::Vts, ".vts", true, true },
        { ConverterInputFormat::Vtu, ".vtu", true, true },
        { ConverterInputFormat::Vtm, ".vtm", true, true },
        { ConverterInputFormat::Stl, ".stl", true, true },
        { ConverterInputFormat::Inp, ".inp", true, true },
        { ConverterInputFormat::Xyz, ".xyz", false, true },
        { ConverterInputFormat::Netcdf, ".nc", true, true },
    } };
    return specs;
}

/**
 * @brief 文字列に含まれる英字を小文字へ変換する。
 *
 * ファイル名と拡張子を大文字・小文字を区別せず比較するために使用する。
 *
 * @param value 変換対象の文字列。
 * @return 英字を小文字へ変換した文字列。
 */
inline std::string LowercaseSingleFileString( std::string value )
{
    std::transform( value.begin(), value.end(), value.begin(), []( const unsigned char c ) {
        return static_cast<char>( std::tolower( c ) );
    } );
    return value;
}

/**
 * @brief 指定位置から文字列末尾までがASCII数字だけで構成されているか判定する。
 *
 * @param value 判定対象の文字列。
 * @param begin 判定を開始する文字位置。
 * @return 1文字以上のASCII数字だけで構成されている場合はtrue、それ以外はfalse。
 */
inline bool IsAsciiDigits( const std::string& value, const std::size_t begin )
{
    if ( begin >= value.size() ) return false;
    for ( std::size_t i = begin; i < value.size(); ++i )
    {
        if ( value[i] < '0' || value[i] > '9' ) return false;
    }
    return true;
}

/**
 * @brief 形式マーカーの後ろが許可された連番サフィックスか判定する。
 *
 * 許可する形式は".<digits>"または"s<digits>"とする。
 *
 * @param suffix 形式マーカーより後ろの文字列。
 * @return 許可された連番サフィックスの場合はtrue、それ以外はfalse。
 */
inline bool IsNumericFileSuffix( const std::string& suffix )
{
    return suffix.size() >= 2 && ( suffix[0] == '.' || suffix[0] == 's' ) &&
           IsAsciiDigits( suffix, 1 );
}

/**
 * @brief 列挙値に対応する単一ファイル形式の定義を検索する。
 *
 * @param format 検索する入力形式。
 * @return 対応する定義へのポインタ。未登録の場合はnullptr。
 */
inline const SingleFileFormatSpec* FindSingleFileFormatSpec(
    const ConverterInputFormat format )
{
    const auto& specs = SingleFileFormatSpecs();
    const auto found = std::find_if( specs.begin(), specs.end(), [&]( const auto& spec ) {
        return spec.format == format;
    } );
    return found == specs.end() ? nullptr : &( *found );
}

} // namespace detail

/**
 * @brief 入力形式に対応する正規拡張子を取得する。
 *
 * @param format 入力形式。
 * @return 正規拡張子。未登録の形式の場合は空文字列。
 */
inline const char* ConverterInputFormatExtension( const ConverterInputFormat format )
{
    const auto* spec = detail::FindSingleFileFormatSpec( format );
    return spec ? spec->extension : "";
}

/**
 * @brief 入力形式がExample CLIの単一ファイル入力に対応しているか判定する。
 *
 * @param format 判定する入力形式。
 * @return Example CLIで対応している場合はtrue、それ以外はfalse。
 */
inline bool IsExampleCliInputFormat( const ConverterInputFormat format )
{
    const auto* spec = detail::FindSingleFileFormatSpec( format );
    return spec && spec->example_cli;
}

/**
 * @brief 入力形式がXMLのsingle入力に対応しているか判定する。
 *
 * @param format 判定する入力形式。
 * @return XMLのsingle入力で対応している場合はtrue、それ以外はfalse。
 */
inline bool IsXmlSingleInputFormat( const ConverterInputFormat format )
{
    const auto* spec = detail::FindSingleFileFormatSpec( format );
    return spec && spec->xml_single;
}

/**
 * 単一ファイル名の正規拡張子または埋め込み形式マーカーから形式を解決する。
 *
 * 埋め込み形式マーカーより後ろは ".<digits>" または "s<digits>" のみ許可する。
 * ディレクトリ名に含まれる拡張子文字列を誤認しないよう、ファイル名部分だけを調べる。
 *
 * @param input_path 判定する入力ファイルのパス。
 * @param resolved 判定成功時に、入力形式、正規拡張子、出力基底名を格納する。
 * @param error 判定不能または不正な場合に、エラーメッセージを格納する。
 * @return 判定結果。成功、形式未検出、または不正入力のいずれかを返す。
 */
inline SingleFileFormatResolution ResolveSingleFileFormat( const std::string& input_path, SingleFileFormatInfo& resolved, std::string& error )
{
    resolved = SingleFileFormatInfo{};
    error.clear();

    const filesystem::path path( input_path );
    const std::string filename = path.filename().u8string();
    const std::string lower_filename = detail::LowercaseSingleFileString( filename );
    const std::string lower_extension = detail::LowercaseSingleFileString( path.extension() );

    // 従来の正規拡張子で終わるファイルは、既存どおり stem を出力基底名にする。
    for ( const auto& spec : detail::SingleFileFormatSpecs() )
    {
        if ( lower_extension == spec.extension )
        {
            resolved.format = spec.format;
            resolved.canonical_extension = spec.extension;
            resolved.output_base = path.stem().u8string();
            if ( resolved.output_base.empty() )
            {
                error = "Could not derive an output prefix from single input file: " + input_path;
                return SingleFileFormatResolution::Invalid;
            }
            return SingleFileFormatResolution::Success;
        }
    }

    // ファイル名から見つかった形式マーカーを候補として保持する。
    // positionには形式マーカーの開始位置を記録し、後続処理でその直前までを
    // 出力基底名として切り出すために使用する。
    struct Candidate
    {
        ConverterInputFormat format; // 検出した入力形式。
        std::string extension;        // 既存の変換分岐へ渡す正規拡張子。
        std::size_t position;         // ファイル名内で形式マーカーが始まる位置。
    };

    // ファイル名に複数の有効な形式マーカーが含まれる可能性があるため、
    // すべての候補を収集してから一意に決定できるか確認する。
    std::vector<Candidate> candidates;

    // 登録されている各正規拡張子について、ファイル名内の出現位置を検索する。
    // lower_filenameはファイル名部分だけを小文字化した値なので、ディレクトリ名を
    // 誤認せず、大文字・小文字を区別せずに形式マーカーを検索できる。
    for ( const auto& spec : detail::SingleFileFormatSpecs() )
    {
        // 対象の形式マーカーが最初に現れる位置を取得する。
        std::size_t position = lower_filename.find( spec.extension );

        // 同じ形式マーカーがファイル名内に複数含まれる場合も、すべての位置を調べる。
        while ( position != std::string::npos )
        {
            // 形式マーカーの直後からファイル名末尾までをサフィックスとして取り出す。
            // 例: "foo.vtu.10"の".vtu"を検出した場合、suffixは".10"となる。
            const std::size_t suffix_begin = position + std::string( spec.extension ).size();
            const std::string suffix = lower_filename.substr( suffix_begin );

            // サフィックス全体が".<ASCII数字>"または"s<ASCII数字>"の場合だけ、
            // 連番サフィックス付きの単一ファイル名として候補へ登録する。
            if ( detail::IsNumericFileSuffix( suffix ) )
            {
                candidates.push_back( { spec.format, spec.extension, position } );
            }

            // 現在の位置の次から同じ形式マーカーを再検索する。
            position = lower_filename.find( spec.extension, position + 1 );
        }
    }

    // 有効な形式マーカーがない場合は、呼び出し側で内容判定などを行えるよう未検出を返す。
    if ( candidates.empty() ) return SingleFileFormatResolution::NotFound;

    // 複数候補がある名前は誤った変換形式を選ばないよう不正入力として扱う。
    if ( candidates.size() != 1 )
    {
        error = "Could not uniquely determine the input format from single input file: " +
                input_path;
        return SingleFileFormatResolution::Invalid;
    }

    // 形式マーカーより前を出力基底名とし、連番サフィックスは出力名へ含めない。
    const auto& candidate = candidates.front();
    resolved.format = candidate.format;
    resolved.canonical_extension = candidate.extension;
    resolved.output_base = filename.substr( 0, candidate.position );
    resolved.embedded_extension = true;
    if ( resolved.output_base.empty() )
    {
        error = "Could not derive an output prefix from single input file: " + input_path;
        return SingleFileFormatResolution::Invalid;
    }
    return SingleFileFormatResolution::Success;
}

} // namespace cvt

#endif // CVT__SINGLE_FILE_FORMAT_H_INCLUDE
