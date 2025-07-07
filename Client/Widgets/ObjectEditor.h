#ifndef OBJECTEDITOR_H
#define OBJECTEDITOR_H

#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QColorDialog>

#include "Screen.h"

#include <kvs/PointObject>
#include <kvs/PointImporter>
#include <kvs/PointExporter>
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
#include <kvs/StochasticTexturedPolygonRenderer>

#include "Connect.h"
#include "ObjectItem.h"
#include "MergeWorker.h"
#include "Utils.h"

namespace Ui {
class ObjectEditor;
}

class ObjectEditor : public QDockWidget
{
    Q_OBJECT

public:
    explicit ObjectEditor( kvs::qt::jaea::Screen*,
                           Connect* connect,
                           QWidget *parent = nullptr );
    ~ObjectEditor();

private:
    Ui::ObjectEditor *ui;
    kvs::qt::jaea::Screen* m_screen = nullptr;
    Connect* m_connect = nullptr;
    QStandardItemModel *m_model = nullptr;
    QMap<QString, QPair<QLabel*, QWidget*>> m_label_field_map;

public slots:
    void doMerge( int );
    void serverPointObjectCS( QString volumeDataFilePath, ServerPointObjectPropertiesCS );
    void serverGlyphObjectCS( QString volumeDataFilePath, ServerGlyphObjectPropertiesCS );
    void serverPointObjectIS( QString volumeDataFilePath, ServerPointObjectPropertiesIS );
    void serverGlyphObjectIS( QString volumeDataFilePath, ServerGlyphObjectPropertiesIS );
    void updateInSituTimeStep( int minTimeStep, int maxTimeStep );
    void requestReplaceServerPointObject();
    void requestReplaceServerGlyphObject();
    void insituObjectActive( kvs::Vec3f, kvs::Vec3f, int, float, float );

private slots:
    void onItemSelectionChanged();
    void onBrowser();
    void onDelete();
    void onApply();

    void onFocus();

    void onCoordinate1();
    void onCoordinate2();
    void onCoordinate3();
    void onExport();

    void onParticleLimit();
    void onDensity();

    void onPolygonColor();
    void onPolygonOpacity();

    void onMergeFinished( int );

private:
    void initialize();
    QString createNumberedFileName( const QStandardItem*, const int );
    std::unique_ptr<kvs::ObjectBase> importObject( const QStandardItem* formatItem, const std::string& fileName );
    void calculateTotalMinMaxTimeStep();

    void showFormRow( const QString& key );
    void hideFormRow( const QString& key );
    void hideAllRow();
    void commonProperty();                  //
    void clientServerPointObjectProperty(); // Client Server(CS)
    void inSituPointObjectProperty();       // In-Situ(IS)
    void stlPolygonObjectProperty();        // STL

    void registerObject( QStandardItem* nameItem, const QStandardItem* formatItem );
    void replaceObject( QStandardItem* nameItem, const QStandardItem* formatItem );

signals:
    void updateTotalTimeStepRange( int min, int max, bool isSingleObject );
    void mergingFinish( int requestTimeStep );
    void updateFocus( kvs::Vec3 min, kvs::Vec3 max );
    void updatePointsTranslation();
    void shading( kvs::RendererBase* );
    void updateTotalParticles( int );
    void noItems();
    void updateInSituObjectMinMaxTimeStep( int , int );
    void updateRenderParameterClientMessage( QStandardItemModel* model );
    void updateCoordinateParameterClientMessage( QStandardItemModel* model );
};
#endif // OBJECTEDITOR_H
