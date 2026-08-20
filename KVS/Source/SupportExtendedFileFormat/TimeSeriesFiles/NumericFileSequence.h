/*
 * Copyright (c) 2026 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 */
#ifndef EXTENDED_FILE_FORMAT__NUMERIC_FILE_SEQUENCE_H_INCLUDE
#define EXTENDED_FILE_FORMAT__NUMERIC_FILE_SEQUENCE_H_INCLUDE

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include <vtkGlobFileNames.h>
#include <vtkNew.h>
#include <vtkSortFileNames.h>
#include <vtkStringArray.h>

namespace kvs
{
namespace ExtendedFileFormat
{
namespace detail
{

struct NumericFilename
{
    // 数字以外の文字列と数字列を、ファイル名に現れる順序で保持する。
    std::vector<std::string> text_blocks;
    std::vector<std::string> number_blocks;
};

/**
 * パスからディレクトリ部分を除いたファイル名を取得する。
 *
 * \param[in] path ファイルパス。
 * \return パス区切り文字より後ろのファイル名。
 */
inline std::string NumericSequenceBasename( const std::string& path )
{
    const auto separator = path.find_last_of( "/\\" );
    return separator == std::string::npos ? path : path.substr( separator + 1 );
}

/**
 * 数字列の先頭の 0 を取り除き、数値比較用のキーに正規化する。
 *
 * \param[in] value 正規化する数字列。
 * \return 先頭の 0 を除いた数字列。値が 0 の場合は "0"。
 */
inline std::string NormalizeNumericSequenceKey( const std::string& value )
{
    const auto first_nonzero = value.find_first_not_of( '0' );
    return first_nonzero == std::string::npos ? "0" : value.substr( first_nonzero );
}

/**
 * ファイル名を数字以外の文字列と数字列のブロックに分解する。
 * 分解結果は、複数のファイルについて文字列部分が共通しているか、
 * どの数字部分が変化しているかを調べ、連番部分を特定するために使用する。
 *
 * \param[in] path 解析するファイルパス。
 * \return 分解したファイル名。
 */
inline NumericFilename ParseNumericFilename( const std::string& path )
{
    // ディレクトリ名に含まれる数字を連番候補に含めないよう、ファイル名だけを解析する。
    const std::string filename = NumericSequenceBasename( path );
    NumericFilename parsed;
    // 現在読み取っている同種文字の並びを一時的に保持する。
    std::string block;
    bool reading_number = false;

    for ( const char c : filename )
    {
        const bool is_number = c >= '0' && c <= '9';
        // 数字と数字以外が切り替わった時点で、直前のブロックを対応する配列へ格納する。
        if ( is_number != reading_number )
        {
            if ( reading_number ) parsed.number_blocks.push_back( block );
            else parsed.text_blocks.push_back( block );
            block.clear();
            reading_number = is_number;
        }
        block.push_back( c );
    }

    // ループ終了時に残った最後のブロックを格納する。
    if ( reading_number )
    {
        parsed.number_blocks.push_back( block );
        // 数字で終わる場合も text_blocks の個数を number_blocks より 1 個多く保つ。
        parsed.text_blocks.emplace_back();
    }
    else
    {
        parsed.text_blocks.push_back( block );
    }
    return parsed;
}

/**
 * 正規化済みの数字列を数値の昇順で比較する。
 *
 * 桁数を先に比較するため、整数型の表現範囲を超える数字列も比較できる。
 *
 * \param[in] lhs 左辺の数字列。
 * \param[in] rhs 右辺の数字列。
 * \return lhs が rhs より小さい場合は true。
 */
inline bool NumericSequenceKeyLess( const std::string& lhs, const std::string& rhs )
{
    return lhs.size() != rhs.size() ? lhs.size() < rhs.size() : lhs < rhs;
}

} // namespace detail

class NumericFileSequence
{
public:
    /**
     * ワイルドカードパターンに一致する連番ファイルを解決する。
     *
     * \param[in] pattern VTK が解釈できるファイル名パターン。
     */
    explicit NumericFileSequence( const std::string& pattern ) { this->resolve( pattern ); }

