#ifndef OBJECTEDITOR_H
#define OBJECTEDITOR_H
#include <unordered_set>

#include <QDockWidget>
#include <QFileDialog>
#include <QColorDialog>

#include "Screen.h"
#include <kvs/ParticleBasedRenderer>
#include <kvs/StochasticPolygonRenderer>
#include <kvs/StochasticLineRenderer>
#include <kvs/StochasticTexturedPolygonRenderer>

#include "WebSocketPair.h"
#include "VizMode.h"
#include "Worker.h"
#include "RemoteFileDialog.h"

#include "../../Shared/JsonKeys.h"
#include "../../Shared/ObjectInfoExtractor.h"

namespace Ui
{
class ObjectEditor;
}

class ObjectEditor : public QDockWidget
{
    Q_OBJECT

public:
    explicit ObjectEditor( kvs::qt::jaea::Screen* screen, WebSocketPair* websockets, Viz::Mode* vizMode, QWidget *parent = nullptr );
    ~ObjectEditor();

public:
    void onUpdateServerState( bool serverState );     // true:接続中
    void onOperatorStateUpdate( bool operatorState ); // true:権限あり
    void onReset();
    void onReceiveSelectedFile( const QJsonObject& dataArray );
    void onReceiveObjectDelete( const QJsonObject& dataArray );
    void onReceiveObjectInfoParameter( const QJsonObject& dataArray );
    void onRequestDataAt( int requestTimeStep );
    void onTransferFunctionUpdate();
    void onGlyphParameterUpdate();
    void onUnpack( const QByteArray& binary );
    void onUpdateMaxTimeStep( const int latest );
    // FIXME:KPI
    void onLoadParameter( const QString& filePath );
    void onSaveParameter( const QString& filePath );

signals:
    void updateNumberOfVector( const int numberOfVector );
    void updateTotalTimeStepRange( int min, int max, bool isSingleObject );
    void updateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords );
    void updateTranslation();
    void shading( kvs::RendererBase* );
    void dataRequestCompleted( int requestTimeStep );

private:
    Ui::ObjectEditor *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;
    WebSocketPair* m_web_sockets    = nullptr;
    Viz::Mode* m_viz_mode           = nullptr;

    bool m_is_operator;

    QStandardItemModel *m_model     = nullptr;

    QList<QWidget*> m_group_common_object_widgets;              // 全オブジェクト共通で表示させるウィジェット群
    QList<QWidget*> m_group_common_server_point_object_widgets; // サーバポイントオブジェクト(ClientServer/In-Situ)で表示させるウィジェット群
    QList<QWidget*> m_group_client_server_point_object_widgets; // サーバポイントオブジェクト(ClientServer)で表示させるウィジェット群
    QList<QWidget*> m_group_nontexture_polygon_object_widgets;  // テクスチャ無しポリゴンオブジェクトで表示させるウィジェット群

    // NOTE:フォーカス対象の最小最大オブジェクトの計算結果格納用変数
    kvs::Vec3 m_result_min_object_coords;
    kvs::Vec3 m_result_max_object_coords;

    void updateEditorFromIndex( const QModelIndex& index );
    void setWidgetsVisible( const QList<QWidget*>& widgets, const bool visible );
    void addObjectToModel( ObjectInfoExtractor::ObjectInfo& objectInfo );
    void calculateTotalMinMaxTimeStep();

    template<typename F>
    void updateSelectedObject( F func );

    void updateVisibility( int requestTimeStep );
    void registerObject( ObjectInfoExtractor::ObjectInfo& info );
    void replaceObject( ObjectInfoExtractor::ObjectInfo& info );

private slots:
    void onItemSelection( const QItemSelection &selected, const QItemSelection &deselected );

    // 全オブジェクト共通
    void onFocusCheckBoxToggled( bool checked );

    // サーバポイントオブジェクト(ClientServer/In-Situ共通)
    void onParticleLimitSpinBoxValueChanged( int value );
    void onExtraOpacityFactorDoubleSpinBoxValueChanged( double value );

    // サーバポイントオブジェクト(ClientServerのみ)
    void onCoordinateLineEditTextChanged();

    // テスクチャ無しポリゴン(.stlのみ) // FIXME:KVSMLPolygonObjectは不透明度のみ操作できるようにしたほうがいいかもしれません。
    void onColorLabelDoubleClicked();
    void onOpacityDoubleSpinBoxValueChanged( double value );

    void onBrowse();
    void onDelete();
    void onApply();

    void dataRequestComplete( int requestTimeStep );
};

#endif // OBJECTEDITOR_H
