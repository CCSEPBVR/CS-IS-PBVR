#ifndef STRINGPROCESSOR_H
#define STRINGPROCESSOR_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>
#include <optional>

class StringProcessor
{
public:
    enum Format
    {
        Unknown             = 0,    // Aka Error

        // ServerSide
        ServerPointObject   = 1,    // Server side point object
        ServerGlyphObject   = 2,    // Server side point object

        // SideObject
        PointObjectKVSML    = 3,    // Point Object(.kvsml)
        PointObjectLAS      = 4,    // Point Object(.las)
        PointObjectPTS      = 5,    // Point Object(.pts)

        PolygonObjectKVSML  = 6,    // Nontexture Polygon Object(.kvsml)
        PolygonObjectSTL    = 7,    // Nontexture Polygon Object(.stl)
        PolygonObject3DS    = 8,    // Texture Polygon Object(.3ds)
        PolygonObjectFBX    = 9,    // Texture Polygon Object(.fbx)

        LineObjectKVSML     = 10,   // Line Object(.kvsml)
    };

    struct ObjectInfo
    {
        // Common Object Info
        std::string name;              // Base name (without extension)
        std::string directory;         // Directory path
        Format format;                 // Detected format
        std::pair<int,int> timeStep;   // Min/Max timestep

        // Common Server Point Object Info
        int particleLimit               = 10000000;
        float density                   = 1.0;

        // Client Server Point Object Info
        int numberOfVector;
        int numberOfElements;
        int numberOfSubvolume;
        int numberOfNodes;
        int elementType;
        int fileType;
        int stepNumber;
        std::string coordinateX         = "";
        std::string coordinateY         = "";
        std::string coordinateZ         = "";
        bool isExport                   = false;

        // Nontexture Polygon Object Info
        int rgb[3]                      = { 128, 128, 128 };
        float opacity                   = 0.5;
    };

    explicit StringProcessor( const std::string& filePath )
        : m_local_file_path( filePath )
    {
    }

    std::optional<ObjectInfo> extractFromLocalFile()
    {
        std::filesystem::path pathObject( m_local_file_path );

        ObjectInfo objectInfo;
        objectInfo.name      = pathObject.stem().string();
        objectInfo.directory = pathObject.parent_path().string();
        objectInfo.format    = extractFormat();
        objectInfo.timeStep  = extractTimeStep();

        // std::cout << "Name                  :" << objectInfo.name      << std::endl;
        // std::cout << "Directory             :" << objectInfo.directory << std::endl;
        // std::cout << "Format                :" << objectInfo.format    << std::endl;
        // std::cout << "TimeStep(min, max)    :" << objectInfo.timeStep.first << ", " << objectInfo.timeStep.second << std::endl;

        if( objectInfo.format == Format::Unknown ) return std::nullopt;
        if( objectInfo.timeStep.first == -1 || objectInfo.timeStep.second == -1 ) return std::nullopt;
        return objectInfo;
    }

    std::string formatToString( Format format ) const
    {
        switch( format )
        {
        case Unknown:
            return "Unknow";
        case ServerPointObject:
            return "ServerPointObject";
        case ServerGlyphObject:
            return "ServerGlyphObject";
        case PointObjectKVSML:
            return "PointObject(KVSML)";
        case PointObjectLAS:
            return "las";
        case PointObjectPTS:
            return "pts";
        case PolygonObjectKVSML:
            return "PolygonObject(KVSML)";
        case PolygonObjectSTL:
            return "stl";
        case PolygonObject3DS:
            return "3ds";
        case PolygonObjectFBX:
            return "fbx";
        case LineObjectKVSML:
            return "LineObject(KVSML)";
        default:
            return "Unknow";
        }
    }

private:
    std::string m_local_file_path;

