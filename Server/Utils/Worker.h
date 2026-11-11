#ifndef WORKER_H
#define WORKER_H

#include <sstream>
#include <iomanip>
#include <sys/stat.h>   // メンバー粒子のステップ欠落検出（stat）
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>

#include "../../Shared/ObjectInfoExtractor.h"

#include "ObjectInfoUtils.h"
#include "ServerMode.h"
#include "TaskSignalTransferProtocol.h"

#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/PlotOverLineProperty>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/GenerateParticle>
#include <vismodule/GenerateGlyph>
#include <vismodule/Camera>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/Calculate>
#include <vismodule/ParticleMonitor>

class Worker
{
public:
    using DoneCallBack = std::function<void()>;

    explicit Worker(
        const int requestTimeStep, std::vector<ObjectInfoExtractor::ObjectInfo>* objects,
        kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords,
        const ServerMode server_mode, ParticleProperty* particle_property, GlyphProperty* glyph_property,
        PlotOverLineProperty* pol_property, MultiVolumePropertyList* multi_volume_property_list,
        const std::string& statistic = "normal"
        )
        : m_request_time_step( requestTimeStep ) , m_objects( objects )
        , m_result_min_object_coords( resultMinObjectCoords )
        , m_result_max_object_coords( resultMaxObjectCoords )
        , m_server_mode(server_mode), m_particle_property( particle_property ), m_glyph_property( glyph_property )
        , m_pol_property( pol_property ), m_multi_volume_property_list( multi_volume_property_list )
        , m_statistic( statistic )
    {}

    void setDoneCallBack( DoneCallBack callBack ) { m_done_call_back = std::move( callBack ); }

    void process()
    {
        for( auto& info : *m_objects )
        {
            // 表示しないなら、保持している object を確実に破棄
            if( !info.isDisplay )
            {
                this->resetObject( info );
                continue;
            }

            // リクエストタイムステップに対する実際の読み込みタイムステップを決定
            int resultTimeStep = -1;

            // info.timeStep.first ～ info.timeStep.second の範囲内に m_request_time_step が含まれる場合
            if( m_request_time_step >= info.timeStep.first && m_request_time_step <= info.timeStep.second )
            {
                resultTimeStep = m_request_time_step;
            }
            else if( m_request_time_step < info.timeStep.first )
            {
                if( info.isKeepInitial ) resultTimeStep = info.timeStep.first;
            }
            else // m_request_time_step > info.timeStep.second
            {
                if( info.isKeepFinal ) resultTimeStep = info.timeStep.second;
            }

            // 範囲外で保持もしないなら破棄
            if( resultTimeStep == -1 )
            {
                this->resetObject( info );
                continue;
            }

            // 既に同じ timestep の object を持っていて、置換不要なら「保持」して何もしない
            if( resultTimeStep == info.currentImportedTimeStep && info.object != nullptr )
            {
                if( info.needSameTimeStepReplace == false )
                {
                    continue; // ← ここが重要（以前は nullptr にして消していた）
                }
            }

            // ここに来たら「読み直す/置換する」ので、古い object は必ず破棄
            this->resetObject( info );

            // 新規 import
            this->importObject( info, resultTimeStep );
            info.currentImportedTimeStep = resultTimeStep;
        }

        if( m_done_call_back ) m_done_call_back();
    }

private:
    int m_request_time_step;
    kvs::Vec3 m_result_min_object_coords;
    kvs::Vec3 m_result_max_object_coords;
    std::vector<ObjectInfoExtractor::ObjectInfo>* m_objects = nullptr;
    DoneCallBack m_done_call_back;

    ServerMode m_server_mode;
    ParticleProperty* m_particle_property = nullptr;
    GlyphProperty* m_glyph_property = nullptr;
    PlotOverLineProperty* m_pol_property = nullptr;
    MultiVolumePropertyList* m_multi_volume_property_list = nullptr;
    std::string m_statistic;

private:
    void resetObject( ObjectInfoExtractor::ObjectInfo& info )
    {
        ServerObjectUtils::DestroyObject( info );
    }

