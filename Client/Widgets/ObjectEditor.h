#ifndef OBJECTEDITOR_H
#define OBJECTEDITOR_H

#include <unordered_set>

#include <QDockWidget>
#include <QColorDialog>
#include <QFileDialog>

#include <kvs/ParticleBasedRenderer>
#include <kvs/StochasticLineRenderer>
#include <kvs/StochasticPolygonRenderer>
#include <kvs/StochasticTexturedPolygonRenderer>
#include "Screen.h"

#include "VizMode.h"
#include "WebSocketPair.h"
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
    void reset();

signals:
    void updateStatusBarMessage( const QString& message );
    void updateNumberOfVector( const int numberOfVector );
    void updateTotalTimeStepRange( int min, int max, const bool isSingleObject );
    void updateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords );
    void updateTranslation();
    void shading( kvs::RendererBase* );
    void dataRequestCompleted( const int requestTimeStep );

public slots:
    void onOperatorStateUpdate( const bool operatorState ); // true: 権限有り, false: 権限無し

    // NOTE:バイナリソケット用
    void onUnpack( const QByteArray& binary );

    // NOTE:テキストソケット用
    void onReceiveObjectInfoParameter( const QJsonObject& payload );

    void onReceiveSelectedFile( const QJsonObject& payload );
    void onReceiveObjectDelete( const QJsonObject& payload );

    void onUpdateObjectLatestTimeStep( const QJsonObject& payload );

    void onGlyphParameterUpdate();
    void onTransferFunctionUpdate();

    void onRequestDataAt( int requestTimeStep );

    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    Ui::ObjectEditor *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;

    WebSocketPair* m_web_sockets    = nullptr;
    Viz::Mode* m_viz_mode           = nullptr;

    bool m_is_operator              = false;

    QStandardItemModel *m_model     = nullptr;

    QList<QWidget*> m_group_common_object_widgets;              // 全オブジェクト共通で表示させるウィジェット群
    QList<QWidget*> m_group_common_server_point_object_widgets; // サーバポイントオブジェクト(ClientServer/In-Situ)で表示させるウィジェット群
    QList<QWidget*> m_group_client_server_point_object_widgets; // サーバポイントオブジェクト(ClientServer)で表示させるウィジェット群
    QList<QWidget*> m_group_nontexture_polygon_object_widgets;  // テクスチャ無しポリゴンオブジェクトで表示させるウィジェット群

    // NOTE:フォーカス対象の最小最大オブジェクトの計算結果格納用変数
    kvs::Vec3 m_result_min_object_coords;
    kvs::Vec3 m_result_max_object_coords;

    void setWidgetsVisible( const QList<QWidget*>& widgets, const bool visible );
    void updateUI( const QModelIndex& index );

    template<typename F>
    void updateSelectedObjectInfoParameter( F func );

    void addObjectInfoToModel( ObjectInfoExtractor::ObjectInfo& objectInfo );
    void calculateTotalMinMaxTimeStep();

    void sendNeedSameTimeStepReplacePatches( const std::function<bool( ObjectInfoExtractor::Format )>& isTargetFormat );

    void updateVisibility( const int requestTimeStep );
    void registerObject( ObjectInfoExtractor::ObjectInfo& info );
    void replaceObject( ObjectInfoExtractor::ObjectInfo& info );
    void dataRequestComplete( const int requestTimeStep );

private slots:
    void onSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

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
};

#endif // OBJECTEDITOR_H
