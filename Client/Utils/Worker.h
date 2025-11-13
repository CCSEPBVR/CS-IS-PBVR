#ifndef WORKER_H
#define WORKER_H
#include <QObject>
#include <QStandardItem>

#include "Screen.h"
#include "../../Shared/ObjectInfoExtractor.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker( int requestTimeStep, QStandardItemModel* model, kvs::qt::jaea::Screen* screen, QObject* parent = nullptr )
        : QObject( parent )
        , m_request_time_step( requestTimeStep )
        , m_model( model )
        , m_screen( screen )
    {
    }

public slots:
    void process()
    {
        for( int row = 0; row < m_model->rowCount(); row++ )
        {
            QStandardItem* item = m_model->item( row, 0 ); // UserRoleにObjectInfoが格納されている列
            if( !item ) continue;

            QVariant var = item->data( Qt::UserRole );
            if( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

            ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();

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
                            m_screen->scene()->object( info.objectID.first )->show();
                            info.object = nullptr;
                        }
                        else
                        {
                            if( !info.isRemote ) importObject( info, resultTimeStep );
                            info.currentImportedTimeStep = resultTimeStep;
                            info.needSameTimeStepReplace = false;
                        }
                    }
                    else
                    {
                        if( !info.isRemote ) importObject( info, resultTimeStep );
                        info.currentImportedTimeStep = resultTimeStep;
                        info.needSameTimeStepReplace = false;
                    }

                    QVariant newVar;
                    newVar.setValue( info );
                    item->setData( newVar, Qt::UserRole );
                }
                else
                {
                    if( info.objectID.first == -1 && info.objectID.second == -1 ) // 一度も登録されてない場合
                    {

                    }
                    else // 登録されている場合
                    {
                        m_screen->scene()->object( info.objectID.first )->hide();
                    }
                    info.object = nullptr;
                    QVariant newVar;
                    newVar.setValue( info );
                    item->setData( newVar, Qt::UserRole );
                }
            }
            else
            {
                if( info.objectID.first == -1 && info.objectID.second == -1 ) // 一度も登録されてない場合
                {

                }
                else // 登録されている場合
                {
                    m_screen->scene()->object( info.objectID.first )->hide();
                }
                info.object = nullptr;
                QVariant newVar;
                newVar.setValue( info );
                item->setData( newVar, Qt::UserRole );
            }
        }
        emit done();
    }

signals:
    void done(); // 完了通知

private:
    int m_request_time_step;
    QStandardItemModel* m_model;
    kvs::qt::jaea::Screen* m_screen = nullptr;

    void importObject( ObjectInfoExtractor::ObjectInfo& info, const int&  requestTimeStep )
    {
        std::string fileName = createFileName( info, requestTimeStep );

        std::unique_ptr<kvs::PointObject> pointObject;
        std::unique_ptr<kvs::PolygonObject> polygonObject;
        std::unique_ptr<kvs::LineObject> lineObject;
#ifdef ASSIMP
        std::unique_ptr<kvs::TexturedPolygonObject> texturedPolygonObject;
#endif
        switch( info.format )
        {
        case ObjectInfoExtractor::ClientServerPointObject:
            break;
        case ObjectInfoExtractor::InsituServerPointObject:
            break;
        case ObjectInfoExtractor::ServerGlyphObject:
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
};

#endif // WORKER_H
