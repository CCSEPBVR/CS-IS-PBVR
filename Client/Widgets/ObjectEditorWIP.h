#ifndef OBJECTEDITORWIP_H
#define OBJECTEDITORWIP_H

#include <unordered_set>

#include <QDockWidget>
#include <QList>
#include <QFileDialog>
#include <QColorDialog>

#include "Screen.h"

#include <kvs/ParticleBasedRenderer>
#include <kvs/StochasticPolygonRenderer>
#include <kvs/StochasticLineRenderer>
#include <kvs/StochasticTexturedPolygonRenderer>

#include "WebSocketPair.h"
#include "VizMode.h"
#include "RemoteFileDialog.h"
#include "Worker.h"

#include "../../Shared/ObjectInfoExtractor.h"

namespace Ui
{
class ObjectEditorWIP;
}

class ObjectEditorWIP : public QDockWidget
{
    Q_OBJECT

public:
    explicit ObjectEditorWIP( WebSocketPair* websockets, Viz::Mode* vizMode, kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~ObjectEditorWIP();

    void updateOperatorState( bool operatorState ); // true:権限あり
    void reset();

public slots:
    // void addObjectToModel( const ObjectInfoExtractor::ObjectInfo& objectInfo, ObjectInfoExtractor& oie );
    void addObjectToModel( const ObjectInfoExtractor::ObjectInfo& objectInfo );
    void unpack( const QByteArray& binary );
    void objectInfoUpdate( const QJsonArray& resultMinObjectCoordsArray, const QJsonArray& resultMaxObjectCoordsArray, const QJsonArray& objects );
    void showAtTimeStep( int requestTimeStep );
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );

signals:
    void updateTotalTimeStepRange( int min, int max, bool isSingleObject );
    void updateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords );
    void updateTranslation();
    void shading( kvs::RendererBase* );
    void done( int requestTimeStep );

private:
    Ui::ObjectEditorWIP *ui;

    WebSocketPair* m_web_sockets = nullptr;
    const Viz::Mode* m_viz_mode = nullptr;

    QStandardItemModel *m_model = nullptr;

    QList<QWidget*> m_group_common_object_widgets;              // 全オブジェクト共通で表示させるウィジェット群
    QList<QWidget*> m_group_common_server_point_object_widgets; // サーバポイントオブジェクト(ClientServer/In-Situ)で表示させるウィジェット群
    QList<QWidget*> m_group_client_server_point_object_widgets; // サーバポイントオブジェクト(ClientServer)で表示させるウィジェット群
    QList<QWidget*> m_group_nontexture_polygon_object_widgets;  // テクスチャ無しポリゴンオブジェクトで表示させるウィジェット群

    kvs::qt::jaea::Screen* m_screen = nullptr;

    void initialize();
    void toggleCommonObjectWidgets( bool isObject );                                // true:オブジェクトである。
    void toggleCommonServerObjectWidgets( bool isCommonServerObject );              // true:サーバポイントオブジェクトである。(ClientServer/Insitu共通)
    void toggleClientServerObjectWidgets( bool isClientServerObject );              // true:サーバポイントオブジェクトである。(ClientServer)
    void toggleNontexturePolygonObjectWidgets( bool isNonTexturePolygonObject );    // true:テクスチャ無しポリゴンオブジェクトである。(.stl, .kvsml)    
    void calculateTotalMinMaxTimeStep();

    template<typename F>
    void updateSelectedObject( F func );

    void registerObject( ObjectInfoExtractor::ObjectInfo& info );
    void replaceObject( ObjectInfoExtractor::ObjectInfo& info );

    void updateVisibility( int requestTimeStep );

private slots:
    void onItemSelection( const QItemSelection &selected, const QItemSelection &deselected );

    // 全オブジェクト共通
    void onFocusCheckBoxToggled( bool checked );

    // サーバポイントオブジェクトである。(ClientServer/Insitu共通)
    void onParticleLimitSpinBoxValueChanged( int value );
    void onExtraOpacityFactorDoubleSpinBoxValueChanged( double value );

    // サーバポイントオブジェクト(ClientServer)で表示させるウィジェット群
    void onCoordinateLineEditTextChanged();

    // true:テクスチャ無しポリゴンオブジェクトである。(.stl, .kvsml)
    void onColorLabelDoubleClicked();
    void onOpacityDoubleSpinBoxValueChanged( double value );

    void onBrowse(); // ...ボタン(ファイルブラウザ)が押された時
    void onDelete();
    void onApply();

    void doneObjectEditor( int requestTimeStep );
};

#endif // OBJECTEDITORWIP_H
