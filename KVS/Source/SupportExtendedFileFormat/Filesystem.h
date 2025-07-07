/*
 * Copyright (c) 2022 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
 #ifndef EXTENDED_FILE_FORMAT__FILESYSTEM_H_INCLUDE
 #define EXTENDED_FILE_FORMAT__FILESYSTEM_H_INCLUDE
 
 #include <string>
 #include <cerrno>
 #include <cstdio>
 
 #ifdef _WIN32
 #include <direct.h>
 #include <sys/stat.h>
 #else
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <unistd.h>
 #endif
 
 namespace kvs
 {
 namespace ExtendedFileFormat
 {
     class filesystem
     {
 public:
         class path
         {
 public:
             path() = default;
             path( const std::string& p ) : _path( p ) {}
             path( const char* p ) : _path( p ) {}
 
             path& operator=( const std::string& p )
             {
                 _path = p;
                 return *this;
             }
 
             // 末尾にスラッシュで結合
             path& operator/=( const std::string& other )
             {
                 if ( !_path.empty() && _path.back() != '/' && _path.back() != '\\' )
                 {
 #ifdef _WIN32
                     _path += '\\';
 #else
                     _path += '/';
 #endif
                 }
                 _path += other;
                 return *this;
             }
 
             // 末尾にそのまま連結
             path& operator+=(const std::string& other)
             {
                 _path += other;
                 return *this;
             }
 
             friend bool operator==( const filesystem::path& lhs, const filesystem::path& rhs );
 
             // パスの区切り文字をプラットフォームに応じた区切り文字に変換する
             void make_preferred() {
 #ifdef _WIN32
                 for (char& c : _path) {
                     if (c == '/') c = '\\';
                 }
 #else
                 for (char& c : _path) {
                     if (c == '\\') c = '/';
                 }
 #endif
             }
 
             // ファイル名を返す
             filesystem::path filename() const
             {
                 size_t pos = _path.find_last_of("/\\");
                 if ( pos == std::string::npos )
                 {
                     return _path; // 区切りがなければ全体がファイル名
                 }
                 return filesystem::path(_path.substr( pos + 1 ));
             }
 
             // ファイルの拡張子を取得する
             std::string extension() const
             {
                 size_t pos = _path.find_last_of("/\\");
                 size_t dot = _path.find_last_of('.');
                 if (dot == std::string::npos || (pos != std::string::npos && dot < pos)) {
                     return ""; // 拡張子なし
                 }
                 return _path.substr(dot);
             }
 
             // 親ディレクトリのパスを返す
             filesystem::path parent_path() const
             {
                 size_t pos = _path.find_last_of("/\\");
                 if (pos == std::string::npos) return filesystem::path("");
                 return filesystem::path(_path.substr(0, pos));
             }
 
             // ファイル名から拡張子を除いたファイル名を出力する
             filesystem::path stem() const
             {
                 size_t slash = _path.find_last_of("/\\");
                 size_t dot = _path.find_last_of('.');
                 size_t start = (slash == std::string::npos) ? 0 : slash + 1;
             
                 if (dot == std::string::npos || dot < start) {
                     // 拡張子なし
                     return _path.substr(start);
                 }
                 return filesystem::path(_path.substr(start, dot - start));
             }
 
             // 文字列として出力する
             const std::string& string() const
             {
                 return _path;
             }
 
             // ファイルパスの区切り文字を環境に依存しない文字列に変換する
             std::string generic_string() const
             {
                 std::string tmp_string = _path;
 #ifdef _WIN32
                 for (char& c : tmp_string) {
                     if (c == '\\') c = '/';
                 }
 #endif
                 return tmp_string;
             }
     
             // std::stringに変換する
             operator std::string() const
             {
                 return _path;
             }
 
             // UTF-8前提でそのまま返す
             std::string u8string() const {
                 return _path;
             }
 
             // プラットフォームごとに推奨されるパスの区切り文字を返す
             static constexpr char preferred_separator =
 #ifdef _WIN32
                             '\\';
 #else
                             '/';
 #endif
     
 private:
             std::string _path;      
         };
 
         static bool exists( const std::string& path )
         {
 #ifdef _WIN32
             struct _stat buffer;
             return _stat(path.c_str(), &buffer) == 0;
 #else
             struct stat buffer;
             return stat(path.c_str(), &buffer) == 0;
 #endif
         }
 
         static bool create_directories( const std::string& path )
         {
             size_t pos = 0;
             std::string current;
 
             while ( ( pos = path.find_first_of( "/\\", pos ) ) != std::string::npos )
             {
                 current = path.substr( 0, pos++ );
                 if ( !current.empty() && !exists( current ) ) {
 #ifdef _WIN32
                 if ( _mkdir( current.c_str() ) != 0 && errno != EEXIST )
                     return false;
 #else
                 if ( mkdir(current.c_str(), 0755) != 0 && errno != EEXIST )
                     return false;
 #endif
                 }
             }
 
             if ( !exists( path ) )
             {
 #ifdef _WIN32
                 if ( _mkdir( path.c_str() ) != 0 && errno != EEXIST )
                     return false;
 #else
                 if ( mkdir( path.c_str(), 0755 ) != 0 && errno != EEXIST )
                     return false;
 #endif
             }
 
             return true;
         }
 
         static bool rename( const std::string& old_name, const std::string& new_name )
         {
             return std::rename( old_name.c_str(), new_name.c_str() ) == 0;
         }
     
         static bool remove( const std::string& path )
         {
             return std::remove( path.c_str() ) == 0;
         }
 
         static std::string convert_to_unix_path( const std::string& path )
         {
 #ifdef _WIN32
             std::string result = path;
             for (char& c : result) {
                 if (c == '\\') c = '/';
             }
             return result;
 #else
             return path;
 #endif
         }
     };
 
     inline bool operator==( const filesystem::path& lhs, const filesystem::path& rhs )
     {
         return lhs.string() == rhs.string();
     }
 } // namespace ExtendedFileFormat
 } // namespace kvs
 
 
 // Since std::filesystem cannot be used in Fugaku, a custom implementation is provided.
 /*
 
 #pragma GCC warning                                                                                \
     "This header is for gcc < 8. This must be replaced with <filesystem> in the future"
 
 #if __has_include( <filesystem>)
 #include <filesystem>
 #else
 #include <experimental/filesystem>
 namespace std
 {
 namespace filesystem = std::experimental::filesystem;
 } // namespace std
 #endif
 */
 
 #endif // EXTENDED_FILE_FORMAT__FILESYSTEM_H_INCLUDE