    void importObject( ObjectInfoExtractor::ObjectInfo& info, const int& requestTimeStep )
    {
        std::string fileName;
        if( info.format == ObjectInfoExtractor::Format::ClientServerPointObject ) fileName = toNativePath( info.directory );
        else fileName = createFileName( info, requestTimeStep );

        std::unique_ptr<kvs::PointObject> pointObject;
        std::unique_ptr<kvs::PolygonObject> polygonObject;
        std::unique_ptr<kvs::LineObject> lineObject;

#ifdef ASSIMP
        std::unique_ptr<kvs::TexturedPolygonObject> texturedPolygonObject;
#endif

        switch( info.format )
        {
        case ObjectInfoExtractor::ClientServerPointObject:
            SendGenerateParticleSignal( fileName, requestTimeStep );
            pointObject = std::make_unique<kvs::PointObject>();
            GenerateParticleCS( fileName, requestTimeStep, *m_particle_property, *m_multi_volume_property_list, pointObject );
            pointObject->setMinMaxObjectCoords( m_result_min_object_coords, m_result_max_object_coords );
            pointObject->setMinMaxExternalCoords( m_result_min_object_coords, m_result_max_object_coords );
            info.object = pointObject.release();
            break;

        case ObjectInfoExtractor::InsituServerPointObject:
            pointObject = std::make_unique<kvs::PointObject>();
            if ( m_statistic == "normal" )
            {
                GenerateParticleIS( requestTimeStep, *m_particle_property, *m_multi_volume_property_list, pointObject );
            }
            else
            {
                if ( !GenerateStatisticParticleIS( requestTimeStep, m_statistic, pointObject ) )
                {
                    std::cout << "[Worker] statistic particle is not available: " << m_statistic << std::endl;
                }
            }
            pointObject->setMinMaxObjectCoords( m_result_min_object_coords, m_result_max_object_coords );
            pointObject->setMinMaxExternalCoords( m_result_min_object_coords, m_result_max_object_coords );
            info.object = pointObject.release();
            break;

        case ObjectInfoExtractor::ServerGlyphObject:
            if( m_server_mode == ServerMode::CS )
            {
                SendGenerateGlyphSignal( fileName, requestTimeStep );
                Calculate_minmax_glyph( requestTimeStep, *m_glyph_property, *m_multi_volume_property_list );
                polygonObject = GenerateGlyphCS( fileName, requestTimeStep, *m_glyph_property, *m_multi_volume_property_list );
            }
            else // ServerMode::IS
            {
                polygonObject = GenerateGlyphIS( requestTimeStep, *m_glyph_property, *m_multi_volume_property_list );
            }

            polygonObject->setMinMaxObjectCoords( m_result_min_object_coords, m_result_max_object_coords );
            polygonObject->setMinMaxExternalCoords( m_result_min_object_coords, m_result_max_object_coords );
            info.object = polygonObject.release();
            break;

        case ObjectInfoExtractor::PointObjectKVSML:
        case ObjectInfoExtractor::PointObjectLAS:
        case ObjectInfoExtractor::PointObjectPTS:
            pointObject = std::make_unique<kvs::PointImporter>( fileName );
            pointObject->setMinMaxObjectCoords( m_result_min_object_coords, m_result_max_object_coords );
            pointObject->setMinMaxExternalCoords( m_result_min_object_coords, m_result_max_object_coords );
            info.object = pointObject.release();
            break;

        case ObjectInfoExtractor::PolygonObjectKVSML:
        case ObjectInfoExtractor::PolygonObjectSTL:
            polygonObject = std::make_unique<kvs::PolygonImporter>( fileName );
            polygonObject->setMinMaxObjectCoords( m_result_min_object_coords, m_result_max_object_coords );
            polygonObject->setMinMaxExternalCoords( m_result_min_object_coords, m_result_max_object_coords );
            polygonObject->setColor( kvs::RGBColor( info.polygonColor ) );
            polygonObject->setOpacity( info.polygonOpacity * 255 );
            info.object = polygonObject.release();
            break;

#ifdef ASSIMP
        case ObjectInfoExtractor::PolygonObject3DS:
        case ObjectInfoExtractor::PolygonObjectFBX:
            texturedPolygonObject = std::make_unique<kvs::TexturedPolygonImporter>( fileName );
            texturedPolygonObject->setMinMaxObjectCoords( m_result_min_object_coords, m_result_max_object_coords );
            texturedPolygonObject->setMinMaxExternalCoords( m_result_min_object_coords, m_result_max_object_coords );
            info.object = texturedPolygonObject.release();
            break;
#endif

        case ObjectInfoExtractor::LineObjectKVSML:
            lineObject = std::make_unique<kvs::LineImporter>( fileName );
            lineObject->setMinMaxObjectCoords( m_result_min_object_coords, m_result_max_object_coords );
            lineObject->setMinMaxExternalCoords( m_result_min_object_coords, m_result_max_object_coords );
            info.object = lineObject.release();
            break;

        default:
            break;
        }
    }

    std::string createFileName( const ObjectInfoExtractor::ObjectInfo& info, const int& requestTimeStep )
    {
#ifdef _WIN32
        const char path_sep = '\\';
#else
        const char path_sep = '/';
#endif
        std::ostringstream fullPath;
        fullPath << info.directory << path_sep
            << info.name << "_"
            << std::setw(5) << std::setfill('0') << requestTimeStep
            << info.extension;

        return fullPath.str();
    }