    Format extractFormat()
    {
        std::filesystem::path pathObject( m_local_file_path );
        std::string extension = pathObject.extension().string(); // 拡張子を取得
        std::transform( extension.begin(), extension.end(), extension.begin(), []( unsigned char c ){ return std::tolower(c); } ); // 小文字化

        if( extension == ".kvsml" )
        {
            // 簡易 XML 判定（タグ検索）// FIXME: 簡易的なものなのでtinyxml2などを使用することを考慮されたほうがいいかと。
            std::ifstream file( m_local_file_path );
            if( file.is_open() )
            {
                std::string line;
                while( std::getline( file, line ) )
                {
                    // 空白削除
                    line.erase( std::remove_if(line.begin(), line.end(), ::isspace), line.end() );

                    if( line.find("<PointObject") != std::string::npos )
                    {
                        return Format::PointObjectKVSML;
                    }
                    else if( line.find("<PolygonObject") != std::string::npos )
                    {
                        return Format::PolygonObjectKVSML;
                    }
                    else if( line.find("<LineObject") != std::string::npos )
                    {
                        return Format::LineObjectKVSML;
                    }
                }
                // タグが見つからなかった場合
                return Format::Unknown;
            }
            else
            {
                std::cerr << "Failed to open file: " << m_local_file_path << std::endl;
                return Format::Unknown;
            }
        }
        else if( extension == ".las" ) return Format::PointObjectLAS;
        else if( extension == ".pts" ) return Format::PointObjectPTS;
        else if( extension == ".stl" ) return Format::PolygonObjectSTL;
        else if( extension == ".3ds" ) return Format::PolygonObject3DS;
        else if( extension == ".fbx" ) return Format::PolygonObjectFBX;

        // サーバ側で許可されている拡張子
        static const std::string serverExtensions[] =
            {
                ".pfi", ".pfl", ".stl", ".vtp", ".xyz", ".vtr",
                ".vtk", ".vti", ".vts", ".pvts", ".inp", ".vtu",
                ".pvtu", ".vtm", ".case"
            };

        for( const auto& e: serverExtensions )
        {
            if( extension == e ) return Format::ServerPointObject;
        }

        return Format::Unknown;
    }

    std::pair<int,int> extractTimeStep()
    {
        std::filesystem::path pathObject( m_local_file_path );
        std::string baseName = pathObject.stem().string();  // 拡張子除去

        auto underscorePosition = baseName.find('_');
        if( underscorePosition == std::string::npos )
        {
            std::cout << "No underscore found in filename: " << baseName << std::endl;
            return std::pair<int,int>( -1, -1 );
        }

        std::string prefix = baseName.substr( 0, underscorePosition );
        // std::cout << "Prefix detected: " << prefix << std::endl;

        // 正規表現: prefix_ + 5桁数字
        std::regex regex( "^" + prefix + "_(\\d{5})\\..*$" );

        int minTimeStep = std::numeric_limits<int>::max();
        int maxTimeStep = std::numeric_limits<int>::min();
        bool foundValid = false;

        std::filesystem::path dirPath = pathObject.parent_path();
        if( std::filesystem::exists( dirPath ) && std::filesystem::is_directory( dirPath ) )
        {
            for( const auto& entry : std::filesystem::directory_iterator( dirPath ) )
            {
                if( !entry.is_regular_file() ) continue;

                std::string fileName = entry.path().filename().string();
                std::smatch match;
                if( std::regex_match( fileName, match, regex ) )
                {
                    int value = std::stoi( match[1].str() );
                    // std::cout << "Found valid 5-digit timestep: " << value << " in file: " << fileName << std::endl;

                    minTimeStep = std::min( minTimeStep, value );
                    maxTimeStep = std::max( maxTimeStep, value );
                    foundValid = true;
                }
            }
        }

        if( foundValid )
        {
            return std::pair<int,int>( minTimeStep, maxTimeStep );
        }
        else
        {
            std::cerr << "No valid 5-digit timestep found in directory." << std::endl;
            return std::pair<int,int>( -1, -1 );
        }
    }
};

#endif // STRINGPROCESSOR_H
