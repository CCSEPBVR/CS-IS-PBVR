#ifndef MERGEWORKER_H
#define MERGEWORKER_H
#include <QObject>
#include <QStandardItem>

#include "Screen.h"

#include <kvs/PointObject>
#include <kvs/PointImporter>
#include <kvs/ParticleBasedRenderer>

#include <kvs/PolygonObject>
#include <kvs/PolygonImporter>
#include <kvs/StochasticPolygonRenderer>

#include <kvs/LineObject>
#include <kvs/LineImporter>
#include <kvs/StochasticLineRenderer>

#include <kvs/TexturedPolygonObject>
#if defined(ASSIMP)
#include "TexturedPolygonImporter.h"
#endif
#include <kvs/StochasticPolygonRenderer>

#include "Connect.h"
#include "ObjectItem.h"
#include "Utils.h"

class MergeWorker : public QObject
{
    Q_OBJECT

public:
    explicit MergeWorker( int requestTimeStep,
                          QStandardItemModel* model,
                          kvs::qt::jaea::Screen* screen,
                          Connect* connect,
                          QObject* parent = nullptr )
        : QObject( parent )
        , m_request_time_step( requestTimeStep )
        , m_model( model )
        , m_screen( screen )
        , m_connect( connect ) {}

private:
    int m_request_time_step;
    QStandardItemModel* m_model;
    kvs::qt::jaea::Screen* m_screen;
    Connect* m_connect = nullptr;

signals:
    void finished(); // 終了通知

public slots:
    void process()
    {
        // 登録されているアイテム分ループする。
        for( int row = 0; row < m_model->rowCount(); row++ )
        {
            QStandardItem* nameItem                 = m_model->item( row, 0 );
            QStandardItem* formatItem               = m_model->item( row, 1 );
            QStandardItem* displayCheckBoxItem      = m_model->item( row, 2 );
            QStandardItem* keepInitialCheckBoxItem  = m_model->item( row, 3 );
            QStandardItem* keepFinalCheckBoxItem    = m_model->item( row, 4 );

            if( !nameItem || !formatItem || !displayCheckBoxItem || !keepInitialCheckBoxItem || !keepFinalCheckBoxItem )
            {
                continue;
            }

            QString extension = nameItem->data( ObjectItem::nameItemRole::Extension ).toString();
            QString directoryPath = nameItem->data( ObjectItem::nameItemRole::DirectoryPath ).toString();
            QPair<int,int> minMaxTimeStep = nameItem->data( ObjectItem::nameItemRole::MinMaxTimeStep ).value<QPair<int,int>>();
            QPair<int,int> ids = nameItem->data( ObjectItem::nameItemRole::Ids ).value<QPair<int, int>>();
            bool needSameTimeStepReplace = nameItem->data( ObjectItem::nameItemRole::RequestReplace ).toBool();
            bool focusCheckBox = nameItem->data( ObjectItem::nameItemRole::TemporaryFocus ).toBool();
            bool isFocus = nameItem->data( ObjectItem::nameItemRole::CurrentFocus ).toBool();
            kvs::Vec3 initialMinObjectCoords = nameItem->data( ObjectItem::nameItemRole::InitialMinObjectCoord ).value<kvs::Vec3>();
            kvs::Vec3 initialMaxObjectCoord = nameItem->data( ObjectItem::nameItemRole::InitialMaxObjectCoord ).value<kvs::Vec3>();
            kvs::Vec3 initialMinExternalCoord = nameItem->data( ObjectItem::nameItemRole::InitialMinExternalCoord ).value<kvs::Vec3>();
            kvs::Vec3 initialMaxExternalCoord = nameItem->data( ObjectItem::nameItemRole::InitialMaxExternalCoord ).value<kvs::Vec3>();
            int alreadyImportedTimeStep = nameItem->data( ObjectItem::nameItemRole::ImportedTimeStep ).toInt();

            QString formatText = formatItem->text();
            int formatValue = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).toInt();

            if( formatValue == ObjectItem::Format::ServerPointObjectIS )
            {
                needSameTimeStepReplace = true;
            }

            bool isDisplay = displayCheckBoxItem->data( ObjectItem::DisplayCheckBoxItemRole::Isdisplay ).toBool();
            bool isKeepInitial = keepInitialCheckBoxItem->data( ObjectItem::KeepInitialCheckBoxItemRole::IskeepInitial ).toBool();
            bool isKeepFinal = keepFinalCheckBoxItem->data( ObjectItem::KeepFinalCheckBoxItemRole::IskeepFinal ).toBool();

            if( isDisplay )
            {
                int resultTimeStep = -1;  // 実際に使用するタイムステップ（有効な範囲内なら更新、それ以外は -1 のまま）
                if( m_request_time_step >= minMaxTimeStep.first && m_request_time_step <= minMaxTimeStep.second )
                {
                    resultTimeStep = m_request_time_step;
                }

                if( m_request_time_step < minMaxTimeStep.first )
                {
                    if( isKeepInitial )
                    {
                        resultTimeStep = minMaxTimeStep.first;
                    }
                }

                if( m_request_time_step > minMaxTimeStep.second )
                {
                    if( isKeepFinal )
                    {
                        resultTimeStep = minMaxTimeStep.second;
                    }
                }

                if( resultTimeStep != -1 )
                {
                    // alreadyImportedTimeStep = -1; // AFTER FOR DEBUG
                    if( resultTimeStep == alreadyImportedTimeStep )
                    {
                        if( needSameTimeStepReplace == false )
                        {
                            m_screen->scene()->object( ids.first )->show();
                            nameItem->setData( QVariant(), ObjectItem::nameItemRole::Object );
                        }
                        else
                        {
                            importObject( nameItem, formatItem, resultTimeStep );
                            nameItem->setData( QVariant::fromValue( resultTimeStep ), ObjectItem::nameItemRole::ImportedTimeStep );
                            nameItem->setData( QVariant::fromValue( false ), ObjectItem::nameItemRole::RequestReplace );
                        }
                    }
                    else
                    {
                        importObject( nameItem, formatItem, resultTimeStep );
                        nameItem->setData( QVariant::fromValue( resultTimeStep ), ObjectItem::nameItemRole::ImportedTimeStep );
                        nameItem->setData( QVariant::fromValue( false ), ObjectItem::nameItemRole::RequestReplace );
                    }
                }
                else
                {
                    if( ids.first == -1 && ids.second == -1 ) // 一度も登録されてない場合
                    {

                    }
                    else // 登録されている場合
                    {
                        m_screen->scene()->object( ids.first )->hide();
                    }
                    nameItem->setData( QVariant(), ObjectItem::nameItemRole::Object );
                }
            }
            else
            {
                if( ids.first == -1 && ids.second == -1 ) // 一度も登録されてない場合
                {

                }
                else // 登録されている場合
                {
                    m_screen->scene()->object( ids.first )->hide();
                }
                nameItem->setData( QVariant(), ObjectItem::nameItemRole::Object );
            }
        }
        emit finished();
    }

