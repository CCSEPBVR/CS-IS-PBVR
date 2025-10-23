#ifndef OBJECTEDITORWIP_H
#define OBJECTEDITORWIP_H

#include <QDockWidget>
#include <QList>
#include <QFileDialog>
#include <QColorDialog>

#include "Screen.h"

#include "../../Shared/StringProcessor.h"

namespace Ui
{
class ObjectEditorWIP;
}

class ObjectEditorWIP : public QDockWidget
{
    Q_OBJECT

public:
    explicit ObjectEditorWIP(QWidget *parent = nullptr);
    ~ObjectEditorWIP();

private:
    // メンバ変数群
    Ui::ObjectEditorWIP *ui;

    QStandardItemModel *m_model = nullptr;

    QList<QWidget*> m_group_common_object_widgets;              // 全オブジェクト共通で表示させるウィジェット群
    QList<QWidget*> m_group_common_server_point_object_widgets; // サーバポイントオブジェクト(ClientServer/In-Situ)で表示させるウィジェット群
    QList<QWidget*> m_group_client_server_point_object_widgets; // サーバポイントオブジェクト(ClientServer)で表示させるウィジェット群
    QList<QWidget*> m_group_nontexture_polygon_object_widgets;  // テクスチャ無しポリゴンオブジェクトで表示させるウィジェット群

    kvs::qt::jaea::Screen* m_screen = nullptr;

    // メソッド群
    void initialize();
    void toggleCommonObjectWidgets( bool isObject );                                // true:オブジェクトである。
    void toggleCommonServerObjectWidgets( bool isCommonServerObject );              // true:サーバポイントオブジェクトである。(ClientServer/Insitu共通)
    void toggleClientServerObjectWidgets( bool isClientServerObject );              // true:サーバポイントオブジェクトである。(ClientServer)
    void toggleNontexturePolygonObjectWidgets( bool isNonTexturePolygonObject );    // true:テクスチャ無しポリゴンオブジェクトである。(.stl, .kvsml)
    void calculateTotalMinMaxTimeStep();

private slots:
    void onItemSelection( const QItemSelection &selected, const QItemSelection &deselected );

    // 全オブジェクト共通
    void onFocusCheckBoxToggled( bool checked );

    // サーバポイントオブジェクトである。(ClientServer/Insitu共通)
    void onParticleLimitSpinBoxValueChanged( int value );
    void onDensityDoubleSpinBoxValueChanged( double value );

    // サーバポイントオブジェクト(ClientServer)で表示させるウィジェット群
    void onCoordinateLineEditTextChanged();

    // true:テクスチャ無しポリゴンオブジェクトである。(.stl, .kvsml)
    void onColorLabelDoubleClicked();
    void onOpacityDoubleSpinBoxValueChanged( double value );

    void onBrowse(); // ...ボタン(ファイルブラウザ)が押された時
    void onDelete();
    void onApply();
};

#endif // OBJECTEDITORWIP_H
