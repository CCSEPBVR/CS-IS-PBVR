#ifndef SERVERUTILS_H
#define SERVERUTILS_H

#include <string>
#include <stdexcept>
#include <filesystem>
#include <codecvt>

// "qN" (例: "q1") -> (N - 1) を返す
// 例: "q1" -> 0, "q2" -> 1
inline int toVariableIndex( const std::string& varName )
{
    // "q" で始まるかチェック
    if( varName.size() < 2 || varName[0] != 'q' )
    {
        throw std::invalid_argument( "Invalid variable name format: " + varName );
    }

    // "q" の後ろを数値として読む
    int n = 0;
    try
    {
        n = std::stoi( varName.substr( 1 ) );
    }
    catch(...)
    {
        throw std::invalid_argument( "Invalid number in variable name: " + varName );
    }

    // q0 や負数は想定外
    if( n <= 0 )
    {
        throw std::invalid_argument( "Variable index must be >= 1: " + varName );
    }

    return n - 1;
}

inline std::string toUtf8( const std::filesystem::path& p )
{
#ifdef _WIN32
    std::wstring ws = p.wstring();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes( ws );
#else
    return p.string();
#endif
}

#endif // SERVERUTILS_H
