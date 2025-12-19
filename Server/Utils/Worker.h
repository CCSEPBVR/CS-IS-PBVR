#ifndef WORKER_H
#define WORKER_H

#include <functional>

#include "../../Shared/ObjectInfoExtractor.h"
#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/GenerateParticle>
#include <vismodule/GenerateGlyph>
#include <vismodule/Camera>
#include <vismodule/TransferFunctionSynthesizer>

class Worker
{
public:
    using DoneCallBack = std::function<void()>;

    explicit Worker( 
        const int requestTimeStep, std::vector<ObjectInfoExtractor::ObjectInfo>* objects,
        ParticleProperty* particle_property, GlyphProperty* glyph_property,
        PlotOverLineProperty* pol_property, MultiVolumePropertyList* multi_volume_property_list
    )
        : m_request_time_step( requestTimeStep ) , m_objects( objects )
        , m_particle_property( particle_property ), m_glyph_property( glyph_property )
        , m_pol_property( pol_property ), m_multi_volume_property_list( multi_volume_property_list )
    {
        camera.setWindowSize( 620, 620 ); // クライアントから送信されるようになったら削除
    }

    void setDoneCallBack( DoneCallBack callBack ) { m_done_call_back = std::move( callBack ); }

    void process()
    {
        for( auto& info : *m_objects )
        {
            int resultTimeStep = -1;
            if( info.isDisplay )
            {
                // info.timeStep.first ～ info.timeStep.second の範囲内に m_time_step が含まれる場合、
                // 対応するタイムステップとして resultTimeStep に設定する
                if( m_request_time_step >= info.timeStep.first && m_request_time_step <= info.timeStep.second )
                {
                    resultTimeStep = m_request_time_step;
                }

                if( m_request_time_step < info.timeStep.first )
                {
                    if( info.isKeepInitial )
                    {
                        resultTimeStep = info.timeStep.first;
                    }
                }

                if( m_request_time_step > info.timeStep.second )
                {
                    if( info.isKeepFinal )
                    {
                        resultTimeStep = info.timeStep.second;
                    }
                }

                if( resultTimeStep != -1 )
                {
                    if( resultTimeStep == info.currentImportedTimeStep )
                    {
                        if( info.needSameTimeStepReplace == false )
                        {
                            info.object = nullptr;
                        }
                        else
                        {
                            importObject( info, resultTimeStep, m_particle_property, m_glyph_property, m_pol_property, m_multi_volume_property_list );
                            info.currentImportedTimeStep = resultTimeStep;
                        }
                    }
                    else
                    {
                        importObject( info, resultTimeStep, m_particle_property, m_glyph_property, m_pol_property, m_multi_volume_property_list );
                        info.currentImportedTimeStep = resultTimeStep;
                    }
                }
                else
                {
                    info.object = nullptr;
                }
            }
            else
            {
                info.object = nullptr;
            }
        }
        if( m_done_call_back ) m_done_call_back();
    }

private:
    int m_request_time_step;
    std::vector<ObjectInfoExtractor::ObjectInfo>* m_objects;
    DoneCallBack m_done_call_back;
    ParticleProperty* m_particle_property;
    GlyphProperty* m_glyph_property;
    PlotOverLineProperty* m_pol_property;
    MultiVolumePropertyList* m_multi_volume_property_list;
    vismodule::Camera camera;

    void importObject(
        ObjectInfoExtractor::ObjectInfo& info, const int&  requestTimeStep,
        ParticleProperty* particle_property, GlyphProperty* glyph_property,
        PlotOverLineProperty* pol_property, MultiVolumePropertyList* multi_volume_property_list
    )
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
            pointObject = std::make_unique<kvs::PointObject>();
            SetParticleParameterCS( fileName, requestTimeStep, &camera, *particle_property, *multi_volume_property_list );
            GenerateParticleCS( *particle_property, *multi_volume_property_list, pointObject );
            info.object = pointObject.release();
            break;
        case ObjectInfoExtractor::InsituServerPointObject:
            SetParticleParameterIS( requestTimeStep, &camera, *particle_property, *multi_volume_property_list );
            GenerateParticleIS( *particle_property, *multi_volume_property_list, pointObject );
            info.object = pointObject.release();
            break;
        case ObjectInfoExtractor::ServerGlyphObject:
            SetGlyphParameterCS( *particle_property, *glyph_property, *multi_volume_property_list );
            polygonObject = GenerateGlyphCS( *particle_property, *glyph_property, *multi_volume_property_list );
            info.object = polygonObject.release();
            break;
        case ObjectInfoExtractor::PointObjectKVSML:
        case ObjectInfoExtractor::PointObjectLAS:
        case ObjectInfoExtractor::PointObjectPTS:
            pointObject = std::make_unique<kvs::PointImporter>( fileName );
            pointObject.get()->setMinMaxObjectCoords( info.currentMinObjectCoord, info.currentMaxObjectCoord );
            pointObject.get()->setMinMaxExternalCoords( info.currentMinObjectCoord, info.currentMaxObjectCoord );
            info.object = pointObject.release();
            break;
        case ObjectInfoExtractor::PolygonObjectKVSML:
        case ObjectInfoExtractor::PolygonObjectSTL:
            polygonObject = std::make_unique<kvs::PolygonImporter>( fileName );
            polygonObject->setMinMaxObjectCoords( info.currentMinObjectCoord, info.currentMaxObjectCoord );
            polygonObject->setMinMaxExternalCoords( info.currentMinObjectCoord, info.currentMaxObjectCoord );
            polygonObject->setColor( kvs::RGBColor( info.polygonColor ) );
            polygonObject->setOpacity( info.polygonOpacity * 255 );
            info.object = polygonObject.release();
            break;
#ifdef ASSIMP
        case ObjectInfoExtractor::PolygonObject3DS:
        case ObjectInfoExtractor::PolygonObjectFBX:
            texturedPolygonObject = std::make_unique<kvs::TexturedPolygonImporter>( fileName );
            texturedPolygonObject->setMinMaxObjectCoords( info.currentMinObjectCoord, info.currentMaxObjectCoord );
            texturedPolygonObject->setMinMaxExternalCoords( info.currentMinObjectCoord, info.currentMaxObjectCoord );
            info.object = texturedPolygonObject.release();
            break;
#endif
        case ObjectInfoExtractor::LineObjectKVSML:
            lineObject = std::make_unique<kvs::LineImporter>( fileName );
            lineObject.get()->setMinMaxObjectCoords( info.currentMinObjectCoord, info.currentMaxObjectCoord );
            lineObject.get()->setMinMaxExternalCoords( info.currentMinObjectCoord, info.currentMaxObjectCoord );
            info.object = lineObject.release();
            break;
        default:
            return;
        }
    }

    std::string createFileName( const ObjectInfoExtractor::ObjectInfo& info, const int& requestTimeStep )
    {
#ifdef _WIN32
        const char path_sep = '\\';
#else
        const char path_sep = '/';
#endif
        std::ostringstream oss;
        oss << info.directory << path_sep
            << info.name << "_"
            << std::setw(5) << std::setfill('0') << requestTimeStep
            << info.extension;

        const std::string fullPath = oss.str();
        return fullPath;
    }

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