    /*  統計量・メンバーごとの読み出しパスを決める。
     *
     *  統計量は PARTICLE_DIR 直下（サブディレクトリなし、接頭辞 ave/var/cov）。
     *  メンバーは PARTICLE_DIR/ensNNNN/ の下で、接頭辞は通常版と同じ "t"。
     *  メンバーの区別をディレクトリ名で行うのは、1メンバー1ステップで4ファイル出るため
     *  直下に並べるとファイル数が実用限界を超えること、および ParticleFile の名前解析が
     *  接頭辞の長さに依存しており "t" のままなら無改造で使えることによる。 */
    /*  メンバー粒子の要求ステップのファイルが揃っているかを確認する。
     *
     *  ParticleFile::generatePointObject の while( !read_success ) は
     *  「in-situ 側が書き込み中のファイルはいずれ現れる」前提の sleep なしリトライで、
     *  永遠に現れないファイル（出力OFFで欠落したステップ）だと抜けられない。
     *  この読み出しは WebSocket ハンドラ内で同期実行されるため、固まるのは
     *  1リクエストではなく uWS のイベントループごと＝サーバー全体がハングする。
     *  ParticleMonitor::stepExisted() は state.txt しか見ないので、メンバー個別・
     *  ステップ個別の欠落は素通りする。そこで読み出し前にここで弾く。
     *
     *  メンバー粒子は連番・総数が常に 1/1 なのでファイル名が一意に決まる。
     *  kvsml 本体と 3つの .dat がすべて存在し、かつサイズが 0 でないことを確認する
     *  （書きかけ・writer 死亡でも同じループに捕まるため）。
     *
     *  particlePrefix は "<PARTICLE_DIR>/ensNNNN/t" の形。 */
    static bool memberStepFilesReady( const std::string& particlePrefix, const int timeStep )
    {
        std::ostringstream base;
        base << particlePrefix << "_"
             << std::setfill( '0' ) << std::setw( 5 ) << timeStep
             << "_0000001_0000001";

        static const char* suffixes[] = { ".kvsml", "_coord.dat", "_color.dat", "_normal.dat" };
        for ( const char* suffix : suffixes )
        {
            const std::string path = base.str() + suffix;
            struct stat st;
            if ( stat( path.c_str(), &st ) != 0 || st.st_size == 0 )
            {
                std::cout << "[Worker][StatisticParticle] step file not ready: " << path << std::endl;
                return false;
            }
        }
        return true;
    }

    static bool resolveParticlePath( const std::string& statistic,
                                     std::string* subdir, std::string* prefix )
    {
        if ( subdir == nullptr || prefix == nullptr ) return false;
        if ( statistic == "mean" )     { *subdir = ""; *prefix = "ave"; return true; }
        if ( statistic == "variance" ) { *subdir = ""; *prefix = "var"; return true; }
        if ( statistic == "cv" )       { *subdir = ""; *prefix = "cov"; return true; }

        // "member:3" -> subdir "ens0003" / prefix "t"
        static const char member_tag[] = "member:";
        if ( statistic.rfind( member_tag, 0 ) == 0 )
        {
            const int id = std::atoi( statistic.c_str() + ( sizeof( member_tag ) - 1 ) );
            if ( id < 0 ) return false;
            std::ostringstream os;
            os << "ens" << std::setfill( '0' ) << std::setw( 4 ) << id;
            *subdir = os.str();
            /*  接頭辞は "t"（アンダースコアを含まない）。
             *  ParticleFile は file_length = 接頭辞長 + 1 + ... として
             *  「接頭辞の直後にアンダースコア1つ」を前提にファイル名長を検査する
             *  （ParticleFile.cpp:32）。通常版の既定も "t" である
             *  （ParticleMonitor.cpp:32 の particlePath += "/t"）。
             *  "t_" を渡すと長さ判定が外れ、ファイルが見つからない。 */
            *prefix = "t";
            return true;
        }

        *subdir = "";
        *prefix = "";
        return false;
    }

    static std::string particleDirectoryPrefix( const std::string& prefix,
                                                const std::string& subdir = "" )
    {
        const char* envBuf = std::getenv( "PARTICLE_DIR" );
        std::string particlePath;
        if ( envBuf == nullptr )
        {
            particlePath = subdir.empty() ? ( "./" + prefix )
                                          : ( "./" + subdir + "/" + prefix );
        }
        else
        {
            particlePath = envBuf;
#ifdef _WIN32
            const char path_sep = '\\';
#else
            const char path_sep = '/';
#endif
            if ( particlePath.empty() || particlePath[particlePath.size() - 1] != path_sep ) particlePath += path_sep;
            if ( !subdir.empty() )
            {
                particlePath += subdir;
                particlePath += path_sep;
            }
            particlePath += prefix;
        }
        return particlePath;
    }