private:
    void importObject( QStandardItem* nameItem, const QStandardItem* formatItem, const int& resultTimeStep )
    {
        std::string fileName = Utils::toNativePath( createNumberdFileName( nameItem, resultTimeStep ) );

        QColor polygonColor = nameItem->data( ObjectItem::nameItemRole::CurrentPolygonColor ).value<QColor>();
        float polygonOpacity = nameItem->data( ObjectItem::nameItemRole::CurrentPolygonOpacity ).value<float>();
        QPair<int,int> ids = nameItem->data( ObjectItem::nameItemRole::Ids ).value<QPair<int, int>>();
        enum ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>();

        std::unique_ptr<kvs::PointObject> pointObject;
        std::unique_ptr<kvs::glsl::ParticleBasedRenderer> particleBasedRenderer;
        std::unique_ptr<kvs::PolygonObject> polygonObject;
        std::unique_ptr<kvs::LineObject> lineObject;
        std::unique_ptr<kvs::TexturedPolygonObject> texturedPolygonObject;

        kvs::Vec3 currentMinObjectCoords = nameItem->data( ObjectItem::nameItemRole::CurrentMinObjectCoord ).value<kvs::Vec3>();
        kvs::Vec3 currentMaxObjectCoords = nameItem->data( ObjectItem::nameItemRole::CurrentMaxObjectCoord ).value<kvs::Vec3>();

        kvs::Vec3 min = currentMinObjectCoords;
        kvs::Vec3 max = currentMaxObjectCoords;

        switch( format )
        {
        case ObjectItem::Format::ServerPointObjectCS:
        case ObjectItem::Format::ServerPointObjectIS:
            pointObject = std::unique_ptr<kvs::PointObject>( m_connect->generateParticles( resultTimeStep ) );
            if( pointObject == nullptr )
            {
                qWarning() << "OBJECT IS NULLPTR";
                nameItem->setData( QVariant::fromValue(static_cast<kvs::PointObject*>(nullptr)), ObjectItem::nameItemRole::Object );
            }
            else
            {
                pointObject->setXform( m_screen->scene()->objectManager()->xform() );
                pointObject->setMinMaxObjectCoords( min, max );
                pointObject->setMinMaxExternalCoords( min, max );
                nameItem->setData( QVariant::fromValue( pointObject.release() ), ObjectItem::nameItemRole::Object );
            }
            break;

        case ObjectItem::Format::ServerGlyphObjectCS:
        case ObjectItem::Format::ServerGlyphObjectIS:
            polygonObject = std::unique_ptr<kvs::PolygonObject>( m_connect->generateGlyphPolygons( resultTimeStep ) );
            if( polygonObject == nullptr )
            {
                qWarning() << "OBJECT IS NULLPTR";
                nameItem->setData( QVariant::fromValue( static_cast<kvs::PolygonObject*>( nullptr ) ), ObjectItem::nameItemRole::Object );
            }
            else
            {
                polygonObject->setXform( m_screen->scene()->objectManager()->xform() );
                polygonObject.get()->setMinMaxObjectCoords( min, max );
                polygonObject.get()->setMinMaxExternalCoords( min, max );
                nameItem->setData( QVariant::fromValue( polygonObject.release() ), ObjectItem::nameItemRole::Object );
            }
            break;

        case ObjectItem::Format::PointObjectKVSML:
        case ObjectItem::Format::PointObjectLAS:
        case ObjectItem::Format::PointObjectPTS:
            pointObject = std::make_unique<kvs::PointImporter>( fileName );
            pointObject->setXform( m_screen->scene()->objectManager()->xform() );
            pointObject.get()->setMinMaxObjectCoords( min, max );
            pointObject.get()->setMinMaxExternalCoords( min, max );
            nameItem->setData( QVariant::fromValue( pointObject.release() ), ObjectItem::nameItemRole::Object );
            break;

        case ObjectItem::Format::PolygonObjectSTL:
            polygonObject = std::make_unique<kvs::PolygonImporter>( fileName );
            polygonObject->setXform( m_screen->scene()->objectManager()->xform() );
            polygonObject.get()->setMinMaxObjectCoords( min, max );
            polygonObject.get()->setMinMaxExternalCoords( min, max );
            polygonObject->setColor( kvs::RGBColor( polygonColor.red(), polygonColor.green(), polygonColor.blue() ) );
            polygonObject->setOpacity( polygonOpacity * 255 );
            nameItem->setData( QVariant::fromValue( polygonObject.release() ), ObjectItem::nameItemRole::Object );
            // nameItem->setData( QVariant::fromValue( m_screen->registerObject( polygonObject.release(), new kvs::StochasticPolygonRenderer() ) ), ObjectItem::nameItemRole::Ids );
            break;

        case ObjectItem::Format::PolygonObjectKVSML:
            polygonObject = std::make_unique<kvs::PolygonImporter>( fileName );
            polygonObject->setXform( m_screen->scene()->objectManager()->xform() );
            polygonObject.get()->setMinMaxObjectCoords( min, max );
            polygonObject.get()->setMinMaxExternalCoords( min, max );
            polygonObject->setOpacity( polygonOpacity * 255 );
            nameItem->setData( QVariant::fromValue( polygonObject.release() ), ObjectItem::nameItemRole::Object );
            break;

#if defined(ASSIMP)
#ifdef ASSIMP
        case ObjectItem::Format::PolygonObjectFBX:
        case ObjectItem::Format::PolygonObject3DS:
#endif
            texturedPolygonObject = std::make_unique<kvs::TexturedPolygonImporter>( fileName );
            texturedPolygonObject->setXform( m_screen->scene()->objectManager()->xform() );
            texturedPolygonObject.get()->setMinMaxObjectCoords( min, max );
            texturedPolygonObject.get()->setMinMaxExternalCoords( min, max );
            // texturedPolygonObject->setOpacity( polygonOpacity * 255 );
            nameItem->setData( QVariant::fromValue( texturedPolygonObject.release() ), ObjectItem::nameItemRole::Object );
            break;
#endif

        case ObjectItem::Format::LineObjectKVSML:
            lineObject = std::make_unique<kvs::LineImporter>( fileName );
            lineObject->setXform( m_screen->scene()->objectManager()->xform() );
            lineObject.get()->setMinMaxObjectCoords( min, max );
            lineObject.get()->setMinMaxExternalCoords( min, max );
            nameItem->setData( QVariant::fromValue( lineObject.release() ), ObjectItem::nameItemRole::Object );
            // nameItem->setData( QVariant::fromValue( m_screen->registerObject( lineObject.release(), new kvs::StochasticLineRenderer() ) ), ObjectItem::nameItemRole::Ids );
            break;
        default:
            return;
        }
    }

    QString createNumberdFileName( const QStandardItem* nameItem, const int requestTimeStep )
    {
        QString objectName = nameItem->text();
        QString extension = nameItem->data( ObjectItem::nameItemRole::Extension ).toString();
        QString directoryPath = nameItem->data( ObjectItem::nameItemRole::DirectoryPath ).toString();
        QString extractedNumber = QString::number( requestTimeStep ).rightJustified( 5, '0' );

        QString numberFileName = ( directoryPath + "/" + objectName + "_" + extractedNumber + extension );
        return numberFileName;
    }
};

#endif // MERGEWORKER_H