    /** \return 連番ファイルの解決に成功した場合は true。 */
    bool isSuccess() const { return m_error.empty(); }
    /** \return 連番ファイルの解決に失敗した場合は true。 */
    bool isFailure() const { return !this->isSuccess(); }
    /** \return 解決失敗時のエラーメッセージ。成功時は空文字列。 */
    const std::string& errorMessage() const { return m_error; }
    /** \return 連番の数値に基づいて昇順に並べたファイルパス。 */
    const std::vector<std::string>& filePaths() const { return m_file_paths; }
    /** \return 解決したファイル数。 */
    int numberOfFiles() const { return static_cast<int>( m_file_paths.size() ); }

private:
    /**
     * 解決結果を破棄し、エラー状態を設定する。
     *
     * \param[in] message 設定するエラーメッセージ。
     */
    void fail( const std::string& message )
    {
        m_file_paths.clear();
        m_error = message;
    }

    /**
     * パターンに一致するファイルを検証し、連番順に並べる。
     * ファイル名を文字列ブロックと数字ブロックに分解し、複数ファイル間で
     * 値が変化する数字ブロックを連番部分として特定する。連番値は先頭の 0 を
     * 除いて比較し、整数型へ変換せずに数値順へ並べる。
     * 構造が異なるファイル、連番部分を一意に決定できないファイル、または
     * 同じ連番値を持つファイルが含まれる場合はエラー状態を設定する。
     *
     * \param[in] pattern VTK が解釈できるファイル名パターン。
     */
    void resolve( const std::string& pattern )
    {
        // VTK の glob 機能でパターンに一致する候補を収集する。
        // サブディレクトリは再帰的に検索せず、ディレクトリ自体も候補から除外する。
        vtkNew<vtkGlobFileNames> glob;
        glob->RecurseOff();
        glob->AddFileNames( pattern.c_str() );

        // この段階では VTK の数値ソートを使わず、後段で特定した連番部分だけを基準に並べる。
        vtkNew<vtkSortFileNames> filter;
        filter->GroupingOff();
        filter->NumericSortOff();
        filter->IgnoreCaseOff();
        filter->SkipDirectoriesOn();
        filter->SetInputFileNames( glob->GetFileNames() );

        vtkStringArray* candidates = filter->GetFileNames();
        const int number_of_files = candidates->GetNumberOfValues();
        // 一致するファイルがなければ、空の結果ではなく解決失敗として扱う。
        if ( number_of_files == 0 )
        {
            this->fail( "No files matched numeric sequence pattern: " + pattern );
            return;
        }

        // VTK の文字列配列から、呼び出し側へ公開するファイルパス配列へコピーする。
        m_file_paths.reserve( number_of_files );
        for ( int i = 0; i < number_of_files; ++i )
        {
            m_file_paths.push_back( candidates->GetValue( i ) );
        }
        // 候補が 1 件だけの場合は比較対象がないため、そのファイルをそのまま採用する。
        if ( number_of_files == 1 ) return;

        // 各ファイル名を文字列ブロックと数字ブロックに分解する。
        std::vector<detail::NumericFilename> parsed_files;
        parsed_files.reserve( number_of_files );
        for ( const auto& path : m_file_paths )
        {
            parsed_files.push_back( detail::ParseNumericFilename( path ) );
        }

        // 先頭ファイルを基準に、数字ブロックの有無とファイル名構造の一致を検証する。
        const auto& first = parsed_files.front();
        if ( first.number_blocks.empty() )
        {
            this->fail( "Matched filenames contain no numeric sequence: " + pattern );
            return;
        }

        // 文字列ブロックの内容と数字ブロック数が異なるファイルは、同じ連番系列ではない。
        for ( std::size_t i = 1; i < parsed_files.size(); ++i )
        {
            if ( parsed_files[i].text_blocks != first.text_blocks ||
                 parsed_files[i].number_blocks.size() != first.number_blocks.size() )
            {
                this->fail( "Matched filenames do not share one numeric sequence pattern: " +
                            pattern );
                return;
            }
        }

        // 値が変化する数字ブロックを連番部分として特定する。
        // varying_blocks には正規化後の数値が変化する位置を記録する。
        // raw_varying_blocks には、"1" と "01" のように表記だけが変化する位置も記録する。
        std::vector<std::size_t> varying_blocks;
        std::vector<std::size_t> raw_varying_blocks;
        for ( std::size_t block = 0; block < first.number_blocks.size(); ++block )
        {
            const std::string first_raw = first.number_blocks[block];
            const std::string first_value = detail::NormalizeNumericSequenceKey( first_raw );
            bool raw_varies = false;
            bool value_varies = false;
            for ( std::size_t i = 1; i < parsed_files.size(); ++i )
            {
                const std::string& raw = parsed_files[i].number_blocks[block];
                raw_varies = raw_varies || raw != first_raw;
                value_varies = value_varies ||
                    detail::NormalizeNumericSequenceKey( raw ) != first_value;
            }
            if ( raw_varies ) raw_varying_blocks.push_back( block );
            if ( value_varies ) varying_blocks.push_back( block );
        }

        // 正規化後の値が変化する数字ブロックが 1 個だけなら、それを連番部分とする。
        std::size_t sequence_block = 0;
        if ( varying_blocks.size() == 1 )
        {
            sequence_block = varying_blocks.front();
        }
        else if ( varying_blocks.empty() && raw_varying_blocks.size() == 1 )
        {
            // 数値は同じだが 0 埋め表記が異なる場合、後段で連番値の重複として
            // 報告するため、比較対象の数字ブロックを選択する。
            sequence_block = raw_varying_blocks.front();
        }
        else if ( varying_blocks.empty() )
        {
            // どの数字ブロックにも変化がなく、連番部分を決定できない。
            this->fail( "Matched filenames contain no varying numeric sequence: " + pattern );
            return;
        }
        else
        {
            // 複数の数字ブロックが変化すると、どの位置が連番を表すか一意に決定できない。
            this->fail( "Matched filenames contain multiple varying numeric sequences: " +
                        pattern );
            return;
        }

        // 各ファイルについて、連番値の比較キーと元のパスを対応付ける。
        struct Entry
        {
            std::string key;
            std::string path;
        };
        std::vector<Entry> entries;
        entries.reserve( parsed_files.size() );
        for ( std::size_t i = 0; i < parsed_files.size(); ++i )
        {
            entries.push_back( { detail::NormalizeNumericSequenceKey(
                                     parsed_files[i].number_blocks[sequence_block] ),
                                 m_file_paths[i] } );
        }

        // 正規化した数字列を桁数と辞書順で比較し、整数型の上限に依存せず数値順に並べる。
        // キーが同じ場合は、結果を一定にするためファイルパスを第 2 の比較条件とする。
        std::sort( entries.begin(), entries.end(), []( const Entry& lhs, const Entry& rhs ) {
            if ( lhs.key != rhs.key ) return detail::NumericSequenceKeyLess( lhs.key, rhs.key );
            return lhs.path < rhs.path;
        } );

        // ソート後は同じキーが隣接するため、隣り合う要素だけを比較して重複を検出できる。
        // "1" と "01" のように 0 埋め表現が異なっても、同じ連番値なら重複として扱う。
        for ( std::size_t i = 1; i < entries.size(); ++i )
        {
            if ( entries[i - 1].key == entries[i].key )
            {
                this->fail( "Duplicate numeric sequence value " + entries[i].key + ": " +
                            entries[i - 1].path + " and " + entries[i].path );
                return;
            }
        }

        // 検証済みの数値順を最終的なファイルパス配列へ反映する。
        for ( std::size_t i = 0; i < entries.size(); ++i )
        {
            m_file_paths[i] = entries[i].path;
        }
    }

private:
    std::vector<std::string> m_file_paths;
    std::string m_error;
};

} // namespace ExtendedFileFormat
} // namespace kvs

#endif // EXTENDED_FILE_FORMAT__NUMERIC_FILE_SEQUENCE_H_INCLUDE