    // FIXME:実装する場所に不適
    static bool GenerateStatisticParticleIS(
        const int timeStep,
        const std::string& statistic,
        std::unique_ptr<kvs::PointObject>& pointObject )
    {
        std::string subdir;
        std::string prefix;
        if ( !resolveParticlePath( statistic, &subdir, &prefix ) || prefix.empty() )
        {
            std::cout << "[Worker][StatisticParticle] unknown statistic=" << statistic << std::endl;
            pointObject->setCoords( kvs::ValueArray<kvs::Real32>() );
            pointObject->setColors( kvs::ValueArray<kvs::UInt8>() );
            pointObject->setNormals( kvs::ValueArray<kvs::Real32>() );
            return false;
        }

        ParticleMonitor pm;
        const std::string particlePrefix = particleDirectoryPrefix( prefix, subdir );
        std::cout << "[Worker][StatisticParticle] read statistic=" << statistic
                  << ", prefix=" << particlePrefix
                  << ", timestep=" << timeStep << std::endl;
        pm.setParticleFilePrefix( particlePrefix );
        pm.check();
        if ( !pm.stepExisted() )
        {
            std::cout << "[Worker][StatisticParticle] step does not exist for prefix="
                      << particlePrefix << std::endl;
            pointObject->setCoords( kvs::ValueArray<kvs::Real32>() );
            pointObject->setColors( kvs::ValueArray<kvs::UInt8>() );
            pointObject->setNormals( kvs::ValueArray<kvs::Real32>() );
            return false;
        }

        /*  メンバー指定のときは、要求ステップのファイルが揃っているか先に確認する。
         *  揃っていない状態で readParticleFile() に渡すとサーバー全体がハングする
         *  （memberStepFilesReady のコメント参照）。
         *  subdir が空でない＝メンバー指定。統計量（ave/var/cov）は毎ステップ出力され、
         *  かつ総数がノード数で変わるためここでは確認しない。 */
        if ( !subdir.empty() && !memberStepFilesReady( particlePrefix, timeStep ) )
        {
            std::cout << "[Worker][StatisticParticle] skip: member step is missing. statistic="
                      << statistic << ", timestep=" << timeStep << std::endl;
            pointObject->setCoords( kvs::ValueArray<kvs::Real32>() );
            pointObject->setColors( kvs::ValueArray<kvs::UInt8>() );
            pointObject->setNormals( kvs::ValueArray<kvs::Real32>() );
            return false;
        }

        pm.setTimeStep_particle( timeStep );
        pm.readParticleFile();
        std::cout << "[Worker][StatisticParticle] monitor subpixel="
                  << pm.getSubpixelLevel()
                  << ", requested timestep=" << timeStep
                  << std::endl;


        vismodule::PointObject* vismodulePointObject = new vismodule::PointObject;
        pm.getParticle( vismodulePointObject );

        kvs::ValueArray<kvs::Real32> kvsCoords;
        kvs::ValueArray<kvs::UInt8> kvsColors;
        kvs::ValueArray<kvs::Real32> kvsNormals;

        const size_t ncoords = vismodulePointObject->coords().size();
        const size_t ncolors = vismodulePointObject->colors().size();
        const size_t nnormals = vismodulePointObject->normals().size();

        kvsCoords.allocate( ncoords );
        kvsColors.allocate( ncolors );
        kvsNormals.allocate( nnormals );

        std::memcpy( kvsCoords.data(), vismodulePointObject->coords().pointer(), vismodulePointObject->coords().byteSize() );
        std::memcpy( kvsColors.data(), vismodulePointObject->colors().pointer(), vismodulePointObject->colors().byteSize() );
        std::memcpy( kvsNormals.data(), vismodulePointObject->normals().pointer(), vismodulePointObject->normals().byteSize() );

        pointObject->setCoords( kvsCoords );
        pointObject->setColors( kvsColors );
        pointObject->setNormals( kvsNormals );
        std::cout << "[Worker][StatisticParticle] loaded vertices="
                  << pointObject->numberOfVertices()
                  << ", coords=" << pointObject->coords().size()
                  << ", colors=" << pointObject->colors().size()
                  << ", normals=" << pointObject->normals().size()
                  << std::endl;

        delete vismodulePointObject;
        return true;
    }

public:
    static std::string toNativePath( const std::string& path )
    {
        std::string nativePath = path;
#ifdef _WIN32
        for ( char& c : nativePath )
            if ( c == '/' ) c = '\\';
#endif
        return nativePath;
    }
};

#endif // WORKER_H
